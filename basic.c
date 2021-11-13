/*
 * The routines in this file move the cursor around on the screen. They
 * compute a new value for the cursor, then adjust ".". The display code
 * always updates the cursor location, so only moves between lines, or
 * functions that adjust the top line in the window and invalidate the
 * framing, are hard.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

/*
 * Move the cursor to the
 * beginning of the current line.
 * Trivial.
 */
int gobol(f, n)
int f, n;
{
        curwp->w_doto  = 0;
        return (TRUE);
}

/*
 * Move the cursor backwards by "n" characters. If "n" is less than zero call
 * "forwchar" to actually do the move. Otherwise compute the new cursor
 * location. Error if you try and move out of the buffer. Set the flag if the
 * line pointer for dot changes.
 */
int backchar(f, n)
int f, n;
{
        register LINE   *lp;

        if (n < 0)
                return (forwchar(f, -n));
        while (n--) {
                if (curwp->w_doto == 0) {
                        if ((lp=lback(curwp->w_dotp)) == curbp->b_linep)
                                return (FALSE);
                        curwp->w_dotp  = lp;
                        curwp->w_doto  = llength(lp);
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_doto--;
        }
        return (TRUE);
}

/*
 * Move the cursor to the end of the current line. Trivial. No errors.
 */
int goeol(f, n)
int f, n;
{
        curwp->w_doto  = llength(curwp->w_dotp);
        return (TRUE);
}

/*
 * Move the cursor forwards by "n" characters. If "n" is less than zero call
 * "backchar" to actually do the move. Otherwise compute the new cursor
 * location, and move ".". Error if you try and move off the end of the
 * buffer. Set the flag if the line pointer for dot changes.
 */
int forwchar(f, n)
int f, n;
{
        if (n < 0)
                return (backchar(f, -n));
        while (n--) {
                if (curwp->w_doto == llength(curwp->w_dotp)) {
                        if (curwp->w_dotp == curbp->b_linep)
                                return (FALSE);
                        curwp->w_dotp  = lforw(curwp->w_dotp);
                        curwp->w_doto  = 0;
                        curwp->w_flag |= WFMOVE;
                } else
                        curwp->w_doto++;
        }
        return (TRUE);
}

int gotoline(f, n)	/* move to a particular line.
			   argument (n) must be a positive integer for
			   this to actually do anything		*/
int f, n;
{
	register int status;	/* status return */
	char arg[NSTRING];	/* buffer to hold argument */

	/* get an argument if one doesnt exist */
	if (f == FALSE) {
		if ((status=mlreply("Line to GOTO: ", arg, NSTRING)) != TRUE) {
			mlwrite("[Aborted]");
			return(status);
		}
		n = asc_int(arg);
	}

	if (n < 1)		/* if a bogus argument...then leave */
		return(FALSE);

	/* first, we go to the start of the buffer */
        curwp->w_dotp  = lforw(curbp->b_linep);
        curwp->w_doto  = 0;
	return(forwline(f, n-1));
}

/*
 * Goto the beginning of the buffer. Massive adjustment of dot. This is
 * considered to be hard motion; it really isn't if the original value of dot
 * is the same as the new value of dot. Normally bound to "M-<".
 */
int gobob(f, n)
int f, n;
{
        curwp->w_dotp  = lforw(curbp->b_linep);
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * Move to the end of the buffer. Dot is always put at the end of the file
 * (ZJ). The standard screen code does most of the hard parts of update.
 * Bound to "M->".
 */
int goeob(f, n)
int f, n;
{
        curwp->w_dotp  = curbp->b_linep;
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * Move forward by full lines. If the number of lines to move is less than
 * zero, call the backward line function to actually do it. The last command
 * controls how the goal column is set. Bound to "C-N". No errors are
 * possible.
 */
int forwline(f, n)
int f, n;
{
        register LINE   *dlp;

        if (n < 0)
                return (backline(f, -n));

	/* flag this command as a line move */
        thisflag |= CFCPCN;

	/* if we are on the last line as we start....fail the command */
	if (curwp->w_dotp == curbp->b_linep)
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
        if ((lastflag&CFCPCN) == 0)
                curgoal = getccol(FALSE);

	/* and move the point down */
        dlp = curwp->w_dotp;
        while (n-- && dlp!=curbp->b_linep)
                dlp = lforw(dlp);

	/* reseting the current position */
        curwp->w_dotp  = dlp;
        curwp->w_doto  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

/*
 * This function is like "forwline", but goes backwards. The scheme is exactly
 * the same. Check for arguments that are less than zero and call your
 * alternate. Figure out the new line and call "movedot" to perform the
 * motion. No errors are possible. Bound to "C-P".
 */
int backline(f, n)
int f, n;
{
        register LINE   *dlp;

        if (n < 0)
                return (forwline(f, -n));


	/* flag this command as a line move */
        thisflag |= CFCPCN;

	/* if we are on the last line as we start....fail the command */
	if (lback(curwp->w_dotp) == curbp->b_linep)
		return(FALSE);

	/* if the last command was not note a line move,
	   reset the goal column */
        if ((lastflag&CFCPCN) == 0)
                curgoal = getccol(FALSE);

	/* and move the point up */
        dlp = curwp->w_dotp;
        while (n-- && lback(dlp)!=curbp->b_linep)
                dlp = lback(dlp);

	/* reseting the current position */
        curwp->w_dotp  = dlp;
        curwp->w_doto  = getgoal(dlp);
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

#if	WORDPRO
int gobop(f, n)	/* go back to the beginning of the current paragraph
		   here we look for a <NL><NL> or <NL><TAB> or <NL><SPACE>
		   combination to delimit the beginning of a paragraph	*/

int f, n;	/* default Flag & Numeric argument */

{
	register int suc;	/* success of last backchar */

	if (n < 0)	/* the other way...*/
		return(goeop(f, -n));

	while (n-- > 0) {	/* for each one asked for */

		/* first scan back until we are in a word */
		suc = backchar(FALSE, 1);
		while (!inword() && suc)
			suc = backchar(FALSE, 1);
		curwp->w_doto = 0;	/* and go to the B-O-Line */

		/* and scan back until we hit a <NL><NL> or <NL><TAB>
		   or a <NL><SPACE>					*/
		while (lback(curwp->w_dotp) != curbp->b_linep)
			if (llength(curwp->w_dotp) != 0 &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != TAB &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != ' ')
				curwp->w_dotp = lback(curwp->w_dotp);
			else
				break;

		/* and then forward until we are in a word */
		if (lback(curwp->w_dotp) == curbp->b_linep)
			suc = (curwp->w_dotp != curbp->b_linep);
		else
			suc = forwchar(FALSE, 1);
		while (suc && !inword())
			suc = forwchar(FALSE, 1);
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}

int goeop(f, n)	/* go forword to the end of the current paragraph
		   here we look for a <NL><NL> or <NL><TAB> or <NL><SPACE>
		   combination to delimit the beginning of a paragraph	*/

int f, n;	/* default Flag & Numeric argument */

{
	register int suc;	/* success of last backchar */

	if (n < 0)	/* the other way...*/
		return(gobop(f, -n));

	while (n-- > 0) {	/* for each one asked for */

		/* first scan forward until we are in a word */
		suc = TRUE; /* forwchar(FALSE, 1); */
		while (!inword() && suc)
			suc = forwchar(FALSE, 1);
		curwp->w_doto = 0;	/* and go to the B-O-Line */
		if (suc)	/* of next line if not at EOF */
			curwp->w_dotp = lforw(curwp->w_dotp);

		/* and scan forword until we hit a <NL><NL> or <NL><TAB>
		   or a <NL><SPACE>					*/
		while (curwp->w_dotp != curbp->b_linep) {
			if (llength(curwp->w_dotp) != 0 &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != TAB &&
			    lgetc(curwp->w_dotp, curwp->w_doto) != ' ')
				curwp->w_dotp = lforw(curwp->w_dotp);
			else
				break;
		}

		/* and then backward until we are in a word */
		suc = backchar(FALSE, 1);
		while (suc && !inword()) {
			suc = backchar(FALSE, 1);
		}
		curwp->w_doto = llength(curwp->w_dotp);	/* and to the EOL */
	}
	curwp->w_flag |= WFMOVE;	/* force screen update */
	return(TRUE);
}
#endif

/*
 * This routine, given a pointer to a LINE, and the current cursor goal
 * column, return the best choice for the offset. The offset is returned.
 * Used by "C-N" and "C-P".
 */
int getgoal(dlp)
LINE *dlp;
{
        register int    c;
        register int    col;
        register int    newcol;
        register int    dbo;

        col = 0;
        dbo = 0;
        while (dbo != llength(dlp)) {
                c = lgetc(dlp, dbo);
                newcol = col;
                if (c == '\t')
                        newcol += tabsize - (newcol % tabsize) - 1;
                else if ((c&0x7F) < 0x20 || (c&0x7F) == 0x7F)
                        ++newcol;
                ++newcol;
                if (newcol > curgoal)
                        break;
                col = newcol;
                ++dbo;
        }
        return (dbo);
}

/*
 * Scroll forward by a specified number of lines, or by a full page if no
 * argument. Bound to "C-V". The "2" in the arithmetic on the window size is
 * the overlap; this value is the default overlap value in ITS EMACS. Because
 * this zaps the top line in the display window, we have to do a hard update.
 */
int forwpage(f, n)
int f, n;
{
        register LINE   *lp, *dp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Forget the overlap   */
                        n = 1;                  /* if tiny window.      */
        } else if (n < 0)
                return (backpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif

	lp = curwp->w_linep;
        dp = curwp->w_dotp;
        while (n > 0 && dp != curbp->b_linep)
                {--n; lp = lforw(lp); dp = lforw(dp);}
        curwp->w_linep = lp;
        curwp->w_dotp  = dp;
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * This command is like "forwpage", but it goes backwards. The "2", like
 * above, is the overlap between the two windows. The value is from the ITS
 * EMACS manual. Bound to "M-V". We do a hard update for exactly the same
 * reason.
 */
int backpage(f, n)
int f, n;
{
        register LINE   *lp, *dp;

        if (f == FALSE) {
                n = curwp->w_ntrows - 2;        /* Default scroll.      */
                if (n <= 0)                     /* Don't blow up if the */
                        n = 1;                  /* window is tiny.      */
        } else if (n < 0)
                return (forwpage(f, -n));
#if     CVMVAS
        else                                    /* Convert from pages   */
                n *= curwp->w_ntrows;           /* to lines.            */
#endif

	lp = curwp->w_linep;
	dp = curwp->w_dotp;
        while (n > 0 && lback(lp) != curbp->b_linep)
                {--n; lp = lback(lp); dp = lback(dp);}
        while (n > 0 && lback(dp) != curbp->b_linep)
                {--n; dp = lback(dp);}
        curwp->w_linep = lp;
        curwp->w_dotp  = dp;
        curwp->w_doto  = 0;
        curwp->w_flag |= WFHARD;
        return (TRUE);
}

/*
 * Set the mark in the current window to the value of "." in the window. No
 * errors are possible. Bound to "M-.".
 */
int setmark(f, n)
int f, n;
{
        curwp->w_markp = curwp->w_dotp;
        curwp->w_marko = curwp->w_doto;
        mlwrite("[Mark set]");
        return (TRUE);
}

/*
 * Clear the mark in the current window. No errors are possible.
 * Bound to ^X <space>
 */
int remmark(f, n)
int f, n;
{
	curwp->w_markp = NULL;
	curwp->w_marko = 0;
	mlwrite("[Mark removed]");
	return (TRUE);
}

/*
 * Swap the values of "." and "mark" in the current window. This is pretty
 * easy, bacause all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible error is "no mark". Bound to
 * "C-X C-X".
 */
int swapmark(f, n)
int f, n;
{
        register LINE   *odotp;
        register int    odoto;

        if (curwp->w_markp == NULL) {
                mlwrite("No mark in this window");
                return (FALSE);
        }
        odotp = curwp->w_dotp;
        odoto = curwp->w_doto;
        curwp->w_dotp  = curwp->w_markp;
        curwp->w_doto  = curwp->w_marko;
        curwp->w_markp = odotp;
        curwp->w_marko = odoto;
        curwp->w_flag |= WFMOVE;
        return (TRUE);
}

#if	DECEDT

NOSHARE int advset = TRUE;

/* advance command.  sets advance flag true */
int advadv(f, n)
int f, n;
{
advset = TRUE;
return(TRUE);
}

/* backup command.  sets backup flag false */
int advback(f, n)
int f, n;
{
advset = FALSE;
return(TRUE);
}

/* bottom command.  goes to top or bottom of buffer */
int advbot(f, n)
int f, n;
{
return( (advset == TRUE)? goeob(f, n): gobob(f, n) );
}

/* character command.  goes forward or backward a character */
int advchar(f, n)
int f, n;
{
return( (advset == TRUE)? forwchar(f, n): backchar(f, n) );
}

/* word command.  goes forward or backward a word */
int advword(f, n)
int f, n;
{
return( (advset == TRUE)? forwword(f, n): backword(f, n) );
}

/* line command.  goes to beginning of next or previous line */
int advline(f, n)
int f, n;
{
int stat;
if (advset != TRUE && curwp->w_doto > 0) stat = TRUE;
else stat = ((advset == TRUE)? scdnwind(f, n): scupwind(f, n));
thisflag &= ~CFCPCN;
return( (stat == TRUE)? gobol(f, n): stat);
}

/* end of line command.  goes to end of next or previous line */
int adveol(f, n)
int f, n;
{
int stat;

if (advset == TRUE && curwp->w_doto < llength(curwp->w_dotp)) stat = TRUE;
else stat = ((advset == TRUE)? forwline(f, n): backline(f, n));
thisflag &= ~CFCPCN;
return( (stat == TRUE)? goeol(f, n): stat);
}

/* search command.  searches forward or reverse */
int advsearch(f, n)
int f, n;
{
return( (advset == TRUE)? forwsearch(f, n): backsearch(f, n) );
}

/* hunt command.  hunts forward or reverse */
int advhunt(f, n)
int f, n;
{
return( (advset == TRUE)? forwhunt(f, n): backhunt(f, n) );
}

/* smooth next page command.  goes forward page */
int scnxpage(f, n)
int f, n;
{
#if	1
	int s, adv;

	/* EDT-like paging */
	gobol(f, n);
	if (f == FALSE) n = 1;
	n *= (curwp->w_ntrows * 2 + 1) / 3;
	if (isvt100) {
		s = TRUE;
		adv = (n >= 0);
		if (!adv) n = -n;
		while (n-- > 0 && s == TRUE)	{
			s = (adv? scdnwind(TRUE, 1): scupwind(TRUE, 1) );
			if (s == TRUE && n > 0) update(HOOK);
		}
	}
	else
		s = scdnwind(TRUE, n);
	thisflag &= ~CFCPCN;
	return(s);
#else
	/* Emacs-like paging */
	return(forwpage(f, n));
#endif
}

/* smooth previous page command.  goes backward page */
int scprpage(f, n)
int f, n;
{
	if (f == FALSE) { f = TRUE; n = 1; }
	return( scnxpage(f, -n) );
}

/* page command.  goes forward or backward a page */
int advpage(f, n)
int f, n;
{
	return( (advset == TRUE)? scnxpage(f, n): scprpage(f, n) );
}

/*
 * Special insertion.  The argument is the ascii value of the character
 * to insert into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning.
 */
int specins(f, n)
int f, n;
{
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	n = ((f == TRUE)? (n & 0xFF): 0);
	return( (n == '\n')? lnewline(): linsert(1, n) );
}

/*
 * Undelete the last deleted character.
 */
int yankchar(f, n)
int f, n;
{
	int s;
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (ykchar == -1 || n <= 0) s = FALSE;
	else if (ykchar == '\n') {
		s = TRUE;
		while (s == TRUE && n-- > 0) s = lnewline();
	}
	else s = linsert(n, ykchar);
	return(s);
}

/*
 * Octal insertion.
 * Quote next character and insert in C backslash octal format.
 * Unbound
 */
int octalins(f, n)
int f, n;
{
	register int    s;
	register int    c;
	int i, len;
	char oct[10];

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	mlwrite(": character to insert ");
        c = tgetc() & 0xFF;
	len = 0;
	while (c > 0)	{
		oct[len++] = (char) ('0' + c % 8);
		c /= 8;
		}
	while (len < 3) oct[len++] = '0';

	s = TRUE;
	while (n-- > 0 && s == TRUE)	{
		linsert(1, '\\');
		i = len;
		while(i > 0) s = linsert(1, oct[--i]);
		}

	return(s);
}

/* null commnad.  does nothing */
int nullcmd(f, n)
int f, n;
{
return(TRUE);
}
#endif
