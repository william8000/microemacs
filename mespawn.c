/*	Spawn:	various DOS access commands
		for MicroEMACS
*/

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if VMS | BSD | USG | (MSDOS & (MSC | TURBO | GNUC | ZORTECH))
#define	HAS_TIME	1
#include <time.h>
#else
#define	HAS_TIME	0
#endif

/*
**  The following variable is set true while we spawn a sub-process.
**  It is used by the SCS window routine interface to distinguish
**  between the close needed to spawn, and the close when we are
**  really done.
*/
int spawning = FALSE;


#if     AMIGA
#define  NEW   1006L
#endif

#if		ST520 & MEGAMAX
#include <osbind.h>
#include <string.h>
#define LOAD_EXEC 0 	/* load and execute the program */
char	*STcmd,		/* the command filename & path  */
	*STargs,	/* command args (if any)        */
	*STenv,		/* environment                  */
	*STwork;	/* work area			*/
#endif

#if     VMS
#define EFN     0                               /* Event flag.          */

#include        <ssdef.h>                       /* Random headers.      */
#include        <stsdef.h>
#include        <descrip.h>
#include        <iodef.h>

NOSHARE extern  int     oldmode[3];		/* In "termio.c"        */
NOSHARE extern  int     newmode[3];		/* In "termio.c"        */
NOSHARE extern  short   iochan;			/* In "termio.c"        */
#endif

#if	MSDOS & (MSC | TURBO)
#include	<process.h>
#endif

#if	FREEBSD2
#include	<unistd.h>
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int spawncli(f, n)
int f, n;
{
#if     AMIGA
        long newcli;

#endif

#if     V7 | USG | BSD
        register char *cp;
        char    *getenv();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
        mlwrite("[Starting new CLI]");
        sgarbf = TRUE;
        Execute("NEWCLI \"CON:0/0/640/200/MicroEMACS Subprocess\"", 0L, 0L);
        return(TRUE);
#endif

#if     VMS
#if	SCROLL
	ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif
	movecursor(term.t_nrow, 0);             /* In last line.        */
	mlputs("[Starting DCL]\r\n");
	TTflush(); 	                     /* Ignore "ttcol".      */
	sgarbf = TRUE;
	return (sys(NULL, NULL, NULL));		/* NULL => DCL.         */
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
#endif
#if	ST520
	mlwrite("Not in TOS");
#endif
#if     MSDOS & (AZTEC | MSC | TURBO | C86 | GNUC | ZORTECH)
#ifdef	BCCEASYWIN
	mlwrite("Not in EasyWin");
	return(FALSE);
#else
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
	system("command.com");
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
#endif
#if     MSDOS & LATTICE
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
        sys("\\command.com", "");               /* Run CLI.             */
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
#if     V7 | USG | BSD
#if	SCROLL
	ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
        spawning = TRUE;
        TTclose();                              /* stty to old settings */
        if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
                system(cp);
        else
#if	BSD
                system("exec /bin/csh");
#else
                system("exec /bin/sh");
#endif
        sgarbf = TRUE;
#if defined(__MINGW32__)
        /* sleep not present */
#elif HAS_TIME && (_POSIX_C_SOURCE >= 199309L)
	{
		struct timespec request, remaining;
		request.tv_sec = 0;
		request.tv_nsec = 250000000; /* 1/4 sec */
		nanosleep(&request, &remaining);
	}
#else
        sleep(2);
#endif
        TTopen();
        spawning = FALSE;
        return(TRUE);
#endif
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawncmd(f, n)
int f, n;
{
        register int    s;
        char            line[NLINE];

#if	ST520 & MEGAMAX
	int i,j,k;
	char *sptr,*tptr;
#endif

#if     AMIGA
        long newcli;
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
        Execute(line, 0L, newcli);
        Close(newcli);
        tgetc();     /* Pause.               */
        sgarbf = TRUE;
        return(TRUE);
#endif
#if     ST520 & MEGAMAX
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
        spawning = TRUE;
	TTclose();
/*
 * break the line into the command and its args
 * be cute about it, if there is no '.' in the filename, try
 * to find .prg, .tos or .ttp in that order
 * in any case check to see that the file exists before we run 
 * amok
 */
	STenv = NULL;
	if((tptr = index(&line[0],' ')) == NULL) { /* no args */
		STcmd = (char *)malloc(strlen(line) + 1);
		strcpy(STcmd,line);
		STargs = NULL;
	}
	else {  /* seperate out the args from the command */
		/* resist the temptation to do ptr arithmetic */
		STcmd = (char *)malloc(strlen(line) + 1);
		for(i = 0,sptr = &line[0]; sptr != tptr; sptr++,i++)
			STcmd[i] = *sptr;
		STcmd[i] = '\0';
		for(; *tptr == ' ' || *tptr == '\t'; tptr++);
		if(*tptr == '\0')
			STargs = NULL;
		else {
			STargs = (char *)malloc(strlen(tptr) + 2);
/* first byte of STargs is the length of the string */
			STargs[0] = strlen(tptr);
			STargs[1] = NULL; /* fake it for strcat */
			strcat(STargs,tptr);
		}
	}
/*
 * before we issue the command look for the '.', if it's not there
 * try adding .prg, .tos and .ttp to see if they exist, if not
 * issue the command as is
 */
	if((tptr = index(STcmd,'.')) == NULL) {
 		STwork = (char *)malloc(strlen(STcmd) + 4);
 		strcpy(STwork,STcmd);
 		strcat(STwork,".prg");
 		tptr = index(STwork,'.');
 		if(Fsfirst(1,STwork) != 0) { /* try .tos */
 			strcpy(tptr,".tos");
 			if(Fsfirst(1,STwork) != 0) { /* try .ttp */
 				strcpy(tptr,".ttp");
 				if(Fsfirst(1,STwork) != 0) /* never mind */
 					*STwork = NULL;
 				}
 			}
 	}
 	if(*STwork != NULL)
	        Pexec(LOAD_EXEC,STwork,STargs,STenv); 		
	else
	        Pexec(LOAD_EXEC,STcmd,STargs,STenv);
	TTopen();
        spawning = FALSE;
        mlputs("\r\n\n[End]");                  /* Pause.               */
        TTgetc();			     /* Pause.               */
        sgarbf = TRUE;
        return (TRUE);
#endif
#if     VMS
#if	SCROLL
        ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        s = sys(line, NULL, NULL);		/* Run the command.     */
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
		mlputs("\r\n\n[End]");		/* Pause.               */
		TTflush();
		tgetc();
        }
        sgarbf = TRUE;
        return (s);
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
        return (FALSE);
#endif
#if     MSDOS | (ST520 & LATTICE)
#ifdef	BCCEASYWIN
	mlwrite("Not in EasyWin");
	return(FALSE);
#else
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs("\r\n\n[End]");
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
#endif
#endif
#if     V7 | USG | BSD
#if	SCROLL
	ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        spawning = TRUE;
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        spawning = FALSE;
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
		mlputs("[End]");
		TTflush();
		while ((s = tgetc()) != '\r' && s != ' ')
			;
	}
	if (f != TRUE || clexec == FALSE) sgarbf = TRUE;
	return (TRUE);
#endif
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int pipecmd(f, n)
int f, n;
{
        register int    s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char	line[NLINE];	/* command line send to shell */

#if	AMIGA
	static char filnam[] = "ram:command";
        long newcli;
#endif
#if	VMS
	static char filnam[] = "command.log";
#endif
#if	MSDOS
	char filnam[NSTRING];
#endif
#if	DECUSC
	char *filnam[];
#endif
#if	(AMIGA | VMS | MSDOS | DECUSC)
#else
	static char filnam[] = "command";
#endif

#if	MSDOS
	char *tmp;
	char *getenv();
	FILE *fp;
	FILE *fopen();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	MSDOS
	if ((tmp = getenv("TMP")) == NULL)
		strcpy(filnam, "command");
	else if (tmp[strlen(tmp)-1] == ':' || tmp[strlen(tmp)-1] == '\\')
		{strcpy(filnam, tmp); strcat(filnam, "command");}
	else {
		strcpy(filnam, tmp);
                strcat(filnam,"\\command");
        }
#endif

#if     CPM | DECUSC
        mlwrite("Not availible under CP/M-86");
        return(FALSE);
#endif

#if     SCROLL
        ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif

	/* get the command to pipe in */
        if ((s=mlreply("@", line, NLINE)) != TRUE)
                return(s);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind("command", FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

#if     AMIGA
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " >");
	strcat(line, filnam);
        Execute(line, 0L, newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
#ifdef	BCCEASYWIN
	mlwrite("Not in EasyWin");
	return(FALSE);
#else
	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	if ((fp = fopen(filnam, "r")) == NULL) {
		s = FALSE;
	} else {
		fclose(fp);
		s = TRUE;
	}
#endif
#endif
#if     V7 | USG | BSD
	if (n == 1)	TTputc('\n');		/* Already have '\r'    */
	TTflush();
        spawning = TRUE;
	if (n == 1)	TTclose();		/* stty to old modes    */
	strcat(line,">");
	strcat(line,filnam);
        system(line);
        if (n == 1) {
		TTopen();
		TTflush();
		sgarbf = TRUE;
	}
        spawning = FALSE;
        s = TRUE;
#endif
#if	VMS
	if (n == 1)
		TTputc('\n');                /* Already have '\r'    */
	TTflush();
	s = sys(line, NULL, filnam);	/* Run the command */
        TTflush();
        if (n == 1)
		sgarbf = TRUE;
#endif

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);
	curwp = spltwp; curbp = curwp->w_bufp;

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curbp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
static int filtcom PP((int f, int n, int same_buffer, CONSTA char *prompt));

static int filtcom(f, n, same_buffer, prompt)
int f, n, same_buffer;
CONSTA char *prompt;
{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
	register WINDOW *wp;	/* pointer to new window */
        char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */

#if	AMIGA
	static char filnam1[] = "ram:fltinp";
	static char filnam2[] = "ram:fltout";
        long newcli;
#else
#if	VMS
	static char filnam1[] = "fltinp.com";
	static char filnam2[] = "fltout.log";
#else
#if	DECUSC
	char *filnam1, *filnam2;
#else
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";
#endif
#endif
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	/* don't allow this command if we are in read only mode */
	if (same_buffer && (curbp->b_mode&MDVIEW) != 0)
		return(rdonly());

#if     CPM | DECUSC
        mlwrite("Not available under CP/M-86");
        return(FALSE);
#endif

#if     SCROLL
        ttscroll(0, term.t_nrow, 0);		/* undo scrolling region */
#endif

	/* get the filter name and its args */
        if ((s=mlreply(prompt, line, NLINE)) != TRUE)
                return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, "fltinp");	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1,"w") != TRUE) {
		mlwrite("[Cannot write filter file]");
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

	strcpy(bp->b_fname, tmpnam);	/* restore name */

#if     AMIGA
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " <ram:fltinp >ram:fltout");
        Execute(line,0L,newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
#ifdef	BCCEASYWIN
	mlwrite("Not in EasyWin");
	return(FALSE);
#else
	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	s = TRUE;
#endif
#endif
#if     V7 | USG | BSD
	if (n == 1)	TTputc('\n');		/* Already have '\r'    */
        TTflush();
        spawning = TRUE;
        if (n == 1)	TTclose();		/* stty to old modes    */
	strcat(line," <fltinp >fltout");
        system(line);
	if (n == 1) {
		TTopen();
		TTflush();
		sgarbf = TRUE;
	}
        spawning = FALSE;
        s = TRUE;
#endif
#if	VMS
	if (n == 1)	TTputc('\n');		/* Already have '\r'    */
        TTflush();
	s = sys(line, filnam1, filnam2);
        TTflush();
	if (n == 1)	sgarbf = TRUE;
        s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE) {
		mlwrite("[Execution failed]");

	} else if (same_buffer) {
		if ((s = readin(filnam2,FALSE,FALSE)) == FALSE) {
			mlwrite("[Failed reading execution results]");
		}
		bp->b_flag |= BFCHG;		/* flag it as changed */
		strcpy(bp->b_fname, tmpnam);	/* restore name after readin */

	} else {
		/* get rid of the command output buffer if it exists */
	        if ((bp=bfind("fltout", FALSE, 0)) != FALSE) {
			/* try to make sure we are off screen */
			wp = wheadp;
			while (wp != NULL) {
				if (wp->w_bufp == bp) {
					onlywind(FALSE, 1);
					break;
				}
				wp = wp->w_wndp;
			}
			if ((s = zotbuf(bp)) != TRUE) goto done;
		}

		/* split the current window to make room for the command output */
		if ((s = splitwind(FALSE, 1)) == FALSE)
				goto done;
		curwp = spltwp; curbp = curwp->w_bufp;

		/* and read the stuff in */
		if ((s = getfile(filnam2, FALSE)) == FALSE)
			goto done;

		/* make this window in VIEW mode, update all mode lines */
		curbp->b_mode |= MDVIEW;
		wp = wheadp;
		while (wp != NULL) {
			wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}

done:

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(s);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter(f, n)
int f, n;
{
	return( filtcom(f, n, TRUE, "#") );
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X %
 */
int filtnew(f, n)
int f, n;
{
	return( filtcom(f, n, FALSE, "%") );
}

#if     VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
int sys(cmd, inname, outname)
register char   *cmd, *inname, *outname;
{
        struct  dsc$descriptor  cdsc, idsc, odsc;
        struct  dsc$descriptor  *cdscp, *idscp, *odscp;
        long    status;
        long    substatus;
        long    iosb[2];

        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                return (FALSE);
        cdscp = NULL;                           /* Assume DCL.          */
        if (cmd != NULL) {                      /* Build descriptor.    */
                cdsc.dsc$a_pointer = cmd;
                cdsc.dsc$w_length  = strlen(cmd);
                cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
                cdsc.dsc$b_class   = DSC$K_CLASS_S;
                cdscp = &cdsc;
        }
        idscp = NULL;                           /* Assume SYS$INPUT:    */
        if (inname != NULL) {			/* Build descriptor.    */
                idsc.dsc$a_pointer = inname;
                idsc.dsc$w_length  = strlen(inname);
                idsc.dsc$b_dtype   = DSC$K_DTYPE_T;
                idsc.dsc$b_class   = DSC$K_CLASS_S;
                idscp = &idsc;
        }
        odscp = NULL;				/* Assume SYS$OUTPUT:   */
        if (outname != NULL) {			/* Build descriptor.    */
                odsc.dsc$a_pointer = outname;
                odsc.dsc$w_length  = strlen(outname);
                odsc.dsc$b_dtype   = DSC$K_DTYPE_T;
                odsc.dsc$b_class   = DSC$K_CLASS_S;
                odscp = &odsc;
        }
        status = LIB$SPAWN(cdscp, idscp, odscp, 0, 0, 0, &substatus, 0, 0, 0);
        if (status != SS$_NORMAL)
                substatus = status;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                return (FALSE);
        if ((substatus&STS$M_SUCCESS) == 0)     /* Command failed.      */
                return (FALSE);
        return (TRUE);
}
#endif

#if	MSDOS & (MWC86 | LATTICE)

/*
 * This routine, once again by Bob McNamara, is a C translation of the "system"
 * routine in the MWC-86 run time library. It differs from the "system" routine
 * in that it does not unconditionally append the string ".exe" to the end of
 * the command name. We needed to do this because we want to be able to spawn
 * off "command.com". We really do not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very very strange things.
 */
int sys(cmd, tail)
char    *cmd;
char    *tail;
{
#if MWC86
        register unsigned n;
        NOSHARE extern   char     *__end;

        n = __end + 15;
        n >>= 4;
        n = ((n + dsreg() + 16) & 0xFFF0) + 16;
        return(execall(cmd, tail, n));
#endif

#if LATTICE
        return(forklp(cmd, tail, (char *)NULL));
#endif
}
#endif

#if	MSDOS & LATTICE
/*	System: a modified version of lattice's system() function
		that detects the proper switchar and uses it
		written by Dana Hogget				*/

int system(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *getenv();
	static char *swchar = "/C";	/*  Execution switch  */
	union REGS inregs;	/*  parameters for dos call  */
	union REGS outregs;	/*  Return results from dos call  */
	char *shell;		/*  Name of system command processor  */
	char *p;		/*  Temporary pointer  */
	int ferr;		/*  Error condition if any  */

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL) {
		return (-1);		/*  No shell located  */
	}

	p = cmd;
	while (p && isspace(*p)) {		/*  find out if null command */
		p++;
	}

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (p && *p) {
		/**  detect current switch character and us it  **/
		inregs.h.ah = 0x37;	/*  get setting data  */
		inregs.h.al = 0x00;	/*  get switch character  */
		intdos(&inregs, &outregs);
		*swchar = outregs.h.dl;
		ferr = forkl(shell, "command", swchar, cmd, (char *)NULL);
	} else {
		ferr = forkl(shell, "command", (char *)NULL);
	}

	return (ferr ? ferr : wait());
}
#endif

#if MSDOS & C86
char *getenv(str)
char *str;
{
	return(NULL);
}
#endif

