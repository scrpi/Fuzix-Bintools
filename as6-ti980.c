/*
 * TI980 assembler.
 *
 * No 10bit support at this point.
 */

#include	"as.h"

SYM	sym[] = {
	{	0,	"a",		TWR,		0	},
	{	0,	"e",		TWR,		1	},
	{	0,	"x",		TWR,		2	},
	{	0,	"m",		TWR,		3	},
	{	0,	"s",		TWR,		4	},
	{	0,	"l",		TWR,		5	},
	{	0,	"b",		TWR,		6	},
	{	0,	"p",		TWR,		7	},
	{	0,	"st",		TWR,		8	},

	{	0,	"defb",		TDEFB,		XXXX	},
	{	0,	"defw",		TDEFW,		XXXX	},
	{	0,	"defs",		TDEFS,		XXXX	},
	{	0,	"defm",		TDEFM,		XXXX	},
	{	0,	"org",		TORG,		XXXX	},
	{	0,	"equ",		TEQU,		XXXX	},
	{	0,	"export",	TEXPORT,	XXXX	},
	{	0,	".byte",	TDEFB,		XXXX	},
	{	0,	".word",	TDEFW,		XXXX	},
	{	0,	".blkb",	TDEFS,		XXXX	},
	{	0,	".ds",		TDEFS,		XXXX	},
	{	0,	".ascii",	TDEFM,		XXXX	},
	{	0,	".org",		TORG,		XXXX	},
	{	0,	".equ",		TEQU,		XXXX	},
	{	0,	".export",	TEXPORT,	XXXX	},
	{	0,	"abs",		TSEGMENT,	ABSOLUTE},
	{	0,	"code",		TSEGMENT,	CODE	},
	{	0,	"data",		TSEGMENT,	DATA	},
	{	0,	"bss",		TSEGMENT,	BSS	},
	{	0,	"discard",	TSEGMENT,	DISCARD	},
	{	0,	"common",	TSEGMENT,	COMMON	},
	{	0,	"zp",		TSEGMENT,	ZP	},
	{	0,	".abs",		TSEGMENT,	ABSOLUTE},
	{	0,	".code",	TSEGMENT,	CODE	},
	{	0,	".data",	TSEGMENT,	DATA	},
	{	0,	".bss",		TSEGMENT,	BSS	},
	{	0,	".discard",	TSEGMENT,	DISCARD	},
	{	0,	".common",	TSEGMENT,	COMMON	},
	{	0,	".zp",		TSEGMENT,	ZP	},
	{	0,	".literal",	TSEGMENT,	LITERAL	},
	{	0,	".commondata",	TSEGMENT,	COMMONDATA },
	{	0,	".buffers",	TSEGMENT,	BUFFERS	},

	/* 0x0000-0x9FFF: opcodes with an effective address */
	{	0,	"lda",		TEA,		0x0000	},
	{	0,	"lde",		TEA,		0x0800	},
	{	0,	"ldx",		TEA,		0x1000	},
	{	0,	"ldm",		TEA,		0x1800	},
	{	0,	"add",		TEA,		0x2000	},
	{	0,	"sub",		TEA,		0x2800	},
	{	0,	"ior",		TEA,		0x3000	},
	{	0,	"and",		TEA,		0x3800	},
	{	0,	"bix",		TEA,		0x4000	},
	{	0,	"dmt",		TEA,		0x4800	},
	{	0,	"imo",		TEA,		0x5000	},
	{	0,	"div",		TEA,		0x5800	},
	{	0,	"cpl",		TEA,		0x6000	},
	{	0,	"cpa",		TEA,		0x6800	},
	{	0,	"brl",		TEA,		0x7000	},
	{	0,	"bru",		TEA,		0x7800	},
	{	0,	"sta",		TEA,		0x8000	},
	{	0,	"ste",		TEA,		0x8800	},
	{	0,	"stx",		TEA,		0x9000	},
	{	0,	"mpy",		TEA,		0x9800	},
	/* 0xA000-0xBFFF: like 0000-9FFF but extended
	   immediates are 32bit */
	{	0,	"dst",		TDEA,		0xA000	},
	{	0,	"dsb",		TDEA,		0xA800	},
	{	0,	"dld",		TDEA,		0xB000	},
	{	0,	"dad",		TDEA,		0xB800	},
	/* 0xC000-0xC7FF: reg/reg ops */
	{	0,	"rsu",		TRR,		0xC000	},
	{	0,	"rad",		TRR,		0xC080	},
	{	0,	"rco",		TRR,		0xC100	},
	{	0,	"riv",		TRR,		0xC200	},
	{	0,	"reo",		TRR,		0xC280	},
	{	0,	"rin",		TRR,		0xC300	},
	{	0,	"rca",		TRR,		0xC400	},
	{	0,	"ror",		TRR,		0xC480	},
	{	0,	"rmo",		TRR,		0xC500	},
	{	0,	"rcl",		TRR,		0xC600	},
	{	0,	"ran",		TRR,		0xC680	},
	{	0,	"rde",		TRR,		0xC700	},
	{	0,	"rex",		TRR,		0xC780	},
	/* 0xC800-0xCBFF: Shifts */
	{	0,	"ara",		TSHIFT,		0xC800	},
	{	0,	"ard",		TSHIFT,		0xC820	},
	{	0,	"lra",		TSHIFT,		0xC840	},
	{	0,	"lrd",		TSHIFT,		0xC860	},
	{	0,	"ala",		TSHIFT,		0xC880	},
	{	0,	"ald",		TSHIFT,		0xC8A0	},
	{	0,	"lla",		TSHIFT,		0xC8C0	},
	{	0,	"lld",		TSHIFT,		0xC8E0	},
	{	0,	"rto",		TSHIFT,		0xC900	},
	{	0,	"rtz",		TSHIFT,		0xC940	},
	{	0,	"lto",		TSHIFT,		0xC980	},
	{	0,	"ltz",		TSHIFT,		0xC9A0	},
	{	0,	"cra",		TSHIFT,		0xCA00	},
	{	0,	"cre",		TSHIFT,		0xCA20	},
	{	0,	"crx",		TSHIFT,		0xCA40	},
	{	0,	"crm",		TSHIFT,		0xCA60	},
	{	0,	"nrm",		TIMPL,		0xCA9F	}, /* Oddity */
	{	0,	"crs",		TSHIFT,		0xCB20	},
	{	0,	"crl",		TSHIFT,		0xCB40	},
	{	0,	"crb",		TSHIFT,		0xCB60	},
	{	0,	"cld",		TSHIFT,		0xCB80	},
	{	0,	"crd",		TSHIFT,		0xCBC0	},

	/* 0xCC00-CFFFF: Skips and some oddments */
	{	0,	"sze",		TR,		0xCC00	},
	{	0,	"sse",		TSENSE,		0xCC10	},
	{	0,	"soo",		TR,		0xCC20	},
	{	0,	"sod",		TR,		0xCC40	},
	{	0,	"smi",		TR,		0xCC60	},
	{	0,	"snz",		TR,		0xCC80	},
	{	0,	"ssn",		TSENSE,		0xCC90	},
	{	0,	"sno",		TR,		0xCCA0	},
	{	0,	"sev",		TR,		0xCCC0	},
	{	0,	"spl",		TR,		0xCCE0	},
	{	0,	"slt",		TIMPL,		0xCD00	},
	{	0,	"seq",		TIMPL,		0xCD20	},
	{	0,	"sgt",		TIMPL,		0xCD40	},
	{	0,	"sov",		TIMPL,		0xCD60	},
	{	0,	"sge",		TIMPL,		0xCD80	},
	{	0,	"sne",		TIMPL,		0xCDA0	},
	{	0,	"sle",		TIMPL,		0xCDC0	},
	{	0,	"snv",		TIMPL,		0xCDE0	},
	{	0,	"idl",		TIDLE,		0xCE00	},
	{	0,	"soc",		TIMPL,		0xCF60	},
	{	0,	"snc",		TIMPL,		0xCFE0	},

	/* 0xD000-0xDAFF: Device stuff and weirdness */
	{	0,	"api",		TAPI,		0xD000	},
	{	0,	"rds",		TDEV,		0xD800	},
	{	0,	"wds",		TDEV,		0xD820	},
	{	0,	"lsb",		TMEM,		0xD880	},
	{	0,	"lsr",		TMEM,		0xD890	},
	{	0,	"lrf",		TMEM,		0xD8A0	},
	{	0,	"ssb",		TMEM,		0xD8C0	},
	{	0,	"srf",		TMEM,		0xD8E0	},
	{	0,	"ati",		TATI,		0xD900	},

	/* 0xDB00-0xDEFF: Bit operations */
	{	0,	"tabz",		TBIT,		0xDB00	},
	{	0,	"tabo",		TBIT,		0xDB10	},
	{	0,	"tmbz",		TBITM,		0xDB20	},
	{	0,	"tmbo",		TBITM,		0xDB30	},
	{	0,	"sabz",		TBIT,		0xDB40	},
	{	0,	"sabo",		TBIT,		0xDB50	},
	{	0,	"smbz",		TBITM,		0xDB60	},
	{	0,	"smbo",		TBITM,		0xDB70	},

	/* DF00-DFFF: String ops */
	{	0,	"mvc",		TIMPL,		0xDF00	},
	{	0,	"clc",		TIMPL,		0xDF80	},
};


/*
 * Set up the symbol table.
 * Sweep through the initializations
 * of the "phash", and link them into the
 * buckets. Because it is here, a
 * "sizeof" works.
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
	"Bcc out of range",		/* 17 */
	"register required",		/* 18 */
	"address required",		/* 19 */
	"invalid ID",			/* 20 */
	"invalid register",		/* 21 */
	"divide by 0",			/* 22 */
	"constant out of range",	/* 23 */
	"data in BSS",			/* 24 */
	"segment overflow",		/* 25 */
	"data in direct page",		/* 26 */
	"segment conflict",		/* 27 */
	"unsupported in 10bit mode",	/* 28 */
	"too many Jcc instructions",	/* 29 */
	"invalid instruction form"	/* 30 */
};

/*
 * Make sure that the
 * mode and register fields of
 * the type of the "ADDR" pointed to
 * by "ap" can participate in an addition
 * or a subtraction.
 */
void isokaors(ADDR *ap, int paren)
{
	int mode;

	mode = ap->a_type&TMMODE;
	if (mode == TUSER)
		return;
	aerr(ADDR_REQUIRED);
}
