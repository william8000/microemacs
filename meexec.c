/*	This file is for functions dealing with execution of
	commands, command lines, buffers, files and startup files

	written 1986 by Daniel Lawrence				*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

static int dobuf PP((BUFFER *bp));
static int docmd PP((char *cline));
static int cbuf PP((int f, int n, int bufnum));
static VOID errormesg PP((CONSTA char *mesg, BUFFER *bp, LINE *lp));
static VOID freewhile PP((WHBLOCK *wp));


/* namedcmd:	execute a named command even if it is not bound */

int namedcmd(f, n)

int f, n;	/* command arguments [passed through to command executed] */

{
	int (*kfunc) PP((int f, int n)); /* ptr to the requexted function to bind to */
#ifdef NO_PROTOTYPE
	int (*getname())();
#endif

	/* get the function name to execute */
	kfunc = getname(": ");
	if (kfunc == NULL) {
		mlwrite("[No such function]");
		return(FALSE);
	}

	/* and then execute the command */
	return((*kfunc)(f, n));
}

/*	execcmd:	Execute a command line command to be typed in
			by the user					*/

int execcmd(f, n)

int f, n;	/* default Flag and Numeric argument */

{
	register int status;		/* status return */
	char cmdstr[NSTRING];		/* string holding command to execute */

	UNUSED_ARGS_FN;

	/* get the line wanted */
	if ((status = mlreply(": ", cmdstr, NSTRING)) != TRUE)
		return(status);

	execlevel = 0;
	return(docmd(cmdstr));
}

/*	docmd:	take a passed string as a command line and translate
		it to be executed as a command. This function will be
		used by execute-command-line and by all source and
		startup files. Lastflag/thisflag is also updated.

	format of the command line is:

		{# arg} <command-name> {<argument string(s)>}

*/

static int docmd(cline)

char *cline;	/* command line to execute */

{
	register int f;		/* default argument flag */
	register int n;		/* numeric repeat value */
	int (*fnc) PP((int f, int n)); /* function to execute */
	int status;		/* return status of function */
	int oldcle;		/* old contents of clexec flag */
	char *oldestr;		/* original exec string */
	char tkn[NSTRING];	/* next token off of command line */
	char buf[NSTRING];	/* buffer for getval */
#ifdef NO_PROTOTYPE
	int (*fncmatch())();
#endif
		
	/* if we are scanning and not executing..go back here */
	if (execlevel)
		return(TRUE);

	oldestr = execstr;	/* save last ptr to string to execute */
	execstr = cline;	/* and set this one as current */

	/* first set up the default command values */
	f = FALSE;
	n = 1;
	lastflag = thisflag;
	thisflag = 0;

	if ((status = macarg(tkn)) != TRUE) {	/* and grab the first token */
		execstr = oldestr;
		return(status);
	}

	/* process leadin argument */
	if (gettyp(tkn) != TKCMD) {
		f = TRUE;
		strcpy(tkn, getval(tkn,buf));
		n = asc_int(tkn);

		/* and now get the command to execute */
		if ((status = macarg(tkn)) != TRUE) {
			execstr = oldestr;
			return(status);	
		}	
	}

	/* and match the token to see if it exists */
	if ((fnc = fncmatch(tkn)) == NULL) {
		mlwrite("[No such Function]");
		execstr = oldestr;
		return(FALSE);
	}
	
	/* save the arguments and go execute the command */
	oldcle = clexec;		/* save old clexec flag */
	clexec = TRUE;			/* in cline execution */
	status = (*fnc)(f, n);		/* call the function */
	cmdstatus = status;		/* save the status */
	clexec = oldcle;		/* restore clexec flag */
	execstr = oldestr;
	return(status);
}

/* token:	chop a token off a string
		return a pointer past the token
*/

char *token(src, tok, size)

char *src, *tok;	/* source string, destination token string */
int size;		/* maximum size of token */

{
	register int quotef;	/* is the current string quoted? */
	register char c;	/* temporary character */

	/* first scan past any whitespace in the source string */
	while (*src == ' ' || *src == '\t')
		++src;

	/* scan through the source string */
	quotef = FALSE;
	while (*src) {
		/* process special characters */
		if (*src == '~') {
			++src;
			if (*src == 0)
				break;
			switch (*src++) {
				case 'r':	c = 13; break;
				case 'n':	c = 10; break;
				case 'l':	c = 10; break;
				case 't':	c = 9;  break;
				case 'b':	c = 8;  break;
				case 'f':	c = 12; break;
				default:	c = *(src-1);
			}
			if (--size > 0) *tok++ = c;
		} else {
			/* check for the end of the token */
			if (quotef) {
				if (*src == '"')
					break;
			} else {
				if (*src == ' ' || *src == '\t')
					break;
			}

			/* set quote mode if qoute found */
			if (*src == '"')
				quotef = TRUE;

			/* record the character */
			c = *src++;
			if (--size > 0) *tok++ = c;
		}
	}

	/* terminate the token and exit */
	if (*src)
		++src;
	*tok = 0;
	return(src);
}

int macarg(tok)	/* get a macro line argument */

char *tok;	/* buffer to place argument */

{
	int savcle;	/* buffer to store original clexec */
	int status;

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	status = nextarg("", tok, NSTRING, ctoec('\n'));
	clexec = savcle;	/* restore execution mode */
	return(status);
}

/*	nextarg:	get the next argument	*/

int nextarg(prompt, buffer, size, terminator)

CONSTA char *prompt;	/* prompt to use if we must be interactive */
char *buffer;		/* buffer to put token into */
int size;		/* size of the buffer */
int terminator;		/* terminating char to be used on interactive fetch */

{
	register CONSTA char *sp;	/* return pointer from getval */
	char buf[NSTRING];		/* buffer for getval */

	/* if we are interactive, go get it! */
	if (clexec == FALSE)
		return(getstring(prompt, buffer, size, terminator));

	/* grab token and advance past */
	execstr = token(execstr, buffer, size);

	/* evaluate it */
	if ((sp = getval(buffer,buf)) == NULL) return(FALSE);
	if (buffer != sp) {
		/* buffer and sp might overlap so do not use memcpy */
		while (*sp != '\0') {
			*buffer++ = *sp++;
		}
		*buffer = '\0';
	}
	return(TRUE);
}

/*	storemac:	Set up a macro buffer and flag to store all
			executed command lines there			*/

int storemac(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register BUFFER *bp;		/* pointer to macro buffer */
	char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlwrite("No macro specified");
		return(FALSE);
	}

	/* range check the macro number */
	if (n < 1 || n > 99) {
		mlwrite("Macro number out of range");
		return(FALSE);
	}

	/* construct the macro buffer name */
	strcpy(bname, "[Macro xx]");
	bname[7] = (char) ('0' + (n / 10));
	bname[8] = (char) ('0' + (n % 10));

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite("Can not create macro");
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

#if	PROC
/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

int storeproc(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register BUFFER *bp;		/* pointer to macro buffer */
	register int status;		/* return status */
	char bname[NBUFN];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return(storemac(f, n));

	/* get the name of the procedure */
        if ((status = mlreply("Procedure name: ", &bname[1], NBUFN-2)) != TRUE)
                return(status);

	/* construct the macro buffer name */
	bname[0] = '[';
	strcat(bname, "]");

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite("Can not create macro");
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

/*	execproc:	Execute a procedure				*/

int execproc(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        char bufn[NBUFN];		/* name of buffer to execute */

	UNUSED_ARG(f);

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute procedure: ", &bufn[1], NBUFN-2)) != TRUE)
                return(status);

	/* construct the buffer name */
	bufn[0] = '[';
	strcat(bufn, "]");

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite("No such procedure");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}
#endif

/*	execbuf:	Execute the contents of a buffer of commands	*/

int execbuf(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int    s;		/* status */
#ifdef NO_PROTOTYPE
        BUFFER *getdefb();
#endif

	UNUSED_ARG(f);

	/* make a completion for a buffer name */
	bp = getdefb();
	bp = getcbuf("Execute buffer", bp ? bp->b_bname: "main", TRUE);

        if (bp == NULL) {
		mlwrite("No such buffer");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((s = dobuf(bp)) != TRUE)
			return(s);
	return(TRUE);
}

/*	dobuf:	execute the contents of the buffer pointed to
		by the passed BP

	Directives start with a "!" and include:

	!endm		End a macro
	!if (cond)	conditional execution
	!else
	!endif
	!return		Return (terminating current macro)
	!goto <label>	Jump to a label in the current macro
	!force		Force macro to continue...even if command fails
	!while (cond)	Execute a loop if the condition is true
	!endwhile
	
	Line Labels begin with a "*" as the first nonblank char, like:

	*LBL01
*/

static int dobuf(bp)

BUFFER *bp;	/* buffer to execute */

{
        register int status;	/* status return */
	register LINE *lp;	/* pointer to line to execute */
	register LINE *hlp;	/* pointer to line header */
	register LINE *glp;	/* line to goto */
	LINE *mp;		/* Macro line storage temp */
	int dirnum;		/* directive index */
	int linlen;		/* length of line to execute */
	int i;			/* index */
	int force;		/* force TRUE result? */
	WINDOW *wp;		/* ptr to windows to scan */
	WHBLOCK *whlist;	/* ptr to !WHILE list */
	WHBLOCK *scanner;	/* ptr during scan */
	WHBLOCK *whtemp;	/* temporary ptr to a WHBLOCK */
	char *einit;		/* initial value of eline */
	char *eline;		/* text of line to execute */
	char tkn[NSTRING];	/* buffer to evaluate an expresion in */

#if	DEBUGM
	char *sp;			/* temp for building debug string */
	register char *ep;	/* ptr to end of outline */
#endif

	/* clear IF level flags/while ptr */
	execlevel = 0;
	whlist = NULL;
	scanner = NULL;

	/* scan the buffer to execute, building WHILE header blocks */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* scan the current line */
		eline = lp->l_text;
		i = lp->l_used;

		/* trim leading whitespace */
		while (i-- > 0 && (*eline == ' ' || *eline == '\t'))
			++eline;

		/* if theres nothing here, don't bother */
		if (i <= 0)
			goto nxtscan;

		/* if is a while directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'w' && eline[2] == 'h') {
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL) {
noram:				errormesg("%%Out of memory during while scan",
						bp, lp);
failexit:			freewhile(scanner);
				freewhile(whlist);
				return(FALSE);
			}
			whtemp->w_begin = lp;
			whtemp->w_type = BTWHILE;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if is a BREAK directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'b' && eline[2] == 'r') {
			if (scanner == NULL) {
				errormesg(
				  "%%!BREAK outside of any !WHILE loop",bp,lp);
				goto failexit;
			}
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL)
				goto noram;
			whtemp->w_begin = lp;
			whtemp->w_type = BTBREAK;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if it is an endwhile directive, record the spot... */
		if (eline[0] == '!' && strncmp(&eline[1], "endw", 4) == 0) {
			if (scanner == NULL) {
				errormesg(
				  "%%!ENDWHILE with no preceding !WHILE",bp,lp);
				goto failexit;
			}
			/* move top records from the scanner list to the
			   whlist until we have moved all BREAK records
			   and one WHILE record */
			do {
				scanner->w_end = lp;
				whtemp = whlist;
				whlist = scanner;
				scanner = scanner->w_next;
				whlist->w_next = whtemp;
			} while (whlist->w_type == BTBREAK);
		}

nxtscan:	/* on to the next line */
		lp = lp->l_fp;
	}

	/* while and endwhile should match! */
	if (scanner != NULL) {
		errormesg("%%!WHILE with no matching !ENDWHILE", bp, lp);
		goto failexit;
	}

	/* let the first command inherit the flags from the last one..*/
	thisflag = lastflag;

	/* starting at the beginning of the buffer */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((einit = eline = malloc(linlen+1)) == NULL) {
			errormesg("%%Out of Memory during macro execution",
					bp, lp);
			freewhile(whlist);
			return(FALSE);
		}
		bytecopy(eline, lp->l_text, linlen);
		eline[linlen] = 0;	/* make sure it ends */

		/* trim leading whitespace */
		while (*eline == ' ' || *eline == '\t')
			++eline;

		/* dump comments and blank lines */
		if (*eline == ';' || *eline == 0)
			goto onward;

#if	DEBUGM
		/* if $debug == TRUE, every line to execute
		   gets echoed and a key needs to be pressed to continue
		   ^G will abort the command */
	
		if (macbug) {
			strcpy(outline, "<<<");
	
			/* debug macro name */
			strcat(outline, bp->b_bname);
			strcat(outline, ":");
	
			/* debug if levels */
			strcat(outline, me_itoa(execlevel));
			strcat(outline, ":");

			/* and lastly the line */
			strcat(outline, eline);
			strcat(outline, ">>>");
	
			/* change all '%' to ':' so mlwrite won't expect arguments */
			sp = outline;
			while (*sp)
			    if (*sp++ == '%') {
				/* advance to the end */
				ep = --sp;
				while (*ep++)
					;
				/* null terminate the string one out */
				*(ep + 1) = 0;
				/* copy backwards */
				while(ep-- > sp)
					*(ep + 1) = *ep;

				/* and advance sp past the new % */
				sp += 2;					
			    }
	
			/* write out the debug line */
			mlforce(outline);
			update(TRUE);
	
			/* and get the keystroke */
			if (get1key() == abortc) {
				mlforce("[Macro aborted]");
				freewhile(whlist);
				return(FALSE);
			}
		}
#endif

		/* Parse directives here.... */
		dirnum = -1;
		if (*eline == '!') {
			/* Find out which directive this is */
			++eline;
			for (dirnum = 0; dirnum < NUMDIRS; dirnum++)
				if (strncmp(eline, dname[dirnum],
				            strlen(dname[dirnum])) == 0)
					break;

			/* and bitch if it's illegal */
			if (dirnum == NUMDIRS) {
				errormesg("%%Unknown Directive", bp, lp);
				freewhile(whlist);
				return(FALSE);
			}

			/* service only the !ENDM macro here */
			if (dirnum == DENDM) {
				mstore = FALSE;
				bstore = NULL;
				goto onward;
			}

			/* restore the original eline....*/
			--eline;
		}

		/* if macro store is on, just salt this away */
		if (mstore) {
			/* allocate the space for the line */
			linlen = (int) strlen(eline);
			if ((mp=lalloc(linlen)) == NULL) {
				mlwrite("Out of memory while storing macro");
				return (FALSE);
			}
	
			/* copy the text into the new line */
			for (i=0; i<linlen; ++i)
				lputc(mp, i, eline[i]);
	
			/* attach the line to the end of the buffer */
	       		bstore->b_linep->l_bp->l_fp = mp;
			mp->l_bp = bstore->b_linep->l_bp;
			bstore->b_linep->l_bp = mp;
			mp->l_fp = bstore->b_linep;
			goto onward;
		}
	

		force = FALSE;

		/* dump comments */
		if (*eline == '*')
			goto onward;

		/* now, execute directives */
		if (dirnum != -1) {
			/* skip past the directive */
			while (*eline && *eline != ' ' && *eline != '\t')
				++eline;
			execstr = eline;

			switch (dirnum) {
			case DIF:	/* IF directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == FALSE)
						++execlevel;
				} else
					++execlevel;
				goto onward;

			case DWHILE:	/* WHILE directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == TRUE)
						goto onward;
				}
				/* drop down and act just like !BREAK */
				/* fall through */

			case DBREAK:	/* BREAK directive */
				if (dirnum == DBREAK && execlevel)
					goto onward;

				/* jump down to the endwhile */
				/* find the right while loop */
				whtemp = whlist;
				while (whtemp) {
					if (whtemp->w_begin == lp)
						break;
					whtemp = whtemp->w_next;
				}
			
				if (whtemp == NULL) {
					mlwrite("%%Internal While loop error");
					freewhile(whlist);
					return(FALSE);
				}
			
				/* reset the line pointer back.. */
				lp = whtemp->w_end;
				goto onward;

			case DELSE:	/* ELSE directive */
				if (execlevel == 1)
					--execlevel;
				else if (execlevel == 0 )
					++execlevel;
				goto onward;

			case DENDIF:	/* ENDIF directive */
				if (execlevel)
					--execlevel;
				goto onward;

			case DGOTO:	/* GOTO directive */
				/* .....only if we are currently executing */
				if (execlevel == 0) {

					/* grab label to jump to */
					eline = token(eline, golabel, NPAT);
					linlen = (int) strlen(golabel);
					glp = hlp->l_fp;
					while (glp != hlp) {
						if (*glp->l_text == '*' &&
						    (strncmp(&glp->l_text[1], golabel,
						            linlen) == 0)) {
							lp = glp;
							goto onward;
						}
						glp = glp->l_fp;
					}
					mlwrite("%%No such label");
					freewhile(whlist);
					return(FALSE);
				}
				goto onward;
	
			case DRETURN:	/* RETURN directive */
				if (execlevel == 0)
					goto eexec;
				goto onward;

			case DENDWHILE:	/* ENDWHILE directive */
				if (execlevel) {
					--execlevel;
					goto onward;
				} else {
					/* find the right while loop */
					whtemp = whlist;
					while (whtemp) {
						if (whtemp->w_type == BTWHILE &&
						    whtemp->w_end == lp)
							break;
						whtemp = whtemp->w_next;
					}
		
					if (whtemp == NULL) {
						mlwrite("%%Internal While loop error");
						freewhile(whlist);
						return(FALSE);
					}
		
					/* reset the line pointer back.. */
					lp = whtemp->w_begin->l_bp;
					goto onward;
				}

			case DFORCE:	/* FORCE directive */
				force = TRUE;
				break;

			default:	/* should never happen */
				break;

			}
		}

		/* execute the statement */
		status = docmd(eline);
		if (force)		/* force the status */
			status = TRUE;

		/* check for a command error */
		if (status != TRUE) {
			/* look if buffer is showing */
			wp = wheadp;
			while (wp != NULL) {
				if (wp->w_bufp == bp) {
					/* and point it */
					wp->w_dotp = lp;
					wp->w_doto = 0;
					wp->w_flag |= WFHARD;
				}
				wp = wp->w_wndp;
			}
			/* in any case set the buffer . */
			bp->b_dotp = lp;
			bp->b_doto = 0;
			free(einit);
			execlevel = 0;
			freewhile(whlist);
			return(status);
		}

onward:		/* on to the next line */
		free(einit);
		lp = lp->l_fp;
	}

eexec:	/* exit the current function */
	execlevel = 0;
	freewhile(whlist);
        return(TRUE);
}

/* errormesg:	display a macro execution error along with the buffer and
		line currently being executed */

static VOID errormesg(mesg, bp, lp)

CONSTA char *mesg;	/* error message to display */
BUFFER *bp;		/* buffer error occured in */
LINE *lp;		/* line " */

{
	char buf[NSTRING];

	/* build error message line */
	strcpy(buf, mesg);
	strcat(buf, " in <");
	strcat(buf, bp->b_bname);
	strcat(buf, "> at line ");
	strcat(buf, int_asc(getlinenum(bp, lp)));
	mlforce(buf);
}

VOID makelit(s, len)	/* expand all "%" to "%%" */
char *s;
int len;
{
	register char *sp;	/* temp for expanding string */
	register char *ep;	/* ptr to end of string to expand */
	int templen;

	sp = s;
	len -= 2;
	while(*sp && len-- > 0)
		if (*sp++ == '%') {
			/* advance to the end */
			ep = --sp;
			templen = len+1;
			while(*ep++ != '\0' && templen-- >= 0) ;
			/* null terminate the string one out */
			*(ep+1) = '\0';
			/* copy backwards */
			while(ep-- > sp)
				*(ep+1) = *ep;
			/* and advance sp past the new % */
			sp += 2;
			--len;
		}
	*sp = '\0';
}

static VOID freewhile(wp)	/* free a list of while block pointers */

WHBLOCK *wp;	/* head of structure to free */

{
	if (wp == NULL)
		return;
	if (wp->w_next)
		freewhile(wp->w_next);
	free((char *)wp);
}

int execfile(f, n)	/* execute a series of commands in a file */

int f, n;	/* default flag and numeric arg to pass on to file */

{
	register int status;	/* return status of name query */
	char fname[NSTRING];	/* name of file to execute */
	CONSTA char *fspec;	/* full file spec */

	UNUSED_ARG(f);

	fspec = gtfilename("File to execute", FALSE);
	if (!fspec)
		return(FALSE);
	bytecopy(fname, fspec, NSTRING-1);

#if	1
	/* look up the path for the file */
	fspec = flook(fname, TRUE);

	/* if it isn't around */
	if (fspec == NULL)
		return(FALSE);

#endif
	/* otherwise, execute it */
	while (n-- > 0)
		if ((status=dofile(fspec)) != TRUE)
			return(status);

	return(TRUE);
}

/*	dofile:	yank a file into a buffer and execute it
		if there are no errors, delete the buffer on exit */

int dofile(fname)

CONSTA char *fname;	/* file name to execute */

{
	register BUFFER *bp;	/* buffer to place file to exeute */
	register BUFFER *cb;	/* temp to hold current buf while we read */
	register int status;	/* results of various calls */
	char bname[NBUFN];	/* name of buffer */
	int old_buffer;		/* true if using an existing buffer */
	int old_mode;		/* saved mode */

	makename(bname, fname);		/* derive the name of the buffer */
	bp = bfind(bname, FALSE, 0);
	old_buffer = (bp != NULL);
	if (bp == NULL &&
	    (bp = bfind(bname, TRUE, 0)) == NULL) /* get the needed buffer */
		return(FALSE);

	old_mode = bp->b_mode;
	bp->b_mode |= MDVIEW;	/* mark the buffer as read only */
	cb = curbp;		/* save the old buffer */
	curbp = bp;		/* make this one current */
	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE, FALSE)) != TRUE) {
		curbp = cb;	/* restore the current buffer */
		return(status);
	}

	/* go execute it! */
	curbp = cb;		/* restore the current buffer */
	if ((status = dobuf(bp)) != TRUE)
		return(status);

	/* if not displayed, remove the now unneeded buffer and exit */
	if (bp->b_nwnd == 0 && !old_buffer)
		zotbuf(bp);
	else
		bp->b_mode = old_mode;
	return(TRUE);
}

/*	cbuf:	Execute the contents of a numbered buffer	*/

static int cbuf(f, n, bufnum)

int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
	char bufname[12];

	UNUSED_ARG(f);

	/* make the buffer name */
	strcpy(bufname, "[Macro xx]");
	bufname[7] = (char) ('0' + (bufnum / 10));
	bufname[8] = (char) ('0' + (bufnum % 10));

	/* find the pointer to that buffer */
        if ((bp=bfind(bufname, FALSE, 0)) == NULL) {
        	mlwrite("Macro not defined");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

int cbuf1(f, n) int f, n; { return(cbuf(f, n, 1)); }
int cbuf2(f, n) int f, n; { return(cbuf(f, n, 2)); }
int cbuf3(f, n) int f, n; { return(cbuf(f, n, 3)); }
int cbuf4(f, n) int f, n; { return(cbuf(f, n, 4)); }
int cbuf5(f, n) int f, n; { return(cbuf(f, n, 5)); }
int cbuf6(f, n) int f, n; { return(cbuf(f, n, 6)); }
int cbuf7(f, n) int f, n; { return(cbuf(f, n, 7)); }
int cbuf8(f, n) int f, n; { return(cbuf(f, n, 8)); }
int cbuf9(f, n) int f, n; { return(cbuf(f, n, 9)); }
int cbuf10(f, n) int f, n; { return(cbuf(f, n, 10)); }
int cbuf11(f, n) int f, n; { return(cbuf(f, n, 11)); }
int cbuf12(f, n) int f, n; { return(cbuf(f, n, 12)); }
int cbuf13(f, n) int f, n; { return(cbuf(f, n, 13)); }
int cbuf14(f, n) int f, n; { return(cbuf(f, n, 14)); }
int cbuf15(f, n) int f, n; { return(cbuf(f, n, 15)); }
int cbuf16(f, n) int f, n; { return(cbuf(f, n, 16)); }
int cbuf17(f, n) int f, n; { return(cbuf(f, n, 17)); }
int cbuf18(f, n) int f, n; { return(cbuf(f, n, 18)); }
int cbuf19(f, n) int f, n; { return(cbuf(f, n, 19)); }
int cbuf20(f, n) int f, n; { return(cbuf(f, n, 20)); }
int cbuf21(f, n) int f, n; { return(cbuf(f, n, 21)); }
int cbuf22(f, n) int f, n; { return(cbuf(f, n, 22)); }
int cbuf23(f, n) int f, n; { return(cbuf(f, n, 23)); }
int cbuf24(f, n) int f, n; { return(cbuf(f, n, 24)); }
int cbuf25(f, n) int f, n; { return(cbuf(f, n, 25)); }
int cbuf26(f, n) int f, n; { return(cbuf(f, n, 26)); }
int cbuf27(f, n) int f, n; { return(cbuf(f, n, 27)); }
int cbuf28(f, n) int f, n; { return(cbuf(f, n, 28)); }
int cbuf29(f, n) int f, n; { return(cbuf(f, n, 29)); }
int cbuf30(f, n) int f, n; { return(cbuf(f, n, 30)); }
int cbuf31(f, n) int f, n; { return(cbuf(f, n, 31)); }
int cbuf32(f, n) int f, n; { return(cbuf(f, n, 32)); }
int cbuf33(f, n) int f, n; { return(cbuf(f, n, 33)); }
int cbuf34(f, n) int f, n; { return(cbuf(f, n, 34)); }
int cbuf35(f, n) int f, n; { return(cbuf(f, n, 35)); }
int cbuf36(f, n) int f, n; { return(cbuf(f, n, 36)); }
int cbuf37(f, n) int f, n; { return(cbuf(f, n, 37)); }
int cbuf38(f, n) int f, n; { return(cbuf(f, n, 38)); }
int cbuf39(f, n) int f, n; { return(cbuf(f, n, 39)); }
int cbuf40(f, n) int f, n; { return(cbuf(f, n, 40)); }
int cbuf41(f, n) int f, n; { return(cbuf(f, n, 41)); }
int cbuf42(f, n) int f, n; { return(cbuf(f, n, 42)); }
int cbuf43(f, n) int f, n; { return(cbuf(f, n, 43)); }
int cbuf44(f, n) int f, n; { return(cbuf(f, n, 44)); }
int cbuf45(f, n) int f, n; { return(cbuf(f, n, 45)); }
int cbuf46(f, n) int f, n; { return(cbuf(f, n, 46)); }
int cbuf47(f, n) int f, n; { return(cbuf(f, n, 47)); }
int cbuf48(f, n) int f, n; { return(cbuf(f, n, 48)); }
int cbuf49(f, n) int f, n; { return(cbuf(f, n, 49)); }
int cbuf50(f, n) int f, n; { return(cbuf(f, n, 50)); }
int cbuf51(f, n) int f, n; { return(cbuf(f, n, 51)); }
int cbuf52(f, n) int f, n; { return(cbuf(f, n, 52)); }
int cbuf53(f, n) int f, n; { return(cbuf(f, n, 53)); }
int cbuf54(f, n) int f, n; { return(cbuf(f, n, 54)); }
int cbuf55(f, n) int f, n; { return(cbuf(f, n, 55)); }
int cbuf56(f, n) int f, n; { return(cbuf(f, n, 56)); }
int cbuf57(f, n) int f, n; { return(cbuf(f, n, 57)); }
int cbuf58(f, n) int f, n; { return(cbuf(f, n, 58)); }
int cbuf59(f, n) int f, n; { return(cbuf(f, n, 59)); }
int cbuf60(f, n) int f, n; { return(cbuf(f, n, 60)); }
int cbuf61(f, n) int f, n; { return(cbuf(f, n, 61)); }
int cbuf62(f, n) int f, n; { return(cbuf(f, n, 62)); }
int cbuf63(f, n) int f, n; { return(cbuf(f, n, 63)); }
int cbuf64(f, n) int f, n; { return(cbuf(f, n, 64)); }
int cbuf65(f, n) int f, n; { return(cbuf(f, n, 65)); }
int cbuf66(f, n) int f, n; { return(cbuf(f, n, 66)); }
int cbuf67(f, n) int f, n; { return(cbuf(f, n, 67)); }
int cbuf68(f, n) int f, n; { return(cbuf(f, n, 68)); }
int cbuf69(f, n) int f, n; { return(cbuf(f, n, 69)); }
int cbuf70(f, n) int f, n; { return(cbuf(f, n, 70)); }
int cbuf71(f, n) int f, n; { return(cbuf(f, n, 71)); }
int cbuf72(f, n) int f, n; { return(cbuf(f, n, 72)); }
int cbuf73(f, n) int f, n; { return(cbuf(f, n, 73)); }
int cbuf74(f, n) int f, n; { return(cbuf(f, n, 74)); }
int cbuf75(f, n) int f, n; { return(cbuf(f, n, 75)); }
int cbuf76(f, n) int f, n; { return(cbuf(f, n, 76)); }
int cbuf77(f, n) int f, n; { return(cbuf(f, n, 77)); }
int cbuf78(f, n) int f, n; { return(cbuf(f, n, 78)); }
int cbuf79(f, n) int f, n; { return(cbuf(f, n, 79)); }
int cbuf80(f, n) int f, n; { return(cbuf(f, n, 80)); }
int cbuf81(f, n) int f, n; { return(cbuf(f, n, 81)); }
int cbuf82(f, n) int f, n; { return(cbuf(f, n, 82)); }
int cbuf83(f, n) int f, n; { return(cbuf(f, n, 83)); }
int cbuf84(f, n) int f, n; { return(cbuf(f, n, 84)); }
int cbuf85(f, n) int f, n; { return(cbuf(f, n, 85)); }
int cbuf86(f, n) int f, n; { return(cbuf(f, n, 86)); }
int cbuf87(f, n) int f, n; { return(cbuf(f, n, 87)); }
int cbuf88(f, n) int f, n; { return(cbuf(f, n, 88)); }
int cbuf89(f, n) int f, n; { return(cbuf(f, n, 89)); }
int cbuf90(f, n) int f, n; { return(cbuf(f, n, 90)); }
int cbuf91(f, n) int f, n; { return(cbuf(f, n, 91)); }
int cbuf92(f, n) int f, n; { return(cbuf(f, n, 92)); }
int cbuf93(f, n) int f, n; { return(cbuf(f, n, 93)); }
int cbuf94(f, n) int f, n; { return(cbuf(f, n, 94)); }
int cbuf95(f, n) int f, n; { return(cbuf(f, n, 95)); }
int cbuf96(f, n) int f, n; { return(cbuf(f, n, 96)); }
int cbuf97(f, n) int f, n; { return(cbuf(f, n, 97)); }
int cbuf98(f, n) int f, n; { return(cbuf(f, n, 98)); }
int cbuf99(f, n) int f, n; { return(cbuf(f, n, 99)); }
