/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     ANSI

#define	MROW	25

#if	AMIGA
#define NROW    23                      /* Screen size.                 */
#define NCOL    77                      /* Edit if you want to.         */
#else
#if	COLOR
#define NROW    25                      /* Screen size.                 */
#else
#define NROW    24                      /* Screen size.                 */
#endif
#define NCOL    80                      /* Edit if you want to.         */
#endif
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

static  VOID    ansimove PP((int row, int col));	/* Forward references.          */
static  VOID    ansiel PP((void));
static  VOID    ansiep PP((void));
static  VOID    ansibeep PP((void));
static  int     ansiopen PP((void));
static	VOID	ansirev PP((int state));
static	int	ansiclose PP((void));
static	int	ansikopen PP((void));
static	int	ansikclose PP((void));
static	int	ansicres PP((CONSTA char *res));
static  VOID	ansiparm PP((int n));

#if	COLOR
static	VOID	ansifcol PP((int color));
static	VOID	ansibcol PP((int color));

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */

#if	AMIGA
/* apperently the AMIGA does not follow the ANSI standards as
   regards to colors....maybe because of the default pallette
   settings?
*/

int coltran[8] = {2, 3, 5, 7, 0, 4, 6, 1};	/* color translation table */
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
NOSHARE TERM    term    = {
	MROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        ansiopen,
        ansiclose,
	ansikopen,
	ansikclose,
        ttgetc,
        ttputc,
        ttflush,
        ansimove,
        ansiel,
        ansiep,
        ansibeep,
	ansirev,
	ansicres
#if	COLOR
	, ansifcol,
	ansibcol
#endif
};

#if	COLOR
static VOID ansifcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+30);
#else
	ansiparm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

static VOID ansibcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+40);
#else
	ansiparm(color+40);
#endif
	ttputc('m');
        cbcolor = color;
}
#endif

static VOID ansimove(row, col)
int row, col;
{
        ttputc(ESC);
        ttputc('[');
        ansiparm(row+1);
        ttputc(';');
        ansiparm(col+1);
        ttputc('H');
}

static VOID ansiel()
{
        ttputc(ESC);
        ttputc('[');
        ttputc('K');
}

static VOID ansiep()
{
#if	COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('[');
        ttputc('J');
}

static VOID ansirev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	int ftmp, btmp;		/* temporaries for colors */
#endif

	ttputc(ESC);
	ttputc('[');
	ttputc(state ? '7': '0');
	ttputc('m');
#if	COLOR
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif
}

static int ansicres(res)	/* change screen resolution */
CONSTA char *res;
{
	UNUSED_ARG(res);
	return(TRUE);
}

int spal(dummy)		/* change pallette settings */
char *dummy;
{
	/* none for now */
	UNUSED_ARG(dummy);
	return TRUE;
}

static VOID ansibeep()
{
        ttputc(BEL);
        ttflush();
}

static VOID ansiparm(n)
register int    n;
{
        register int q,r;

        q = n/10;
        if (q != 0) {
		r = q/10;
		if (r != 0) {
			ttputc((r%10)+'0');
		}
		ttputc((q%10) + '0');
        }
        ttputc((n%10) + '0');
}

static int ansiopen()
{
	register char *cp;
	int n;

#if     V7 | USG | BSD
        if ((cp = getenv("TERM")) == NULL) {
                puts("Shell variable TERM not defined!");
                exit(1);
        }
	if (strcmp(cp, "vt100") != 0 &&
	    strcmp(cp, "ansi") != 0 &&
	    strcmp(cp, "rxvt") != 0 &&
	    strcmp(cp, "xterm") != 0) {
                puts("Terminal type not 'vt100'!");
                exit(1);
        }
#endif
	/* Set the terminal size using */
	/*	LINES and COLUMNS environment variables */

	n = 0;
	cp = getenv("LINES");
	if (cp != NULL && *cp != '\0') n = atoi(cp);
	if (n > 1) term.t_nrow = term.t_mrow = (short) (n - 1);

	n = 0;
	cp = getenv("COLUMNS");
	if (cp != NULL && *cp != '\0') n = atoi(cp);
	if (n > 1) term.t_ncol = term.t_mcol = (short) n;

	strcpy(sres, "NORMAL");
	revexist = TRUE;
        return ttopen();
}

static int ansiclose()

{
#if	COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	return ttclose();
}

static int ansikopen()	/* open the keyboard (a noop here) */

{
	return TRUE;
}

static int ansikclose()	/* close the keyboard (a noop here) */

{
	return TRUE;
}

#if	FLABEL
int fnclabel(f, n)	/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
VOID ansihello PP((void));

VOID ansihello PP((void))
{
}
#endif

