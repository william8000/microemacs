/*
 * The functions in this file implement commands that search in the forward
 * and backward directions.  There are no special characters in the search
 * strings.  Probably should have a regular expression search, or something
 * like that.
 *
 * Aug. 1986 John M. Gamble:
 *	Made forward and reverse search use the same scan routine.
 *
 *	Added a limited number of regular expressions - 'any',
 *	'character class', 'closure', 'beginning of line', and
 *	'end of line'.
 *
 *	Replacement metacharacters will have to wait for a re-write of
 *	the replaces function, and a new variation of ldelete().
 *
 *	For those curious as to my references, i made use of
 *	Kernighan & Plauger's "Software Tools."
 *	I deliberately did not look at any published grep or editor
 *	source (aside from this one) for inspiration.  I did make use of
 *	Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	June, 1985 and modified them for the limited needs of character class
 *	matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	are therefore my own responsibility.
 *
 * April 1987: John M. Gamble
 *	Deleted the "if (n == 0) n = 1;" statements in front of the
 *	search/hunt routines.  Since we now use a do loop, these
 *	checks are unnecessary.  Consolidated common code into the
 *	function delins().  Renamed global mclen matchlen,
 *	and added the globals matchline, matchoff, patmatch, and
 *	mlenold.
 *	This gave us the ability to unreplace regular expression searches,
 *	and to put the matched string into an evironment variable.
 *	SOON TO COME: Meta-replacement characters!
 *
 *	25-apr-87	DML
 *	- cleaned up an unneccessary if/else in forwsearch() and
 *	  backsearch()
 *	- savematch() failed to malloc room for the terminating byte
 *	  of the match string (stomp...stomp...). It does now. Also
 *	  it now returns gracefully if malloc fails
 *
 *	July 1987: John M. Gamble
 *	Set the variables matchlen and matchoff in the 'unreplace'
 *	section of replaces().  It would confuse the function
 *	savematch() if you replaced, unreplaced, then replaced
 *	again (serves you right for being so wishy-washy...)
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if	MAGIC
#define	MCNIL		0	/* Like the '\0' for strings.*/
#define	LITCHAR		1
#define	ANY		2
#define	CCL		3
#define	NCCL		4
#define	BOL		5
#define	EOL		6
#define	DITTO		7
#define	CLOSURE		0x0100	/* An or-able value for a closure modifier.*/
#define	CLOSURE_1	0x0200	/* An or-able value for a closure modifier.*/
#define	ZEROONE		0x0400	/* An or-able value for a closure modifier.*/

#define ALLCLOS		(CLOSURE | CLOSURE_1 | ZEROONE)
#define	MASKCLO		0x00ff

#define	MC_ANY		'.'	/* 'Any' character (except newline).*/
#define	MC_CCL		'['	/* Character class.*/
#define	MC_NCCL		'^'	/* Negate character class.*/
#define	MC_RCCL		'-'	/* Range in character class.*/
#define	MC_ECCL		']'	/* End of character class.*/
#define	MC_BOL		'^'	/* Beginning of line.*/
#define	MC_EOL		'$'	/* End of line.*/
#define	MC_CLOSURE	'*'	/* Closure - does not extend past newline.*/
#define	MC_CLOSURE_1	'+'	/* Closure - one to many characters match.*/
#define	MC_ZEROONE	'?'	/* Closure - zero to one characters match.*/
#define	MC_DITTO	'&'	/* Use matched string in replacement */
#define	MC_ESC		'\\'	/* Escape - suppress meta-meaning.*/

#define	BIT(n)		(1 << (n))	/* An integer with one bit set.*/

#endif

#if	LATTICE | C86 | DECUSC
#define	void	int
#endif

#if	C86
#define	STATIC	/* */
#else
#if	DECUSC
#define	STATIC	extern
#else
#define	STATIC	static
#endif
#endif

	/* UCAST must map a possibly signed char to an unsigned value */
	/* In standard C, (unsigned int) will sign extend first like (int) */
	/* Not all compilers understand (unsigned char) */

#if	LATTICE | C86 | DECUSC | BSD29 | V7 | CPM | WMCS
#define	UCAST	(unsigned int)
#else
#define	UCAST	(unsigned char)
#endif

STATIC int    amatch PP((register MC *mcptr, int direct, LINE **pcwline, int *pcwoff));
STATIC int    readpattern PP((CONSTA char *prompt, char *apat, int srch));
STATIC int    replaces PP((int kind, int f, int n));
STATIC int    nextch PP((LINE **pcurline, int *pcuroff, int dir));
STATIC int    mcstr PP((void));
STATIC int    mceq PP((int bc, MC *mt));
STATIC int    cclmake PP((char **ppatptr, MC *mcptr));
STATIC int    biteq PP((int bc, BITMAP cclmap));
STATIC int	boundry PP((LINE *curline, int curoff, int dir));
STATIC VOID	savematch PP((void));
STATIC int	fbound PP((int jump, LINE **pcurline, int *pcuroff, int dir));
STATIC int	delins PP((int dlength, char *instr, int usemeta));

#if	MAGIC
STATIC void     setbit PP((int bc, BITMAP cclmap));
STATIC BITMAP   clearbits PP((void));
STATIC int mcscanner PP((MC *mcpatrn, int direct, int beg_or_end));
STATIC VOID rmcclear PP((void));
STATIC int rmcstr PP((void));
#endif

#if	DECUSC
#define	STATIC	/* */
#endif

static int FAR deltaf[HICHAR];
static int FAR deltab[HICHAR];
static int lastchfjump, lastchbjump;
static int patlenadd;

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */

int forwsearch(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status;

	status = TRUE;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return(backsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Search", &pat[0], TRUE)) == TRUE) {
		do {
#if	MAGIC
			if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
				status = mcscanner(&mcpat[0], FORWARD, PTEND);
			else
#endif
				status = scanner(&pat[0], FORWARD, PTEND);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status != TRUE)
			mlwrite("Not found");
	}
	return(status);
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */

int forwhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status;

	status = TRUE;

	if (n < 0)		/* search backwards */
		return(backhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (pat[0] == '\0')
	{
		mlwrite("No pattern set");
		return (FALSE);
	}
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) != 0 &&
		 mcpat[0].mc_type == MCNIL)
	{
		if (!mcstr())
			return (FALSE);
	}
#endif

	/* Search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do
	{
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(&mcpat[0], FORWARD, PTEND);
		else
#endif
			status = scanner(&pat[0], FORWARD, PTEND);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status != TRUE)
		mlwrite("Not found");

	return(status);
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int backsearch(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int status;

	status = TRUE;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return(forwsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Reverse search", &pat[0], TRUE)) == TRUE) {
		do {
#if	MAGIC
			if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
				status = mcscanner(&tapcm[0], REVERSE, PTBEG);
			else
#endif
				status = scanner(&tap[0], REVERSE, PTBEG);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status != TRUE)
			mlwrite("Not found");
	}
	return(status);
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int backhunt(f, n)

int f, n;	/* default flag / numeric argument */

{
	register int	status;

	status = TRUE;

	if (n < 0)
		return(forwhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (tap[0] == '\0')
	{
		mlwrite("No pattern set");
		return (FALSE);
	}
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) != 0 &&
		 tapcm[0].mc_type == MCNIL)
	{
		if (!mcstr())
			return (FALSE);
	}
#endif

	/* Go search for it for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do
	{
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(&tapcm[0], REVERSE, PTBEG);
		else
#endif
			status = scanner(&tap[0], REVERSE, PTBEG);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status != TRUE)
		mlwrite("Not found");

	return(status);
}

#if	MAGIC
/*
 * mcscanner -- Search for a meta-pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 */
STATIC int mcscanner(mcpatrn, direct, beg_or_end)
MC	*mcpatrn;		/* pointer into pattern */
int	direct;		/* which way to go.*/
int	beg_or_end;	/* put point at beginning or end of pattern.*/
{
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/*
	 * Save the old matchlen length, in case it is
	 * horribly different (closure) from the old length.
	 * This is terribly important for query-replace undo
	 * command.
	 */
	mlenold = mtchlen;

	/* Setup local scan pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff  = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct))
	{
		/* Save the current position in case we need to
		 * restore it on a match, and initialize matchlen to
		 * zero in case we are doing a search for replacement.
		 */
		matchline = curline;
		matchoff = curoff;
		mtchlen = 0;

		if (amatch(mcpatrn, direct, &curline, &curoff))
		{
			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers.
			 */
			if (beg_or_end == PTEND)	/* at end of string */
			{
				curwp->w_dotp = curline;
				curwp->w_doto = curoff;
			}
			else		/* at beginning of string */
			{
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE; /* flag that we have moved */
			savematch();
			return (TRUE);
		}

		/* Advance the cursor.
		 */
		nextch(&curline, &curoff, direct);
	}

	return (FALSE);	/* We could not find a match.*/
}

/*
 * amatch -- Search for a meta-pattern in either direction.  Based on the
 *	recursive routine amatch() (for "anchored match") in
 *	Kernighan & Plauger's "Software Tools".
 */
STATIC int	amatch(mcptr, direct, pcwline, pcwoff)
register MC	*mcptr;	/* string to scan for */
int		direct;		/* which way to go.*/
LINE		**pcwline;	/* current line during scan */
int		*pcwoff;	/* position within current line */
{
	register int c;			/* character at current position */
	LINE *curline;			/* current line during scan */
	int curoff;			/* position within current line */
	int	nchars;		/* number of chars matched in closure.*/
	int	cl_min;		/* minimum number of chars matched in closure.*/
	int	cl_type;	/* Which closure type?.*/

	/* Set up local scan pointers to ".", and get
	 * the current character.  Then loop around
	 * the pattern pointer until success or failure.
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	/* The beginning-of-line and end-of-line metacharacters
	 * do not compare against characters, they compare
	 * against positions.
	 * BOL is guaranteed to be at the start of the pattern
	 * for forward searches, and at the end of the pattern
	 * for reverse searches.  The reverse is true for EOL.
	 * So, for a start, we check for them on entry.
	 */
	if (mcptr->mc_type == BOL)
	{
		if (curoff != 0)
			return (FALSE);
		mcptr++;
	}

	if (mcptr->mc_type == EOL)
	{
		if (curoff != llength(curline))
			return (FALSE);
		mcptr++;
	}

	while (mcptr->mc_type != MCNIL)
	{
		c = nextch(&curline, &curoff, direct);

		/* Is the current meta-character modified
		 * by a closure?
		 */
		if ((cl_type = (mcptr->mc_type & ALLCLOS)) != 0)
		{
			/* Try to match as many characters as possible
			 * against the current meta-character.  A
			 * newline never matches a closure.
			 */
			nchars = 0;
			cl_min = 0;
			if (cl_type == ZEROONE) {
				if (mceq(c, mcptr)) {
					nextch(&curline, &curoff, direct);
					nchars++;
				}
			} else {
				/* Match as many characters as possible
				 * against the current meta-character.
				 */
				while (mceq(c, mcptr)) {
					c = nextch(&curline, &curoff, direct);
					nchars++;
				}

				/* Minimum number of characters that may
				 * match is 0 or 1.
				 */
				if (cl_type == CLOSURE_1) cl_min = 1;
			}

			/* We are now at the character that made us
			 * fail.  Try to match the rest of the pattern.
			 * Shrink the closure by one for each failure.
			 */
			mcptr++;
			for (;;) {
				if (nchars < cl_min)
					return FALSE;

				nextch(&curline, &curoff, direct ^ REVERSE);

				if (amatch(mcptr, direct, &curline, &curoff)) {
					mtchlen += nchars;
					goto success;
				}
				nchars--;
			}
		}
		else			/* Not closure.*/
		{
			/* The only way we'd get a BOL metacharacter
			 * at this point is at the end of the reversed pattern.
			 * The only way we'd get an EOL metacharacter
			 * here is at the end of a regular pattern.
			 * So if we match one or the other, and are at
			 * the appropriate position, we are guaranteed success
			 * (since the next pattern character has to be MCNIL).
			 * Before we report success, however, we back up by
			 * one character, so as to leave the cursor in the
			 * correct position.  For example, a search for ")$"
			 * will leave the cursor at the end of the line, while
			 * a search for ")<NL>" will leave the cursor at the
			 * beginning of the next line.  This follows the
			 * notion that the meta-character '$' (and likewise
			 * '^') match positions, not characters.
			 */
			if (mcptr->mc_type == BOL) {
				if (curoff == llength(curline))
				{
					nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				}
				else
					return (FALSE);
			}

			if (mcptr->mc_type == EOL) {
				if (curoff == 0)
				{
					nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				}
				else
					return (FALSE);
			}

			/* Neither BOL nor EOL, so go through
			 * the meta-character equal function.
			 */
			if (!mceq(c, mcptr))
				return (FALSE);
		}

		/* Increment the length counter and
		 * advance the pattern pointer.
		 */
		mtchlen++;
		mcptr++;
	}			/* End of mcptr loop.*/

	/* A SUCCESSFULL MATCH!!!
	 * Reset the "." pointers.
	 */
success:
	*pcwline = curline;
	*pcwoff  = curoff;

	return (TRUE);
}
#endif

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *	Fast version using simplified version of Boyer and Moore
 *	Software-Practice and Experience, vol 10, 501-506 (1980)
 */
int scanner(patrn, direct, beg_or_end)
char	*patrn;		/* string to scan for */
int	direct;		/* which way to go.*/
int	beg_or_end;	/* put point at beginning or end of pattern.*/
{
	register int	c;		/* character at current position */
	register char	*patptr;	/* pointer into pattern */
	LINE	*curline;		/* current line during scan */
	int	curoff;			/* position within current line */
	LINE	*scanline;		/* current line during scanning */
	int	scanoff;		/* position in scanned line */
	int	jump;			/* next offset */
	int	bc;

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/* Set up local pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	if (direct == FORWARD && curline == curbp->b_linep) return(FALSE);

	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	jump = patlenadd;

	while (!fbound(jump, &curline, &curoff, direct))
	{
		/* Save the current position in case we match
		 * the search string at this point.
		 */
		matchline = curline;
		matchoff = curoff;

		/* Setup scanning pointers.
		 */
		scanline = curline;
		scanoff = curoff;
		patptr = &patrn[0];

		/* Scan through the pattern for a match.
		 */
		while ((c = UCAST *patptr++) != '\0') {
			/* if(!me_eq((int)c,nextch(&scanline,&scanoff,direct))) */
			bc = UCAST nextch(&scanline,&scanoff,direct);
			if ((curwp->w_bufp->b_mode & MDEXACT) == 0)
			{
				if (islower(c)) c ^= DIFCASE;
				if (islower(bc)) bc ^= DIFCASE;
			}
			if (c != bc)
			{
				jump = (direct == FORWARD)
					? lastchfjump
					: lastchbjump;
				goto fail;
			}
		}

		/* A SUCCESSFULL MATCH!!!
		 * reset the global "." pointers
		 */
		if (beg_or_end == PTEND)	/* at end of string */
		{
			curwp->w_dotp = scanline;
			curwp->w_doto = scanoff;
		}
		else		/* at beginning of string */
		{
			curwp->w_dotp = matchline;
			curwp->w_doto = matchoff;
		}

		curwp->w_flag |= WFMOVE; /* Flag that we have moved.*/
		savematch();
		return TRUE;

fail:;			/* continue to search */
	}

	return FALSE;	/* We could not find a match */
}

/*
 * fbound -- Return information depending on whether we have hit a boundry
 *	(and may therefore search no further) or if a trailing character
 *	of the search string has been found.  See boundry() for search
 *	restrictions.
 */
static int fbound(jump, pcurline, pcuroff, dir)
LINE	**pcurline;
int	*pcuroff, dir, jump;
{
	register int spare, curoff;
	register LINE	*curline;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD)
	{
		while (jump != 0)
		{
			curoff += jump;
			spare = curoff - llength(curline);

			while (spare > 0)
			{
				curline = lforw(curline);/* skip to next line */
				curoff = spare - 1;
				spare = curoff - llength(curline);
				if (curline == curbp->b_linep)
					return TRUE;	/* hit end of buffer */
			}

			if (spare == 0)
			  jump = deltaf[UCAST '\n'];
			else
			  jump = deltaf[UCAST lgetc(curline, curoff)];
		}

		/* The last character matches, so back up to start
		 * of possible match.
		 */
		curoff -= patlenadd;

		while (curoff < 0)
		{
			curline = lback(curline);/* skip back a line */
			curoff += llength(curline) + 1;
		}

	}
	else			/* Reverse.*/
	{
		jump++;		/* allow for offset in reverse */
		while (jump != 0)
		{
			curoff -= jump;
			while (curoff < 0)
			{
				curline = lback(curline);	/* skip back a line */
				curoff += llength(curline) + 1;
				if (curline == curbp->b_linep)
					return TRUE;	/* hit end of buffer */
			}

			if (curoff == llength(curline))
			  jump = deltab[UCAST '\n'];
			else
			  jump = deltab[UCAST lgetc(curline, curoff)];
		}

		/* The last character matches, so back up to start
		 * of possible match.
		 */
		curoff += mtchlen;
		spare = curoff - llength(curline);
		while (spare > 0)
		{
	   		curline = lforw(curline);/* skip back a line */
	   		curoff = spare - 1;
			spare = curoff - llength(curline);
		}
	}

	*pcurline = curline;
	*pcuroff = curoff;
	return FALSE;
}


/*
 * setjtable -- Settting up search delta forward and delta backward
 *	tables.  The reverse search string and string lengths are
 *	set here, for table initialization and for substitution
 *	purposes.  The default for any character to jump is the
 *	pattern length.
 */
VOID setjtable()
{
	char *apat;
	int		i;

	apat = pat;

	rvstrcpy(tap, apat);
	patlenadd = (mlenold = mtchlen = (int) strlen(apat)) - 1;

	for (i = 0; i < HICHAR; i++)
	{
		deltaf[i] = mtchlen;
		deltab[i] = mtchlen;
	}

	/* Now put in the characters contained
	 * in the pattern, duplicating the CASE
	 */
	for (i = 0; i < patlenadd; i++)
	{
#if	0
		/*
		 * Debugging & tracing information.
		 */
		mlwrite("Considering %d with jump %d",
				UCAST apat[i], patlenadd - i);
		tgetc();
		if (isletter(apat[i]))
		{
			mlwrite("Its other case is %d",
				CHCASE[UCAST apat[i]));
			tgetc();
		}
#endif
		deltaf[UCAST apat[i]] = patlenadd - i;
		if (isletter(apat[i]))
			deltaf[UCAST CHCASE(UCAST apat[i])]
			 = patlenadd - i;

		deltab[UCAST tap[i]] = patlenadd - i;
		if (isletter(tap[i]))
			deltab[UCAST CHCASE(UCAST tap[i])]
			 = patlenadd - i;
	}

	/* The last character will have the pattern length
	 * unless there are duplicates of it.  Get the number to
	 * jump from the arrays delta, and overwrite with zeroes in delta
	 * duplicating the CASE.
	 */
	lastchfjump = patlenadd + deltaf[UCAST apat[patlenadd]];
	lastchbjump = patlenadd + deltab[UCAST apat[0]];

	deltaf[UCAST apat[patlenadd]] = 0;
	if (isletter(apat[patlenadd]))
		deltaf[UCAST CHCASE(apat[patlenadd])] = 0;

	deltab[UCAST apat[0]] = 0;
	if (isletter(apat[0]))
		deltab[UCAST CHCASE(apat[0])] = 0;
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int	me_eq(bc, pc)
register int	bc;
register int	pc;
{
	if ((curwp->w_bufp->b_mode & MDEXACT) == 0)
	{
		if (islower(bc))
			bc ^= DIFCASE;

		if (islower(pc))
			pc ^= DIFCASE;
	}

	return (bc == pc);
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.  If it is the
 *	search string, create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and defined that way).
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.  Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
STATIC int	readpattern(prompt, apat, srch)
CONSTA char	*prompt;
char	apat[];
int	srch;
{
	int status;
	char tpat[NPAT+20];

	strcpy(tpat, prompt);	/* copy prompt to output string */
	strcat(tpat, " [");	/* build new prompt string */
	expandp(&apat[0], &tpat[strlen(tpat)], NPAT/2);	/* add old pattern */
	strcat(tpat, "]<META>: ");

	/* Read a pattern.  Either we get one,
	 * or we just get the META charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mltreply(tpat, tpat, NPAT, metac)) == TRUE)
	{
		strcpy(apat, tpat);
		if (srch)	/* If we are doing the search string.*/
		{
			/* If we are doing the search string set the
			 * delta tables.
			 */
			setjtable();
		}
#if	MAGIC
		/* Only make the meta-pattern if in magic mode,
		 * since the pattern in question might have an
		 * invalid meta combination.
		 */
		if ((curwp->w_bufp->b_mode & MDMAGIC) == 0) {
			mcclear();
			rmcclear();
		}
		else
			status = (srch? mcstr(): rmcstr());
#endif
	}
	else if (status == FALSE && apat[0] != 0)	/* Old one */
		status = TRUE;

	return(status);
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */

static VOID savematch()
{
	register char *ptr;	/* ptr into malloced last match string */
	register unsigned int j; /* index */
	LINE *curline;		/* line of last match */
	int curoff;		/* offset "      "    */

	/* free any existing match string */
	if (patmatch != NULL)
		free(patmatch);

	/* attempt to allocate a new one */
	ptr = patmatch = malloc(mtchlen + 1);
	if (ptr == NULL)
		return;

	/* save the match! */
	curoff = matchoff;
	curline = matchline;

	for (j = 0; j < mtchlen; j++)
		*ptr++ = (char) nextch(&curline, &curoff, FORWARD);

	/* null terminate the match string */
	*ptr = '\0';
}

/*
 * rvstrcpy -- Reverse string copy.
 */
VOID rvstrcpy(rvstr, str)
register char	*rvstr, *str;
{
	register int i;

	str += (i = (int) strlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

/*
 * sreplace -- Search and replace.
 */
int sreplace(f, n)

int f;		/* default flag */
int n;		/* # of repetitions wanted */

{
	return(replaces(FALSE, f, n));
}

/*
 * qreplace -- search and replace with query.
 */
int qreplace(f, n)
int f;		/* default flag */
int n;		/* # of repetitions wanted */
{
	return(replaces(TRUE, f, n));
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 */
STATIC int	replaces(kind, f, n)
int	kind;	/* Query enabled flag */
int	f;	/* default flag */
int	n;	/* # of repetitions wanted */
{
	register int status;	/* success flag on pattern inputs */
	register int rlength;	/* length of replacement string */
	register int numsub;	/* number of substitutions */
	register int nummatch;	/* number of found matches */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	char c;			/* input char for query */
	char tpat[NPAT];	/* temporary to hold search pattern */
	LINE *origline;		/* original "." position */
	int origoff;		/* and offset (for . query option) */
	LINE *lastline;		/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* Check for negative repetitions.
	 */
	if (f && n < 0)
		return(FALSE);

	/* Ask the user for the text of a pattern.
	 */
	if ((status = readpattern(
	    (kind == FALSE ? "Replace" : "Query replace"), &pat[0], TRUE))
								!= TRUE)
		return(status);

	/* Ask for the replacement string.
	 */
	if ((status = readpattern("with", &rpat[0], FALSE)) == ABORT)
		return(status);

	/* Find the length of the replacement string.
	 */
	rlength = (int) strlen(&rpat[0]);

	/* Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[mtchlen - 1] == '\n');
	nlrepl = FALSE;

	lastline = NULL;
	lastoff = 0;

	if (kind)
	{
		/* Build query replace question string.
		 */
		strcpy(tpat, "Replace '");
		expandp(&pat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, "' with '");
		expandp(&rpat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, "'? ");

		/* Initialize last replaced pointers.
		 */
		lastline = NULL;
		lastoff = 0;
	}

	/* Save original . position, init the number of matches and
	 * substitutions, and scan through the file.
	 */
	origline = curwp->w_dotp;
	origoff = curwp->w_doto;
	numsub = 0;
	nummatch = 0;

	while ( (f == FALSE || n > nummatch) &&
		(nlflag == FALSE || nlrepl == FALSE) )
	{
		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of
		 * the matched string.
		 */
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
		{
			if (!mcscanner(&mcpat[0], FORWARD, PTBEG))
				break;
		}
		else
#endif
			if (!scanner(&pat[0], FORWARD, PTBEG))
				break;		/* all done */

		++nummatch;	/* Increment # of matches */

		/* Check if we are on the last line.
		 */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		/* Check for query.
		 */
		if (kind)
		{
			/* Get the query.
			 */
pprompt:		mlwrite(&tpat[0], &pat[0], &rpat[0]);
qprompt:
			update(TRUE);  /* show the proposed place to change */
			c = (char) tgetc();		/* and input */
			mlwrite("");			/* and clear it */

			/* And respond appropriately.
			 */
			switch (c)
			{
				case 'y':	/* yes, substitute */
				case 'Y':
				case ' ':
					savematch();
					break;

				case 'n':	/* no, onword */
				case 'N':
				case '\010':	/* ^H */
				case '\177':	/* DEL */
					forwchar(FALSE, 1);
					continue;

				case '!':	/* yes/stop asking */
				case 'p':
				case 'P':
					kind = FALSE;
					break;

				case 'u':	/* undo last and re-prompt */
				case 'U':

					/* Restore old position.
					 */
					if (lastline == NULL)
					{
						/* There is nothing to undo.
						 */
						TTbeep();
						goto pprompt;
					}
					curwp->w_dotp = lastline;
					curwp->w_doto = lastoff;
					lastline = NULL;
					lastoff = 0;

					/* Delete the new string.
					 */
					backchar(FALSE, rlength);
					status = delins(rlength,patmatch,FALSE);
					if (status != TRUE)
						return (status);

					/* Record one less substitution,
					 * backup, and reprompt.
					 */
					--numsub;
					backchar(FALSE, (int) mlenold);
					matchline = curwp->w_dotp;
					matchoff  = curwp->w_doto;
					goto pprompt;

				case '.':	/* abort! and return */
					/* restore old position */
					curwp->w_dotp = origline;
					curwp->w_doto = origoff;
					curwp->w_flag |= WFMOVE;
					/* fall through */

				case BELL:	/* abort! and stay */
					mlwrite("Aborted!");
					return(FALSE);

				default:	/* bitch and beep */
					TTbeep();
					/* fall through */

				case '?':	/* help me */
					mlwrite(
"(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: ");
					goto qprompt;

			}	/* end of switch */
		}	/* end of "if kind" */

		/* if this is the point origin, flag so we a can reset it */
		if (curwp->w_dotp == origline) {
			origline = NULL;
			lastline = curwp->w_dotp->l_bp;
		}

		/*
		 * Delete the sucker, and insert its
		 * replacement.
		 */
		status = delins((int)mtchlen, &rpat[0], TRUE);
		if (origline == NULL) {
			origline = lastline->l_fp;
			origoff = 0;
		}
		if (status != TRUE)
			return (status);

		/* Save where we are if we might undo this....
		 */
		if (kind)
		{
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
		}

		else if (mtchlen == 0)
		{
			mlwrite("Empty string replaced, stopping.");
			return(FALSE);
		}

		numsub++;	/* increment # of substitutions */
	}

	/* And report the results.
	 */
	mlwrite("%d substitutions", numsub);
	return(TRUE);
}

/*
 * delins -- Delete a specified length from the current
 *	point, then insert the string.
 */
static int delins(dlength, instr, usemeta)
int	dlength;
char	*instr;
int	usemeta;
{
	int	status;
#if	MAGIC
	RMC	*rmcptr;
#endif

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if ((status = ldelete((long) dlength, FALSE)) != TRUE)
	{
		mlwrite("%%ERROR while deleting");
		return(FALSE);
	}
	else
#if	MAGIC
		if ((rmagical && usemeta) &&
		    (curwp->w_bufp->b_mode & MDMAGIC) != 0)
		{
			rmcptr = &rmcpat[0];
			while (rmcptr->mc_type != MCNIL && status == TRUE)
			{
				if (rmcptr->mc_type == LITCHAR)
					status = linstr(rmcptr->rstr);
				else
					status = linstr(patmatch);
				rmcptr++;
			}
		}
		else
#endif
			status = linstr(instr);

	return (status);
}

/*
 * expandp -- Expand control key sequences for output.
 */
int expandp(srcstr, deststr, maxlength)
char *srcstr;	/* string to expand */
char *deststr;	/* destination of expanded string */
int maxlength;	/* maximum chars in destination */
{
	unsigned /* char */ c;	/* current char to translate */

	/* Scan through the string.
	 */
	while ((c = UCAST *srcstr++) != 0)
	{
		if (c == '\n')		/* it's a newline */
		{
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		}
		else if ((c&0x7f) < 0x20 || (c&0x7f) == 0x7f) /* cntl char */
		{
			*deststr++ = '^';
			*deststr++ = (char) ((c ^ 0x40) & 0x7f);
			maxlength -= 2;
		}
		else if (c == '%')
		{
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		}
		else			/* any other character */
		{
			*deststr++ = (char) c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4)
		{
			*deststr++ = '$';
			*deststr = '\0';
			return(FALSE);
		}
	}
	*deststr = '\0';
	return(TRUE);
}

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundry restrictions
 *	in future, a' la VMS EDT.  At the moment, just return TRUE or
 *	FALSE depending on if a boundry is hit (ouch).
 */
static int boundry(curline, curoff, dir)
LINE	*curline;
int	curoff, dir;
{
	register int	border;

	if (dir == FORWARD)
	{
		border = (curline == curbp->b_linep) ||
			 ((curoff == llength(curline)) &&
			  (lforw(curline) == curbp->b_linep));
	}
	else
	{
		border = /* (curline == curbp->b_linep) || */
			 ((curoff == 0) &&
			  (lback(curline) == curbp->b_linep));
	}
	return (border);
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
STATIC int nextch(pcurline, pcuroff, dir)
LINE	**pcurline;
int	*pcuroff;
int	dir;
{
	register LINE	*curline;
	register int	curoff;
	register int	c;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD)
	{
		if (curoff == llength(curline))		/* if at EOL */
		{
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\n';			/* and return a <NL> */
		}
		else
			c = lgetc(curline, curoff++);	/* get the char */
	}
	else			/* Reverse.*/
	{
		if (curoff == 0)
		{
			curline = lback(curline);
			curoff = llength(curline);
			c = '\n';
		}
		else
			c = lgetc(curline, --curoff);

	}
	*pcurline = curline;
	*pcuroff = curoff;

	return (c);
}

#if	MAGIC
/*
 * mcstr -- Set up the 'magic' array.  The closure symbol is taken as
 *	a literal character when (1) it is the first character in the
 *	pattern, and (2) when preceded by a symbol that does not allow
 *	closure, such as a newline, beginning of line symbol, or another
 *	closure symbol.
 *
 *	Coding comment (jmg):  yes, i know i have gotos that are, strictly
 *	speaking, unnecessary.  But right now we are so cramped for
 *	code space that i will grab what i can in order to remain
 *	within the 64K limit.  C compilers actually do very little
 *	in the way of optimizing - they expect you to do that.
 */
STATIC int mcstr()
{
	MC	*mcptr, *rtpcm;
	char	*patptr;
 	int	mj;
 	int	pchr;
 	int	status;
 	int	does_closure;

 	status = TRUE;
 	does_closure = FALSE;

	/* If we had metacharacters in the MC array previously,
	 * free up any bitmaps that may have been allocated.
	 */
	if (magical)
		mcclear();

	mj = 0;
	mcptr = &mcpat[0];
	patptr = &pat[0];

	while ((pchr = *patptr) != '\0' && status)
	{
		switch (pchr)
		{
			case MC_CCL:
				status = cclmake(&patptr, mcptr);
				magical = TRUE;
				does_closure = TRUE;
				break;

			case MC_BOL:
				/*
				 * If the BOL character isn't the
				 * first in the pattern, we assume
				 * it's a literal instead.
				 */
				if (mj != 0)
					goto litcase;

				mcptr->mc_type = BOL;
				magical = TRUE;
				break;

			case MC_EOL:
				/*
				 * If the EOL character isn't the
				 * last in the pattern, we assume
				 * it's a literal instead.
				 */
				if (*(patptr + 1) != '\0')
				if (*(patptr + 1) != '\0')
					goto litcase;

				mcptr->mc_type = EOL;
				magical = TRUE;
				break;

			case MC_ANY:
				mcptr->mc_type = ANY;
				magical = TRUE;
				does_closure = TRUE;
				break;

			case MC_CLOSURE:
			case MC_CLOSURE_1:
			case MC_ZEROONE:
				/* Does the closure symbol mean closure here?
				 * If so, back up to the previous element
				 * and indicate it is enclosed.
				 */
				if (!does_closure)
					goto litcase;
				mj--;
				mcptr--;
				if (pchr == MC_CLOSURE)
					mcptr->mc_type |= CLOSURE;
				else if (pchr == MC_CLOSURE_1)
					mcptr->mc_type |= CLOSURE_1;
				else
					mcptr->mc_type |= ZEROONE;
				magical = TRUE;
				does_closure = FALSE;
				break;

			/* Note: no break between MC_ESC case and the default.
			 */
			case MC_ESC:
				if (*(patptr + 1) != '\0')
				{
					pchr = *++patptr;
					magical = TRUE;
				}
				/* fall through */
			default:
litcase:			mcptr->mc_type = LITCHAR;
				mcptr->u.lchar = pchr;
				does_closure = (pchr != '\n');
				break;
		}		/* End of switch.*/
		mcptr++;
		patptr++;
		mj++;
	}		/* End of while.*/

	/* Close off the meta-string.
	 */
	mcptr->mc_type = MCNIL;

	/* Set up the reverse array, if the status is good.  Please note the
	 * structure assignment - your compiler may not like that.
	 * If the status is not good, nil out the meta-pattern.
	 * The only way the status would be bad is from the cclmake()
	 * routine, and the bitmap for that member is guarenteed to be
	 * freed.  So we stomp a MCNIL value there, and call mcclear()
	 * to free any other bitmaps.
	 */
	if (status)
	{
		rtpcm = &tapcm[0];
		while (--mj >= 0)
		{
#if	LATTICE | DECUSC | C86
			movmem(--mcptr, rtpcm++, sizeof (MC));
#else
			*rtpcm++ = *--mcptr;
#endif
		}
		rtpcm->mc_type = MCNIL;
	}
	else
	{
		(--mcptr)->mc_type = MCNIL;
		mcclear();
	}

	return(status);
}

/*
 * rmcstr -- Set up the replacement 'magic' array.  Note that if there
 *	are no meta-characters encountered in the replacement string,
 *	the array is never actually created - we will just use the
 *	character array rpat[] as the replacement string.
 */
static int rmcstr()
{
	RMC	*rmcptr;
	char	*patptr;
	int	status = TRUE;
	int	mj;

	patptr = &rpat[0];
	rmcptr = &rmcpat[0];
	mj = 0;
	rmagical = FALSE;

	while (*patptr && status == TRUE)
	{
		switch (*patptr)
		{
			case MC_DITTO:

				/* If there were non-magical characters
				 * in the string before reaching this
				 * character, plunk it in the replacement
				 * array before processing the current
				 * meta-character.
				 */
				if (mj != 0)
				{
					rmcptr->mc_type = LITCHAR;
					if ((rmcptr->rstr = malloc(mj + 1)) ==
						NULL)
					{
						mlwrite("%%Out of memory");
						status = FALSE;
						break;
					}
					bytecopy(rmcptr->rstr, patptr - mj, mj);
					rmcptr++;
					mj = 0;
				}
				rmcptr->mc_type = DITTO;
				rmcptr++;
				rmagical = TRUE;
				break;

			case MC_ESC:
				rmcptr->mc_type = LITCHAR;

				/* We malloc mj plus two here, instead
				 * of one, because we have to count the
				 * current character.
				 */
				if ((rmcptr->rstr = malloc(mj + 2)) == NULL)
				{
					mlwrite("%%Out of memory");
					status = FALSE;
					break;
				}

				bytecopy(rmcptr->rstr, patptr - mj, mj + 1);

				/* If MC_ESC is not the last character
				 * in the string, find out what it is
				 * escaping, and overwrite the last
				 * character with it.
				 */
				if (*(patptr + 1) != '\0')
					*((rmcptr->rstr) + mj) = *++patptr;

				rmcptr++;
				mj = 0;
				rmagical = TRUE;
				break;

			default:
				mj++;
		}
		patptr++;
	}

	if (rmagical && mj > 0)
	{
		rmcptr->mc_type = LITCHAR;
		if ((rmcptr->rstr = malloc(mj + 1)) == NULL)
		{
			mlwrite("%%Out of memory");
			status = FALSE;
		}
		bytecopy(rmcptr->rstr, patptr - mj, mj);
		rmcptr++;
	}

	rmcptr->mc_type = MCNIL;
	return(status);
}

/*
 * mcclear -- Free up any CCL bitmaps, and MCNIL the MC arrays.
 */
VOID mcclear()
{
	register MC	*mcptr;

	mcptr = &mcpat[0];

	while (mcptr->mc_type != MCNIL)
	{
		if ((mcptr->mc_type & MASKCLO) == CCL ||
		    (mcptr->mc_type & MASKCLO) == NCCL)
			free(mcptr->u.cclmap);
		mcptr++;
	}
	mcpat[0].mc_type = tapcm[0].mc_type = MCNIL;
	magical = FALSE;
}

/*
 * rmcclear -- Free up any strings, and MCNIL the RMC array.
 */
static VOID rmcclear()
{
	register RMC	*rmcptr;

	rmcptr = &rmcpat[0];

	while (rmcptr->mc_type != MCNIL)
	{
		if (rmcptr->mc_type == LITCHAR)
			free(rmcptr->rstr);
		rmcptr++;
	}

	rmcpat[0].mc_type = MCNIL;
}

/*
 * mceq -- meta-character equality with a character.  In Kernighan & Plauger's
 *	Software Tools, this is the function omatch(), but i felt there
 *	were too many functions with the 'match' name already.
 */
STATIC int	mceq(bc, mt)
int	bc;
MC	*mt;
{
	register int result;

	switch (mt->mc_type & MASKCLO)
	{
		case LITCHAR:
			result = me_eq(bc, mt->u.lchar);
			break;

		case ANY:
			result = (bc != '\n');
			break;

		case CCL:
			if (!(result = biteq(bc, mt->u.cclmap)))
			{
				if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
				    (isletter(bc)))
				{
					result = biteq(CHCASE(bc), mt->u.cclmap);
				}
			}
			break;

		case NCCL:
			result = !biteq(bc, mt->u.cclmap);

			if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
			    (isletter(bc)))
			{
				result &= !biteq(CHCASE(bc), mt->u.cclmap);
			}
			break;

		default:
			mlwrite("mceq: what is %d?", mt->mc_type);
			result = FALSE;
			break;

	}	/* End of switch.*/

	return (result);
}

/*
 * cclmake -- create the bitmap for the character class.
 *	ppatptr is left pointing to the end-of-character-class character,
 *	so that a loop may automatically increment with safety.
 */
STATIC int	cclmake(ppatptr, mcptr)
char	**ppatptr;
MC	*mcptr;
{
	BITMAP		bmap;
	register char	*patptr;
	register int	pchr, ochr;

	if ((bmap = clearbits()) == NULL)
	{
		mlwrite("%%Out of memory");
		return (FALSE);
	}

	mcptr->u.cclmap = bmap;
	patptr = *ppatptr;

	/*
	 * Test the initial character(s) in ccl for
	 * special cases - negate ccl, or an end ccl
	 * character as a first character.  Anything
	 * else gets set in the bitmap.
	 */
	if (*++patptr == MC_NCCL)
	{
		patptr++;
		mcptr->mc_type = NCCL;
	}
	else
		mcptr->mc_type = CCL;

	if ((ochr = *patptr) == MC_ECCL)
	{
		mlwrite("%%No characters in character class");
		return (FALSE);
	}
	else
	{
		if (ochr == MC_ESC)
			ochr = *++patptr;

		setbit(ochr, bmap);
		patptr++;
	}

	while (ochr != '\0' && (pchr = *patptr) != MC_ECCL)
	{
		switch (pchr)
		{
			/* Range character loses its meaning
			 * if it is the last character in
			 * the class.
			 */
			case MC_RCCL:
				if (*(patptr + 1) == MC_ECCL)
					setbit(pchr, bmap);
				else
				{
					pchr = *++patptr;
					while (++ochr <= pchr)
						setbit(ochr, bmap);
				}
				break;

			/* Note: no break between case MC_ESC and the default.
			 */
			case MC_ESC:
				pchr = *++patptr;
				/* fall through */
			default:
				setbit(pchr, bmap);
				break;
		}
		patptr++;
		ochr = pchr;
	}

	*ppatptr = patptr;

	if (ochr == '\0')
	{
		mlwrite("%%Character class not ended");
		free(bmap);
		return (FALSE);
	}
	return (TRUE);
}

/*
 * biteq -- is the character in the bitmap?
 */
STATIC int	biteq(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc >= HICHAR)
		return (FALSE);

	return( (*(cclmap + (bc >> 3)) & BIT(bc & 7))? TRUE: FALSE );
}

/*
 * clearbits -- Allocate and zero out a CCL bitmap.
 */
STATIC	BITMAP clearbits()
{
	BITMAP		cclstart, cclmap;
	register int	j;

	if ((cclmap = cclstart = (BITMAP) malloc(HIBYTE)) != NULL)
		for (j = 0; j < (HIBYTE); j++)
			*cclmap++ = 0;

	return (cclstart);
}

/*
 * setbit -- Set a bit (ON only) in the bitmap.
 */
STATIC void setbit(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc < HICHAR)
		*(cclmap + (bc >> 3)) |= BIT(bc & 7);
}
#endif
