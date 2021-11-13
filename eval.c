/*	EVAL.C:	Expresion evaluation functions for
		MicroEMACS

	written 1986 by Daniel Lawrence				*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include	"evar.h"

#if defined(__linux__) || FREEBSD2 || DARWIN
#include	<unistd.h>
#endif

#if VMS | BSD | USG | (MSDOS & (MSC | TURBO | GNUC | ZORTECH))
#define	HAS_TIME	1
#include <time.h>
#else
#define	HAS_TIME	0
#endif

#if	TERMCAP | VMSVT
NOSHARE extern int isansi;
NOSHARE extern int usedcolor;
NOSHARE extern int termflag;
#endif

static CONST char *ltos PP((int val));
static char *mkupper PP((char *str));
static VOID PASCAL NEAR setwlist PP((CONSTA char *wclist));
static CONST char *gtenv PP((CONSTA char *vname));
static CONST char *gtfun PP((char *fname));
static CONST char *gtusr PP((CONSTA char *vname));
static int gtlbl PP((char *tok));
static int getvarname PP((CONSTA char *prompt, char *var));
static char *PASCAL NEAR getwlist PP((char *buf));
static int sindex PP((char *source, char *pattern));
static int PASCAL NEAR is_num PP((char *st));
static char *itoan PP((int i, int w));
static CONST char *fixnull PP((CONSTA char *s));


#if	DEBUGM
static VOID pad PP((char *s, int len));
#endif

#if	(MSC & MSDOS) | VMS
#else

#ifdef abs
#undef abs
#endif

#define	abs(x)	me_abs(x)

static int me_abs PP((int x));

static int me_abs(x)	/* take the absolute value of an integer */
int x;
{
	return(x < 0 ? -x : x);
}
#endif

static int ernd PP((void));

static int ernd()	/* returns a random integer */
{
	seed = abs(seed * 1721 + 10007);
	return(seed);
}


VOID varinit()		/* initialize the user variable list */

{
	register int i;

	for (i=0; i < MAXVARS; i++)
		uv[i].u_name[0] = 0;
}

#if WINMOD
NOSHARE int wmodchanged = 0;
#endif

#if COLOR
/*	Convert a color name to an integer, returns TRUE if OK */

static int color_to_int PP((CONSTA char *name, int *color));

static int color_to_int(name, color)
CONSTA char *name;
int *color;
{
	int i, len;

	len = strlen(name);

	for(i = 0; i < NCOLORS; i++) {
		if (strcompare(name, cname[i], len) == 0) {
			*color = i;
			curwp->w_flag |= WFCOLR;
#if WINMOD
			wmodchanged = 1;
#endif
#if	TERMCAP | VMSVT
			if (isansi && (termflag&8) == 0) {
				usedcolor = TRUE;
				eolexist = FALSE;
			}
#endif
			return(TRUE);
		}
	}
	return(FALSE);
}
#endif

/*	Get a variable name with completion */

static int getvarname(prompt, var)

CONSTA char *prompt;	/* prompt to user on command line */
char *var;		/* variable name */

{
	int status;

	complete(prompt, NULL, CMP_VARIABLE, var, NVSIZE+1,
			ctoec('\n'), FALSE, &status);
	return(status);
}

#if BCOMPL
/*	comp_variable:	Attempt a completion on a variable name	*/

VOID comp_variable(name, cpos, matchlen)

char *name;	/* command containing the current name to complete */
int *cpos;	/* ptr to position of next character to insert */
int *matchlen;	/* ptr to new position */

{
	register int vnum;

	if (*cpos <= 0)
		return;

	--(*cpos); --(*matchlen);
	if (name[0] == '$') {
		for(vnum = 0; vnum < NEVARS; vnum++)
			if (!comp_test(name+1, NVSIZE, cpos, matchlen,
					envars[vnum], TRUE))
				break;
	} else if (name[0] == '%') {
		for(vnum = 0; vnum < MAXVARS; vnum++)
			if (!comp_test(name+1, NVSIZE, cpos, matchlen,
					uv[vnum].u_name, TRUE))
				break;
	}
	++(*cpos); ++(*matchlen);
	return;
}
#endif

static CONST char *gtfun(fname)	/* evaluate a function */

char *fname;		/* name of function to evaluate */

{
	register int fnum;		/* index to function to eval */
	register int arg;		/* value of some arguments */
	char arg1[NSTRING];		/* value of first argument */
	char arg2[NSTRING];		/* value of second argument */
	char arg3[NSTRING];		/* value of third argument */
	static char FAR result[2 * NSTRING];	/* string result */
#if	ENVFUNC
	char *getenv();
#endif

	arg1[0] = arg2[0] = arg3[0] = '\0';

	/* look the function up in the function table */
	fname[3] = 0;	/* only first 3 chars significant */
	mklower(fname);	/* and let it be upper or lower case */
	for (fnum = 0; fnum < NFUNCS; fnum++)
		if (strcmp(fname, funcs[fnum].f_name) == 0)
			break;

	/* return errorm on a bad reference */
	if (fnum == NFUNCS)
		return(errorm);

	/* if needed, retrieve the first argument */
	if (funcs[fnum].f_type >= MONAMIC) {
		if (macarg(arg1) != TRUE)
			return(errorm);

		/* if needed, retrieve the second argument */
		if (funcs[fnum].f_type >= DYNAMIC) {
			if (macarg(arg2) != TRUE)
				return(errorm);
	
			/* if needed, retrieve the third argument */
			if (funcs[fnum].f_type >= TRINAMIC)
				if (macarg(arg3) != TRUE)
					return(errorm);
		}
	}
		

	/* and now evaluate it! */
	switch (fnum) {
		case UFADD:	return(me_itoa(asc_int(arg1) + asc_int(arg2)));
		case UFSUB:	return(me_itoa(asc_int(arg1) - asc_int(arg2)));
		case UFTIMES:	return(me_itoa(asc_int(arg1) * asc_int(arg2)));
		case UFDIV:	return(me_itoa(asc_int(arg1) / asc_int(arg2)));
		case UFMOD:	return(me_itoa(asc_int(arg1) % asc_int(arg2)));
		case UFNEG:	return(me_itoa(-asc_int(arg1)));
		case UFCAT:	bytecopy(result, arg1, NSTRING-1);
				bytecopy(&result[strlen(result)], arg2,
					NSTRING-1-strlen(result));
				return(result);
		case UFLEFT:	return(bytecopy(result, arg1, asc_int(arg2)));
		case UFRIGHT:	arg = asc_int(arg2);
				if (arg > (int)strlen(arg1)) arg = strlen(arg1);
				return(strcpy(result,
					&arg1[strlen(arg1)-arg]));
		case UFMID:	arg = asc_int(arg2);
				if (arg > (int)strlen(arg1)) arg = strlen(arg1);
				return(bytecopy(result, &arg1[arg-1],
					asc_int(arg3)));
		case UFNOT:	return(ltos(stol(arg1) == FALSE));
		case UFEQUAL:	return(ltos(asc_int(arg1) == asc_int(arg2)));
		case UFLESS:	return(ltos(asc_int(arg1) < asc_int(arg2)));
		case UFGREATER:	return(ltos(asc_int(arg1) > asc_int(arg2)));
		case UFSEQUAL:	return(ltos(strcmp(arg1, arg2) == 0));
		case UFSLESS:	return(ltos(strcmp(arg1, arg2) < 0));
		case UFSGREAT:	return(ltos(strcmp(arg1, arg2) > 0));
		case UFIND:	return(bytecopy(result,
					fixnull(getval(arg1,arg2)),
					NSTRING-1));
		case UFAND:	return(ltos(stol(arg1) && stol(arg2)));
		case UFOR:	return(ltos(stol(arg1) || stol(arg2)));
		case UFLENGTH:	return(me_itoa(strlen(arg1)));
		case UFUPPER:	return(mkupper(arg1));
		case UFLOWER:	return(mklower(arg1));
		case UFTRUTH:	return(ltos(asc_int(arg1) == 42));
		case UFASCII:	return(me_itoa((int)arg1[0]));
		case UFCHR:	result[0] = (char) asc_int(arg1);
				result[1] = '\0';
				return(result);
		case UFGTCMD:	cmdstr(getcmd(), result);
				return(result);
		case UFGTKEY:	result[0] = (char) tgetc();
				result[1] = '\0';
				return(result);
		case UFRND:	return(me_itoa((ernd() % abs(asc_int(arg1))) + 1));
		case UFABS:	return(me_itoa(abs(asc_int(arg1))));
		case UFSINDEX:	return(me_itoa(sindex(arg1, arg2)));
		case UFENV:
#if	ENVFUNC
				return(fixnull((char *)getenv(arg1)));
#else
				return("");
#endif
		case UFBIND:	return(transbind(arg1));
 		case UFBAND:	return(int_asc(asc_int(arg1) & asc_int(arg2)));
 		case UFBOR:	return(int_asc(asc_int(arg1) | asc_int(arg2)));
 		case UFBXOR:	return(int_asc(asc_int(arg1) ^ asc_int(arg2)));
		case UFBNOT:	return(int_asc(~asc_int(arg1)));
		case UFISNUM:	return(ltos(is_num(arg1)));
		case UFEXIST:	return(ltos(fexist(arg1)));
	}

	exit(-11);	/* never should get here */
}

static CONST char *gtusr(vname)	/* look up a user var's value */

CONSTA char *vname;		/* name of user variable to fetch */

{

	register int vnum;	/* ordinal number of user var */

	/* scan the list looking for the user var name */
	for (vnum = 0; vnum < MAXVARS; vnum++)
		if (strncmp(vname, uv[vnum].u_name, NVSIZE-1) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == MAXVARS)
		return(errorm);

	return(uv[vnum].u_value);
}

/*	getwlist:	place in a buffer a list of characters
			considered "in a word"			*/

static char *PASCAL NEAR getwlist(buf)

char *buf;	/* buffer to place list of characters */

{
	register int index;
	register char *sp;

	/* if we are defaulting to a standard word char list... */
	if (wlflag == FALSE)
		return("");

	/* build the string of characters in the return buffer */
	sp = buf;
	for (index = 0; index < 256; index++)
		if (wordlist[index])
			*sp++ = (char) index;
	*sp = 0;
	return(buf);
}

#if	DECEDT
NOSHARE extern int advset;
#endif

static CONST char *gtenv(vname)

CONSTA char *vname;		/* name of environment variable to retrieve */

{
	register int vnum;	/* ordinal number of var refrenced */
#ifdef NO_PROTOTYPE
	char *getkill();
#endif
#if	HAS_TIME
#if	VMS && !defined(vax11c)
	time_t
#else
	long
#endif
		clock;
	static char ndate[9];
	struct tm *t;
#endif
	static char result[2 * NSTRING];	/* string result */

	/* scan the list, looking for the referenced name */
	for (vnum = 0; vnum < NEVARS; vnum++)
		if (strcmp(vname, envars[vnum]) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == NEVARS)
		return(errorm);

	/* otherwise, fetch the appropriate value */
	switch (vnum) {
		case EVFILLCOL:	return(me_itoa(fillcol));
		case EVPAGELEN:	return(me_itoa(term.t_nrow + 1));
		case EVCURCOL:	return(me_itoa(getccol(FALSE)));
		case EVCURLINE: return(me_itoa(getcline()));
		case EVRAM:	return(me_itoa((int)(envram / 1024L)));
		case EVFLICKER:	return(ltos(flickcode));
		case EVCURWIDTH:return(me_itoa(term.t_nrow));
		case EVCBUFNAME:return(curbp->b_bname);
		case EVCBFLAGS:	return(me_itoa(curbp->b_flag));
		case EVCFNAME:	return(curbp->b_fname);
		case EVSRES:	return(sres);
		case EVDEBUG:	return(ltos(macbug));
		case EVSTATUS:	return(ltos(cmdstatus));
		case EVPALETTE:	return(palstr);
		case EVASAVE:	return(me_itoa(gasave));
		case EVACOUNT:	return(me_itoa(gacount));
		case EVLASTKEY: return(me_itoa(lastkey));
		case EVCURCHAR:
			return(curwp->w_dotp->l_used ==
					curwp->w_doto ? me_itoa('\n') :
				me_itoa(lgetc(curwp->w_dotp, curwp->w_doto)));
		case EVDISCMD:	return(ltos(discmd));
		case EVDOSYNC:  return(ltos(dosync));
		case EVVERSION:	return(VERSION);
		case EVPROGNAME:return(PROGNAME);
		case EVSEED:	return(me_itoa(seed));
		case EVDISINP:	return(ltos(disinp));
		case EVWLINE:	return(me_itoa(curwp->w_ntrows));
		case EVCWLINE:	return(me_itoa(getwpos()));
		case EVTARGET:	saveflag = lastflag;
				return(me_itoa(curgoal));
		case EVSEARCH:	return(pat);
		case EVREPLACE:	return(rpat);
		case EVMATCH:	return((patmatch == NULL)? "": patmatch);
		case EVKILL:	return(getkill());
		case EVCMODE:	return(me_itoa(curbp->b_mode));
		case EVGMODE:	return(me_itoa(gmode));
		case EVTPAUSE:	return(me_itoa(term.t_pause));
		case EVPENDING:
#if	TYPEAH
				return(ltos(typahead()));
#else
				return(falsem);
#endif
		case EVLWIDTH:	return(me_itoa(llength(curwp->w_dotp)));
		case EVLINE:	return(getctext(result, NSTRING));
		case EVHARDTAB:	return(int_asc(tabsize));
		case EVSOFTTAB:	return(int_asc(stabsize));
		case EVFCOL:	return(me_itoa(curwp->w_fcol));
		case EVHSCROLL:	return(ltos(hscroll));
		case EVHJUMP:	return(int_asc(hjump));
		case EVADVANCE:
#if	DECEDT
			return(me_itoa(advset));
#else
			return(me_itoa(1));
#endif
		case EVVT100KEYS: return(me_itoa(vt100keys));
		case EVTIME:
#if	HAS_TIME
			time(&clock);
			return(ctime(&clock));
#else
			return("");
#endif
		case EVNDATE:
#if	HAS_TIME
			time(&clock);
			t = localtime(&clock);
			if (t->tm_year < 70) t->tm_year += 100;
			strcpy(ndate, itoan(t->tm_year+1900, 4));
			strcat(ndate, itoan(t->tm_mon+1, 2));
			strcat(ndate, itoan(t->tm_mday, 2));
			return(ndate);
#else
			return("");
#endif
		case EVDISPHIGH:return(ltos(disphigh));
		case EVWCHARS:	return(getwlist(result));
		case EVPOPFLAG: return(ltos(popflag));
		case EVCPOPFLAG: return(ltos(cpopflag));
		case EVYANKPNT:	return(ltos(yanktype));
		case EVCURWIND: return(int_asc(getcwnum()));
		case EVNUMWIND: return(int_asc(gettwnum()));
#if COLOR
		case EVGFCOLOR: return(cname[gfcolor]);
		case EVGBCOLOR: return(cname[gbcolor]);
		case EVCFCOLOR: return(cname[ (int) curwp->w_fcolor]);
		case EVCBCOLOR: return(cname[ (int) curwp->w_bcolor]);
		case EVBFCOLOR: return(cname[bfcolor]);
		case EVBBCOLOR: return(cname[bbcolor]);
		case EVSFCOLOR: return(cname[sfcolor]);
		case EVSBCOLOR: return(cname[sbcolor]);
#else
		case EVGFCOLOR:
		case EVGBCOLOR:
		case EVCFCOLOR:
		case EVCBCOLOR:
		case EVBFCOLOR:
		case EVBBCOLOR:
		case EVSFCOLOR:
		case EVSBCOLOR: return("");
#endif
#if WINMOD
		case EVWINTITLE: return(wintitle);
#else
		case EVWINTITLE: return("");
#endif
		case EVNUMCOLS:	return(int_asc(term.t_ncol));
		case EVNUMROWS:	return(int_asc(term.t_nrow));
		case EVCINDTYPE: return(int_asc(cindtype));
		case EVPID: return(int_asc((int) getpid()));
#if DISPSEVEN
		case EVDISPSEVEN: return(ltos(dispseven));
#else
		case EVDISPSEVEN: return(ltos(0));
#endif
	}
	return("");	/* again, we should never get here */
}

static CONST char *fixnull(s)	/* Don't return NULL pointers! */

CONSTA char *s;

{
	if (s == NULL)
		return("");
	else
		return(s);
}
char *getkill()		/* return some of the contents of the kill buffer */

{
	register int size;	/* max number of chars to return */
	static char value[NSTRING];	/* temp buffer for value */

	if (kbufh == NULL)
		/* no kill buffer....just a null string */
		value[0] = 0;
	else {
		/* copy in the contents... */
		if (kused < NSTRING)
			size = kused;
		else
			size = NSTRING - 1;
		bytecopy(value, kbufh->d_chunk, size);
	}

	/* and return the constructed value */
	return(value);
}

int setvar(f, n)		/* set a variable */

int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */

{
	register int status;	/* status return */
	VDESC vd;		/* variable num/type */
	char var[NVSIZE+1];	/* name of variable to fetch */
	char value[NSTRING];	/* value to set variable to */

	/* first get the variable to set.. */
	if (clexec == FALSE) {
		status = getvarname("Variable to set: ", var);
		if (status != TRUE)
			return(status);
	} else {	/* macro line argument */
		/* grab token and skip it */
		execstr = token(execstr, var, NVSIZE+1);
	}

	/* check the legality and find the var */
	findvar(var, &vd, NVSIZE+1);
	
	/* if its not legal....bitch */
	if (vd.v_type == -1) {
		mlwrite("%%No such variable as '%s'", var);
		return(FALSE);
	}

	/* get the value for that variable */
	if (f == TRUE)
		strcpy(value, me_itoa(n));
	else {
		status = mlreply("Value: ", &value[0], NSTRING);
		if (status != TRUE)
			return(status);
	}

	/* and set the appropriate value */
	status = svar(&vd, value);

#if	DEBUGM
	/* if $debug == TRUE, every assignment will echo a statment to
	   that effect here. */
	
	if (macbug) {
		strcpy(outline, "(((");

		/* assignment status */
		strcat(outline, ltos(status));
		strcat(outline, ":");

		/* variable name */
		strcat(outline, var);
		strcat(outline, ":");

		/* and lastly the value we tried to assign */
		bytecopy(&outline[strlen(outline)], value, NSTRING-NVSIZE-20);
		strcat(outline, ")))");

		/* expand '%' to "%%" so mlwrite wont bitch */
		makelit(outline, NSTRING);

		/* write out the debug line */
		mlforce(outline);
		update(TRUE);

		/* and get the keystroke to hold the output */
		if (get1key() == abortc) {
			mlforce("[Macro aborted]");
			status = FALSE;
		}
	}
#endif

	/* and return it */
	return(status);
}

VOID findvar(var, vd, size)	/* find a variables type and name */

char *var;	/* name of var to get */
VDESC *vd;	/* structure to hold type and ptr */
int size;	/* size of var array */

{
	register int vnum;	/* subscript in varable arrays */
	register int vtype;	/* type to return */
	char buf[NSTRING];	/* buffer for getval */

	vnum = 0;

fvar:	vtype = -1;
	switch (var[0]) {

		case '$': /* check for legal enviromnent var */
			for (vnum = 0; vnum < NEVARS; vnum++)
				if (!strncmp(&var[1], envars[vnum], NVSIZE-1)) {
					vtype = TKENV;
					break;
				}
			break;

		case '%': /* check for existing legal user variable */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (!strncmp(&var[1],uv[vnum].u_name,NVSIZE-1)){
					vtype = TKVAR;
					break;
				}
			if (vnum < MAXVARS)
				break;

			/* create a new one??? */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (uv[vnum].u_name[0] == 0) {
					vtype = TKVAR;
					bytecopy(uv[vnum].u_name,
							&var[1], NVSIZE);
					break;
				}
			break;

		case '&':	/* indirect operator? */
			var[4] = 0;
			if (strcmp(&var[1], "ind") == 0) {
				/* grab token, and eval it */
				execstr = token(execstr, var, size);
				bytecopy(var, fixnull(getval(var,buf)), size);
				goto fvar;
			}
	}

	/* return the results */
	vd->v_num = vnum;
	vd->v_type = vtype;
	return;
}

int svar(var, value)		/* set a variable */

VDESC *var;		/* variable to set */
CONSTA char *value;	/* value to set to */

{
	register int vnum;	/* ordinal number of var refrenced */
	register int vtype;	/* type of variable to set */
	register int status;	/* status return */
	register char * sp;	/* scratch string pointer */
	int c;			/* translated character */

	/* simplify the vd structure (we are gonna look at it a lot) */
	vnum = var->v_num;
	vtype = var->v_type;

	/* and set the appropriate value */
	status = TRUE;
	switch (vtype) {
	case TKVAR: /* set a user variable */
		sp = malloc(strlen(value) + 1);
		if (sp == NULL)
			return(FALSE);
		strcpy(sp, value);
		if (uv[vnum].u_value != NULL)
			free(uv[vnum].u_value);
		uv[vnum].u_value = sp;
		break;

	case TKENV: /* set an environment variable */
		status = TRUE;	/* by default */
		switch (vnum) {
		case EVFILLCOL:	fillcol = asc_int(value);
				break;
		case EVPAGELEN:	status = newsize(TRUE, asc_int(value));
				break;
		case EVCURCOL:	status = setccol(asc_int(value));
				break;
		case EVCURLINE:	status = gotoline(TRUE, asc_int(value));
				break;
		case EVRAM:	break;
		case EVFLICKER:	flickcode = stol(value);
				break;
		case EVCURWIDTH:status = newwidth(TRUE, asc_int(value));
				break;
		case EVCBFLAGS:	curbp->b_flag = (shrt)
				 ((curbp->b_flag & ~(BFCHG|BFINVS|BFTRUNC)) |
				  (asc_int(value) & (BFCHG|BFINVS|BFTRUNC)));
				upmode();
				break;
		case EVCBUFNAME:bytecopy(curbp->b_bname, value, NBUFN-1);
				upmode();
				break;
		case EVCFNAME:	bytecopy(curbp->b_fname, value, NFILEN-1);
				upmode();
				break;
		case EVSRES:	status = TTrez((char *) value);
				break;
		case EVDEBUG:	macbug = stol(value);
				break;
		case EVSTATUS:	cmdstatus = stol(value);
				break;
		case EVPALETTE:	bytecopy(palstr, value, 48);
				spal(palstr);
				break;
		case EVASAVE:	gasave = asc_int(value);
				break;
		case EVACOUNT:	gacount = asc_int(value);
				break;
		case EVLASTKEY:	lastkey = asc_int(value);
				break;
		case EVCURCHAR:	ldelete(1L, FALSE);	/* delete 1 char */
				c = asc_int(value);
				if (c == '\n')
					lnewline();
				else
					linsert(1, c);
				backchar(FALSE, 1);
				break;
		case EVDISCMD:	discmd = stol(value);
				break;
		case EVDOSYNC:	dosync = stol(value);
				break;
		case EVVERSION:	break;
		case EVPROGNAME:break;
		case EVSEED:	seed = asc_int(value);
				break;
		case EVDISINP:	disinp = stol(value);
				break;
		case EVWLINE:	status = resize(TRUE, asc_int(value));
				break;
		case EVCWLINE:	status = forwline(TRUE,
						asc_int(value) - getwpos());
				break;
		case EVTARGET:	curgoal = asc_int(value);
				thisflag = saveflag;
				break;
		case EVSEARCH:	strcpy(pat, value);
				setjtable();
#if	MAGIC
				mcclear();
#endif
				break;
		case EVREPLACE:	strcpy(rpat, value);
				break;
		case EVMATCH:	break;
		case EVKILL:	break;
		case EVCMODE:	curbp->b_mode = asc_int(value);
				curwp->w_flag |= WFMODE;
				break;
		case EVGMODE:	gmode = asc_int(value);
				break;
		case EVTPAUSE:	term.t_pause = asc_int(value);
				break;
		case EVPENDING:	break;
		case EVLWIDTH:	break;
		case EVLINE:	putctext(value); break;
		case EVHARDTAB:	tabsize = asc_int(value);
				if (tabsize <= 0) tabsize = 1;
				upwind();
				break;
		case EVSOFTTAB:	stabsize = asc_int(value);
				upwind();
				break;
		case EVFCOL:	curwp->w_fcol = asc_int(value);
				if (curwp->w_fcol < 0)
					curwp->w_fcol = 0;
				curwp->w_flag |= WFHARD | WFMODE;
				break;
		case EVHSCROLL:	hscroll = stol(value);
				lbound = 0;
				break;
		case EVHJUMP:	hjump = asc_int(value);
				if (hjump < 1)
					hjump = 1;
				if (hjump > term.t_ncol - 1)
					hjump = term.t_ncol - 1;
				break;
		case EVADVANCE:
#if	DECEDT
				advset = asc_int(value);
#endif
				break;
		case EVVT100KEYS: vt100keys = asc_int(value); break;
		case EVTIME: break;
 		case EVDISPHIGH:
				c = disphigh;
				disphigh = stol(value);
				if (c != disphigh)
					upwind();
				break;
		case EVWCHARS:	setwlist(value);
				break;
		case EVPOPFLAG: popflag = stol(value);
				break;
		case EVCPOPFLAG: cpopflag = stol(value);
				break;
 		case EVYANKPNT:	yanktype = stol(value);
				break;
		case EVCURWIND:	(VOID) nextwind(TRUE, asc_int(value));
				break;
		case EVNUMWIND:	break;
#if COLOR
		case EVGFCOLOR: color_to_int(value, &gfcolor); break;
		case EVGBCOLOR: color_to_int(value, &gbcolor); break;
		case EVCFCOLOR: c = curwp->w_fcolor;
				color_to_int(value, &c);
				curwp->w_fcolor = (shrt) c;
				break;
		case EVCBCOLOR: c = curwp->w_bcolor;
				color_to_int(value, &c);
				curwp->w_bcolor = (shrt) c;
				break;
		case EVBFCOLOR: color_to_int(value, &bfcolor); break;
		case EVBBCOLOR: color_to_int(value, &bbcolor); break;
		case EVSFCOLOR: color_to_int(value, &sfcolor); break;
		case EVSBCOLOR: color_to_int(value, &sbcolor); break;
#else
		case EVGFCOLOR:
		case EVGBCOLOR:
		case EVCFCOLOR:
		case EVCBCOLOR:
		case EVBFCOLOR:
		case EVBBCOLOR:
		case EVSFCOLOR:
		case EVSBCOLOR: break;
#endif
#if WINMOD
		case EVWINTITLE: strncpy(wintitle, value, NSTRING);
				wintitle[NSTRING-1] = '\0';
				wmodchanged = 1;
				break;
#else
		case EVWINTITLE: break;
#endif
		case EVNUMCOLS:	break;
		case EVNUMROWS:	break;
		case EVCINDTYPE: cindtype = asc_int(value); break;
		case EVPID: break;
 		case EVDISPSEVEN:
#if DISPSEVEN
				c = dispseven;
				dispseven = stol(value);
				if (c != dispseven) upwind();
#endif
				break;
		}
		break;
	}
	return(status);
}

/* atoi and itoa defined to asc_int and int_asc in estruct.h */

/*	asc_int:	ascii string to integer......This is too
		inconsistant to use the system's	*/

int asc_int(st)

CONSTA char *st;

{
	int result;	/* resulting number */
	int sign;	/* sign of resulting number */
	char c;		/* current char being examined */

	result = 0;
	sign = 1;

	/* skip preceding whitespace */
	while (*st == ' ' || *st == '\t')
		++st;

	/* check for sign */
	if (*st == '-') {
		sign = -1;
		++st;
	}
	if (*st == '+')
		++st;

	/* scan digits, build value */
	while ((c = *st++) != '\0')
		if (c >= '0' && c <= '9')
			result = result * 10 + c - '0';
		else
			break;

	return(result * sign);
}

/*	int_asc:	integer to ascii string.......... This is too
			inconsistant to use the system's	*/

static char *itoan(i, w)

int i;	/* integer to translate to a string */
int w;	/* num width */

{
	register int digit;		/* current digit being used */
	register char *sp;		/* pointer into result */
	register int sign;		/* sign of resulting number */
	static char result[INTWIDTH+1];	/* resulting string */

	/* record the sign...*/
	sign = 1;
	if (i < 0) {
		sign = -1;
		i = -i;
	}

	/* and build the string (backwards!) */
	sp = result + INTWIDTH;
	*sp = 0;
	do {
		digit = i % 10;
		*(--sp) = (char) ('0' + digit);	/* and install the new digit */
		i = i / 10;
		w--;
	} while (i);

	/* padd */
	while (w-- > 0)
		*(--sp) = '0';

	/* and fix the sign */
	if (sign == -1) {
		*(--sp) = '-';	/* and install the minus sign */
	}

	return(sp);
}

char *me_itoa(i)
int i;
{
	return(itoan(i, 0));
}

int gettyp(tok)	/* find the type of a passed token */

char *tok;	/* token to analyze */

{
	register char c;	/* first char in token */

	/* grab the first char (this is all we need) */
	c = *tok;

	/* no blanks!!! */
	if (c == 0)
		return(TKNUL);

	/* a numeric literal? */
	if (c >= '0' && c <= '9')
		return(TKLIT);

	switch (c) {
		case '"':	return(TKSTR);

		case '!':	return(TKDIR);
		case '@':	return(TKARG);
		case '#':	return(TKBUF);
		case '$':	return(TKENV);
		case '%':	return(TKVAR);
		case '&':	return(TKFUN);
		case '*':	return(TKLBL);

		default:	return(TKCMD);
	}
}

CONSTA char *getval(tok,buf)	/* find the value of a token */

char *tok;		/* token to evaluate */
char buf[NSTRING];	/* string buffer for some returns */

{
	register int status;	/* error return */
	register BUFFER *bp;	/* temp buffer pointer */
	register int blen;	/* length of buffer argument */
	register int distmp;	/* temporary discmd flag */
	char tmpbuf[NSTRING];	/* buffer for recursive calls */

	switch (gettyp(tok)) {
		case TKNUL:	return("");

		case TKARG:	/* interactive argument */
				strcpy(tok, getval(&tok[1],tmpbuf));
				distmp = discmd;	/* echo it always! */
				discmd = TRUE;
				status = getstring(tok,
					   buf, NSTRING, ctoec('\n'));
				discmd = distmp;
				if (status == ABORT)
					return(errorm);
				return(buf);

		case TKBUF:	/* buffer contents fetch */

				/* grab the right buffer */
				strcpy(tok, getval(&tok[1],tmpbuf));
				bp = bfind(tok, FALSE, 0);
				if (bp == NULL)
					return(errorm);
		
				/* if the buffer is displayed, get the window
				   vars instead of the buffer vars */
				if (bp->b_nwnd > 0) {
					curbp->b_dotp = curwp->w_dotp;
					curbp->b_doto = curwp->w_doto;
				}

				/* make sure we are not at the end */
				if (bp->b_linep == bp->b_dotp)
					return(errorm);
		
				/* grab the line as an argument */
				blen = bp->b_dotp->l_used - bp->b_doto;
				if (blen >= NSTRING)
					blen = NSTRING-1;
				bytecopy(buf, bp->b_dotp->l_text + bp->b_doto,
					blen);
				buf[blen] = 0;
		
				/* and step the buffer's line ptr ahead a line */
				bp->b_dotp = bp->b_dotp->l_fp;
				bp->b_doto = 0;

				/* if displayed buffer, reset window ptr vars*/
				if (bp->b_nwnd > 0) {
					curwp->w_dotp = curbp->b_dotp;
					curwp->w_doto = 0;
					curwp->w_flag |= WFMOVE;
				}

				/* and return the spoils */
				return(buf);		

		case TKVAR:	return(gtusr(tok+1));
		case TKENV:	return(gtenv(tok+1));
		case TKFUN:	return(gtfun(tok+1));
		case TKDIR:	return(errorm);
		case TKLBL:	return(me_itoa(gtlbl(tok)));
		case TKLIT:	return(tok);
		case TKSTR:	return(tok+1);
		case TKCMD:	return(tok);
	}
	return "";
}

static int gtlbl(tok)	/* find the line number of the given label */

char *tok;	/* label name to find */

{
	return(1);
}

int stol(val)	/* convert a string to a numeric logical */

CONSTA char *val;	/* value to check for stol */

{
	/* check for logical values */
	if (val[0] == 'F' || val[0] == 'f' || val[0] == 'N' || val[0] == 'n')
		return(FALSE);
	if (val[0] == 'T' || val[0] == 't' || val[0] == 'Y' || val[0] == 'y')
		return(TRUE);

	/* check for numeric truth (!= 0) */
	return((asc_int(val) != 0));
}

static CONST char *ltos(val)	/* numeric logical to string logical */

int val;	/* value to translate */

{
	if (val)
		return(truem);
	else
		return(falsem);
}

static char *mkupper(str)	/* make a string upper case */

char *str;		/* string to upper case */

{
	char *sp;

	sp = str;
	while (*sp) {
		if ('a' <= *sp && *sp <= 'z')
			*sp += 'A' - 'a';
		++sp;
	}
	return(str);
}

char *mklower(str)	/* make a string lower case */

char *str;		/* string to lower case */

{
	char *sp;

	sp = str;
	while (*sp) {
		if ('A' <= *sp && *sp <= 'Z')
			*sp += 'a' - 'A';
		++sp;
	}
	return(str);
}

static int sindex(source, pattern)	/* find pattern within source */

char *source;	/* source string to search */
char *pattern;	/* string to look for */

{
	char *sp;	/* ptr to current position to scan */
	char *csp;	/* ptr to source string during comparison */
	char *cp;	/* ptr to place to check for equality */

	/* scanning through the source string */
	sp = source;
	while (*sp) {
		/* scan through the pattern */
		cp = pattern;
		csp = sp;
		while (*cp) {
			if (!me_eq(*cp, *csp))
				break;
			++cp;
			++csp;
		}

		/* was it a match? */
		if (*cp == 0)
			return((int)(sp - source) + 1);
		++sp;
	}

	/* no match at all.. */
	return(0);
}

static VOID PASCAL NEAR setwlist(wclist)

CONSTA char *wclist;	/* list of characters to consider "in a word" */

{
	register int index;

	/* if we are turning this facility off, just flag so */
	if (wclist == NULL || *wclist == 0) {
		wlflag = FALSE;
		return;
	}

	/* first clear the table */
	for (index = 0; index < 256; index++)
		wordlist[index] = FALSE;

	/* and for each character in the new value, set that element
	   of the word character list */
	while (*wclist)
		wordlist[ (*wclist++) & 0xFF ] = TRUE;
	wlflag = TRUE;
	return;
}

/*	is_num:	ascii string is integer......This is too
		inconsistant to use the system's	*/
 
static int PASCAL NEAR is_num(st)
 
char *st;
 
{
	int period_flag;	/* have we seen a period yet? */

	/* skip preceding whitespace */
	while (*st == ' ' || *st == '\t')
		++st;
 
	/* check for sign */
	if ((*st == '-') || (*st == '+'))
		++st;
 
	/* scan digits */
	period_flag = FALSE;
	while ((*st >= '0' && *st <= '9') ||
	       (*st == '.' && period_flag == FALSE)) {
		if (*st == '.')
			period_flag = TRUE;
		st++;
	}
 
	/* scan rest of line for just white space */
	while (*st) {
		if ((*st != '\t') && (*st != ' '))
			return(FALSE);
		st++;
	}
	return(TRUE);
}

#if	DEBUGM
int dispvar(f, n)		/* display a variable's value */

int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */

{
	register int status;	/* status return */
	VDESC vd;		/* variable num/type */
	char var[NVSIZE+1];	/* name of variable to fetch */
	char buf[NSTRING];	/* buffer for getval */

	/* first get the variable to display.. */
	if (clexec == FALSE) {
		status = getvarname("Variable to display: ", var);
		if (status != TRUE)
			return(status);
	} else {	/* macro line argument */
		/* grab token and skip it */
		execstr = token(execstr, var, NVSIZE + 1);
	}

	/* check the legality and find the var */
	findvar(var, &vd, NVSIZE + 1);
	
	/* if its not legal....bitch */
	if (vd.v_type == -1) {
		mlwrite("%%No such variable as '%s'", var);
		return(FALSE);
	}

	/* and display the value */
	strcpy(outline, var);
	strcat(outline, " = ");

	/* and lastly the current value */
	strcat(outline, fixnull(getval(var,buf)));

	/* expand '%' to "%%" so mlwrite wont bitch */
	makelit(outline, NSTRING);

	/* write out the result */
	mlforce(outline);
	update(TRUE);

	/* and return */
	return(TRUE);
}

/*	describe-variables	Bring up a fake buffer and list the contents
				of all the environment variables
*/

int desvars(f, n)
{
	return(varlist(""));
}

int varlist(mstring)
CONSTA char *mstring;
{
	register BUFFER *varbuf; /* buffer to put binding list into */
	register int uindex;	/* index into uvar table */
	char outseq[NSTRING+NVSIZE+20];	/* output buffer for keystroke sequence */
	int mlen, len;

	mlen = strlen(mstring);

	/* and get a buffer for it */
	varbuf = bfind("Variable list", TRUE, 0);
	if (varbuf == NULL || bclear(varbuf) == FALSE) {
		mlwrite("Can not display variable list");
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite("[Building variable list]");

	/* build the environment variable list */
	for (uindex = 0; uindex < NEVARS; uindex++) {

		/* add in the environment variable name */
		strcpy(outseq, "$");
		strcat(outseq, envars[uindex]);

		if (mlen > 0) {
			len = strlen(outseq);
			if (len > mlen) len = mlen;
			if (strncmp(mstring, outseq, len) != 0)
				continue;
		}

		pad(outseq, 14);

		/* keep length in range */
		outseq[NVSIZE+20-1] = '\0';

		/* add in the value */
		bytecopy(&outseq[strlen(outseq)],
				gtenv(envars[uindex]), NSTRING-1);

		/* and add it as a line into the buffer */
		if (addline(varbuf, outseq) != TRUE)
			return(FALSE);
	}

	if (addline(varbuf, "") != TRUE)
		return(FALSE);

	/* build the user variable list */
	for (uindex = 0; uindex < MAXVARS; uindex++) {
		if (uv[uindex].u_name[0] == 0)
			break;

		/* add in the user variable name */
		strcpy(outseq, "%");
		strcat(outseq, uv[uindex].u_name);

		if (mlen > 0) {
			len = strlen(outseq);
			if (len > mlen) len = mlen;
			if (strncmp(mstring, outseq, len) != 0)
				continue;
		}

		pad(outseq, 14);
		
		/* keep length in range */
		outseq[NVSIZE+20-1] = '\0';

		/* add in the value */
		bytecopy(&outseq[strlen(outseq)],
				uv[uindex].u_value, NSTRING-1);
		strcat(outseq, "\n");

		/* and add it as a line into the buffer */
		if (addline(varbuf, outseq) != TRUE)
			return(FALSE);
	}

	/* display the list */
	wpopup(varbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

/*	describe-functions	Bring up a fake buffer and list the
				names of all the functions
*/

int desfunc(f, n)
int f, n;
{
	register BUFFER *fncbuf; /* buffer to put binding list into */
	register int uindex;	/* index into funcs table */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* get a buffer for it */
	fncbuf = bfind("Function list", TRUE, 0);
	if (fncbuf == NULL || bclear(fncbuf) == FALSE) {
		mlwrite("Can not display function list");
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite("[Building function list]");

	/* build the function list */
	for (uindex = 0; uindex < NFUNCS; uindex++) {

		/* add in the environment variable name */
		strcpy(outseq, "&");
		strcat(outseq, funcs[uindex].f_name);

		/* and add it as a line into the buffer */
		if (addline(fncbuf, outseq) != TRUE)
			return(FALSE);
	}

	if (addline(fncbuf, "") != TRUE)
		return(FALSE);

	/* display the list */
	wpopup(fncbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

static VOID pad(s, len)	/* pad a string to indicated length */

char *s;	/* string to add spaces to */
int len;	/* wanted length of string */

{
	while (((int) strlen(s)) < len) {
		strcat(s, "          ");
		s[len] = '\0';
	}
}
#endif

