/*	wmod:	winmod video driver
		for MicroEMACS
*/

#ifndef termdef
#define	termdef	1			/* don't define "term" external */
#endif

#include <stdio.h>

#ifndef USEWINMOD
#define USEWINMOD 1
#endif

#include "p2c.h"
#include "windecs.h"
#include "popup.h"
#include "estruct.h"
#include "edef.h"

int module_emacs = 0;		/* help linking */

/*
**  The following variable is set true in "mespawn.c" while we are
**  spawning a sub-process.  This allows us to distinguish between
**  the close needed to spawn, and the close when we are really done.
*/
extern int spawning;


static int wmodopen(void), wmodclose(void), wmodkopen(void), wmodkclose(void), wmodgetc(void), wmodputc(int c), wmodflush(void), wmodcres(char *res);
static VOID wmodmove(int row, int col), wmodeol(void), wmodeop(void), wmodbeep(void), wmodrev(int state);
#if	COLOR
static VOID wmodfcol(int color), wmodbcol(int color);
#endif


NOSHARE extern int wmodchanged;		/* declared in eval.c */

NOSHARE extern int termflag;

NOSHARE TERM FAR term = {
	WIN_MAX_LEN,
	WIN_STD_LEN,
	WIN_MAX_WID,
	WIN_STD_WID,
	8,	/* margin */
	64,	/* scroll size */
	10,	/* times to pause through update */
        wmodopen,
        wmodclose,
        wmodkopen,
        wmodkclose,
        wmodgetc,
        wmodputc,
        wmodflush,
        wmodmove,
        wmodeol,
        wmodeop,
        wmodbeep,
        wmodrev,
        wmodcres
#if	COLOR
	, wmodfcol,
	wmodbcol
#endif
};

static win_rec *w;
static int need_win_finish = 0;
static int first_repaint = 1;

static win_attr cur_color = WIN_NORMAL;

static boolean map_control_z = FALSE;
static boolean map_null = FALSE;


#if	COLOR
static int cfcolor, cbcolor;	/* WINMOD colors, not cname index */

NOSHARE int usedcolor = 1;	/* true if used a color */

static int color_map[8] = {	/* convert cname index to WINMOD color */
	WIN_FOR_BLACK,
	WIN_FOR_RED,
	WIN_FOR_GREEN,
	WIN_FOR_YELLOW,
	WIN_FOR_BLUE,
	WIN_FOR_MAGENTA,
	WIN_FOR_CYAN,
	WIN_FOR_WHITE
};
#endif

static char *termgeom = (char *)NULL;

static int last_row = 0;
static int last_col = 0;

#define	ROW	0
#define	COL	1
#define	LEN	2
#define	WID	3

#define	MIN_WID	5
#define	MIN_LEN	5


int wmodinit(argc, argv)
int argc;
char *argv[];
{
	int i;
	char *s;

	termgeom = (char *)NULL;

	for(i = 0; i < argc; i++) {
		s = argv[i];
		if (s[0] == '-' && (s[1] == 't' || s[1] == 'T')) {
			while (*s != '\0' && *s != 'g' && *s != 'G')
				s++;
			if (*s != '\0') {
				termgeom = s+1;
				return(0);
			}
		}
	}
	return(0);
}

static void wmodsides(sp, sides, value)
char **sp;
int sides[WIN_BORDER_TYPES];
int value;
{
	char *s;

	for(s = *sp; *s != '\0'; s++) {
		if (*s == 'l' || *s == 'L')
			sides[WIN_BORDER_LEFT] = value;
		else if (*s == 'r' || *s == 'R')
			sides[WIN_BORDER_RIGHT] = value;
		else if (*s == 't' || *s == 'T')
			sides[WIN_BORDER_TOP] = value;
		else if (*s == 'b' || *s == 'B')
			sides[WIN_BORDER_BOT] = value;
		else
			break;
	}
	*sp = s;	
}

static win_attr border_color, shadow_color;
static int has_border;

static int border_type[WIN_BORDER_TYPES];
static int shadow_type[WIN_BORDER_TYPES];

static int wmodopen( void )
{
	int i;
	int b2;
	int term_type;
	int geom[4];	/* col, row, len, wid */
	char *s;

        if ( spawning ) {
        	win_restore_terminal(w);
        	return TRUE;
        }

	vt100keys = 1;
	isvt100 = TRUE;

	first_repaint = 1;

	border_color = WIN_NORMAL;
	shadow_color = WIN_REVERSE;

	need_win_finish = !win_init_called(&w);

	if (need_win_finish) {
		/* i = -1; */
		/* win_init(&w, &i, &i, NULLTERTYPE, -1, -1, TRUE); */
		win_init_param_struct win_init_params = DEFAULT_WIN_INIT_PARAMS;
		win_init(&w, &win_init_params);

		win_open( &w, WINDOW_FILLS_INNER_SCREEN, 
                          WIN_NORMAL, "main", 4, WIN_REVERSE, WIN_REVERSE,
                          TRUE, WIN_NORMAL );
	}

	has_border = 1;

	s = termgeom;

	for(i = 0; i < WIN_BORDER_TYPES; i++) {
		border_type[i] = WIN_LINE_SINGLE;
		shadow_type[i] = FALSE;
	}

	for(i = 0; i < 4; i++) {
		while (s && *s && (*s < '0' || *s > '9')) {
			if (*s == 'o' || *s == 'O') {
				has_border = 0;
				s++;
			} else if (*s == 'b' || *s == 'B') {
				s++;
				has_border = 1;
				wmodsides(&s, border_type, WIN_LINE_SINGLE);
			} else if (*s == 'd' || *s == 'D') {
				s++;
				has_border = 1;
				wmodsides(&s, border_type, WIN_LINE_DOUBLE);
			} else if (*s == 's' || *s == 'S') {
				s++;
				wmodsides(&s, shadow_type, TRUE);
			} else {
				s++;
			}
		}
		geom[i] = 0;
		while (s && *s >= '0' && *s <= '9') {
			geom[i] = geom[i] * 10 + *s - '0';
			s++;
		}
		if (geom[i] < 0) geom[i] = 0;
		if (geom[i] > 1000) geom[i] = 1000;
	}

	b2 = 2 * has_border;

	if (geom[COL] <= 0)
		geom[COL] = 1;
	if (geom[COL] + MIN_WID + b2 >= WIN_STD_WID)
		geom[COL] = WIN_STD_WID - 1 - MIN_WID - b2;
	if (geom[WID] <= 0)
		geom[WID] = WIN_STD_WID;
	if (geom[WID] < MIN_WID)
		geom[WID] = MIN_WID;
	if (geom[COL] + geom[WID] > WIN_STD_WID + 1)
		geom[WID] = WIN_STD_WID + 1 - geom[COL];

	if (geom[ROW] <= 0)
		geom[ROW] = 1;
	if (geom[ROW] + MIN_LEN + b2 >= WIN_STD_LEN)
		geom[ROW] = WIN_STD_LEN - 1 - MIN_LEN - b2;
	if (geom[LEN] <= 0)
		geom[LEN] = WIN_STD_LEN;
	if (geom[LEN] < MIN_LEN)
		geom[LEN] = MIN_LEN;
	if (geom[ROW] + geom[LEN] > WIN_STD_LEN + 1)
		geom[LEN] = WIN_STD_LEN + 1 - geom[ROW];

	term.t_nrow = geom[LEN] - 1 - b2;
	term.t_ncol = geom[WID] - b2;

	term.t_margin = (term.t_ncol + 9) / 10;
	term.t_scrsiz = (term.t_ncol * 4 + 1) / 5;

	i = win_open( &w, INNER_COORDINATES( geom[ROW], geom[COL] ),
                          geom[LEN], geom[WID],
                          WIN_NORMAL, wintitle, strlen(wintitle),
                          border_color, border_color,
                          has_border, border_color );

	if (i == -1) popup_string( w, "Unable to open window!" );

#if	COLOR
	cfcolor = WIN_FOR_WHITE;
	cbcolor = WIN_FOR_BLACK;
#endif

	if (has_border) {
		win_set_border(w, border_type, border_color);
	}

	win_set_shadow(w, shadow_type, shadow_color);

	term_type = win_term_type(w);
	map_control_z = (term_type == XWINDOW);
	map_null = (term_type == VT100 || term_type == CONSOLE ||
			(term_type >= VT100FIRST && term_type <= VT100LAST));

	revexist = TRUE;
	eolexist = TRUE;

	return TRUE;
}

static int wmodclose( void )
{
        if ( spawning ) {
        	win_reset_terminal(w);
        	return TRUE;
        }

	if (need_win_finish) {
		win_finish( &w );
	} else {
		win_close_top_window( w );
	}
        return TRUE;
}

static int wmodkopen()
{
	strcpy(sres, "NORMAL");
	return TRUE;
}

static int wmodkclose()
{
	return TRUE;
}

static int wmodflush()
{
	int scr_row, scr_col;

	if (wmodchanged) {
		if (bfcolor >= 0 && bfcolor < 8 &&
		    bbcolor >= 0 && bbcolor < 8) {
			border_color =
				WIN_MAKE_COLOR(color_map[bfcolor],
					color_map[bbcolor]);
		}
		if (has_border) {
			win_set_border(w, border_type, border_color);
			win_set_title(w, wintitle, strlen(wintitle));
			win_set_title_color(w, border_color);
		}

		if (sfcolor >= 0 && sfcolor < 8 &&
		    sbcolor >= 0 && sbcolor < 8) {
			shadow_color =
				WIN_MAKE_COLOR(color_map[sbcolor],
						color_map[sfcolor]);
			win_set_shadow(w, shadow_type, shadow_color);
		}
	}

	win_update(w);
	win_rc_win_to_scr(w, last_row, last_col, &scr_row, &scr_col);
	win_scr_set(w, scr_row, scr_col);
	win_flush(w);
	return TRUE;
}

VOID wmodmove(row, col)
int row, col;
{
	last_row = row;
	last_col = col;
	win_move(w, row+1, col+1);
}

VOID wmodeol()
{
	if (last_col < term.t_ncol) {
		win_chr(w, ' ', term.t_ncol - last_col);
		wmodmove(last_row, last_col);
	}
}

VOID wmodeop()
{
#if	COLOR
	wmodfcol(gfcolor);
	wmodbcol(gbcolor);
#endif
	win_clr(w);
	if (first_repaint) {
		first_repaint = 0;
	} else {
		win_repaint(w);
	}
}

VOID wmodrev(state)	/* change reverse video status */
int state;		/* FALSE = normal video, TRUE = reverse video */
{
	cur_color = 
		(win_attr) (state? WIN_REVERSE:
#if COLOR
			WIN_MAKE_COLOR(cfcolor, cbcolor)
#else
			WIN_NORMAL
#endif
			);
	win_set_color(w, cur_color);
}

static int wmodcres(char *res)	/* change screen resolution */
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
static VOID wmodfcol(color)	/* set the current output color */
int color;
{
	if (color >= 0 && color < 8) {
		cfcolor = color_map[ color ];
		cur_color = WIN_MAKE_COLOR(cfcolor, cbcolor);
		win_set_color(w, cur_color);
	}
}

static VOID wmodbcol(color)	/* set the current background color */
int color;
{
	if (color >= 0 && color < 8) {
		cbcolor = color_map[ color ];
		cur_color = WIN_MAKE_COLOR(cfcolor, cbcolor);
		win_set_color(w, cur_color);
	}
}
#endif

static VOID wmodbeep()
{
	win_bell(w);
}

static int wmodgetc()
{
	int ch;

        /* Turn on the cursor while waiting for the event */
        win_setup_cursor( w, WIN_CURSOR_BLOCK, 1, WIN_MONO_REVERSE, TRUE );
        win_update( w );
        win_setup_current_window_input( w );
        win_flush( w );

#if WIN_USE_KB
	ch = kb_read_byte();
#else
	ch = win_read_char(w);
#endif

	/* On some terminal types, kb maps ESC to ^Z so that ESC does an exit */
	/* Reverse this mapping here because in emacs, ESC isn't an exit */

	if (ch == 26 && map_control_z) ch = 27;

	/* In order to capture interrupts, kb does not place some terminals */
	/* in full raw mode. */
	/* ^C causes an interrupt which kb converts to a null. */
	/* Convert the nulls back into a ^C. */
	/* emacs needs ^C more than ^@. */
	/* ^@ sets the mark. */
	/* ^X^C is the way to leave emacs without saving the file. */

	if (ch == 0 && map_null) ch = 3;

        /* Turn off the cursor while handling the event */
        win_remove_cursor( w );
	return ch;
}

static int wmodputc(ch)
int ch;
{
	if (ch == '\b') {
		wmodmove(last_row, --last_col);
	} else {
		win_chr(w, ch, 1);
		++last_col;
	}
	return ch;
}

/* Scroll 0-based in current window */

VOID ttscroll(rowa, rowb, lines)
int rowa, rowb, lines;
{
	win_set_color(w, WIN_NORMAL);
	win_scroll(w, rowa + 1, 1, rowb - rowa + 1, term.t_ncol, lines, ' ');
	win_set_color(w, cur_color);
}

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif

