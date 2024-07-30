/*	ESTRUCT:	Structure and preprocesser defined for
			MicroEMACS 3.9

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			substantially modified by Daniel Lawrence
*/

#ifndef _ESTRUCT_H_
#define	_ESTRUCT_H_

#ifdef	LATTICE
#undef	LATTICE		/* don't use their definitions...use ours	*/
#endif
#ifdef	MSDOS
#undef	MSDOS
#endif
#ifdef	CPM
#undef	CPM
#endif
#ifdef	AMIGA
#undef	AMIGA
#endif
#ifdef	EGA
#undef	EGA
#endif

/*	Program Identification.....

	PROGNAME should always be MicroEMACS for a distibrution
	unmodified version. People using MicroEMACS as a shell
	for other products should change this to reflect their
	product. Macros can query this via the $progname variable
*/

#define	PROGNAME	"MicroEMACS"
#define	VERSION		"3.9"

/*	Machine/OS definitions			*/

#define AMIGA   0                       /* AmigaDOS			*/
#define ST520   0                       /* ST520, TOS                   */
#define MSDOS   0                       /* MS-DOS                       */
#define V7      0                       /* V7 UNIX or Coherent or BSD4.2*/
#ifdef BSD
#undef BSD
#endif
#define	BSD	0			/* UNIX BSD 4.2	and ULTRIX	*/
#define	BSD29	0			/* BSD2.9 - also needs BSD set	*/
#define	USG	0			/* UNIX system V		*/
#ifdef VMS
#undef VMS
#endif
#define VMS     0                       /* VAX/VMS                      */
#define CPM     0                       /* CP/M-86                      */
#define	FINDER	0			/* Macintosh OS			*/
#define	WMCS	0			/* Wicat's MCS			*/
#define	DECUSC	0			/* RSX11M with DECUS C compiler */

/*	Compiler definitions			*/
#define	UNIX	0	/* a random UNIX compiler */
#define MWC86   0	/* marc williams compiler */
#define	LATTICE	0	/* Lattice 2.14 thruough 3.0 compilers */
#define	AZTEC	0	/* Aztec C 3.20e */
#define	MSC	0	/* MicroSoft C compile version 3, 4 & 5 + Xenix */
#define	TURBO	0	/* Turbo C/MSDOS */
#define	ZORTECH	0	/* Zortech C/MSDOS */
#define	DTL	0	/* DataLight C v3.12 */
#define MEGAMAX	0	/* Megamax C compiler */
#define	C86	0	/* CI compiler */
#define	GNUC	0	/* GNU CC compiler */

/*	Auto-configure OS and Compiler */

#ifdef __TURBOC__	/* Turbo C or Borland C++/MSDOS */
#undef MSDOS
#define	MSDOS 1
#undef TURBO
#define TURBO 1
#endif

#if defined(_WIN32) && !(defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__INTERIX))	/* WinNT or Win95 */
#undef MSDOS
#define	MSDOS 1
#endif

#ifdef M_I86		/* Microsoft C/MSDOS */
#ifndef M_XENIX			/* Xenix sets this too */
#ifndef __ZTC__
#ifndef __TURBOC__		/* just to be safe */
#undef MSDOS
#define	MSDOS 1
#undef MSC
#define MSC 1
#endif
#endif
#endif
#endif

#if defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__INTERIX) 	/* CygWin b20 */
#undef USG
#define USG 1
#endif

#ifdef GNUDOS		/* GNU C/MSDOS */
#undef MSDOS
#define	MSDOS 1
#undef GNUC
#define	GNUC 1
#endif

#ifdef __ZTC__	/* Zortech C */
#undef MSDOS
#define	MSDOS 1
#undef ZORTECH
#define ZORTECH 1
#endif

#ifdef vms		/* VAX/VMS */
#undef VMS
#define	VMS 1
#endif

#if defined(M_XENIX) || defined(_AIX) || defined(m88k) || defined(sun) || defined(__linux__) || defined(__USLC__) || defined(_SYSTYPE_SVR4) || SYSTEM_SYSV || defined(__CYGWIN32__) || defined(__hpux) || defined(__INTERIX) || defined(__MINGW32__)
#undef USG
#define	USG 1
#if !(SYSTEM_AUX || defined(__linux__) || defined(__CYGWIN32__) || defined(__INTERIX) || defined(__MINGW32__))
#undef MSC
#define	MSC 1
#endif
#endif

#ifdef __FreeBSD__
#define FREEBSD2 1
#else
#define FREEBSD2 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define DARWIN 1
#ifdef USG
#undef USG
#define USG 0
#endif
#else
#define DARWIN 0
#endif

#if	(defined(sun) || defined(mips) || FREEBSD2 || DARWIN || SYSTEM_BSD) && !USG
#undef BSD
#define BSD 1
#endif

/*	Debugging options	*/
#define	RAMSIZE	0	/* dynamic RAM memory usage tracking */
#define	RAMSHOW	0	/* auto dynamic RAM reporting */

/*   Special keyboard definitions            */

#define WANGPC	0		/* WangPC - mostly escape sequences     */

/*	Terminal Output definitions		*/

#define ANSI    0			/* ANSI escape sequences	*/
#define	HP150	0			/* HP150 screen driver		*/
#define	HP110	0			/* HP110 screen driver		*/
#define	VMSVT	0			/* various VMS terminal entries	*/
#define EM_VT52 0                       /* VT52 terminal (Zenith).      */
#define RAINBOW 0                       /* Use Rainbow fast video.      */
#define TERMCAP 0                       /* Use TERMCAP                  */
#define	IBMPC	0 			/* IBM-PC CGA/MONO/EGA driver	*/
#define	DG10	0			/* Data General system/10	*/
#define	TIPC	0			/* TI Profesional PC driver	*/
#define	Z309	0			/* Zenith 100 PC family	driver	*/
#define	MAC	0			/* Macintosh			*/
#define	ATARI	0			/* Atari 520/1040ST screen	*/
#define	WINMOD	0			/* Use winmod display library	*/

/*	Auto-configure terminal */

#if !(ANSI | VMSVT | EM_VT52 | TERMCAP | IBMPC | WINMOD)
#   ifdef USEWINMOD
#       undef WINMOD
#       define	WINMOD 1
#   else
#       if MSDOS
#           if (ZORTECH && defined(DOS16RM)) || (defined(_MSC_VER) && defined(_WIN32))
#               undef ANSI
#               define	ANSI 1
#           else
#               undef IBMPC
#               define	IBMPC 1
#           endif
#       endif
#       if VMS
#           undef VMSVT
#           define	VMSVT 1
#       endif
#       if USG | BSD
#	    if defined(__MINGW32__) || defined(__ANDROID__)
#		undef ANSI
#		define ANSI 1
#	    else
#		undef TERMCAP
#		define	TERMCAP 1
#	    endif
#       endif
#   endif
# endif

#if (TERMCAP | VMSVT | WINMOD)		/* true if ttscroll exists */
#define	SCROLL	1
#else
#define	SCROLL	0
#endif

/*	Configuration options	*/

#define CVMVAS  1	/* arguments to page forward/back in pages	*/
#define	CLRMSG	0	/* space clears the message line with no insert	*/
#define	CFENCE	1	/* fench matching in CMODE			*/
#define	ACMODE	1	/* auto C mode on .C and .H files		*/
#define	TYPEAH	1	/* type ahead causes update to be skipped	*/
#define DEBUGM	1	/* $debug triggers macro debugging		*/
#define	LOGFLG	0	/* send all executed commands to EMACS.LOG	*/
#define	VISMAC	0	/* update display during keyboard macros	*/
#define	CTRLZ	0	/* add a ^Z at end of files under MSDOS only	*/

#define	REVSTA	1	/* Status line appears in reverse video		*/
#define	COLOR	1	/* color commands and windows			*/

#define	FILOCK	0	/* file locking under unix BSD 4.2		*/
#define	ISRCH	1	/* Incremental searches like ITS EMACS		*/
#define	WORDPRO	1	/* Advanced word processing features		*/
#define	FLABEL	0	/* function key label code [HP150]		*/
#define	APROP	1	/* Add code for Apropos command			*/
#define	CRYPT	1	/* file encryption enabled?			*/
#define MAGIC	1	/* include regular expression matching?		*/
#define	AEDIT	1	/* advanced editing options: en/detabbing	*/
#define	PROC	1	/* named procedures				*/
#define	ABACKUP	1	/* auto-saved files written to .ASV		*/
#define	MENUS	VMS	/* pull down menus				*/
#define	DECEDT	1	/* simulate DEC EDT keypad editor		*/
#define	CALLABLE (VMS|WINMOD)	/* make a callable interface		*/
#define	BCOMPL	1	/* buffer name completion			*/
#define	DISPSEVEN (TERMCAP&DECEDT) /* display seven bits when dispseven set */

#define ASCII	1	/* always using ASCII char sequences for now	*/
#define EBCDIC	0	/* later IBM mainfraim versions will use EBCDIC	*/

/*	System dependant library redefinitions, structures and includes	*/

#define	me_itoa	int_asc
/* #define	atoi	asc_int */

#define	PASCAL
#define	NEAR
#define	DNEAR

#if	VMS && defined(vax11c)
#define	CONST	readonly
#define	CONSTA	const
#define	VOID	void
#define NOSHARE noshare
#else
#if	defined(__STDC__) || defined(__GNUC__) || defined(_AIX) || defined(__TURBOC__) || defined(__ZTC__) || defined(_WIN32)
#define	CONST	const
#define	CONSTA	const
#define	VOID	void
#define	NOSHARE
#ifndef UNUSED_ARG
#define UNUSED_ARG(a) ((void)a)
#endif
#ifndef PP
#ifndef M_LINT
#define PP(x)	x		/* function prototype */
#endif
#endif
#else
#define	CONST
#define CONSTA
#define	VOID	char
#define NOSHARE
#endif
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG(a)
#endif

#ifndef PP
#define PP(x)	()		/* function prototype */
#ifndef NO_PROTOTYPE
#define	NO_PROTOTYPE
#endif
#endif

#if defined(__STDC__) || defined(__ZTC__) || defined(__BORLANDC__) || defined(vax11c) || defined(m88k) || defined(_WIN32) || defined(__linux__)
#include <stdlib.h>
#endif

#if defined(_MSC_VER) && (defined(_WINDOWS) || defined(_WIN32))
#include <string.h>
#endif

#if	TURBO
#include      <dos.h>
#include      <mem.h>
#undef peek
#undef poke
#define       peek(a,b,c,d)   movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define       poke(a,b,c,d)   movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#endif

#if	USG | FREEBSD2
#include	<string.h>
#endif

#if	ZORTECH
#include      <dos.h>
#include      <string.h>
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if	VMS
/* #define	abs	xabs */
#define	getname	xgetname
#if !defined(vax11c)
#include <string.h>
#endif
#endif

#if	LATTICE
#define	unsigned
#endif

#if	AZTEC
#undef	fputc
#undef	fgetc
#if	MSDOS
#define	fgetc	a1getc
#else
#define	fgetc	agetc
#endif
#define	fputc	aputc
#define	int86	sysint
#define	intdos(a, b)	sysint(33, a, b)
#define	inp	inportb
#define	outp	outportb

struct XREG {
	int ax,bx,cx,dx,si,di,ds,es;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh,d1,d2,e1,e2;
};

struct SREG {
	unsigned cs, ss, ds, es;
};
union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & DTL
#include <dos.h>
#endif

#if	MSDOS & MWC86
#include	<dos.h>
#define	int86(a, b, c)	intcall(b, c, a)
#define	intdos(a, b)	intcall(a, b, DOSINT)
#define	inp(a)	in(a)
#define	outp(a, b)	out(a, b)
#define movmem(a, b, c)	memcpy(b, a, c)

struct XREG {
	unsigned int ax,bx,cx,dx,si,di,ds,es,flags;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
	unsigned int ds,es,flags;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & MSC
#include	<dos.h>
#include	<memory.h>
#define	peek(a,b,c,d)	movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define	poke(a,b,c,d)	movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if	MSDOS & GNUC
#undef USG
#include	<dos.h>
#include	<memory.h>
#define	movmem(a, b, c)		memcpy(b, a, c)
#define	USG 0
#endif

#if	MSDOS & LATTICE
#undef	CPM
#undef	LATTICE
#include	<dos.h>
#undef	CPM
#endif

#if	MSDOS & C86
#include	<dos.h>
#define	intdos	sysint21
#define	int86	sysint
#define	inp	inportb
#define	outp	outportb

struct XREG {
	int ax,bx,cx,dx,si,di,ds,es,flags;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
	int ds,es,flags;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	VMS
#define	unlink(a)	delete(a)

/* define input and output buffer sizes */
#define	NIBUF	128		/* input buffer size */
#define	NOBUF	1024		/* output buffer size */
#endif

/*	define some ability flags */

#if	IBMPC | Z309
#define	MEMMAP	1
#else
#define	MEMMAP	0
#endif

#if	((MSDOS) & (LATTICE | AZTEC | MSC | TURBO | GNUC)) | V7 | USG | BSD | VMS
#define	ENVFUNC	1
#else
#define	ENVFUNC	0
#endif

#if USG | BSD
#define	SYMEXPAND 1		/* Enable $ symbol expansion in file names */
#else
#define SYMEXPAND 0
#endif

	/* unix v.3 dirent */
#if (defined(M_UNIX) && defined(M_COFF)) || defined(_SCO_DS) || defined(__USLC__) || defined(_AIX) || defined(__linux__) || defined(m88k) || defined(_SYSTYPE_SVR4) || SYSTEM_SVR3 || SYSTEM_SVR4 || defined(__SVR4) || defined(__CYGWIN32__) || defined(__hpux) || defined(__INTERIX) || defined(__MINGW32__)
#define	USGV3	1
#else
#define	USGV3	0
#endif

#ifndef FAR
#if (ZORTECH && !defined(DOS386)) || TURBO
#define	FAR	far
#else
#define	FAR
#endif
#endif

/*	internal constants	*/

#if	DECEDT
#define	NBINDS	512			/* max # of bound keys		*/
#else
#define	NBINDS	300			/* max # of bound keys		*/
#endif
#define	NFBIND	0x80			/* # of bindings in fast lookup	*/

#if USGV3
#define NFILEN	512			/* # of bytes, file name	*/
#define NLINE	512			/* # of bytes, input line	*/
#define	NSTRING	512			/* # of bytes, string buffers	*/
#define NPAT	512			/* # of bytes, pattern		*/
#else
#define NFILEN	80			/* # of bytes, file name	*/
#define NLINE	256			/* # of bytes, input line	*/
#define	NSTRING	128			/* # of bytes, string buffers	*/
#define NPAT	128			/* # of bytes, pattern		*/
#endif

#if USGV3
#define NBUFN	32			/* # of bytes, buffer name	*/
#else
#define NBUFN   16                      /* # of bytes, buffer name      */
#endif
#if DECEDT | USGV3
#define NKBDM	2048			/* # of strokes, keyboard macro */
#else
#define NKBDM   256                     /* # of strokes, keyboard macro */
#endif
#ifdef HUGE
#undef HUGE	/* conflicts with math.h */
#endif
#define HUGE    1000                    /* Huge number                  */
#define	NLOCKS	100			/* max # of file locks active	*/
#define	NCOLORS	8			/* number of supported colors	*/
#define	KBLOCK	250			/* sizeof kill buffer chunks	*/
#define	NBLOCK	16			/* line block chunk size	*/
#define	NVSIZE	10			/* max #chars in a var name	*/

#define CTRL    0x0100                  /* Control flag, or'ed in       */
#define META    0x0200                  /* Meta flag, or'ed in          */
#define CTLX    0x0400                  /* ^X flag, or'ed in            */
#define	SPEC	0x0800			/* special key (function keys)	*/
#define	MOUS	0x1000			/* alternate input device (mouse) */
#define	SHFT	0x2000			/* shifted (for function keys)	*/
#define	ALTD	0x4000			/* ALT key...			*/

#define	FUNC	(SPEC|CTLX|META)	/* prefix for special functions */

#ifdef	FALSE
#undef	FALSE
#endif
#ifdef	TRUE
#undef	TRUE
#endif

#define FALSE   0                       /* False, no, bad, etc.         */
#define TRUE    1                       /* True, yes, good, etc.        */
#define ABORT   2                       /* Death, ^G, abort, etc.       */
#define	FAILED	3			/* not-quite fatal false return	*/
#define	HOOK	4			/* execute called with FUNC	*/

#define	STOP	0			/* keyboard macro not in use	*/
#define	PLAY	1			/*		  playing	*/
#define	RECORD	2			/*		  recording	*/

/*	Directive definitions	*/

#define	DIF		0
#define DELSE		1
#define DENDIF		2
#define DGOTO		3
#define DRETURN		4
#define DENDM		5
#define DWHILE		6
#define	DENDWHILE	7
#define	DBREAK		8
#define DFORCE		9

#define NUMDIRS		10

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define	PTBEG	0	/* Leave the point at the beginning on search	*/
#define	PTEND	1	/* Leave the point at the end on search		*/
#define	FORWARD	0			/* forward direction		*/
#define REVERSE	1			/* backwards direction		*/

#define FIOSUC  0                       /* File I/O, success.           */
#define FIOFNF  1                       /* File I/O, file not found.    */
#define FIOEOF  2                       /* File I/O, end of file.       */
#define FIOERR  3                       /* File I/O, error.             */
#define	FIOMEM	4			/* File I/O, out of memory	*/
#define	FIOFUN	5			/* File I/O, eod of file/bad line*/
#define	FIODEL	6			/* Can't delete/rename file	*/

#define CFCPCN  0x0001                  /* Last command was C-P, C-N    */
#define CFKILL  0x0002                  /* Last command was a kill      */

#define	BELL	0x07			/* a bell character		*/
#define	TAB	0x09			/* a tab character		*/

#if	V7 | USG | BSD
#define	PATHCHR	':'
#else
#if	WMCS
#define	PATHCHR	','
#else
#define	PATHCHR	';'
#endif
#endif

#define	INTWIDTH	sizeof(int) * 3

/*	Macro argument token types					*/

#define	TKNUL	0			/* end-of-string		*/
#define	TKARG	1			/* interactive argument		*/
#define	TKBUF	2			/* buffer argument		*/
#define	TKVAR	3			/* user variables		*/
#define	TKENV	4			/* environment variables	*/
#define	TKFUN	5			/* function....			*/
#define	TKDIR	6			/* directive			*/
#define	TKLBL	7			/* line label			*/
#define	TKLIT	8			/* numeric literal		*/
#define	TKSTR	9			/* quoted string literal	*/
#define	TKCMD	10			/* command name			*/

/*	Internal defined functions					*/

#define	nextab(a)	(a & ~7) + 8
#ifdef	abs
#undef	abs
#endif

/* DIFCASE represents the integer difference between upper
   and lower case letters.  It is an xor-able value, which is
   fortunate, since the relative positions of upper to lower
   case letters is the opposite of ascii in ebcdic.
*/

#ifdef	islower
#undef	islower
#endif

#if	ASCII

#define	DIFCASE		0x20
#define isletter(c)	(('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c))
#define islower(c)	(('a' <= c && 'z' >= c))
#endif

#if	EBCDIC

#define	DIFCASE		0x40
#define isletter(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c) || ('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#define islower(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c))
#endif

/*	Dynamic RAM tracking and reporting redefinitions	*/

#if	RAMSIZE
#define	malloc	allocate
#define	free	release
#endif

/* patch for bug in MSC for XENIX */

#if	USG & MSC
typedef short shrt;
#else
typedef char shrt;
#endif

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
typedef struct  WINDOWTAG {
        struct  WINDOWTAG *w_wndp;	/* Next window                  */
        struct  BUFFERTAG *w_bufp;	/* Buffer displayed in window   */
        struct  LINETAG *w_linep;	/* Top line in the window       */
        struct  LINETAG *w_dotp;	/* Line containing "."          */
        int     w_doto;                 /* Byte offset for "."          */
        struct  LINETAG *w_markp;	/* Line containing "mark"       */
        int     w_marko;                /* Byte offset for "mark"       */
        int     w_toprow;               /* Origin 0 top row of window   */
        int     w_ntrows;               /* # of rows of text in window  */
        int     w_force;                /* If NZ, forcing row.          */
        shrt    w_flag;                 /* Flags.                       */
#if	COLOR
	shrt	w_fcolor;		/* current forground color	*/
	shrt	w_bcolor;		/* current background color	*/
#endif
        int	w_fcol;			/* first col to display		*/
}       WINDOW;

#define WFFORCE 0x01                    /* Window needs forced reframe  */
#define WFMOVE  0x02                    /* Movement from line to line   */
#define WFEDIT  0x04                    /* Editing within a line        */
#define WFHARD  0x08                    /* Better to a full display     */
#define WFMODE  0x10                    /* Update mode line.            */
#define	WFCOLR	0x20			/* Needs a color change		*/

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 * 	Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
typedef struct  BUFFERTAG {
        struct  BUFFERTAG *b_bufp;	/* Link to next BUFFER          */
        struct  LINETAG *b_dotp;	/* Link to "." LINE structure   */
        int     b_doto;                 /* Offset of "." in above LINE  */
        struct  LINETAG *b_markp;	/* The same as the above two,   */
        int     b_marko;                /* but for the "mark"           */
        int	b_fcol;			/* first col to display		*/
        struct  LINETAG *b_linep;	/* Link to the header LINE      */
        struct	LINETAG *b_topline;	/* Link to narrowed top text	*/
        struct	LINETAG *b_botline;	/* Link to narrows bottom text	*/
	shrt	b_active;		/* window activated flag	*/
        shrt    b_nwnd;                 /* Count of windows on buffer   */
        shrt    b_flag;                 /* Flags                        */
	int	b_mode;			/* editor mode of this buffer	*/
        char    b_fname[NFILEN];        /* File name                    */
        char    b_bname[NBUFN];         /* Buffer name                  */
	int	b_fmode;		/* Mode of file			*/
#if	VMS
	char	b_fab_rfm;		/* vms record format		*/
	char	b_fab_rat;		/* vms record attribute		*/
	char	b_fab_fsz;		/* vms header size		*/
	unsigned b_fab_mrs;		/* vms records size		*/
#endif
#if	CRYPT
	char	b_key[NPAT];		/* current encrypted key	*/
#endif
}       BUFFER;

#define BFINVS  0x01                    /* Internal invisable buffer    */
#define BFCHG   0x02                    /* Changed since last write     */
#define	BFTRUNC 0x04			/* buffer was truncated when read */
#define	BFNAROW 0x08			/* buffer has been narrowed	*/

#define	V_TEXT_SIZE	80		/* min size of v_text */

typedef struct  VIDEOTAG {
	int	v_flag;			/* Flags */
#if	COLOR
	int	v_fcolor;		/* current forground color */
	int	v_bcolor;		/* current background color */
	int	v_rfcolor;		/* requested forground color */
	int	v_rbcolor;		/* requested background color */
#endif
	char	v_text[V_TEXT_SIZE];	/* Screen data. */
}	VIDEO;

#define VFCHG   0x0001                  /* Changed flag			*/
#define	VFEXT	0x0002			/* extended (beyond column 80)	*/
#define	VFREV	0x0004			/* reverse video status		*/
#define	VFREQ	0x0008			/* reverse video request	*/
#define	VFCOL	0x0010			/* color change requested	*/
#define	VFSCROL	0x0020			/* cleared by scrolling		*/

#define	WFSCROL	0x0040			/* window scrolled		*/


/*	mode flags	*/
#define	NUMMODES	10		/* # of defined modes		*/

#define	MDWRAP	0x0001			/* word wrap			*/
#define	MDCMOD	0x0002			/* C indentation and fence match*/
#define	MDSPELL	0x0004			/* spell error parsing		*/
#define	MDEXACT	0x0008			/* Exact matching for searches	*/
#define	MDVIEW	0x0010			/* read-only buffer		*/
#define MDOVER	0x0020			/* overwrite mode		*/
#define MDMAGIC	0x0040			/* regular expresions in search */
#define	MDCRYPT	0x0080			/* encrytion mode active	*/
#define	MDASAVE	0x0100			/* auto-save mode		*/
#define MDBINARY 0x0200			/* fopen in binary mode		*/

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct  {
        struct  LINETAG *r_linep;	/* Origin LINE address.         */
        int     r_offset;               /* Origin LINE offset.          */
        long	r_size;                 /* Length in characters.        */
}       REGION;

/*
 * All text is kept in circularly linked lists of "LINE" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "BUFFER". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; it's implied. Future
 * additions will include update hints, and a list of marks into the line.
 */
typedef struct  LINETAG {
        struct  LINETAG *l_fp;		/* Link to the next line        */
        struct  LINETAG *l_bp;		/* Link to the previous line    */
        int     l_size;                 /* Allocated size               */
        int     l_used;                 /* Used size                    */
        char    l_text[1];              /* A bunch of characters.       */
}       LINE;

#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define llength(lp)     ((lp)->l_used)

/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
typedef struct  {
	short	t_mrow;			/* max number of rows allowable	*/
	short	t_nrow;			/* current number of rows used	*/
	short	t_mcol;			/* max Number of columns.	*/
	short	t_ncol;			/* current Number of columns.	*/
	short	t_margin;		/* min margin for extended lines*/
	short	t_scrsiz;		/* size of scroll region "	*/
	int	t_pause;		/* # times thru update to pause	*/
	int	(*t_open) PP((void));	/* Open terminal at the start.	*/
	int	(*t_close) PP((void));	/* Close terminal at end.	*/
	int	(*t_kopen) PP((void));	/* Open keyboard		*/
	int	(*t_kclose) PP((void));	/* close keyboard		*/
	int	(*t_getchar) PP((void));/* Get character from keyboard. */
	int	(*t_putchar) PP((int c));/* Put character to display.	*/
	int	(*t_flush) PP((void));	/* Flush output buffers.	*/
	int	(*t_move) PP((int row, int col)); /* Move the cursor, origin 0. */
	int	(*t_eeol) PP((void));	/* Erase to end of line.	*/
	int	(*t_eeop) PP((void));	/* Erase to end of page.	*/
	int	(*t_beep) PP((void));	/* Beep.			*/
	int	(*t_rev) PP((int state));/* set reverse video state	*/
	int	(*t_rez) PP((char *res)); /* change screen resolution	*/
#if	COLOR
	int	(*t_setfor) PP((int color));	/* set forground color	*/
	int	(*t_setback) PP((int color));	/* set background color	*/
#endif
}	TERM;

/*	TEMPORARY macros for terminal I/O  (to be placed in a machine
					    dependant place later)	*/

#define	TTopen		(VOID) (*term.t_open)
#define	TTclose		(VOID) (*term.t_close)
#define	TTkopen		(VOID) (*term.t_kopen)
#define	TTkclose	(VOID) (*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(VOID) (*term.t_putchar)
#define	TTflush		(VOID) (*term.t_flush)
#define	TTmove		(VOID) (*term.t_move)
#define	TTeeol		(VOID) (*term.t_eeol)
#define	TTeeop		(VOID) (*term.t_eeop)
#define	TTbeep		(VOID) (*term.t_beep)
#define	TTrev		(VOID) (*term.t_rev)
#define	TTrez		(*term.t_rez)
#if	COLOR
#define	TTforg		(VOID) (*term.t_setfor)
#define	TTbacg		(VOID) (*term.t_setback)
#endif

/*	structure for the table of initial key bindings		*/

typedef struct  {
	short	k_code;				/* Key code		*/
	int	(*k_fp) PP((int f, int n));	/* Routine to handle it	*/
}	KEYTAB;

/*	structure for the name binding table		*/

typedef struct {
	CONSTA char *n_name;			/* name of function key */
	int (*n_func) PP((int f, int n));	/* function name is bound to */
}	NBIND;

/*	The editor holds deleted text chunks in the KILL buffer. The
	kill buffer is logically a stream of ascii characters, however
	due to its unpredicatable size, it gets implemented as a linked
	list of chunks. (The d_ prefix is for "deleted" text, as k_
	was taken up by the keycode structure)
*/

typedef	struct KILLTAG {
	struct KILLTAG *d_next;	/* link to next chunk, NULL if last */
	char d_chunk[KBLOCK];	/* deleted text */
} KILL;

/*	When emacs' command interpetor needs to get a variable's name,
	rather than it's value, it is passed back as a VDESC variable
	description structure. The v_num field is a index into the
	appropriate variable table.
*/

typedef struct VDESCTAG {
	int v_type;	/* type of variable */
	int v_num;	/* ordinal pointer to variable in list */
} VDESC;

/*	The !WHILE directive in the execution language needs to
	stack references to pending whiles. These are stored linked
	to each currently open procedure via a linked list of
	the following structure
*/

typedef struct WHBLKTAG {
	LINE *w_begin;		/* ptr to !while statement */
	LINE *w_end;		/* ptr to the !endwhile statement*/
	int w_type;		/* block type */
	struct WHBLKTAG *w_next;	/* next while */
} WHBLOCK;

#define	BTWHILE		1
#define	BTBREAK		2

/* HICHAR - 1 is the largest character we will deal with.
 * HIBYTE represents the number of bytes in the bitmap.
 */

#define	HICHAR		256
#define	HIBYTE		HICHAR >> 3

#define	CHCASE(c)	((c) ^ DIFCASE)	/* Toggle the case of a letter.*/

#if	MAGIC

/*
 * Defines for the metacharacters in the regular expressions in search
 * routines.
 */

typedef char	*BITMAP;

typedef	struct {
	short		mc_type;
	union {
		int	lchar;
		BITMAP	cclmap;
	} u;
} MC;

typedef struct {
	short		mc_type;
	char *rstr;
} RMC;
#endif

#endif
