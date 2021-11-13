/*	EVAR.H:	Environment and user variable definitions
		for MicroEMACS

		written 1986 by Daniel Lawrence
*/

#ifndef _EVAR_H_
#define _EVAR_H_

/*	structure to hold user variables and their definitions	*/

typedef struct UVARTAG {
	char u_name[NVSIZE + 1];		/* name of user variable */
	char *u_value;				/* value (string) */
} UVAR;

/*	current user variables (This structure will probably change)	*/

#if	DECUSC
#define	MAXVARS		16
#else
#define	MAXVARS		255
#endif

static UVAR FAR uv[MAXVARS];	/* user variables */

/*	list of recognized environment variables	*/

static CONST char FAR *envars[] = {
	"fillcol",		/* current fill column */
	"pagelen",		/* number of lines used by editor */
	"curcol",		/* current column pos of cursor */
	"curline",		/* current line in file */
	"ram",			/* ram in use by malloc */
	"flicker",		/* flicker supression */
	"curwidth",		/* current screen width */
	"cbufname",		/* current buffer name */
	"cfname",		/* current file name */
	"sres",			/* current screen resolution */
	"debug",		/* macro debugging */
	"status",		/* returns the status of the last command */
	"palette",		/* current palette string */
	"asave",		/* # of chars between auto-saves */
	"acount",		/* # of chars until next auto-save */
	"lastkey",		/* last keyboard char struck */
	"curchar",		/* current character under the cursor */
	"discmd",		/* display commands on command line */
	"version",		/* current version number */
	"progname",		/* returns current prog name - "MicroEMACS" */
	"seed",			/* current random number seed */
	"disinp",		/* display command line input characters */
	"wline",		/* # of lines in current window */
	"cwline",		/* current screen line in window */
	"target",		/* target for line moves */
	"search",		/* search pattern */
	"replace",		/* replacement pattern */
	"match",		/* last matched magic pattern */
	"kill",			/* kill buffer (read only) */
	"cmode",		/* mode of current buffer */
	"gmode",		/* global modes */
	"tpause",		/* length to pause for paren matching */
	"pending",		/* type ahead pending flag */
	"lwidth",		/* width of current line */
	"line",			/* text of current line */
	"advance",		/* edt direction */
	"vt100key",		/* wait after escape */
	"fcol",			/* first column */
	"hardtab",		/* current hard tab size */
	"softtab",		/* current soft tab size */
	"hjump",		/* horizontal screen jump size */
	"hscroll",		/* horizontal scrolling flag */
	"cbflags",		/* current buffer flags */
	"time",			/* system time and date */
	"ndate",		/* date as numerics */
	"curwind",		/* current window ordinal on current screen */
	"disphigh",		/* display high bit characters escaped */
	"lterm",		/* current line terminator for writes */
	"numwind",		/* number of windows on current screen */
	"wchars",		/* set of characters legal in words */
	"yankpnt",		/* point placement at yanked/included text */
	"popflag",		/* pop-up windows active? */
	"cpopflag",		/* completion pop-up windows active? */
	"gfcolor",		/* global forground color */
	"gbcolor",		/* global background color */
	"cfcolor",		/* current forground color */
	"cbcolor",		/* current background color */
	"bfcolor",		/* border forground color */
	"bbcolor",		/* border background color */
	"sfcolor",		/* shadow forground color */
	"sbcolor",		/* shadow background color */
	"wintitle",		/* window title */
	"numcols",		/* number of columns on screen */
	"numrows",		/* number of rows on screen */
	"cindtype",		/* c-indent type */
	"pid",			/* process id */
	"dispseven",		/* display seven bits only */
	"dosync"                /* sync the filesystem when saving */
};

#define	NEVARS	sizeof(envars) / sizeof(char *)

/* 	and its preprocesor definitions		*/

#define	EVFILLCOL	0
#define	EVPAGELEN	1
#define	EVCURCOL	2
#define	EVCURLINE	3
#define	EVRAM		4
#define	EVFLICKER	5
#define	EVCURWIDTH	6
#define	EVCBUFNAME	7
#define	EVCFNAME	8
#define	EVSRES		9
#define	EVDEBUG		10
#define	EVSTATUS	11
#define	EVPALETTE	12
#define	EVASAVE		13
#define	EVACOUNT	14
#define	EVLASTKEY	15
#define	EVCURCHAR	16
#define	EVDISCMD	17
#define	EVVERSION	18
#define	EVPROGNAME	19
#define	EVSEED		20
#define	EVDISINP	21
#define	EVWLINE		22
#define EVCWLINE	23
#define	EVTARGET	24
#define	EVSEARCH	25
#define	EVREPLACE	26
#define	EVMATCH		27
#define	EVKILL		28
#define	EVCMODE		29
#define	EVGMODE		30
#define	EVTPAUSE	31
#define	EVPENDING	32
#define	EVLWIDTH	33
#define	EVLINE		34
#define	EVADVANCE	35
#define	EVVT100KEYS	36
#define	EVFCOL		37
#define	EVHARDTAB	38
#define	EVSOFTTAB	39
#define	EVHJUMP		40
#define	EVHSCROLL	41
#define	EVCBFLAGS	42
#define	EVTIME		43
#define	EVNDATE		44
#define	EVCURWIND	45
#define	EVDISPHIGH	46
#define	EVLTERM		47
#define	EVNUMWIND	48
#define	EVWCHARS	49
#define	EVYANKPNT	50
#define	EVPOPFLAG	51
#define	EVCPOPFLAG	52
#define	EVGFCOLOR	53
#define	EVGBCOLOR	54
#define	EVCFCOLOR	55
#define	EVCBCOLOR	56
#define	EVBFCOLOR	57
#define	EVBBCOLOR	58
#define	EVSFCOLOR	59
#define	EVSBCOLOR	60
#define	EVWINTITLE	61
#define	EVNUMCOLS	62
#define	EVNUMROWS	63
#define	EVCINDTYPE	64
#define	EVPID		65
#define	EVDISPSEVEN	66
#define EVDOSYNC        67

/*	list of recognized user functions	*/

typedef struct UFUNCTAG {
	CONSTA char *f_name;	/* name of function */
	int f_type;		/* 1 = monamic, 2 = dynamic */
} UFUNC;

#define	NILNAMIC	0
#define	MONAMIC		1
#define	DYNAMIC		2
#define	TRINAMIC	3

static UFUNC FAR funcs[] = {
	{ "add", DYNAMIC },	/* add two numbers together */
	{ "sub", DYNAMIC },	/* subtraction */
	{ "tim", DYNAMIC },	/* multiplication */
	{ "div", DYNAMIC },	/* division */
	{ "mod", DYNAMIC },	/* mod */
	{ "neg", MONAMIC },	/* negate */
	{ "cat", DYNAMIC },	/* concatenate string */
	{ "lef", DYNAMIC },	/* left string(string, len) */
	{ "rig", DYNAMIC },	/* right string(string, pos) */
	{ "mid", TRINAMIC },	/* mid string(string, pos, len) */
	{ "not", MONAMIC },	/* logical not */
	{ "equ", DYNAMIC },	/* logical equality check */
	{ "les", DYNAMIC },	/* logical less than */
	{ "gre", DYNAMIC },	/* logical greater than */
	{ "seq", DYNAMIC },	/* string logical equality check */
	{ "sle", DYNAMIC },	/* string logical less than */
	{ "sgr", DYNAMIC },	/* string logical greater than */
	{ "ind", MONAMIC },	/* evaluate indirect value */
	{ "and", DYNAMIC },	/* logical and */
	{ "or",  DYNAMIC },	/* logical or */
	{ "len", MONAMIC },	/* string length */
	{ "upp", MONAMIC },	/* uppercase string */
	{ "low", MONAMIC },	/* lower case string */
	{ "tru", MONAMIC },	/* Truth of the universe logical test */
	{ "asc", MONAMIC },	/* char to integer conversion */
	{ "chr", MONAMIC },	/* integer to char conversion */
	{ "gtk", NILNAMIC },	/* get 1 charater */
	{ "rnd", MONAMIC },	/* get a random number */
	{ "abs", MONAMIC },	/* absolute value of a number */
	{ "sin", DYNAMIC },	/* find the index of one string in another */
	{ "env", MONAMIC },	/* retrieve a system environment var */
	{ "bin", MONAMIC },	/* loopup what function name is bound to a key */
	{ "gtc", NILNAMIC },	/* get 1 emacs command */
	{ "ban", DYNAMIC },	/* bitwise and */
	{ "bor", DYNAMIC },	/* bitwise or */
	{ "bxo", DYNAMIC },	/* bitwise xor */
	{ "bno", MONAMIC },	/* bitwise not */
	{ "exi", MONAMIC }, 	/* check if a file exists */
	{ "isn", MONAMIC }	/* is the arg a number? */
};

#define	NFUNCS	sizeof(funcs) / sizeof(UFUNC)

/* 	and its preprocesor definitions		*/

#define	UFADD		0
#define	UFSUB		1
#define	UFTIMES		2
#define	UFDIV		3
#define	UFMOD		4
#define	UFNEG		5
#define	UFCAT		6
#define	UFLEFT		7
#define	UFRIGHT		8
#define	UFMID		9
#define	UFNOT		10
#define	UFEQUAL		11
#define	UFLESS		12
#define	UFGREATER	13
#define	UFSEQUAL	14
#define	UFSLESS		15
#define	UFSGREAT	16
#define	UFIND		17
#define	UFAND		18
#define	UFOR		19
#define	UFLENGTH	20
#define	UFUPPER		21
#define	UFLOWER		22
#define	UFTRUTH		23
#define	UFASCII		24
#define	UFCHR		25
#define	UFGTKEY		26
#define	UFRND		27
#define	UFABS		28
#define	UFSINDEX	29
#define	UFENV		30
#define	UFBIND		31
#define UFGTCMD		32
#define UFBAND		33
#define UFBOR		34
#define UFBXOR		35
#define UFBNOT		36
#define UFEXIST		37
#define	UFISNUM		38

#endif
