$! vmsmake.com -- build emacs executable
$
$ set noon
$
$ p1 = "2OC''p1'"
$
$ c := @vmscc *
$
$ c ANSI 'p1'
$ c BASIC 'p1'
$ c BIND 'p1'
$ c BUFFER 'p1'
$ c CRYPT 'p1'
$ c DG10 'p1'
$ c DISPLAY 'p1'
$ c DOLOCK 'p1'
$ c EMACS 'p1'
$ c EVAL 'p1'
$ c MEEXEC 'p1'
$ c FILE 'p1'
$ c FILEIO 'p1'
$ c HP110 'p1'
$ c HP150 'p1'
$ c IBMPC 'p1'
$ c INPUT 'p1'
$ c ISEARCH 'p1'
$ c MELINE 'p1'
$ c LOCK 'p1'
$ c MAIN 'p1'
$ c MEMENU 'p1'
$ c RANDOM 'p1'
$ c REGION 'p1'
$ c SEARCH 'p1'
$ c MESPAWN 'p1'
$ c ST520 'p1'
$ c TCAP 'p1'
$ c TERMIO 'p1'
$ c TIPC 'p1'
$ c VMSVT 'p1'
$ c VT52 'p1'
$ c WINDOW 'p1'
$ c WORD 'p1'
$ c Z309 'p1'
$
$!@vmslib
$
$ @vmslink 'p1' 'p2'
$
