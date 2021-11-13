/*
 *  Advanced VMS terminal driver
 *
 *  Knows about any terminal defined in SMGTERMS.TXT and TERMTABLE.TXT
 *  located in SYS$SYSTEM.
 *
 *  Author:  Curtis Smith
 *  Last Updated: 07/14/87
 */

#include	<stdio.h>		/* Standard I/O package		*/
#include	"estruct.h"		/* Emacs' structures		*/
#include	"edef.h"		/* Emacs' definitions		*/

#if	VMSVT

#include	 <descrip.h>		/* Descriptor definitions	*/

#include	<iodef.h>
#include	<ttdef.h>
#include	<tt2def.h>

/*  These would normally come from iodef.h and ttdef.h  */
#ifndef IO$_SENSEMODE
#define IO$_SENSEMODE	0x27		/* Sense mode of terminal	*/
#endif

/* #define TT$_UNKNOWN	0x00 */		/* Unknown terminal		*/

/** Forward references **/
int vmsopen(), vmsclose(), vmskopen(), vmskclose(), ttgetc(), ttputc();
int ttflush(), vmsmove(), vmseeol(), vmseeop(), vmsbeep(), vmsrev();
int vmscres();
NOSHARE extern int eolexist, revexist, termflag;
NOSHARE extern char sres[];

#define	ESC	27

#if COLOR
int vmsfcol(), vmsbcol();

NOSHARE int isansi = 0;		/* has support for ansi color */
NOSHARE int cfcolor = -1;	/* current forground color */
NOSHARE int cbcolor = -1;	/* current background color */
NOSHARE int usedcolor = 0;	/* true if used a color */
#endif

/** SMG stuff **/
static char * begin_reverse, * end_reverse, * erase_to_end_line;
static char * erase_whole_display;
static int termtype;
static int vmstermtype;

#define SMG$K_BEGIN_REVERSE		0x1bf
#define SMG$K_END_REVERSE		0x1d6
#define SMG$K_SET_CURSOR_ABS		0x23a
#define SMG$K_ERASE_WHOLE_DISPLAY	0x1da
#define SMG$K_ERASE_TO_END_LINE		0x1d9


/* Dispatch table. All hard fields just point into the terminal I/O code. */
NOSHARE TERM	term	= {
	24 - 1,				/* Max number of rows allowable */
	/* Filled in */ - 1,		/* Current number of rows used	*/
	132,				/* Max number of columns	*/
	/* Filled in */ 0,		/* Current number of columns	*/
	64,				/* Min margin for extended lines*/
	8,				/* Size of scroll region	*/
	100,				/* # times thru update to pause */
	vmsopen,			/* Open terminal at the start	*/
	vmsclose,			/* Close terminal at end	*/
	vmskopen,			/* Open keyboard		*/
	vmskclose,			/* Close keyboard		*/
	ttgetc,				/* Get character from keyboard	*/
	ttputc,				/* Put character to display	*/
	ttflush,			/* Flush output buffers		*/
	vmsmove,			/* Move cursor, origin 0	*/
	vmseeol,			/* Erase to end of line		*/
	vmseeop,			/* Erase to end of page		*/
	vmsbeep,			/* Beep				*/
	vmsrev,				/* Set reverse video state	*/
	vmscres				/* Change screen resolution	*/
#if	COLOR
	, vmsfcol,			/* Set forground color		*/
	vmsbcol				/* Set background color		*/
#endif
};

NOSHARE int oldwidth;
NOSHARE int vmsinrev;

NOSHARE extern char obuf[NOBUF];	/* output buffer */
NOSHARE extern int nobuf;		/* # of bytes in obuf */

#define fastputc(c)	{if (nobuf >= NOBUF) ttflush(); obuf[nobuf++] = c;}

/***
 *  ttputs  -  Send a string to ttputc
 *
 *  Nothing returned
 ***/
ttputs(string)
char * string;				/* String to write		*/
{
	if (string)
		while (*string != '\0')
			fastputc(*string++);
}

/***
 * ttputi - Send an integer to ttputc
 ***/
ttputi(i)
int i;
{
	char buf[20];
	int len;

	if (i < 0) {ttputc('-'); i = -i;}
	len = 0;
	do {
		buf[len++] = '0' + i % 10;
		i /= 10;
	} while (i > 0);

	while (len-- > 0)
		fastputc(buf[len]);
}

/***
 *  vmsmove  -  Move the cursor (0 origin)
 *
 *  Nothing returned
 ***/
vmsmove(row, col)
int row;				/* Row position			*/
int col;				/* Column position		*/
{
	char buffer[32];
	int ret_length;
	static int request_code = SMG$K_SET_CURSOR_ABS;
	static int max_buffer_length = sizeof(buffer);
	static int arg_list[3] = { 2 };
	register char * cp;

	register int i;

	if (row == phrow+1 && col == 0 && row != scrlbot) {
		fastputc(13); fastputc(10);
	}

	else if (vmstermtype == TT$_UNKNOWN) {
		fastputc('\033');	fastputc('=');
		fastputc(row+' ');	fastputc(col+' ');
	}

	else {

		/* Set the arguments into the arg_list array
		 * SMG assumes the row/column positions are 1 based (boo!)
		 */
		arg_list[1] = row + 1;
		arg_list[2] = col + 1;

		if ((smg$get_term_data(		/* Get terminal data	*/
			&termtype,		/* Terminal table address */
			&request_code,		/* Request code		*/
			&max_buffer_length,	/* Maximum buffer length */
			&ret_length,		/* Return length	*/
			buffer,			/* Capability data buffer */
			arg_list)		/* Argument list array	*/

		/* We'll know soon enough if this doesn't work		*/
				& 1) == 0) {
					ttputs("OOPS");
					return;
				}

		/* Send out resulting sequence				*/
		i = ret_length;
		cp = buffer;
		while (i-- > 0)
			fastputc(*cp++);
	}
	phrow = row;
}


/***
 *  vmsrev  -  Set the reverse video status
 *
 *  Nothing returned
 ***/
vmsrev(status)
int status;				/* TRUE if setting reverse	*/
{
#if	COLOR
	if (usedcolor) return;
#endif
	if (vmsinrev == status) return;
	vmsinrev = status;
	if (!revexist)	return;
	if (status)
		ttputs(begin_reverse);
	else 
		ttputs(end_reverse);
}

/***
 *  vmscres  -  Change screen resolution (which it doesn't)
 *
 *  Nothing returned
 ***/
vmscres()
{
	/* But it could.  For vt100/vt200s, one could switch from
	80 and 132 columns modes */
}


#if	COLOR
vmsparm(n)
register int n;
{
	register q,r;

	q = n/10;
	if (q != 0)	{
		r = q/10;
		if (r != 0)	{fastputc('0' + r%10);}
		fastputc('0' + q%10);
	}
	fastputc('0' + n%10);
}

/***
 *  vmsfcol  -  Set the forground color
 *
 *  Nothing returned
 ***/
vmsfcol(color)
int color;
{
	if (!usedcolor || color == cfcolor)	return;
	fastputc(ESC);
	fastputc('[');
	vmsparm(color + 30);
	fastputc('m');
	cfcolor = color;
}

/***
 *  vmsbcol  -  Set the background color
 *
 *  Nothing returned
 ***/
vmsbcol(color)
int color;
{
	if (!usedcolor || color == cbcolor)	return;
	fastputc(ESC);
	fastputc('[');
	vmsparm(color + 40);
	fastputc('m');
	cbcolor = color;
}
#endif

/***
 *  vmseeol  -  Erase to end of line
 *
 *  Nothing returned
 ***/
vmseeol()
{
#if	COLOR
	vmsfcol(gfcolor);
	vmsbcol(gbcolor);
#endif
	ttputs(erase_to_end_line);
}


/***
 *  vmseeop  -  Erase to end of page (clear screen)
 *
 *  Nothing returned
 ***/
vmseeop()
{
#if	COLOR
	vmsfcol(gfcolor);
	vmsbcol(gbcolor);
#endif
	ttputs(erase_whole_display);
	phrow = 1000;
}


/***
 *  vmsbeep  -  Ring the bell
 *
 *  Nothing returned
 ***/
vmsbeep()
{
	fastputc('\007');
}


/***
 *  vmsgetstr  -  Get an SMG string capability by name
 *
 *  Returns:	Escape sequence
 *		NULL	No escape sequence available
 ***/ 
char * vmsgetstr(request_code)
int request_code;			/* Request code			*/
{
	register char * result;
	static char seq_storage[1024];
	static char * buffer = seq_storage;
	static int arg_list[2] = { 1, 1 };
	int max_buffer_length, ret_length;

	/*  Precompute buffer length */
	
	max_buffer_length = (seq_storage + sizeof(seq_storage)) - buffer;

	/* Get terminal commands sequence from master table */

	if ((smg$get_term_data(	/* Get terminal data		*/
		&termtype,	/* Terminal table address	*/
		&request_code,	/* Request code			*/
		&max_buffer_length,/* Maximum buffer length	*/
		&ret_length,	/* Return length		*/
		buffer,		/* Capability data buffer	*/
		arg_list)	/* Argument list array		*/

	/* If this doesn't work, try again with no arguments */
	
		& 1) == 0 && 

		(smg$get_term_data(	/* Get terminal data		*/
			&termtype,	/* Terminal table address	*/
			&request_code,	/* Request code			*/
			&max_buffer_length,/* Maximum buffer length	*/
			&ret_length,	/* Return length		*/
			buffer)		/* Capability data buffer	*/

	/* Return NULL pointer if capability is not available */
	
			& 1) == 0)
				return NULL;

	/* Check for empty result */
	if (ret_length == 0)
		return NULL;
	
	/* Save current position so we can return it to caller */

	result = buffer;

	/* NIL terminate the sequence for return */
	
	buffer[ret_length] = 0;

	/* Advance buffer */

	buffer += ret_length + 1;

	/* Return capability to user */
	return result;
}


/** I/O information block definitions **/
struct iosb {			/* I/O status block			*/
	short	i_cond;		/* Condition value			*/
	short	i_xfer;		/* Transfer count			*/
	long	i_info;		/* Device information			*/
};
struct termchar {		/* Terminal characteristics		*/
	char	t_class;	/* Terminal class			*/
	char	t_type;		/* Terminal type			*/
	short	t_width;	/* Terminal width in characters		*/
	long	t_mandl;	/* Terminal's mode and length		*/
	long	t_extend;	/* Extended terminal characteristics	*/
};
static struct termchar tc;	/* Terminal characteristics		*/

/***
 *  vmsgtty - Get terminal type from system control block
 *
 *  Nothing returned
 ***/
vmsgtty()
{
	short fd;
	int status;
	struct iosb iostatus;
	$DESCRIPTOR(devnam, "SYS$INPUT");

	/* Assign input to a channel */
	status = sys$assign(&devnam, &fd, 0, 0);
	if ((status & 1) == 0)
		exit (status);

	/* Get terminal characteristics */
	status = sys$qiow(		/* Queue and wait		*/
		0,			/* Wait on event flag zero	*/
		fd,			/* Channel to input terminal	*/
		IO$_SENSEMODE,		/* Get current characteristic	*/
		&iostatus,		/* Status after operation	*/
		0, 0,			/* No AST service		*/
		&tc,			/* Terminal characteristics buf */
		sizeof(tc),		/* Size of the buffer		*/
		0, 0, 0, 0);		/* P3-P6 unused			*/

	/* De-assign the input device */
	if ((sys$dassgn(fd) & 1) == 0)
		exit(status);

	/* Jump out if bad status */
	if ((status & 1) == 0)
		exit(status);
	if ((iostatus.i_cond & 1) == 0)
		exit(iostatus.i_cond);
}


/***
 *  vmsopen  -  Get terminal type and open terminal
 *
 *  Nothing returned
 ***/
vmsopen()
{
	phrow = scrltop = scrlbot = 1000;
	vmsinrev = FAILED;

	/* Get terminal type */
	vmsgtty();
	vmstermtype = tc.t_type;

	if (vmstermtype == TT$_UNKNOWN) {
#if	0
		printf("Terminal type is unknown!\n");
		printf("Try set your terminal type with SET TERMINAL/INQUIRE\n");
		printf("Or get help on SET TERMINAL/DEVICE_TYPE\n");
		exit(3);
#endif
		/* assume ADM3A */

		/* Set sizes */
		term.t_nrow = 80;
		term.t_ncol = 24;

		/* set some capabilities */
		begin_reverse = "";
		end_reverse = "";
		revexist = FALSE;
		erase_to_end_line = "";
		eolexist = FALSE;
		erase_whole_display = "\032";

		/* Set resolution */
		strcpy(sres, "NORMAL");

		oldwidth = term.t_ncol;
		isvt100 = FALSE;

		/* Open terminal I/O drivers */
		ttopen();

		return;
	}

	/* Access the system terminal definition table for the		*/
	/* information of the terminal type returned by IO$_SENSEMODE	*/
	if ((smg$init_term_table_by_type(&tc.t_type, &termtype) & 1) == 0)
		return -1;
		
	/* Set sizes */
	term.t_nrow = ((unsigned int) tc.t_mandl >> 24) - 1;
	term.t_ncol = tc.t_width;

	/* Get some capabilities */
	begin_reverse = vmsgetstr(SMG$K_BEGIN_REVERSE);
	end_reverse = vmsgetstr(SMG$K_END_REVERSE);
	revexist = begin_reverse != NULL && end_reverse != NULL &&
			(termflag&4) == 0;
	erase_to_end_line = vmsgetstr(SMG$K_ERASE_TO_END_LINE);
	eolexist = erase_whole_display != NULL;
	erase_whole_display = vmsgetstr(SMG$K_ERASE_WHOLE_DISPLAY);

	/* Set resolution */
	strcpy(sres, "NORMAL");

	oldwidth = term.t_ncol;
	switch (vmstermtype) {
		case TT$_VT100:
		case TT$_VT101:
		case TT$_VT102:
		case TT$_VT105:
		case TT$_VT125:
		case TT$_VT131:
		case TT$_VT132:
		case TT$_VT200_SERIES:
			isvt100 = TRUE;
			break;
		default:
			isvt100 = FALSE;
			break;
	}
#if	COLOR
	isansi = isvt100;
	cfcolor = -1;
	cbcolor = -1;
#endif
	if (isvt100) {
		ttputs("\033=");	/* enter keypad mode */
		if (!eolexist) {
			erase_to_end_line = "\033[K";
			eolexist = TRUE;
		}
	}

	/* Open terminal I/O drivers */
	ttopen();
}

vmsclose()
{
	if (isvt100) {
		if (!(tc.t_extend & TT2$M_APP_KEYPAD))
			ttputs("\033>");	/* leave keypad mode */
		ttsetwid(oldwidth);
	}
#if	COLOR
	if (usedcolor) {
		vmsfcol(7);
		vmsbcol(0);
	}
#endif
	ttclose();
}


/***
 *  vmskopen  -  Open keyboard (not used)
 *
 *  Nothing returned
 ***/
vmskopen()
{
}


/***
 *  vmskclose  -  Close keyboard (not used)
 *
 *  Nothing returned
 ***/
vmskclose()
{
}


/***
 *  fnclabel  -  Label function keys (not used)
 *
 *  Nothing returned
 ***/
#if	FLABEL
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif


/***
 *  spal  -  Set palette type  (Are you kidding?)
 *
 *  Nothing returned
 ***/
spal(dummy)
char *dummy;
{
}

#if	CALLABLE

/* VMS Callable editor interface */

me$edit(indesc, outdesc, comdesc, joudesc, options, fileio, workio, xlate)
struct dsc$descriptor *indesc, *outdesc, *comdesc, *joudesc;
int *options;
int (*fileio)(), (*workio)(), (*xlate)();
{
	int argc;
	char *argv[10];
	char inname[NFILEN], outname[NFILEN], comname[NFILEN];
	int len;
	int files;

	argc = files = 0;

	argv[argc++] = "emacs";

	argv[argc++] = "-t";	/* usually required under VMS */

	if (outdesc && outdesc->dsc$w_length && outdesc->dsc$a_pointer) {
		len = outdesc->dsc$w_length;
		if (len >= NFILEN) len = NFILEN - 1;
		bytecopy(outname, outdesc->dsc$a_pointer, len);
		argv[argc++] = outname;
		++files;
	}

	if (indesc && indesc->dsc$w_length && indesc->dsc$a_pointer) {
		len = indesc->dsc$w_length;
		if (len >= NFILEN) len = NFILEN - 1;
		bytecopy(inname, indesc->dsc$a_pointer, len);
		if (files > 0)
			argv[argc++] = "-v";	/* mark as view-only */
		argv[argc++] = inname;
		++files;
	}

	if (comdesc && comdesc->dsc$w_length && comdesc->dsc$a_pointer) {
		len = comdesc->dsc$w_length;
		if (len >= NFILEN-1) len = NFILEN - 2;
		comname[0] = '@';
		bytecopy(comname+1, comdesc->dsc$a_pointer, len);
		argv[argc++] = comname;
	}

	if (files >= 2)	argv[argc++] = "-2";	/* split window */

	argv[argc] = NULL;

	emacsmain(argc, argv);

	return(1);
}
#endif

#else

/***
 *  hellovms  -  Avoid error because of empty module
 *
 *  Nothing returned
 ***/
hellovms()
{
}

#endif

