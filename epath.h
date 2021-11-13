/*	EPATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependant basis.

									*/

/*	possible names and paths of help files under different OSs	*/

#ifndef _EPATH_H_
#define	_EPATH_H_

CONST char *pathname[] =

#if	AMIGA
{
	".emacsrc",
	"emacs.hlp",
	"",
	"sys:c/",
	"sys:t/",
	"sys:s/",
	":c/",
	":t/",
	":s/"
};
#endif

#if	ST520
{
	"emacs.rc",
	"emacs.hlp",
	"\\",
	"\\bin\\",
	"\\util\\",
	""
};
#endif

#if	FINDER
{
	"emacs.rc",
	"emacs.hlp",
	"/bin",
	"/sys/public",
	""
};
#endif

#if	MSDOS
{
	"emacs.rc",
	"emacs.hlp",
	/* "\\sys\\public\\", */
	/* "\\usr\\bin\\", */
	/* "\\bin\\", */
	"\\",
	"c:\\",
	"c:\\dos\\",
	"c:\\bin\\",
	""
};
#endif

#if	V7 | BSD | USG
{
	".emacsrc",
	"emacs.hlp",
	"/usr/local/lib/",
	"/usr/lib/",
	""
};
#endif

#if	VMS
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"microemacs$lib:",
	"sys$sysdevice:[vmstools]"
};
#endif

#if	DECUSC
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"lb:[1,1]",
	"lb:"
};
#endif

#if	WMCS
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"sys$disk/syslib.users/"
};
#endif

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))

#endif
