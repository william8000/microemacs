/*
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 */
#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#undef	CTRL

	/* Use Posix style termios */

#if defined(__linux__) || FREEBSD2 || DARWIN || defined(__INTERIX)
#define	USETERMIOS	1
#else
#define	USETERMIOS	0
#endif

	/* Use SYSV style ioctl() */

#if USETERMIOS || defined(__MINGW32__)
#define	USEIOCTL 0
#else
#if defined(sparc)
#define	USEIOCTL 1
#else
#define	USEIOCTL USG
#endif	/* sparc */
#endif	/* USETERMIOS */

#if   (MSDOS & TURBO) || defined(__MINGW32__)
#include <conio.h>
#endif

#if     AMIGA
#define NEW 1006L
#define AMG_MAXBUF      1024L
static long terminal;
static char     scrn_tmp[AMG_MAXBUF+1];
static long     scrn_tmp_p = 0;
#endif

#if ST520 & MEGAMAX
#include <osbind.h>
	int STscancode = 0;	
#endif

#if     VMS
#include        <stsdef.h>
#include        <ssdef.h>
#include        <descrip.h>
#include        <iodef.h>
#include        <ttdef.h>
#include	<tt2def.h>

#define EFN     0			/* Event flag		*/

NOSHARE char    obuf[NOBUF];		/* Output buffer	*/
NOSHARE int     nobuf;			/* # of bytes in above	*/
NOSHARE char    ibuf[NIBUF];		/* Input buffer		*/
NOSHARE int     nibuf;			/* # of bytes in above	*/
NOSHARE int     ibufi;			/* Read index		*/
NOSHARE int     oldmode[3];		/* Old TTY mode bits	*/
NOSHARE int     newmode[3];		/* New TTY mode bits	*/
NOSHARE short   iochan;			/* TTY I/O channel	*/
#endif

#if     CPM
#include        <bdos.h>
#endif

#if     MSDOS & (LATTICE | MSC | TURBO | AZTEC | MWC86 | C86 | GNUC | ZORTECH)
union REGS rg;		/* cpu register for use of DOS calls */
int nxtchar = -1;	/* character held from type ahead    */
#endif

#if RAINBOW
#include "rainbow.h"
#endif

#if	USETERMIOS		/* Posix termios */
#include	<signal.h>
#if defined(__ANDROID__)
#include	<termios.h>
#else
#include	<sys/termios.h>
#endif
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#if defined(__linux__) && !defined(_SYS_IOCTL_H) && !defined(TIOCGWINSZ)
#include	<sys/ioctl.h>
#endif
static struct	termios	otermio;	/* original terminal characteristics */
static struct	termios	ntermio;	/* charactoristics to use inside */
#endif

#if	USG && !defined(__MINGW32__)	/* System V */
#if	!USETERMIOS
#include	<signal.h>
#include	<termio.h>
#include	<fcntl.h>
static struct	termio	otermio;	/* original terminal characteristics */
static struct	termio	ntermio;	/* charactoristics to use inside */
#endif	/* !USETERMIOS */

#if MSC
/* #include <sgtty.h> */	/* not needed */
#undef FIOCLEX
#undef FIONCLEX
#include <sys/ioctl.h>

static int tafd;		/* extra fd opened with no wait	*/
static int fcstat;		/* saved fcntl status		*/
#define	NIBUF	63
static char	ibuf[NIBUF];	/* Input buffer			*/
static int	nibuf = 0;	/* # of bytes in above		*/
static int	ibufi = 0;	/* Read index			*/
#else
static int use_kbdpoll = 0;	/* TRUE if OK to use		*/
static int kbdflgs = 0;		/* saved keyboard fd flags	*/
static int kbdpoll = 0;		/* in O_NDELAY mode		*/
static int kbdqp = 0;		/* there is a char in kbdq	*/
static char kbdq = 0;		/* char we've already read	*/
#endif
#endif

#if V7 | BSD
#if !USETERMIOS
#include	<signal.h>
#if USEIOCTL
#include	<sys/ioctl.h>
#include	<termio.h>
struct	termio	otermio;	/* original terminal characteristics */
struct	termio	ntermio;	/* charactoristics to use inside */
#else
#include        <sgtty.h>        /* for stty/gtty functions */
#endif
struct  sgttyb  ostate;          /* saved tty state */
struct  sgttyb  nstate;          /* values for editor mode */
struct tchars	otchars;	/* Saved terminal special character set */
struct tchars	ntchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif	/* !USETERMIOS */
#endif	/* BSD | V7 */
				/* A lot of nothing */
#if BSD
#include <sys/ioctl.h>		/* to get at the typeahead */
#if BSD29
#else
#define	TBUFSIZ	128
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif	/* BSD29 */
#endif	/* BSD */

NOSHARE extern int termflag;

#if	VMS

bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	int err, code, mypid, ownerpid;

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	vttidy();

	err = 0;
	mypid = 0;
	code = 771;	/* jpi$_owner */
	if (lib$getjpi(&code, &mypid, 0, &ownerpid, 0, 0) != 1) err = 1;
	else if (ownerpid == 0 || mypid == ownerpid) err = 2;
	else if (lib$attach(&ownerpid) != 1) err = 3;

	TTopen();

	if (err == 0) {
		sgarbf = TRUE;
		curwp->w_flag = WFHARD;
		return(TRUE);
	}

	if (err == 1) mlwrite("[Error getting pid]");
	else if (err == 2) mlwrite("[No parent to attach]");
	else mlwrite("[Attach failed]");
	return(FALSE);
}

#endif

#if	V7 | BSD | USG

#if	ABACKUP

#if USG || DARWIN
#define	BKHANDLER(sig,name,is_fatal) \
	static VOID name(signum) int signum; \
	{ signal(sig, name); filebkall(FALSE, 1); \
	if (is_fatal) { vttidy(); exit(1); } }
#else
#define	BKHANDLER(sig,name,is_fatal) \
	static int name(signum) int signum; \
	{ signal(sig, name); filebkall(FALSE, 1); \
	if (is_fatal) { vttidy(); exit(1); } return TRUE; }
#endif

#ifdef SIGHUP
BKHANDLER(SIGHUP, sighuphandler, TRUE)
#endif
#ifdef SIGQUIT
BKHANDLER(SIGQUIT, sigquithandler, TRUE)
#endif
#ifdef SIGILL
BKHANDLER(SIGILL, sigillhandler, TRUE)
#endif
#ifdef SIGABRT
BKHANDLER(SIGABRT, sigabrthandler, TRUE)
#endif
#ifdef SIGEMT
BKHANDLER(SIGEMT, sigemthandler, TRUE)
#endif
#ifdef SIGFPE
BKHANDLER(SIGFPE, sigfpehandler, TRUE)
#endif
#ifdef SIGBUS
BKHANDLER(SIGBUS, sigbushandler, TRUE)
#endif
#ifdef SIGSEGV
BKHANDLER(SIGSEGV, sigsegvhandler, TRUE)
#endif
#ifdef SIGTERM
BKHANDLER(SIGTERM, sigtermhandler, TRUE)
#endif
#ifdef SIGUSR1
BKHANDLER(SIGUSR1, sigusr1handler, FALSE)
#endif
#ifdef SIGUSR2
BKHANDLER(SIGUSR2, sigusr2handler, FALSE)
#endif

#endif	/* ABACKUP */

#ifdef	SIGTSTP

int bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	vttidy();
	kill(getpid(), SIGTSTP);
	return(TRUE);
}

#if USG
static VOID rtfrmshell(sig)
int sig;
#else
static rtfrmshell()
#endif
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
#if !USG
	return(TRUE);
#endif
}

#else

bktoshell()		/* no suspend, just make a new shell */
{
	return( spawncli(FALSE, 1) );
}

#endif
#endif	/* unix */

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
ttopen()
{
#if     AMIGA
	char oline[NSTRING];
#if	AZTEC
	extern	Enable_Abort;	/* Turn off ctrl-C interrupt */

	Enable_Abort = 0;	/* for the Manx compiler */
#endif
	strcpy(oline, "RAW:0/0/640/200/");
	strcat(oline, PROGNAME);
	strcat(oline, " ");
	strcat(oline, VERSION);
	strcat(oline, "/Amiga");
        terminal = Open(oline, NEW);
#endif
#if     VMS
        struct  dsc$descriptor  idsc;
        struct  dsc$descriptor  odsc;
        char    oname[40];
        int     iosb[2];
        int     status;

        odsc.dsc$a_pointer = "TT";
        odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
        odsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        odsc.dsc$b_class        = DSC$K_CLASS_S;
        idsc.dsc$b_dtype        = DSC$K_DTYPE_T;
        idsc.dsc$b_class        = DSC$K_CLASS_S;
        do {
                idsc.dsc$a_pointer = odsc.dsc$a_pointer;
                idsc.dsc$w_length  = odsc.dsc$w_length;
                odsc.dsc$a_pointer = &oname[0];
                odsc.dsc$w_length  = sizeof(oname);
                status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
                if (status!=SS$_NORMAL && status!=SS$_NOTRAN)
                        exit(status);
                if (oname[0] == 0x1B) {
                        odsc.dsc$a_pointer += 4;
                        odsc.dsc$w_length  -= 4;
                }
        } while (status == SS$_NORMAL);
        status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
        if (status != SS$_NORMAL)
                exit(status);

	/* sense mode returns:
	 *	mode[0] :0-7 = class, :8-15 = type, :16-31 = width
	 *	mode[1] :0-23 = characteristics, :24-31 = length
	 */

        status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        newmode[0] = oldmode[0];
        newmode[1] = oldmode[1] /* | TT$M_NOECHO */ ;
        newmode[2] = oldmode[2];
	if (!(termflag&1)) {
	        newmode[1] &= ~(TT$M_TTSYNC|TT$M_HOSTSYNC);
	}
	newmode[2] |= TT2$M_PASTHRU;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        term.t_nrow = (newmode[1]>>24) - 1;
	if (term.t_nrow > term.t_mrow) term.t_mrow = term.t_nrow;
        term.t_ncol = newmode[0]>>16;
	if (term.t_ncol > term.t_mcol) term.t_mcol = term.t_ncol;
#endif
#if     CPM
#endif

#if     MSDOS & (((HP150 == 0) & LATTICE) | TURBO | GNUC | ZORTECH)
	/* kill the ctrl-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 0;		/* set it OFF */
	intdos(&rg, &rg);	/* go for it! */
#endif

#if	USEIOCTL | USETERMIOS		/* USG, newer BSD, and Posix */
#if	USEIOCTL
	ioctl(0, TCGETA, &otermio);	/* save old settings */
#else
	tcgetattr(0, &otermio);		/* save old settings */
#endif
	memcpy(&ntermio, &otermio, sizeof(otermio));
	ntermio.c_oflag = 0;		/* setup new settings */
	if (termflag&1) {
		ntermio.c_iflag = otermio.c_iflag & (IXON|IXANY|IXOFF);
	}
	else
		ntermio.c_iflag = 0;
	ntermio.c_cflag = otermio.c_cflag;
	ntermio.c_lflag = 0;
#if FREEBSD2 || DARWIN || defined(__INTERIX)
	/* no c_line field */
#else
	ntermio.c_line = otermio.c_line;
#endif
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
#if	USEIOCTL
	ioctl(0, TCSETAW, &ntermio);	/* and activate them */
#ifdef	TCXONC
	/* ioctl(0, TCXONC, 1);	*/	/* restart suspended (^S'ed) output */
#endif
#else
	tcsetattr(0, TCSADRAIN, &ntermio);
	tcflow(0, TCION);
	tcflow(1, TCOON);
#endif
#endif	/* USEIOCTL */

#if	USG && !defined(__MINGW32__)
#if	MSC
	tafd = open("/dev/tty", O_RDONLY, 0);
	ioctl(tafd, TCSETA, &ntermio);	/* and activate them */
	fcstat = fcntl(tafd, F_GETFL, 0);
	fcntl(tafd, F_SETFL, fcstat | O_NDELAY);
#else
	use_kbdpoll = TRUE;
	{
		/* Disable polling for type ahead on xterms */
		/*	xterm creates a pseudo tty with a 4096 byte buffer. */
		/*	Playing with NDELAY on input can cause writes */
		/*	to drop bytes if the output buffer is full. */
		/*	xterms are so fast that disabling type ahead is not a problem. */
		/*	Type ahead detection is more important on slow serial lines. */
		const char *s;
		s = getenv("DISPLAY");
		if (s != NULL && *s != '\0') {
			use_kbdpoll = FALSE;
		}
	}
	kbdflgs = fcntl( 0, F_GETFL, 0 );
	kbdpoll = FALSE;
#endif	/* MSC */
#endif	/* USG */

#if     V7 | BSD
#if	!USETERMIOS
#if	!USEIOCTL
        gtty(0, &ostate);                       /* save old state */
        gtty(0, &nstate);                       /* get base of new state */
        nstate.sg_flags |= RAW;
        nstate.sg_flags &= ~(ECHO|CRMOD);       /* no echo for now... */
        stty(0, &nstate);                       /* set mode */
#endif
	ioctl(0, TIOCGETC, &otchars);		/* Save old characters */
	ioctl(0, TIOCSETC, &ntchars);		/* Place new character into K */
#endif	/* !TERMIOS */
#endif	/* V7 | BSD */

	/* unbuffer output */

#if	BSD
#if	BSD29
#else
	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
#if !SYSTEM_SUN
	setbuffer(stdout, &tobuf[0], TBUFSIZ);
#endif
#ifdef	_IONBF
	setvbuf(stdin, NULL, _IONBF, 0);
#endif	/* _IONBF */
#endif	/* BSD29 */
#endif	/* BSD */

#if	BSD | V7 | USG

	/* enable file saving on interrupts */

#if	ABACKUP
#ifdef SIGHUP
	signal(SIGHUP, sighuphandler);
#endif
#ifdef SIGQUIT
	signal(SIGQUIT, sigquithandler);
#endif
#ifdef SIGILL
	signal(SIGILL, sigillhandler);
#endif
#ifdef SIGABRT
	signal(SIGABRT, sigabrthandler);
#endif
#ifdef SIGEMT
	signal(SIGEMT, sigemthandler);
#endif
#ifdef SIGFPE
	signal(SIGFPE, sigfpehandler);
#endif
#ifdef SIGBUS
	signal(SIGBUS, sigbushandler);
#endif
#ifdef SIGSEGV
	signal(SIGSEGV, sigsegvhandler);
#endif
#ifdef SIGTERM
	signal(SIGTERM, sigtermhandler);
#endif
#ifdef SIGUSR1
	signal(SIGUSR1, sigusr1handler);
#endif
#ifdef SIGUSR2
	signal(SIGUSR2, sigusr2handler);
#endif
#endif	/* ABACKUP */

	/* enable job control */

#ifdef	SIGTSTP
	signal(SIGTSTP,SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT,rtfrmshell);	/* suspend & restart emacs */
#endif	/* SIGTSTP */

#endif	/* UNIX */

	/* on all screens we are not sure of the initial position
	   of the cursor					*/
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
ttclose()
{
#if     AMIGA
#if	LATTICE
        amg_flush();
        Close(terminal);
#endif
#if	AZTEC
        amg_flush();
	Enable_Abort = 1;	/* Fix for Manx */
        Close(terminal);
#endif
#endif

#if     VMS
        int     status;
        int     iosb[2];

        ttflush();
#if	1
#else
	SYS$QIOW(EFN+1, iochan, IO$_WRITELBLK | IO$M_NOFORMAT,
		0, 0, 0, "", 1, 0, 0, 0, 0);
#endif
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                 oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                exit(status);
        status = SYS$DASSGN(iochan);
        if (status != SS$_NORMAL)
                exit(status);
#endif
#if     CPM
#endif
#if     MSDOS & (HP150 == 0) & LATTICE
	/* restore the ctrl-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 1;		/* set it ON */
	intdos(&rg, &rg);	/* go for it! */
#endif

#if	USG && !defined(__MINGW32__)
#if	MSC
	fcntl(tafd, F_SETFL, fcstat);
	ioctl(tafd, TCSETA, &otermio);	/* restore terminal settings */
#else
	fcntl(0, F_SETFL, kbdflgs);
#endif
#endif

#if	USETERMIOS	/* Posix */
	tcsetattr(0, TCSADRAIN, &otermio);	/* restore terminal settings */
#endif

#if	USEIOCTL	/* USG and newer BSD */
	ioctl(0, TCSETA, &otermio);	/* restore terminal settings */
#endif

#if     V7 | BSD
#if	!USETERMIOS
#if	!USEIOCTL
        stty(0, &ostate);
#endif
	ioctl(0, TIOCSETC, &otchars);	/* Place old character into K */
#endif	/* !USETERMIOS */
#endif	/* V7 | BSD */
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)
#if     AMIGA | (ST520 & MEGAMAX)
        char c;
#else
        int c;
#endif
{
#if     AMIGA
        scrn_tmp[scrn_tmp_p++] = c;
        if(scrn_tmp_p>=AMG_MAXBUF)
                amg_flush();
#endif
#if	ST520 & MEGAMAX
	Bconout(2,c);
#endif
#if     VMS
        if (nobuf >= NOBUF)
                ttflush();
        obuf[nobuf++] = c;
#endif

#if     CPM
        bios(BCONOUT, c, 0);
#endif

#if     MSDOS & MWC86
        putcnb(c);
#endif

#if	MSDOS & (LATTICE | AZTEC | MSC | TURBO | GNUC | ZORTECH) /* & ~IBMPC */
	bdos(6, c, 0);
#endif

#if	MSDOS & C86
	bdos(6, (long) c);
#endif

#if RAINBOW
        Put_Char(c);                    /* fast video */
#endif


#if     (V7 | USG | BSD | DECUSC) || (defined(_MSC_VER) && defined(_WIN32))
        putc(c, stdout);
#endif

}

#if	AMIGA
amg_flush()
{
        if(scrn_tmp_p)
                Write(terminal,scrn_tmp,scrn_tmp_p);
        scrn_tmp_p = 0;
}
#endif

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
#if     AMIGA
        amg_flush();
#endif
#if     VMS
        int     status;
        int     iosb[2];

        status = SS$_NORMAL;
        if (nobuf != 0) {
#if 1
                status = SYS$QIOW(EFN, iochan, IO$_WRITELBLK|IO$M_NOFORMAT,
                         iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0);
                if (status == SS$_NORMAL)
                        status = iosb[0] & 0xFFFF;
#else
                status = SYS$QIO(EFN+1, iochan, IO$_WRITELBLK|IO$M_NOFORMAT,
                         0, 0, 0, obuf, nobuf, 0, 0, 0, 0);
#endif
                nobuf = 0;
        }
        return (status);
#endif

#if     CPM
#endif

#if     MSDOS
#endif

#if     V7 | USG | BSD | DECUSC
        fflush(stdout);
#endif
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
int ttgetc()
{
#if     AMIGA
        char ch;
        amg_flush();
        Read(terminal, &ch, 1L);
        return(255 & (int)ch);
#endif
#if	ST520 & MEGAMAX
	long ch;
/*
 * blink the cursor only if nothing is happening, this keeps the
 * cursor on steadily during movement making it easier to track
 */
	STcurblink(TRUE);  /* the cursor blinks while we wait */
	ch = Bconin(2);
	STcurblink(FALSE); /* the cursor is steady while we work */
	STscancode = (ch >> 16) & 0xff;
       	return(255 & (int)ch);
#endif
#if     VMS
        int     status;
        int     iosb[2];
        int     term[2];

        while (ibufi >= nibuf) {
                ibufi = 0;
                term[0] = 0;
                term[1] = 0;
                status = SYS$QIOW(EFN, iochan,
			 IO$_READLBLK|IO$M_TIMED|IO$M_NOFILTR|IO$M_NOECHO,
                         iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
                if (status != SS$_NORMAL && status != SS$_DATAOVERUN &&
		    status != SS$_PARTESCAPE)
                        if (!(termflag&1)) exit(status);
                status = iosb[0] & 0xFFFF;
                if (status!=SS$_NORMAL && status!=SS$_DATAOVERUN &&
		    status!=SS$_PARTESCAPE && status!=SS$_TIMEOUT)
                        if (!(termflag&1)) exit(status);
                nibuf = (iosb[0]>>16) + (iosb[1]>>16);
                if (nibuf == 0) {
                        status = SYS$QIOW(EFN, iochan,
				 IO$_READLBLK|IO$M_NOFILTR|IO$M_NOECHO,
                                 iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
                        if ((status != SS$_NORMAL && status != SS$_DATAOVERUN &&
                             status != SS$_PARTESCAPE)
                        || (((status = (iosb[0]&0xFFFF)) != SS$_NORMAL) &&
			    (status != SS$_PARTESCAPE &&
				status != SS$_DATAOVERUN)))
                                if (!(termflag&1)) exit(status);
                        nibuf = (iosb[0]>>16) + (iosb[1]>>16);
                }
        }
        return (ibuf[ibufi++] & 0xFF);    /* Allow multinational  */
#endif

#if     CPM
        return (biosb(BCONIN, 0, 0));
#endif

#if RAINBOW
        int Ch;

        while ((Ch = Read_Keyboard()) < 0);

        if ((Ch & Function_Key) == 0)
                if (!((Ch & 0xFF) == 015 || (Ch & 0xFF) == 0177))
                        Ch &= 0xFF;

        return Ch;
#endif

#if     MSDOS & MWC86
        return (getcnb());
#endif

#if	MSDOS & (LATTICE | MSC | TURBO | AZTEC | C86 | GNUC | ZORTECH)
	int c;		/* character read */

	/* if a char already is ready, return it */
	if (nxtchar >= 0) {
		c = nxtchar;
		nxtchar = -1;
		return(c);
	}

	/* call the dos to get a char */
	rg.h.ah = 7;		/* dos Direct Console Input call */
	intdos(&rg, &rg);
	c = rg.h.al;		/* grab the char */
	return(c & 255);
#endif

#if     V7 | BSD
	int ch;

	do {
		ch = fgetc(stdin);
	} while ((termflag&1) != 0 && (ch == '\021' || ch == '\023'));
	return(ch & 0xFF);
#endif

#if	USG && !defined(__MINGW32__)
#if	MSC
        /* 
         * if (nxtchar >= 0) {c = nxtchar; nxtchar = -1;}
         * else read(0, &c, sizeof(c));
         */
        int count;
	int ch;
	do {
	if (ibufi >= nibuf) {
		ibufi = nibuf = 0;
		while(nibuf < NIBUF &&
		      (count = read(tafd, &ibuf[nibuf], NIBUF-nibuf)) > 0)
			nibuf += count;
		if (nibuf <= 0) {
			while(read(0, ibuf, sizeof(char)) < 1)
				;
		}
	}
	ch = ibuf[ibufi++] & 0xFF;
	} while ((termflag&1) != 0 && (ch == '\021' || ch == '\023'));
	return(ch);
#else
	if( kbdqp )
		kbdqp = FALSE;
	else
	{
		if( kbdpoll && fcntl( 0, F_SETFL, kbdflgs ) < 0 )
			return FALSE;
		kbdpoll = FALSE;
		while (read(0, &kbdq, 1) != 1)
			;
	}
	return ( kbdq & 0xFF );
#endif
#endif

#if	DECUSC
	return( kbin() );
#endif

#if (defined(_MSC_VER) && defined(_MSC_VER)) || defined(__MINGW32__)
	return ( _getch() & 0xFF );
#endif
}

#if	TYPEAH
		/* & (~ST520 | ~LATTICE) */
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

NOSHARE int tacnt = 0;

int typahead()
{
#if	WINMOD
	return(FALSE);
#else
#if	(MSDOS & (MSC)) || (defined(_MSC_VER) && defined(_WIN32)) || defined(__MINGW32__)
	if (kbhit() != 0)
		return(TRUE);
	else
		return(FALSE);
#endif

#if	MSDOS & (LATTICE | AZTEC | MWC86 | C86 | TURBO | GNUC | ZORTECH)
	int c;		/* character read */
	int flags;	/* cpu flags from dos call */

	if (nxtchar >= 0)
		return(TRUE);

	rg.h.ah = 6;	/* Direct Console I/O call */
	rg.h.dl = 255;	/*         does console input */
#if	LATTICE | AZTEC | C86
	flags = intdos(&rg, &rg);
#else
#if	TURBO | GNUC | ZORTECH
	intdos(&rg, &rg);
#else
	intcall(&rg, &rg, 0x21);
#endif
	flags = rg.x.flags;
#endif
	c = rg.h.al;	/* grab the character */

	/* no character pending */
	if ((flags & 64) != 0)
		return(FALSE);

	/* save the character and return true */
	nxtchar = c;
	return(TRUE);
#endif

#if	BSD
#ifdef	FIONREAD
	int x;	/* holds # of pending chars */

	return((ioctl(0,FIONREAD,&x) < 0) ? 0 : x);
#endif
#endif

#if	USG && !defined(__MINGW32__)
#if	MSC
	int count;

	/* if (nxtchar >= 0) return(TRUE);
	 * if (read(tafd, &c, sizeof(c)) == sizeof(c)) {
	 *	nxtchar = c & 127;
	 *	return(TRUE);
	 * }
	 */
	 
	/* check for unread chars in input buffer */

	if (ibufi < nibuf) return(TRUE);

	/* only do inquire every few times because of how slow it is */

	if (tacnt-- > 0) return(FALSE);

	tacnt = 2;

	ibufi = nibuf = 0;
	while(nibuf < NIBUF &&
	      (count = read(tafd, &ibuf[nibuf], NIBUF-nibuf)) > 0)
		nibuf += count;

	return(ibufi < nibuf);
#else
	if( use_kbdpoll && !kbdqp )
	{
		if (tacnt-- > 0) return(FALSE);
		tacnt = 2;
		if( !kbdpoll && fcntl( 0, F_SETFL, kbdflgs | O_NDELAY ) < 0 )
			return(FALSE);
		kbdqp = (1 == read( 0, &kbdq, 1 ));
		kbdpoll = TRUE;
	}
	return ( kbdqp );
#endif
#endif

#if	VMS
	int	iosb[2], tabuffer[2], term[2], status;

	/* check for unread chars in input buffer */

        if (ibufi < nibuf) return(TRUE);

	/* only do inquire every few times because of how slow it is */

	if (tacnt-- > 0) return(FALSE);

	tacnt = 5;

	/* type ahead count returns:
	 *	buffer[0] :0-15 = count :16-23 = first char :24-31 reserved
	 *	buffer[1] reserved
	 */

	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE|IO$M_TYPEAHDCNT,
			iosb, 0, 0, tabuffer, 0, 0, 0, 0, 0);

	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return(FALSE);

	if ((tabuffer[0] & 0xFFFF) == 0) return(FALSE);

	ibufi = 0;
	term[0] = 0;
	term[1] = 0;
	status = SYS$QIOW(EFN, iochan,
			IO$_READLBLK|IO$M_TIMED|IO$M_NOFILTR|IO$M_NOECHO,
			iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
	if (status != SS$_NORMAL && status != SS$_DATAOVERUN &&
	    status != SS$_PARTESCAPE)
		if (!(termflag&1)) exit(status);
	status = iosb[0] & 0xFFFF;
	if (status!=SS$_NORMAL && status!=SS$_DATAOVERUN &&
	    status!=SS$_PARTESCAPE && status!=SS$_TIMEOUT)
		if (!(termflag&1)) exit(status);
	nibuf = (iosb[0]>>16) + (iosb[1]>>16);

	return( (nibuf > 0) ? TRUE : FALSE );
#endif
	return(FALSE);
#endif	/* WINMOD */
}
#endif

#if	VMSVT | TERMCAP
VOID ttsetwid(n)
int n; {
	if (isvt100 == TRUE) {
		ttputc('\033'); ttputc('['); ttputc('?'); ttputc('3');
		if (n > 80) ttputc('h'); else ttputc('l');
	}
}

VOID ttscroll(rowa, rowb, lines)
int rowa, rowb, lines;
{
	if (isvt100 == TRUE) {
		++rowa; ++rowb;
		if (lines == 0 || rowa != scrltop || rowb != scrlbot) {
			scrltop = rowa;
			scrlbot = rowb;

			/* define scrolling region: CSI top ; bot r */
			ttputc(27); ttputc('[');
			if (rowa >= 10) ttputc('0' + rowa / 10);
			ttputc('0' + rowa % 10); ttputc(';');
			if (rowb >= 10) ttputc('0' + rowb / 10);
			ttputc('0' + rowb % 10); ttputc('r');
			phrow = 1000;
		}

		if (lines > 0) {
			/* scroll up: CSI bot H, and then n X nl */
			if (rowb != phrow+1) {
				ttputc(27); ttputc('[');
				if (rowb >= 10) ttputc('0' + rowb / 10);
				ttputc('0' + rowb % 10); ttputc('H');
				phrow = rowb-1;
			}
			while (lines-- > 0) ttputc(10);
		}
		else if (lines < 0) {
			/* scroll down: CSI top H, and then n X ESC M */
			if (rowa != phrow+1) {
				ttputc(27); ttputc('[');
				if (rowa >= 10) ttputc('0' + rowa / 10);
				ttputc('0' + rowa % 10); ttputc('H');
				phrow = rowa-1;
			}
			while (lines++ < 0) {ttputc(27); ttputc('M');}
		}
		ttrow = 1000;
	}
}
#endif

#if	TERMCAP

#ifdef TIOCGWINSZ
#if defined(M_UNIX) && !defined(_SYS_WINSIZE_H)
#ifndef _SYS_STREAM_H
#include <sys/stream.h>
#endif
#include <sys/ptem.h>
#endif
#endif

/* Return the current window size */

VOID getwinsize(ncols, nrows)
short *ncols, *nrows;
{
#ifdef TIOCGWINSZ

	/* Get the screen size on systems where the curses calls */
	/* return a fixed size instead of the current xterm size. */

	/* Remove the code below if it gives any compile errors. */

	int i;
	struct winsize ws;
	static int did_ioctl = FALSE;
	static int new_cols = 0;
	static int new_rows = 0;

	if (did_ioctl == FALSE) {

		did_ioctl = TRUE;

#ifdef M_UNIX
		/* SCO messes this up, so only check if inside an xterm */
		{
			const char *s;
			s = getenv("DISPLAY");
			if (s == NULL || *s == '\0') {
				s = getenv("TERM");
				if (s == NULL || strcmp(s, "xterm") != 0) {
					return;
				}
			}
		}
#endif

		ws.ws_col = 0;
		ws.ws_row = 0;

		i = ioctl(1, TIOCGWINSZ, &ws);

		if (i == -1) i = ioctl(0, TIOCGWINSZ, &ws);

		if (i == 0 && ws.ws_col > 0 && ws.ws_row > 1) {
			new_cols = ws.ws_col;
			new_rows = ws.ws_row - 1;
		}

#if 0	/* DEBUG */
		fprintf(stderr, "getwinsize: ioctl %d -> cols %d rows %d\n",
			i, ws.ws_col, ws.ws_row);
#endif
	}

	if (new_cols > 0 && new_rows > 0) {
		*ncols = (short) new_cols;
		*nrows = (short) new_rows;
	}
#endif
}

#endif
