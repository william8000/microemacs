/*	EBIND:		Initial default key to function bindings for
			MicroEMACS 3.7
*/

#ifndef _EBIND_H_
#define	_EBIND_H_

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
NOSHARE KEYTAB  keytab[NBINDS] = {
	{CTRL|'A',		gobol},
	{CTRL|'B',		backchar},
	{CTRL|'C',		insspace},
	{CTRL|'D',		forwdel},
	{CTRL|'E',		goeol},
	{CTRL|'F',		forwchar},
	{CTRL|'G',		ctrlg},
	{CTRL|'H',		backdel},
	{CTRL|'I',		tab},
	{CTRL|'J',		indent},
	{CTRL|'K',		killtext},
	{CTRL|'L',		me_refresh},
	{CTRL|'M',		newline},
	{CTRL|'N',		forwline},
	{CTRL|'O',		openline},
	{CTRL|'P',		backline},
	{CTRL|'Q',		quote},
	{CTRL|'R',		backsearch},
	{CTRL|'S',		forwsearch},
	{CTRL|'T',		twiddle},
	{CTRL|'U',		unarg},
	{CTRL|'V',		forwpage},
	{CTRL|'W',		killregion},
	{CTRL|'X',		cex},
	{CTRL|'Y',		yank},
	{CTRL|'Z',		backpage},
	{CTRL|']',		meta},
	{CTLX|CTRL|'A',		fileapp},
	{CTLX|CTRL|'B',		listbuffers},
	{CTLX|CTRL|'C',		quit},          /* Hard quit.           */
#if	AEDIT
	{CTLX|CTRL|'D',		detab},
	{CTLX|CTRL|'E',		entab},
#endif
	{CTLX|CTRL|'F',		filefind},
	{CTLX|CTRL|'I',		insfile},
	{CTLX|CTRL|'L',		lowerregion},
	{CTLX|CTRL|'M',		delmode},
	{CTLX|CTRL|'N',		mvdnwind},
	{CTLX|CTRL|'O',		deblank},
	{CTLX|CTRL|'P',		mvupwind},
	{CTLX|CTRL|'R',		fileread},
	{CTLX|CTRL|'S',		filesave},
#if	AEDIT
	{CTLX|CTRL|'T',		trim},
#endif
	{CTLX|CTRL|'U',		upperregion},
	{CTLX|CTRL|'V',		viewfile},
	{CTLX|CTRL|'W',		filewrite},
	{CTLX|CTRL|'X',		swapmark},
	{CTLX|CTRL|'Z',		shrinkwind},
	{CTLX|' ',		remmark},
	{CTLX|'?',		deskey},
	{CTLX|'!',		spawncmd},
	{CTLX|'@',		pipecmd},
	{CTLX|'#',		me_filter},
	{CTLX|'$',		spawncmd},
	{CTLX|'%',		filtnew},
	{CTLX|'=',		showcpos},
	{CTLX|'(',		ctlxlp},
	{CTLX|')',		ctlxrp},
	{CTLX|'<',		narrow},
	{CTLX|'>',		widen},
	{CTLX|'^',		enlargewind},
	{CTLX|' ',		remmark},
	{CTLX|'0',		delwind},
	{CTLX|'1',		onlywind},
	{CTLX|'2',		splitwind},
	{CTLX|'A',		setvar},
	{CTLX|'B',		usebuffer},
	{CTLX|'C',		spawncli},
#if	BSD
	{CTLX|'D',		bktoshell},
#endif
	{CTLX|'E',		ctlxe},
	{CTLX|'F',		setfillcol},
#if	DEBUGM
	{CTLX|'G',		dispvar},
#endif
	{CTLX|'K',		killbuffer},
	{CTLX|'M',		me_setmode},
	{CTLX|'N',		filename},
	{CTLX|'O',		nextwind},
	{CTLX|'P',		prevwind},
#if	ISRCH
	{CTLX|'R',		risearch},
	{CTLX|'S',		fisearch},
#endif
	{CTLX|'W',		resize},
	{CTLX|'X',		nextbuffer},
	{CTLX|'Z',		enlargewind},
#if	WORDPRO
	{META|CTRL|'C',		wordcount},
#endif
#if	PROC
	{META|CTRL|'E',		execproc},
#endif
#if	CFENCE
	{META|CTRL|'F',		getfence},
#endif
	{META|CTRL|'H',		delbword},
	{META|CTRL|'K',		unbindkey},
	{META|CTRL|'L',		reposition},
	{META|CTRL|'M',		delgmode},
	{META|CTRL|'N',		namebuffer},
	{META|CTRL|'R',		qreplace},
	{META|CTRL|'S',		newsize},
	{META|CTRL|'T',		newwidth},
	{META|CTRL|'V',		scrdwnxt},
#if	WORDPRO
	{META|CTRL|'W',		killpara},
#endif
	{META|CTRL|'X',		execcmd},
	{META|CTRL|'Z',		scrupnxt},
	{META|' ',		setmark},
	{META|'?',		me_help},
	{META|'!',		reposition},
	{META|'.',		setmark},
	{META|'>',		goeob},
	{META|'<',		gobob},
	{META|'\\',		delfword},
#if	DISPSEVEN | DECEDT
	{META|'+',		me_compose},
	{META|'=',		me_unquotep},
	{META|';',		me_unprefix},
	{META|'\'',		me_add_acute},
	{META|'_',		me_add_bar},
	{META|',',		me_add_cedilla},
	{META|'^',		me_add_circ},
	{META|'`',		me_add_grave},
	{META|'/',		me_add_slash},
	{META|'~',		me_add_tilde},
	{META|'"',		me_add_umlaut},
	{META|'*',		me_add_ring},
#else
	{META|'~',		unmark},
#endif
#if	APROP
	{META|'A',		apro},
#endif
	{META|'B',		backword},
	{META|'C',		capword},
	{META|'D',		delfword},
#if	CRYPT
	{META|'E',		me_setkey},
#endif
	{META|'F',		forwword},
	{META|'G',		gotoline},
	{META|'K',		bindtokey},
	{META|'L',		lowerword},
	{META|'M',		setgmode},
#if	WORDPRO
	{META|'N',		goeop},
	{META|'P',		gobop},
	{META|'Q',		fillpara},
#endif
	{META|'R',		sreplace},
#if	BSD | V7 | USG | VMS
	{META|'S',		bktoshell},
#endif
	{META|'U',		upperword},
	{META|'V',		backpage},
	{META|'W',		copyregion},
	{META|'X',		namedcmd},
	{META|'Z',		quickexit},
	{META|0x7F,              delbword},

#if	MSDOS & (HP150 == 0) & (WANGPC == 0) & (HP110 == 0)
	{SPEC|CTRL|'_',		forwhunt},
	{SPEC|CTRL|'S',		backhunt},
	{SPEC|71,		gobol},
#if	DECEDT
	{SPEC|72,		scupwind},
	{SPEC|80,		scdnwind},
#else
	{SPEC|72,		backline},
	{SPEC|80,		forwline},
#endif
	{SPEC|73,		backpage},
	{SPEC|75,		backchar},
	{SPEC|77,		forwchar},
	{SPEC|79,		goeol},
	{SPEC|81,		forwpage},
	{SPEC|82,		insspace},
	{SPEC|83,		forwdel},
	{SPEC|115,		backword},
	{SPEC|116,		forwword},
#if	WORDPRO
	{SPEC|132,		gobop},
	{SPEC|118,		goeop},
#endif
#if	DECEDT
#else
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
	{SPEC|92,		cbuf9},
	{SPEC|93,		cbuf10},
#endif
#endif

#if	HP150
	{SPEC|32,		backline},
	{SPEC|33,		forwline},
	{SPEC|35,		backchar},
	{SPEC|34,		forwchar},
	{SPEC|44,		gobob},
	{SPEC|46,		forwpage},
	{SPEC|47,		backpage},
	{SPEC|82,		nextwind},
	{SPEC|68,		openline},
	{SPEC|69,		killtext},
	{SPEC|65,		forwdel},
	{SPEC|64,		ctlxe},
	{SPEC|67,		me_refresh},
	{SPEC|66,		reposition},
	{SPEC|83,		me_help},
	{SPEC|81,		deskey},
#endif

#if	HP110
	{SPEC|0x4b,		backchar},
	{SPEC|0x4d,		forwchar},
	{SPEC|0x48,		backline},
	{SPEC|0x50,		forwline},
	{SPEC|0x43,		me_help},
	{SPEC|0x73,		backword},
	{SPEC|0x74,		forwword},
	{SPEC|0x49,		backpage},
	{SPEC|0x51,		forwpage},
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
#endif

#if	AMIGA
	{SPEC|'?',		me_help},
	{SPEC|'A',		backline},
	{SPEC|'B',		forwline},
	{SPEC|'C',		forwchar},
	{SPEC|'D',		backchar},
	{SPEC|'T',		backpage},
	{SPEC|'S',		forwpage},
	{SPEC|'a',		backword},
	{SPEC|'`',		forwword},
	{SPEC|'P',		cbuf1},
	{SPEC|'Q',		cbuf2},
	{SPEC|'R',		cbuf3},
	{SPEC|'S',		cbuf4},
	{SPEC|'T',		cbuf5},
	{SPEC|'U',		cbuf6},
	{SPEC|'V',		cbuf7},
	{SPEC|'W',		cbuf8},
	{SPEC|'X',		cbuf9},
	{SPEC|'Y',		cbuf10},
	{127,			forwdel},

#endif

#if	ST520
	{SPEC|'b',		me_help},
	{SPEC|'H',		backline},
	{SPEC|'P',		forwline},
	{SPEC|'M',		forwchar},
	{SPEC|'K',		backchar},
	{SPEC|'t',		setmark},
	{SPEC|'a',		yank},
	{SPEC|'R',		insspace},
	{SPEC|'G',		gobob},
	{127,			forwdel},
	{SPEC|84,		cbuf1},
	{SPEC|85,		cbuf2},
	{SPEC|86,		cbuf3},
	{SPEC|87,		cbuf4},
	{SPEC|88,		cbuf5},
	{SPEC|89,		cbuf6},
	{SPEC|90,		cbuf7},
	{SPEC|91,		cbuf8},
	{SPEC|92,		cbuf9},
	{SPEC|93,		cbuf10},

#endif

#if  WANGPC
	SPEC|0xE0,              quit,           /* Cancel */
	SPEC|0xE1,              me_help,        /* Help */
	SPEC|0xF1,              me_help,        /* ^Help */
	SPEC|0xE3,              ctrlg,          /* Print */
	SPEC|0xF3,              ctrlg,          /* ^Print */
	SPEC|0xC0,              backline,       /* North */
	SPEC|0xD0,              gobob,          /* ^North */
	SPEC|0xC1,              forwchar,       /* East */
	SPEC|0xD1,              goeol,          /* ^East */
	SPEC|0xC2,              forwline,       /* South */
	SPEC|0xD2,              gobop,          /* ^South */
	SPEC|0xC3,              backchar,       /* West */
	SPEC|0xD3,              gobol,          /* ^West */
	SPEC|0xC4,              ctrlg,          /* Home */
	SPEC|0xD4,              gobob,          /* ^Home */
	SPEC|0xC5,              filesave,       /* Execute */
	SPEC|0xD5,              ctrlg,          /* ^Execute */
	SPEC|0xC6,              insfile,        /* Insert */
	SPEC|0xD6,              ctrlg,          /* ^Insert */
	SPEC|0xC7,              forwdel,        /* Delete */
	SPEC|0xD7,              killregion,     /* ^Delete */
	SPEC|0xC8,              backpage,       /* Previous */
	SPEC|0xD8,              prevwind,       /* ^Previous */
	SPEC|0xC9,              forwpage,       /* Next */
	SPEC|0xD9,              nextwind,       /* ^Next */
	SPEC|0xCB,              ctrlg,          /* Erase */
	SPEC|0xDB,              ctrlg,          /* ^Erase */
	SPEC|0xDC,              ctrlg,          /* ^Tab */
	SPEC|0xCD,              ctrlg,          /* BackTab */
	SPEC|0xDD,              ctrlg,          /* ^BackTab */
	SPEC|0x80,              ctrlg,          /* Indent */
	SPEC|0x90,              ctrlg,          /* ^Indent */
	SPEC|0x81,              ctrlg,          /* Page */
	SPEC|0x91,              ctrlg,          /* ^Page */
	SPEC|0x82,              ctrlg,          /* Center */
	SPEC|0x92,              ctrlg,          /* ^Center */
	SPEC|0x83,              ctrlg,          /* DecTab */
	SPEC|0x93,              ctrlg,          /* ^DecTab */
	SPEC|0x84,              ctrlg,          /* Format */
	SPEC|0x94,              ctrlg,          /* ^Format */
	SPEC|0x85,              ctrlg,          /* Merge */
	SPEC|0x95,              ctrlg,          /* ^Merge */
	SPEC|0x86,              setmark,        /* Note */
	SPEC|0x96,              ctrlg,          /* ^Note */
	SPEC|0x87,              ctrlg,          /* Stop */
	SPEC|0x97,              ctrlg,          /* ^Stop */
	SPEC|0x88,              forwsearch,     /* Srch */
	SPEC|0x98,              backsearch,     /* ^Srch */
	SPEC|0x89,              sreplace,       /* Replac */
	SPEC|0x99,              qreplace,       /* ^Replac */
	SPEC|0x8A,              ctrlg,          /* Copy */
	SPEC|0x9A,              ctrlg,          /* ^Copy */
	SPEC|0x8B,              ctrlg,          /* Move */
	SPEC|0x9B,              ctrlg,          /* ^Move */
	SPEC|0x8C,              namedcmd,       /* Command */
	SPEC|0x9C,              spawncmd,	/* ^Command */
	SPEC|0x8D,              ctrlg,          /* ^ */
	SPEC|0x9D,              ctrlg,          /* ^^ */
	SPEC|0x8E,              ctrlg,          /* Blank */
	SPEC|0x9E,              ctrlg,          /* ^Blank */
	SPEC|0x8F,              gotoline,       /* GoTo */
	SPEC|0x9F,              usebuffer,      /* ^GoTo */
#endif
 
#if	DECEDT
	
#if	MSDOS & IBMPC

	/* PC function keys */

	{SPEC|';',		forwsearch},	/* f1 search-forward */
	{SPEC|'<',		backsearch},	/* f2 search-reverse */
	{SPEC|'=',		forwhunt},	/* f3 hunt-forward */
	{SPEC|'>',		backhunt},	/* f4 hunt-backward */
	{SPEC|'?',		nextwind},	/* f5 next-window */
	{SPEC|'@',		ctlxe},		/* f6 execute-macro */
	{SPEC|'A',		filefind},	/* f7 find-file */
	{SPEC|'B',		execfile},	/* f8 execute-file */
	{SPEC|'C',		filesave},	/* f9 save-file */
	{SPEC|'D',		quit},		/* f10 exit-emacs */

	/* Shifted PC function keys */

#if	APROP
	{SPEC|'V',		apro},		/* F3 apropos */
#endif
	{SPEC|'W',		deskey},	/* F4 describe-key */
	{SPEC|'X',		killregion},	/* F5 kill-region */
	{SPEC|'Y',		copyregion},	/* F6 copy-region */
	{SPEC|'Z',		setmark},	/* F7 set-mark */
	{SPEC|'[',		remmark},	/* F8 remove-mark */
	{SPEC|'\\',		spawncmd},	/* F9 shell-command */
	{SPEC|']',		spawncli},	/* F10 i-shell */

	/* PC keypad */

	{META|SPEC|'G',		gobob},		/* M-home beginning-of-file */
	{META|SPEC|'I',		gobob},		/* M-pgup beginning-of-file */
	{META|SPEC|'O',		goeob},		/* M-end end-of-file */
	{META|SPEC|'Q',		goeob},		/* M-pgdn end-of-file */

	{META|SPEC|'H',		gobob},		/* M-up beginning-of-file */
	{META|SPEC|'P',		goeob},		/* M-down end-of-file */
	{META|SPEC|'K',		backword},	/* M-right */
	{META|SPEC|'M',		forwword},	/* M-left */

#else
	/* vtxxx terminals */

	{SPEC|'A',		scupwind},	/* up arrow */
	{SPEC|'B',		scdnwind},	/* down arrow */
	{SPEC|'C',		forwchar},	/* right arrow */
	{SPEC|'D',		backchar},	/* left arrow */

	{META|SPEC|'A',		gobob},		/* gold up arrow */
	{META|SPEC|'B',		goeob},		/* gold down arrow */
	{META|SPEC|'C',		forwword},	/* gold right arrow */
	{META|SPEC|'D',		backword},	/* gold left arrow */

	{SPEC|'P',		meta},		/* gold */
	{SPEC|'Q',		deskey},	/* help */
	{SPEC|'R',		advhunt},	/* find next */
#if defined(__INTERIX)
	{SPEC|'S',		backpage},	/* page up */
	{SPEC|'T',		forwpage},	/* page down */
#else
	{SPEC|'S',		delfline},	/* del line */
#endif
	{SPEC|'w',		advbot},	/* page */
	{SPEC|'x',		advpage},	/* section */
	{SPEC|'y',		apkillregion},	/* append */
	{SPEC|'m',		delfword},	/* delete word */
	{SPEC|'t',		advadv},	/* advance */
	{SPEC|'u',		advback},	/* backup */
	{SPEC|'v',		cutregion},	/* cut */
	{SPEC|'l',		forwdel},	/* del char */
	{SPEC|'q',		advword},	/* adv word */
	{SPEC|'r',		adveol},	/* adv end of line */
	{SPEC|'s',		advchar},	/* adv char */
	{SPEC|'p',		advline},	/* line */
	{SPEC|'n',		setmark},	/* select */
	{SPEC|'M',		nullcmd},	/* enter */

	{META|SPEC|'P',		meta},		/* gold gold */
#if	APROP
	{META|SPEC|'Q',		apro},		/* gold help */
#endif
	{META|SPEC|'R',		advsearch},	/* gold find */
	{META|SPEC|'S',		yank},		/* undel line */
	{META|SPEC|'w',		namedcmd},	/* command */
#if	WORDPRO
	{META|SPEC|'x',		fillpara},	/* fill */
#endif
	{META|SPEC|'y',		sreplace},	/* change */
	{META|SPEC|'m',		yank},		/* undel word */
	{META|SPEC|'t',		goeob},		/* bottom */
	{META|SPEC|'u',		gobob},		/* top */
	{META|SPEC|'v',		yank},		/* paste */
	{META|SPEC|'l',		yankchar},	/* undel char */
	{META|SPEC|'q',		caseregion},	/* change case */
	{META|SPEC|'r',		killtext},	/* del line */
	{META|SPEC|'s',		specins},	/* special insertion */
	{META|SPEC|'p',		openline},	/* open line */
	{META|SPEC|'n',		remmark},	/* unselect */
	{META|SPEC|'M',		qreplace},	/* substitute */

	/* linux rxvt key pad */
	{SPEC|'o',		deskey},	/* help */
	{SPEC|'j',		advhunt},	/* find next */
	{SPEC|'k',		forwdel},	/* del char */
#if	APROP
	{META|SPEC|'o',		apro},		/* gold help */
#endif
	{META|SPEC|'j',		advsearch},	/* gold find */
	{META|SPEC|'k',		yankchar},	/* undel char */
	{SPEC|CTRL|'H',		goeol},		/* end */
	{META|SPEC|CTRL|'H',	goeol},		/* gold end */
	{META|SPEC|CTRL|'E',	gobob},		/* gold page up */
	{META|SPEC|CTRL|'F',	goeob},		/* gold page down */

	/* xenix console */
	{SPEC|'H',		gobol},		/* home */
	{SPEC|'I',		backpage},	/* page up */
	{SPEC|'F',		goeol},		/* end */
	{SPEC|'G',		forwpage},	/* page down */
	{SPEC|'L',		setovrwrt},	/* insert */

	{META|SPEC|'H',		gobol},		/* gold home */
	{META|SPEC|'I',		gobob},		/* gold page up */
	{META|SPEC|'F',		goeol},		/* gold end */
	{META|SPEC|'G',		goeob},		/* gold page down */

#if defined(__INTERIX)
	{SPEC|'U',		goeol},		/* end */
#else
	{SPEC|'U',		forwsearch},	/* f9 search-forward */
#endif
	{SPEC|'V',		backsearch},	/* f10 search-reverse */
	{SPEC|'W',		forwhunt},	/* f11 hunt-forward */
	{SPEC|'X',		backhunt},	/* f12 hunt-backward */

	/* vt200 key pad */
	{SPEC|CTRL|'A',		advsearch},	/* search */
	{SPEC|CTRL|'B',		setovrwrt},	/* insert */
	{SPEC|CTRL|'C',		forwdel},	/* delete */
	{SPEC|CTRL|'D',		setmark},	/* select */
	{SPEC|CTRL|'E',		scprpage},	/* previous screen */
	{SPEC|CTRL|'F',		scnxpage},	/* next screen */

	/* vt200 function keys */
	{SPEC|CTRL|'W',		meta},		/* escape */
	{SPEC|CTRL|'X',		backchar},	/* back space */
	{SPEC|CTRL|'Y',		indent},	/* newline */
#if	APROP
	{SPEC|CTRL|'\\',	apro},		/* help */
#endif
	{SPEC|CTRL|']',		nullcmd},	/* do */

	/* Linux console */
	{SPEC|'[',		meta},		/* prefix on some F keys */

	/* Interix F keys */
	{SPEC|CTRL|'I',		forwsearch},	/* f9 search-forward */
	{SPEC|CTRL|'J',		backsearch},	/* f10 search-reverse */
	{SPEC|CTRL|'K',		forwhunt},	/* f11 hunt-forward */
	{SPEC|CTRL|'L',		backhunt},	/* f12 hunt-backward */

	/* Teleray function keys */

	{SPEC|CTRL|'G',		gobol},		/* home */
	{META|SPEC|CTRL|'G',	gobol},		/* gold home */
	{SPEC|CTRL|'P',		killregion},	/* kill */
	{SPEC|CTRL|'R',		advhunt},	/* locate/find */
	{SPEC|CTRL|'U',		splitwind},	/* split */
	{SPEC|'7',		nextwind},	/* alt screen */
	{SPEC|CTRL|'Z',		usebuffer},	/* next */
	{SPEC|'<',		filesave},	/* file */
	{SPEC|'!',		insspace},	/* insert */
	{SPEC|'"',		quickexit},	/* close */

	/* IBM RS/6000 console */
	{SPEC|139,		setovrwrt},	/* insert */
	/* {SPEC|'P',		forwdel},	** delete */
	/* {SPEC|'H',		gobol},		** home */
	{SPEC|146,		goeol},		/* end */
	{SPEC|150,		backpage},	/* page up */
	{SPEC|154,		forwpage},	/* page down */

	{META|SPEC|139,		setovrwrt},	/* gold insert */
	/* {META|SPEC|'P',	delfword},	** gold delete */
	/* {META|SPEC|'H',	gobol},		** gold home */
	{META|SPEC|146,		goeol},		/* gold end */
	{META|SPEC|150,		gobob},		/* gold page up */
	{META|SPEC|154,		goeob},		/* gold page down */
#endif

	/* extras */

#if	CFENCE
	{META|CTRL|'B',		getfence},	/* backward/forward paren */
#endif
#if	WORDPRO
	{META|'J',		fillpara},	/* justify paragraph */
#endif
#if	BSD
#else
	{CTLX|'D',		delwind},	/* delete current window */
#endif
	{CTRL|'@',		setmark},	/* set mark */
	{CTLX|'4',		filewfind},	/* find file to other window */
	{CTLX|'5',		filevfind},	/* view file to other window */
	{CTLX|'6',		bufffind},	/* Find buff to other window */
#endif

	{0x7F,			backdel},

	/* special internal bindings */
	{FUNC|'W',		wrapword},	/* called on word wrap */

	{FUNC|'C',		nullproc},	/* every command input */
#if	ACMODE
	{FUNC|'R',		acmode},	/* on file read */
#else
	{FUNC|'R',		nullproc},	/* on file read */
#endif

	{0,			NULL}
};

#if RAINBOW

#include "rainbow.h"

/*
 * Mapping table from the LK201 function keys to the internal EMACS character.
 */

short lk_map[][2] = {
	Up_Key,                         CTRL+'P',
	Down_Key,                       CTRL+'N',
	Left_Key,                       CTRL+'B',
	Right_Key,                      CTRL+'F',
	Shift+Left_Key,                 META+'B',
	Shift+Right_Key,                META+'F',
	Control+Left_Key,               CTRL+'A',
	Control+Right_Key,              CTRL+'E',
	Prev_Scr_Key,                   META+'V',
	Next_Scr_Key,                   CTRL+'V',
	Shift+Up_Key,                   META+'<',
	Shift+Down_Key,                 META+'>',
	Cancel_Key,                     CTRL+'G',
	Find_Key,                       CTRL+'S',
	Shift+Find_Key,                 CTRL+'R',
	Insert_Key,                     CTRL+'Y',
	Options_Key,                    CTRL+'D',
	Shift+Options_Key,              META+'D',
	Remove_Key,                     CTRL+'W',
	Shift+Remove_Key,               META+'W',
	Select_Key,                     CTRL+'@',
	Shift+Select_Key,               CTLX+CTRL+'X',
	Interrupt_Key,                  CTRL+'U',
	Keypad_PF2,                     META+'L',
	Keypad_PF3,                     META+'C',
	Keypad_PF4,                     META+'U',
	Shift+Keypad_PF2,               CTLX+CTRL+'L',
	Shift+Keypad_PF4,               CTLX+CTRL+'U',
	Keypad_1,                       CTLX+'1',
	Keypad_2,                       CTLX+'2',
	Do_Key,                         CTLX+'E',
	Keypad_4,                       CTLX+CTRL+'B',
	Keypad_5,                       CTLX+'B',
	Keypad_6,                       CTLX+'K',
	Resume_Key,                     META+'!',
	Control+Next_Scr_Key,           CTLX+'N',
	Control+Prev_Scr_Key,           CTLX+'P',
	Control+Up_Key,                 CTLX+CTRL+'P',
	Control+Down_Key,               CTLX+CTRL+'N',
	Help_Key,                       CTLX+'=',
	Shift+Do_Key,                   CTLX+'(',
	Control+Do_Key,                 CTLX+')',
	Keypad_0,                       CTLX+'Z',
	Shift+Keypad_0,                 CTLX+CTRL+'Z',
	Main_Scr_Key,                   CTRL+'C',
	Keypad_Enter,                   CTLX+'!',
	Exit_Key,                       CTLX+CTRL+'C',
	Shift+Exit_Key,                 CTRL+'Z'
};

#define lk_map_size     (sizeof(lk_map)/2)
#endif

#endif
