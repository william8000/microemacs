/*	Menu:	Menu processor
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	MENUS

NOSHARE extern VIDEO   **vscreen;	/* Virtual screen. */

/* menu codes return by getmkey() */

#define	MENUMERR	0	/* unrecognized command */
#define	MENUMUP		1	/* go up */
#define	MENUMDOWN	2	/* go down */
#define	MENUMLEFT	3	/* go left */
#define	MENUMRIGHT	4	/* go right */
#define	MENUMENTER	5	/* CR */

/* box drawing characters */

#if	IBMPC
#define	BOXUR	218
#define	BOXUL	191
#define	BOXLR	192
#define	BOXLL	217
#define	BOXH	196
#define	BOXV	179
#else
#define	BOXUR	'.'
#define	BOXUL	'.'
#define	BOXLR	'\''
#define	BOXLL	'\''
#define	BOXH	'-'
#define	BOXV	'|'
#endif

extern int fileread(), filesave(), filewrite(), insfile();
extern int spawncli(), spawncmd(), quit();
extern int forwsearch(), forwhunt(), nullproc(), sreplace();

extern int forwline(), backline(), forwchar(), backchar();
extern int newline(), openline(), indent(), tab(), insspace();
#if DECEDT
extern int scupwind(), scdnwind();
#endif

#define	FPNULL	0

typedef struct {
	char *m_text;
	int m_up;
	int m_down;
	int m_left;
	int m_right;
	int (*m_func)();
	} MENUTABLE;

CONST MENUTABLE menudata[] = {
/* 0 */	{ "", 0, 1, 0, 0, FPNULL },
/* 1 */	{ "File", 0, 2, 0, 9, FPNULL },			/* File option */
/* 2 */		{ "New", 1, 0, 0, 3, fileread },	/* create new file */
/* 3 */		{ "Open", 1, 0, 2, 4, fileread },	/* read old file */
/* 4 */		{ "Insert", 1, 0, 3, 5, insfile },	/* insert file */
/* 5 */		{ "Save", 1, 0, 4, 6, filesave },	/* save current file */
/* 6 */		{ "save As", 1, 0, 5, 7, filewrite },	/* write a file */
/* 7 */		{ "Dos", 1, 0, 6, 8, spawncli },	/* do a DOS command */
/* 8 */		{ "Exit", 1, 0, 7, 0, quit },		/* exit */
/* 9 */	{ "Edit", 0, 10, 1, 13, FPNULL },
/* 10 */	{ "Search", 9, 0, 0, 11, forwsearch },	/* search */
/* 11 */	{ "Last search", 9, 0, 10, 12, forwhunt }, /* hunt */
/* 12 */	{ "Replace", 9, 0, 11, 0, sreplace },	/* replace */
/* 13 */ { "Compile", 0, 0, 9, 0, spawncmd }
	};

NOSHARE int menutwid;	/* width of entries on top line */

NOSHARE int menulast;	/* last option on main menu */

menuinit()	/* create menu line */
{
	LINE *lalloc();
	int i, j, num, wid, m, len;

	if (menuflag)	{
		menulast = 1;

		if (menuline != NULL) free(menuline);
		menuline = lalloc(term.t_ncol);
		if (menuline == NULL)	{
			mlwrite("[Can not create menu line]");
			menuflag = 0;
			return;
		}
		menuline->l_fp = NULL;
		menuline->l_bp = NULL;
		menuline->l_used = term.t_ncol;
		num = menucount(0);
		menutwid = llength(menuline) / num;
		wid = menutwid - 1;
		for (i = 0; i < llength(menuline); i++)
			lputc(menuline, i, ' ');
		j = 0;
		for (m = 1; m != 0; m = menudata[m].m_right)	{
			for (i = 0; menudata[m].m_text[i] && i < wid; i++)
				lputc(menuline, i+j, menudata[m].m_text[i]);
			j += menutwid;
		}
	}
}

/* find number of options beneath m */

int menucount(m)
int m;
{
	int c;

	c = 0;
	for(m = menudata[m].m_down; m != 0; m = menudata[m].m_right)
		c++;
	return(c);
}

/* find length of longest string under m */

int menulen(m)
int m;
{
	int i, len;

	len = 0;
	for(m = menudata[m].m_down; m != 0; m = menudata[m].m_right) {
		i = strlen( menudata[m].m_text );
		if (i > len) len = i;
	}
	return(len);
}

NOSHARE int menukey;	/* current key pressed */

getmkey()		/* read a menu movement command */
{
	int c, cmd;
	int (*f)(), (*getbind())();
#if	C86
	int (*fbackline)(), (*fforwline)();
	int (*fbackchar)(), (*fforwchar)(), (*findent)(), (*ftab)();
	int (*finsspace)(), (*fnewline)(), (*fopenline)();
#if DECEDT
	int (*fscupwind)(), (*fscdnwind)();

	fscupwind = scupwind;
	fscdnwind = scdnwind;
#endif

	fbackline = backline;
        fforwline = forwline;
	fbackchar = backchar;
	fforwchar = forwchar;
	findent   = indent;
	ftab      = tab;
	finsspace = insspace;
	fnewline  = newline;
	fopenline = openline;
#else
#if DECEDT
#define	fscupwind scupwind
#define fscdnwind scdnwind
#endif
#define	fbackline backline
#define	fforwline forwline
#define fbackchar backchar
#define fforwchar forwchar
#define findent   indent
#define ftab      tab
#define finsspace insspace
#define fnewline  newline
#define	fopenline openline
#endif

	TTflush();

	c = getcmd();
	menukey = c & 0x7F;
	if (menukey >= 'a' && menukey <= 'z') menukey += 'A' - 'a';

	f = getbind(c);

	if (f == fbackline) cmd = MENUMUP;
	else if (f == fforwline) cmd = MENUMDOWN;
#if DECEDT
	else if (f == fscupwind) cmd = MENUMUP;
	else if (f == fscdnwind) cmd = MENUMDOWN;
#endif
	else if (f == fbackchar) cmd = MENUMLEFT;
	else if (f == fforwchar || f == findent || f == ftab ||
		 f == finsspace) cmd = MENUMRIGHT;
	else if (f == fnewline || f == fopenline) cmd = MENUMENTER;
	else if (c == ' ') cmd = MENUMRIGHT;
	else if (c == '\n') cmd = MENUMENTER;
	else cmd = MENUMERR;

	return(cmd);
}

int menumatch(m)	/* check if menukey is in menu item m */
int m;
{
	char *cp;

	if (menukey >= 'A' && menukey <= 'Z')
		for(cp = menudata[m].m_text; *cp != '\0'; cp++)
			if (*cp == menukey) return(1);
	return(0);
}

menudo(r, c, m)		/* execute a menu enter command */
int r, c, m;
{
	int status;

	status = HOOK;
	if (menudata[m].m_down != 0)
		status = menupull(r, c, m);
	if (menudata[m].m_func != FPNULL)
		status = (*menudata[m].m_func)(FALSE, 1);
	return(status);
}

menuput(row, col, len, normal)		/* display a string */
int row, col, len, normal;
{
	char *str;

	str = &(vscreen[row]->v_text[col]);

	if (row > 0) normal = 1 - normal;

	TTrev(normal);

	movecursor(row, col);
	if (normal)	{
#if	COLOR
		TTforg(gbcolor);
		TTbacg(gfcolor);
#endif
#if	IBMPC
		menuwrite(row, col, str, len, gbcolor, gfcolor);
#endif
	}
	else	{
#if	COLOR
		TTforg(gfcolor);
		TTbacg(gbcolor);
#endif
#if	IBMPC
		menuwrite(row, col, str, len, gfcolor, gbcolor);
#endif
	}

	ttcol += len;

#if	IBMPC
#else
	while(len-- > 0) TTputc(*str++);

	TTrev(0);
#endif
}

int menupull(row, col, men)	/* pull down a menu */
int row, col, men;
{
	int nrows, ncols;
	int c, r, m, curmrow, status, tmprow, tmpm;
	char *save, *sp, *tp;
	register VIDEO *vp;

	/* initialize */

	++row;
	nrows = menucount(men) + 2;
	if (nrows > term.t_nrow) nrows = term.t_nrow;
	if (row + nrows > term.t_nrow) row = term.t_nrow - nrows;

	ncols = menulen(men) + 2;
	if (ncols > term.t_ncol) ncols = term.t_ncol;
	if (col + ncols > term.t_ncol) col = term.t_ncol - ncols;

	save = malloc(nrows * ncols);
	status = HOOK;

	if (save == NULL) {
		mlwrite("[No memory to pull down menu]");
		return(status);
	}

	/* pull down the menu */

	m = menudata[men].m_down;
	sp = save;
	for(r = row; r < row + nrows; r++) {
		/* save old line */
		tp = &(vscreen[r]->v_text[col]);
		for(c = 0; c < ncols; c++) *sp++ = *tp++;

		/* create new line */
		tp = &(vscreen[r]->v_text[col]);
		if (r == row) {
			*tp++ = BOXUR;
			for(c = 0; c < ncols-2; c++) *tp++ = BOXH;
			*tp = BOXUL;
		}
		else if (r == row + nrows - 1) {
			*tp++ = BOXLR;
			for(c = 0; c < ncols-2; c++) *tp++ = BOXH;
			*tp = BOXLL;
		}
		else	{
			*tp++ = BOXV;
			for(c = 0; c < ncols-2 && menudata[m].m_text[c]; c++)
				*tp++ = menudata[m].m_text[c];
			for(; c < ncols-2; c++)
				*tp++ = ' ';
			*tp = BOXV;
			m = menudata[m].m_right;
		}

		/* display new line */
		menuput(r, col, ncols, 1);
	}

	/* process keys */

	m = menudata[men].m_down;
	curmrow = row + 1;
	menuput(curmrow, col, ncols, 0);

	do {
		movecursor(curmrow, col);
		c = getmkey();
		if (c == MENUMLEFT)
			m = 0;
		else if (c == MENUMRIGHT || c == MENUMENTER)
			status = menudo(row+nrows-1, col, m);
		else if (c == MENUMUP) {
			if (menudata[m].m_left != 0) {
				menuput(curmrow, col, ncols, 1);
				m = menudata[m].m_left; --curmrow;
				menuput(curmrow, col, ncols, 0);
			}
		}
		else if (c == MENUMDOWN) {
			if (menudata[m].m_right != 0 &&
			    curmrow < row + nrows - 2) {
				menuput(curmrow, col, ncols, 1);
				m = menudata[m].m_right; curmrow++;
				menuput(curmrow, col, ncols, 0);
			}
		}
		else	{
			tmprow = row + 1;
			tmpm = menudata[men].m_down;
			while (tmpm != 0 && (!menumatch(tmpm)) &&
			  tmprow < row + nrows - 2) {
				tmpm = menudata[tmpm].m_right;
				++tmprow;
			}
			if (tmpm != 0)	{
				menuput(curmrow, col, ncols, 1);
				m = tmpm;
				curmrow = tmprow;
				menuput(curmrow, col, ncols, 0);
				status = menudo(curmrow, col, m);
			}
			else
				TTbeep();
		}
	} while (m != 0 && status == HOOK);

	/* restore the screen */

	sp = save;
	for(r = row; r < row + nrows; r++) {
		movecursor(r, col);
		if (vscreen[r]->v_flag & VFREQ) TTrev(1); else TTrev(0);
#if	COLOR
		TTforg(vscreen[r]->v_rfcolor);
		TTbacg(vscreen[r]->v_rbcolor);
#endif
#if	IBMPC
		menuwrite(r, col, sp, ncols,
			vscreen[r]->v_rfcolor, vscreen[r]->v_rbcolor);
#endif
		tp = &(vscreen[r]->v_text[col]);
		for(c = 0; c < ncols; c++) {
#if	IBMPC
#else
			TTputc(*sp);
#endif
			*tp++ = *sp++;
		}
		ttcol += ncols;
	}

	free(save);

	/* return */
	return(status);
}

menutput(col, m, normal)		/* display a string on top line */
int col, m, normal;
{
	int len;

	len = strlen(menudata[m].m_text);
	if (len >= menutwid) len = menutwid - 1;
	menuput(0, col, len, normal);
}

int menu(f, n)			/* actual menu command */
{
	int i, m, c, curmcol, wid, status, tmpcol, tmpm;

	if (!menuflag) return(TRUE);

	status = HOOK;
	m = 1;
	curmcol = 0;

	if (f == TRUE)
		while (--n > 0 && menudata[m].m_right != 0) {
			m = menudata[m].m_right;
			curmcol += menutwid;
		}
	else
		while (m != menulast && menudata[m].m_right != 0) {
			m = menudata[m].m_right;
			curmcol += menutwid;
		}

	menutput(curmcol, m, 0);

	do {
		movecursor(0, curmcol);
		c = getmkey();
		if (c == MENUMLEFT) {
			menutput(curmcol, m, 1);
			if (menudata[m].m_left != 0)
				{m = menudata[m].m_left; curmcol -= menutwid;}
			else
				while(menudata[m].m_right != 0) {
					m = menudata[m].m_right;
					curmcol += menutwid;
				}
			menutput(curmcol, m, 0);
		}
		else if (c == MENUMRIGHT) {
			menutput(curmcol, m, 1);
			if (menudata[m].m_right != 0)
				{m = menudata[m].m_right; curmcol += menutwid;}
			else
				{m = 1; curmcol = 0;}
			menutput(curmcol, m, 0);
		}
		else if (c == MENUMUP)
			status = TRUE;
		else if (c == MENUMDOWN || c == MENUMENTER)
			status = menudo(0, curmcol, m);
		else	{
			tmpcol = 0;
			tmpm = menudata[0].m_down;
			while (tmpm != 0 && !menumatch(tmpm))	{
				tmpm = menudata[tmpm].m_right;
				tmpcol += menutwid;
			}
			if (tmpm != 0)	{
				menutput(curmcol, m, 1);
				m = tmpm;
				curmcol = tmpcol;
				status = menudo(0, curmcol, m);
				menutput(curmcol, m, 0);
			}
			else
				TTbeep();
		}
	} while (status == HOOK);

	menutput(curmcol, m, 1);

	menulast = m;

	return(status);
}

#endif

