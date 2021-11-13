/*
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 * Supported monitor cards include CGA, MONO and EGA.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     IBMPC

#if	MSDOS & GNUC
#include	<pc.h>
#endif
 
#define NROW	60			/* Max Screen size.		*/
#define NCOL	132			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define	SPACE	32			/* space character		*/

#if	MSDOS & GNUC
#define	SCADC	((char *)ScreenPrimary)
#define	SCADM	((char *)ScreenPrimary)
#define	SCADE 	((char *)ScreenPrimary)
#else
#define	SCADC	0xb8000000L		/* CGA address of screen RAM	*/
#define	SCADM	0xb0000000L		/* MONO address of screen RAM	*/
#define SCADE	0xb8000000L		/* EGA address of screen RAM	*/
#endif

#define MONOCRSR 0x0B0D			/* monochrome cursor	    */
#define CGACRSR 0x0607			/* CGA cursor		    */
#define EGACRSR 0x0709			/* EGA cursor		    */

#define	CDCGA	0			/* color graphics card		*/
#define	CDCGA40	1			/* CGA 40 wide mode		*/
#define	CDMONO	2			/* monochrome text card		*/
#define	CDEGA	3			/* EGA color adapter 80 x 43	*/
#define	CDVGA	4			/* VGA color adapter 80 x 50	*/
#define	CDVGA12	5			/* VGA 12 line mode		*/
#define	CDVGA132 6			/* Tseng ET4000 or ATI 132 x 25	*/
#define	CDVGA44	7			/* ATI 132 x 44			*/
#define	CDVGA43	8			/* VGA 132 wide 43 line mode	*/
#define	CDTVGA132 9			/* Trident 8900 132 x 25	*/
#define	CDTVGA43 10			/* Trident 8900 132 x 43	*/
#define	CDTVGA60 11			/* Trident 8900 132 x 60	*/
#define	CDV7VGA132 12			/* Video 7 132 x 25		*/
#define	CDV7VGA43 13			/* Video 7 132 x 43		*/
#define	CDGVGA132 14			/* Genoa 132 x 25		*/
#define	CDGVGA44 15			/* Genoa 132 x 43		*/
#define	CDGVGA60 16			/* Genoa 132 x 60		*/
#define	CDPVGA132 17			/* Paradise 132 x 25		*/
#define	CDPVGA43 18			/* Paradise 132 x 43		*/
#define	CDCVGA132 19			/* Chips & Technologies 132 x 25 */
#define	CDCVGA50 20			/* Chips & Technologies 132 x 50 */
#define CDTLVGA44 21			/* Tseng Labs 132 x 44		*/

#define NDRIVE	22			/* number of screen drivers	*/

#define	CDSENSE	99			/* detect the card type		*/

static int dtype = -1;			/* current display type		*/
static char FAR drvname[NDRIVE][8] = {	/* screen resolution names	*/
	"CGA", "CGA40", "MONO", "EGA", "VGA", "VGA12",
	"VGA132", "VGA44", "VGA43",
	"TVGA132", "TVGA43", "TVGA60",
	"V7VGA132", "V7VGA43",
	"GVGA132", "GVGA44", "GVGA60",
	"PVGA132", "PVGA43",
	"CVGA132", "CVGA50",
	"TLVGA44"
};
static long scadd;			/* address of screen ram	*/
#if ZORTECH && defined(__SMALL__)
#define	ZORDIS	1
#else
#define ZORDIS	0
#endif
#define	ZOR386	0
#if ZORTECH && 0
#ifdef DOS386
#undef ZOR386
#define ZOR386 1
#endif
#endif
#if ZORDIS
#include <disp.h>
#endif
#if ZOR386
static short far * FAR scptr[NROW];	/* pointer to screen lines	*/
#else
static short * FAR scptr[NROW];		/* pointer to screen lines	*/
#endif
static unsigned short FAR sline[NCOL];	/* screen line image		*/
static int num_cols;			/* current number of columns	*/
static int orig_mode;			/* screen mode on entry		*/
static int egaexist = FALSE;		/* is an EGA card available?	*/
static int vgaexist = FALSE;		/* is video graphics array available? */
extern union REGS rg;			/* cpu register for use of DOS calls */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     ibmmove();
extern  int     ibmeeol();
extern  int     ibmeeop();
extern  int     ibmbeep();
extern  int     ibmopen();
extern	int	ibmrev();
extern	int	ibmcres();
extern	int	ibmclose();
extern	int	ibmputc();
extern	int	ibmkopen();
extern	int	ibmkclose();

#if	COLOR
extern	int	ibmfcol();
extern	int	ibmbcol();

static int	cfcolor = -1;	/* current forground color */
static int	cbcolor = -1;	/* current background color */
static int	ctrans[] =	/* ansi to ibm color translation table */
	{0, 4, 2, 6, 1, 5, 3, 7,
	 8, 12, 10, 14, 9, 13, 11, 15};
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM FAR term = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        ibmopen,
        ibmclose,
	ibmkopen,
	ibmkclose,
        ttgetc,
	ibmputc,
        ttflush,
        ibmmove,
        ibmeeol,
        ibmeeop,
        ibmbeep,
	ibmrev,
	ibmcres
#if	COLOR
	, ibmfcol,
	ibmbcol
#endif
};

#if	COLOR
ibmfcol(color)		/* set the current output color */

int color;	/* color to set */

{
	cfcolor = ctrans[color];
}

ibmbcol(color)		/* set the current background color */

int color;	/* color to set */

{
        cbcolor = ctrans[color];
}
#endif

ibmmove(row, col)
{
#if ZORDIS
	disp_move(row, col);
	disp_flush();
#else
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
#endif
}

ibmeeol()	/* erase to the end of the line */

{
#if ZORDIS
	disp_eeol();
#else
	unsigned short attr;	/* attribute byte mask to place in RAM */
	unsigned short *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;	/* current column cursor lives */
	int crow;	/*	   row	*/

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
	else
		attr = 0x0700;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i=0; i < term.t_ncol; i++)
		*lnptr++ = SPACE | attr;

#if	!GNUC
	if (flickcode && (dtype == CDCGA || dtype == CDCGA40)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}
#endif			

	/* and send the string out */
#if ZOR386
#if 1
	{
		int i;
		for(i = ccol; i < term.t_ncol; i++)
			scptr[crow][i] = SPACE | attr;
	}
#else
	movedata(getDS(), (unsigned)&sline[0],
		FP_SEG(&scptr[crow][ccol]), FP_OFF(&scptr[crow][ccol]),
		(term.t_ncol-ccol)*2);
#endif
#else
	movmem(&sline[0], scptr[crow]+ccol, (term.t_ncol-ccol)*2);
#endif
#endif
}

ibmputc(ch)	/* put a character at the current position in the
		   current colors */

int ch;

{
#if ZORDIS
	disp_putc(ch);
	disp_flush();
#else
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (dtype != CDMONO)
		rg.h.bl = cfcolor;
	else
		rg.h.bl = 0x07;
#else
	rg.h.bl = 0x07;
#endif
	int86(0x10, &rg, &rg);
#endif
}

ibmeeop()
{
#if ZORDIS
	disp_eeop();
#else
	short attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.x.dx = (term.t_nrow << 8) | (term.t_ncol - 1);
				/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		attr = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		attr = 0;
#else
	attr = 0;
#endif
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
#endif
}

ibmrev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
	/* This never gets used under the IBM-PC driver */
}

ibmcres(res)	/* change screen resolution */

char *res;	/* resolution to change to */

{
	int i;		/* index */

	for (i = 0; i < NDRIVE; i++)
		if (strcompare(res, drvname[i], 8) == 0) {
			scinit(i);
			return(TRUE);
		}
	mlwrite("No such resolution!");
	return(FALSE);
}

spal(dummy)	/* reset the pallette registers */
char *dummy;
{
	/* nothing here now..... */
}

ibmbeep()
{
#if	MWC86
	putcnb(BEL);
#else
	bdos(6, BEL, 0);
#endif
}

ibmopen()
{
	scinit(CDSENSE);
	revexist = TRUE;
        ttopen();
}

ibmclose()

{
#if	COLOR
	ibmfcol(7);
	ibmbcol(0);
#endif
	/* exit in the same mode that we entered */
	if (dtype != orig_mode)
		scinit(orig_mode);

	ttclose();
}

ibmkopen()	/* open the keyboard */

{
	rg.x.ax = 0x3301;	/* 33 -> Control-C check, 01 -> set state */
	rg.h.dl = 0x00;		/* 0 -> disable checking */
	intdos(&rg, &rg);
}

ibmkclose()	/* close the keyboard */

{
}

scinit(type)	/* initialize the screen head pointers */

int type;	/* type of adapter to init for */

{
#if ZORDIS
	disp_open();
	strcpy(sres, "CGA");
#ifdef DOS386
	newsize(TRUE, 25);
	newwidth(TRUE, 80);
#else
	newsize(TRUE, disp_numrows);
	newwidth(TRUE, disp_numcols);
#endif
#else
	union {
		long laddr;	/* long form of address */
		short *paddr;	/* pointer form of address */
	} addr;
	int i, ncols;
#if ZOR386
	short far *scr_base_addr;
#endif

	/* if asked...find out what display is connected */
	if (type == CDSENSE)
		type = getboard();

	/* if we have nothing to do....don't do it */
	if (dtype == type)
		return(TRUE);

	/* if we try to switch to EGA and there is none, don't */
	if (type == CDEGA && !egaexist)
		return(FALSE);

	/* if we try to switch to VGA and there is none, don't */
	if (type >= CDVGA && !vgaexist)
		return(FALSE);

	/* if we had the EGA or VGA open... close it */
	if (dtype >= CDEGA || dtype == CDCGA40) {
		/* put the beast into 80 column mode */
		rg.x.ax = 3;
		int86(16, &rg, &rg);
	}

	ncols = 80;
	scadd = SCADE;

	/* and set up the various parameters as needed */
	switch (type) {
		case CDMONO:	/* Monochrome adapter */
				scadd = SCADM;
				newsize(TRUE, 25);
				break;

		case CDCGA:	/* Color graphics adapter */
				scadd = SCADC;
				newsize(TRUE, 25);
				break;

		case CDCGA40:	/* Color graphics adapter */
				scadd = SCADC;
				/* put the beast into 40 column mode */
				rg.x.ax = 1;
				int86(16, &rg, &rg);
				newsize(TRUE, 25);
				ncols = 40;
				break;

		case CDEGA:	/* Enhanced graphics adapter */
				egaopen(CDEGA);
				newsize(TRUE, 43);
				break;

		case CDVGA:	/* video graphics array - acts as EGA but more lines */
				egaopen(CDVGA);
				newsize(TRUE, 50);
				break;

		case CDVGA12:	/* Video Graphics Array 12 line mode */
				egaopen(CDVGA12);
				newsize(TRUE, 12);
				break;

		case CDVGA132:	/* Video Graphics Array 132 wide */
		case CDTVGA132:
		case CDV7VGA132:
		case CDGVGA132:
		case CDPVGA132:
		case CDCVGA132:
				egaopen(type);
				newsize(TRUE, 25);
				ncols = 132;
				break;

		case CDVGA44:	/* Video Graphics Array 132 wide 44 lines */
		case CDGVGA44:
		case CDTLVGA44:
				egaopen(type);
				newsize(TRUE, 44);
				ncols = 132;
				break;

		case CDVGA43:	/* Video Graphics Array 132 wide 43 lines */
		case CDTVGA43:
		case CDV7VGA43:
		case CDPVGA43:
				egaopen(type);
				newsize(TRUE, 43);
				ncols = 132;
				break;

		case CDCVGA50:	/* Video Graphics Array 132 wide 50 lines */
				egaopen(type);
				newsize(TRUE, 50);
				ncols = 132;
				break;

		case CDTVGA60:	/* Video Graphics Array 132 wide 60 lines */
		case CDGVGA60:
				egaopen(type);
				newsize(TRUE, 60);
				ncols = 132;
				break;
	}

	/* reset the $sres environment variable */
	strcpy(sres, drvname[type]);
	dtype = type;

#if ZOR386
	scr_base_addr = (short far *)
		_x386_mk_protected_ptr((unsigned short)(scadd >> 16));
#endif

	/* initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
#if ZOR386
		scptr[i] = &scr_base_addr[ncols * i];
#else
		addr.laddr = scadd + (long)(ncols * i * 2);
		scptr[i] = addr.paddr;
#endif
	}
	newwidth(TRUE, ncols);
#endif

	return(TRUE);
}

/* getboard:	Determine which type of display board is attached.
		Current known types include:

		CDMONO	Monochrome graphics adapter
		CDCGA	Color Graphics Adapter
		CDEGA	Extended graphics Adapter
		CDVGA	Video Graphics Array
*/

/* getboard:	Detect the current display adapter
		if MONO		set to MONO
		   CGA		set to CGA	EGAexist = FALSE
		   EGA		set to CGA	EGAexist = TRUE
		   VGA		set to CGA	EGAexist = TRUE  VGAexist = TRUE
*/

int getboard()

{
	int type;	/* board type to return */

	type = CDCGA;
	int86(0x11, &rg, &rg);
	if ((((rg.x.ax >> 4) & 3) == 3))
		type = CDMONO;

	/* test for 40 col mode */
	rg.h.ah = 15;
	int86(0x10, &rg, &rg);
	if (rg.h.al == 1)	type = CDCGA40;
	else if (rg.h.al == 35)	type = CDVGA132;	/* ATI 132 x 25 */
	else if (rg.h.al == 51)	type = CDVGA44;		/* ATI 132 x 44 */
	else if (rg.h.al == 0x57) type = CDTVGA132;	/* Trident 132 x 25 */
	else if (rg.h.al == 0x59) type = CDTVGA43;	/* Trident 132 x 43 */
	else if (rg.h.al == 0x5A) type = CDTVGA60;	/* Trident 132 x 60 */
	else if (rg.h.al == 0x41) type = CDV7VGA132;	/* Video7, 132 x 25 */
	else if (rg.h.al == 0x42) type = CDV7VGA43;	/* Video7, 132 x 43 */
	else if (rg.h.al == 0x60) type = CDGVGA132;	/* Genoa 132 x 25 */
	else if (rg.h.al == 0x63) type = CDGVGA44;	/* Genoa 132 x 44 */
	else if (rg.h.al == 0x64) type = CDGVGA60;	/* Genoa 132 x 60 */
	else if (rg.h.al == 0x55) type = CDPVGA132;	/* Paradise 132 x 25 */
	else if (rg.h.al == 0x54) type = CDPVGA43;	/* Paradise 132 x 43 */
	else if (rg.h.al == 0x60) type = CDCVGA132;	/* C & T 132 x 25 */
	else if (rg.h.al == 0x61) type = CDCVGA50;	/* C & T 132 x 50 */
	else if (rg.h.al == 0x22) type = CDTLVGA44;	/* Tseng 132 x 44 */

	/* save the original video mode */
	orig_mode = type;

	/* test if EGA present */
	rg.x.ax = 0x1200;
	rg.x.bx = 0xff10;
	int86(0x10,&rg, &rg);		/* If EGA, bh=0-1 and bl=0-3 */
	egaexist = !(rg.x.bx & 0xfefc);	/* Yes, it's EGA */
	if (egaexist) {
		/* Adapter says it's an EGA. We'll get the same response
		   from a VGA, so try to tell the two apart */
		rg.x.ax = 0x1a00;	/* read display combination */
		int86(0x10,&rg,&rg);
		if (rg.h.al == 0x1a && (rg.h.bl == 7 || rg.h.bl == 8)) {
			/* Function is supported and it's a PS/2 50,60,80 with
			   analog display, so it's VGA (I hope!) */
			vgaexist = TRUE;
		} else {
			/* Either BIOS function not supported or something
			   other then VGA so set it to be EGA */
			vgaexist = FALSE;
		}
	}
	return(type);
}

egaopen(mode)	/* init the computer to work with the EGA or VGA */
int mode;	/* mode to select [CDEGA/CDVGA] */
{
	/* ATI */

	if (mode == CDVGA132) {
		rg.x.ax = 35;	/* mode 0x23, 132 x 25, Tseng Labs ET4000 */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDVGA44) {
		rg.x.ax = 51;	/* mode 0x33, 132 x 44, ATI */
		int86(16, &rg, &rg);
		return;
	}

	/* Trident */

	if (mode == CDTVGA132) {
		rg.x.ax = 0x57;		/* 132 x 25, Trident 8900 */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDTVGA43) {
		rg.x.ax = 0x59;		/* 132 x 43, Trident 8900 */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDTVGA60) {
		rg.x.ax = 0x5A;		/* 132 x 60, Trident 8900 */
		int86(16, &rg, &rg);
		return;
	}

	/* Video 7 */

	if (mode == CDV7VGA132) {
		rg.x.ax = 0x41;		/* 132 x 25, Video7 */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDV7VGA43) {
		rg.x.ax = 0x42;		/* 132 x 43, Video7 */
		int86(16, &rg, &rg);
		return;
	}

	/* Genoa */

	if (mode == CDGVGA132) {
		rg.x.ax = 0x60;		/* 132 x 25, Genoa */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDGVGA44) {
		rg.x.ax = 0x63;		/* 132 x 43, Genoa */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDGVGA60) {
		rg.x.ax = 0x64;		/* 132 x 60, Genoa */
		int86(16, &rg, &rg);
		return;
	}

	/* Paradise */

	if (mode == CDPVGA132) {
		rg.x.ax = 0x55;		/* 132 x 25, Paradise */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDPVGA43) {
		rg.x.ax = 0x54;		/* 132 x 43, Paradise */
		int86(16, &rg, &rg);
		return;
	}

	/* Chips and Technologies */

	if (mode == CDCVGA132) {
		rg.x.ax = 0x60;		/* 132 x 25, C & T */
		int86(16, &rg, &rg);
		return;
	}
	if (mode == CDCVGA50) {
		rg.x.ax = 0x61;		/* 132 x 50, C & T */
		int86(16, &rg, &rg);
		return;
	}

	/* Tseng Labs */

	if (mode == CDTLVGA44) {
		rg.x.ax = 0x22;		/* 132 x 44, Tseng */
		int86(16, &rg, &rg);
		return;
	}

	/* set the proper number of scan lines */
	rg.h.ah = 18;
	switch (mode) {
		case CDEGA:	rg.h.al = 1; break;
		case CDVGA43:
		case CDVGA:	rg.h.al = 2; break;
		case CDVGA12:	rg.h.al = 0; break;
	}
	rg.h.bl = 48;
	int86(16, &rg, &rg);

	/* put the beast into EGA 43/VGA 50 line mode */
	rg.x.ax = (mode == CDVGA43? 35: 3);
	int86(16, &rg, &rg);

	/* set the proper character set */
	if (mode == CDVGA12) {
		rg.h.al = 20;	/*  to 8 by 16 double dot ROM         */
	} else {
		rg.h.al = 18;	/*  to 8 by 8 double dot ROM         */
	}
	rg.h.ah = 17;		/* set char. generator function code */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);

	/* select the alternative Print Screen function */
	rg.h.ah = 18;		/* alternate select function code    */
	rg.h.al = 0;		/* clear AL for no good reason       */
	rg.h.bl = 32;		/* alt. print screen routine         */
	int86(16, &rg, &rg);

	/* resize the cursor */
	rg.h.ah = 1;		/* set cursor size function code */
	rg.x.cx = 0x0607;	/* turn cursor on code */
	int86(0x10, &rg, &rg);

	/* video bios bug patch */
#if	!GNUC
	outp(0x3d4, 10);
	outp(0x3d5, 6);
#endif
}

scwrite(row, outstr, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be term.t_ncol long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
#if ZORDIS
	disp_move(row, 0);
	disp_setattr(((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15));
	disp_printf("%.*s", term.t_ncol, outstr);
	disp_setattr(((cbcolor & 15) << 4) | (cfcolor & 15));
#else
	unsigned short attr;	/* attribute byte mask to place in RAM */
	unsigned short *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
	else
		attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i=0; i<term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

#if	!GNUC
	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}
#endif

	/* and send the string out */
#if ZOR386
#if 1
	{
		int i;
		for(i = 0; i < term.t_ncol; i++)
			scptr[row][i] = sline[i];
	}
#else
	movedata(getDS(), (unsigned)&sline[0],
		FP_SEG(scptr[row]), FP_OFF(scptr[row]),
		term.t_ncol*2);
#endif
#else
	movmem(&sline[0], scptr[row],term.t_ncol*2);
#endif
#endif
}

#if	MENUS

menuwrite(row, col, outstr, len, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
int col;	/* col of screen */
char *outstr;	/* string to write out */
int len;	/* length of string */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
	unsigned short attr;	/* attribute byte mask to place in RAM */
	unsigned short *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
	else
		attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i=0; i<len; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;
	
		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}

	/* and send the string out */
#if ZOR386
	movedata(getDS(), (unsigned)&sline[0],
		FP_SEG(&scptr[row][col]), FP_OFF(&scptr[row][col]),
		len*2);
#else
	movmem(&sline[0], scptr[row]+col, len*2);
#endif
}

#endif

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
ibmhello()
{
}
#endif

