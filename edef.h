/*	EDEF:		Global variable definitions for
			MicroEMACS 3.9

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
*/

#ifndef _EDEF_H_
#define	_EDEF_H_

/* handle constant and voids properly */

#define	SRNORM	0			/* end past, begin front	*/
#define	SRBEGIN	1			/* always at front		*/
#define	SREND	2			/* always one past end		*/


/*	Completion types		*/

#define	CMP_NONE	0
#define	CMP_BUFFER	1
#define	CMP_COMMAND	2
#define	CMP_FILENAME	3
#define	CMP_VARIABLE	4
#define	CMP_MODE	5

/*	External function declarations		*/

#ifdef NO_PROTOTYPEx

extern char *flook();
extern char *getctext();
extern CONSTA char *getval();
extern char *me_itoa();
extern char *bytecopy();
extern char *token();
extern CONSTA char *transbind();
extern BUFFER *getcbuf();
extern char *gtfilename();
extern char *gtmodename();
extern char *complete();
#if	BCOMPL
extern char *getffile();
extern char *getnfile();
#endif
extern BUFFER	*bfind();	/* Lookup a buffer by name	*/
extern LINE	*lalloc();	/* Allocate a line		*/

#else

	/* from basic.c */

extern int backchar PP((int f, int n));	/* Move backward by characters  */
extern int forwchar PP((int f, int n));	/* Move forward by characters   */
extern int gobol PP((int f, int n));	/* Move to start of line        */
extern int goeol PP((int f, int n));	/* Move to end of line          */
extern int backline PP((int f, int n));	/* Move backward by lines       */
extern int forwline PP((int f, int n));	/* Move forward by lines        */
extern int getgoal PP((LINE *dlp));
extern int backpage PP((int f, int n));	/* Move backward by pages       */
extern int forwpage PP((int f, int n));	/* Move forward by pages        */
extern int setmark PP((int f, int n));	/* Set mark                     */
extern int remmark PP((int f, int n));	/* clear mark                   */
extern int swapmark PP((int f, int n));	/* Swap "." and mark            */
extern int gotoline PP((int f, int n));	/* go to a numbered line	*/
extern int gobob PP((int f, int n));	/* Move to start of buffer      */
extern int goeob PP((int f, int n));	/* Move to end of buffer        */
#if	WORDPRO
extern int gobop PP((int f, int n));	/* go to beginning/paragraph	*/
extern int goeop PP((int f, int n));	/* go to end/paragraph		*/
#endif
#if	DECEDT
extern int advbot PP((int f, int n));		/* goto top or bottom of buffer */
extern int advchar PP((int f, int n));		/* advance character */
extern int advhunt PP((int f, int n));		/* hunt */
extern int advline PP((int f, int n));		/* advance line */
extern int adveol PP((int f, int n));		/* advance end of line */
extern int advpage PP((int f, int n));		/* advance page */
extern int advsearch PP((int f, int n));	/* search */
extern int advword PP((int f, int n));		/* advance word */
extern int advadv PP((int f, int n));		/* set advance to go forward */
extern int advback PP((int f, int n));		/* set advance to go backward */
extern int nullcmd PP((int f, int n));		/* null enter command */
extern int specins PP((int f, int n));		/* special insertion */
extern int yankchar PP((int f, int n));		/* Undelete last deleted char */
extern int caseregion PP((int f, int n));	/* change case */
extern int regionwrite PP((int f, int n));	/* write a region to a file */
extern int regionappend PP((int f, int n));	/* append a region to a file */
extern int cutregion PP((int f, int n));	/* Kill region, remove mark */
extern int apkillregion PP((int f, int n));	/* Kill region, add to kill buffer */
extern int apcopyregion PP((int f, int n));	/* Append region to kill buffer */
extern int octalins PP((int f, int n));		/* Octal insertion */
extern int showslen PP((int f, int n));		/* Show string length */
extern int delfline PP((int f, int n));		/* Delete forward line */
extern int setovrwrt PP((int f, int n));	/* toggle overwrite mode */
extern int scdnwind PP((int f, int n));		/* smooth scroll cursor down */
extern int scupwind PP((int f, int n));		/* smooth scroll cursor up */
extern int scnxpage PP((int f, int n));		/* smooth scroll next page */
extern int scprpage PP((int f, int n));		/* smooth scroll previous page */
#endif

	/* from bind.c */

extern int me_help PP((int f, int n));		/* get the help file here	*/
extern int deskey PP((int f, int n));		/* describe a key's binding	*/
extern int bindtokey PP((int f, int n));	/* bind a function to a key	*/
extern int unbindkey PP((int f, int n));	/* unbind a key's function	*/
extern int desbind PP((int f, int n));		/* describe bindings		*/
#if APROP
extern int apro PP((int f, int n));		/* apropos fuction		*/
extern int buildlist PP((int type, CONSTA char *mstring));
#endif
extern int startup PP((CONSTA char *sfname));
extern CONSTA char *flook PP((CONSTA char *fname, int hflag));
extern VOID cmdstr PP((int c, char *seq));
#if C86 | DECUSC | DARWIN
extern int *getbind PP((int c));
extern int *fncmatch PP((char *fname));
#else
extern int (*getbind PP((int c))) (int f, int n);
extern int (*fncmatch PP((char *fname))) (int f, int n);
#endif
extern CONSTA char *transbind PP((char *skey));
extern VOID fkeyinit PP((void));
extern int chrcompare PP((int a, int b));
extern int strcompare PP((CONSTA char *a, CONSTA char *b, int len));

	/* from buffer.c */

extern BUFFER *getdefb PP((void));
extern int usebuffer PP((int f, int n));	/* Switch a window to a buffer  */
extern int nextbuffer PP((int f, int n));	/* switch to the next buffer	*/
extern int prevbuffer PP((int f, int n));	/* switch to the prev bufffer	*/
extern int swbuffer PP((BUFFER *bp));
extern int killbuffer PP((int f, int n));	/* Make a buffer go away.       */
extern int PASCAL NEAR popbuffer PP((int f, int n)); /* pop-up a buffer		*/
extern int zotbuf PP((BUFFER *bp));
extern int namebuffer PP((int f, int n));	/* rename the current buffer	*/
extern int listbuffers PP((int f, int n));	/* Display list of buffers     */
extern int reposition PP((int f, int n));	/* Reposition window            */
extern int makelist PP((CONSTA char *name, int iflag));
extern int addline PP((BUFFER *bp, CONSTA char *text));
extern int anycb PP((void));
extern BUFFER *bfind PP((CONSTA char *bname, int cflag, int bflag));
extern int bclear PP((BUFFER *bp));
extern int unmark PP((int f, int n));		/* unmark current buffer	*/
#if	BCOMPL
extern int comp_test PP((char *name, int namelen, int *cpos, int *matchlen,
		CONSTA char *testname, int exact));
#endif
#if	CALLABLE
extern VOID frallbuffers PP((void));	/* free all buffers */
#endif

	/* from crypt.c */

#if CRYPT
extern int me_setkey PP((int f, int n));	/* set encryption key		*/
extern VOID me_crypt PP((register char *bptr, register unsigned len));
#endif

	/* from eval.c */

extern VOID varinit PP((void));
#if BCOMPL
extern VOID comp_variable PP((char *name, int *cpos, int *matchlen));
#endif
extern char *getkill PP((void));
extern int setvar PP((int f, int n));		/* set a variables value */
extern VOID findvar PP((char *var, VDESC *vd, int size));
extern int svar PP((VDESC *var, CONSTA char *value));
extern int asc_int PP((CONSTA char *st));
extern char *me_itoa PP((int i));
extern int gettyp PP((char *tok));
extern CONSTA char *getval PP((char *tok, char buf[NSTRING]));
extern int stol PP((CONSTA char *val));
extern char *mklower PP((char *str));
extern int spal PP((char *pal));
#if DEBUGM
extern int dispvar PP((int f, int n));		/* display a variable */
extern int desvars PP((int f, int n));		/* describe variables */
extern int varlist PP((CONSTA char *mstring));
extern int desfunc PP((int f, int n));		/* describe functions */
#endif

	/* from exec.c */

extern int namedcmd PP((int f, int n));		/* execute named command	*/
extern int execcmd PP((int f, int n));		/* execute a command line	*/
extern char *token PP((char *src, char *tok, int size));
extern int macarg PP((char *tok));
extern int nextarg PP((CONSTA char *prompt, char *buffer, int size, int terminator));
#if	PROC
extern int storemac PP((int f, int n));		/* store text for macro		*/
extern int storeproc PP((int f, int n));	/* store names procedure */
#endif
extern int execproc PP((int f, int n));		/* execute procedure */
extern int execbuf PP((int f, int n));		/* exec commands from a buffer	*/
extern VOID makelit PP((char *s, int len));
extern int execfile PP((int f, int n));		/* exec commands from a file	*/
extern int dofile PP((CONSTA char *name));
extern int cbuf1 PP((int f, int n));		/* execute numbered comd buffer */
extern int cbuf2 PP((int f, int n));
extern int cbuf3 PP((int f, int n));
extern int cbuf4 PP((int f, int n));
extern int cbuf5 PP((int f, int n));
extern int cbuf6 PP((int f, int n));
extern int cbuf7 PP((int f, int n));
extern int cbuf8 PP((int f, int n));
extern int cbuf9 PP((int f, int n));
extern int cbuf10 PP((int f, int n));
extern int cbuf11 PP((int f, int n));
extern int cbuf12 PP((int f, int n));
extern int cbuf13 PP((int f, int n));
extern int cbuf14 PP((int f, int n));
extern int cbuf15 PP((int f, int n));
extern int cbuf16 PP((int f, int n));
extern int cbuf17 PP((int f, int n));
extern int cbuf18 PP((int f, int n));
extern int cbuf19 PP((int f, int n));
extern int cbuf20 PP((int f, int n));
extern int cbuf21 PP((int f, int n));
extern int cbuf22 PP((int f, int n));
extern int cbuf23 PP((int f, int n));
extern int cbuf24 PP((int f, int n));
extern int cbuf25 PP((int f, int n));
extern int cbuf26 PP((int f, int n));
extern int cbuf27 PP((int f, int n));
extern int cbuf28 PP((int f, int n));
extern int cbuf29 PP((int f, int n));
extern int cbuf30 PP((int f, int n));
extern int cbuf31 PP((int f, int n));
extern int cbuf32 PP((int f, int n));
extern int cbuf33 PP((int f, int n));
extern int cbuf34 PP((int f, int n));
extern int cbuf35 PP((int f, int n));
extern int cbuf36 PP((int f, int n));
extern int cbuf37 PP((int f, int n));
extern int cbuf38 PP((int f, int n));
extern int cbuf39 PP((int f, int n));
extern int cbuf40 PP((int f, int n));
extern int cbuf41 PP((int f, int n));
extern int cbuf42 PP((int f, int n));
extern int cbuf43 PP((int f, int n));
extern int cbuf44 PP((int f, int n));
extern int cbuf45 PP((int f, int n));
extern int cbuf46 PP((int f, int n));
extern int cbuf47 PP((int f, int n));
extern int cbuf48 PP((int f, int n));
extern int cbuf49 PP((int f, int n));
extern int cbuf50 PP((int f, int n));
extern int cbuf51 PP((int f, int n));
extern int cbuf52 PP((int f, int n));
extern int cbuf53 PP((int f, int n));
extern int cbuf54 PP((int f, int n));
extern int cbuf55 PP((int f, int n));
extern int cbuf56 PP((int f, int n));
extern int cbuf57 PP((int f, int n));
extern int cbuf58 PP((int f, int n));
extern int cbuf59 PP((int f, int n));
extern int cbuf60 PP((int f, int n));
extern int cbuf61 PP((int f, int n));
extern int cbuf62 PP((int f, int n));
extern int cbuf63 PP((int f, int n));
extern int cbuf64 PP((int f, int n));
extern int cbuf65 PP((int f, int n));
extern int cbuf66 PP((int f, int n));
extern int cbuf67 PP((int f, int n));
extern int cbuf68 PP((int f, int n));
extern int cbuf69 PP((int f, int n));
extern int cbuf70 PP((int f, int n));
extern int cbuf71 PP((int f, int n));
extern int cbuf72 PP((int f, int n));
extern int cbuf73 PP((int f, int n));
extern int cbuf74 PP((int f, int n));
extern int cbuf75 PP((int f, int n));
extern int cbuf76 PP((int f, int n));
extern int cbuf77 PP((int f, int n));
extern int cbuf78 PP((int f, int n));
extern int cbuf79 PP((int f, int n));
extern int cbuf80 PP((int f, int n));
extern int cbuf81 PP((int f, int n));
extern int cbuf82 PP((int f, int n));
extern int cbuf83 PP((int f, int n));
extern int cbuf84 PP((int f, int n));
extern int cbuf85 PP((int f, int n));
extern int cbuf86 PP((int f, int n));
extern int cbuf87 PP((int f, int n));
extern int cbuf88 PP((int f, int n));
extern int cbuf89 PP((int f, int n));
extern int cbuf90 PP((int f, int n));
extern int cbuf91 PP((int f, int n));
extern int cbuf92 PP((int f, int n));
extern int cbuf93 PP((int f, int n));
extern int cbuf94 PP((int f, int n));
extern int cbuf95 PP((int f, int n));
extern int cbuf96 PP((int f, int n));
extern int cbuf97 PP((int f, int n));
extern int cbuf98 PP((int f, int n));
extern int cbuf99 PP((int f, int n));

	/* from display.c */

extern VOID vtinit PP((void));
extern VOID vttidy PP((void));
extern int upscreen PP((int f, int n));		/* force screen update		*/
extern int update PP((int force));
extern int PASCAL NEAR pop PP((BUFFER *popbufp));
extern VOID upmode PP((void));
extern VOID upwind PP((void));
extern VOID movecursor PP((int row, int col));
extern VOID mlerase PP((void));
#if defined(_AIX) || defined(__BORLANDC__) || (defined(vms) && defined(vax11c))
extern VOID mlwrite();
#else
extern VOID mlwrite PP((CONSTA char *fmt, ...));
#endif
extern VOID mlforce PP((CONSTA char *s));
extern VOID mlputs PP((CONSTA char *s));

	/* from file.c */

extern int fileread PP((int f, int n));	/* Get a file, read only        */
extern int insfile PP((int f, int n));	/* insert a file		*/
extern int filefind PP((int f, int n));	/* Get a file, read write       */
#if DECEDT
extern int filewfind PP((int f, int n));	/* Find file into other window */
extern int filevfind PP((int f, int n));	/* View file into other window */
extern int bufffind PP((int f, int n));		/* FInd buffer into other window */
extern int filerevert PP((int f, int n));	/* Restore last saved version	*/
#endif
extern int viewfile PP((int f, int n));	/* find a file in view mode	*/
#if CRYPT
extern int resetkey PP((void));
#endif
extern int getfile PP((CONSTA char *fname, int lockfl));
extern int readin PP((CONSTA char *fname, int lockfl, int recover));
extern VOID makename PP((char *bname, CONSTA char *fname));
extern VOID unqname PP((char *name));
extern int filewrite PP((int f, int n));	/* Write a file                 */
extern int PASCAL NEAR fileapp PP((int f, int n));	/* Append a file		*/
extern int filelist PP((char *mstring));
extern int filesave PP((int f, int n));		/* Save current file            */
extern int writeout PP((CONSTA char *fn, CONSTA char *mode));
extern int filename PP((int f, int n));		/* Adjust file name             */
#if ACMODE
extern int acmode PP((int f, int n));		/* set into C mode for .c and .h */
#endif
extern VOID makeasvname PP((char *tempname, char *fname));
extern int filercover PP((int f, int n));	/* Get a saved file		*/
extern int fileback PP((int f, int n));		/* Autosaves the file	*/
extern int filebkall PP((int f, int n));	/* Autosave all files	*/

	/* from fileio.c */

extern int ffropen PP((CONSTA char *fn, int getstat));
extern int ffwopen PP((CONSTA char *fn, CONSTA char *mode));
extern int ffrclose PP((void));
extern int ffwclose PP((void));
extern int ffputline PP((char *buf, int nbuf));
extern int ffgetline PP((void));
extern int fexist PP((char *fname));
#if ABACKUP
extern int ffisnewer PP((char *fn));
#endif
extern char *getffile PP((CONSTA char *fspec, int match_len, int exact, int is_arg));
extern char *getnfile PP((CONSTA char *match_name, int match_len, int exact));
extern VOID PASCAL NEAR expandargs PP((int *pargs, char ***pargv));

	/* from input.c */

extern int mlyesno PP((CONSTA char *prompt));
extern int mlreply PP((CONSTA char *prompt, char *buf, int nbuf));
extern int mltreply PP((CONSTA char *prompt, char *buf, int nbuf, int eolchar));
extern int ectoc PP((int c));
extern int ctoec PP((int c));
extern char *complete PP((CONSTA char *prompt, CONSTA char *defval, int type,
		char *buf, int maxlen, int eolchar, int needeol, int *status));
#if DARWIN
extern int *getname PP((CONSTA char *prompt));
#else
extern int (*getname PP((CONSTA char *prompt))) (int f, int n);
#endif
extern BUFFER *getcbuf PP((CONSTA char *prompt, CONSTA char *defval,
		int createflag));
extern char *gtfilename PP((CONSTA char *prompt, int needeol));
extern char *gtmodename PP((CONSTA char *prompt));
extern int tgetc PP((void));
extern int get1key PP((void));
extern int getcmd PP((void));
extern int getstring PP((CONSTA char *prompt, char *buf, int nbuf, int eolchar));
extern VOID outstring PP((CONSTA char *s));

	/* from isearch.c */

#if	ISRCH
extern int fisearch PP((int f, int n));		/* forward incremental search	*/
extern int risearch PP((int f, int n));		/* reverse incremental search	*/
#endif

	/* from meline.c */

extern LINE *lalloc PP((int used));
extern VOID lfree PP((LINE *lp));
extern VOID lchange PP((int flag));
extern int insspace PP((int f, int n));	/* insert a space forword	*/
extern int linstr PP((char *instr));
extern int linsert PP((int n, int c));
extern int lnewline PP((void));
extern int ldelete PP((long n, int kflag));
extern char *getctext PP((char *line, int maxlen));
extern int putctext PP((CONSTA char *iline));
extern VOID kdelete PP((void));
extern int kinsert PP((int c));
extern int yank PP((int f, int n));		/* Yank back from killbuffer.   */

	/* from main.c */

extern int meta PP((int f, int n));		/* meta prefix dummy function	*/
extern int cex PP((int f, int n));		/* ^X prefix dummy function	*/
#if DECEDT
extern int special PP((int f, int n));		/* special prefix dummy function */
#endif
extern int unarg PP((int f, int n));		/* ^U repeat arg dummy function	*/
extern int execute PP((int c, int f, int n));
extern int quicksave PP((int f, int n));	/* save all modified files	*/
extern int quickexit PP((int f, int n));	/* low keystroke style exit.    */
extern int quit PP((int f, int n));		/* Quit                         */
extern int ctlxlp PP((int f, int n));		/* Begin macro                  */
extern int ctlxrp PP((int f, int n));		/* End macro                    */
extern int ctlxe PP((int f, int n));		/* Execute macro                */
extern int ctrlg PP((int f, int n));		/* Abort out of things          */
extern int rdonly PP((void));
extern int resterr PP((void));
extern int nullproc PP((int f, int n));		/* does nothing... */
extern char *bytecopy PP((char *dst, CONSTA char *src, int maxlen));
#if	CALLABLE
extern int emacsmain PP((int argc, char *argv[]));
extern int emacscmd PP((char *line, int n));
extern int emacsone PP((char *name));
extern int emacspgm PP((int argc, char *argv[]));
#endif
#if	WINMOD
extern int wmodinit PP((int argc, char *argv[]));
#endif

	/* from menu.c */

#if	MENUS
extern int menu PP((int f, int n));		/* enter menu area */
#endif

	/* from random.c */

extern int modelist PP((CONSTA char *mstring));
extern int PASCAL NEAR getlinenum PP((BUFFER *bp, LINE *sline));
extern int forwdel PP((int f, int n));		/* Forward delete               */
extern int backdel PP((int f, int n));		/* Backward delete              */
extern int getccol PP((int bflg));		/* Get current column           */
extern int setccol PP((int pos));
extern int getcline PP((void));
extern int insbrace PP((int n, int c));
extern int inspound PP((void));
extern int fmatch PP((int ch));
extern int killtext PP((int f, int n));		/* Kill forward                 */
extern int setfillcol PP((int f, int n));	/* Set fill column.             */
extern int showcpos PP((int f, int n));		/* Show the cursor position     */
extern int PASCAL NEAR showfiles PP((int f, int n)); /* Show files in a directory	*/
extern int PASCAL NEAR showmodes PP((int f, int n)); /* Show files list of modes	*/
extern int twiddle PP((int f, int n));		/* Twiddle characters           */
extern int tab PP((int f, int n));		/* Insert tab                   */
extern int newline PP((int f, int n));		/* Insert CR-LF                 */
extern int indent PP((int f, int n));		/* Insert CR-LF, then indent    */
extern int openline PP((int f, int n));		/* Open up a blank line         */
extern int deblank PP((int f, int n));		/* Delete blank lines           */
extern int quote PP((int f, int n));		/* Insert literal               */
extern int me_setmode PP((int f, int n));	/* set an editor mode		*/
extern int delmode PP((int f, int n));		/* delete a mode		*/
extern int setgmode PP((int f, int n));		/* set a global mode		*/
extern int delgmode PP((int f, int n));		/* delete a global mode		*/
extern int writemsg PP((int f, int n));		/* write text on message line	*/
extern int me_clrmes PP((int f, int n));	/* clear the message line	*/
extern int istring PP((int f, int n));		/* insert string in text	*/
#if	CFENCE
extern int getfence PP((int f, int n));		/* move cursor to a matching fence */
#endif
#if	AEDIT
extern int trim PP((int f, int n));		/* trim whitespace from end of line */
extern int detab PP((int f, int n));		/* detab rest of line */
extern int entab PP((int f, int n));		/* entab rest of line */
#endif
#if DISPSEVEN | DECEDT
extern int me_compose PP((int f, int n));	/* Compose char			*/
extern int me_unquotep PP((int f, int n));	/* Unquote char			*/
extern int me_unprefix PP((int f, int n));	/* Unprefix char		*/
extern int me_add_acute PP((int f, int n));	/* Add acute accent		*/
extern int me_add_bar PP((int f, int n));	/* Add bar accent		*/
extern int me_add_cedilla PP((int f, int n));	/* Add bar cedilla		*/
extern int me_add_circ PP((int f, int n));	/* Add circumflex accent	*/
extern int me_add_grave PP((int f, int n));	/* Add grave accent		*/
extern int me_add_slash PP((int f, int n));	/* Add slash accent		*/
extern int me_add_tilde PP((int f, int n));	/* Add tilde accent		*/
extern int me_add_umlaut PP((int f, int n));	/* Add umlaut accent		*/
extern int me_add_ring PP((int f, int n));	/* Add ring accent		*/
#endif

	/* from region.c */

extern int killregion PP((int f, int n));	/* Kill region.                 */
extern int copyregion PP((int f, int n)); 	/* Copy region to kill buffer.  */
extern int getregion PP((REGION *rp));
extern int upperregion PP((int f, int n));	/* Upper case region.          */
extern int lowerregion PP((int f, int n));	/* Lower case region.          */
extern int narrow PP((int f, int n));		/* narrow to region */
extern int widen PP((int f, int n));		/* widen from narrowed region */

	/* from search.c */

extern VOID setjtable PP((void));
extern VOID mcclear PP((void));
extern int scanner PP((char *patrn, int direct, int beg_or_end));	/* Handy search routine */
extern VOID rvstrcpy PP((char *rvstr, char *str));
extern int expandp PP((char *srcstr, char *deststr, int maxlength));
extern int forwsearch PP((int f, int n));	/* Search forward               */
extern int backsearch PP((int f, int n));	/* Search backwards             */
extern int forwhunt PP((int f, int n));		/* hunt forward for next match	*/
extern int backhunt PP((int f, int n));		/* hunt backwards for next match*/
extern int me_eq PP((int bc, int pc));		/* Compare chars, match case	*/
extern int sreplace PP((int f, int n));		/* search and replace		*/
extern int qreplace PP((int f, int n));		/* search and replace w/query	*/

	/* from spawn.c */

extern int spawncli PP((int f, int n));		/* Run CLI in a subjob.         */
extern int spawncmd PP((int f, int n));		/* Run a command in a subjob.   */
extern int pipecmd PP((int f, int n));		/* pipe command into buffer	*/
extern int me_filter PP((int f, int n));	/* filter buffer through dos	*/
extern int filtnew PP((int f, int n));		/* filter buffer into new buffer*/

	/* from termio.c */

extern int ttopen PP((void));
extern int ttgetc PP((void));
extern int ttflush PP((void));
extern int ttclose PP((void));
#if     AMIGA | (ST520 & MEGAMAX)
extern int ttputc PP((char c));
#else
extern int ttputc PP((int c));
#endif

#if	VMSVT | TERMCAP | WINMOD
extern VOID ttsetwid PP((int n));
extern VOID ttscroll PP((int rowa, int rowb, int lines));
#endif
#if	TYPEAH
extern int typahead PP((void));
#endif
#if	TERMCAP
extern VOID getwinsize PP((short *ncols, short *nrows));
#endif
#if	BSD | V7 | USG | VMS
extern int bktoshell PP((int f, int n));	/* suspend emacs to parent shell*/
#endif

#if	FLABEL
extern int fnclabel PP((int f, int n));	/* set function key label	*/
#endif

	/* from window.c */

extern int getwpos PP((void));
extern int PASCAL NEAR getcwnum PP((void));
extern int PASCAL NEAR gettwnum PP((void));
extern int newsize PP((int f, int n));		/* change the current screen size */
extern int newwidth PP((int f, int n));		/* change the current screen width */
extern int resize PP((int f, int n));		/* resize current window	*/
extern int splitwind PP((int f, int n));	/* Split current window         */
extern int onlywind PP((int f, int n));
extern int mvdnwind PP((int f, int n));		/* Move window down             */
extern int mvupwind PP((int f, int n));		/* Move window up               */
extern int enlargewind PP((int f, int n));	/* Enlarge display window.     */
extern int shrinkwind PP((int f, int n));	/* Shrink window.               */
extern int nextwind PP((int f, int n));		/* Move to the next window      */
extern int prevwind PP((int f, int n));		/* Move to the previous window  */
extern int delwind PP((int f, int n));		/* delete the current window	*/
extern int me_refresh PP((int f, int n));	/* Refresh the screen           */
extern int scrupnxt PP((int f, int n));		/* scroll next window back	*/
extern int scrdwnxt PP((int f, int n));		/* scroll next window down	*/
extern int savewnd PP((int f, int n));		/* save current window		*/
extern int restwnd PP((int f, int n));		/* restore current window	*/
extern int wpopup PP((BUFFER *popbufp));	/* Pop up window creation	*/
#if	AEDIT
extern int scrlforw PP((int f, int n));		/* scroll forward */
extern int scrlback PP((int f, int n));		/* scroll backward */
#endif
#if	CALLABLE
extern VOID frallwindows PP((void));
#endif

	/* from word.c */

extern int inword PP((void));
extern int forwword PP((int f, int n));		/* Advance by words             */
extern int backword PP((int f, int n));		/* Backup by words              */
extern int upperword PP((int f, int n));	/* Upper case word.             */
extern int lowerword PP((int f, int n));	/* Lower case word.             */
extern int capword PP((int f, int n));		/* Initial capitalize word.     */
extern int delfword PP((int f, int n));		/* Delete forward word.         */
extern int delbword PP((int f, int n));		/* Delete backward word.        */
extern int wrapword PP((int f, int n));		/* wordwrap function		*/
#if	WORDPRO
extern int fillpara PP((int f, int n));		/* fill current paragraph	*/
extern int killpara PP((int f, int n));		/* kill the current paragraph	*/
extern int wordcount PP((int f, int n));	/* count words in region	*/
#endif

#endif

#if	MSDOS & GNUC
	/* <memory> in estruct.h */
#else
#if defined(__STDC__) || defined(_AIX) || defined(__STDLIB_H__) || defined(__STDLIB_H) || defined(_STDLIB_H) || defined(_INC_STDLIB)
	/* <stdlib.h> in estruct.h */
#else
extern char *malloc();
extern char *realloc();
#endif
#endif

#if	!USG
#ifndef __STDC__
#ifndef __SC__
#ifndef _INC_STRING /* msc */
extern char *strcat();
extern char *strcpy();
extern char *strncpy();
#endif
#endif
#endif
#endif

#ifdef	maindef

/* for MAIN.C */

/* initialized global definitions */

NOSHARE int	fillcol = 80;		/* Current fill column		*/
NOSHARE short FAR kbdm[NKBDM];		/* Macro			*/
NOSHARE char	*execstr = NULL;	/* pointer to string to execute	*/
NOSHARE char FAR golabel[NPAT] = "";	/* current line to go to	*/
NOSHARE unsigned char FAR wordlist[256]; /* characters considered "in words" */
NOSHARE int	wlflag = FALSE;		/* word list enabled flag	*/
NOSHARE int	execlevel = 0;		/* execution IF level		*/
NOSHARE int	eolexist = TRUE;	/* does clear to EOL exist	*/
NOSHARE int	revexist = FALSE;	/* does reverse video exist?	*/
NOSHARE int	isvt100 = FALSE;	/* vt100 compatible		*/
NOSHARE int	flickcode = FALSE;	/* do flicker supression?	*/
CONST char FAR	*modename[] = {		/* name of modes		*/
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER",
	"MAGIC", "CRYPT", "ASAVE", "BINARY"};
CONST char FAR	*modecode = "WCSEVOMYAB";/* letters to represent modes	*/
NOSHARE int	gmode = 0;		/* global editor mode		*/
NOSHARE int	gfcolor = 7;		/* global forgrnd color (white)	*/
NOSHARE int	gbcolor	= 0;		/* global backgrnd color (black)*/
NOSHARE int	bfcolor = 7;		/* border forgrnd color (white)	*/
NOSHARE int	bbcolor	= 0;		/* border backgrnd color (black)*/
NOSHARE int	sfcolor = 0;		/* shadow forgrnd color (white)	*/
NOSHARE int	sbcolor	= 7;		/* shadow backgrnd color (black)*/
#if WINMOD
NOSHARE char FAR wintitle[NSTRING] = "microemacs"; /* window title	*/
#endif
NOSHARE int	gasave = 256;		/* global ASAVE size		*/
NOSHARE int	gacount = 256;		/* count until next ASAVE	*/
NOSHARE int	sgarbf	= TRUE; 	/* TRUE if screen is garbage	*/
NOSHARE int	mpresf	= FALSE;	/* TRUE if message in last line */
NOSHARE int	clexec	= FALSE;	/* command line execution flag	*/
NOSHARE int	mstore	= FALSE;	/* storing text to macro flag	*/
NOSHARE int	discmd	= TRUE;		/* display command flag		*/
NOSHARE int	disinp	= TRUE;		/* display input characters	*/
NOSHARE int     dosync = FALSE;         /* sync the filesystem when saving */
NOSHARE int	popflag = FALSE;	/* pop-up windows enabled?	*/
NOSHARE int	cpopflag = TRUE;	/* completion pop-up windows enabled? */
NOSHARE int	cpending = FALSE;	/* input character pending?	*/
NOSHARE int	charpending;		/* character pushed back	*/
NOSHARE BUFFER	*bstore = NULL;		/* buffer to store macro text to*/
NOSHARE int	vtrow	= 0;		/* Row location of SW cursor */
NOSHARE int	vtcol	= 0;		/* Column location of SW cursor */
NOSHARE int	ttrow	= HUGE; 	/* Row location of HW cursor */
NOSHARE int	ttcol	= HUGE; 	/* Column location of HW cursor */
NOSHARE int	lbound	= 0;		/* leftmost column of current line
					   being displayed */
NOSHARE int	taboff	= 0;		/* tab offset for display	*/
NOSHARE int	hscroll = MEMMAP;	/* horizontal scrolling flag	*/
NOSHARE int	hjump = 1;		/* horizontal jump size		*/
NOSHARE int	tabsize = 8;		/* tab offset for display	*/
NOSHARE int	stabsize = 0;		/* current soft tab size	*/
NOSHARE int	metac = CTRL | '[';	/* current meta character */
NOSHARE int	ctlxc = CTRL | 'X';	/* current control X prefix char */
NOSHARE int	reptc = CTRL | 'U';	/* current universal repeat char */
NOSHARE int	abortc = CTRL | 'G';	/* current abort command char	*/
NOSHARE int	yanktype = SRNORM;	/* current yank style		*/
NOSHARE	int	cindtype = 0;		/* current c indent type	*/

NOSHARE int	quotec = 0x11;		/* quote char during mlreply() */
CONST char FAR	*cname[] = {		/* names of colors		*/
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE",
	"MAGENTA", "CYAN", "WHITE"};
NOSHARE KILL *kbufp  = NULL;		/* current kill buffer chunk pointer */
NOSHARE KILL *kbufh  = NULL;		/* kill buffer header pointer	*/
NOSHARE int kused = KBLOCK;		/* # of bytes used in kill buffer */
NOSHARE WINDOW *swindow = NULL;		/* saved window pointer		*/
NOSHARE int cryptflag = FALSE;		/* currently encrypting?	*/
NOSHARE short	*kbdptr;		/* current position in keyboard buf */
NOSHARE short	*kbdend = &kbdm[0];	/* ptr to end of the keyboard	*/
NOSHARE int	kbdmode = STOP;		/* current keyboard macro mode	*/
NOSHARE int	kbdrep = 0;		/* number of repetitions	*/
NOSHARE int	restflag = FALSE;	/* restricted use?		*/
NOSHARE int	lastkey = 0;		/* last keystoke		*/
NOSHARE int	seed = 0;		/* random number seed		*/
NOSHARE long	envram = 0L;	/* # of bytes current in use by malloc */
NOSHARE int	macbug = FALSE;		/* macro debuging flag		*/
CONST char FAR	*errorm = "ERROR";	/* error literal		*/
CONST char FAR	*truem = "TRUE";	/* true literal			*/
CONST char FAR	*falsem = "FALSE";	/* false litereal		*/
NOSHARE int	cmdstatus = TRUE;	/* last command status		*/
NOSHARE int	firstflag = TRUE;	/* first time			*/
NOSHARE char FAR palstr[49] = "";	/* palette string		*/
NOSHARE int	saveflag = 0;		/* Flags, saved with the $target var */
NOSHARE char	*fline = NULL;		/* dynamic return line */
NOSHARE int	flen = 0;		/* current length of fline */
NOSHARE int	disphigh = FALSE;	/* display high bit chars escaped */
NOSHARE int	menuflag = FALSE;	/* menu mode */
NOSHARE LINE	*menuline = NULL;	/* menu	line */
NOSHARE int	ykchar = -1;		/* last deleted character */
NOSHARE int	vt100keys = 0;		/* wait after escapes */

#if DISPSEVEN
NOSHARE int	dispseven = FALSE;	/* display only seven bit characters */
#endif

/* uninitialized global definitions */

NOSHARE int	currow;		/* Cursor row			*/
NOSHARE int	thisflag;	/* Flags, this command		*/
NOSHARE int	lastflag;	/* Flags, last command		*/
NOSHARE int	curgoal;	/* Goal for C-P, C-N		*/
NOSHARE WINDOW	*curwp;		/* Current window		*/
NOSHARE BUFFER	*curbp;		/* Current buffer		*/
NOSHARE WINDOW	*wheadp;	/* Head of list of windows	*/
NOSHARE BUFFER	*bheadp;	/* Head of list of buffers	*/
NOSHARE BUFFER	*blistp;	/* Buffer for C-X C-B		*/
NOSHARE WINDOW	*spltwp;	/* New window created by split	*/

NOSHARE char FAR sres[NBUFN];	/* current screen resolution	*/

NOSHARE char FAR pat[NPAT];	/* Search pattern		*/
NOSHARE char FAR tap[NPAT];	/* Reversed pattern array.	*/
NOSHARE char FAR rpat[NPAT];	/* replacement pattern		*/

NOSHARE int	(* FAR fkeytab[NFBIND])(int f, int n);  /* fast lookup table		*/
NOSHARE int	(*ffuncc)(int f, int n);	        /* FUNC | 'C' hook		*/
NOSHARE KEYTAB	fkeylast;		/* last looked up key		*/

NOSHARE unsigned int	frlen;	/* number of chars in fline */

NOSHARE char FAR mlline[NSTRING];	/* saved message line 		*/
NOSHARE int	mllen;			/* length of saved message line	*/

NOSHARE int	scrltop, scrlbot;	/* scrolling region */
NOSHARE int	phrow;			/* physical terminal row */

/* The variable matchlen holds the length of the matched
 * string - used by the replace functions.
 * The variable patmatch holds the string that satisfies
 * the search command.
 * The variables matchline and matchoff hold the line and
 * offset position of the start of match.
 */
NOSHARE unsigned int	mtchlen = 0;
NOSHARE unsigned int	mlenold  = 0;
NOSHARE char		*patmatch = NULL;
NOSHARE LINE		*matchline = NULL;
NOSHARE int		matchoff = 0;

#if	MAGIC
/*
 * The variable magical determines if there are actual
 * metacharacters in the string - if not, then we don't
 * have to use the slower MAGIC mode search functions.
 */
NOSHARE short	magical = FALSE;
NOSHARE short	rmagical = FALSE;
NOSHARE MC FAR	mcpat[NPAT];		/* the magic pattern		*/
NOSHARE MC FAR	tapcm[NPAT];		/* the reversed magic pattern	*/
NOSHARE RMC FAR	rmcpat[NPAT];		/* the replacement magic array */

#endif

/* directive name table:
	This holds the names of all the directives....	*/

CONST char FAR *dname[] = {
	"if", "else", "endif",
	"goto", "return", "endm",
	"while", "endwhile", "break",
	"force"
};

#if	DEBUGM
/*	vars needed for macro debugging output	*/
NOSHARE char FAR outline[NSTRING];	/* global string to hold debug line text */
#endif

#else

/* for all the other .C files */

/* initialized global external declarations */

NOSHARE extern	int	fillcol;	/* Fill column			*/
NOSHARE extern	short FAR kbdm[];	/* Holds kayboard macro data	*/
NOSHARE extern	char	*execstr;	/* pointer to string to execute	*/
NOSHARE extern	char FAR golabel[];	/* current line to go to	*/
NOSHARE extern	unsigned char FAR wordlist[]; /* characters considered "in words" */
NOSHARE extern	int 	wlflag;	/* word list enabled flag	*/
NOSHARE extern	int	execlevel;	/* execution IF level		*/
NOSHARE extern	int	eolexist;	/* does clear to EOL exist?	*/
NOSHARE extern	int	revexist;	/* does reverse video exist?	*/
NOSHARE extern	int	isvt100;	/* vt100 compatible		*/
NOSHARE extern	int	flickcode;	/* do flicker supression?	*/
extern CONST	char FAR *modename[];	/* text names of modes		*/
extern CONST	char FAR *modecode;	/* letters to represent modes	*/
NOSHARE extern	KEYTAB keytab[];	/* key bind to functions table	*/
NOSHARE extern	NBIND names[];		/* name to function table	*/
NOSHARE extern	int	gmode;		/* global editor mode		*/
NOSHARE extern	int	gfcolor;	/* global forgrnd color (white)	*/
NOSHARE extern	int	gbcolor;	/* global backgrnd color (black)*/
NOSHARE extern	int	bfcolor;	/* border forgrnd color (white)	*/
NOSHARE extern	int	bbcolor;	/* border backgrnd color (black)*/
NOSHARE extern	int	sfcolor;	/* shadow forgrnd color (white)	*/
NOSHARE extern	int	sbcolor;	/* shadow backgrnd color (black)*/
#if WINMOD
NOSHARE extern	char FAR wintitle[];	/* window title			*/
#endif
NOSHARE extern	int	gasave;		/* global ASAVE size		*/
NOSHARE extern	int	gacount;	/* count until next ASAVE	*/
NOSHARE extern	int	sgarbf; 	/* State of screen unknown	*/
NOSHARE extern	int	mpresf; 	/* Stuff in message line	*/
NOSHARE extern	int	clexec;		/* command line execution flag	*/
NOSHARE extern	int	mstore;		/* storing text to macro flag	*/
NOSHARE extern	int	discmd;		/* display command flag		*/
NOSHARE extern	int	disinp;		/* display input characters	*/
NOSHARE extern	int	dosync;		/* sync the filesystem when saving */
NOSHARE extern	int	popflag;	/* pop-up windows enabled?	*/
NOSHARE extern	int	cpopflag;	/* completion pop-up windows enabled? */
NOSHARE extern	int	cpending;	/* input character pending?	*/
NOSHARE extern	int	charpending;	/* character pushed back	*/
NOSHARE extern	BUFFER	*bstore;	/* buffer to store macro text to*/
NOSHARE extern	int	vtrow;		/* Row location of SW cursor */
NOSHARE extern	int	vtcol;		/* Column location of SW cursor */
NOSHARE extern	int	ttrow;		/* Row location of HW cursor */
NOSHARE extern	int	ttcol;		/* Column location of HW cursor */
NOSHARE extern	int	lbound;		/* leftmost column of current line
					   being displayed */
NOSHARE extern	int	taboff;		/* tab offset for display	*/
NOSHARE extern	int	hscroll;	/* horizontal scrolling flag	*/
NOSHARE extern	int	hjump;		/* horizontal jump size		*/
NOSHARE extern	int	tabsize;	/* tab offset for display	*/
NOSHARE extern	int	stabsize;	/* current soft tab size	*/
NOSHARE extern	int	metac;		/* current meta character */
NOSHARE extern	int	ctlxc;		/* current control X prefix char */
NOSHARE extern	int	reptc;		/* current universal repeat char */
NOSHARE extern	int	abortc;		/* current abort command char	*/
NOSHARE extern	int	yanktype;	/* current yank style		*/
NOSHARE	extern	int	cindtype;	/* current c indent type	*/

NOSHARE extern	int	quotec;		/* quote char during mlreply() */
extern CONST	char FAR *cname[];	/* names of colors		*/
NOSHARE extern KILL *kbufp;		/* current kill buffer chunk pointer */
NOSHARE extern KILL *kbufh;		/* kill buffer header pointer	*/
NOSHARE extern int kused;		/* # of bytes used in KB	*/
NOSHARE extern WINDOW *swindow;		/* saved window pointer		*/
NOSHARE extern int cryptflag;		/* currently encrypting?	*/
NOSHARE extern	short	*kbdptr;	/* current position in keyboard buf */
NOSHARE extern	short	*kbdend;	/* ptr to end of the keyboard */
NOSHARE extern	int kbdmode;		/* current keyboard macro mode	*/
NOSHARE extern	int kbdrep;		/* number of repetitions	*/
NOSHARE extern	int restflag;		/* restricted use?		*/
NOSHARE extern	int lastkey;		/* last keystoke		*/
NOSHARE extern	int seed;		/* random number seed		*/
NOSHARE extern	long envram;		/* # of bytes current in use by malloc */
NOSHARE extern	int	macbug;		/* macro debuging flag		*/
extern CONST	char FAR *errorm;	/* error literal		*/
extern CONST	char FAR *truem;	/* true literal			*/
extern CONST	char FAR *falsem;	/* false litereal		*/
NOSHARE extern	int	cmdstatus;	/* last command status		*/
NOSHARE extern	char FAR palstr[];	/* palette string		*/
NOSHARE extern	int	saveflag;	/* Flags, saved with the $target var */
NOSHARE extern	char	*fline;		/* dynamic return line */
NOSHARE extern	int	flen;		/* current length of fline */
NOSHARE extern	int	disphigh;	/* display high bit chars escaped */
NOSHARE extern	int	firstflag;	/* first time			*/
NOSHARE extern	int	menuflag;	/* menu mode */
NOSHARE extern	LINE	*menuline;	/* menu	line */
NOSHARE extern	int	ykchar;		/* last deleted character */
NOSHARE extern	int	vt100keys;	/* wait after escapes */

#if DISPSEVEN
NOSHARE extern	int	dispseven;	/* display only seven bit characters */
#endif

/* uninitialized global external declarations */

NOSHARE extern	int	currow;		/* Cursor row			*/
NOSHARE extern	int	thisflag;	/* Flags, this command		*/
NOSHARE extern	int	lastflag;	/* Flags, last command		*/
NOSHARE extern	int	curgoal;	/* Goal for C-P, C-N		*/
NOSHARE extern	WINDOW	*curwp;		/* Current window		*/
NOSHARE extern	BUFFER	*curbp;		/* Current buffer		*/
NOSHARE extern	WINDOW	*wheadp;	/* Head of list of windows	*/
NOSHARE extern	BUFFER	*bheadp;	/* Head of list of buffers	*/
NOSHARE extern	BUFFER	*blistp;	/* Buffer for C-X C-B		*/
NOSHARE extern	WINDOW	*spltwp;	/* New window created by split	*/

NOSHARE extern	char FAR sres[NBUFN];	/* current screen resolution	*/

NOSHARE extern	char FAR pat[];		/* Search pattern		*/
NOSHARE extern	char FAR tap[];		/* Reversed pattern array.	*/
NOSHARE extern	char FAR rpat[];	/* Replacement pattern		*/

NOSHARE extern	unsigned int	mtchlen; /* length of found string	*/
NOSHARE extern	unsigned int	mlenold; /* previous length of found str*/
NOSHARE extern	char	*patmatch;	/* the found string		*/
NOSHARE extern	LINE	*matchline;	/* line pointer to found string	*/
NOSHARE extern	int	matchoff;	/* offset to the found string	*/

NOSHARE extern	int (* FAR fkeytab[NFBIND])(int f, int n);	/* fast lookup table	*/
NOSHARE extern	int	(*ffuncc)(int f, int n);		/* FUNC | 'C' hook	*/
NOSHARE extern	KEYTAB	fkeylast;		/* last looked up key	*/

NOSHARE extern	unsigned int	frlen;		/* number of chars in fline */

NOSHARE extern	char FAR mlline[NSTRING];	/* saved message line 	*/
NOSHARE extern	int	mllen;		/* length of saved message line	*/

NOSHARE extern	int	scrltop, scrlbot;	/* scrolling region */
NOSHARE extern	int	phrow;			/* physical terminal row */

#if	MAGIC

NOSHARE extern	short	magical;	/* meta-characters in pattern?	*/
NOSHARE extern	short	rmagical;
NOSHARE extern	MC FAR	mcpat[];	/* the magic pattern		*/
NOSHARE extern	MC FAR	tapcm[];	/* the reversed magic pattern	*/
NOSHARE extern	RMC FAR	rmcpat[];	/* the replacement magic array	*/

#endif

extern CONST char FAR *dname[];		/* directive name table		*/

#if	DEBUGM
/*	vars needed for macro debugging output	*/
NOSHARE extern char FAR outline[];	/* global string to hold debug line text */
#endif

#endif

/* terminal table defined only in TERM.C */

#ifndef	termdef
NOSHARE extern	TERM FAR term;		/* Terminal information.	*/
#endif

#if DISPSEVEN | DECEDT
NOSHARE extern int FAR me_char_len[256];	/* character name length */
extern CONST char FAR *me_char_name[256];	/* character name data */
#endif

#endif
