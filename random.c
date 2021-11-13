/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

static int killtxt PP((int f, int n, int delflag));
static int adjustmode PP((int kind, int global));
static VOID setcindent PP((int target));
static int cinsert PP((void));
static int getcindent PP((void));
static int getpindent PP((void));

/*
 * Set fill column to n.
 */
int setfillcol(f, n)
int f, n;
{
        fillcol = n;
	mlwrite("[Fill column is %d]",n);
        return(TRUE);
}

/*
 * Display the current position of the cursor, in origin 1 X-Y coordinates,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 * Normally this is bound to "C-X =".
 */
int showcpos(f, n)
int f, n;
{
        register LINE   *lp;		/* current line */
        register long   numchars;	/* # of chars in file */
        register int	numlines;	/* # of lines in file */
        register long   predchars;	/* # chars preceding point */
        register int	predlines;	/* # lines preceding point */
        register int    curchar;	/* character under cursor */
        int ratio;
        int ratio_decimal1;
        int ratio_decimal2;
        int col;
	int savepos;			/* temp save for current offset */
	int ecol;			/* column pos/end of current line */

	/* starting at the beginning of the buffer */
        lp = lforw(curbp->b_linep);

	/* start counting chars and lines */
        numchars = 0;
        numlines = 0;
        predchars = 0;
        predlines = 0;
        curchar = '\n';
        while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp) {
			predlines = numlines;
			predchars = numchars + curwp->w_doto;
			if ((curwp->w_doto) == llength(lp))
				curchar = '\n';
			else
				curchar = lgetc(lp, curwp->w_doto);
		}
		/* on to the next line */
		++numlines;
		numchars += llength(lp) + 1;
		lp = lforw(lp);
        }

	/* if at end of file, record it */
	if (curwp->w_dotp == curbp->b_linep) {
		predlines = numlines;
		predchars = numchars;
		curchar = '\n';
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = curwp->w_doto;
	curwp->w_doto = llength(curwp->w_dotp);
	ecol = getccol(FALSE);
	curwp->w_doto = savepos;

        ratio = 0;              /* Ratio before dot. */
        ratio_decimal1 = 0;
        ratio_decimal2 = 0;
        if (numchars != 0) {
#if ST520 | MSDOS | BSD29 | CPM | WMCS | DECUSC | MWC86 | LATTICE | AZTEC | TURBO | C86
		ratio = (100L*predchars) / numchars;	/* avoid linking in real number code */
#else
		ratio = (int) ((10000.0 * predchars) / numchars + 0.5);
		ratio_decimal2 = ratio % 10;
		ratio = ratio / 10;
		ratio_decimal1 = ratio % 10;
		ratio = ratio / 10;
#endif
	}

	/* summarize and report the info */
	mlwrite("Line %d/%d Col %d/%d Char %D/%D (%d.%d%d%%) char = 0x%x (%d)",
		predlines+1, numlines+1, col, ecol,
		predchars, numchars, ratio, ratio_decimal1, ratio_decimal2,
		curchar, curchar);
        return (TRUE);
}

int PASCAL NEAR getlinenum(bp, sline)	/* get the a line number */

BUFFER *bp;	/* buffer to get current line from */
LINE *sline;	/* line to search for */

{
        register LINE   *lp;		/* current line */
        register int	numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
        lp = lforw(bp->b_linep);

	/* start counting lines */
        numlines = 0;
        while (lp != bp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == sline)
			break;
		++numlines;
		lp = lforw(lp);
        }

	/* and return the resulting count */
	return(numlines + 1);
}

int getcline()	/* get the current line number */
{
	return(getlinenum(curbp, curwp->w_dotp));
}

/*
 * Return current column.  Stop at first non-blank given TRUE argument.
 */
int getccol(bflg)
int bflg;
{
        register int c, i, col;
        col = 0;
        for (i=0; i<curwp->w_doto; ++i) {
                c = lgetc(curwp->w_dotp, i);
                if (c!=' ' && c!='\t' && bflg)
                        break;
                if (c == '\t')
                        col += tabsize - (col % tabsize) - 1;
                else if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7F)
                        ++col;
                ++col;
        }
        return(col);
}

/*
 * Set current column.
 */
int setccol(pos)

int pos;	/* position to set cursor */

{
        register int c;		/* character being scanned */
	register int i;		/* index into current line */
	register int col;	/* current cursor column   */
	register int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(curwp->w_dotp);

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= pos)
			break;

		/* advance one character */
                c = lgetc(curwp->w_dotp, i);
                if (c == '\t')
                        col += tabsize - (col % tabsize) - 1;
                else if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7F)
                        ++col;
                ++col;
        }

	/* set us at the new position */
	curwp->w_doto = i;

	/* and tell whether we made it */
	return(col >= pos);
}

/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so "WFEDIT" is good enough.
 */
int twiddle(f, n)
int f, n;
{
        register LINE   *dotp;
        register int    doto;
	int cl, cr;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        dotp = curwp->w_dotp;
        doto = curwp->w_doto;
        if (doto==llength(dotp) && --doto<0)
                return (FALSE);
        cr = lgetc(dotp, doto);
        if (--doto < 0)
                return (FALSE);
        cl = lgetc(dotp, doto);
	lputc(dotp, doto+0, (char) cr);
	lputc(dotp, doto+1, (char) cl);
        lchange(WFEDIT);
        return (TRUE);
}

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity. Bound to "C-Q"
 */
int quote(f, n)
int f, n;
{
        register int    s;
        register int    c;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        c = tgetc();
        if (n < 0)
                return (FALSE);
        if (n == 0)
                return (TRUE);
        if (c == '\n') {
                do {
                        s = lnewline();
                } while (s==TRUE && --n);
                return (s);
        }
        return (linsert(n, c));
}

#if DISPSEVEN | DECEDT
/*
 * Find the result of a compose sequence
 */
static int find_compose_result PP(( int c0, int c1 ));

static int find_compose_result(c0, c1)
int c0, c1;
{
	int c, c0_low, c1_low;
	int s0_low, s1_low;
	int first_exact, none_exact;
	CONST char *s;

	first_exact = none_exact = -1;

	c0_low = c0;
	if (c0 >= 'A' && c0 <= 'Z') c0_low += 'a' - 'A';

	c1_low = c1;
	if (c1 >= 'A' && c1 <= 'Z') c1_low += 'a' - 'A';

	for (c = 0; c < 256; c++) {
		s = me_char_name[ c ];
		if (*s != '{') continue;
		s++;
		s0_low = (s[0] & 0xFF);
		if (s0_low == '\0') continue;
		if (s0_low >= 'A' && s0_low <= 'Z') s0_low += 'a' - 'A';
		if (s0_low == c0_low) {
			s1_low = (s[1] & 0xFF);
			if (s1_low == '\0') continue;
			if (s1_low >= 'A' && s1_low <= 'Z') s1_low += 'a' - 'A';
			if (s1_low == c1_low) {
				if ((s[0] & 0xFF) == c0) {
					if ((s[1] & 0xFF) == c1) return c;
					if (first_exact < 0) first_exact = c;
				} else {
					if (none_exact < 0) none_exact = c;
				}
			} else if (c1_low == '*' && s1_low == s0_low && (s[0] & 0xFF) == c0) {
				return c;
			}
		}
	}
	if (first_exact >= 0) return first_exact;
	if (none_exact >= 0) return none_exact;
	return c0;
}

/*
 * Compose the next two characters and insert then into the buffer.
 * The character is always read, even if it is inserted 0 times,
 * for regularity. Bound to "M-+"
 */
int me_compose(f, n)
int f, n;
{
	int c0, c1, c_result, ln_s, c_abort;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	c_abort = ectoc(abortc);

	c0 = tgetc();
	if (c0 == c_abort) return ctrlg(FALSE, 0);

	c1 = tgetc();
	if (c1 == c_abort) return ctrlg(FALSE, 0);

	c_result = find_compose_result(c1, c0);
	if (c_result == c1) c_result = find_compose_result(c0, c1);

	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c_result == '\n') {
		do {
			ln_s = lnewline();
		} while (ln_s == TRUE && --n);
		return ln_s;
	}
	return linsert(n, c_result);
}

/*
 * Unquote a quoted-printable character
 */
int me_unquotep(f, n)
int f, n;
{
	int ch, num;
	int doto, len;
	int is_quoted;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	while (n-- > 0) {
		is_quoted = FALSE;
		doto = curwp->w_doto;
		len = llength(curwp->w_dotp);
		if (doto < len && lgetc(curwp->w_dotp, doto) == '=') {
			if (doto + 1 == len) {
				is_quoted = TRUE;
				ldelete(2, FALSE);
			} else if (doto + 2 < len) {
				is_quoted = TRUE;
				num = 0;
				ch = lgetc(curwp->w_dotp, doto+1);
				if (ch >= '0' && ch <= '9') {
					num = ch - '0';
				} else if (ch >= 'A' && ch <= 'F') {
					num = ch - 'A' + 10;
				} else {
					is_quoted = FALSE;
				}
				ch = lgetc(curwp->w_dotp, doto+2);
				if (ch >= '0' && ch <= '9') {
					num = num * 16 + ch - '0';
				} else if (ch >= 'A' && ch <= 'F') {
					num = num * 16 + ch - 'A' + 10;
				} else {
					is_quoted = FALSE;
				}
				if (is_quoted) {
					ldelete(3, FALSE);
					linsert(1, num);
				}
			}
		}
		if (!is_quoted) {
			if (!forwchar(1, 1)) {
				break;
			}
		}
	}
	return TRUE;
}

/*
 * Unprefix a two-byte sequence
 */
int me_unprefix(f, n)
int f, n;
{
	int ch;
	int doto, len;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	while (n-- > 0) {
		doto = curwp->w_doto;
		len = llength(curwp->w_dotp);
		if (doto < len && lgetc(curwp->w_dotp, doto) == 0xC3) {
			ch = lgetc(curwp->w_dotp, doto+1);
			ldelete(2, FALSE);
			linsert(1, (ch | 0x40));
		} else if (!forwchar(1, 1)) {
			break;
		}
	}
	return TRUE;
}

/*
 * Add a diacritical mark
 */
static int add_dia_mark PP(( int mark ));

static int add_dia_mark(mark)
int mark;
{
	int old_ch, new_ch;
	int doto;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	doto = curwp->w_doto;
	if (doto > 0) --doto;		/* aim at char to left of cursor */
	old_ch = lgetc(curwp->w_dotp, doto);
	new_ch = find_compose_result(old_ch, mark);
	if (new_ch != old_ch) {
		lputc(curwp->w_dotp, doto, (char) new_ch);
		lchange(WFHARD);
	}
	return TRUE;
}

/*
 * Add diacritical marks to the current character.
 */
int me_add_acute(f, n)
int f, n;
{
	return add_dia_mark('\'');
}

int me_add_bar(f, n)
int f, n;
{
	return add_dia_mark('_');
}

int me_add_cedilla(f, n)
int f, n;
{
	return add_dia_mark(',');
}

int me_add_circ(f, n)
int f, n;
{
	return add_dia_mark('^');
}

int me_add_grave(f, n)
int f, n;
{
	return add_dia_mark('`');
}

int me_add_slash(f, n)
int f, n;
{
	return add_dia_mark('/');
}

int me_add_tilde(f, n)
int f, n;
{
	return add_dia_mark('~');
}

int me_add_umlaut(f, n)
int f, n;
{
	return add_dia_mark('"');
}

int me_add_ring(f, n)
int f, n;
{
	return add_dia_mark('*');
}
#endif

/*
 * Set tab size if given non-default argument (n <> 1).  Otherwise, insert a
 * tab into file.  If given argument, n, of zero, change to true tabs.
 * If n > 1, simulate tab stop every n-characters using spaces. This has to be
 * done in this slightly funny way because the tab (in ASCII) has been turned
 * into "C-I" (in 10 bit code) already. Bound to "C-I".
 */
int tab(f, n)
int f, n;
{
        if (n < 0)
                return (FALSE);
        if (n == 0 || n > 1) {
                stabsize = n;
                return(TRUE);
        }
        if (! stabsize)
                return(linsert(1, '\t'));
        return(linsert(stabsize - (getccol(FALSE) % stabsize), ' '));
}

#if	AEDIT
int detab(f, n)		/* change tabs to spaces */

int f,n;	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */
	int did_detab;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	did_detab = FALSE;

	if (f == FALSE)
		n = 1;

	/* loop thru detabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* detab the entire current line */
		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* if we have a tab */
			if (lgetc(curwp->w_dotp, curwp->w_doto) == '\t') {
				did_detab = TRUE;
				ldelete(1L, FALSE);
				insspace(TRUE,
					tabsize - (curwp->w_doto % tabsize));
			}
			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	if (did_detab)
		lchange(WFEDIT); /* yes, we have made at least an edit */
	return(TRUE);
}

int entab(f, n)		/* change spaces to tabs where posible */

int f,n;	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register int cchar;	/* current character */
	int did_entab;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	did_entab = FALSE;

	if (f == FALSE)
		n = 1;

	/* loop thru entabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* entab the entire current line */
		fspace = -1;
		ccol = 0;
		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* get the current character */
			cchar = lgetc(curwp->w_dotp, curwp->w_doto);

			switch (cchar) {
				case '\t': /* a tab...count em up */
					if (fspace >= 0) {
						backchar(TRUE, ccol-fspace);
						ldelete((long)(ccol-fspace),
							FALSE);
						fspace = -1;
						did_entab = TRUE;
					}
					ccol += tabsize - (ccol % tabsize);
					break;

				case ' ':  /* a space...compress? */
					if (fspace < 0)
						fspace = ccol;
					++ccol;

					/* see if it is time to compress */

					if (ccol <
					fspace + tabsize - (fspace % tabsize))
						break;

					if (ccol > fspace + 1) {
			/* there is a bug here dealing with mixed space/tabed
			   lines.......it will get fixed		*/
			/* WB 4/21/88 - casted ldelete arg to long */
						backchar(TRUE, ccol-fspace-1);
						ldelete( (long) (ccol-fspace),
							FALSE);
						linsert(1, '\t');
						backchar(FALSE, 1);
						did_entab = TRUE;
					}
					fspace = -1;

					break;

				default:   /* any other char...just count */
					ccol++;
					fspace = -1;
					break;
			}
			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	if (did_entab)
		lchange(WFEDIT); /* yes, we have made at least an edit */
	return(TRUE);
}

int trim(f, n)	/* trim trailing whitespace from the point to eol */

int f,n;	/* default flag and numeric repeat count */

{
	register LINE *lp;	/* current line pointer */
	register int offset;	/* original line offset position */
	register int length;	/* current length */
	register int inc;	/* increment to next line [sgn(n)] */
	int did_trim;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	did_trim = FALSE;

	if (f == FALSE)
		n = 1;

	/* loop thru trimming n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		lp = curwp->w_dotp;		/* find current line text */
		offset = curwp->w_doto;		/* save original offset */
		length = lp->l_used;		/* find current length */

		/* trim the current line */
		while (length > offset) {
			if (lgetc(lp, length-1) != ' ' &&
			    lgetc(lp, length-1) != '\t')
			    	break;
			length--;
		}
		if (length != lp->l_used) {
			did_trim = TRUE;
			lp->l_used = length;
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	if (did_trim)
		lchange(WFEDIT);
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	return(TRUE);
}
#endif

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
int openline(f, n)
int f, n;
{
        register int    i;
        register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        if (n == 0)
                return (TRUE);
        i = n;                                  /* Insert newlines.     */
        do {
                s = lnewline();
        } while (s==TRUE && --i);
        if (s == TRUE)                          /* Then back up overtop */
                s = backchar(f, n);             /* of them all.         */
        return (s);
}

/*
 * Insert a newline. Bound to "C-M". If we are in CMODE, do automatic
 * indentation as specified.
 */
int newline(f, n)
int f, n;
{
	register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return (FALSE);

	/* if we are in C mode and this is a default <NL> */
	if (n == 1 && (curbp->b_mode & MDCMOD) &&
	    curwp->w_dotp != curbp->b_linep)
		return(cinsert());

        /*
         * If a newline was typed, fill column is defined, the argument is non-
         * negative, wrap mode is enabled, and we are now past fill column,
	 * and we are not read-only, perform word wrap.
         */
        if ((curwp->w_bufp->b_mode & MDWRAP) && fillcol > 0 &&
	    getccol(FALSE) > fillcol &&
	    (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
		execute(FUNC|'W', HOOK, 1);

	/* insert some lines */
	while (n--) {
		if ((s=lnewline()) != TRUE)
			return (s);
	}
	return (TRUE);
}

static VOID setcindent(target)	/* set the indent to target */
int target;
{
	int ch, i;

	if (target < 0 || target > NSTRING) return;
	if (cindtype != 0) {
		i = 0;
		while (i < llength(curwp->w_dotp) &&
			((ch = lgetc(curwp->w_dotp, i)) == ' ' || ch == '\t'))
			i++;
		curwp->w_doto = 0;
		ldelete(i, FALSE);
	}
	if (cindtype != 1)
		while (getccol(FALSE) < target)
			linsert(1, '\t');
	while (getccol(FALSE) > target)
		backdel(FALSE, 1);
	if (getccol(FALSE) < target)
		linsert(target - getccol(FALSE), ' ');
}

static int getcindent()	/* get indent of current line */
{
	int ch;

	curwp->w_doto = 0;		/* debut de ligne */
	/* aller au debut de la ligne apres la tabulation */
	while (curwp->w_doto < llength(curwp->w_dotp) &&
		((ch = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ' ||
		  ch == '\t'))
		forwchar(FALSE, 1);

	/* delete back first */
	return(getccol(FALSE));		/* c'est l'indent que l'on doit avoir */
}

static int getpindent()	/* get indent of line with matching parens */
{
	register int count, i;
	register int ch;
	register LINE *bracelp;

	bracelp = curwp->w_dotp;
	curwp->w_doto = llength(curwp->w_dotp);

	count = 0;
	i = 20;		/* max number of lines to scan */

	while (backchar(FALSE, 1) != FALSE) {
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (i-- <= 0 || count == 0) break;
		}
		else	{
			ch = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ch == ')')  ++count;
			else if (ch == '(' && --count == 0)
				{ i = 99; break; }
		}
	}

	if (i != 99)
		curwp->w_dotp = bracelp;

	return( getcindent() );
}

static int cinsert()	/* insert a newline and indentation for C */

{
	register int i;		/* index into line to copy indent from */
	register int llen;	/* length of line to copy indent from */
	register int bracef;	/* was there a brace at the end of line? */
	int semif;		/* was there a semicolon at the end of line? */
	register LINE *lp;	/* current line pointer */
	char ichar[NSTRING];	/* buffer to hold indent of last line */
	int target, linecount, oldoff;
	LINE *scanlp, *oldlp;

	/* trim the whitespace before the point */
	lp = curwp->w_dotp;
	i = curwp->w_doto;
	while (i > 0 && (lgetc(lp, i-1) == ' ' || lgetc(lp, i-1) == '\t')) {
		backdel(FALSE, 1);
		i--;
	}

	/* check for a brace */
	bracef = (i > 0 && lgetc(lp, i-1) == '{');
	semif = (i > 0 && lgetc(lp, i-1) == ';');

	llen = 0;
	oldoff = curwp->w_doto;
	if (oldoff < llength(lp)) {
		llen = getcindent();
		curwp->w_doto = oldoff;
		llen -= getccol(FALSE);
	}

	/* put in the newline */
	if (lnewline() == FALSE)
		return(FALSE);

	lp = oldlp = curwp->w_dotp;
	/* oldoff = curwp->w_doto; -- must be 0 after lnewline */

	/* don't double indent a line */
	if (llength(lp) > 0 &&
	    (llen > 0 || lgetc(lp,0)=='#'))
		return(TRUE);

	if (lp != curbp->b_linep)
		lp = lp->l_bp;

	/* hunt for the last non-blank line to get indentation from */
	while (lp != curbp->b_linep &&
	       (llength(lp) == 0 || lgetc(lp,0) == '#'))
		lp = lp->l_bp;

	/* grab a pointer to text to copy indentation from */
	/* save the indent of the last non blank line */
	curwp->w_dotp = lp;
	llen = ((semif || bracef)? getpindent(): getcindent());
	i = curwp->w_doto;
	if (i > NSTRING-1) i = NSTRING-1;

	bytecopy(ichar, curwp->w_dotp->l_text, i);

	curwp->w_dotp = oldlp;
	curwp->w_doto = 0;

	/* insert this saved indentation */
	linstr(ichar);

	oldoff = curwp->w_doto;

	/* if no brace, we are done */
	if (!bracef) return(TRUE);

	linecount = 100;		/* max number of lines to scan */
	if (lp != curbp->b_linep)
		lp = lp->l_bp;
	target = -1;
	for (; lp != curbp->b_linep && linecount-- > 0; lp = lp->l_bp)
		if (llength(lp) > 0 && lgetc(lp, llength(lp)-1) == '{') {
			curwp->w_dotp = lp;
			target = getpindent();
			scanlp = lp->l_fp;
			while (scanlp != curbp->b_linep &&
					scanlp != oldlp &&
					llength(scanlp) == 0)
				scanlp = scanlp->l_fp;
			if (scanlp == curbp->b_linep || scanlp == oldlp)
				{ target = -1; continue; }
			curwp->w_dotp = scanlp;
			target = getcindent() - target;
			if (target >= 0) break;
		}
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	if (target < 0)
		tab(FALSE, 1);
	else
		setcindent(getcindent() + target);

	return(TRUE);
}

int insbrace(n, c) /* insert a brace into the text here...we are in CMODE */

int n;	/* repeat count */
int c;	/* brace to insert (always { for now) */

{
	register int ch;	/* last character before input */
	register int oc;	/* caractere oppose a c */
	register int i, count;
	register int target;	/* column brace should go after */
	register LINE *oldlp;
	register int  oldoff;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* if we aren't at the beginning of the line... */
	if (curwp->w_doto != 0)

	/* scan to see if all space before this is white space */
		for (i = curwp->w_doto - 1; i >= 0; --i) {
			ch = lgetc(curwp->w_dotp, i);
			if (ch != ' ' && ch != '\t')
				return(linsert(n, c));
		}

	/* chercher le caractere oppose correspondant */
	switch (c) {
		case '}': oc = '{'; break;
		case ']': oc = '['; break;
		case ')': oc = '('; break;
		default: return(FALSE);
	}
	
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* scan backwards to find matching brace */

	count = 1;
	i = 10000;		/* max number of lines to scan */

	while (count > 0) {
		if (backchar(FALSE, 1) == FALSE) break;
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (i-- <= 0) break;
		}
		else	{
			ch = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ch == c)  ++count;
			if (ch == oc) --count;
		}
	}

	/* if we found a brace, match parens */

	target = 0;

	if (count == 0)
		target = getpindent();

	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;

	if (count != 0) {	/* no match, use old format */
		target = getccol(FALSE);
		target -= 1;
		target -= target % (stabsize == 0 ? tabsize : stabsize);
		mlwrite("[no match]");
	}

	setcindent(target);

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

int inspound()	/* insert a # into the text here...we are in CMODE */

{
	register int ch;	/* last character before input */
	register int i;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* if we are at the beginning of the line, no go */
	if (curwp->w_doto == 0)
		return(linsert(1,'#'));

	/* scan to see if all space before this is white space */
	for (i = curwp->w_doto - 1; i >= 0; --i) {
		ch = lgetc(curwp->w_dotp, i);
		if (ch != ' ' && ch != '\t')
			return(linsert(1, '#'));
	}

	/* delete back first */
	while (getccol(FALSE) >= 1)
		backdel(FALSE, 1);

	/* and insert the required pound */
	return(linsert(1, '#'));
}

/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Normally this command is bound to "C-X C-O". Any argument is
 * ignored.
 */
int deblank(f, n)
int f, n;
{
        register LINE   *lp1;
        register LINE   *lp2;
        long nld;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        lp1 = curwp->w_dotp;
        while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
                lp1 = lp2;
        lp2 = lp1;
        nld = 0;
        while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
                ++nld;
        if (nld == 0)
                return (TRUE);
        curwp->w_dotp = lforw(lp1);
        curwp->w_doto = 0;
        return (ldelete(nld, FALSE));
}

/*
 * Insert a newline, then enough tabs and spaces to duplicate the indentation
 * of the previous line. Assumes tabs are every eight characters. Quite simple.
 * Figure out the indentation of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by inserting the right number
 * of tabs and spaces. Return TRUE if all ok. Return FALSE if one of the
 * subcomands failed. Normally bound to "C-J".
 */
int indent(f, n)
int f, n;
{
        register int    nicol;
        register int    c;
        register int    i;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        while (n--) {
                nicol = 0;
                for (i=0; i<llength(curwp->w_dotp); ++i) {
                        c = lgetc(curwp->w_dotp, i);
                        if (c!=' ' && c!='\t')
                                break;
                        if (c == '\t')
                                nicol += tabsize - (nicol % tabsize) - 1;
                        ++nicol;
                }
                if (lnewline() == FALSE
                || ((i=nicol/tabsize)!=0 && linsert(i, '\t')==FALSE)
                || ((i=nicol%tabsize)!=0 && linsert(i,  ' ')==FALSE))
                        return (FALSE);
        }
        return (TRUE);
}

/*
 * Delete forward. This is real easy, because the basic delete routine does
 * all of the work. Watches for negative arguments, and does the right thing.
 * If any argument is present, it kills rather than deletes, to prevent loss
 * of text if typed with a big argument. Normally bound to "C-D".
 */
int forwdel(f, n)
int f, n;
{
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (backdel(f, -n));
        if (f != FALSE) {                       /* Really a kill.       */
                if ((lastflag&CFKILL) == 0)
                        kdelete();
                thisflag |= CFKILL;
        }
        else ykchar = ((curwp->w_doto == curwp->w_dotp->l_used)? '\n':
        	lgetc(curwp->w_dotp, curwp->w_doto));
        return (ldelete((long)n, f));
}

/*
 * Delete backwards. This is quite easy too, because it's all done with other
 * functions. Just move the cursor back, and delete forwards. Like delete
 * forward, this actually does a kill if presented with an argument. Bound to
 * both "RUBOUT" and "C-H".
 */
int backdel(f, n)
int f, n;
{
        register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (forwdel(f, -n));
        if (f != FALSE) {                       /* Really a kill.       */
                if ((lastflag&CFKILL) == 0)
                        kdelete();
                thisflag |= CFKILL;
        }
        else ykchar = ((curwp->w_doto == 0)? '\n':
        	lgetc(curwp->w_dotp, (curwp->w_doto-1)));
        if ((s=backchar(f, n)) == TRUE)
                s = ldelete((long)n, f);
        return (s);
}

/*
 * Kill text. If called without an argument, it kills from dot to the end of
 * the line, unless it is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the start of the line to dot.
 * If called with a positive argument, it kills from dot forward over that
 * number of newlines. If called with a negative argument it kills backwards
 * that number of newlines. Normally bound to "C-K".
 */

int killtext(f, n)
int f, n;
{
	return( killtxt(f, n, FALSE) );
}

#if DECEDT
int delfline(f, n)
int f, n;
{
	return( killtxt(f, n, TRUE) );
}
#endif

static int killtxt(f, n, delflag)
int f, n, delflag;
{
        register LINE   *nextp;
        long chunk;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((lastflag&CFKILL) == 0)             /* Clear kill buffer if */
                kdelete();                      /* last wasn't a kill.  */
        thisflag |= CFKILL;
        if (f == FALSE) {
                chunk = llength(curwp->w_dotp)-curwp->w_doto;
                if ((delflag == TRUE) || (chunk == 0))
                        ++chunk;
        } else if (n == 0) {
                chunk = curwp->w_doto;
                curwp->w_doto = 0;
        } else if (n > 0) {
                chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
                nextp = lforw(curwp->w_dotp);
                while (--n) {
                        if (nextp == curbp->b_linep)
                                return (FALSE);
                        chunk += llength(nextp)+1;
                        nextp = lforw(nextp);
                }
        } else {
                mlwrite("neg kill");
                return (FALSE);
        }
        return(ldelete(chunk, TRUE));
}

int
me_setmode(f, n) /* prompt and set an editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(TRUE, FALSE));
}

int
delmode(f, n)	/* prompt and delete an editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(FALSE, FALSE));
}

int
setgmode(f, n)	/* prompt and set a global editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(TRUE, TRUE));
}

int
delgmode(f, n)	/* prompt and delete a global editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(FALSE, TRUE));
}

static int
adjustmode(kind, global)	/* change the editor mode status */

int kind;	/* true = set,		false = delete */
int global;	/* true = global flag,	false = current buffer flag */
{
	register char *scan;		/* scanning pointer to convert prompt */
	register int i;			/* loop index */
	/* register status; */		/* error return on input */
#if	COLOR
	register int uflag;		/* was modename uppercase?	*/
#if	TERMCAP | VMSVT
	NOSHARE extern int usedcolor, isansi, termflag;
#endif
#endif
	char prompt[50];	/* string to prompt user with */
	char cbuf[NPAT];		/* buffer to recieve mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt,"Global mode to ");
	else
		strcpy(prompt,"Mode to ");

	if (kind == TRUE)
		strcat(prompt, "add: ");
	else
		strcat(prompt, "delete: ");

	/* prompt the user and get an answer */

#if 1
	/* 12Mar93 wb complete mode name */
	scan = gtmodename(prompt);
	if (!scan)
		return(FALSE);
	strncpy(cbuf, scan, NPAT);
	cbuf[NPAT-1] = '\0';
#else
	status = mlreply(prompt, cbuf, NPAT - 1);
	if (status != TRUE)
		return(status);
#endif

	/* make it uppercase */

	scan = cbuf;
#if	COLOR
	uflag = (*scan >= 'A' && *scan <= 'Z');
#endif
	while (*scan != 0) {
		if (*scan >= 'a' && *scan <= 'z')
			*scan = (char) (*scan - 32);
		scan++;
	}

	/* test it first against the colors we know */
	for (i=0; i<NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if	COLOR
#if	TERMCAP | VMSVT
			if (isansi && (termflag&8) == 0) {
				usedcolor = TRUE;
				eolexist = FALSE;
			}
#endif
			if (uflag) {
				if (global)
					gfcolor = i;
				else
					curwp->w_fcolor = (shrt) i;
			} else {
				if (global)
					gbcolor = i;
				else
					curwp->w_bcolor = (shrt) i;
			}

			curwp->w_flag |= WFCOLR;
#endif
			mlerase();
			return(TRUE);
		}
	}

	/* test it against the modes we know */

	for (i=0; i < NUMMODES; i++) {
		if (strcmp(cbuf, modename[i]) == 0) {
			/* finding a match, we process it */
			if (kind == TRUE) {
				if (global)
					gmode |= (1 << i);
				else
					curbp->b_mode |= (1 << i);
			} else {
				if (global)
					gmode &= ~(1 << i);
				else
					curbp->b_mode &= ~(1 << i);
			}
			/* display new mode line */
			if (global == 0)
				upmode();
			mlerase();	/* erase the junk */
			return(TRUE);
		}
	}

	mlwrite("No such mode!");
	return(FALSE);
}

/*	show-modes	Bring up a fake buffer and list the
			names of all the modes
*/

int modelist(mstring)
CONSTA char *mstring;
{
	BUFFER *modebuf;	/* buffer to put mode list into */
	CONST char *sp;
	int matchflag;
	int mlen, len;
	int i;

	/* get a buffer for the mode list */
	modebuf = bfind("Mode List", TRUE, 0);
	if (modebuf == NULL || bclear(modebuf) == FALSE) {
		mlwrite("Can not display mode list");
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite("[Building Mode List]");

	mlen = strlen(mstring);

	/* build the mode list */
	for (i = 0; i < NCOLORS + NUMMODES; i++) {
		sp = (i < NCOLORS?
			cname[i]: modename[i-NCOLORS]);
		len = strlen(sp);
		if (len > mlen) len = mlen;

		/* is this a match? */
		matchflag = (len == 0 || !strcompare(mstring, sp, len));

		if (matchflag) {
			/* add a name to the buffer */
			if (addline(modebuf, sp) != TRUE)
				return(FALSE);
		}
	}

	/* display the list */
	wpopup(modebuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

int PASCAL NEAR showmodes(f, n)
int f,n;	/* prefix flag and argument */
{
	return(modelist(""));
}

/*	This function simply clears the message line,
		mainly for macro usage			*/

int
me_clrmes(f, n)

int f, n;	/* arguments ignored */

{
	mlforce("");
	return(TRUE);
}

/*	This function writes a string on the message line
		mainly for macro usage			*/

int writemsg(f, n)

int f, n;	/* arguments ignored */

{
	register int status;
	char buf[NPAT];		/* buffer to recieve message into */

	if ((status = mlreply("Message to write: ", buf, NPAT - 1)) != TRUE)
		return(status);

	/* expand all '%' to "%%" so mlwrite won't expect arguments */
	makelit(buf, NPAT);

	/* write the message out */
	mlforce(buf);
	return(TRUE);
}

#if	CFENCE
/*	the cursor is moved to a matching fence	*/

int getfence(f, n)

int f, n;	/* not used */

{
	register LINE *oldlp;	/* original line pointer */
	register int oldoff;	/* and offset */
	register int sdir;	/* direction of search (1/-1) */
	register int count;	/* current fence level count */
	register int ch;	/* fence type to match against */
	register int ofence;	/* open fence */
	register int c;		/* current character in scan */

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* get the current character */
	if (oldoff == llength(oldlp))
		ch = '\n';
	else
		ch = lgetc(oldlp, oldoff);

	/* setup proper matching fence */
	switch (ch) {
		case '(': ofence = ')'; sdir = FORWARD; break;
		case '{': ofence = '}'; sdir = FORWARD; break;
		case '[': ofence = ']'; sdir = FORWARD; break;
		case '<': ofence = '>'; sdir = FORWARD; break;
		case ')': ofence = '('; sdir = REVERSE; break;
		case '}': ofence = '{'; sdir = REVERSE; break;
		case ']': ofence = '['; sdir = REVERSE; break;
		case '>': ofence = '<'; sdir = REVERSE; break;
		default: TTbeep(); return(FALSE);
	}

	/* set up for scan */
	count = 1;

	/* scan until we find it, or reach the end of file */
	while (count > 0) {
		if (sdir == FORWARD)
			{if (forwchar(FALSE, 1) == FALSE) break;}
		else
			{if (backchar(FALSE, 1) == FALSE) break;}

		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\n';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == ofence)
			--count;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		curwp->w_flag |= WFMOVE;
		return(TRUE);
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	TTbeep();
	return(FALSE);
}
#endif

/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/

int
fmatch(ch)

int ch;	/* fence type to match against */

{
	register LINE *oldlp;	/* original line pointer */
	register int oldoff;	/* and offset */
	register LINE *toplp;	/* top line in current window */
	register int count;	/* current fence level count */
	register char opench;	/* open fence */
	register int c;		/* current character in scan */
	register int i;

	/* first get the display update out there */
	update(FALSE);

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* setup proper open fence for passed close fence */
	if (ch == ')')
		opench = '(';
	else if (ch == '}')
		opench = '{';
	else if (ch == ']')
		opench = '[';
	else
		opench = '<';

	/* find the top line and set up for scan */
	toplp = curwp->w_linep->l_bp;
	count = 1;
	backchar(FALSE, 1);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && backchar(FALSE, 1) == TRUE &&
			curwp->w_dotp != toplp) {
		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\n';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == opench)
			--count;
	}

	/* if count is zero, we have a match, display the sucker */
	/* there is a real machine dependant timing problem here we have
	   yet to solve......... */
	if (count == 0) {
		for (i = 0; i < term.t_pause; i++)
			update(FALSE);
#if	defined(M_XENIX) || PORT_HAS_NAP
		nap(250L);
#else
#ifdef _AIX	/* use usleep */
		usleep(250000);
#endif
#endif
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	return(TRUE);
}

int
istring(f, n)	/* ask for and insert a string into the current
		   buffer at the current point */

int f, n;	/* ignored arguments */

{
	register int status;	/* status return code */
	char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mltreply("String to insert<META>: ", tstring, NPAT, metac);
	if (status != TRUE)
		return(status);

	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	/* insert it */
	while (n-- > 0 && status == TRUE)
		status = linstr(tstring);

	return(TRUE);
}

#if	DECEDT
/*
 * Display the length of a string which the cursor is inside.
 * Does not worry about backslashes.
 * Unbound.
 */
int showslen(f, n)
int f, n;
{
	int i, left, right;
	int qch;

	left = right = (-1);
	qch = '\0';
	if (llength(curwp->w_dotp) > 0) {
		for (i=curwp->w_doto; i<llength(curwp->w_dotp); ++i) {
			qch = lgetc(curwp->w_dotp, i);
			if (qch == '\'' || qch == '\"' || qch == '`')
				{right = i; break;}
		}
	}
	if (right >= 0) {
		for(i=curwp->w_doto-1; i>=0; --i)
			if (lgetc(curwp->w_dotp, i) == qch)
				{left = i; break;}
	}
	if (left >= 0)
		mlwrite("String length is %d", right - 1 - left);
	else
		mlwrite("[String not found]");
        return (TRUE);
}

/* set over write -- toggle overwrite mode */

int setovrwrt(f, n)
int f, n;
{
	if (curwp->w_bufp->b_mode & MDOVER)
		curwp->w_bufp->b_mode &= ~MDOVER;
	else
		curwp->w_bufp->b_mode |= MDOVER;
	upmode();
	return (TRUE);
}
#endif

