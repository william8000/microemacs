$ v=f$verify(0)
$ set noon
$ status = 1
$ if f$locate("*", p1) .lt. f$length(p1) then goto nomove
$	p4 = "''p3'"
$	p3 = "''p2'"
$	p2 = "''p1'"
$	p1 = ""
$ nomove:
$ p1 = "''p1' ''p3'C"
$!c :== @[wb.c]c *
$ vmscc :== @vmscc *
$!
$! C options name options
$!  O = optimize, C = compile only, D = debug
$!  L = listing, cross ref, M = map, K = link only, S = link with STRLIB
$!  P = print listing, B = beep when done, T = time
$!
$
$ IF p2 .EQS. "" THEN INQUIRE/nopunct p2 "$_C_Module:	"
$ IF p2 .EQS. "" THEN goto fixver
$
$ if f$locate("1", p1) .lt. f$length(p1) then goto c11
$ if f$locate("P", p1) .lt. f$length(p1) then p1 = "L''p1'"
$ OPT = "/NOLIST"
$ if f$locate("O", p1) .lt. f$length(p1) then -
	OPT = "/NODEBUG/OPTIMIZE/NOLIST"
$ if f$locate("L", p1) .lt. f$length(p1) then OPT = "/LIST/CROSS/NOOBJECT"
$ if f$locate("D", p1) .lt. f$length(p1) then OPT = "/DEBUG/LIST"
$ if f$locate("T", p1) .lt. f$length(p1) then sho time
$
$ if f$locate("K", p1) .lt. f$length(p1) then goto endvcomp
$ write sys$output "Compiling ", p2, opt, "."
$ CC'OPT' 'p2'
$ status = $status
$ if .not. status then goto done
$ endvcomp:
$ p2 = F$EXTRACT(0, F$LOCATE("/", p2), p2)
$ p2 = f$extract(0, f$locate(".C", p2), p2)
$ if f$locate("C", p1) .lt. f$length(p1) then goto done
$ if f$locate("P", p1) .lt. f$length(p1) then pri 'p2'
$ if f$locate("L", p1) .lt. f$length(p1) then goto done
$ opt = "''p2'"
$ if f$locate("S", p1) .lt. f$length(p1) then -
	opt = "''opt',[wb.c]strlib,[wb.c]scan,[wb.c]seed"
$ opt = "LINK/NOSHARE/NOMAP ''opt',lb:[1,1]vaxcrtl/lib"
$ if p4 .nes. "" .and. p4 .nes. "*" then opt = p4
$ write sys$output "Linking V4 ''opt'."
$ 'opt'
$ status = $status
$ goto DONE
$
$ C11:
$ if f$locate("T", p1) .lt. f$length(p1) then sho time
$ if f$locate("K", p1) .lt. f$length(p1) then goto end1comp
$ p2 = F$EXTRACT(0, F$LOCATE("/", p2), p2)
$ p2 = f$extract(0, f$locate(".C", p2), p2)
$ opt = ""
$ write sys$output "Preprocessing ", p2, opt, "."
$ MP 'p2'.c
$ status = $status
$ if .not. status then goto done
$ write sys$output "Compiling ", p2, opt, "."
$ xcc 'p2'.mpc
$ status = $status
$ if .not. status then goto done
$ write sys$output "Assembling ", p2, opt, "."
$ xas 'p2'.s
$ status = $status
$ if .not. status then goto done
$ if f$locate("T", p1) .lt. f$length(p1) then sho time
$ end1comp:
$ if f$locate("C", p1) .lt. f$length(p1) then goto done
$ opt = "''p2'"
$ opt = "tkb ''p2'=''opt',c/lb"
$ if p4 .nes. "" .and. p4 .nes. "*" then opt = p4
$ write sys$output "Linking ''opt'."
$ 'opt'
$ status = $status
$ goto DONE
$
$ DONE:
$ if status then write sys$output "''p2' OK."
$ if .not. status then write sys$output "*** Errors in ''p2'."
$ if f$locate("T", p1) .lt. f$length(p1) then sho time
$ if f$locate("B", p1) .lt. f$length(p1) then beep
$ fixver: if v then set verify
$ exit 'status'
