/*
 * The routines in this file
 * provide support for VT52 style terminals
 * over a serial line. The serial I/O services are
 * provided by routines in "termio.c". It compiles
 * into nothing if not a VT52 style device. The
 * bell on the VT52 is terrible, so the "beep"
 * routine is conditionalized on defining BEL.
 */
#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"edef.h"

#if     EM_VT52

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	100			/* # times thru update to pause */
#define BIAS    0x20                    /* Origin 0 coordinate bias.    */
#define ESC     0x1B                    /* ESC character.               */
#define BEL     0x07                    /* ascii bell character         */

extern  VOID    vt52move PP((int row, int col));	/* Forward references.          */
extern  VOID    vt52eeol PP((void));
extern  VOID    vt52eeop PP((void));
extern  VOID    vt52beep PP((void));
extern  int     vt52open PP((void));
extern	VOID	vt52rev PP((int status));
extern	int	vt52cres PP((CONSTA char *res));
extern	int	vt52kopen PP((void));
extern	int	vt52kclose PP((void));

#if	COLOR
extern	VOID	vt52fcol PP((int color));
extern	VOID	vt52bcol PP((int color));
#endif

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        &vt52open,
        &ttclose,
	&vt52kopen,
	&vt52kclose,
        &ttgetc,
        &ttputc,
        &ttflush,
        &vt52move,
        &vt52eeol,
        &vt52eeop,
        &vt52beep,
        &vt52rev,
        &vt52cres
#if	COLOR
	, &vt52fcol,
	&vt52bcol
#endif
};

VOID vt52move(row, col)
int row, col;
{
        ttputc(ESC);
        ttputc('Y');
        ttputc(row+BIAS);
        ttputc(col+BIAS);
}

VOID vt52eeol()
{
        ttputc(ESC);
        ttputc('K');
}

VOID vt52eeop()
{
        ttputc(ESC);
        ttputc('J');
}

VOID vt52rev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{
	/* can't do this here, so we won't */
	UNUSED_ARG(status);
}

int vt52cres(res)	/* change screen resolution - (not here though) */
CONSTA char *res;

{
	UNUSED_ARG(res);
	return(TRUE);
}

int spal(pal)		/* change palette string */
char *pal;
{
	/*	Does nothing here	*/
	UNUSED_ARG(pal);
	return TRUE;
}

#if	COLOR
VOID vt52fcol(color)	/* set the foreground color [NOT IMPLIMENTED] */
int color;
{
	UNUSED_ARG(color);
}

VOID vt52bcol(color)	/* set the background color [NOT IMPLIMENTED] */
int color;
{
	UNUSED_ARG(color);
}
#endif

VOID vt52beep()
{
#ifdef  BEL
        ttputc(BEL);
        ttflush();
#endif
}

int vt52open()
{
#if     V7 | BSD
        register char *cp;

        if ((cp = getenv("TERM")) == NULL) {
                puts("Shell variable TERM not defined!");
                exit(1);
        }
        if (strcmp(cp, "vt52") != 0 && strcmp(cp, "z19") != 0) {
                puts("Terminal type not 'vt52'or 'z19' !");
                exit(1);
        }
#endif
        ttopen();
        return TRUE;
}

int vt52kopen()

{
	return TRUE;
}

int vt52kclose()

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

VOID vt52hello PP((void));

VOID vt52hello()

{
}

#endif
