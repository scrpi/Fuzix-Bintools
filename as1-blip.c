/*
 * BLIP assembler.
 * Assemble one line of input.
 *
 * BLIP's encoding is table-driven. Every (verb, operand-form) pair maps to one
 * opcode in isa/opcodes.toml; the table blip_optab[] (generated into
 * blip-optab.h) holds, for each, its page, byte, and trailing-operand kind. We
 * parse a line into a *normalized key* (verb + operand form with the value
 * placeholders collapsed, e.g. "LD A,(SP+n)") that matches the generated key,
 * look it up, and emit: the 0x80 prefix for a page-1 opcode (isa.md §5.1), the
 * opcode byte, then the operand bytes little-endian (isa.md §3).
 *
 * The assembly syntax is the §4.1 house style: bare $-hex immediates, register
 * as operand, parentheses = memory, no postbyte.
 */
#include	"as.h"
#include	<string.h>
#include	"blip-optab.h"

int passbegin(int pass)
{
	segment = CODE;		/* default segment */
	return 1;		/* all passes required */
}

void doflush(void)
{
}

/*
 * Register table. Names are matched directly (not via the symbol table) so we
 * keep the spelling for the key. The code is the §8.4 move-selector nibble.
 */
struct reg { const char *name; int code; int is16; };
static const struct reg regtab[] = {
	{ "D", D, 1 }, { "X", X, 1 }, { "Y", Y, 1 }, { "SP", SP, 1 },
	{ "PC", PC, 1 }, { "USP", USP, 1 },
	{ "A", A, 0 }, { "B", B, 0 }, { "CC", CC, 0 },
	{ NULL, 0, 0 }
};

static const struct reg *findreg(const char *id)
{
	const struct reg *r;
	for (r = regtab; r->name; r++)
		if (strcmp(r->name, id) == 0)
			return r;
	return NULL;
}

/* Evaluate a constant/relocatable expression into ap (shared evaluator). */
void getaddr(ADDR *ap)
{
	memset(ap, 0, sizeof(*ap));
	expr1(ap, LOPRI, 0);
}

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

/*
 * Try to read a register at the current position. On success consume it and
 * return its table entry; on failure leave the input untouched (ip is a plain
 * cursor into the line, so saving/restoring it is a clean backtrack).
 */
static const struct reg *try_reg(void)
{
	char *save = ip;
	char id[NCPS];
	const struct reg *r;
	int c = getnb();

	if (!is_symstart(c)) {
		ip = save;
		return NULL;
	}
	getid(id, c);
	r = findreg(id);
	if (r == NULL)
		ip = save;
	return r;
}

/*
 * One parsed operand: its canonical text (for the key), the single value
 * expression it may carry, and whether it is a bare register (for movsel).
 */
struct opnd {
	char	text[24];
	ADDR	val;
	int	haveval;
	int	bareexpr;	/* bare $-expr: try $i / rel / mask at lookup */
	int	plainreg;	/* operand is exactly a register */
	int	regcode;
};

/* Parse one addressing form (register-based or an expression) into o->text. */
static void parse_aform(struct opnd *o, int inparen)
{
	const struct reg *r, *r2;
	int c, c2;

	c = getnb();
	if (c == '-') {				/* pre-decrement: (-R) / (--R) */
		c2 = getnb();
		if (c2 == '-')
			strcat(o->text, "--");
		else {
			unget(c2);
			strcat(o->text, "-");
		}
		r = try_reg();
		if (r == NULL)
			qerr(NEED_REGISTER);
		strcat(o->text, r->name);
		return;
	}
	unget(c);

	r = try_reg();
	if (r) {
		strcat(o->text, r->name);
		c = getnb();
		if (c == '+') {
			c2 = getnb();
			if (c2 == ')' || c2 == '\n' || c2 == ';' || c2 == ',') {
				unget(c2);
				strcat(o->text, "+");	/* (R+) auto-inc by 1 */
				return;
			}
			if (c2 == '+') {
				strcat(o->text, "++");	/* (R++) auto-inc by 2 */
				return;
			}
			unget(c2);
			r2 = try_reg();
			if (r2) {			/* (R+A/B/D) accumulator offset */
				strcat(o->text, "+");
				strcat(o->text, r2->name);
				return;
			}
			getaddr(&o->val);		/* (R+n) signed offset */
			o->haveval = 1;
			strcat(o->text, "+n");
			return;
		}
		if (c == '-') {				/* (R-n): signed offset */
			unget('-');
			getaddr(&o->val);
			o->haveval = 1;
			strcat(o->text, "+n");
			return;
		}
		unget(c);				/* plain register */
		if (!inparen) {
			o->plainreg = 1;
			o->regcode = r->code;
		}
		return;
	}

	/* Not a register: an expression. Inside parens it is an absolute
	   address ($i); bare it is an immediate / branch target / mask, which
	   the lookup disambiguates by trying $i / rel / mask. */
	getaddr(&o->val);
	o->haveval = 1;
	strcat(o->text, "$i");
	if (!inparen)
		o->bareexpr = 1;
}

static void parse_operand(struct opnd *o)
{
	int c = getnb();

	memset(o, 0, sizeof(*o));
	if (c == '(') {
		strcat(o->text, "(");
		parse_aform(o, 1);
		if (getnb() != ')')
			qerr(BRACKET_EXPECTED);
		strcat(o->text, ")");
		return;
	}
	unget(c);
	parse_aform(o, 0);
}

/* Collect up to two table entries matching key (only off8/off16 pairs do). */
static int find_matches(const char *key, int idx[2])
{
	int n = 0, i;
	for (i = 0; i < (int)BLIP_OPTAB_N && n < 2; i++)
		if (strcmp(key, blip_optab[i].key) == 0)
			idx[n++] = i;
	return n;
}

static int fits_s8(ADDR *ap)
{
	int v = (int)(int16_t)ap->a_value;
	return ap->a_segment == ABSOLUTE && v >= -128 && v <= 127;
}

/* PC-relative branch operand (mirrors the shared relative-reloc convention). */
/*
 * PC-relative branch operand. The two relocation helpers differ for in-segment
 * targets: outrabrel writes the offset *directly* (so we compute it ourselves),
 * while outrel (via outrawrel) subtracts the location itself (so we only bias
 * to the end of the operand). Both yield an offset relative to the next
 * instruction.
 */
static void emit_rel(ADDR *ap, int size)
{
	if (ap->a_segment == ABSOLUTE) {
		if (size == 1)
			outrab(ap);
		else
			outraw(ap);
		return;
	}
	if (size == 1) {
		ap->a_value -= dot[segment];	/* dot is past the opcode byte */
		ap->a_value -= 1;		/* ... bias past the offset byte */
		outrabrel(ap);
	} else {
		ap->a_value -= 2;		/* outrel subtracts the location */
		outrawrel(ap);
	}
}

static void emit(int ent, struct opnd *o, int regsel)
{
	const struct optab *e = &blip_optab[ent];

	if (e->page == 1)
		outab(0x80);
	outab(e->byte);

	switch (e->trail) {
	case T_IMM8:
	case T_MASK8:
		/* one byte: accept signed or unsigned 8-bit when known */
		if (o->val.a_segment == ABSOLUTE) {
			int v = (int)(int16_t)o->val.a_value;
			if (v < -128 || v > 255)
				aerr(CONSTANT_RANGE);
		}
		outrab(&o->val);
		break;
	case T_OFF8:
		if (o->val.a_segment == ABSOLUTE && !fits_s8(&o->val))
			aerr(CONSTANT_RANGE);
		outrab(&o->val);
		break;
	case T_NONE:
		break;
	case T_IMM16:
	case T_OFF16:
	case T_ABS16:
		outraw(&o->val);
		break;
	case T_MOVSEL:
		outab(regsel);
		break;
	case T_REL8:
		emit_rel(&o->val, 1);
		break;
	case T_REL16:
		emit_rel(&o->val, 2);
		break;
	}
}

/* Resolve a key to a table entry, choosing off8 vs off16 by operand range. */
static int resolve(const char *key, struct opnd *vop)
{
	int idx[2], n;

	n = find_matches(key, idx);
	if (n == 0)
		return -1;
	if (n == 1)
		return idx[0];
	/* off8 / off16 pair: prefer the 8-bit form when the offset fits. */
	if (vop->haveval && fits_s8(&vop->val))
		return blip_optab[idx[0]].trail == T_OFF8 ? idx[0] : idx[1];
	return blip_optab[idx[0]].trail == T_OFF16 ? idx[0] : idx[1];
}

/*
 * Encode one BLIP instruction. "verb" is the mnemonic already read by asmline;
 * the operands remain on the input.
 */
static void blip_instr(const char *verb)
{
	struct opnd o[2];
	int nop = 0;
	char key[80];
	int c, ent, regsel = 0;
	struct opnd *vop = NULL;

	c = getnb();
	if (c != '\n' && c != ';') {
		unget(c);
		parse_operand(&o[0]);
		nop = 1;
		c = getnb();
		if (c == ',') {
			parse_operand(&o[1]);
			nop = 2;
		} else
			unget(c);
	}

	/* Build the lookup key. Two plain registers (neither USP) are the
	   generic register move "verb reg,reg" (movsel); otherwise the operand
	   text is literal (covers the USP-banking moves' specific opcodes). */
	strcpy(key, verb);
	if (nop == 2 && o[0].plainreg && o[1].plainreg
	    && o[0].regcode != USP && o[1].regcode != USP) {
		strcat(key, " reg,reg");
		if (strcmp(verb, "XCHG") == 0)
			regsel = (o[0].regcode << 4) | o[1].regcode;
		else	/* LD dst,src keeps src in the high nibble (§8.4) */
			regsel = (o[1].regcode << 4) | o[0].regcode;
	} else {
		if (nop >= 1) {
			strcat(key, " ");
			strcat(key, o[0].text);
		}
		if (nop == 2) {
			strcat(key, ",");
			strcat(key, o[1].text);
		}
	}

	if (o[0].haveval)
		vop = &o[0];
	else if (nop == 2 && o[1].haveval)
		vop = &o[1];

	ent = resolve(key, vop ? vop : &o[0]);

	/* A bare expression could be an immediate, a branch target, or a push
	   mask; retry the other normalizations the table might use. */
	if (ent < 0 && nop == 1 && o[0].bareexpr) {
		const char *alt[] = { "rel", "mask" };
		unsigned i;
		for (i = 0; i < 2 && ent < 0; i++) {
			strcpy(key, verb);
			strcat(key, " ");
			strcat(key, alt[i]);
			ent = resolve(key, &o[0]);
		}
	}

	if (ent < 0) {
		aerr(INVALID_FORM);
		return;
	}
	emit(ent, vop ? vop : &o[0], regsel);
}

void asmline(void)
{
	SYM *sp;
	int c;
	int delim;
	VALUE value;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;

loop:
	if ((c=getnb())=='\n' || c==';')
		return;
	if (is_symstart(c) == 0 && c != '.')
		qerr(UNEXPECTED_CHR);
	getid(id, c);
	if ((c=getnb()) == ':') {
		sp = lookup(id, uhash, 1);
		if (pass == 0) {
			if ((sp->s_type&TMMODE) != TNEW
			&&  (sp->s_type&TMASG) == 0)
				sp->s_type |= TMMDF;
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot[segment];
			sp->s_segment = segment;
		} else if (pass != 3) {
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot[segment];
			sp->s_segment = segment;
		} else {
			if ((sp->s_type&TMMDF) != 0)
				err('m', MULTIPLE_DEFS);
			if (sp->s_value != dot[segment])
				err('p', PHASE_ERROR);
		}
		goto loop;
	}
	/*
	 * If the first token is an id and not an operation code, assume it is
	 * the name in front of an "equ" directive.
	 */
	if ((sp=lookup(id, phash, 0)) == NULL) {
		getid(id1, c);
		if ((sp1=lookup(id1, phash, 0)) == NULL
		||  (sp1->s_type&TMMODE) != TEQU) {
			err('o', SYNTAX_ERROR);
			return;
		}
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		sp = lookup(id, uhash, 1);
		if ((sp->s_type&TMMODE) != TNEW
		&&  (sp->s_type&TMASG) == 0)
			err('m', MULTIPLE_DEFS);
		sp->s_type &= ~(TMMODE|TPUBLIC);
		sp->s_type |= TUSER|TMASG;
		sp->s_value = a1.a_value;
		sp->s_segment = a1.a_segment;
		goto loop;
	}
	unget(c);
	switch (sp->s_type&TMMODE) {
	case TORG:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_segment != ABSOLUTE)
			qerr(MUST_BE_ABSOLUTE);
		outsegment(ABSOLUTE);
		dot[segment] = a1.a_value;
		outabsolute(a1.a_value);
		break;

	case TEXPORT:
		getid(id, getnb());
		sp = lookup(id, uhash, 1);
		sp->s_type |= TPUBLIC;
		break;

	case TSEGMENT:
		segment = sp->s_value;
		outsegment(segment);
		break;

	case TDEFB:
		do {
			getaddr(&a1);
			constify(&a1);
			istuser(&a1);
			outrab(&a1);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFW:
		do {
			getaddr(&a1);
			constify(&a1);
			istuser(&a1);
			outraw(&a1);
		} while ((c=getnb()) == ',');
		unget(c);
		break;

	case TDEFM:
		if ((delim=getnb()) == '\n')
			qerr(MISSING_DELIMITER);
		while ((c=get()) != delim) {
			if (c == '\n')
				qerr(MISSING_DELIMITER);
			outab(c);
		}
		break;

	case TDEFS:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		for (value = 0 ; value < a1.a_value; value++)
			outab(0);
		break;

	case TINST:
		blip_instr(id);
		break;

	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
