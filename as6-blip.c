/*
 * BLIP assembler.
 * Basic symbol table: assembler directives and the instruction verbs.
 *
 * Unlike the 6809 table, instruction mnemonics carry no opcode here: BLIP's
 * encoding is table-driven from the opcode map (blip-optab.h, generated from
 * isa/opcodes.toml). Each verb is just tagged TINST; as1-blip.c parses the
 * operands, builds the normalized key, and looks up the opcode. Register names
 * are matched directly in as1-blip.c (so they are not symbols here).
 */
#include	"as.h"

SYM	sym[] = {
	/* Assembler directives (both bare and dotted spellings). */
	{	0,	"defw",		TDEFW,		0	},
	{	0,	"defs",		TDEFS,		0	},
	{	0,	"defm",		TDEFM,		0	},
	{	0,	"org",		TORG,		0	},
	{	0,	"equ",		TEQU,		0	},
	{	0,	"export",	TEXPORT,	0	},
	{	0,	".byte",	TDEFB,		0	},
	{	0,	".word",	TDEFW,		0	},
	{	0,	".blkb",	TDEFS,		0	},
	{	0,	".ds",		TDEFS,		0	},
	{	0,	".ascii",	TDEFM,		0	},
	{	0,	".org",		TORG,		0	},
	{	0,	".equ",		TEQU,		0	},
	{	0,	".export",	TEXPORT,	0	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"literal",	TSEGMENT,	LITERAL	},
	{	0,	"commondata",	TSEGMENT,	COMMONDATA },
	{	0,	"buffers",	TSEGMENT,	BUFFERS	},
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},

	/* Instruction verbs (isa.md §8.2 / isa/opcodes.toml). The operand form
	   selects the opcode; see as1-blip.c. Uppercase per the §4.1 house style. */
	{	0,	"ABX",		TINST,		0	},
	{	0,	"ADC",		TINST,		0	},
	{	0,	"ADD",		TINST,		0	},
	{	0,	"AND",		TINST,		0	},
	{	0,	"ANDCC",	TINST,		0	},
	{	0,	"ASL",		TINST,		0	},
	{	0,	"ASR",		TINST,		0	},
	{	0,	"BCC",		TINST,		0	},
	{	0,	"BCS",		TINST,		0	},
	{	0,	"BEQ",		TINST,		0	},
	{	0,	"BGE",		TINST,		0	},
	{	0,	"BGT",		TINST,		0	},
	{	0,	"BHI",		TINST,		0	},
	{	0,	"BIT",		TINST,		0	},
	{	0,	"BLE",		TINST,		0	},
	{	0,	"BLS",		TINST,		0	},
	{	0,	"BLT",		TINST,		0	},
	{	0,	"BMI",		TINST,		0	},
	{	0,	"BNE",		TINST,		0	},
	{	0,	"BPL",		TINST,		0	},
	{	0,	"BRA",		TINST,		0	},
	{	0,	"BRN",		TINST,		0	},
	{	0,	"BSR",		TINST,		0	},
	{	0,	"BVC",		TINST,		0	},
	{	0,	"BVS",		TINST,		0	},
	{	0,	"CLI",		TINST,		0	},
	{	0,	"CLR",		TINST,		0	},
	{	0,	"CMP",		TINST,		0	},
	{	0,	"COM",		TINST,		0	},
	{	0,	"CWAI",		TINST,		0	},
	{	0,	"DAA",		TINST,		0	},
	{	0,	"DEC",		TINST,		0	},
	{	0,	"EOR",		TINST,		0	},
	{	0,	"HALT",		TINST,		0	},
	{	0,	"INC",		TINST,		0	},
	{	0,	"JMP",		TINST,		0	},
	{	0,	"JSR",		TINST,		0	},
	{	0,	"LBCC",		TINST,		0	},
	{	0,	"LBCS",		TINST,		0	},
	{	0,	"LBEQ",		TINST,		0	},
	{	0,	"LBGE",		TINST,		0	},
	{	0,	"LBGT",		TINST,		0	},
	{	0,	"LBHI",		TINST,		0	},
	{	0,	"LBLE",		TINST,		0	},
	{	0,	"LBLS",		TINST,		0	},
	{	0,	"LBLT",		TINST,		0	},
	{	0,	"LBMI",		TINST,		0	},
	{	0,	"LBNE",		TINST,		0	},
	{	0,	"LBPL",		TINST,		0	},
	{	0,	"LBRA",		TINST,		0	},
	{	0,	"LBRN",		TINST,		0	},
	{	0,	"LBSR",		TINST,		0	},
	{	0,	"LBVC",		TINST,		0	},
	{	0,	"LBVS",		TINST,		0	},
	{	0,	"LD",		TINST,		0	},
	{	0,	"LDMMU",	TINST,		0	},
	{	0,	"LEA",		TINST,		0	},
	{	0,	"LSR",		TINST,		0	},
	{	0,	"MUL",		TINST,		0	},
	{	0,	"NEG",		TINST,		0	},
	{	0,	"NOP",		TINST,		0	},
	{	0,	"OR",		TINST,		0	},
	{	0,	"ORCC",		TINST,		0	},
	{	0,	"PSHS",		TINST,		0	},
	{	0,	"PULS",		TINST,		0	},
	{	0,	"ROL",		TINST,		0	},
	{	0,	"ROR",		TINST,		0	},
	{	0,	"RTI",		TINST,		0	},
	{	0,	"RTS",		TINST,		0	},
	{	0,	"SBC",		TINST,		0	},
	{	0,	"SCC",		TINST,		0	},	/* Scc set-on-condition (D) */
	{	0,	"SCS",		TINST,		0	},
	{	0,	"SEI",		TINST,		0	},
	{	0,	"SEQ",		TINST,		0	},
	{	0,	"SEX",		TINST,		0	},
	{	0,	"SGE",		TINST,		0	},
	{	0,	"SGT",		TINST,		0	},
	{	0,	"SHI",		TINST,		0	},
	{	0,	"SLE",		TINST,		0	},
	{	0,	"SLS",		TINST,		0	},
	{	0,	"SLT",		TINST,		0	},
	{	0,	"SMI",		TINST,		0	},
	{	0,	"SNE",		TINST,		0	},
	{	0,	"SPL",		TINST,		0	},
	{	0,	"ST",		TINST,		0	},
	{	0,	"STMMU",	TINST,		0	},
	{	0,	"SUB",		TINST,		0	},
	{	0,	"SVC",		TINST,		0	},
	{	0,	"SVS",		TINST,		0	},
	{	0,	"SWI",		TINST,		0	},
	{	0,	"SWI2",		TINST,		0	},
	{	0,	"SWI3",		TINST,		0	},
	{	0,	"SYNC",		TINST,		0	},
	{	0,	"TAS",		TINST,		0	},
	{	0,	"TST",		TINST,		0	},
	{	0,	"XCHG",		TINST,		0	}
};

/*
 * Set up the symbol table: link each entry into the predefined-symbol hash.
 */
void syminit(void)
{
	SYM *sp;
	int hash;

	sp = &sym[0];
	while (sp < &sym[sizeof(sym)/sizeof(SYM)]) {
		hash = symhash(sp->s_id);
		sp->s_fp = phash[hash];
		phash[hash] = sp;
		++sp;
	}
}

char *etext[] = {
	"unexpected character",		/* 10 */
	"phase error",			/* 11 */
	"multiple definitions",		/* 12 */
	"syntax error",			/* 13 */
	"must be absolute",		/* 14 */
	"missing delimiter",		/* 15 */
	"invalid constant",		/* 16 */
	"bra out of range",		/* 17 */
	"condition required",		/* 18 */
	"too many bra expansions",	/* 19 */
	"index register expected",	/* 20 */
	"invalid form",			/* 21 */
	"immediate only",		/* 22 */
	"divide by 0",			/* 23 */
	"constant out of range",	/* 24 */
	"data in BSS",			/* 25 */
	"segment overflow",		/* 26 */
	"data in ZP",			/* 27 */
	"instruction requires 6309",	/* 28 */
	"segment conflict",		/* 29 */
	"address required",		/* 30 */
	"invalid indirect",		/* 31 */
	"invalid ID",			/* 32 */
	"register required",		/* 33 */
	"register required",		/* 34 */
	"expected ']'"			/* 35 */
};

/*
 * Make sure the mode/register fields of the ADDR can take part in an add or
 * subtract (used by the shared expression evaluator).
 */
void isokaors(ADDR *ap, int paren)
{
	int mode;

	mode = ap->a_type&TMMODE;
	if (mode == TUSER)
		return;
	aerr(ADDR_REQUIRED);
}
