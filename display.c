/*
 * The functions in this file handle redisplay. There are two halves, the
 * ones that update the virtual display screen, and the ones that make the
 * physical display screen the same as the virtual display screen. These
 * functions use hints that are left in the windows by the commands.
 *
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

NOSHARE VIDEO   **vscreen;		/* Virtual screen. */
#if	SCROLL
NOSHARE VIDEO   **vsave;		/* Used for scrolling. */

#endif
#if	MEMMAP == 0
static VIDEO   **pscreen;	/* Physical screen. */
#if	SCROLL
static VIDEO   **psave;		/* Used for scrolling. */
#endif
#endif

NOSHARE int didlins, didldel;

static int	curcol;		/* Cursor column		*/

/* define the character mapping table */

#if DISPSEVEN | DECEDT

NOSHARE int me_char_len[256];	/* length of charname entries	*/

CONST char FAR *me_char_name[256] = {
	/*   0  0 */ "^@",
	/*   1  1 */ "^A",
	/*   2  2 */ "^B",
	/*   3  3 */ "^C",
	/*   4  4 */ "^D",
	/*   5  5 */ "^E",
	/*   6  6 */ "^F",
	/*   7  7 */ "^G",
	/*   8  8 */ "^H",
	/*   9  9 */ "^I",
	/*  10  a */ "^J",
	/*  11  b */ "^K",
	/*  12  c */ "^L",
	/*  13  d */ "^M",
	/*  14  e */ "^N",
	/*  15  f */ "^O",
	/*  16 10 */ "^P",
	/*  17 11 */ "^Q",
	/*  18 12 */ "^R",
	/*  19 13 */ "^S",
	/*  20 14 */ "^T",
	/*  21 15 */ "^U",
	/*  22 16 */ "^V",
	/*  23 17 */ "^W",
	/*  24 18 */ "^X",
	/*  25 19 */ "^Y",
	/*  26 1a */ "^Z",
	/*  27 1b */ "^[",
	/*  28 1c */ "^\\",
	/*  29 1d */ "^]",
	/*  30 1e */ "^^",
	/*  31 1f */ "^_",
	/*  32 20 */ " ",
	/*  33 21 */ "!",
	/*  34 22 */ "\"",
	/*  35 23 */ "#",
	/*  36 24 */ "$",
	/*  37 25 */ "%",
	/*  38 26 */ "&",
	/*  39 27 */ "'",
	/*  40 28 */ "(",
	/*  41 29 */ ")",
	/*  42 2a */ "*",
	/*  43 2b */ "+",
	/*  44 2c */ ",",
	/*  45 2d */ "-",
	/*  46 2e */ ".",
	/*  47 2f */ "/",
	/*  48 30 */ "0",
	/*  49 31 */ "1",
	/*  50 32 */ "2",
	/*  51 33 */ "3",
	/*  52 34 */ "4",
	/*  53 35 */ "5",
	/*  54 36 */ "6",
	/*  55 37 */ "7",
	/*  56 38 */ "8",
	/*  57 39 */ "9",
	/*  58 3a */ ":",
	/*  59 3b */ ";",
	/*  60 3c */ "<",
	/*  61 3d */ "=",
	/*  62 3e */ ">",
	/*  63 3f */ "?",
	/*  64 40 */ "@",
	/*  65 41 */ "A",
	/*  66 42 */ "B",
	/*  67 43 */ "C",
	/*  68 44 */ "D",
	/*  69 45 */ "E",
	/*  70 46 */ "F",
	/*  71 47 */ "G",
	/*  72 48 */ "H",
	/*  73 49 */ "I",
	/*  74 4a */ "J",
	/*  75 4b */ "K",
	/*  76 4c */ "L",
	/*  77 4d */ "M",
	/*  78 4e */ "N",
	/*  79 4f */ "O",
	/*  80 50 */ "P",
	/*  81 51 */ "Q",
	/*  82 52 */ "R",
	/*  83 53 */ "S",
	/*  84 54 */ "T",
	/*  85 55 */ "U",
	/*  86 56 */ "V",
	/*  87 57 */ "W",
	/*  88 58 */ "X",
	/*  89 59 */ "Y",
	/*  90 5a */ "Z",
	/*  91 5b */ "[",
	/*  92 5c */ "\\",
	/*  93 5d */ "]",
	/*  94 5e */ "^",
	/*  95 5f */ "_",
	/*  96 60 */ "`",
	/*  97 61 */ "a",
	/*  98 62 */ "b",
	/*  99 63 */ "c",
	/* 100 64 */ "d",
	/* 101 65 */ "e",
	/* 102 66 */ "f",
	/* 103 67 */ "g",
	/* 104 68 */ "h",
	/* 105 69 */ "i",
	/* 106 6a */ "j",
	/* 107 6b */ "k",
	/* 108 6c */ "l",
	/* 109 6d */ "m",
	/* 110 6e */ "n",
	/* 111 6f */ "o",
	/* 112 70 */ "p",
	/* 113 71 */ "q",
	/* 114 72 */ "r",
	/* 115 73 */ "s",
	/* 116 74 */ "t",
	/* 117 75 */ "u",
	/* 118 76 */ "v",
	/* 119 77 */ "w",
	/* 120 78 */ "x",
	/* 121 79 */ "y",
	/* 122 7a */ "z",
	/* 123 7b */ "{",
	/* 124 7c */ "|",
	/* 125 7d */ "}",
	/* 126 7e */ "~",
	/* 127 7f */ "^?",
	/* 128 80 */ "^!@",
	/* 129 81 */ "^!A",
	/* 130 82 */ "^!B",
	/* 131 83 */ "^!C",
	/* 132 84 */ "^!D",
	/* 133 85 */ "^!E",
	/* 134 86 */ "^!F",
	/* 135 87 */ "^!G",
	/* 136 88 */ "^!H",
	/* 137 89 */ "^!I",
	/* 138 8a */ "^!J",
	/* 139 8b */ "^!K",
	/* 140 8c */ "^!L",
	/* 141 8d */ "^!M",
	/* 142 8e */ "^!N",
	/* 143 8f */ "^!O",
	/* 144 90 */ "^!P",
	/* 145 91 */ "^!Q",
	/* 146 92 */ "^!R",
	/* 147 93 */ "^!S",
	/* 148 94 */ "^!T",
	/* 149 95 */ "^!U",
	/* 150 96 */ "^!V",
	/* 151 97 */ "^!W",
	/* 152 98 */ "^!X",
	/* 153 99 */ "^!Y",
	/* 154 9a */ "^!Z",
	/* 155 9b */ "^![",
	/* 156 9c */ "^!\\",
	/* 157 9d */ "^!]",
	/* 158 9e */ "^!^",
	/* 159 9f */ "^!_",
	/* 160 a0 */ "{NB space}",
	/* 161 a1 */ "{!!}",	/* inverted ! */
	/* 162 a2 */ "{cc}",	/* cent */
	/* 163 a3 */ "{LL}",	/* pound */
	/* 164 a4 */ "{ll}",	/* loz */
	/* 165 a5 */ "{y_}",	/* yen */
	/* 166 a6 */ "{||}",	/* pipe */
	/* 167 a7 */ "{SS}",	/* section */
	/* 168 a8 */ "{\"\"}",	/* uml */
	/* 169 a9 */ "{C*}",	/* (C) */
	/* 170 aa */ "{a_}",
	/* 171 ab */ "{<<}",
	/* 172 ac */ "{|_}",	/* not */
	/* 173 ad */ "{Soft Hyp}",
	/* 174 ae */ "{R*}",	/* (R) */
	/* 175 af */ "{__}",	/* macr */
	/* 176 b0 */ "{0^}",
	/* 177 b1 */ "{+-}",
	/* 178 b2 */ "{2^}",
	/* 179 b3 */ "{3^}",
	/* 180 b4 */ "{''}",	/* acute */
	/* 181 b5 */ "{m,}",	/* mu */
	/* 182 b6 */ "{PP}",	/* paragraph */
	/* 183 b7 */ "{**}",	/* small bullet */
	/* 184 b8 */ "{,,}",	/* ced */
	/* 185 b9 */ "{1^}",
	/* 186 ba */ "{o_}",
	/* 187 bb */ "{>>}",
	/* 188 bc */ "{1/4}",
	/* 189 bd */ "{1/2}",
	/* 190 be */ "{3/4}",
	/* 191 bf */ "{??}",	/* inverted ? */
	/* 192 c0 */ "{A`}",
	/* 193 c1 */ "{A'}",
	/* 194 c2 */ "{A^}",
	/* 195 c3 */ "{A~}",
	/* 196 c4 */ "{A\"}",
	/* 197 c5 */ "{A*}",
	/* 198 c6 */ "{AE}",
	/* 199 c7 */ "{C,}",
	/* 200 c8 */ "{E`}",
	/* 201 c9 */ "{E'}",
	/* 202 ca */ "{E^}",
	/* 203 cb */ "{E\"}",
	/* 204 cc */ "{I`}",
	/* 205 cd */ "{I'}",
	/* 206 ce */ "{I^}",
	/* 207 cf */ "{I\"}",
	/* 208 d0 */ "{D_}",
	/* 209 d1 */ "{N~}",
	/* 210 d2 */ "{O`}",
	/* 211 d3 */ "{O'}",
	/* 212 d4 */ "{O^}",
	/* 213 d5 */ "{O~}",
	/* 214 d6 */ "{O\"}",
	/* 215 d7 */ "{xx}",	/* cross */
	/* 216 d8 */ "{O/}",
	/* 217 d9 */ "{U`}",
	/* 218 da */ "{U'}",
	/* 219 db */ "{U^}",
	/* 220 dc */ "{U\"}",
	/* 221 dd */ "{Y'}",
	/* 222 de */ "{T_}",	/* TH */
	/* 223 df */ "{ss}",
	/* 224 e0 */ "{a`}",
	/* 225 e1 */ "{a'}",
	/* 226 e2 */ "{a^}",
	/* 227 e3 */ "{a~}",
	/* 228 e4 */ "{a\"}",
	/* 229 e5 */ "{a*}",
	/* 230 e6 */ "{ae}",
	/* 231 e7 */ "{c,}",
	/* 232 e8 */ "{e`}",
	/* 233 e9 */ "{e'}",
	/* 234 ea */ "{e^}",
	/* 235 eb */ "{e\"}",
	/* 236 ec */ "{i`}",
	/* 237 ed */ "{i'}",
	/* 238 ee */ "{i^}",
	/* 239 ef */ "{i\"}",
	/* 240 f0 */ "{d_}",
	/* 241 f1 */ "{n~}",
	/* 242 f2 */ "{o`}",
	/* 243 f3 */ "{o'}",
	/* 244 f4 */ "{o^}",
	/* 245 f5 */ "{o~}",
	/* 246 f6 */ "{o\"}",
	/* 247 f7 */ "{//}",	/* division */
	/* 248 f8 */ "{o/}",
	/* 249 f9 */ "{u`}",
	/* 250 fa */ "{u'}",
	/* 251 fb */ "{u^}",
	/* 252 fc */ "{u\"}",
	/* 253 fd */ "{y'}",
	/* 254 fe */ "{t_}",	/* th */
	/* 255 ff */ "^!?"
} ;

#endif

#define fastputc(c)	TTputc(c)

/* these declarations must match termio.c */

#if     VMS & ~WINMOD
NOSHARE extern char obuf[NOBUF];	/* Output buffer	*/
NOSHARE extern int nobuf;		/* # of bytes in above	*/

#undef fastputc
#define fastputc(c)	{if (nobuf >= NOBUF) ttflush(); obuf[nobuf++] = (c);}
#endif

#if     (V7 | USG | BSD) & ~WINMOD
#undef fastputc
#define fastputc(c)        putc((c), stdout)
#endif

#if	SCROLL & COLOR
NOSHARE extern int usedcolor;
#endif

NOSHARE extern int termflag;

static VOID updall PP((WINDOW *wp));
static VOID upddex PP((void));
static VOID updgar PP((void));
static VOID updext PP((void));
static VOID updone PP((WINDOW *wp));
static VOID updscroll PP((WINDOW *wp));
static VOID updpos PP((void));
static int updupd PP((int force));
static int reframe PP((WINDOW *wp));
static VOID mlputli PP((long l, int r));
static VOID mlputi PP((int i, int r));
static VOID vteeol PP((void));
static VOID vtmove PP((int row, int col));
static VOID vtputc PP((int c));
static VOID vtputl PP((LINE *lp));
static int vtputs PP((CONSTA char *s));
static VOID modeline PP((WINDOW *wp));

#if MEMMAP
static VOID lupdate PP((int row, VIDEO *vp1));
#else
static VOID lupdate PP((int row, VIDEO *vp1, VIDEO *vp2));
#endif

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
VOID vtinit()
{
    register int i;
    register VIDEO *vp;
    int video_size;
    static VIDEO *vpsave;
#if	MEMMAP == 0
    register VIDEO *vm;
    static VIDEO *vmsave;
#endif

#if DISPSEVEN | DECEDT
    for (i = 0; i < 256; i++) me_char_len[ i ] = (int) strlen(me_char_name[ i ]);
#endif

    TTopen();		/* open the screen */
    TTkopen();		/* open the keyboard */
    TTrev(FALSE);
    if (firstflag) {
	vscreen = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));
#if	SCROLL
	vsave = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));
#endif
    }

    if (vscreen == NULL) exit(1);
#if	SCROLL
    if (vsave == NULL) exit(1);
#endif

#if	MEMMAP == 0
    if (firstflag) {
	pscreen = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));
#if	SCROLL
	psave = (VIDEO **) malloc(term.t_mrow*sizeof(VIDEO *));
#endif
    }

    if (pscreen == NULL) exit(1);
#if	SCROLL
    if (psave == NULL) exit(1);
#endif
#endif

    video_size = sizeof(VIDEO);
    if (term.t_mcol > V_TEXT_SIZE)
        video_size = video_size + term.t_mcol - V_TEXT_SIZE;
    video_size = ((video_size - 1) | (sizeof(int) - 1)) + 1;

    if (firstflag) {
	vpsave = (VIDEO *) malloc(term.t_mrow*video_size);
#if	MEMMAP == 0
	vmsave = (VIDEO *) malloc(term.t_mrow*video_size);
#endif
    }

    vp = vpsave;
#if	MEMMAP == 0
    vm = vmsave;
#endif

    for (i = 0; i < term.t_mrow; ++i)
        {
        if (vp == NULL)
            exit(1);

	vp->v_flag = 0;
#if	COLOR
	vp->v_rfcolor = 7;
	vp->v_rbcolor = 0;
#endif
        vscreen[i] = vp;
	vp = (VIDEO *) (((char *)vp) + video_size);

#if	MEMMAP == 0
        if (vm == NULL)
            exit(1);

	vm->v_flag = 0;
	pscreen[i] = vm;
	vm = (VIDEO *) (((char *)vm) + video_size);
#endif
        }
}

/*
 * Clean up the virtual terminal system, in anticipation for a return to the
 * operating system. Move down to the last line and clear it out (the next
 * system prompt will be written in the line). Shut down the channel to the
 * terminal.
 */
VOID vttidy()
{
#if	SCROLL
    ttscroll(0, term.t_nrow, 0);
#endif
    mlerase();
    movecursor(term.t_nrow, 0);
    TTflush();
    TTclose();
    TTkclose();
}

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values; this might be a good
 * idea during the early stages.
 */
static VOID vtmove(row, col)
int row, col;
{
    vtrow = row;
    vtcol = col;
}

/* Write a character to the virtual screen. The virtual row and
   column are updated. If we are not yet on left edge, don't print
   it yet. If the line is too long put a "$" in the last column.
   This routine only puts printing characters into the virtual
   terminal buffers. Only column overflow is checked.
*/

static VOID vtputc(c)

int c;

{
	register VIDEO *vp;	/* ptr to line being updated */
#if DISPSEVEN
	CONST char *s;
#endif

	vp = vscreen[vtrow];

	if (c == '\t') {
		do {
			vtputc(' ');
		} while (((vtcol + taboff) % tabsize) != 0);
	} else if (vtcol >= term.t_ncol) {
		++vtcol;
		vp->v_text[term.t_ncol - 1] = '$';
#if DISPSEVEN
	} else if (dispseven) {
		if (c >= 0x20 && c < 0x7F) {
			if (vtcol >= 0) vp->v_text[vtcol] = (char) c;
			++vtcol;
		} else {
			s = me_char_name[ c & 0xFF ];
			while (*s != '\0') {
				if (vtcol >= term.t_ncol) {
					++vtcol;
					vp->v_text[term.t_ncol - 1] = '$';
					break;
				}
				if (vtcol >= 0) vp->v_text[vtcol] = *s;
				vtcol++;
				s++;
			}
		}
#endif
	} else if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7F ||
			(disphigh && ((c&0x80) != 0))) {
		vtputc('^');
		if (disphigh && ((c&0x80) != 0)) vtputc('!');
		vtputc((c&0x7F) ^ 0x40);
	} else {
		if (vtcol >= 0)
			vp->v_text[vtcol] = (char) c;
		++vtcol;
	}
}

/* Write a line to the virtual screen.  Does equivalent of
	for (j=0; j<llength(lp); ++j) vtputc(lgetc(lp, j));
 */

static VOID vtputl(lp)
LINE *lp;	/* line to update */
{
	register VIDEO *vp;	/* ptr to line being updated */
	register int i, c;
#if DISPSEVEN
	CONST char *s;
#endif

	vp = vscreen[vtrow];

	for (i = 0; i < llength(lp); ++i) {
		c = lgetc(lp, i);
		if (c == '\t') {
			do {
				if (vtcol >= term.t_ncol) {
					vp->v_text[term.t_ncol - 1] = '$';
					return;
				} else {
					if (vtcol >= 0)
						vp->v_text[vtcol] = ' ';
					++vtcol;
				}
			} while (((vtcol + taboff) % tabsize) != 0);
		} else if (vtcol >= term.t_ncol) {
			vp->v_text[term.t_ncol - 1] = '$';
			return;
		} else if (c < 0x20 || c >= 0x7F) {
#if DISPSEVEN
			if (dispseven) {
				s = me_char_name[ c&0xFF ];
				while (*s != '\0') {
					if (vtcol >= term.t_ncol) {
						++vtcol;
						vp->v_text[term.t_ncol-1] = '$';
						return;
					}
					if (vtcol >= 0) vp->v_text[vtcol] = *s;
					vtcol++;
					s++;
				}
				continue;	/* skip extra vtcol increment */
			}
#endif
			if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7F ||
			    (disphigh && ((c&0x80) != 0))) {
				if (vtcol >= 0) vp->v_text[vtcol] = '^';
				++vtcol;
				if (disphigh && ((c&0x80) != 0) &&
				    vtcol < term.t_ncol) {
					if (vtcol >= 0) vp->v_text[vtcol] = '!';
					++vtcol;
				}
				if (vtcol >= term.t_ncol) {
					vp->v_text[term.t_ncol - 1] = '$';
					return;
				} else if (vtcol >= 0)
					vp->v_text[vtcol] =
						(char) ((c&0x7F) ^ 0x40);
			} else if (vtcol >= 0)
				vp->v_text[vtcol] = (char) c;
			++vtcol;
		} else {
			if (vtcol >= 0)
				vp->v_text[vtcol] = (char) c;
			++vtcol;
		}
	}
}

/* Write a string to the virtual screen and return its length */

static int vtputs(s)
CONSTA char *s;
{
	register VIDEO *vp;	/* ptr to line being updated */
	register int i;

	vp = vscreen[vtrow];

	for (i = 0; *s != '\0' && vtcol < term.t_ncol; ++i) {
		if (vtcol >= 0) vp->v_text[vtcol] = *(s++);
		++vtcol;
	}
	return(i);
}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
static VOID vteeol()
{
    register VIDEO      *vp;

    vp = vscreen[vtrow];
    if (vtcol < 0) vtcol = 0;
    while (vtcol < term.t_ncol)
        vp->v_text[vtcol++] = ' ';
}

/* upscreen:	user routine to force a screen update
		always finishes complete update		*/

int upscreen(f, n)
int f, n;
{
	UNUSED_ARGS_FN;
	update(TRUE);
	return(TRUE);
}

/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 * force == HOOK like FALSE, but also does not update cursor
 *   (for edt page scrolling)
 */
int update(force)

int force;	/* force update past type ahead? */

{
	register WINDOW *wp;

#if	TYPEAH
	if (force != TRUE && typahead())
		return(TRUE);
#endif
#if	VISMAC == 0
	if (force != TRUE && kbdmode == PLAY)
		return(TRUE);
#endif

	/* update any windows that need refreshing */
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag) {
			/* if the window has changed, service it */
			reframe(wp);	/* check the framing */
			if ((wp->w_flag & ~WFMODE) == WFEDIT)
				updone(wp);	/* update EDITed line */
			else if ((wp->w_flag & ~(WFMODE | WFMOVE)) == WFSCROL)
				updscroll(wp);
			else if (wp->w_flag & ~WFMOVE)
				updall(wp);	/* update all lines */
			if (wp->w_flag & WFMODE)
				modeline(wp);	/* update modeline */
			wp->w_flag = 0;
			wp->w_force = 0;
		}
		/* on to the next window */
		wp = wp->w_wndp;
	}

	/* recalc the current hardware cursor location */
	updpos();

#if	MEMMAP
	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
#endif

	/* check for lines to de-extend */
	upddex();

	/* if screen is garbage, re-plot it */
	if (sgarbf != FALSE)
		updgar();

	/* update the virtual screen to the physical screen */
	if (updupd(force) != ABORT) {

		/* update the cursor and flush the buffers */
#if	MEMMAP | RAINBOW
#else
		/* Fast way to move the cursor a little bit */
		if (ttrow == currow &&
	            ttcol < curcol - lbound &&
	            ttcol + 5 > curcol - lbound) {
#if	COLOR
		  TTforg(vscreen[currow]->v_rfcolor);
		  TTbacg(vscreen[currow]->v_rbcolor);
#endif
	          do {fastputc(pscreen[currow]->v_text[ttcol]);++ttcol;}
	          while (ttcol < curcol - lbound);
	        }
#endif
		if (force != HOOK)
			movecursor(currow, curcol - lbound);
	}

	if (force != HOOK) TTflush();
	return(TRUE);
}

/*	reframe:	check to see if the cursor is on in the window
			and re-frame it if needed or wanted		*/

static int reframe(wp)

WINDOW *wp;

{
	register LINE *lp, *blp;
	register int i;
	int scrolled;
#if	SCROLL
	register LINE *slp;
	int top, bot, j, k;
	VIDEO *vp;
#endif

	scrolled = 0;

	blp = wp->w_bufp->b_linep;	/* last line of buffer */

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
		lp = wp->w_linep;
		for (i = 0; i < wp->w_ntrows; i++) {

			/* if the line is in the window, no reframe */
			if (lp == wp->w_dotp)
				return(TRUE);

			/* if we are at the end of the file, reframe */
			if (lp == blp)
				break;

			/* on to the next line */
			lp = lforw(lp);
		}
	}

	/* reaching here, we need a window refresh */
	i = wp->w_force;

	/* how far back to reframe? */
	if (i > 0) {		/* only one screen worth of lines max */
		if (--i >= wp->w_ntrows)
			i = wp->w_ntrows - 1;
	} else if (i < 0) {	/* negative update???? */
		i += wp->w_ntrows;
		if (i < 0)
			i = 0;
	} else
		i = wp->w_ntrows / 2;

	/* backup to new line at top of window */
	lp = wp->w_dotp;
	while (i != 0 && lback(lp) != blp) {
		--i;
		lp = lback(lp);
	}

#if	SCROLL

	if (isvt100 && (termflag&2) == 0) {
		/* backup to get end of buffer on last line */
		i = wp->w_ntrows - 2;
		if (lp != blp) {
			slp = lp;
			if (wp->w_dotp != blp)
				slp = lforw(slp);
			if (slp != blp)
				while(i > 0 && lforw(slp) != blp) {
					--i;
					slp = lforw(slp);
				}
			while(i > 0 && lback(lp) != blp) {
				--i;
				lp = lback(lp);
			}
		}
	}
	
	/* check if scrolling possible */
	if (sgarbf == FALSE && isvt100 == TRUE && wp->w_ntrows > 2 &&
			(termflag&2) == 0) {
		slp = wp->w_linep;
		i = 0;
		while (i < wp->w_ntrows && slp != lp && slp != blp)
			{++i; slp = lforw(slp);}
		if (slp != lp)	{
			slp = wp->w_linep;
			i = 0;
			while (-i < wp->w_ntrows && slp != lp && slp != blp)
				{--i; slp = lback(slp);}
		}

		scrolled = (slp == lp);

		if (slp == lp && i != 0) {
			top = wp->w_toprow + menuflag;
			bot = top + wp->w_ntrows - 1;
			ttscroll(top, bot, i);
			if (i > 0) {		/* scroll page up */
				for (j = top; j < top + i; j++) {
					vp = pscreen[j];
					psave[bot + top - j] = vp;
					vsave[bot + top - j] = vscreen[j];
					vsave[bot + top - j]->v_flag |= VFSCROL;
					for(k = 0; k < term.t_ncol; k++)
						vp->v_text[k] = ' ';
#if	COLOR
					if (usedcolor) {
						vp = vscreen[j];
						vp->v_fcolor = 7;
						vp->v_bcolor = 0;
						vp->v_flag |= VFCHG | VFCOL;
					}
#endif
				}
				for (j = top + i; j <= bot; j++) {
					psave[j - i] = pscreen[j];
					vsave[j - i] = vscreen[j];
				}
			}
			else {			/* scroll page down */
				i = -i;
				for (j = bot + 1 - i; j <= bot; j++) {
					vp = pscreen[j];
					psave[bot + top - j] = vp;
					vsave[bot + top - j] = vscreen[j];
					vsave[bot + top - j]->v_flag |= VFSCROL;
					for(k = 0; k < term.t_ncol; k++)
						vp->v_text[k] = ' ';
#if	COLOR
					if (usedcolor) {
						vp = vscreen[j];
						vp->v_fcolor = 7;
						vp->v_bcolor = 0;
						vp->v_flag |= VFCHG | VFCOL;
					}
#endif
				}
				for (j = top; j < bot + 1 - i; j++) {
					psave[j + i] = pscreen[j];
					vsave[j + i] = vscreen[j];
				}
			}
			for (j = top; j <= bot; j++) {
				pscreen[j] = psave[j];
				vscreen[j] = vsave[j];
			}
		}
	}
#endif

	/* and reset the current line at top of window */
	wp->w_linep = lp;
	if (scrolled) {
		wp->w_flag |= WFSCROL;
		if (wheadp->w_wndp == NULL) didlins = didldel = 0;
	}
	else wp->w_flag |= WFHARD;
	wp->w_flag &= ~WFFORCE;
	return(TRUE);
}

/*	updone:	update the current line	to the virtual screen		*/

static VOID updone(wp)

WINDOW *wp;	/* window to update current line in */

{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the line we want */
	lp = wp->w_linep;
	sline = wp->w_toprow + menuflag;
	while (lp != wp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	/* and update the virtual line */
	vscreen[sline]->v_flag |= VFCHG;
	vscreen[sline]->v_flag &= ~VFREQ;
	taboff = wp->w_fcol;
	vtmove(sline, -taboff);
	vtputl(lp);
#if	COLOR
	vscreen[sline]->v_rfcolor = wp->w_fcolor;
	vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
	vteeol();
	taboff = 0;
}

/*	updscroll: update scrolled lines in a window on the virtual screen */

static VOID updscroll(wp)
WINDOW *wp;	/* window to update lines in */

{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the lines, updating them */
	lp = wp->w_linep;
	sline = wp->w_toprow + menuflag;
	while (sline < wp->w_toprow + wp->w_ntrows + menuflag) {
		if (vscreen[sline]->v_flag & VFSCROL) {
			vscreen[sline]->v_flag |= VFCHG;
			vscreen[sline]->v_flag &= ~VFREQ;
			taboff = wp->w_fcol;
			vtmove(sline, -taboff);
			if (lp != wp->w_bufp->b_linep) vtputl(lp);
#if	COLOR
			vscreen[sline]->v_rfcolor = wp->w_fcolor;
			vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
			vteeol();
		}
		if (lp != wp->w_bufp->b_linep) lp = lforw(lp);
		++sline;
	}

}

/*	updall:	update all the lines in a window on the virtual screen */

static VOID updall(wp)

WINDOW *wp;	/* window to update lines in */

{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

#if MENUS
	/* menu line */
	if (menuflag)	{
		vscreen[0]->v_flag |= (VFCHG | VFREQ);
		taboff = 0;
		vtmove(0, 0);
		vtputl(menuline);
#if	COLOR
		vscreen[0]->v_rfcolor = gbcolor;
		vscreen[0]->v_rbcolor = gfcolor;
#endif
		vteeol();
	}	
#endif

	/* search down the lines, updating them */
	lp = wp->w_linep;
	sline = wp->w_toprow + menuflag;
	while (sline < wp->w_toprow + wp->w_ntrows + menuflag) {

		/* and update the virtual line */
		vscreen[sline]->v_flag |= VFCHG;
		vscreen[sline]->v_flag &= ~VFREQ;
		taboff = wp->w_fcol;
		vtmove(sline, -taboff);
		if (lp != wp->w_bufp->b_linep) {
			/* if we are not at the end */
			vtputl(lp);
			lp = lforw(lp);
		}

		/* on to the next one */
#if	COLOR
		vscreen[sline]->v_rfcolor = wp->w_fcolor;
		vscreen[sline]->v_rbcolor = wp->w_bcolor;
#endif
		vteeol();
		++sline;
	}

}

/*	updpos:	update the position of the hardware cursor and handle extended
		lines. This is the only update for simple moves.	*/

static VOID updpos()

{
	register LINE *lp;
	register int c;
	register int i;
	int j, savcol, savrow;

	/* find the current row */
	lp = curwp->w_linep;
	currow = curwp->w_toprow + menuflag;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
	}

	/* find the current column */
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		c = lgetc(lp, i++);
		if (c == '\t')
			curcol += tabsize - (curcol % tabsize);
#if DISPSEVEN
		else if (dispseven)
			curcol += me_char_len[ c & 0xFF ];
#endif
		else if (disphigh && ((c&0x80) != 0))
			curcol += 3;
		else if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7f)
			curcol += 2;
		else
			++curcol;
	}

	/* if extended, flag so and update the virtual line image */
	if (curcol < curwp->w_fcol || curcol - curwp->w_fcol >= term.t_ncol-1) {
		if (hscroll) {
			curcol -= curwp->w_fcol;
			j = hjump;
			if (j <= 0) j = 1;
			if (j > term.t_ncol-1) j = term.t_ncol-1;
			if (curcol > 0) j = -j;
			while(curcol < 0 || curcol >= term.t_ncol-1) {
				curcol += j;
				curwp->w_fcol -= j;
			}
			if (curwp->w_fcol < 0) {
				curcol += curwp->w_fcol;
				curwp->w_fcol = 0;
			}
			curwp->w_flag |= WFHARD | WFMODE;
			savcol = curcol; savrow = currow;
			updall(curwp);
			modeline(curwp);
			curwp->w_flag &= ~(WFHARD | WFMODE);
			taboff = lbound = curwp->w_fcol;
			curcol = savcol+taboff; currow = savrow;
		}
		else {
			vscreen[currow]->v_flag |= (VFEXT | VFCHG);
			updext();
		}
	} else	{
		taboff = lbound = curwp->w_fcol;
	}
}

/*	upddex:	de-extend any line that derserves it		*/

static VOID upddex()

{
	register WINDOW *wp;
	register LINE *lp;
	register int i;

	wp = wheadp;

	while (wp != NULL) {
		lp = wp->w_linep;
		i = wp->w_toprow + menuflag;

		while (i < wp->w_toprow + wp->w_ntrows + menuflag) {
			if (vscreen[i]->v_flag & VFEXT) {
				if (wp != curwp || lp != wp->w_dotp ||
				   (curcol - wp->w_fcol < term.t_ncol - 1 &&
				    curcol >= wp->w_fcol)) {
					taboff = wp->w_fcol;
					vtmove(i, -taboff);
					if (lp != wp->w_bufp->b_linep)
						vtputl(lp);
					vteeol();

					/* this line no longer is extended */
					vscreen[i]->v_flag &= ~VFEXT;
					vscreen[i]->v_flag |= VFCHG;
				}
			}
			if (lp != wp->w_bufp->b_linep) lp = lforw(lp);
			++i;
		}
		/* and onward to the next window */
		wp = wp->w_wndp;
	}
}

/*	updgar:	if the screen is garbage, clear the physical screen and
		the virtual screen and force a full update		*/

static VOID updgar()

{
	register char *txt;
	register int i,j;

	for (i = 0; i < term.t_nrow; ++i) {
		vscreen[i]->v_flag |= VFCHG;
#if	REVSTA
		vscreen[i]->v_flag &= ~VFREV;
#endif
#if	COLOR
		vscreen[i]->v_fcolor = gfcolor;
		vscreen[i]->v_bcolor = gbcolor;
#endif
#if	MEMMAP == 0
		txt = pscreen[i]->v_text;
		for (j = 0; j < term.t_ncol; ++j)
			txt[j] = ' ';
#endif
	}

#if	VMSVT | TERMCAP
	ttsetwid(term.t_ncol);
#if	SCROLL
	ttscroll(0, term.t_nrow, 0);
#endif
#if	DECEDT
	/* hack to force vt100's into keypad mode */
	if (isvt100) {
                /* Turn on the application keypad */
                fastputc('\033'); fastputc('=');

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
#endif
	movecursor(0, 0);		 /* Erase the screen. */
	(VOID) (*term.t_eeop)();
	sgarbf = FALSE;			 /* Erase-page clears */
	i = mpresf;
	mpresf = FALSE;			 /* the message area. */
#if	COLOR
	mlerase();			/* needs to be cleared if colored */
#endif
	if (i)	mlwrite("%s", mlline);
	didlins = didldel = 0;
}

int PASCAL NEAR pop(popbufp)
BUFFER *popbufp;
{
	register int index;	/* index into the current output line */
	register int llen;	/* length of the current output line */
	register int cline;	/* current screen line number */
	LINE *lp;	/* ptr to next line to display */
	int numlines;	/* remaining number of lines to display */
#if 0
	int c;		/* input character */
#endif
	int did_dash;
	int did_more;

	/* set up to scan pop up buffer */
	lp = lforw(popbufp->b_linep);
	mlerase();
	numlines = term.t_nrow-1;
	if (numlines <= 0)
		return(FALSE);
	cline = 0;
	did_more = FALSE;

	while (lp != popbufp->b_linep) {

		if (numlines <= 0) {
			/* update the virtual screen to the physical screen */
			updupd(FALSE);

			/* tell the user there is more */
			mlwrite("--- more ---");
			TTflush();

			/* and see if they want more */
#if 1
			if (get1key() != ' ') {
				upwind(); return(TRUE);
			}
#else
			if ((c=get1key()) != ' ') {
				cpending = TRUE; charpending = c;
				upwind(); return(TRUE);
			}
#endif

			/* reset the line counters */
			numlines = term.t_nrow-1;
			cline = 0;
			did_more = TRUE;
		}

		--numlines;

		/* update the virtual screen image for this one line */
		vtmove(cline, 0);
		llen = llength(lp);
		for (index = 0; index < llen; index++)
			vtputc(lp->l_text[index]);
		vteeol();
#if	COLOR
		vscreen[cline]->v_rfcolor = gfcolor;
		vscreen[cline]->v_rbcolor = gbcolor;
#endif
		vscreen[cline]->v_flag &= ~VFREQ;
		vscreen[cline++]->v_flag |= VFCHG|VFCOL;

		/* on to the next line */
		lp = lforw(lp);
	}

	did_dash = FALSE;
	while (numlines-- > 0) {
		/* add the barrier line */
		vtmove(cline, 0);

		if (did_dash == FALSE) {
			did_dash = TRUE;
			for (index = 0; index < term.t_ncol; index++)
				vtputc('-');
#if	COLOR
			vscreen[cline]->v_rfcolor = 0;
			vscreen[cline]->v_rbcolor = 7;
#endif
			vscreen[cline]->v_flag |= VFREQ;
		} else {
#if	COLOR
			vscreen[cline]->v_rfcolor = gfcolor;
			vscreen[cline]->v_rbcolor = gbcolor;
#endif
			vscreen[cline]->v_flag &= ~VFREQ;
		}

		vscreen[cline++]->v_flag |= VFCHG|VFCOL;

		vteeol();

		if (!did_more)
			break;
	}

	/* update the virtual screen to the physical screen */
	updupd(FALSE);

	/* tell the user there is more */
	mlwrite("--- end ---");
	TTflush();

#if 1
	get1key();
#else
	if ((c = get1key()) != ' ') {
		cpending = TRUE; charpending = c;
	}
#endif

	upwind();
	return(TRUE);
}

/*	updupd:	update the physical screen from the virtual screen	*/

static int updupd(force)

int force;	/* forced update flag */

{
	register VIDEO *vp1;
	register int i;
#if	SCROLL
	int n, r, j, k, cmp;

	if (didlins < 0 || didlins >= term.t_nrow) didlins = 0;
	if (didldel < 0 || didldel >= term.t_nrow) didldel = 0;
#endif

	for (i = 0; i < term.t_nrow; ++i) {
		vp1 = vscreen[i];

		/* for each line that needs to be updated*/
		if ((vp1->v_flag & VFCHG) != 0) {
#if	TYPEAH
			if (force != TRUE && typahead())
				return(ABORT);
#endif
#if	MEMMAP
			lupdate(i, vp1);
#else
#if	SCROLL
			cmp = 1;
			if (isvt100 && (didldel || didlins) &&
				(vp1->v_flag & (VFREV|VFREQ)) == 0 &&
				i+2 < term.t_nrow &&
 				(termflag&2) == 0 &&
 				(vscreen[i+1]->v_flag & VFCHG) != 0 &&
				(vscreen[i+2]->v_flag & VFCHG) != 0 &&
				(cmp=strncmp(vp1->v_text, pscreen[i]->v_text,
					term.t_ncol)) != 0) {
				/* check for inserted lines */
				n = i+1; j = 1;
				/* look for a match further down */
				if (didlins)
				  while (n < term.t_nrow &&
				    ((vscreen[n]->v_flag&(VFREQ|VFREV))==0)&&
				    strncmp(vscreen[n]->v_text,
				    	pscreen[n]->v_text, term.t_ncol)!=0 &&
				    (j=strncmp(vscreen[n]->v_text,
				    	pscreen[i]->v_text, term.t_ncol))!=0 &&
				    n-i < didlins)
				    	n++;
				r = i+1;
				n = n - i;
				/* see how many lines match */
				if (j == 0)
				  while (r+n < term.t_nrow &&
				    ((vscreen[r+n]->v_flag&(VFREQ|VFREV))==0)&&
				    !strncmp(vscreen[r+n]->v_text,
				    	pscreen[r]->v_text, term.t_ncol))
				    	r++;
				/* scroll the lines */
				if (j == 0 && r > i+1) {
					r = r + n - 1;
					ttscroll(i, r, -n);
					for (j = 0; j < n; j++) {
						psave[j] = pscreen[r-j];
						for (k=0; k<term.t_ncol; k++)
						    psave[j]->v_text[k] = ' ';
#if	COLOR
						if (usedcolor) {
						  vscreen[i+j]->v_flag |=
								VFCHG | VFCOL;
						  vscreen[i+j]->v_fcolor = 7;
						  vscreen[i+j]->v_bcolor = 0;
						  lupdate(i+j,
							vscreen[i+j],
							pscreen[r-j]);
						}
#endif
					}
					for (j = r; j >= i+n; j--) {
						pscreen[j] = pscreen[j-n];
						vscreen[j]->v_flag &=
						  ~(VFCHG | VFCOL | VFSCROL);
					}
					for (j = 0; j < n; j++)
						pscreen[i+j] = psave[j];
				}
				else if (didldel) {
				/* check for deleted lines */
				n = i+1; j = 1;
				while (n < term.t_nrow &&
				    ((vscreen[n]->v_flag&(VFREQ|VFREV))==0)&&
				    strncmp(vscreen[n]->v_text,
				    	pscreen[n]->v_text, term.t_ncol)!=0 &&
				    (j=strncmp(vscreen[i]->v_text,
				    	pscreen[n]->v_text, term.t_ncol))!=0 &&
				    n-i < didldel)
				    	n++;
				r = i+1;
				n = n - i;
				if (j == 0)
				  while (r+n < term.t_nrow &&
				    ((vscreen[r+n]->v_flag&(VFREQ|VFREV))==0)&&
				    !strncmp(vscreen[r]->v_text,
				    	pscreen[r+n]->v_text, term.t_ncol))
				    	r++;
				if (j == 0 && r > i+1) {
					r = r + n - 1;
					ttscroll(i, r, n);
					for (j = 0; j < n; j++) {
						psave[j] = pscreen[i+j];
						for (k=0; k<term.t_ncol; k++)
						    psave[j]->v_text[k] = ' ';
#if	COLOR
						if (usedcolor) {
						  vscreen[r-j]->v_flag |=
								VFCHG | VFCOL;
						  vscreen[r-j]->v_fcolor = 7;
						  vscreen[r-j]->v_bcolor = 0;
						  lupdate(r-j,
							vscreen[r-j],
							pscreen[i+j]);
						}
#endif
					}
					for (j = i; j <= r-n; j++) {
						pscreen[j] = pscreen[j+n];
						vscreen[j]->v_flag &=
						  ~(VFCHG | VFCOL | VFSCROL);
					}
					for (j = 0; j < n; j++)
						pscreen[r-j] = psave[j];
				}
				}
			}
			if (cmp) lupdate(i, vp1, pscreen[i]);
			else vp1->v_flag &= ~(VFCHG | VFCOL | VFSCROL);
#else
			lupdate(i, vp1, pscreen[i]);
#endif
#endif
		}
	}
	didlins = didldel = 0;
	return(TRUE);
}

/*	updext: update the extended line which the cursor is currently
		on at a column greater than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is
								*/

static VOID updext()

{
	register int rcursor;	/* real cursor location */
	register LINE *lp;	/* pointer to current line */
	int islong;

	/* calculate what column the real cursor will end up in */
	islong = (curcol >= curwp->w_fcol);
	if (islong)
		rcursor = (curcol - curwp->w_fcol - term.t_ncol) % term.t_scrsiz
				+ term.t_margin + 1;
	else	{
		rcursor = term.t_scrsiz -
				((curwp->w_fcol - curcol) % term.t_scrsiz);
		if (rcursor > curcol) rcursor = curcol;
	}
	taboff = lbound = curcol - rcursor;

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge					*/
	vtmove(currow, -taboff);	/* start scanning offscreen */
	lp = curwp->w_dotp;		/* line to output */
	vtputl(lp);

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	/* and put a '$' in column 1 */
	if (islong)
		vscreen[currow]->v_text[0] = '$';
}

/*
 * Update a single line. This does not know how to use insert or delete
 * character sequences; we are using VT52 functionality. Update the physical
 * row and column variables. It does try an exploit erase to end of line. The
 * RAINBOW version of this routine uses fast video.
 */
#if	MEMMAP
/*	lupdate specific code for the IBM-PC and other compatables */

static VOID lupdate(row, vp1)

int row;		/* row of screen to update */
VIDEO *vp1;		/* virtual screen image */

{
#if	COLOR
	scwrite(row, vp1->v_text, vp1->v_rfcolor, vp1->v_rbcolor);
	vp1->v_fcolor = vp1->v_rfcolor;
	vp1->v_bcolor = vp1->v_rbcolor;
#else
	if (vp1->v_flag & VFREQ)
		scwrite(row, vp1->v_text, 0, 7);
	else
		scwrite(row, vp1->v_text, 7, 0);
#endif
	/* flag this line as changed */
	vp1->v_flag &= ~(VFCHG | VFCOL | VFSCROL);

}

#else

static VOID lupdate(row, vp1, vp2)

int row;		/* row of screen to update */
VIDEO *vp1;		/* virtual screen image */
VIDEO *vp2;		/* physical screen image */

{
#if RAINBOW
/*	lupdate specific code for the DEC rainbow 100 micro	*/

    register char *cp1;
    register char *cp2;
    register int nch;

    /* since we don't know how to make the rainbow do this, turn it off */
    flags &= (~VFREV & ~VFREQ);

    cp1 = &vp1->v_text[0];                    /* Use fast video. */
    cp2 = &vp2->v_text[0];
    putline(row+1, 1, cp1);
    nch = term.t_ncol;

    do
        {
        *cp2 = *cp1;
        ++cp2;
        ++cp1;
        }
    while (--nch);
    *flags &= ~(VFCHG | VFSCROL);
#else
/*	lupdate code for all other versions		*/

	register char *cp1;
	register char *cp2;
	register char *cp3;
	register char *cp4;
	register char *cp5;
	register int nbflag;	/* non-blanks to the right flag? */
	int rev;		/* reverse video flag */
	int req;		/* reverse video request flag */


	/* set up pointers to virtual and physical lines */
	cp1 = &vp1->v_text[0];
	cp2 = &vp2->v_text[0];

#if	COLOR
	TTforg(vp1->v_rfcolor);
	TTbacg(vp1->v_rbcolor);
#endif

#if	REVSTA | COLOR
	/* if we need to change the reverse video status of the
	   current line, we need to re-write the entire line     */
	rev = (vp1->v_flag & VFREV) == VFREV;
	req = (vp1->v_flag & VFREQ) == VFREQ;
	if ((rev != req)
#if	COLOR
	    || (vp1->v_fcolor != vp1->v_rfcolor) || (vp1->v_bcolor != vp1->v_rbcolor)
#endif
#if	HP150
	/* the HP150 has some reverse video problems */
	    || req || rev
#endif
			) {
		movecursor(row, 0);	/* Go to start of line. */
		/* set rev video if needed */
		if (rev != req)
			(VOID) (*term.t_rev)(req);

		/* scan through the line and dump it to the screen and
		   the virtual screen array				*/
		cp3 = &vp1->v_text[term.t_ncol];
		while (cp1 < cp3) {
			fastputc(*cp1);
			++ttcol;
			*cp2++ = *cp1++;
		}
		/* turn rev video off */
		if (rev != req)
			(VOID) (*term.t_rev)(FALSE);

		/* update the needed flags */
		vp1->v_flag &= ~(VFCHG | VFSCROL);
		if (req)
			vp1->v_flag |= VFREV;
		else
			vp1->v_flag &= ~VFREV;
#if	COLOR
		vp1->v_fcolor = vp1->v_rfcolor;
		vp1->v_bcolor = vp1->v_rbcolor;
#endif
		return;
	}
#endif

	/* advance past any common chars at the left */
	while (cp1 != &vp1->v_text[term.t_ncol] && cp1[0] == cp2[0]) {
		++cp1;
		++cp2;
	}

/* This can still happen, even though we only call this routine on changed
 * lines. A hard update is always done when a line splits, a massive
 * change is done, or a buffer is displayed twice. This optimizes out most
 * of the excess updating. A lot of computes are used, but these tend to
 * be hard operations that do a lot of update, so I don't really care.
 */
	/* if both lines are the same, no update needs to be done */
	if (cp1 == &vp1->v_text[term.t_ncol]) {
 		vp1->v_flag &= ~(VFCHG | VFSCROL);
		return;			/* signal that nothing was done */
	}

	/* find out if there is a match on the right */
	nbflag = FALSE;
	cp3 = &vp1->v_text[term.t_ncol];
	cp4 = &vp2->v_text[term.t_ncol];

	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (cp3[0] != ' ')		/* Note if any nonblank */
			nbflag = TRUE;		/* in right match. */
	}

	cp5 = cp3;

	/* Erase to EOL ? */
	if (nbflag == FALSE && eolexist == TRUE
#if	REVSTA | COLOR
		&& (req != TRUE)
#endif
		) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;

		if (cp3-cp5 <= 3)		/* Use only if erase is */
			cp5 = cp3;		/* fewer characters. */
	}

	movecursor(row, (int)(cp1 - &vp1->v_text[0]));

#if	REVSTA
	TTrev(rev);
#endif

	while (cp1 != cp5) {		/* Ordinary. */
		fastputc(*cp1);
		++ttcol;
		*cp2++ = *cp1++;
	}

	if (cp5 != cp3) {		/* Erase. */
		TTeeol();
		while (cp1 != cp3)
			*cp2++ = *cp1++;
	}
#if	REVSTA
	TTrev(FALSE);
#endif
	vp1->v_flag &= ~(VFCHG | VFSCROL);	/* flag this line as updated */
	return;
#endif
}
#endif

/*
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
static VOID modeline(wp)
    WINDOW *wp;
{
    register char *cp;
    register int c;
    register int n;		/* cursor position count */
    register BUFFER *bp;
    register int i;		/* loop index */
    register int lchar;		/* character to draw line in buffer with */
    register int firstm;	/* is this the first mode? */

    n = wp->w_toprow + wp->w_ntrows + menuflag;	/* Location. */

    vscreen[n]->v_flag |= VFCHG | VFREQ | VFCOL;/* Redraw next time. */
#if	COLOR
    vscreen[n]->v_rfcolor = 0;			/* black on */
    vscreen[n]->v_rbcolor = 7;			/* white.....*/
#endif
    taboff = 0;
    vtmove(n, 0);                       	/* Seek to right line. */
    if (wp == curwp)				/* mark the current buffer */
	lchar = '=';
    else
#if	REVSTA
	if (revexist)
		lchar = ' ';
	else
#endif
		lchar = '-';

    vtputc(lchar);
    bp = wp->w_bufp;

    if ((bp->b_flag&BFCHG) != 0)                /* "*" if changed. */
        vtputc('*');
    else
        vtputc(lchar);

    if ((bp->b_flag&BFTRUNC) != 0)                /* "#" if truncated. */
        vtputc('#');
    else
        vtputc(lchar);

    if ((bp->b_flag&BFNAROW) != 0)                /* "<>" if narrowed. */
        { vtputc('<'); vtputc('>'); }
    else
        { vtputc(lchar); vtputc(lchar); }

    n  = 11;	/* =,#,*,<,>, , , ,(,),  */
    vtputc(' ');				/* Buffer name. */
    n += vtputs(PROGNAME);
    vtputc(' ');
    n += vtputs(VERSION);
    vtputc(' ');

    /* are we horizontally scrolled? */
    if (wp->w_fcol > 0) {
    	vtputc('[');
    	vtputc('<');
    	n += 4 + vtputs(me_itoa(wp->w_fcol));
    	vtputc(']');
    	vtputc(' ');
    }
    vtputc('(');

    /* display the modes */

    firstm = TRUE;
    for (i = 0; i < NUMMODES; i++)	/* add in the mode flags */
	if (wp->w_bufp->b_mode & (1 << i)) {
		if (firstm != TRUE)
			{++n; vtputc(' ');}
		firstm = FALSE;
		n += vtputs(modename[i]);
	}
    vtputc(')');
    vtputc(' ');

#if 0
    vtputc(lchar);
    vtputc((wp->w_flag&WFCOLR) != 0  ? 'C' : lchar);
    vtputc((wp->w_flag&WFMODE) != 0  ? 'M' : lchar);
    vtputc((wp->w_flag&WFHARD) != 0  ? 'H' : lchar);
    vtputc((wp->w_flag&WFEDIT) != 0  ? 'E' : lchar);
    vtputc((wp->w_flag&WFMOVE) != 0  ? 'V' : lchar);
    vtputc((wp->w_flag&WFFORCE) != 0 ? 'F' : lchar);
    vtputc(lchar);
    n += 8;
#endif

    vtputc(lchar);
    vtputc(lchar);
    vtputc(' ');
    n += 3;
    cp = &bp->b_bname[0];

    while ((c = *cp++) != 0)
        {
        vtputc(c);
        ++n;
        }

    vtputc(' ');
    vtputc(lchar);
    vtputc(lchar);
    n += 3;

    if (bp->b_fname[0] != 0)            /* File name. */
        {
	vtputc(' ');
	n += 2 + vtputs("File: ");

        cp = &bp->b_fname[0];

        while ((c = *cp++) != 0)
            {
            vtputc(c);
            ++n;
            }

	vtputc(' ');
        }

    while (n < term.t_ncol)             /* Pad to full width. */
        {
        vtputc(lchar);
        ++n;
        }
}

VOID upmode()	/* update all the mode lines */

{
	register WINDOW *wp;

	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}
}

VOID upwind()	/* force hard updates on all windows */
{
	register WINDOW *wp;

	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFHARD | WFMODE;
		wp = wp->w_wndp;
	}
}

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
VOID movecursor(row, col)
int row, col;
{
    if (row!=ttrow || col!=ttcol)
        {
        ttrow = row;
        ttcol = col;
        TTmove(row, col);
        }
}

/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
VOID mlerase()
{
    int i;
    
    movecursor(term.t_nrow, 0);

    if (discmd == FALSE)
	return;

#if	COLOR
    TTforg(7);
    TTbacg(0);
#endif
    if (eolexist == TRUE)
	    TTeeol();
    else {
        for (i = 0; i < term.t_ncol - 1; i++)
            fastputc(' ');
        movecursor(term.t_nrow, 1);	/* force the move! */
        movecursor(term.t_nrow, 0);
    }
    TTflush();
    mpresf = FALSE;
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Assumes the
 * stack grows down; this assumption is made by the "++" in the argument scan
 * loop. Set the "message line" flag TRUE.
 */


#if defined(__GNUC__) || defined(__hpux) || (defined(vms) && !defined(vax11c)) || defined(_SCO_DS)
#   define HASVA 2
#elif defined(mips) || defined(sparc) || ZORTECH || defined(_AIX) || defined(m88k)
#   define HASVA 1
#else
#   define HASVA 0
#endif

#if HASVA
#if HASVA == 1
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#endif

/*VARARGS*/

#if HASVA
#if HASVA == 1
VOID mlwrite(fmt, va_alist)
    CONSTA char *fmt;	/* format string for output */
    va_dcl		/* pointer to first argument to print */
#else
VOID mlwrite(CONSTA char *fmt, ...)
			/* format string for output */
#endif
#else
VOID mlwrite(fmt, arg)
    CONSTA char *fmt;	/* format string for output */
    char *arg;		/* pointer to first argument to print */
#endif
{
    register int c;	/* current char in format string */
    char *s;
    register char *ap;	/* ptr to current data field */
    int i, f;
#if DISPSEVEN
    CONST char *char_name;
#endif
#if HASVA
    va_list arg;	/* ptr to current data field */
#endif

    /* if we are not currently echoing the command line, abort this */

    if (discmd == FALSE) {
	movecursor(term.t_nrow, 0);
	return;
    }

#if	COLOR
    TTforg(7);
    TTbacg(0);
#endif

    /* if we can not erase to end-of-line, do it manually */

    if (eolexist == FALSE && mpresf == TRUE) {
        mlerase();
        TTflush();
    }

    mllen = 0;
    movecursor(term.t_nrow, 0);
#if HASVA
#if HASVA == 1
    va_start(arg);
#else
    va_start(arg, fmt);
#endif
#else
    ap = (char *) &arg;
#endif
    while ((c = *fmt++) != 0 && mllen < term.t_ncol && mllen < NSTRING-8) {
        if (c != '%') {
            if (c < ' ') c = ' ';
            mlline[mllen++] = (char) c;
            }
        else
            {
            c = *fmt++;
            switch (c) {
                case 'd':
#if HASVA
                    mlputi(va_arg(arg, int), 10);
#else
                    mlputi(*(int *)ap, 10);
                    ap += sizeof(int);
#endif
                    break;

                case 'o':
#if HASVA
                    mlputi(va_arg(arg, int), 8);
#else
                    mlputi(*(int *)ap,  8);
                    ap += sizeof(int);
#endif
                    break;

                case 'x':
#if HASVA
                    mlputi(va_arg(arg, int), 16);
#else
                    mlputi(*(int *)ap, 16);
                    ap += sizeof(int);
#endif
                    break;

                case 'D':
#if HASVA
                    mlputli(va_arg(arg, long), 10);
#else
                    mlputli(*(long *)ap, 10);
                    ap += sizeof(long);
#endif
                    break;

                case 's':
#if HASVA
                    s = va_arg(arg, char *);
#else
                    s = *(char **)ap;
                    ap += sizeof(char *);
#endif
#if DISPSEVEN
		    if (dispseven) {
			while (*s != '\0' && mllen < NSTRING-2) {
			    char_name = me_char_name[ *s++ & 0xFF ];
			    while (*char_name != '\0' && mllen < NSTRING-2) {
				mlline[mllen++] = *char_name++;
			    }
			}
		    } else
#endif
		    {
			while (*s != '\0' && mllen < NSTRING-2)
			    mlline[mllen++] = *s++;
		    }
		    break;

		case 'f':
		    /* break it up */
#if HASVA
                    i = va_arg(arg, int);
#else
		    i = *(int *) ap;
		    ap += sizeof(int);
#endif
		    f = i % 100;

		    /* send out the integer portion */
		    mlputi(i / 100, 10);
		    mlline[mllen++] = '.';
		    mlline[mllen++] = (char) ((f / 10) + '0');
		    mlline[mllen++] = (char) ((f % 10) + '0');

		    break;

                default:
		    mlline[mllen++] = (char) c;
                }
            }
        }

#if HASVA
    va_end(arg);
#endif

    if (mllen >= term.t_ncol) mllen = term.t_ncol - 1;
    mlline[mllen] = '\0';
    ttcol += mllen;
    for(ap = mlline; *ap; ap++)
	fastputc(*ap);

    /* if we can, erase to the end of screen */

    if (eolexist == TRUE && mpresf == TRUE)
        TTeeol();
    TTflush();
    mpresf = ((mllen > 0)? TRUE: FALSE);
}

/*
 * Force a string out to the message line regardless of the
 * current $discmd setting. This is needed when $debug is TRUE
 * and for the write-message and clear-message-line commands
 */
VOID mlforce(s)
	CONSTA char *s;	/* string to force out */
{
	register int oldcmd;	/* original command display flag */

	oldcmd = discmd;	/* save the discmd value */
	discmd = TRUE;		/* and turn display on */
	mlwrite(s);		/* write the string out */
	discmd = oldcmd;	/* and restore the original setting */
}

/*
 * Write out a string. Update the physical cursor position. This assumes that
 * the characters in the string all have width "1"; if this is not the case
 * things will get screwed up a little.
 */
VOID mlputs(s)
    CONSTA char *s;
{
    register int c;

    while ((c = *s++) != 0)
        {
        fastputc(c);
        ++ttcol;
        }
}

/*
 * do the same except as a long integer.
 */

#ifdef NO_PROTOTYPE
static VOID mlputli(l, r)
    long l;
    int r;
#else
static VOID mlputli(long l, int r)
#endif
{
    register long q;
    int digit;

    if (l < 0)
        {
        l = -l;
        mlline[mllen++] = '-';
        }

    q = l/r;

    if (q != 0)
        mlputli(q, r);

    digit = (int)(l%r);
    mlline[mllen++] = (char) ((digit < 10)? '0'+digit: 'A'+digit-10);
}

/*
 * Write out an integer, in the specified radix using mlputli.
 */
static VOID mlputi(i, r)
int i, r;
{
	mlputli( (long) i, r);
}

#if RAINBOW

putline(row, col, buf)
    int row, col;
    char buf[];
    {
    int n;

    n = strlen(buf);
    if (col + n - 1 > term.t_ncol)
        n = term.t_ncol - col + 1;
    Put_Data(row, col, n, buf);
    }
#endif

