/*
 * Buffer management.
 * Some of the functions are internal,
 * and some are actually attached to user
 * keys. Like everyone else, they set hints
 * for the display system.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

static VOID me_ltoa PP((char *buf, int width, long num));

BUFFER *getdefb() /* get the default buffer for a use or kill */
{
	BUFFER *bp;	/* default buffer */

	/* Find the next buffer which will be the default */
	bp = curbp->b_bufp;
	/* cycle through the buffers to find an eligable one */
	while (bp == NULL || (bp->b_flag & BFINVS)) {
		if (bp == NULL) bp = bheadp;
		else bp = bp->b_bufp;
		/* don't get caught in an infinite loop! */
		if (bp == curbp) {bp = NULL; break;}
	}
	return(bp);
}

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
int usebuffer(f, n)
int f, n;
{
        register BUFFER *bp;

	/* make a completion for a buffer name */
	bp = getdefb();
	bp = getcbuf("Use buffer", bp ? bp->b_bname: "main", TRUE);
	if (!bp)
		return(ABORT);

	/* make it invisible if there is an argument */
	if (f == TRUE) {
		if (n != 0) bp->b_flag |= BFINVS;
		else bp->b_flag &= ~BFINVS;
	}
	return(swbuffer(bp));
}

int nextbuffer(f, n)	/* switch to the next buffer in the buffer list */

int f, n;	/* default flag, numeric argument */
{
	register BUFFER *bp;	/* eligable buffer to switch to*/
	register BUFFER *bbp;	/* eligable buffer to switch to*/
	int nbuffers;

	/* make sure the arg is legit */
	if (f == FALSE)
		n = 1;
	if (n < 1) {
		/* return(FALSE); */
		nbuffers = 0;
		bp = bheadp;
		while(bp != NULL) {
			if (!(bp->b_flag & BFINVS)) ++nbuffers;
			bp = bp->b_bufp;
		}
		if (nbuffers > 0) n = nbuffers - ((-n) % nbuffers);
	}

	bp = NULL;
	bbp = curbp;
	while (n-- > 0) {
		/* advance to the next buffer */
		bp = bbp->b_bufp;

		/* cycle through the buffers to find an eligable one */
		while (bp == NULL || bp->b_flag & BFINVS) {
			if (bp == NULL)
				bp = bheadp;
			else
				bp = bp->b_bufp;

			/* don't get caught in an infinite loop! */
			if (bp == bbp)
				return(FALSE);

		}		

		bbp = bp;
	}

	return(swbuffer(bp));
}

int prevbuffer(f, n)	/* switch to the previous buffer in the buffer list */

int f, n;	/* default flag, numeric argument */
{
	if (f == FALSE)
		{ n = 1; f = TRUE; }
	return( nextbuffer(f, -n) );
}

int swbuffer(bp)	/* make buffer BP current */

BUFFER *bp;

{
        register WINDOW *wp;

	if (!bp) return FALSE;

        if (--curbp->b_nwnd == 0) {             /* Last use.            */
                curbp->b_dotp  = curwp->w_dotp;
                curbp->b_doto  = curwp->w_doto;
                curbp->b_markp = curwp->w_markp;
                curbp->b_marko = curwp->w_marko;
		curbp->b_fcol  = curwp->w_fcol;
        }
        curbp = bp;                             /* Switch.              */
	if (curbp->b_active != TRUE) {		/* buffer not active yet*/
		/* read it in and activate it */
		readin(curbp->b_fname, ((curbp->b_mode&MDVIEW) == 0), FALSE);
		curbp->b_dotp = lforw(curbp->b_linep);
		curbp->b_doto = 0;
		curbp->b_active = TRUE;
	}
        curwp->w_bufp  = bp;
        curwp->w_linep = bp->b_linep;           /* For macros, ignored. */
        curwp->w_flag |= WFMODE|WFFORCE|WFHARD; /* Quite nasty.         */
        if (bp->b_nwnd++ == 0) {                /* First use.           */
                curwp->w_dotp  = bp->b_dotp;
                curwp->w_doto  = bp->b_doto;
                curwp->w_markp = bp->b_markp;
                curwp->w_marko = bp->b_marko;
                curwp->w_fcol  = bp->b_fcol;
                return (TRUE);
        }
        wp = wheadp;                            /* Look for old.        */
        while (wp != NULL) {
                if (wp!=curwp && wp->w_bufp==bp) {
                        curwp->w_dotp  = wp->w_dotp;
                        curwp->w_doto  = wp->w_doto;
                        curwp->w_markp = wp->w_markp;
                        curwp->w_marko = wp->w_marko;
			curwp->w_fcol  = bp->b_fcol;
                        break;
                }
                wp = wp->w_wndp;
        }
        return (TRUE);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
int killbuffer(f, n)
int f, n;

{
	register BUFFER *bp;

	UNUSED_ARGS_FN;

	/* make a completion for a buffer name */
	bp = getdefb();
	bp = getcbuf("Kill buffer", bp ? bp->b_bname: "main", TRUE);
	if (!bp) return(ABORT);

	if(bp->b_flag & BFINVS)		/* Deal with special buffers	*/
		return (TRUE);		/* by doing nothing.	*/
	return(zotbuf(bp));
}

/*	Allow the user to pop up a buffer, like we do.... */

int PASCAL NEAR popbuffer(f, n)

int f, n;	/* default and numeric arguments */

{
	register BUFFER *bp;	/* ptr to buffer to dump */

	UNUSED_ARGS_FN;

	/* get the buffer name to pop */
	bp = getdefb();
	bp = getcbuf("Pop buffer", bp ? bp->b_bname : "main", TRUE);
	if (bp == NULL)
		return(ABORT);

	return(pop(bp));
}

int zotbuf(bp)	/* kill the buffer pointed to by bp */

BUFFER *bp;

{
        register BUFFER *bp1;
        register BUFFER *bp2;
        register int    s;

        if (bp->b_nwnd != 0) {                  /* Error if on screen.  */
                mlwrite("Buffer is being displayed");
                return (FALSE);
        }
        if ((s=bclear(bp)) != TRUE)             /* Blow text away.      */
                return (s);
        free((char *) bp->b_linep);             /* Release header line. */
        bp1 = NULL;                             /* Find the header.     */
        bp2 = bheadp;
        while (bp2 != bp) {
                bp1 = bp2;
                bp2 = bp2->b_bufp;
        }
        bp2 = bp2->b_bufp;                      /* Next one in chain.   */
        if (bp1 == NULL)                        /* Unlink it.           */
                bheadp = bp2;
        else
                bp1->b_bufp = bp2;
        free((char *) bp);                      /* Release buffer block */
        return (TRUE);
}

int namebuffer(f,n)	/*	Rename the current buffer	*/

int f, n;		/* default Flag & Numeric arg */

{
	register BUFFER *bp;	/* pointer to scan through all buffers */
	char bufn[NBUFN];	/* buffer to hold buffer name */

	UNUSED_ARGS_FN;

	/* prompt for and get the new buffer name */
ask:	if (mlreply("Change buffer name to: ", bufn, NBUFN) != TRUE)
		return(FALSE);

	/* and check for duplicates */
	bp = bheadp;
	while (bp != NULL) {
		if (bp != curbp) {
			/* if the names the same */
			if (strcmp(bufn, bp->b_bname) == 0)
				goto ask;  /* try again */
		}
		bp = bp->b_bufp;	/* onward */
	}

	strcpy(curbp->b_bname, bufn);	/* copy buffer name to structure */
	curwp->w_flag |= WFMODE;	/* make mode line replot */
	mlerase();
	return(TRUE);
}

/*
	List all of the active buffers.  First update the special
	buffer that holds the list.  Next make sure at least 1
	window is displaying the buffer list, splitting the screen
	if this is what it takes.  Lastly, repaint all of the
	windows that are displaying the list.  Bound to "C-X C-B".
	A numeric argument forces it to list invisable buffers as
	well.
*/

int listbuffers(f, n)
int f, n;
{
        register int    s;

	UNUSED_ARGS_FN;

        if ((s=makelist("", f)) != TRUE)
                return (s);
	return(wpopup(blistp));
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory). Iflag
 * indicates whether to list hidden buffers.
 */
int makelist(name, iflag)
CONSTA char *name;
int iflag;	/* list hidden buffer flag */
{
#define BUF_SIZ_LEN	8	/* digits in buffer size */
#define	BUF_LIST_LEN	256	/* max buffer list line length */
        register char   *cp1;
        register char   *cp2;
        register int    c;
        register BUFFER *bp;
        register LINE   *lp;
        register int    s;
	register int	i;
        long nbytes;		/* # of bytes in current buffer */
        int namelen, len;
        char b[BUF_SIZ_LEN+1+1];
        char line[BUF_LIST_LEN + NUMMODES + BUF_SIZ_LEN + NBUFN + 10];	/* only need BUF_LIST_LEN unless NBUFN is huge */

        blistp->b_flag &= ~BFCHG;               /* Don't complain!      */
        if ((s=bclear(blistp)) != TRUE)         /* Blow old text away   */
                return (s);
        strcpy(blistp->b_fname, "");

	cp1 = line;
	strcpy(cp1, "ACTN MODES           Size Buffer");
	cp1 += 32;
	memset(cp1, ' ', NBUFN - 6);
	cp1 += NBUFN - 6;
	strcpy(cp1, "File");
	if (addline(blistp, line) == FALSE) return FALSE;

	for (cp1 = line; *cp1 != '\0'; cp1++) {
		if (*cp1 != ' ') *cp1 = '-';
	}
	if (addline(blistp, line) == FALSE) return FALSE;
#if 0
        if (addline(blistp, "ACTN MODES           Size Buffer          File") ==
		FALSE
        ||  addline(blistp, "---- -----           ---- ------          ----") ==
		FALSE)
                return (FALSE);
#endif
        bp = bheadp;                            /* For all buffers      */

	/* build line to report global mode settings */
	cp1 = &line[0];
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';

	/* output the mode codes */
	for (i = 0; i < NUMMODES; i++)
		if (gmode & (1 << i))
			*cp1++ = modecode[i];
		else
			*cp1++ = '.';
	memset(cp1, ' ', BUF_SIZ_LEN+3);
	cp1 += BUF_SIZ_LEN+3;
	strcpy(cp1, "Global Modes");
	if (addline(blistp, line) == FALSE)
		return(FALSE);

	namelen = (int) strlen(name);

	/* output the list of buffers */
        while (bp != NULL) {
		if (namelen > 0) {
			len = (int) strlen(bp->b_bname);
			if (len > namelen) len = namelen;
			if (strncmp(name, bp->b_bname, len)) {
				bp = bp->b_bufp;
				continue;
			}
		}

		/* skip invisible buffers if iflag is false */
                if (((bp->b_flag&BFINVS) != 0) && (iflag != TRUE)) {
                        bp = bp->b_bufp;
                        continue;
                }
                cp1 = &line[0];                 /* Start at left edge   */

		/* output status of ACTIVE flag (has the file been read in? */
                if (bp->b_active == TRUE)    /* "@" if activated       */
                        *cp1++ = '@';
                else
                        *cp1++ = ' ';

		/* output status of changed flag */
                if ((bp->b_flag&BFCHG) != 0)    /* "*" if changed       */
                        *cp1++ = '*';
                else
                        *cp1++ = ' ';

		/* output status of truncated flag */
                if ((bp->b_flag&BFTRUNC) != 0)    /* "#" if truncated       */
                        *cp1++ = '#';
                else
                        *cp1++ = ' ';

		/* output status of narrowed flag */
                if ((bp->b_flag&BFNAROW) != 0)    /* "<" if narrowed */
                        *cp1++ = '<';
                else
                        *cp1++ = ' ';

                *cp1++ = ' ';                   /* Gap.                 */

		/* output the mode codes */
		for (i = 0; i < NUMMODES; i++) {
			if (bp->b_mode & (1 << i))
				*cp1++ = modecode[i];
			else
				*cp1++ = '.';
		}
                *cp1++ = ' ';                   /* Gap.                 */
                nbytes = 0L;                    /* Count bytes in buf.  */
                lp = lforw(bp->b_linep);
                while (lp != bp->b_linep) {
                        nbytes += (long)llength(lp)+1L;
                        lp = lforw(lp);
                }
                me_ltoa(b, BUF_SIZ_LEN+1, nbytes); /* 8 digit buffer size. */
                cp2 = &b[0];
                while ((c = *cp2++) != 0)
                        *cp1++ = (char) c;
                *cp1++ = ' ';                   /* Gap.                 */
                cp2 = &bp->b_bname[0];          /* Buffer name          */
                while ((c = *cp2++) != 0)
                        *cp1++ = (char) c;
                cp2 = &bp->b_fname[0];          /* File name            */
                if (*cp2 != 0) {
                        while (cp1 < &line[4+1+NUMMODES+1+BUF_SIZ_LEN+1+NBUFN+1])
                                *cp1++ = ' ';
                        while ((c = *cp2++) != 0) {
                                if (cp1 < &line[BUF_LIST_LEN-1])
                                        *cp1++ = (char) c;
                        }
                }
                *cp1 = 0;                       /* Add to the buffer.   */
                if (addline(blistp, line) == FALSE)
                        return (FALSE);
                bp = bp->b_bufp;
        }
        return (TRUE);                          /* All done             */
}

#ifdef NO_PROTOTYPE
static VOID me_ltoa(buf, width, num)
	char   *buf;
	int    width;
	long   num;
#else
static VOID me_ltoa(char *buf, int width, long num)
#endif

{
        buf[width] = 0;                         /* End of string.       */
        while (num >= 10 && width > 0) {	/* Conditional digits.	*/
                buf[--width] = (char) ((num%10L) + '0');
                num /= 10L;
        }
	if (width > 0) {
		buf[--width] = (char) (num + '0'); /* Always 1 digit.	*/
	}
        while (width > 0) {			/* Pad with blanks.	*/
		buf[--width] = ' ';
	}
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
int addline(bp, text)
BUFFER *bp;	/* buffer to add text to */
CONSTA char    *text;
{
        register LINE   *lp;
        register int    i;
        register int    ntext;

	/* allocate the memory to hold the line */
        ntext = (int) strlen(text);
        if ((lp=lalloc(ntext)) == NULL)
                return (FALSE);

	/* copy the text into the new line */
        for (i=0; i<ntext; ++i)
                lputc(lp, i, text[i]);

	/* add the new line to the end of the buffer */
        bp->b_linep->l_bp->l_fp = lp;		/* Hook onto the end    */
        lp->l_bp = bp->b_linep->l_bp;
        bp->b_linep->l_bp = lp;
        lp->l_fp = bp->b_linep;
        if (bp->b_dotp == bp->b_linep)		/* If "." is at the end */
                bp->b_dotp = lp;		/* move it to new line  */
        return (TRUE);
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
int anycb()
{
        register BUFFER *bp;

        bp = bheadp;
        while (bp != NULL) {
                if ((bp->b_flag&BFINVS)==0 && (bp->b_flag&BFCHG)!=0)
                        return (TRUE);
                bp = bp->b_bufp;
        }
        return (FALSE);
}

/*
 * Find a buffer, by name. Return a pointer
 * to the BUFFER structure associated with it.
 * If the buffer is not found
 * and the "cflag" is TRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
BUFFER  *
bfind(bname, cflag, bflag)
CONSTA char   *bname;
int	cflag, bflag;
{
        register BUFFER *bp;
	register BUFFER *sb;	/* buffer to insert after */
        register LINE   *lp;
        char bufname[NBUFN];

        bytecopy(bufname, bname, NBUFN-1);
        bp = bheadp;
        while (bp != NULL) {
                if (strcmp(bufname, bp->b_bname) == 0)
                        return (bp);
                bp = bp->b_bufp;
        }
        if (cflag != FALSE) {
                if ((bp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
                        return (NULL);
                if ((lp=lalloc(0)) == NULL) {
                        free((char *) bp);
                        return (NULL);
                }
		/* find the place in the list to insert this buffer */
		if (bheadp == NULL || strcmp(bheadp->b_bname, bufname) > 0) {
			/* insert at the beginning */
	                bp->b_bufp = bheadp;
        	        bheadp = bp;
        	} else {
			sb = bheadp;
			while (sb->b_bufp != NULL) {
				if (strcmp(sb->b_bufp->b_bname, bufname) > 0)
					break;
				sb = sb->b_bufp;
			}

			/* and insert it */
       			bp->b_bufp = sb->b_bufp;
        		sb->b_bufp = bp;
       		}

		/* and set up the other buffer fields */
		bp->b_topline = NULL;
		bp->b_botline = NULL;
		bp->b_active = TRUE;
                bp->b_dotp  = lp;
                bp->b_doto  = 0;
                bp->b_markp = NULL;
                bp->b_marko = 0;
                bp->b_fcol  = 0;
		bp->b_flag  = (shrt) bflag;
		bp->b_mode  = gmode;
                bp->b_nwnd  = 0;
                bp->b_linep = lp;
                strcpy(bp->b_fname, "");
                strcpy(bp->b_bname, bufname);
		bp->b_fmode = 0666;
#if	VMS
		bp->b_fab_rfm = 2;	/* FAB$C_VAR */
		bp->b_fab_rat = 2;	/* FAB$M_CR */
		bp->b_fab_fsz = 0;	/* header size */
		bp->b_fab_mrs = 32767;	/* max record size */
#endif
#if	CRYPT
		bp->b_key[0] = 0;
#endif
                lp->l_fp = lp;
                lp->l_bp = lp;
        }
        return (bp);
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
int
bclear(bp)
	BUFFER *bp;
{
        register LINE   *lp;
        register int    s;

        if ((bp->b_flag&BFINVS) == 0            /* Not scratch buffer.  */
        && (bp->b_flag&BFCHG) != 0              /* Something changed    */
        && (s=mlyesno("Discard changes")) != TRUE)
                return (s);
        bp->b_flag  &= ~BFCHG;                  /* Not changed          */
        while ((lp=lforw(bp->b_linep)) != bp->b_linep)
                lfree(lp);
        bp->b_dotp  = bp->b_linep;              /* Fix "."              */
        bp->b_doto  = 0;
        bp->b_markp = NULL;                     /* Invalidate "mark"    */
        bp->b_marko = 0;
        bp->b_fcol  = 0;
        return (TRUE);
}

int
unmark(f, n)	/* unmark the current buffers change flag */

int f, n;	/* unused command arguments */

{
	UNUSED_ARGS_FN;
	curbp->b_flag &= ~BFCHG;
	upmode();
	return(TRUE);
}

#if	CALLABLE
VOID frallbuffers()	/* free all buffers */
{
	while (bheadp != NULL) {
		bheadp->b_flag &= ~BFCHG;	/* clear changed mark */
		if (zotbuf(bheadp) == FALSE) break;
	}
}
#endif

