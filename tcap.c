/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
		for MicroEMACS
*/

#define	termdef	1			/* don't define "term" external */

#include <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if TERMCAP

/* temporarily redefine curses identifiers that conflict with microemacs */
#define WINDOW CURSES_WINDOW
#define meta CURSES_meta

#include <curses.h>
#include <term.h>

#undef WINDOW
#undef meta

#define	MARGIN	8
#define	SCRSIZ	64
#define	NPAUSE	10			/* # times thru update to pause */
#define BEL     0x07
#define ESC     0x1B

#ifndef __NCURSES_H
extern int	tgetnum();
extern int      tput();
extern char     *tgoto();
extern char     *tgetstr();
#endif

static int	tcapclose PP((void));
static int	tcapkopen PP((void));
static int	tcapkclose PP((void));
static VOID	tcapmove PP((int row, int col));
static VOID	tcapeol PP((void));
static VOID	tcapeop PP((void));
static VOID	tcapbeep PP((void));
static VOID	tcaprev PP((int state));
static int	tcapcres PP((char *res));
static int	tcapopen PP((void));
#if	COLOR
static	VOID	tcapfcol PP((int color));
static	VOID	tcapbcol PP((int color));

static VOID tcapparm PP((int n));

static VOID putpad PP((char *str));

NOSHARE int isansi = 0;		/* has support for ansi colors */
static	int cfcolor = -1;	/* current forground color */
static	int cbcolor = -1;	/* current background color */
NOSHARE int usedcolor = 0;	/* true if used a color */
#endif

#define TCAPSLEN 512
static	char tcapbuf[TCAPSLEN];
NOSHARE char *UP, PC, *CM, *CE, *CL, *SO, *SE;

static int oldwidth;
static int tcaprv;

NOSHARE extern int termflag;

NOSHARE TERM term = {
	0,	/* these four values are set dynamically at open time */
	0,
	0,
	0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        tcapopen,
        tcapclose,
        tcapkopen,
        tcapkclose,
        ttgetc,
        ttputc,
        ttflush,
        tcapmove,
        tcapeol,
        tcapeop,
        tcapbeep,
        tcaprev,
        tcapcres
#if	COLOR
	, tcapfcol,
	tcapbcol
#endif
};

#define fastputc(c)	TTputc(c)

#if     V7 | USG | BSD
#undef fastputc
#define fastputc(c)        fputc((c), stdout)
#endif

/* zap null counts on request */
static char *mytgetstr(code, space)
char *code, **space;
{
	char *v, *s;
	int got_dollar, copy_len;

	if (*space >= &tcapbuf[TCAPSLEN-20])
	{
		printf("Terminal description too big before %s!\n", code);
		exit(1);
	}	
	v = tgetstr(code, space);
	if (v != NULL && (termflag&16) == 0) {
		/* Remove null count stored as a leading count */
		while (*v >= '0' && *v <= '9') v++;
		/* Remove null count stored as a trailing $<number> */
		s = v;
		while (*s != '\0' && *s != '$') {
			s++;
		}
		got_dollar = FALSE;
		copy_len = (int) (s - v);
		if (*s == '$') {
			s++;
			if (*s == '<') {
				s++;
				while (*s >= '0' && *s <= '9') s++;
				if (*s == '>') {
					s++;
					if (*s == '\0') got_dollar = TRUE;
				}
			}
		}
		if (got_dollar) {
			s = (char *) malloc(copy_len + 1);
			if (s) {
				memcpy(s, v, copy_len);
				s[ copy_len ] = '\0';
				v = s;
			}
		}
	}
	return(v);
}

static int tcapopen()

{
        char *t, *p;
        char tcbuf[1024];
        char *tv_stype;
        char err_str[72];
	char *IS;
	int n;
	/* char *temp; */

	phrow = scrltop = scrlbot = 1000;
	tcaprv = FAILED;

        if ((tv_stype = getenv("TERM")) == NULL)
        {
                puts("Environment variable TERM not defined!");
                exit(1);
        }

        if ((tgetent(tcbuf, tv_stype)) !=
#if defined(__hpux) && defined(__ia64) && !defined(__hppa)
		0	/* emulated version returns -1 if errors, 0 if ok */
#else
		1	/* standard version returns -1 if no db, 0 if entry not found, 0 if ok */
#endif
        	)
        {
                sprintf(err_str, "Unknown terminal type %s!", tv_stype);
                puts(err_str);
                exit(1);
        }

 
	if ((*tv_stype=='v' && *(tv_stype+1)=='t' && *(tv_stype+2)!='5') ||
	    strncmp(tv_stype, "xterm-", 6) == 0 ||
	    strcmp(tv_stype, "rxvt") == 0 ||
	    strcmp(tv_stype, "linux") == 0) {
		isvt100 = TRUE;
		/* if (*(tv_stype+2) == '2') */ vt100keys = 1;
#if DISPSEVEN
		if (dispseven == FALSE && strcmp(tv_stype, "vt100") == 0) {
			dispseven = TRUE;
			/* upwind(); */ /* vtinit() comes before edinit() */
		}
#endif
	}
#if	COLOR
	cfcolor = -1;
	cbcolor = -1;
	isansi = isvt100;
	if (*tv_stype=='a' && *(tv_stype+1)=='n' && *(tv_stype+2)=='s')
		isansi = TRUE;
#endif

	/* Set the terminal size using (by increasing precendence */
	/*	tgetnum() values from termcap */
	/*	LINES and COLUMNS environment variables */
	/*	winsize ioctl */

	n = 0;
	t = getenv("LINES");
	if (t != NULL && *t != '\0') n = atoi(t);
	if (n <= 0) n = tgetnum("li");
	if (n <= 0) {
		puts("termcap entry incomplete (lines)");
		exit(1);
	}
	term.t_nrow = (short) (n - 1);

	n = 0;
	t = getenv("COLUMNS");
	if (t != NULL && *t != '\0') n = atoi(t);
	if (n <= 0) n = tgetnum("co");
	if (n <= 0) {
		puts("Termcap entry incomplete (columns)");
		exit(1);
        }
	term.t_ncol = (short) n;

	getwinsize(&term.t_ncol, &term.t_nrow);

#if 0	/* DEBUG */
	fprintf(stderr, "tcopen, COLUMNS %s co %d winsiz %d\n", getenv("COLUMNS"), tgetnum("co"), term.t_ncol);
	fprintf(stderr, "tcopen, LINES %s li %d winsiz %d\n", getenv("LINES"), tgetnum("li"), term.t_nrow);
#endif

	term.t_mrow = term.t_nrow;
	term.t_mcol = term.t_ncol;

	if (isvt100 && term.t_mcol < 132) term.t_mcol = 132;
	oldwidth = term.t_ncol;

	/* xterm 384 has a bug with scrolling in big windows */
	if (isvt100 && term.t_mrow > 70 && strncmp(tv_stype, "xterm-", 6) == 0) {
		termflag |= 2;
	}

        p = tcapbuf;
        t = mytgetstr("pc", &p);
        if(t)
                PC = *t;

        CL = mytgetstr("cl", &p);
        CM = mytgetstr("cm", &p);
        CE = mytgetstr("ce", &p);
        UP = mytgetstr("up", &p);
	SE = mytgetstr("se", &p);
	SO = mytgetstr("so", &p);
	IS = mytgetstr("is", &p);
	if (SO != NULL)
		revexist = TRUE;
	if (termflag&4)
		revexist = FALSE;

        if(CL == NULL || CM == NULL || UP == NULL)
        {
                puts("Incomplete termcap entry");
		if (CL == NULL) puts(" [CL empty]");
		if (CM == NULL) puts(" [CM empty]");
		if (UP == NULL) puts(" [UP empty]");
		puts("\n");
                exit(1);
        }

	if (CE == NULL)		/* will we be able to use clear to EOL? */
		eolexist = FALSE;
		
        if (p >= &tcapbuf[TCAPSLEN])
        {
                puts("Terminal description too big!\n");
                exit(1);
        }
        ttopen();
	if (IS != NULL)			/* initialize terminal */
		while(*IS) fastputc(*IS++);

	/* if ((temp = getenv("vt100key")) != NULL) vt100keys = (*temp&1); */

#if	DECEDT
	/* hack to force vt100's into keypad mode */
	if (isvt100) {
                /* Turn on application keypad */
		fastputc('\033'); fastputc('=');

		/* Turn off cursor key mode */
		fastputc('\033'); fastputc('['); fastputc('?');
		fastputc('1'); fastputc('l');

		/*
		** Turn off "real NumLock", so that xterm generates
		** application keypad escape sequences when NumLock
		** is on.
		*/
		fastputc('\033'); fastputc('['); fastputc('?');
		fastputc('1'); fastputc('0'); fastputc('3'); fastputc('5');
		fastputc('l');
	}
#endif
	return TRUE;
}

static int tcapclose()
{
	if (isvt100)	{
		ttsetwid(oldwidth);
	}
#if	COLOR
	if (usedcolor) {
		tcapfcol(7);
		tcapbcol(0);
	}
#endif
	return ttclose();
}

static int tcapkopen()

{
	strcpy(sres, "NORMAL");
	return TRUE;
}

static int tcapkclose()

{
	return TRUE;
}

static VOID tcapmove(row, col)
register int row, col;
{
	if (isvt100 && col == 0)
		if (row == phrow+1 && row != scrlbot)
			{fastputc(13); fastputc(10);}
		else if (row == phrow-1 && row+2 != scrltop)
			{fastputc(13); fastputc(27); fastputc('M');}
		else if (row == phrow)
			{fastputc(13);}
		else {
			fastputc(27); fastputc('[');
			if (row >= 99) {
				fastputc('0' + (row+1)/100);
				fastputc('0' + ((row+1)%100)/10);
			} else if (row >= 9) {
				fastputc('0' + (row+1)/10);
			}
			if (row > 0) fastputc('0' + (row+1) % 10);
			fastputc('H');
		}
        else putpad(tgoto(CM, col, row));
	phrow = row;
}

static VOID tcapeol()
{
#if	COLOR
	tcapfcol(gfcolor);
	tcapbcol(gbcolor);
#endif
        putpad(CE);
}

static VOID tcapeop()
{
#if	COLOR
	tcapfcol(gfcolor);
	tcapbcol(gbcolor);
#endif
        putpad(CL);
	phrow = 1000;
}

static VOID tcaprev(state)	/* change reverse video status */

int state;		/* FALSE = normal video, TRUE = reverse video */

{
#if	COLOR
	/* int ftmp, btmp; */
#endif

#if	COLOR
	/* forground and background already set */
	if (usedcolor) return;
#endif
	if (state == tcaprv)	return;
	tcaprv = state;
	if (!revexist)	return;
	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else	{
		if (SE != NULL)
			putpad(SE);
#if	COLOR
		/*
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		tcapfcol(ftmp);
		tcapbcol(btmp);
		*/
#endif
	}
}

static int tcapcres(res)	/* change screen resolution */
char *res;

{
	UNUSED_ARG(res);
	return(TRUE);
}

int spal(dummy)	/* change palette string */
char *dummy;
{
	/* Does nothing here */
	UNUSED_ARG(dummy);
	return TRUE;
}

#if	COLOR
static VOID tcapparm(n)
int n;
{
	register int q, r;

	q = n/10;
	if (q != 0)	{
		r = q/10;
		if (r != 0)	{fastputc('0' + r%10);}
		fastputc('0' + q%10);
	}
	fastputc('0' + n%10);
}

static VOID tcapfcol(color)	/* set the current output color */
int color;
{
	if (!usedcolor || color == cfcolor)	return;
	fastputc(ESC);
	fastputc('[');
	tcapparm(color + 30);
	fastputc('m');
	cfcolor = color;
}

static VOID tcapbcol(color)	/* set the current background color */
int color;
{
	if (!usedcolor || color == cbcolor)	return;
	fastputc(ESC);
	fastputc('[');
	tcapparm(color + 40);
	fastputc('m');
	cbcolor = color;
}
#endif

static VOID tcapbeep()
{
	fastputc(BEL);
}

static VOID putpad(str)
char    *str;
{
#if USG && MSC
	int cnt;
	cnt = 0;
	while(*str >= '0' && *str <= '9')
		cnt = cnt*10 + *str++ - '0';
	while(cnt-- > 0) fastputc(PC);
#if	defined(sun) || SYSTEM_SUN
	while (*str != '\0') {
		if (str[0] == '$' && str[1] == '<') {
			while (*str != '>' && *str != '\0') {
				str++;
			}
			if (*str == '\0') break;
		} else {
			putc(*str, stdout);
		}
		str++;
	}
#else
	while(*str != '\0') {putc(*str, stdout); str++;}
#endif
#else
	tputs(str, 1, ttputc);
#endif
}

#if 0 /* not used */
putnpad(str, n)
char    *str;
{
	tputs(str, n, ttputc);
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

tcaphello()
{
}

#endif

