/*	This file is for functions having to do with key bindings,
	descriptions, help commands and startup file.

	written 11-feb-86 by Daniel Lawrence
								*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include	"epath.h"

extern int meta(), cex(), unarg(), ctrlg(); /* dummy prefix binding functions */

static unsigned int stock PP((char *keyname));
static int chunbind PP((int c));
static unsigned int getckey PP((int mflag));
static CONSTA char *getfname PP((int (*func)(int f, int n)));
#if APROP
static int strinc PP((CONSTA char *source, CONSTA char *sub));
#endif

#define	ostring(s)	if (discmd || !clexec) outstring(s)

int me_help(f, n) /* give me some help!!!!
		   bring up a fake buffer and read the help file
		   into it with view mode			*/
int f, n;
{
	register BUFFER *bp;	/* buffer pointer to help */
	char *fname;	/* ptr to file returned by flook() */

	fname = NULL;

	/* first check if we are already here */
	bp = bfind("emacs.hlp", FALSE, BFINVS);

	if (bp == NULL) {
		fname = (char *) flook(pathname[1], FALSE);
		if (fname == NULL) {
			mlwrite("[Help file is not online]");
			return(FALSE);
		}
	}

	/* split the current window to make room for the help stuff */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);
	curwp = spltwp; curbp = curwp->w_bufp;

	if (bp == NULL) {
		/* and read the stuff in */
		if (getfile(fname, FALSE) == FALSE)
			return(FALSE);
	} else
		swbuffer(bp);

	/* make this window in VIEW mode, update all mode lines */
	curbp->b_mode |= MDVIEW;
	curbp->b_flag |= BFINVS;
	upmode();
	return(TRUE);
}

int deskey(f, n)	/* describe the command for a certain key */
int f, n;

{
	register int c;		/* key to describe */
	CONSTA char *ptr;	/* string pointer to scan output strings */
	char outseq[NSTRING];	/* output buffer for command sequence */
#ifdef NO_PROTOTYPE
#if	C86 | DECUSC | DARWIN
	int *getbind(int c);
#else
	int (*getbind(int c))(int f, int n);
#endif
#endif

	/* prompt the user to type us a key to describe */
	mlwrite(": describe-key ");

	/* get the command sequence to describe
	   change it to something we can print as well */
	cmdstr(c = getckey(FALSE), &outseq[0]);

	/* and dump it out */
	ostring(outseq);
	ostring(" ");

	/* find the right ->function */
	if ((ptr = getfname(getbind(c))) == NULL)
		ptr = "Not Bound";

	/* output the command sequence */
	ostring(ptr);

	return(TRUE);
}

/* bindtokey:	add a new key to the key binding table		*/

int bindtokey(f, n)

int f, n;	/* command arguments [IGNORED] */

{
	register unsigned int c;/* command key to bind */
	register int (*kfunc)(int f, int n);  /* ptr to the requested function to bind to */
	register KEYTAB *ktp;	/* pointer into the command table */
	register int found;	/* matched command flag */
	char outseq[80];	/* output buffer for keystroke sequence */
#ifdef NO_PROTOTYPE
	int (*getname(CONSTA char *prompt))(int f, int n);
#endif
#if	C86 | DECUSC
	int (*fmeta)(), (*fcex)(), (*funarg)(), (*fctrlg)();

	fmeta = meta; fcex = cex; funarg = unarg; fctrlg = ctrlg;
#else
#define	fmeta	meta
#define	fcex	cex
#define	funarg	unarg
#define	fctrlg	ctrlg
#endif


	/* get the function name to bind it to */
	kfunc = getname(": bind-to-key ");
	if (kfunc == NULL) {
		mlwrite("[No such function]");
		return(FALSE);
	}
	ostring(" ");

	/* get the command sequence to bind */
	TTflush();
	c = getckey((kfunc == fmeta) || (kfunc == fcex) ||
	            (kfunc == funarg) || (kfunc == fctrlg));

	/* change it to something we can print as well */
	cmdstr((int) c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if the function is a prefix key */
	if (kfunc == fmeta || kfunc == fcex ||
	    kfunc == funarg || kfunc == fctrlg) {
		/* search for an existing binding for the prefix key */
		ktp = &keytab[0];
#if DECEDT
		if (kfunc == funarg || kfunc == fctrlg)
#endif
		while (ktp->k_fp != NULL) {
			if (ktp->k_fp == kfunc)
				chunbind(ktp->k_code);
			++ktp;
		}
	}

	/* search the table to see if it exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_fp != NULL) {
		if (c == (int) ktp->k_code) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	if (found) {	/* it exists, just change it then */
		if (c == metac) metac = 0;
		if (c == ctlxc) ctlxc = 0;
		if (c == reptc) reptc = 0;
		if (c == abortc) abortc = 0;

		ktp->k_fp = kfunc;
		if (c > 0 && c < NFBIND) fkeytab[c] = kfunc;
		else if (c == (FUNC|'C')) ffuncc = kfunc;
		fkeylast.k_code = (short) c;
		fkeylast.k_fp = kfunc;
	} else {	/* otherwise we need to add it to the end */
		/* if we run out of binding room, bitch */
		if (ktp >= &keytab[NBINDS]) {
			mlwrite("Binding table FULL!");
			return(FALSE);
		}

		ktp->k_code = (short) c; /* add keycode */
		ktp->k_fp = kfunc;	/* and the function pointer */
		++ktp;			/* and make sure the next is null */
		ktp->k_code = 0;
		ktp->k_fp = NULL;
		if (c > 0 && c < NFBIND) fkeytab[c] = kfunc;
		else if (c == (FUNC|'C')) ffuncc = kfunc;
		fkeylast.k_code = (short) c;
		fkeylast.k_fp = kfunc;
	}

	/* reset the appropriate global prefix variable */
	if (metac == 0 && kfunc == fmeta) metac = c;
	if (ctlxc == 0 && kfunc == fcex) ctlxc = c;
	if (reptc == 0 && kfunc == funarg) reptc = c;
	if (abortc == 0 && kfunc == fctrlg) abortc = c;

	return(TRUE);
}

/* unbindkey:	delete a key from the key binding table	*/

int unbindkey(f, n)

int f, n;	/* command arguments [IGNORED] */

{
	register int c;		/* command key to unbind */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to unbind */
	mlwrite(": unbind-key ");

	/* get the command sequence to unbind */
	c = getckey(FALSE);		/* get a command sequence */

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if it isn't bound, bitch */
	if (chunbind(c) == FALSE) {
		mlwrite("[Key not bound]");
		return(FALSE);
	}
	return(TRUE);
}

static int chunbind(c)

int c;		/* command key to unbind */

{
	register KEYTAB *ktp;	/* pointer into the command table */
	register KEYTAB *sktp;	/* saved pointer into the command table */
	register int found;	/* matched command flag */

	/* search the table to see if the key exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_fp != NULL) {
		if (c == (int) ktp->k_code) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	/* if it isn't bound, bitch */
	if (!found)
		return(FALSE);

	/* save the pointer and scan to the end of the table */
	sktp = ktp;
	while (ktp->k_fp != NULL)
		++ktp;
	--ktp;		/* backup to the last legit entry */

	/* copy the last entry to the current one */
	sktp->k_code = ktp->k_code;
	sktp->k_fp   = ktp->k_fp;

	/* null out the last one */
	ktp->k_code = 0;
	ktp->k_fp = NULL;

	/* clear in fast lookup area */
	if (c > 0 && c < NFBIND) fkeytab[c] = NULL;
	else if (c == (FUNC|'C')) ffuncc = NULL;
	fkeylast.k_code = keytab[0].k_code;
	fkeylast.k_fp = keytab[0].k_fp;

        if (c == metac) metac = 0;
        if (c == ctlxc) ctlxc = 0;
        if (c == reptc) reptc = 0;
        if (c == abortc) abortc = 0;

	return(TRUE);
}

int desbind(f, n)	/* describe bindings
		   bring up a fake buffer and list the key bindings
		   into it with view mode			*/
int f, n;

#if	APROP
{
	return(buildlist(1, ""));
}

int apro(f, n)	/* Apropos (List functions that match a substring) */
int f, n;

{
	char mstring[NSTRING];	/* string to match cmd names to */
	int status;		/* status return */

	status = mlreply("Apropos string: ", mstring, NSTRING - 1);
	if (status != TRUE)
		return(status);

	return(buildlist(0, mstring));
}

int buildlist(type, mstring)  /* build a binding list (limited or full) */

int type;	/* 2 = initial match list, 1 = full list, 0 = partial list */
CONSTA char *mstring;	/* match string if a partial list */

#endif
{
#if	ST520 & LATTICE
#define	register		
#endif
	register KEYTAB *ktp;	/* pointer into the command table */
	register NBIND *nptr;	/* pointer into the name binding table */
	register BUFFER *listbuf; /* buffer to put binding list into */
	int cpos;		/* current position to use in outseq */
	char outseq[80];	/* output buffer for keystroke sequence */
	int mlen, len;

	/* get a buffer for the binding list */
	listbuf = bfind("Binding list", TRUE, 0);
	if (listbuf == NULL || bclear(listbuf) == FALSE) {
		mlwrite("Can not display binding list");
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite("[Building binding list]");

	mlen = strlen(mstring);

	/* build the contents of this window, inserting it line by line */
	nptr = &names[0];
	while (nptr->n_func != NULL) {

		/* add in the command name */
		strcpy(outseq, nptr->n_name);
		cpos = len = strlen(outseq);
		
#if	APROP
		/* if we are executing an apropos command..... */
		if (type == 0 &&
		    /* and current string doesn't include the search string */
		    strinc(outseq, mstring) == FALSE)
			goto fail;
#endif
		if (type == 2 && mlen > 0) {
			if (len > mlen) len = mlen;
			if (strncmp(outseq, mstring, len))
				goto fail;
		}

		/* search down any keys bound to this */
		ktp = &keytab[0];
		while (ktp->k_fp != NULL) {
			if (ktp->k_fp == nptr->n_func) {
				/* pad out some spaces */
				while (cpos < 25)
					outseq[cpos++] = ' ';

				/* add in the command sequence */
				cmdstr(ktp->k_code, &outseq[cpos]);

				/* and add it as a line into the buffer */
				if (addline(listbuf, outseq) != TRUE)
					return(FALSE);

				cpos = 0;	/* and clear the line */
			}
			++ktp;
		}

		/* if no key was bound, we need to dump it anyway */
		if (cpos > 0)	{
			outseq[cpos] = 0;
			if (addline(listbuf, outseq) != TRUE)
				return(FALSE);
		}

fail:		/* and on to the next name */
		++nptr;
	}

	wpopup(listbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

#if	APROP
static int strinc(source, sub)	/* does source include sub? */

CONSTA char *source;	/* string to search in */
CONSTA char *sub;	/* substring to look for */

{
	CONSTA char *sp;	/* ptr into source */
	CONSTA char *nxtsp;	/* next ptr into source */
	CONSTA char *tp;	/* ptr into substring */

	/* for each character in the source string */
	sp = source;
	while (*sp) {
		tp = sub;
		nxtsp = sp;

		/* is the substring here? */
		while (*tp) {
			if (*nxtsp++ != *tp)
				break;
			else
				tp++;
		}

		/* yes, return a success */
		if (*tp == 0)
			return(TRUE);

		/* no, onward */
		sp++;
	}
	return(FALSE);
}
#endif

/* get a command key sequence from the keyboard	*/

static unsigned int getckey(mflag)

int mflag;	/* going for a meta sequence? */

{
	register unsigned int c;	/* character fetched */
	char tok[NSTRING];		/* command incoming */

	/* check to see if we are executing a command line */
	if (clexec) {
		macarg(tok);	/* get the next token */
		return(stock(tok));
	}

	/* or the normal way */
	if (mflag)
		c = get1key();
	else
		c = getcmd();
	return(c);
}

/* execute the startup file */

int startup(sfname)

CONSTA char *sfname;	/* name of startup file (null if default) */

{
	CONSTA char *fname;	/* resulting file name to execute */

	/* look up the startup file */
	if (*sfname != 0)
		fname = flook(sfname, TRUE);
	else
		fname = flook(pathname[0], TRUE);

	/* if it isn't around, don't sweat it */
	if (fname == NULL)
		return(TRUE);

	/* otherwise, execute the sucker */
	return(dofile(fname));
}

/*	Look up the existance of a file along the normal or PATH
	environment variable. Look first in the HOME directory if
	asked and possible
*/

CONSTA char *flook(fname, hflag)

CONSTA char *fname;	/* base file name to search for */
int hflag;	/* Look in the HOME environment variable first? */

{
	register char *home;	/* path to home directory */
	register char *path;	/* environmental PATH variable */
	register char *sp;	/* pointer into path spec */
	register int i;		/* index */
	static char FAR fspec[NSTRING];	/* full path spec to search */
	char *getenv();

	if (fname == NULL || *fname == '\0' || strcmp(fname, "-") == 0) {
		return(NULL);
	}

	/* always try the current directory first */
	if (ffropen(fname, FALSE) == FIOSUC) {
		ffrclose();
		return(fname);
	}

#if	ENVFUNC

	if (hflag) {
		home = getenv("HOME");
		if (home != NULL) {
			/* build home dir file spec */
			strcpy(fspec, home);
#if	VMS
#else
#if	MSDOS
			strcat(fspec, "\\");
#else
			strcat(fspec, "/");
#endif
#endif
			strcat(fspec, fname);

			/* and try it out */
			if (ffropen(fspec, FALSE) == FIOSUC) {
				ffrclose();
				return(fspec);
			}
		}
	}
#endif

#if	ENVFUNC && (VMS == 0)
	/* get the PATH variable */
	path = getenv("PATH");
	if (path != NULL)
		while (*path) {

			/* build next possible file spec */
			sp = fspec;
			while (*path && (*path != PATHCHR))
				*sp++ = *path++;
#if	VMS
#else
#if	MSDOS
			if (sp > fspec && *(sp-1) != '\\') *sp++ = '\\';
#else
			if (sp > fspec && *(sp-1) != '/') *sp++ = '/';
#endif
#endif
			*sp = 0;
			strcat(fspec, fname);

			/* and try it out */
			if (ffropen(fspec, FALSE) == FIOSUC) {
				ffrclose();
				return(fspec);
			}

			if (*path == PATHCHR)
				++path;
		}
#endif

	/* look it up via the old table method */
	for (i=2; i < NPNAMES; i++) {
		strcpy(fspec, pathname[i]);
		strcat(fspec, fname);

		/* and try it out */
		if (ffropen(fspec, FALSE) == FIOSUC) {
			ffrclose();
			return(fspec);
		}
	}

	return(NULL);	/* no such luck */
}

VOID cmdstr(c, seq)	/* change a key command to a string we can print out */

int c;		/* sequence to translate */
char *seq;	/* destination string for sequence */

{
	char *ptr;	/* pointer into current position in sequence */

	ptr = seq;

	/* apply meta sequence if needed */
	if (c & META) {
		*ptr++ = 'M';
		*ptr++ = '-';
	}

	/* apply ^X sequence if needed */
	if (c & CTLX) {
		*ptr++ = '^';
		*ptr++ = 'X';
	}

	/* apply SPEC sequence if needed */
	if (c & SPEC) {
		*ptr++ = 'F';
		*ptr++ = 'N';
	}

	/* apply control sequence if needed */
	if (c & CTRL) {
		*ptr++ = '^';
	}

	c = c & 255;	/* strip the prefixes */

	/* and output the final sequence */

	*ptr++ = (char) c;
	*ptr = 0;	/* terminate the string */
}

/*	This function looks a key binding up in the binding table	*/

#if	C86 | DECUSC | DARWIN
int *getbind(c)
#else
int (*getbind(c))(int f, int n)
#endif

int c;	/* key to find what is bound to it */

{
	register KEYTAB *ktp;

	if (c > 0 && c < NFBIND) return(fkeytab[c]);
	else if (c == (FUNC|'C')) return(ffuncc);
	else if (c == fkeylast.k_code) return(fkeylast.k_fp);

        ktp = &keytab[0];                       /* Look in key table.   */
        while (ktp->k_fp != NULL) {
		if (c == (int) ktp->k_code)	{
			fkeylast.k_code = (short) c;
			fkeylast.k_fp = ktp->k_fp;
                        return(ktp->k_fp);
		}
                ++ktp;
        }

	/* no such binding */
	return(NULL);
}

/* getfname:	This function takes a ptr to function and gets the name
		associated with it
*/

static CONSTA char *getfname(func)

int (*func)(int f, int n);	/* ptr to the requested function to bind to */

{
	register NBIND *nptr;	/* pointer into the name binding table */

	/* skim through the table, looking for a match */
	nptr = &names[0];
	while (nptr->n_func != NULL) {
		if (nptr->n_func == func)
			return(nptr->n_name);
		++nptr;
	}
	return(NULL);
}

/* match fname to a function in the names table
	and return any match or NULL if none */

#if	C86 | DECUSC | DARWIN
int *fncmatch(fname)
#else
int (*fncmatch(fname))(int f, int n)
#endif

char *fname;	/* name to attempt to match */

{
	register NBIND *ffp;	/* pointer to entry in name binding table */
	char firstch;

	/* scan through the table, returning any match */
	ffp = &names[0];
	firstch = (char) ((fname[0]) & ~DIFCASE);
	while (ffp->n_func != NULL) {
		if (firstch == (char) (ffp->n_name[0] & ~DIFCASE) &&
		    strcompare(fname, ffp->n_name, NLINE) == 0)
			return(ffp->n_func);
		++ffp;
	}
	return(NULL);
}

/* stock:	String key name TO Command Key		*/

static unsigned int stock(keyname)

char *keyname;		/* name of key to translate to Command key form */

{
	register unsigned int c;	/* key sequence to return */
	int i, code;

	/* parse it up */
	c = 0;

	/* first, the META prefix */
	if (*keyname == 'M' && *(keyname+1) == '-') {
		c = META;
		keyname += 2;
	}

	/* the control-x prefix */
	if (*keyname == '^' && *(keyname+1) == 'X') {
		c |= CTLX;
		keyname += 2;
	}

	/* next the function prefix */
	if (*keyname == 'F' && *(keyname+1) == 'N') {
		c |= SPEC;
		keyname += 2;
	}

	/* control-x as well... (but not with FN) */
	if (*keyname == '^' && *(keyname+1) == 'X' && !(c & (SPEC|CTLX))) {
		c |= CTLX;
		keyname += 2;
	}

	/* a control char? */
	if (*keyname == '^' && *(keyname+1) != 0) {
		c |= CTRL;
		++keyname;
	}

	/* simplify entry of VT200 escape + [ + number + ~ sequences */
        if ((c & SPEC) && *keyname >= '0' && *keyname <= '9' && *(keyname+1)) {
		code = 0;
		for(i = 0; i < 4 && *keyname >= '0' && *keyname <= '9'; i++)
			code = code*10 + *keyname++ - '0';
		code &= 0xFF;
		if (code>=0x00 && code<=0x1F) code = CTRL | (code+'@');
		c |= code;
	}
	else {
		if (*keyname < 32) {
			c |= CTRL;
			*keyname += '@';
		}

		/* make sure we are not lower case (not with function keys)*/
		if (*keyname >= 'a' && *keyname <= 'z' && !(c & SPEC))
			*keyname -= 32;

		/* the final sequence... */
		c |= *keyname;
	}

	return(c);
}

CONSTA char *transbind(skey)	/* string key name to binding name.... */

char *skey;	/* name of keey to get binding for */

{
	CONSTA char *bindname;
#ifdef NO_PROTOTYPE
#if	C86 | DECUSC
	int *getbind(int c);
#else
	int (*getbind(int c))(int f, int n);
#endif
#endif
	bindname = getfname(getbind( (int) stock(skey)));
	if (bindname == NULL)
		bindname = "ERROR";

	return(bindname);
}

VOID fkeyinit()	/* init fast binding lookup table */
{
	register int c;
	register KEYTAB *ktp;

	for(c = 0; c < NFBIND; c++) fkeytab[c] = NULL;
	ffuncc = NULL;

	ktp = &keytab[0];
	while (ktp->k_fp != NULL) {
		if (ktp->k_code > 0 && ktp->k_code < NFBIND)
			fkeytab[ ktp->k_code ] = ktp->k_fp;
		else if (ktp->k_code == (FUNC|'C'))
			ffuncc = ktp->k_fp;
		++ktp;
	}

	fkeylast.k_code = keytab[0].k_code;
	fkeylast.k_fp = keytab[0].k_fp;
}

int chrcompare(a, b)
int a, b;
{
	if (islower(a)) a ^= DIFCASE;
	if (islower(b)) b ^= DIFCASE;
	if (a > b) return(1);
	if (a < b) return(-1);
	return(0);
}

int strcompare(a, b, len)
CONSTA char *a;
CONSTA char *b;
int len;
{
	char ua, ub;
	while (len-- > 0) {
		ua = *a++;
		if (islower(ua)) ua ^= DIFCASE;
		ub = *b++;
		if (islower(ub)) ub ^= DIFCASE;
		if (ua > ub) return(1);
		if (ua < ub) return(-1);
		if (!ua) return(0);
	}
	return(0);
}

