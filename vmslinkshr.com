$! Link microEmacs 3.9 as a shareable image
$! To use in MAIL,
$!	DEFINE MAIL$EDIT CALLABLE_ME
$
$ opt = "/NOMAP"
$ if p1 .eqs. "M" then opt = "/MAP"
$ set proc/priv=bypass
$ link'opt'/NODEBUG/NOTRACE/SHARE=sys$library:meshr -
	BASIC,BIND,BUFFER,CRYPT,DISPLAY,EVAL,MEEXEC,FILE,-
	FILEIO,INPUT,ISEARCH,MELINE,MAIN,MEMENU,RANDOM,-
	REGION,SEARCH,MESPAWN,TERMIO,VMSVT,WINDOW,WORD,-
	sys$input:/options
    SYS$LIBRARY:LBRSHR/SHARE
    SYS$SHARE:VAXCRTL/SHARE
    universal=me$edit
$
$ set noon
$ set proc/priv=cmkrnl
$ set file/prot=(w:re) sys$library:meshr.exe
$ install rem sys$library:meshr.exe
$ install add sys$library:meshr.exe
$ install lis sys$library:meshr.exe
$ set proc/priv=nobypass
$ set proc/priv=nocmkrnl
