/*	INPUT:	Various input routines for MicroEMACS
		written by Daniel Lawrence
		5/9/86						*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	BCOMPL
static VOID comp_buffer PP((char *name, int *cpos, int *matchlen));
static VOID comp_command PP((char *name, int *cpos, int *matchlen));
static VOID comp_file PP((char *name, int *cpos, int *matchlen));
static VOID comp_prompt PP((CONSTA char *prompt, CONSTA char *defval, int type));
static VOID comp_mode PP((char *prompt, int *cpos, int *matchlen));
#endif

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */

int mlyesno(prompt)

CONSTA char *prompt;

{
	int c;			/* input character */
	char buf[NPAT];		/* prompt to user */

	/* build and prompt the user */
	strcpy(buf, prompt);
	strcat(buf, " [y/n]? ");
	mlwrite(buf);

	for (;;) {
		/* get the responce */
		c = tgetc();

		if (c == ectoc(abortc))		/* Bail out! */
			return(ABORT);

		if (c=='y' || c=='Y' || c=='s' || c=='S')
			return(TRUE);

		if (c=='n' || c=='N')
			return(FALSE);
	}
}

/*
 * Write a prompt into the message line, then read back a response. Keep
 * track of the physical position of the cursor. If we are in a keyboard
 * macro throw the prompt away, and return the remembered response. This
 * lets macros run at full speed. The reply is always terminated by a carriage
 * return. Handle erase, kill, and abort keys.
 */

int mlreply(prompt, buf, nbuf)
    CONSTA char *prompt;
    char *buf;
    int nbuf;
{
	return(nextarg(prompt, buf, nbuf, ctoec('\n')));
}

int mltreply(prompt, buf, nbuf, eolchar)

CONSTA char *prompt;
char *buf;
int nbuf;
int eolchar;

{
	return(nextarg(prompt, buf, nbuf, eolchar));
}

/*	ectoc:	expanded character to character
		collapse the CTRL and SPEC flags back into an ascii code   */

int ectoc(c)

int c;

{
	if (c & CTRL)
		c = c & ~(CTRL | 0x40);
	if (c & SPEC)
		c = c & 255;
	return(c);
}

/*	ctoec:	character to extended character
		pull out the CTRL and SPEC prefixes (if possible)	*/

int ctoec(c)

int c;

{
        if (c>=0x00 && c<=0x1F)
                c = CTRL | (c+'@');
        return (c);
}

static VOID comp_prompt(prompt, defval, type)
CONSTA char *prompt;
CONSTA char *defval;
int type;
{
	int savcmd;

	/* if it exists, prompt the user for a buffer name */
	if (prompt && (discmd ||
			(disinp && (!clexec || type == CMP_NONE)))) {
		savcmd = discmd; discmd = TRUE;
		if (defval)
			mlwrite("%s[%s]: ", prompt, defval);
		else if (type == CMP_BUFFER || type == CMP_FILENAME)
			mlwrite("%s: ", prompt);
		else
			mlwrite("%s", prompt);
		discmd = savcmd;
	}
}

char *complete(prompt, defval, type, buf, maxlen, eolchar, needeol, status)

CONSTA char *prompt;	/* prompt to user on command line */
CONSTA char *defval;	/* default value to display to user */
int type;		/* type of what we are completing */
char *buf;		/* buf pointer, if NULL, use our own */
int maxlen;		/* maximum length of input field */
int eolchar;		/* termination character */
int needeol;		/* needs eol to confirm */
int *status;		/* return status */

{
	register int c;		/* current input character */
	int cpos;		/* current column on screen output */
	int quotef;		/* are we quoting the next char? */
	int len;
	int popsave;
	char *str, chbuf[2];
#if DISPSEVEN
	CONST char *s;
	int ch_len;
#endif
	static char FAR sbuf[NSTRING];/* buffer to hold tentative name */
#ifdef NO_PROTOTYPE
	char *getkill();
#endif

	if (!buf) { buf = sbuf; if (maxlen >= NSTRING) maxlen = NSTRING-1; }

	*status = TRUE;

	comp_prompt(prompt, defval, type);

	/* if we are executing a command line get the next arg and match it */
	if (clexec && type != CMP_NONE) {
		if (macarg(sbuf) != TRUE)
			{ *status = FALSE; return(NULL); }
		if (buf != sbuf)
			bytecopy(buf, sbuf, maxlen-1);
		return(buf);
	}

	/* starting at the beginning of the string buffer */
	cpos = 0;
	quotef = FALSE;

	/* build a name string from the keyboard */
	while (TRUE) {
		c = get1key();

		/* If it is a <ret>, change it to a <NL> */
		if (c == (CTRL | 0x4d) && quotef == FALSE)
			c = CTRL | 0x40 | '\n';

		/* if we are at the end, just match it */
#if	DECEDT
		if (quotef == FALSE && (c & SPEC)) c = eolchar;
#endif
		if (c == eolchar && quotef == FALSE) {
			if (defval && cpos==0)
				strcpy(buf, defval);
			else
				buf[cpos] = '\0';
			if (buf[0] == '\0')
				*status = FALSE;
			return(buf);
		}

		/* change from command form back to character form */
		c = ectoc(c);		

		if (c == ectoc(abortc)) {	/* Bell, abort */
			ctrlg(FALSE, 0);
			TTflush();
			*status = ABORT;
			return(NULL);
		}

		if ((c == 0x7F || c == 0x08 || c == 0x15) && !quotef) {
			/* rubout/erase or C-U, kill */
			len = ((c == 0x15)? 0: cpos-1);
			while (cpos > len && cpos > 0) {
#if DISPSEVEN
				if (dispseven && buf[cpos-1] != '\n') {
					ch_len = me_char_len[buf[--cpos]&0xFF];
					while (ch_len-- > 0 && ttcol > 0) {
						outstring("\b \b");
						--ttcol;
					}
				} else
#endif
				{
					outstring("\b \b");
					--ttcol;

					if ((buf[--cpos]&0x7f) < 0x20 ||
					    (buf[cpos]&0x7f) == 0x7f) {
						outstring("\b \b");
						--ttcol;
					}

					if (buf[cpos] == '\n') {
						outstring("\b\b  \b\b");
						ttcol -= 2;
					}
				}
				TTflush();
			}

		} else if (c == quotec && quotef == FALSE) {
			quotef = TRUE;

#if	BCOMPL
		} else if (type != CMP_NONE && quotef == FALSE &&
				c == '?') {
			buf[cpos] = '\0';
			popsave = popflag;
			popflag = cpopflag;
			switch (type) {
				case CMP_FILENAME:
					filelist(buf);
					break;
				case CMP_BUFFER:
					makelist(buf, TRUE);
					wpopup(blistp);
					break;
				case CMP_COMMAND:
					buildlist(2, buf);
					break;
				case CMP_VARIABLE:
#if DEBUGM
					varlist(buf);
					break;
#else
					popflag = popsave;
					TTbeep();
					TTflush();
					continue;
#endif
				case CMP_MODE:
					modelist(buf);
					break;
			}
			popflag = popsave;
			update(FALSE);
			comp_prompt(prompt, defval, type);
			outstring(buf);
			TTflush();
			continue;

		} else if ((type != CMP_NONE) && quotef == FALSE &&
				(c == 27 || c == ' ' || c == '\t')) {
			/* attempt a completion */
			len = -1;
			buf[cpos] = '\0';
			switch (type) {
				case CMP_BUFFER:
					comp_buffer(buf, &cpos, &len);
					break;
				case CMP_COMMAND:
					comp_command(buf, &cpos, &len);
					break;
				case CMP_FILENAME:
					comp_file(buf, &cpos, &len);
					break;
				case CMP_VARIABLE:
					comp_variable(buf, &cpos, &len);
					break;
				case CMP_MODE:
					comp_mode(buf, &cpos, &len);
					break;
			}

			if (cpos >= len)
				{ TTbeep(); TTflush(); continue; }
			if (disinp) {
				while (cpos < len)
					TTputc(buf[cpos++]);
				TTflush();
			} else {
				cpos = len;
			}

			/* if we have completed all the way... go back */
			if (buf[cpos] == '\0') {
				if (type == CMP_FILENAME && cpos > 0) {
		/* getnfile appends a slash to directories */
		/* if we have a trailing slash, leave match incomplete */
#if USG | BSD | V7
					if (buf[cpos-1] == '/') continue;
#endif
#if MSDOS
					if (buf[cpos-1] == '\\') continue;
#endif
#if VMS
					if (buf[cpos-1] == ']') continue;
#endif
				}
				cpos++;
			}

			if (cpos > 0 && buf[cpos - 1] == '\0') {
				if (needeol) cpos--;
				else return(buf);
			}
#endif
		} else {
			if (c == 0x19 && quotef == FALSE) {
				/* ^Y -- insert contents of kill buffer */
				str = getkill();
				len = strlen(str);
			} else if ((c == '\022' || c == '\023') &&
					quotef == FALSE) {
				/* ^R or ^S -- insert search string */
				str = pat;
				len = strlen(str);
			}
			else { str = chbuf; *chbuf = (char) c; len = 1; }
			quotef = FALSE;

			while (cpos+1 < maxlen && len-- > 0) {
				c = buf[cpos++] = *str++;
				if (c == '\n') { /* put out <NL> for <ret> */
					outstring("<NL>");
					ttcol += 3;
#if DISPSEVEN
				} else if (dispseven) {
					s = me_char_name[ c & 0xFF ];
					while (*s != '\0') {
						if (disinp) TTputc(*s);
						++ttcol;
						++s;
					}
					continue;	/* avoid ttcol inc */
#endif
				} else {
					if ((c&0x7f) < 0x20 ||
					    (c&0x7f) == 0x7f) {
						if (disinp) TTputc('^');
						++ttcol;
						c = ((c&0x7f) ^ 0x40);
					}
					if (disinp) TTputc(c);
				}
				++ttcol;
			}
			TTflush();
		}
	}
}

#if	BCOMPL

/* Generalized comparison for command completion routines */
/* Returns 0 if done, 1 if no match, 2 if partial match */

int comp_test(name, namelen, cpos, matchlen, testname, exact)
char *name;
CONSTA char *testname;
int namelen, *cpos, *matchlen, exact;
{
	int matchflag;
	int index;

	/* is this a match? */
	if (*cpos <= 0) {
		matchflag = TRUE;
	} else if (exact) {
		matchflag = !strncmp(name, testname, *cpos);
	} else {
		matchflag = !strcompare(name, testname, *cpos);
	}

	if (!matchflag)
		return(1);

	/* if it is a match */

	/* if this is the first match, simply record it */
	if (*matchlen < 0) {
		*matchlen = *cpos;
		while (*matchlen+1 < namelen) {
			name[*matchlen] = testname[*matchlen];
			if (!name[*matchlen]) break;
			++(*matchlen);
		}
	} else {
		/* if there's a difference, stop here */
		for (index = *cpos; index <= *matchlen; index++) {
			if (exact) {
				if (name[index] != testname[index])
					break;
			} else {
				if (chrcompare(name[index],testname[index]))
					break;
			}
		}
		if (index < *matchlen)
			*matchlen = index;
		else if (testname[*matchlen])
			name[*matchlen] = testname[*matchlen];
		if (*matchlen <= *cpos)
			return(0);
	}
	return(2);
}

/*	comp_command:	Attempt a completion on a command name	*/

static VOID comp_command(name, cpos, matchlen)

char *name;	/* command containing the current name to complete */
int *cpos;	/* ptr to position of next character to insert */
int *matchlen;	/* ptr to new position */

{
	register int curbind;	/* index into the names[] array */

	curbind = 0;
	while (names[curbind].n_func != NULL) {
		if (!comp_test(name, NSTRING, cpos, matchlen,
				names[curbind].n_name, FALSE))
			break;
		curbind++;
	}
	return;
}

/*	comp_buffer:	Attempt a completion on a buffer name	*/

static VOID comp_buffer(name, cpos, matchlen)

char *name;	/* buffer containing the current name to complete */
int *cpos;	/* ptr to position of next character to insert */
int *matchlen;	/* ptr to new position */

{
	register BUFFER *bp;	/* trial buffer to complete */

	bp = bheadp;
	while (bp) {
		if (!comp_test(name, NBUFN, cpos, matchlen,
				bp->b_bname, TRUE))
			break;
		bp = bp->b_bufp;
	}
	return;
}

/*	comp_file:	Attempt a completion on a file name	*/

static VOID comp_file(name, cpos, matchlen)

char *name;	/* file containing the current name to complete */
int *cpos;	/* ptr to position of next character to insert */
int *matchlen;	/* ptr to new position */

{
	register char *fname;	/* trial file to complete */
	int exact;

	exact = ! (VMS | MSDOS);
	fname = getffile(name, *cpos, exact, FALSE);
	while (fname) {
		if (!comp_test(name, NFILEN, cpos, matchlen,
				fname, exact))
			break;
		fname = getnfile(name, *cpos, exact);
	}
	return;
}

/*	comp_mode:	Attempt a completion on a mode name	*/

static VOID comp_mode(name, cpos, matchlen)

char *name;	/* command containing the current name to complete */
int *cpos;	/* ptr to position of next character to insert */
int *matchlen;	/* ptr to new position */

{
	register int i;

	for (i = 0; i < NCOLORS; i++) {
		if (!comp_test(name, NSTRING, cpos, matchlen,
				cname[i], FALSE))
			return;
	}
	for (i = 0; i < NUMMODES; i++) {
		if (!comp_test(name, NSTRING, cpos, matchlen,
				modename[i], FALSE))
			return;
	}
	return;
}

#endif

/* get a command name from the command line. Command completion means
   that pressing a <SPACE> will attempt to complete an unfinished command
   name if it is unique.
*/

#if	DARWIN
int *getname(prompt)
#else
int (*getname(prompt))(int f, int n)
#endif

CONSTA char *prompt;	/* string to prompt with */

{
	char *sp;	/* ptr to the returned string */
	int status;
#ifdef NO_PROTOTYPE
	int (*fncmatch())(int f, int n);
#endif

	sp = complete(prompt, NULL, CMP_COMMAND, NULL, NSTRING,
			ctoec('\n'), FALSE, &status);
	if (sp == NULL)
		return(NULL);

	return(fncmatch(sp));
}


/*	getcbuf:	get a completion from the user for a buffer name.

			I was goaded into this by lots of other people's
			completion code.
*/

BUFFER *getcbuf(prompt, defval, createflag)

CONSTA char *prompt;	/* prompt to user on command line */
CONSTA char *defval;	/* default value to display to user */
int createflag;		/* should this create a new buffer? */

{
	char *sp;	/* ptr to the returned string */
	int status;

	sp = complete(prompt, defval, CMP_BUFFER, NULL, NBUFN,
			ctoec('\n'), FALSE, &status);
	if (sp == NULL)
		return(NULL);

	return(bfind(sp, createflag, 0));
}

char *gtfilename(prompt, needeol)

CONSTA char *prompt;	/* prompt to user on command line */
int needeol;		/* needs eol to confirm (when writing files) */

{
	char *sp;	/* ptr to the returned string */
	int status;

	sp = complete(prompt, NULL, CMP_FILENAME, NULL, NFILEN,
			ctoec('\n'), needeol, &status);
	if (sp == NULL)
		return(NULL);

	return(sp);
}

char *gtmodename(prompt)
CONSTA char *prompt;	/* prompt to user on command line */
{
	char *sp;	/* ptr to the returned string */
	int status;

	sp = complete(prompt, NULL, CMP_MODE, NULL, NPAT-1,
			ctoec('\n'), FALSE, &status);
	if (sp == NULL)
		return(NULL);
	return(sp);
}

/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/

int tgetc()

{
	int c;	/* fetched character */

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {

		/* if there is some left... */
		if (kbdptr < kbdend)
			return((int)*kbdptr++);

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = &kbdm[0];
			return((int)*kbdptr++);
		}
	}

	/* fetch a character from the terminal driver */
	c = TTgetc();

	/* record it for $lastkey */
	lastkey = c;

	/* save it if we need to */
	if (kbdmode == RECORD) {
		*kbdptr++ = (char) c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}

	/* and finally give the char back */
	return(c);
}

/*	GET1KEY:	Get one keystroke. The only prefixs legal here
			are the SPEC and CTRL prefixes.
								*/

#if	TYPEAH
NOSHARE extern int tacnt;
#endif

int get1key()

{
	int    c, code, i;
#if	AMIGA
	int	d;
#endif

	/* get a keystroke */
	if (cpending) {
		c = charpending; cpending = FALSE;
		if ((c&0xFF00) != 0) return(c);
	} else {
		c = tgetc();
	}

#if	MSDOS | ST520
	if (c == 0) {				/* Apply SPEC prefix	*/
	        c = tgetc();
	        if (c>=0x00 && c<=0x1F)		/* control key? */
        	        c = CTRL | (c+'@');
		return(SPEC | c);
	}
#endif

#if	DECEDT
	/* catch arrow keys and function keys */
	if (c == 27 || (c&0xFF) == 0x8F || (c&0xFF) == 0x9B) {
#if	TYPEAH & (VMS | BSD | MSDOS | (USG & MSC))
		if (c == 27 && kbdmode != PLAY && !vt100keys) {
			tacnt = 0;
			if (!typahead()) return(CTRL | (c+'@'));
		}
#endif
		charpending = ((c == 27)? tgetc(): ((c&0x7F)|0x40));
		cpending = TRUE;
		if (charpending == '[' || charpending == 'O') {
			cpending = FALSE;
			c = tgetc();
			if (c >= '0' && c <= '9') {
				code = 0;
				for(i = 0; i < 3 && c >= '0' && c <= '9'; i++)
					{code = code*10+c-'0'; c = tgetc();}
				if (c == 27 ||
				    (c&0xFF) == 0x8F || (c&0xFF) == 0x9B) {
					charpending = (c&0xFF);
					cpending = TRUE;
				}
				c = code;
				if (c>=0x00 && c<=0x1F) c = CTRL | (c+'@');
			}
			return(SPEC | c);
#if	defined(__INTERIX)
		} else if (charpending == 'F') {
			cpending = FALSE;
			c = tgetc();
			if (c >= '0' && c <= '9') {
				c -= '0';
			} else if (c >= 'A' && c <= 'Z') {
				c = (c - 'A') + 10;
			}
			if (c>=0x00 && c<=0x1F) c = CTRL | (c+'@');
			return(SPEC | c);
#endif
		}
	}
#endif

#if	AMIGA
	/* apply SPEC prefix */
	if ((unsigned)c == 155) {
		c = tgetc();

		/* first try to see if it is a cursor key */
		if ((c >= 'A' && c <= 'D') || c == 'S' || c == 'T')
			return(SPEC | c);

		/* next, a 2 char sequence */
		d = tgetc();
		if (d == '~')
			return(SPEC | c);

		/* decode a 3 char sequence */
		c = d + 32;
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = tgetc();
		return(SPEC | c);
	}
#endif

#if  WANGPC
	if (c == 0x1F) {			/* Apply SPEC prefix    */
	        c = tgetc();
		return(SPEC | c);
	}
#endif

        if (c>=0x00 && c<=0x1F)                 /* C0 control -> C-     */
                c = CTRL | (c+'@');
        return (c);
}

/*	GETCMD:	Get a command from the keyboard. Process all applicable
		prefix keys
							*/

NOSHARE extern int inmeta, incex, inspecial;

int getcmd()

{
	int c;		/* fetched keystroke */
	int oldmeta;
	int code, i;

	/* get initial character */
	c = get1key();

	/* process prefixes */
	if (inmeta == TRUE || c == metac ||
	    incex == TRUE || c == ctlxc) {
		oldmeta = inmeta;
		if (c == ctlxc) {c = get1key(); incex = TRUE;}
		if (c == metac) {
			c = get1key(); inmeta = TRUE;
			if (oldmeta == FALSE && c == metac)
				{c = get1key(); oldmeta = TRUE;}
		}
	        if (islower(c))		/* Force to upper */
        	        c ^= DIFCASE;
	        if (c>=0x00 && c<=0x1F)		/* control key */
	        	c = CTRL | (c+'@');
		/* temporary ESC sequence fix......... */
		if (inmeta == TRUE && (c == '[' || c == 'O')) {
			c = get1key();
			if (c >= '0' && c <= '9') {
				code = 0;
				for(i = 0; i < 3 && c >= '0' && c <= '9'; i++)
					{code = code*10+c-'0'; c = get1key();}
#if DECEDT
				if (c == (CTRL|(27+'@')))
					{charpending=27; cpending=TRUE;}
#endif
				c = code;
			        if (c>=0x00 && c<=0x1F) c = CTRL | (c+'@');
			}
			inmeta = oldmeta;
			inspecial = TRUE;
		}
		if (inmeta == TRUE)	{inmeta = FALSE; c |= META;}
		if (incex == TRUE)	{incex = FALSE;	c |= CTLX;}
	}

	if (inspecial == TRUE)	{inspecial = FALSE; c |= SPEC;}

	/* otherwise, just return it */
	return(c);
}

/*	A more generalized prompt/reply function allowing the caller
	to specify the proper terminator. If the terminator is not
	a return ('\n') it will echo as "<NL>"
							*/
int getstring(prompt, buf, nbuf, eolchar)

CONSTA char *prompt;
char *buf;
int nbuf;
int eolchar;

{
	int status;

	complete(prompt, NULL, CMP_NONE, buf, nbuf, eolchar, FALSE, &status);

	return(status);
}

VOID outstring(s)	/* output a string of input characters */

CONSTA char *s;	/* string to output */

{
	if (disinp)
		while (*s)
			TTputc(*s++);
}

#ifdef NOT_USED

ostring(s)	/* output a string of output characters */

char *s;	/* string to output */

{
	if (discmd)
		while (*s)
			TTputc(*s++);
}

#endif
