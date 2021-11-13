$! SpawnEmacs.com -- spawn a kept version of emacs
$!
$!	p1 = file to edit
$!	p2 = directory to spawn emacs from
$
$	set noon
$
$	file = p1
$	defdir = p2
$
$	master_pid = f$getjpi("", "MASTER_PID")
$
$	emacs_process = f$getjpi(master_pid, "PRCNAM") + "_EMACS"
$!	define/key/nolog pf4 "ATTACH ""''emacs_process'"""/terminate/noecho
$	define/key/nolog pf4 "spawnemacs"/terminate/noecho
$
$	context = ""
$ loop:
$	pid = f$pid(context)
$	if pid .eqs. "" then goto not_found
$	if f$getjpi(pid, "PRCNAM") .eqs. emacs_process then goto found
$	goto loop
$
$ found:
$	attach /id='pid'
$	exit
$
$ not_found:
$	default = f$environment("DEFAULT")
$	if defdir .nes. "" then set def 'defdir'
$	spawn /nokeypad /process="''emacs_process'" -
		@$emacs:emacsprocess 'file'
$	if defdir .nes. "" then set def 'default'
$	exit
$
