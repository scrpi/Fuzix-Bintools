/*
 * TI980 assembler.
 *
 * TODO:
 *	- relocations
 *	- literals
 */
#include	"as.h"

static int asmflags = 0;
static unsigned basepage = 0;

/*
 * Word addressed machine. When we create a label we use dot/2 for the
 * value as the internal thinking is in bytes here and in the linker
 *
 * This should mean we can boot the word size hacks out of the linker
 */

#define BYTES_PER_WORD	2

/*
 *	Set up for the start of each pass
 */
int passbegin(int pass)
{
	asmflags = 0;
	basepage = 0;
	segment = 1;		/* Default to code */
	/*  we do not do pass 1 and 2 . May need to revisit for branches */
	if (pass == 1 || pass == 2)
		return 0;
	return 1;
}

void doflush(void)
{
}

static void constify(ADDR *ap)
{
	if ((ap->a_type & TMMODE) == (TUSER|TMINDIR))
		ap->a_type = TUSER;
}

/*
 * Read in an address
 * descriptor, and fill in
 * the supplied "ADDR" structure
 * with the mode and value.
 * Exits directly to "qerr" if
 * there is no address field or
 * if the syntax is bad.
 */
void getaddr(ADDR *ap)
{
	int c;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;
	ap->a_segment = ABSOLUTE;
	ap->a_value = 0;

	c = getnb();

	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);
	expr1(ap, LOPRI, 0);
	constify(ap);
}

void checkrange(ADDR *ap, unsigned l, unsigned h)
{
	if (ap->a_segment != ABSOLUTE)
		aerr(CONSTANT_RANGE);
	if (ap->a_value < l || ap->a_value > h)
		aerr(CONSTANT_RANGE);
	/* TODO deal with symbol cases and check segment ABSOLUTE etc */
}

unsigned signed8(ADDR *ap)
{
	int v = ap->a_value;
	if (ap->a_segment != ABSOLUTE)
		return 0;
	if (v < -128 || v > 127)
		return 0;
	return 1;
}

void getconst(ADDR *ap, unsigned low, unsigned high)
{
	getaddr(ap);
	istuser(ap);
	checkrange(ap, low, high);
}

void getreg(ADDR *ap)
{
	getaddr(ap);
	if ((ap->a_type & TMMODE) != TWR || ap->a_value)
		aerr(REG_REQUIRED);
}

unsigned reg(ADDR *ap)
{
	return ap->a_type & TMREG;
}

void not_status(ADDR *ap)
{
	if (reg(ap) == 8)
		aerr(REG_REQUIRED);	/* FIXME not status error needed */
}

/*
 *	An EA can have the following forms
 *	=n		constant (8 or 16bit)
 *	offset		signed offset versus PC
 *	offset,mode	signed or unsigned offset versus PC, B or X
 *	*offset		indirect signed offset versus PC
 *	*offset,mode	indirect signed or unsigned offset versus PC, B or X
 *
 *	8bit offsets are packed into the instruction. 16bit forms are encoded
 *	specially and use the word (or words) after.
 */
unsigned getea(ADDR *ap)
{
	unsigned r = 0;
	unsigned indir = 0;
	ADDR a2;
	int c;

	ap->a_type = 0;
	ap->a_flags = 0;
	ap->a_sym = NULL;
	ap->a_segment = ABSOLUTE;

	c = getnb();

	if (c == '=')
		r = 7;
	else if (c == '*')
		indir = 1;
	else
		unget(c);

	c = getnb();
	if (c == '<')
		ap->a_flags |= A_LOW;
	else if (c == '>')
		ap->a_flags |= A_HIGH;
	else
		unget(c);
	expr1(ap, LOPRI, 0);

	c = getnb();
	if (c == ',')  {
		getconst(&a2, 0, 7);
		r = a2.a_value;
	} else
		unget(c);
	if (indir)
		r |= 4;
	return r;
}

static void outaw(unsigned v)
{
	outab((v >> 8) & 0xFF);
	outab(v & 0xFF);
}


static void encode_ea(unsigned opcode, int l, int ext)
{
	ADDR a1;
	unsigned t = getea(&a1);
	opcode |= t << 8;
	switch(t) {
	case 0:	/* PC + s */
	case 2:	/* PC + X + s , might be pc rel */
	case 4:	/* 0 with indirect */
	case 6:	/* 2 with indirect */
		/* Must be reachable from dot[segment] signed */
#if 0
TODO
		checkpcrel(&a1);
#endif
		/* TODO - calc pcrel form if ok */
		outaw(opcode | (a1.a_value & 0xFF));
		break;
	case 1:	/* B + u, assumed unsigned offset */
	case 3:	/* B + X + d */
	case 5:	/* B + u with indirect */
		checkrange(&a1, 0, 255);
		outaw(opcode | (a1.a_value & 0xFF));
		break;
	case 7:	/* Immediate */
		if (signed8(&a1)) {
			outaw(opcode | (a1.a_value & 0xFF));
			return;
		}
		/* FIXME: ext check seems wrong
		if (!ext)
			aerr(CONSTANT_RANGE); */
		outaw(opcode);	/* byte bits not used */
		if (ext == 0)
			outraw(&a1);
		else {
			/* TODO: should be a 32bit reloc */
			outaw(a1.a_value);
			outaw(a1.a_value >> 16);
		}
		break;
	}
}

#if 0

static void memref(ADDR *ap, unsigned type)
{
	/* TODO: check by type */
	unsigned indirect = 0;
	unsigned literal = 0;
	unsigned indexed = 0;
	unsigned dp = 0;

	int c = getnb();
	if (c == '*')
		indirect = 1;
	else if (c == '#')
		literal = 1;
	else
		unget(c);

	c = getnb();
	if (indirect && c == '*') {
		/* ** special form */
		ap->a_value = 0;
		c = getnb();
		/* * ** or ** */
		if (c != '*') {
			indirect = 0;
			unget(c);
		}
		goto encode;
	} else if (literal == 0 && c == '@')
		/* Force segment 0 */
		dp = 1;
	else
		unget(c);

	/* Now we should have an address */
	getaddr(ap);
	istuser(ap);
	if (!literal) {
		/* And it may be followed by ,1 for indexed */
		c = getnb();
		if (c == ',') {
			c = getnb();
			if (c == '1')
				indexed = 1;
			else
				aerr(SYNTAX_ERROR);
		} else
			unget(c);
	}
encode:
	if (indirect)
		ap->a_type |= TINDIRECT;
	if (literal)
		ap->a_type |= TIMMED;
	if (indexed)
		ap->a_type |= TINDEX;
	if (dp)
		ap->a_segment = ZP;
}

/*
 *	opbase
 *	1	- memory ref (wil be turned to 3 in extended)
 *	2	- stx/ldx
 */
static void write_memref(unsigned opcode, ADDR *ap, unsigned opbase)
{
	if (ap->a_type & TINDIRECT)
		opcode |= 0x8000;
	if (ap->a_type & TINDEX)
		opcode |= 0x4000;
	if (ap->a_segment != ZP)
		opcode |= 0x0200;
	/*if (ap->a_type & TIMMED)
			need to allocate a word etc */
	ap->a_value |= opcode;
	outraw(ap);
#if 0
	if (opbase == 1 && (asmflags & FEAT_EXT))
		opbase = 3;
	/* ? What do we do with abs values > range ? */
	/* Literal - IMMED never mixes with others */
	if (ap->a_type & TIMMED)
		opbase = 0;
	/* Can write directly */
	if (ap->a_segment == ABSOLUTE && ap->a_sym == NULL) {
		ap->a_value |= opcode;
		outab((ap->a_value >> 8) & 0xFF);
		outab(ap->a_value & 0xFF);
		return;
	}
	if (ap->a_sym) {
		outbyte(opbase + 4);
		outbyte(a->sym->s_number & 0xFF);
		outbyte(a->sym->s_number >> 8);
	} else {
		outbyte(opbase);
		outbyte(ap->a_segment);
	}
	outbyte((ap->a_value >> 8) & 0xFF);
	outbyte(ap->a_value & 0xFF);
	outab(opcode >> 8)
	outab(opcode);
#endif
}

#endif

/*
 * Assemble one line.
 * The line in in "ib", the "ip"
 * scans along it. The code is written
 * right out.
 */
void asmline(void)
{
	SYM *sp;
	int c;
	int opcode;
	VALUE value;
	int delim;
	SYM *sp1;
	char id[NCPS];
	char id1[NCPS];
	ADDR a1;
	ADDR a2;
	unsigned ext;

loop:
	ext = 0;
	if ((c = getnb()) == '\n' || c == ';')
		return;
	if (c == '@') {
		c = getnb();
		ext = 1;
	}
	if (is_symstart(c) == 0 && c != '.')
		qerr(UNEXPECTED_CHR);
	getid(id, c);
	if ((c = getnb()) == ':' && !ext) {
		sp = lookup(id, uhash, 1);
		if (pass == 0) {
			if ((sp->s_type&TMMODE) != TNEW
			&&  (sp->s_type&TMASG) == 0)
				sp->s_type |= TMMDF;
			sp->s_type &= ~TMMODE;
			sp->s_type |= TUSER;
			sp->s_value = dot[segment] / BYTES_PER_WORD;
			sp->s_segment = segment;
		} else {
			if ((sp->s_type&TMMDF) != 0)
				err('m', MULTIPLE_DEFS);
			if (sp->s_value != dot[segment] / BYTES_PER_WORD)
				err('p', PHASE_ERROR);
		}
		goto loop;
	}
	/*
	 * If the first token is an
	 * id and not an operation code,
	 * assume that it is the name in front
	 * of an "equ" assembler directive.
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
		/* FIXME: review .equ to an external symbol/offset and
		   what should happen */
		goto loop;
	}
	unget(c);
	opcode = sp->s_value;
	switch (sp->s_type&TMMODE) {
	case TORG:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		if (a1.a_segment != ABSOLUTE)
			qerr(MUST_BE_ABSOLUTE);
		outsegment(ABSOLUTE);
		dot[segment] = a1.a_value;
		/* Tell the binary generator we've got a new absolute
		   segment. */
		outabsolute(a1.a_value);
		break;

	case TEXPORT:
		getid(id, getnb());
		sp = lookup(id, uhash, 1);
		sp->s_type |= TPUBLIC;
		break;
		/* .code etc */
	case TSEGMENT:
		segment = sp->s_value;
		/* Tell the binary generator about a segment switch to a non
		   absolute segnent */
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
		/* Force alignment as really are a word machine */
		if (dot[segment] & 1)
			outab(0);
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
		/* Force alignment as really are a word machine */
		if (dot[segment] & 1)
			outab(0);
		break;

	case TDEFS:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		/* Write out the bytes. The BSS will deal with the rest */
		for (value = 0 ; value < a1.a_value; value++) {
			/* We are a word machine so space is in words */
			outab(0);
			outab(0);
		}
		break;
	case TEA:
		encode_ea(opcode, 1, ext);
		break;
	case TDEA:
		encode_ea(opcode, 2, ext);
		break;
	case TRR:
		getreg(&a1);
		comma();
		getreg(&a2);
		not_status(&a1);
		outaw(opcode | (reg(&a1) << 4) | reg(&a2));
		break;
	case TR:
		getreg(&a1);
		outaw(opcode | reg(&a1));
		break;
	case TSHIFT:
		getconst(&a1, 0, 31);
		outaw(opcode | a1.a_value);
		break;
	case TIMPL:
		outaw(opcode);
		break;
	case TMEM:
		getaddr(&a1);
		constify(&a1);
		istuser(&a1);
		outraw(&a1);
		break;
	case TBIT:
	case TSENSE:
	case TIDLE:
		getconst(&a1, 0, 15);
		outaw(opcode | a1.a_value);
		break;
	case TBITM:
		getconst(&a1, 0, 15);
		outaw(opcode | a1.a_value);
		comma();
		getaddr(&a2);
		constify(&a1);
		istuser(&a1);
		outraw(&a2);
		break;
	case TAPI:
		getconst(&a1, 0, 31);
		outaw(opcode | a1.a_value);
		comma();
		getaddr(&a2);
		constify(&a1);
		istuser(&a1);
		outraw(&a2);
		break;
	case TATI:
		getconst(&a1, 0, 31);
		comma();
		getconst(&a2, 0, 7);
		outaw(opcode | (a1.a_value << 5) | a2.a_value);
		comma();
		getaddr(&a2);
		constify(&a1);
		istuser(&a1);
		outraw(&a2);
		break;
	default:
		aerr(SYNTAX_ERROR);
	}
	goto loop;
}
