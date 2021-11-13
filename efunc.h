/*	EFUNC.H:	MicroEMACS function declarations and names

		This file list all the C code functions used by MicroEMACS
	and the names to use to bind keys to them. To add functions,
	declare it here in both the extern function list and the name
	binding table.

*/

#ifndef _EFUNC_H_
#define	_EFUNC_H_

/*	External function declarations		*/

extern  int ctrlg( int f, int n );      /* Abort out of things          */
extern  int quit( int f, int n );       /* Quit                         */
extern  int ctlxlp( int f, int n );     /* Begin macro                  */
extern  int ctlxrp( int f, int n );     /* End macro                    */
extern  int ctlxe( int f, int n );      /* Execute macro                */
extern  int fileread( int f, int n );   /* Get a file, read only        */
extern  int filefind( int f, int n );   /* Get a file, read write       */
extern  int filewrite( int f, int n );  /* Write a file                 */
extern	int fileapp( int f, int n );    /* Append a file		*/
extern  int filesave( int f, int n );   /* Save current file            */
extern  int filename( int f, int n );   /* Adjust file name             */
#if	ABACKUP
extern  int filercover( int f, int n ); /* Get a saved file		*/
extern	int fileback( int f, int n );   /* Autosaves the file	*/
extern	int filebkall( int f, int n );	/* Autosave all files	*/
#endif
extern  int getccol();              /* Get current column           */
extern  int gobol( int f, int n );	/* Move to start of line        */
extern  int forwchar( int f, int n );   /* Move forward by characters   */
extern  int goeol( int f, int n );	/* Move to end of line          */
extern  int backchar( int f, int n );   /* Move backward by characters  */
extern  int forwline( int f, int n );   /* Move forward by lines        */
extern  int backline( int f, int n );   /* Move backward by lines       */
extern  int forwpage( int f, int n );   /* Move forward by pages        */
extern  int backpage( int f, int n );   /* Move backward by pages       */
extern  int gobob( int f, int n );	/* Move to start of buffer      */
extern  int goeob( int f, int n );	/* Move to end of buffer        */
extern  int setfillcol( int f, int n ); /* Set fill column.             */
extern  int setmark( int f, int n );    /* Set mark                     */
extern  int remmark( int f, int n );    /* clear mark                   */
extern  int swapmark( int f, int n );   /* Swap "." and mark            */
extern  int forwsearch( int f, int n ); /* Search forward               */
extern  int backsearch( int f, int n ); /* Search backwards             */
extern	int sreplace( int f, int n );	/* search and replace		*/
extern	int qreplace( int f, int n );	/* search and replace w/query	*/
extern  int showcpos( int f, int n );   /* Show the cursor position     */
extern	int showfiles( int f, int n );  /* Show files in a directory	*/
extern	int showmodes( int f, int n );	/* Show files list of modes	*/
extern  int nextwind( int f, int n );   /* Move to the next window      */
extern  int prevwind( int f, int n );   /* Move to the previous window  */
extern  int onlywind( int f, int n );   /* Make current window only one */
extern  int splitwind( int f, int n );  /* Split current window         */
extern  int mvdnwind( int f, int n );   /* Move window down             */
extern  int mvupwind( int f, int n );   /* Move window up               */
extern  int enlargewind( int f, int n ); /* Enlarge display window.     */
extern  int shrinkwind( int f, int n ); /* Shrink window.               */
extern  int listbuffers( int f, int n ); /* Display list of buffers     */
extern  int usebuffer( int f, int n );  /* Switch a window to a buffer  */
extern  int killbuffer( int f, int n ); /* Make a buffer go away.       */
extern  int reposition( int f, int n ); /* Reposition window            */
extern  int refresh( int f, int n );    /* Refresh the screen           */
extern  int twiddle( int f, int n );    /* Twiddle characters           */
extern  int tab( int f, int n );        /* Insert tab                   */
extern  int newline( int f, int n );    /* Insert CR-LF                 */
extern  int indent( int f, int n );     /* Insert CR-LF, then indent    */
extern  int openline( int f, int n );   /* Open up a blank line         */
extern  int deblank( int f, int n );    /* Delete blank lines           */
extern  int quote( int f, int n );      /* Insert literal               */
#if DISPSEVEN | DECEDT
extern	int me_compose ( int f, int n ); /* Compose char		*/
extern	int me_unquotep ( int f, int n ); /* Unquote char		*/
extern	int me_unprefix ( int f, int n ); /* Unprefix char		*/
extern	int me_add_acute ( int f, int n ); /* Add acute accent		*/
extern	int me_add_bar ( int f, int n ); /* Add bar accent		*/
extern	int me_add_cedilla ( int f, int n ); /* Add bar cedilla		*/
extern	int me_add_circ ( int f, int n ); /* Add circumflex accent	*/
extern	int me_add_grave ( int f, int n ); /* Add grave accent		*/
extern	int me_add_slash ( int f, int n ); /* Add slash accent		*/
extern	int me_add_tilde ( int f, int n ); /* Add tilde accent		*/
extern	int me_add_umlaut ( int f, int n ); /* Add umlaut accent	*/
extern	int me_add_ring ( int f, int n ); /* Add ring accent		*/
#endif
extern  int backword( int f, int n );   /* Backup by words              */
extern  int forwword( int f, int n );   /* Advance by words             */
extern  int forwdel( int f, int n );    /* Forward delete               */
extern  int backdel( int f, int n );    /* Backward delete              */
extern  int killtext( int f, int n );   /* Kill forward                 */
extern  int yank( int f, int n );       /* Yank back from killbuffer.   */
extern  int upperword( int f, int n );  /* Upper case word.             */
extern  int lowerword( int f, int n );  /* Lower case word.             */
extern  int upperregion( int f, int n ); /* Upper case region.          */
extern  int lowerregion( int f, int n ); /* Lower case region.          */
extern  int capword( int f, int n );    /* Initial capitalize word.     */
extern  int delfword( int f, int n );   /* Delete forward word.         */
extern  int delbword( int f, int n );   /* Delete backward word.        */
extern  int killregion( int f, int n ); /* Kill region.                 */
extern  int copyregion( int f, int n ); /* Copy region to kill buffer.  */
extern  int spawncli( int f, int n );   /* Run CLI in a subjob.         */
extern  int spawncmd( int f, int n );	/* Run a command in a subjob.   */
#if	BSD | V7 | USG | VMS
extern	int bktoshell();		/* suspend emacs to parent shell*/
#endif
extern  int quickexit( int f, int n );  /* low keystroke style exit.    */
extern	int quicksave( int f, int n );  /* save all modified files	*/
extern	int me_setmode( int f, int n );	/* set an editor mode		*/
extern	int delmode( int f, int n );	/* delete a mode		*/
extern	int gotoline( int f, int n );	/* go to a numbered line	*/
extern	int namebuffer( int f, int n );	/* rename the current buffer	*/
#if	WORDPRO
extern	int gobop( int f, int n );	/* go to beginning/paragraph	*/
extern	int goeop( int f, int n );	/* go to end/paragraph		*/
extern	int fillpara( int f, int n );	/* fill current paragraph	*/
#endif
extern	int me_help( int f, int n );	/* get the help file here	*/
extern	int deskey( int f, int n );	/* describe a key's binding	*/
extern	int viewfile( int f, int n );   /* find a file in view mode	*/
extern	int insfile( int f, int n );    /* insert a file		*/
extern	int scrupnxt( int f, int n );	/* scroll next window back	*/
extern	int scrdwnxt( int f, int n );	/* scroll next window down	*/
extern	int bindtokey( int f, int n );	/* bind a function to a key	*/
extern	int unbindkey( int f, int n );	/* unbind a key's function	*/
extern	int namedcmd( int f, int n );	/* execute named command	*/
extern	int desbind( int f, int n );	/* describe bindings		*/
extern	int execcmd( int f, int n );	/* execute a command line	*/
extern	int execbuf( int f, int n );	/* exec commands from a buffer	*/
extern	int execfile( int f, int n );	/* exec commands from a file	*/
extern	int nextbuffer( int f, int n );	/* switch to the next buffer	*/
extern	int prevbuffer( int f, int n );	/* switch to the prev bufffer	*/
#if	WORDPRO
extern	int killpara( int f, int n );	/* kill the current paragraph	*/
#endif
extern	int setgmode( int f, int n );	/* set a global mode		*/
extern	int delgmode( int f, int n );	/* delete a global mode		*/
extern	int insspace( int f, int n );	/* insert a space forword	*/
extern	int forwhunt( int f, int n );	/* hunt forward for next match	*/
extern	int backhunt( int f, int n );	/* hunt backwards for next match*/
extern	int pipecmd( int f, int n );	/* pipe command into buffer	*/
extern	int filter( int f, int n );	/* filter buffer through dos	*/
extern	int filtnew( int f, int n );	/* filter buffer into new buffer*/
extern	int delwind( int f, int n );	/* delete the current window	*/
extern	int cbuf1( int f, int n );	/* execute numbered comd buffer */
extern	int cbuf2( int f, int n );
extern	int cbuf3( int f, int n );
extern	int cbuf4( int f, int n );
extern	int cbuf5( int f, int n );
extern	int cbuf6( int f, int n );
extern	int cbuf7( int f, int n );
extern	int cbuf8( int f, int n );
extern	int cbuf9( int f, int n );
extern	int cbuf10( int f, int n );
extern	int cbuf11( int f, int n );
extern	int cbuf12( int f, int n );
extern	int cbuf13( int f, int n );
extern	int cbuf14( int f, int n );
extern	int cbuf15( int f, int n );
extern	int cbuf16( int f, int n );
extern	int cbuf17( int f, int n );
extern	int cbuf18( int f, int n );
extern	int cbuf19( int f, int n );
extern	int cbuf20( int f, int n );
extern	int cbuf21( int f, int n );
extern	int cbuf22( int f, int n );
extern	int cbuf23( int f, int n );
extern	int cbuf24( int f, int n );
extern	int cbuf25( int f, int n );
extern	int cbuf26( int f, int n );
extern	int cbuf27( int f, int n );
extern	int cbuf28( int f, int n );
extern	int cbuf29( int f, int n );
extern	int cbuf30( int f, int n );
extern	int cbuf31( int f, int n );
extern	int cbuf32( int f, int n );
extern	int cbuf33( int f, int n );
extern	int cbuf34( int f, int n );
extern	int cbuf35( int f, int n );
extern	int cbuf36( int f, int n );
extern	int cbuf37( int f, int n );
extern	int cbuf38( int f, int n );
extern	int cbuf39( int f, int n );
extern	int cbuf40( int f, int n );
extern  int cbuf41( int f, int n );
extern  int cbuf42( int f, int n );
extern  int cbuf43( int f, int n );
extern  int cbuf44( int f, int n );
extern  int cbuf45( int f, int n );
extern  int cbuf46( int f, int n );
extern  int cbuf47( int f, int n );
extern  int cbuf48( int f, int n );
extern  int cbuf49( int f, int n );
extern  int cbuf50( int f, int n );
extern  int cbuf51( int f, int n );
extern  int cbuf52( int f, int n );
extern  int cbuf53( int f, int n );
extern  int cbuf54( int f, int n );
extern  int cbuf55( int f, int n );
extern  int cbuf56( int f, int n );
extern  int cbuf57( int f, int n );
extern  int cbuf58( int f, int n );
extern  int cbuf59( int f, int n );
extern  int cbuf60( int f, int n );
extern  int cbuf61( int f, int n );
extern  int cbuf62( int f, int n );
extern  int cbuf63( int f, int n );
extern  int cbuf64( int f, int n );
extern  int cbuf65( int f, int n );
extern  int cbuf66( int f, int n );
extern  int cbuf67( int f, int n );
extern  int cbuf68( int f, int n );
extern  int cbuf69( int f, int n );
extern  int cbuf70( int f, int n );
extern  int cbuf71( int f, int n );
extern  int cbuf72( int f, int n );
extern  int cbuf73( int f, int n );
extern  int cbuf74( int f, int n );
extern  int cbuf75( int f, int n );
extern  int cbuf76( int f, int n );
extern  int cbuf77( int f, int n );
extern  int cbuf78( int f, int n );
extern  int cbuf79( int f, int n );
extern  int cbuf80( int f, int n );
extern  int cbuf81( int f, int n );
extern  int cbuf82( int f, int n );
extern  int cbuf83( int f, int n );
extern  int cbuf84( int f, int n );
extern  int cbuf85( int f, int n );
extern  int cbuf86( int f, int n );
extern  int cbuf87( int f, int n );
extern  int cbuf88( int f, int n );
extern  int cbuf89( int f, int n );
extern  int cbuf90( int f, int n );
extern  int cbuf91( int f, int n );
extern  int cbuf92( int f, int n );
extern  int cbuf93( int f, int n );
extern  int cbuf94( int f, int n );
extern  int cbuf95( int f, int n );
extern  int cbuf96( int f, int n );
extern  int cbuf97( int f, int n );
extern  int cbuf98( int f, int n );
extern  int cbuf99( int f, int n );
extern	int storemac( int f, int n );	/* store text for macro		*/
extern	int resize( int f, int n );	/* resize current window	*/
extern	int me_clrmes( int f, int n );	/* clear the message line	*/
extern	int meta( int f, int n );	/* meta prefix dummy function	*/
extern	int cex( int f, int n );        /* ^X prefix dummy function	*/
extern	int unarg( int f, int n );	/* ^U repeat arg dummy function	*/
extern	int istring( int f, int n );	/* insert string in text	*/
extern	int unmark( int f, int n );	/* unmark current buffer	*/
#if	ISRCH
extern	int fisearch( int f, int n );	/* forward incremental search	*/
extern	int risearch( int f, int n );	/* reverse incremental search	*/
#endif
#if	WORDPRO
extern	int wordcount( int f, int n );	/* count words in region	*/
#endif
extern	int savewnd( int f, int n );	/* save current window		*/
extern	int restwnd( int f, int n );	/* restore current window	*/
extern	int upscreen( int f, int n );	/* force screen update		*/
extern	int writemsg( int f, int n );	/* write text on message line	*/
extern	int popbuffer( int f, int n );	/* pop-up a buffer		*/
#if	FLABEL
extern	int fnclabel( int f, int n );	/* set function key label	*/
#endif
#if	APROP
extern	int apro( int f, int n );	/* apropos fuction		*/
#endif
#if	CRYPT
extern	int me_setkey( int f, int n );	/* set encryption key		*/
#endif
extern	int wrapword( int f, int n );	/* wordwrap function		*/
#if	CFENCE
extern	int getfence( int f, int n );	/* move cursor to a matching fence */
#endif
extern	int newsize( int f, int n );	/* change the current screen size */
extern	int setvar( int f, int n );		/* set a variables value */
extern	int newwidth( int f, int n );	/* change the current screen width */

extern	int narrow( int f, int n );	/* narrow to region */
extern	int widen( int f, int n );	/* widen from narrowed region */

#if	DEBUGM
extern	int desfunc( int f, int n  );   /* describe functions */
extern	int desvars( int f, int n );	/* describe variables */
extern	int dispvar( int f, int n );	/* display a variable */
#endif

#if	ACMODE
extern	int acmode( int f, int n );	/* set into C mode for .c and .h */
#endif

#if	DECEDT
extern	int advbot( int f, int n );	/* goto top or bottom of buffer */
extern	int advchar( int f, int n );	/* advance character */
extern	int advhunt( int f, int n );	/* hunt */
extern	int advline( int f, int n );	/* advance line */
extern	int adveol( int f, int n );	/* advance end of line */
extern	int advpage( int f, int n );	/* advance page */
extern	int advsearch( int f, int n );	/* search */
extern	int advword( int f, int n );	/* advance word */
extern	int advadv( int f, int n );	/* set advance to go forward */
extern	int advback( int f, int n );	/* set advance to go backward */

extern	int nullcmd( int f, int n );	/* null enter command */
extern  int specins( int f, int n );	/* special insertion */
extern	int yankchar( int f, int n );	/* Undelete last deleted char */
extern	int caseregion( int f, int n );	/* change case */
extern	int regionwrite( int f, int n ); /* write a region to a file */
extern	int regionappend( int f, int n ); /* append a region to a file */
extern	int cutregion( int f, int n );	/* Kill region, remove mark */
extern  int apkillregion( int f, int n ); /* Kill region, add to kill buffer */
extern  int apcopyregion( int f, int n ); /* Append region to kill buffer */
extern	int special( int f, int n );	/* special prefix dummy function */
extern	int octalins( int f, int n );	/* Octal insertion */
extern	int filerevert( int f, int n );	/* Restore last saved version	*/
extern	int filewfind( int f, int n );	/* Find file into other window */
extern	int filevfind( int f, int n );	/* View file into other window */
extern	int bufffind( int f, int n );	/* FInd buffer into other window */
extern	int showslen( int f, int n );	/* Show string length */
extern	int delfline( int f, int n );	/* Delete forward line */
extern	int setovrwrt( int f, int n );	/* toggle overwrite mode */
extern	int scdnwind( int f, int n );	/* smooth scroll cursor down */
extern	int scupwind( int f, int n );	/* smooth scroll cursor up */
extern	int scnxpage( int f, int n );	/* smooth scroll next page */
extern	int scprpage( int f, int n );	/* smooth scroll previous page */
#endif

#if	AEDIT
extern	int trim( int f, int n );	/* trim whitespace from end of line */
extern	int detab( int f, int n );	/* detab rest of line */
extern	int entab( int f, int n );	/* entab rest of line */

extern	int scrlforw( int f, int n );	/* scroll forward */
extern	int scrlback( int f, int n );	/* scroll backward */
#endif

#if	PROC
extern	int storeproc( int f, int n );	/* store names procedure */
extern	int execproc( int f, int n );	/* execute procedure */
#endif

#if	MENUS
extern	int menu( int f, int n );	/* enter menu area */
#endif

extern	int nullproc( int f, int n );	/* does nothing... */

/*	Name to function binding table

		This table gives the names of all the bindable functions
	end their C function address. These are used for the bind-to-key
	function.
*/

NOSHARE NBIND	names[] = {
	{"abort-command",		ctrlg},
	{"add-global-mode",		setgmode},
	{"add-mode",			me_setmode},
#if	DECEDT
	{"adv-bottom",			advbot},
	{"adv-character",		advchar},
	{"adv-end-of-line",		adveol},
	{"adv-hunt",			advhunt},
	{"adv-line",			advline},
	{"adv-page",			advpage},
	{"adv-search",			advsearch},
	{"adv-word",			advword},
	{"advance",			advadv},
	{"append-and-copy-region",	apcopyregion},
	{"append-and-kill-region",	apkillregion},
#endif
	{"append-file",			fileapp},
#if	DECEDT
	{"append-region",		regionappend},
#endif
#if	APROP
	{"apropos",			apro},
#endif
#if	ACMODE
	{"auto-mode",			acmode},
#endif
#if	ABACKUP
	{"auto-save-all",		filebkall},
	{"auto-save-file",		fileback},
#endif
#if	DECEDT
	{"backup",			advback},
#endif
	{"backward-character",		backchar},
	{"begin-macro",			ctlxlp},
	{"beginning-of-file",		gobob},
	{"beginning-of-line",		gobol},
	{"bind-to-key",			bindtokey},
	{"buffer-position",		showcpos},
#if	DECEDT
	{"case-region-change",		caseregion},
#endif
	{"case-region-lower",		lowerregion},
	{"case-region-upper",		upperregion},
	{"case-word-capitalize",	capword},
	{"case-word-lower",		lowerword},
	{"case-word-upper",		upperword},
	{"change-file-name",		filename},
	{"change-screen-size",		newsize},
	{"change-screen-width",		newwidth},
#if	DECEDT
	{"char-to-octal-insert",	octalins},
#endif
	{"clear-and-redraw",		refresh},
#if	DECEDT
	{"clear-mark",			remmark},
#endif
	{"clear-message-line",		me_clrmes},
#if	DISPSEVEN | DECEDT
	{"compose-acute",		me_add_acute},
	{"compose-bar",			me_add_bar},
	{"compose-cedilla",		me_add_cedilla},
	{"compose-character",		me_compose},
	{"compose-circ",		me_add_circ},
	{"compose-grave",		me_add_grave},
	{"compose-slash",		me_add_slash},
	{"compose-tilde",		me_add_tilde},
	{"compose-umlaut",		me_add_umlaut},
	{"compose-ring",		me_add_ring},
#endif
	{"copy-region",			copyregion},
#if	WORDPRO
	{"count-words",			wordcount},
#endif
#if	DECEDT
	{"cut-region",			cutregion},
#endif
	{"ctlx-prefix",			cex},
	{"delete-blank-lines",		deblank},
	{"delete-buffer",		killbuffer},
	{"delete-global-mode",		delgmode},
	{"delete-mode",			delmode},
	{"delete-next-character",	forwdel},
#if	DECEDT
	{"delete-next-line",		delfline},
#endif
	{"delete-next-word",		delfword},
	{"delete-other-windows",	onlywind},
	{"delete-previous-character",	backdel},
	{"delete-previous-word",	delbword},
#if	DECEDT
	{"delete-to-kill-buffer",	killregion},
#endif
	{"delete-window",		delwind},
	{"describe-bindings",		desbind},
#if	DEBUGM
	{"describe-functions",		desfunc},
#endif
	{"describe-key",		deskey},
#if	DEBUGM
	{"describe-variables",		desvars},
#endif
#if	AEDIT
	{"detab-line",			detab},
#endif
#if	DEBUGM
	{"display",			dispvar},
#endif
	{"end-macro",			ctlxrp},
	{"end-of-file",			goeob},
	{"end-of-line",			goeol},
#if	AEDIT
	{"entab-line",			entab},
#endif
#if	DECEDT
	{"enter",			nullcmd},
#endif
	{"exchange-point-and-mark",	swapmark},
	{"execute-buffer",		execbuf},
	{"execute-command-line",	execcmd},
	{"execute-file",		execfile},
	{"execute-macro",		ctlxe},
	{"execute-macro-1",		cbuf1},
	{"execute-macro-2",		cbuf2},
	{"execute-macro-3",		cbuf3},
	{"execute-macro-4",		cbuf4},
	{"execute-macro-5",		cbuf5},
	{"execute-macro-6",		cbuf6},
	{"execute-macro-7",		cbuf7},
	{"execute-macro-8",		cbuf8},
	{"execute-macro-9",		cbuf9},
	{"execute-macro-10",		cbuf10},
	{"execute-macro-11",		cbuf11},
	{"execute-macro-12",		cbuf12},
	{"execute-macro-13",		cbuf13},
	{"execute-macro-14",		cbuf14},
	{"execute-macro-15",		cbuf15},
	{"execute-macro-16",		cbuf16},
	{"execute-macro-17",		cbuf17},
	{"execute-macro-18",		cbuf18},
	{"execute-macro-19",		cbuf19},
	{"execute-macro-20",		cbuf20},
	{"execute-macro-21",		cbuf21},
	{"execute-macro-22",		cbuf22},
	{"execute-macro-23",		cbuf23},
	{"execute-macro-24",		cbuf24},
	{"execute-macro-25",		cbuf25},
	{"execute-macro-26",		cbuf26},
	{"execute-macro-27",		cbuf27},
	{"execute-macro-28",		cbuf28},
	{"execute-macro-29",		cbuf29},
	{"execute-macro-30",		cbuf30},
	{"execute-macro-31",		cbuf31},
	{"execute-macro-32",		cbuf32},
	{"execute-macro-33",		cbuf33},
	{"execute-macro-34",		cbuf34},
	{"execute-macro-35",		cbuf35},
	{"execute-macro-36",		cbuf36},
	{"execute-macro-37",		cbuf37},
	{"execute-macro-38",		cbuf38},
	{"execute-macro-39",		cbuf39},
	{"execute-macro-40",		cbuf40},
        {"execute-macro-41",            cbuf41},
        {"execute-macro-42",            cbuf42},
        {"execute-macro-43",            cbuf43},
        {"execute-macro-44",            cbuf44},
        {"execute-macro-45",            cbuf45},
        {"execute-macro-46",            cbuf46},
        {"execute-macro-47",            cbuf47},
        {"execute-macro-48",            cbuf48},
        {"execute-macro-49",            cbuf49},
        {"execute-macro-50",            cbuf50},
        {"execute-macro-51",            cbuf51},
        {"execute-macro-52",            cbuf52},
        {"execute-macro-53",            cbuf53},
        {"execute-macro-54",            cbuf54},
        {"execute-macro-55",            cbuf55},
        {"execute-macro-56",            cbuf56},
        {"execute-macro-57",            cbuf57},
        {"execute-macro-58",            cbuf58},
        {"execute-macro-59",            cbuf59},
        {"execute-macro-60",            cbuf60},
        {"execute-macro-61",            cbuf61},
        {"execute-macro-62",            cbuf62},
        {"execute-macro-63",            cbuf63},
        {"execute-macro-64",            cbuf64},
        {"execute-macro-65",            cbuf65},
        {"execute-macro-66",            cbuf66},
        {"execute-macro-67",            cbuf67},
        {"execute-macro-68",            cbuf68},
        {"execute-macro-69",            cbuf69},
        {"execute-macro-70",            cbuf70},
        {"execute-macro-71",            cbuf71},
        {"execute-macro-72",            cbuf72},
        {"execute-macro-73",            cbuf73},
        {"execute-macro-74",            cbuf74},
        {"execute-macro-75",            cbuf75},
        {"execute-macro-76",            cbuf76},
        {"execute-macro-77",            cbuf77},
        {"execute-macro-78",            cbuf78},
        {"execute-macro-79",            cbuf79},
        {"execute-macro-80",            cbuf80},
        {"execute-macro-81",            cbuf81},
        {"execute-macro-82",            cbuf82},
        {"execute-macro-83",            cbuf83},
        {"execute-macro-84",            cbuf84},
        {"execute-macro-85",            cbuf85},
        {"execute-macro-86",            cbuf86},
        {"execute-macro-87",            cbuf87},
        {"execute-macro-88",            cbuf88},
        {"execute-macro-89",            cbuf89},
        {"execute-macro-90",            cbuf90},
        {"execute-macro-91",            cbuf91},
        {"execute-macro-92",            cbuf92},
        {"execute-macro-93",            cbuf93},
        {"execute-macro-94",            cbuf94},
        {"execute-macro-95",            cbuf95},
        {"execute-macro-96",            cbuf96},
        {"execute-macro-97",            cbuf97},
        {"execute-macro-98",            cbuf98},
        {"execute-macro-99",            cbuf99},
	{"execute-named-command",	namedcmd},
#if	PROC
	{"execute-procedure",		execproc},
#endif
#if	DECEDT
	{"exit",			quickexit},
#endif
	{"exit-emacs",			quit},
#if	WORDPRO
	{"fill-paragraph",		fillpara},
#endif
	{"filter-buffer",		filter},
	{"filter-new-buffer",		filtnew},
	{"find-file",			filefind},
#if	DECEDT
	{"find-file-into-other-window",	filewfind},
	{"find-buffer-into-other-window", bufffind},
#endif
	{"forward-character",		forwchar},
	{"goto-line",			gotoline},
#if	CFENCE
	{"goto-matching-fence",		getfence},
#endif
	{"grow-window",			enlargewind},
	{"handle-tab",			tab},
	{"hunt-forward",		forwhunt},
	{"hunt-backward",		backhunt},
	{"help",			me_help},
	{"i-shell",			spawncli},
#if	DECEDT
	{"include",			insfile},
#endif
#if	ISRCH
	{"incremental-search",		fisearch},
#endif
	{"insert-file",			insfile},
	{"insert-space",		insspace},
	{"insert-string",		istring},
#if	WORDPRO & DECEDT
	{"justify-paragraph",		fillpara},
#endif
#if	WORDPRO
	{"kill-paragraph",		killpara},
#endif
	{"kill-region",			killregion},
	{"kill-to-end-of-line",		killtext},
#if	FLABEL
	{"label-function-key",		fnclabel},
#endif
	{"list-buffers",		listbuffers},
#if	MENUS
	{"menu",			menu},
#endif
	{"meta-prefix",			meta},
	{"move-window-down",		mvdnwind},
	{"move-window-up",		mvupwind},
	{"name-buffer",			namebuffer},
	{"narrow-to-region",		narrow},
	{"newline",			newline},
	{"newline-and-indent",		indent},
	{"next-buffer",			nextbuffer},
	{"next-line",			forwline},
	{"next-page",			forwpage},
#if	WORDPRO
	{"next-paragraph",		goeop},
#endif
	{"next-window",			nextwind},
	{"next-word",			forwword},
	{"nop",				nullproc},
	{"open-line",			openline},
#if	DECEDT
	{"over-write",			setovrwrt},
#endif
	{"pipe-command",		pipecmd},
	{"pop-buffer",			popbuffer},
	{"previous-buffer",		prevbuffer},
	{"previous-line",		backline},
	{"previous-page",		backpage},
#if	WORDPRO
	{"previous-paragraph",		gobop},
#endif
	{"previous-window",		prevwind},
	{"previous-word",		backword},
	{"query-replace-string",	qreplace},
	{"quick-exit",			quickexit},
	{"quick-save",			quicksave},
#if	DECEDT
	{"quit",			quit},
#endif
	{"quote-character",		quote},
	{"read-file",			fileread},
#if	ABACKUP
	{"recover-file",		filercover},
#endif
	{"redraw-display",		reposition},
	{"remove-mark",			remmark},
	{"resize-window",		resize},
	{"restore-window",		restwnd},
	{"replace-string",		sreplace},
#if	ISRCH
	{"reverse-incremental-search",	risearch},
#endif
#if	DECEDT
	{"revert-file",			filerevert},
#endif
#if	PROC
	{"run",				execproc},
#endif
	{"save-file",			filesave},
	{"save-window",			savewnd},
#if	AEDIT
	{"scroll-backward",		scrlback},
	{"scroll-forward",		scrlforw},
#endif
	{"scroll-next-down",		scrdwnxt},
	{"scroll-next-up",		scrupnxt},
	{"search-forward",		forwsearch},
	{"search-reverse",		backsearch},
	{"select-buffer",		usebuffer},
	{"set",				setvar},
#if	CRYPT
	{"set-encryption-key",		me_setkey},
#endif
	{"set-fill-column",		setfillcol},
	{"set-mark",			setmark},
#if	DECEDT
	{"set-screen-width",		newwidth},
#endif
	{"shell-command",		spawncmd},
	{"show-files",			showfiles},
	{"show-modes",			showmodes},
	{"shrink-window",		shrinkwind},
#if	DECEDT
	{"smooth-next-line",		scdnwind},
	{"smooth-next-page",		scnxpage},
	{"smooth-previous-line",	scupwind},
	{"smooth-previous-page",	scprpage},
	{"source",			execfile},
	{"special-insertion",		specins},
	{"special-prefix", 		special},
#endif
	{"split-current-window",	splitwind},
	{"store-macro",			storemac},
#if	PROC
	{"store-procedure",		storeproc},
#endif
#if	DECEDT
	{"string-length",		showslen},
#endif
#if	BSD | V7 | USG | VMS
	{"suspend-emacs",		bktoshell},
#endif
	{"transpose-characters",	twiddle},
#if	AEDIT
	{"trim-line",			trim},
#endif
	{"unbind-key",			unbindkey},
	{"universal-argument",		unarg},
	{"unmark-buffer",		unmark},
	{"update-screen",		upscreen},
#if	DISPSEVEN | DECEDT
	{"unprefix",			me_unprefix},
	{"unquote-printable",		me_unquotep},
#endif
	{"view-file",			viewfile},
#if	DECEDT
	{"view-file-into-other-window",	filevfind},
#endif
	{"widen-from-region",		widen},
	{"wrap-word",			wrapword},
	{"write-file",			filewrite},
	{"write-message",		writemsg},
#if	DECEDT
	{"write-region",		regionwrite},
#endif
	{"yank",			yank},
#if	DECEDT
	{"yank-character",		yankchar},
#endif

	{"",			NULL}
};

#endif
