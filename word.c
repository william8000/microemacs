/*
 * The routines in this file implement commands that work word or a
 * paragraph at a time.  There are all sorts of word mode commands.  If I
 * do any sentence mode commands, they are likely to be put in this file. 
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.	Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 */
int wrapword(f, n)

int f;		/* default flag */
int n;		/* numeric argument */
 
{
	register int cnt;	/* size of word wrapped to next line */
	register int c;		/* temporary character */

	UNUSED_ARGS_FN;

	/* backup from the <NL> 1 char */
	if (!backchar(FALSE, 1))
		return(FALSE);

	/* back up until we aren't in a word,
	   make sure there is a break in the line */
	cnt = 0;
	while (((c = lgetc(curwp->w_dotp, curwp->w_doto)) != ' ')
				&& (c != '\t')) {
		cnt++;
		if (!backchar(FALSE, 1))
			return(FALSE);
		/* if we make it to the beginning, start a new line */
		if (curwp->w_doto == 0) {
			goeol(FALSE, 0);
			return(lnewline());
		}
	}

	/* delete the forward white space */
 	if (!forwdel(FALSE, 1))
		return(FALSE);

	/* put in a end of line */
	if (!lnewline())
		return(FALSE);

	/* and past the first word */
	while (cnt-- > 0) {
		if (forwchar(FALSE, 1) == FALSE)
			return(FALSE);
	}
	return(TRUE);
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
int backword(f, n)
int f, n;
{
	if (n < 0)
		return (forwword(f, -n));
	if (backchar(FALSE, 1) == FALSE)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return (forwchar(FALSE, 1));
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
int forwword(f, n)
int f, n;
{
	if (n < 0)
		return (backword(f, -n));
	while (n--) {
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}

		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int upperword(f, n)
int f, n;
{
	register int	c;

	UNUSED_ARG(f);

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (c>='a' && c<='z') {
				c -= 'a'-'A';
				lputc(curwp->w_dotp, curwp->w_doto, (char) c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int lowerword(f, n)
int f, n;
{
	register int	c;

	UNUSED_ARG(f);

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (c>='A' && c<='Z') {
				c += 'a'-'A';
				lputc(curwp->w_dotp, curwp->w_doto, (char) c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int capword(f, n)
int f, n;
{
	register int	c;

	UNUSED_ARG(f);

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (c>='a' && c<='z') {
				c -= 'a'-'A';
				lputc(curwp->w_dotp, curwp->w_doto, (char) c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return (FALSE);
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (c>='A' && c<='Z') {
					c += 'a'-'A';
					lputc(curwp->w_dotp, curwp->w_doto,
						(char) c);
					lchange(WFHARD);
				}
				if (forwchar(FALSE, 1) == FALSE)
					return (FALSE);
			}
		}
	}
	return (TRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. With a zero argument, just
 * kill one word and no whitespace. Bound to "M-D".
 */
int delfword(f, n)
int f, n;
{
	register LINE	*dotp;	/* original cursor line */
	register int	doto;	/*	and row */
	register int c;		/* temp char */
	long size;		/* # of chars to delete */

	UNUSED_ARG(f);

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode&MDVIEW)
		return(rdonly());

	/* ignore the command if there is a negative argument */
	if (n < 0)
		return (FALSE);

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;	/* this command is a kill */

	/* save the current cursor position */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;

	/* figure out how many characters to give the axe */
	size = 0;

#if	DECEDT
#else
	/* get us into a word.... */
	while (inword() == FALSE) {
		if (forwchar(FALSE, 1) == FALSE)
			return(FALSE);
		++size;
	}
#endif

	if (n == 0) {
		/* skip one word, no whitespace! */
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
			++size;
		}
	} else {
		/* skip n words.... */
		while (n--) {
	
			/* if we are at EOL; skip to the beginning of the next */
			while (curwp->w_doto == llength(curwp->w_dotp)) {
				if (forwchar(FALSE, 1) == FALSE)
					return(FALSE);
				++size;
			}
	
			/* move forward till we are at the end of the word */
			while (inword() == TRUE) {
				if (forwchar(FALSE, 1) == FALSE)
					return(FALSE);
				++size;
			}
	
			/* if there are more words, skip the interword stuff */
			if (n != 0)
				while (inword() == FALSE) {
					if (forwchar(FALSE, 1) == FALSE)
						return(FALSE);
					++size;
				}
		}
	
		/* skip whitespace and newlines */
		while ((curwp->w_doto == llength(curwp->w_dotp)) ||
			((c = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ') ||
			(c == '\t')) {
				if (forwchar(FALSE, 1) == FALSE)
					break;
				++size;
		}
	}

	/* restore the original position and delete the words */
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return (ldelete(size, TRUE));
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
int delbword(f, n)
int f, n;
{
	long size;

	UNUSED_ARG(f);

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode&MDVIEW)
		return(rdonly());

	/* ignore the command if there is a nonpositive argument */
	if (n <= 0)
		return (FALSE);

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;	/* this command is a kill */

	if (backchar(FALSE, 1) == FALSE)
		return (FALSE);
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return (FALSE);
			++size;
		}
		while (inword() != FALSE) {
			++size;
			if (backchar(FALSE, 1) == FALSE)
				goto bckdel;
		}
	}
	if (forwchar(FALSE, 1) == FALSE)
		return (FALSE);
bckdel:	return (ldelete(size, TRUE));
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The default word character list is hard coded. If $wchars
 * has been set by the user, use that instead
 */

int inword()
{
	register int	c;

	/* the end of a line is never in a word */
	if (curwp->w_doto == llength(curwp->w_dotp))
		return(FALSE);

	/* grab the word to check */
	c = lgetc(curwp->w_dotp, curwp->w_doto);

	/* if we are using the table.... */
	if (wlflag)
		return(wordlist[c]);

#if	DECEDT
	return(((c&0x7F) > 0x20 && (c&0x7F) < 0x7F) ? TRUE : FALSE );
#else
	if (c>='a' && c<='z')
		return (TRUE);
	if (c>='A' && c<='Z')
		return (TRUE);
	if (c>='0' && c<='9')
		return (TRUE);
	if (c == '_')
		return(TRUE);
	return (FALSE);
#endif
}

#if	WORDPRO
int fillpara(f, n) /* Fill the current paragraph according to the current
		   fill column						*/

int f, n;	/* deFault flag and Numeric argument */

{
	register int c;			/* current char during scan	*/
	register int wordlen;		/* length of current word	*/
	register int clength;		/* position on line during fill	*/
	register int i;			/* index during word copy	*/
	register int newlength;		/* tentative new line length	*/
	register int eopflag;		/* Are we at the End-Of-Paragraph? */
	register int firstwflag;	/* first word? (needs no space)	*/
	register LINE *eopline;		/* pointer to line just past EOP */
	register int dotflag;		/* was the last char a period?	*/
	char wbuf[NSTRING];		/* buffer for current word	*/

	UNUSED_ARG(f);

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (fillcol == 0) {	/* no fill column set */
		mlwrite("No fill column set");
		return(FALSE);
	}

	while (n-- > 0) {

		/* record the pointer to the line just past the EOP */
		goeop(FALSE, 1);
		eopline = lforw(curwp->w_dotp);

		/* and back top the beginning of the paragraph */
		gobop(FALSE, 1);

		/* initialize various info */
		clength = curwp->w_doto;
		if (clength && curwp->w_dotp->l_text[0] == TAB)
			clength = tabsize;
		wordlen = 0;
		dotflag = 0;

		/* scan through lines, filling words */
		firstwflag = TRUE;
		eopflag = FALSE;
		while (!eopflag) {
			/* get the next character in the paragraph */
			if (curwp->w_doto == llength(curwp->w_dotp)) {
				c = ' ';
				if (lforw(curwp->w_dotp) == eopline ||
				    curwp->w_dotp == curbp->b_linep)
					eopflag = TRUE;
			} else
				c = lgetc(curwp->w_dotp, curwp->w_doto);

			/* and then delete it */
			ldelete(1L, FALSE);

			/* if not a separator, just add it in */
			if (c != ' ' && c != '\t') {
				if (wordlen < NSTRING - 1)
					wbuf[wordlen++] = (char) c;
			} else if (wordlen) {
				/* at a word break with a word waiting */
				/* calculate tentative new length with word added */
				newlength = clength + 1 + dotflag + wordlen;
				if (newlength <= fillcol || clength == 0) {
					/* add word to current line */
					if (!firstwflag) {
						linsert(1, ' '); /* the space */
						++clength;
#if 0
						/* add a second space between sentences */
						if (dotflag) {
							linsert(1, ' '); /* the space */
							++clength;
						}
#endif
					}
					firstwflag = FALSE;
				} else {
					/* start a new line */
					lnewline();
					clength = 0;
				}

				/* and add the word in in either case */
				for (i=0; i<wordlen; i++) {
					linsert(1, wbuf[i]);
					++clength;
				}

				/* check if this word ends a sentence */
				dotflag = 0;
				if (wordlen >= 2) {
					c = wbuf[ wordlen - 1 ];
					if (c == '!' || c == '?' || c == '.') {
						c = wbuf[ wordlen - 2 ];
						if ((c >= 'a' && c <= 'z') ||
							c == '"' || c == '\'' ||
							c == ')' || c == ']') {
							c = wbuf[0];
							if (c < 'A' || c > 'Z') {
								dotflag = 1;
							}
						}
					}
				}

				/* reset the word length */
				wordlen = 0;
			}
		}
		/* and add a last newline for the end of our new paragraph */
		if (curwp->w_dotp != curbp->b_linep)
			lnewline();
	}

	return(TRUE);
}

int killpara(f, n)	/* delete n paragraphs starting with the current one */

int f;	/* default flag */
int n;	/* # of paras to delete */

{
	register int status;	/* returned status of functions */

	UNUSED_ARG(f);

	while (n--) {		/* for each paragraph to delete */

		/* mark out the end and beginning of the para to delete */
		goeop(FALSE, 1);

		/* set the mark here */
		curwp->w_markp = curwp->w_dotp;
		curwp->w_marko = curwp->w_doto;

		/* go to the beginning of the paragraph */
		gobop(FALSE, 1);
		curwp->w_doto = 0;	/* force us to the beginning of line */
	
		/* and delete it */
		if ((status = killregion(FALSE, 1)) != TRUE)
			return(status);

		/* and clean up the 2 extra lines */
		ldelete(2L, TRUE);
	}
	return(TRUE);
}


/*	wordcount:	count the # of words in the marked region,
			along with average word sizes, # of chars, etc,
			and report on them.			*/

int wordcount(f, n)

int f, n;	/* ignored numeric arguments */

{
	register LINE *lp;	/* current line to scan */
	register int offset;	/* current char to scan */
	long size;		/* size of region left to count */
	register int ch;	/* current character to scan */
	register int wordflag;	/* are we in a word now? */
	register int lastword;	/* were we just in a word? */
	long nwords;		/* total # of words */
	long nchars;		/* total number of chars */
	int nlines;		/* total number of lines in region */
	int avgch;		/* average number of chars/word */
	int status;		/* status return code */
	REGION region;		/* region to look at */

	UNUSED_ARGS_FN;

	/* make sure we have a region to count */
	if ((status = getregion(&region)) != TRUE)
		return(status);
	lp = region.r_linep;
	offset = region.r_offset;
	size = region.r_size;

	/* count up things */
	lastword = FALSE;
	nchars = 0L;
	nwords = 0L;
	nlines = 0;
	while (size-- > 0L) {

		/* get the current character */
		if (offset == llength(lp)) {	/* end of line */
			ch = '\n';
			lp = lforw(lp);
			offset = 0;
			++nlines;
		} else {
			ch = lgetc(lp, offset);
			++offset;
		}

		/* and tabulate it */
		wordflag = ((ch >= 'a' && ch <= 'z') ||
			    (ch >= 'A' && ch <= 'Z') ||
			    (ch >= '0' && ch <= '9'));
		if (wordflag == TRUE && lastword == FALSE)
			++nwords;
		lastword = wordflag;
		++nchars;
	}

	/* and report on the info */
	if (nwords > 0L)
		avgch = (int)((100L * nchars) / nwords);
	else
		avgch = 0;

	mlwrite("Words %D Chars %D Lines %d Avg chars/word %f",
		nwords, nchars, nlines + 1, avgch);
	return(TRUE);
}
#endif
