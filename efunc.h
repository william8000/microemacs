/*	EFUNC.H:	MicroEMACS function declarations and names

		This file list all the C code functions used by MicroEMACS
	and the names to use to bind keys to them. To add functions,
	declare it here in both the extern function list and the name
	binding table.

*/

#ifndef _EFUNC_H_
#define	_EFUNC_H_

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
	{"clear-and-redraw",		me_refresh},
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
	{"filter-buffer",		me_filter},
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
