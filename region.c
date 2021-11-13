/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

static int regionio PP((int f, int n, CONSTA char *mode));

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 * Bound to "C-W".
 */
int killregion(f, n)
int f, n;
{
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        if ((lastflag&CFKILL) == 0)             /* This is a kill type  */
                kdelete();                      /* command, so do magic */
        thisflag |= CFKILL;                     /* kill buffer stuff.   */
        curwp->w_dotp = region.r_linep;
        curwp->w_doto = region.r_offset;
        return (ldelete(region.r_size, TRUE));
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank. Bound to "M-W".
 */
int copyregion(f, n)
int f, n;
{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        REGION          region;

        if ((s=getregion(&region)) != TRUE)
                return (s);
        if ((lastflag&CFKILL) == 0)             /* Kill type command.   */
                kdelete();
        thisflag |= CFKILL;
        linep = region.r_linep;                 /* Current line.        */
        loffs = region.r_offset;                /* Current offset.      */
        while (region.r_size-- > 0L) {
                if (loffs == llength(linep)) {  /* End of line.         */
                        if ((s=kinsert('\n')) != TRUE)
                                return (s);
                        linep = lforw(linep);
                        loffs = 0;
                } else {                        /* Middle of line.      */
                        if ((s=kinsert(lgetc(linep, loffs))) != TRUE)
                                return (s);
                        ++loffs;
                }
        }
	mlwrite("[region copied]");
        return (TRUE);
}

/*	Narrow-to-region (^X-<) makes all but the current region in
	the current buffer invisable and unchangable
*/

int narrow(f, n)
int f, n;
{
        register int status;	/* return status */
	BUFFER *bp;		/* buffer being narrowed */
	WINDOW *wp;		/* windows to fix up pointers in as well */
	REGION creg;		/* region boundry structure */

	/* find the proper buffer and make sure we aren't already narrow */
	bp = curwp->w_bufp;		/* find the right buffer */
	if (bp->b_flag&BFNAROW) {
		mlwrite("%%This buffer is already narrowed");
		return(FALSE);
	}

	/* find the boundries of the current region */
        if ((status=getregion(&creg)) != TRUE)
                return(status);
        curwp->w_dotp = creg.r_linep;	/* only by full lines please! */
        curwp->w_doto = 0;
	creg.r_size += (long)creg.r_offset;
	if (creg.r_size <= (long)curwp->w_dotp->l_used) {
		mlwrite("%%Must narrow at least 1 full line");
		return(FALSE);
	}

	/* archive the top fragment */
	if (bp->b_linep->l_fp != creg.r_linep) {
		bp->b_topline = bp->b_linep->l_fp;
		creg.r_linep->l_bp->l_fp = (LINE *)NULL;
		bp->b_linep->l_fp = creg.r_linep;
		creg.r_linep->l_bp = bp->b_linep;
	}

	/* move forward to the end of this region
	   (a long number of bytes perhaps) */
	while (creg.r_size > (long)32000) {
		forwchar(TRUE, 32000);
		creg.r_size -= (long)32000;
	}
	forwchar(TRUE, (int)creg.r_size);
	curwp->w_doto = 0;		/* only full lines! */

	/* archive the bottom fragment */
	if (bp->b_linep != curwp->w_dotp) {
		bp->b_botline = curwp->w_dotp;
		bp->b_botline->l_bp->l_fp = bp->b_linep;
		bp->b_linep->l_bp->l_fp = (LINE *)NULL;
		bp->b_linep->l_bp = bp->b_botline->l_bp;
	}

	/* let all the proper windows be updated */
	wp = wheadp;
	while (wp) {
		if (wp->w_bufp == bp) {
			wp->w_linep = creg.r_linep;
			wp->w_dotp = creg.r_linep;
			wp->w_doto = 0;
			wp->w_markp = creg.r_linep;
			wp->w_marko = 0;
			wp->w_flag |= (WFHARD|WFMODE);
		}
		wp = wp->w_wndp;
	}

	/* and now remember we are narrowed */
	bp->b_flag |= BFNAROW;
	mlwrite("[Buffer is narrowed]");
        return(TRUE);
}

/*	widen-from-region (^X->) restores a narrowed region	*/

int widen(f, n)
int f, n;
{
	LINE *lp;	/* temp line pointer */
	BUFFER *bp;	/* buffer being narrowed */
	WINDOW *wp;	/* windows to fix up pointers in as well */

	/* find the proper buffer and make sure we are narrow */
	bp = curwp->w_bufp;		/* find the right buffer */
	if ((bp->b_flag&BFNAROW) == 0) {
		mlwrite("%%This buffer is not narrowed");
		return(FALSE);
	}

	/* recover the top fragment */
	if (bp->b_topline != (LINE *)NULL) {
		lp = bp->b_topline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep->l_fp;
		lp->l_fp->l_bp = lp;
		bp->b_linep->l_fp = bp->b_topline;
		bp->b_topline->l_bp = bp->b_linep;
		bp->b_topline = (LINE *)NULL;
	}

	/* recover the bottom fragment */
	if (bp->b_botline != (LINE *)NULL) {
		lp = bp->b_botline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep;
		bp->b_linep->l_bp->l_fp = bp->b_botline;
		bp->b_botline->l_bp = bp->b_linep->l_bp;
		bp->b_linep->l_bp = lp;
		bp->b_botline = (LINE *)NULL;
	}

	/* let all the proper windows be updated */
	wp = wheadp;
	while (wp) {
		if (wp->w_bufp == bp)
			wp->w_flag |= (WFHARD|WFMODE);
		wp = wp->w_wndp;
	}
	/* and now remember we are not narrowed */
	bp->b_flag &= (~BFNAROW);
	mlwrite("[Buffer is widened]");
        return(TRUE);
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int lowerregion(f, n)
int f, n;
{
        register LINE   *linep;
        register int    loffs;
        register int    c;
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size-- > 0L) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
                        if (c>='A' && c<='Z')
                                lputc(linep, loffs, (char) (c+'a'-'A'));
                        ++loffs;
                }
        }
        return (TRUE);
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
int upperregion(f, n)
int f, n;
{
        register LINE   *linep;
        register int    loffs;
        register int    c;
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size-- > 0L) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
                        if (c>='a' && c<='z')
				lputc(linep, loffs, (char) (c-'a'+'A'));
			++loffs;
                }
        }
        return (TRUE);
}

/*
 * This routine figures out the
 * bounds of the region in the current window, and
 * fills in the fields of the "REGION" structure pointed
 * to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for
 * mark. This should save time. Return a standard code.
 * Callers of this routine should be prepared to get
 * an "ABORT" status; we might make this have the
 * conform thing later.
 */
int getregion(rp)
REGION *rp;
{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;

        if (curwp->w_markp == NULL) {
                mlwrite("No mark set in this window");
                return (FALSE);
        }
        if (curwp->w_dotp == curwp->w_markp) {
                rp->r_linep = curwp->w_dotp;
                if (curwp->w_doto < curwp->w_marko) {
                        rp->r_offset = curwp->w_doto;
                        rp->r_size = (long)(curwp->w_marko-curwp->w_doto);
                } else {
                        rp->r_offset = curwp->w_marko;
                        rp->r_size = (long)(curwp->w_doto-curwp->w_marko);
                }
                return (TRUE);
        }
        blp = curwp->w_dotp;
        bsize = (long)curwp->w_doto;
        flp = curwp->w_dotp;
        fsize = (long)(llength(flp)-curwp->w_doto+1);
        while (flp!=curbp->b_linep || lback(blp)!=curbp->b_linep) {
                if (flp != curbp->b_linep) {
                        flp = lforw(flp);
                        if (flp == curwp->w_markp) {
                                rp->r_linep = curwp->w_dotp;
                                rp->r_offset = curwp->w_doto;
                                rp->r_size = fsize+curwp->w_marko;
                                return (TRUE);
                        }
                        fsize += llength(flp)+1;
                }
                if (lback(blp) != curbp->b_linep) {
                        blp = lback(blp);
                        bsize += llength(blp)+1;
                        if (blp == curwp->w_markp) {
                                rp->r_linep = blp;
                                rp->r_offset = curwp->w_marko;
                                rp->r_size = bsize - curwp->w_marko;
                                return (TRUE);
                        }
                }
        }
        mlwrite("Bug: lost mark");
        return (FALSE);
}

#if	DECEDT

/*
 * Kill the region to the kill buffer, and then remove the mark.
 */

int cutregion(f, n)
int f, n;
{
	int s;
	s = killregion(f, n);
	if (s == TRUE) { curwp->w_markp = NULL; curwp->w_marko = 0; }
	return(s);
}

/*
 * Append the region to the kill buffer, and then kill it.
 */

int apkillregion(f, n)
int f, n;
{
	lastflag |= CFKILL;
	return(killregion(f, n));
}

/*
 * Append all of the characters in the
 * region to the kill buffer.
 */
int apcopyregion(f, n)
int f, n;
{
	lastflag |= CFKILL;
	return(copyregion(f, n));
}

/*
 * Change case in region. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers.
 */
int caseregion(f, n)
int f, n;
{
        register LINE   *linep;
        register int    loffs;
        register int    c;
        register int    s;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return (s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size-- > 0L) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
                        if (c>='a' && c<='z')
				lputc(linep, loffs, (char) (c-'a'+'A'));
			else if (c>='A' && c<='Z')
				lputc(linep, loffs, (char) (c-'A'+'a'));
                        ++loffs;
                }
        }
        return (TRUE);
}

/*
 * Ask for a file name, and write the
 * contents of the current region to that file.
 *
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */

static int regionio(f, n, mode)
int f, n;
CONSTA char *mode;
{
        register LINE   *linep;
        register int    s;
        register int    loffs, len;
        register int    nline;
        REGION          region;
	char		*fn;

	if (restflag)		/* don't allow this command if restricted */
		return(resterr());

	if ((s=getregion(&region)) != TRUE)
		return (s);

	fn = gtfilename(((*mode=='w')? "Write file": "Append file"), TRUE);
	if (fn == NULL)
		return (FALSE);

#if	CRYPT
	s = resetkey();
	if (s != TRUE)
		return(s);
#endif

	/* turn off ALL keyboard translation in case we get a dos error */
	(VOID) (*term.t_kclose)();

        if ((s=ffwopen(fn, mode)) != FIOSUC) {	/* Open writes message. */
		(VOID) (*term.t_kopen)();
                return (FALSE);
        }
	mlwrite("[Writing...]");		/* tell us were writing */
        nline = 0;                              /* Number of lines.     */
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size > 0L) {
		len = llength(linep) - loffs;
		if (len > region.r_size) len = region.r_size;
		region.r_size -= (len + 1);
                if ((s=ffputline(&linep->l_text[loffs], len)) != FIOSUC)
                        break;
                ++nline;
                linep = lforw(linep);
		loffs = 0;
        }
        if (s == FIOSUC) {                      /* No write error.      */
                s = ffwclose();
                if (s == FIOSUC) {              /* No close error.      */
                        if (nline == 1)
                                mlwrite("[Wrote 1 line]");
                        else
                                mlwrite("[Wrote %d lines]", nline);
                }
        } else                                  /* Ignore close error   */
		ffwclose();			/* if a write error.	*/
	(VOID) (*term.t_kopen)();
        if (s != FIOSUC)                        /* Some sort of error.  */
                return (FALSE);
        return (TRUE);
}

int regionwrite(f, n)
int f, n;
{
	return(regionio(f, n, "w"));
}

int regionappend(f, n)
int f, n;
{
	return(regionio(f, n, "a"));
}
#endif

