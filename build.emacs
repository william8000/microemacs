#!/bin/bash
#
# build.emacs -- make a stand-alone emacs binary
#
#	Use this on unix-like systems.
#
#	On most systems, you can run this script with no options.

# set -x

do_strip=yes
do_opt=yes
do_debug=no
do_clean=no
do_profile=no
do_warn=no
do_jobs=maybe
do_pipe=
do_static=
do_dry_run=
do_64=
do_install=no
is_gcc=no
is_bgcc=no
set_bgcc=no
curses=
do_mcs=
make_opts=
do_quiet=

if [ -z "$MAKE" ]
then
	if [ -x "/usr/local/bin/gmake" ]
	then MAKE="/usr/local/bin/gmake"
	else MAKE='make'
	fi
fi

set_bgcc_flags(){
	is_gcc=yes ; is_bgcc=yes ; do_strip=no ; do_debug=yes ; do_opt=no
}

if [ -z "$DEF_CC" ]
then
	if [ -x "$HOME/system/bin/terminal-gcc" ]
	then
		DEF_CC=terminal-gcc
		DEF_WARN="-D__linux__=1 -D__ANDROID__=1 -D__TERMINAL_IDE__=1 -Bsymbolic -fPIE"
		# -Bsymbolic -fPIE is a work-around to prevent the message below on Android 4.3 
		# CANNOT LINK EXECUTABLE: ./emacs invalid R_ARM_COPY relocation against DT_SYMBOLIC shared library libc.so (built with -Bsymbolic?)
	fi
fi

case "$DEF_CC" in
bgcc*)	set_bgcc_flags
	CC=bgcc
	export CC
	;;
*gcc*)	is_gcc=yes
	;;
esac

if [ -z "$FULL_OPT" ]
then
	case "$CC" in
	*gcc*)	FULL_OPT="-O3" ;;
	*)	FULL_OPT="-O" ;;
	esac
fi

while [ "X$1" != "X" ]
do
	case "$1" in
	-s|-strip)	do_strip=yes ;;
	+s|+strip)	do_strip=no ;;
	-sym)		do_strip=no ;;
	-g|-debug)	do_debug=yes ;;
	-O)		do_opt=yes ;;
	-O0)		do_opt=no ;;
	-p)		do_profile=yes ;;
	-w|-W3|-warn)	do_warn=yes ;;
	-pipe)		do_pipe=yes ;;
	-j)		do_jobs=yes ;;
	+j|-j0|-j1)	do_jobs=no ;;
	-static)	do_static=yes ;;
	-clean)		do_clean=yes ;;
	-n|-dry-run)	do_dry_run=yes ;;
	-32|-m32)	do_64=32 ;;
	-64|-m64)	do_64=64 ;;
	-ansi)		DEF_WARN="$DEF_WARN -DUSEANSI=1" ;;
	-winmod)	DEF_WARN="$DEF_WARN -DUSEWINMOD=1" ;;
	-install)	do_install=yes ;;
	-quiet)		do_quiet=yes ;;
	-curses)	curses=curses ;;
	-ncurses)	curses=ncurses ;;
	-cursest)	curses="curses -ltermcap" ;;
	-termcap)	curses="termcap" ;;
	-clang)		DEF_CC='clang' ; DEF_WARN="$DEF_WARN -Wno-deprecated-non-prototype" ;;
	-bgcc)		if [ "$is_bgcc" != yes ]
			then
				set_bgcc=yes
				set_bgcc_flags
			fi
			;;
	-x)		set -x ;;
	-*)		echo "$0: Invalid option $1" ; exit ;;
	*)		echo "$0: Invalid parameter $1" ; exit ;;
	esac
	shift
done

if [ "$set_bgcc" = yes ]
then
	save_do_opt="$do_opt"
	save_do_strip="$do_strip"
	save_is_bgcc="$is_bgcc"
	save_is_gcc="$is_gcc"

	# shellcheck disable=SC1091
	. setgcc.sh -bgcc

	do_opt="$save_do_opt"
	do_strip="$save_do_strip"
	is_bgcc="$save_is_bgcc"
	is_gcc="$save_is_gcc"

	CC="bgcc"
	export CC
fi

if [ "$do_debug" = yes ] ; then do_strip=no ; do_opt=no ; fi
if [ "$do_profile" = yes ] ; then do_strip=no ; fi

# 64 bit uses more ram
#if [ -z "$do_64" ]
#then
#	if [ "$(uname -m)" = x86_64 ] ; then do_64=64 ; fi
#fi

if [ "$do_64" = 64 ] ; then DEF_WARN="$DEF_WARN -m64" ; fi

MEEXTRAFLAGS="$DEF_WARN"

if [ "$do_warn" = yes ]
then
	MEEXTRAFLAGS="$MEEXTRAFLAGS $FULL_WARN"
fi

if [ "$do_opt" = yes ]
then
	MEEXTRAFLAGS="$MEEXTRAFLAGS $FULL_OPT"
fi

if [ "$do_static" = yes ]
then
	MEEXTRAFLAGS="$MEEXTRAFLAGS -static"
fi

if [ -n "$SPICE_INC" ] && [ ! -d "$SPICE_INC" ]
then
	echo "$0: SPICE_INC $SPICE_INC directory not found"
	SPICE_INC=
fi

if [ -z "$SPICE_INC" ] && [ ! -d "$SPICE_INC" ]
then
	# Building without SCS spice environment
	MEEXTRAFLAGS="$MEEXTRAFLAGS -D__PORT_H__"
else
	# Building under SCS spice environment
	MEEXTRAFLAGS="$MEEXTRAFLAGS -I$SPICE_INC"
fi

case "$(uname -r)" in
  3.2)	# options for SCO Unix 3.2v4
	if [ "$is_gcc" = no ]
	then
		case "$(uname -s)" in
		SCO_SV)
			;;
		*)
			MEEXTRAFLAGS="$MEEXTRAFLAGS -nointl"
			if [ $do_opt = yes ]
			then MEEXTRAFLAGS="$MEEXTRAFLAGS -Otacl -CSON" ; fi
			;;
		esac
	fi
	;;
esac

case "$(uname -s)" in
UNIX_SV) # options for SCO UnixWare 2.1.2
	MEEXTRAFLAGS="$MEEXTRAFLAGS -D_SYSTYPE_SVR4=1"
	if [ "$do_opt" = yes ]
	then MEEXTRAFLAGS="$MEEXTRAFLAGS -O -Khost,fixed_frame,inline,pentium" ; fi
	;;
FreeBSD|Linux)
	do_pipe=yes
	if [ "$do_opt" = yes ]
	then MEEXTRAFLAGS="$MEEXTRAFLAGS -O3 -fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations" ; fi
	do_mcs=no
	;;
Darwin)
	do_strip=no
	do_pipe=yes
	if [ "$do_opt" = yes ]
	then MEEXTRAFLAGS="$MEEXTRAFLAGS -O3 -fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations" ; fi
	;;
HP-UX)
	case "$(uname -m)" in
	ia64) if [ -z "$LIB_PREFIX" ] && [ -f /usr/lib/hpux64/libc.so ] ; then LIB_PREFIX="/usr/lib/hpux64/lib" ; fi
	esac
	;;
esac

if [ -z "$LIB_PREFIX" ]
then
	if [ "$do_64" = 64 ] && [ -f "/lib64/libc.so" ]
	then
		LIB_PREFIX="/lib64/lib"
	elif [ -f "/lib/libc.a" ] || [ -f "/lib/libc_s" ]
	then
		LIB_PREFIX="/lib/lib"
	elif [ -d "/cygnus/cygwin-b20/H-i586-cygwin32/lib" ]
	then
		LIB_PREFIX="/cygnus/cygwin-b20/H-i586-cygwin32/lib/lib"
	elif [ -d /lib/386 ]
	then
		LIB_PREFIX="/lib/386/Slib"
	else
		LIB_PREFIX="/lib/lib"
	fi
fi

if [ "$do_quiet" != yes ]
then
	echo "LIB_PREFIX is $LIB_PREFIX"
fi

# MEEXTRALIB="-ltermcap -lmalloc -lc_s -lc -lx -lc_s"

MEEXTRALIB=

if [ -z "$curses" ]
then
	if [ -f "${LIB_PREFIX}ncurses.a" ]		|| [ -f "/usr${LIB_PREFIX}ncurses.a" ] || \
		[ -f "${LIB_PREFIX}ncurses.so" ]	|| [ -f "/usr${LIB_PREFIX}ncurses.so" ] || \
		[ -f "${LIB_PREFIX}ncurses.dylib" ]	|| [ -f "/usr${LIB_PREFIX}ncurses.dylib" ]
	then
		# Use the ncurses library
		curses=ncurses
	fi
fi

if [ -z "$curses" ]
then
	if [ -f "${LIB_PREFIX}termcap.a" ] || [ -f "/usr${LIB_PREFIX}termcap.a" ]
	then
		# Use the termcap library
		curses=termcap
	else
		case "$LD_LIBRARY_PATH" in
		*:/usr/ucblib:*)
			if [ -f /usr/ucblib/libtermcap.a ]
			then
				# Use the termcap compatibility library
				curses=termcap
			fi
			;;
		esac
	fi
fi

if [ -z "$curses" ]
then
	if [ -f "${LIB_PREFIX}curses.a" ]		|| [ -f "/usr${LIB_PREFIX}curses.a" ] || \
		[ -f "${LIB_PREFIX}curses.so" ]		|| [ -f "/usr${LIB_PREFIX}curses.so" ] || \
		[ -f "${LIB_PREFIX}curses.dylib" ]	|| [ -f "/usr${LIB_PREFIX}curses.dylib" ] || \
		[ -f "/usr/ccs${LIB_PREFIX}curses.a" ]
	then
		# Use the curses library
		curses=curses
	fi
fi

if [ -n "$curses" ]
then
	MEEXTRALIB="$MEEXTRALIB -l$curses"
fi

if [ "$is_bgcc" = yes ]
then
	# Use bounds checking helper library (not needed for Linux)
	# MEEXTRALIB="$MEEXTRALIB -lcheck"
	:
else
	if [ -f "${LIB_PREFIX}malloc.a" ] || [ -f "/usr${LIB_PREFIX}malloc.a" ]
	then
		# Use fast malloc library
		MEEXTRALIB="$MEEXTRALIB -lmalloc"
	fi

	if [ $do_debug = no ] && [ $do_strip = yes ] && [ -f "${LIB_PREFIX}c_s.a" ]
	then
		# Use shared version of c runline library
		MEEXTRALIB="$MEEXTRALIB -lc_s"
	fi
fi

if [ -f "${LIB_PREFIX}x.a" ]
then
	# SCO needs -lx for nap call
	MEEXTRALIB="$MEEXTRALIB -lc -lx"
fi

#if [ -f "/usr/ucblib/libucb.a" ]
#then
#	if [ "$(uname)" = SunOS ]
#	then
#		# Solaris needs ucb for usleep
#		# Do this last so we do not get more ucb than we want
#		MEEXTRALIB="$MEEXTRALIB -lc /usr/ucblib/libucb.a"
#	fi
#fi

MEEXTRALIB="$MEEXTRALIB $DEF_LIB"

if [ "$is_bgcc" = yes ] && [ -n "$MEEXTRALIB" ]
then
	new_lib=
	for name in $MEEXTRALIB
	do
		case "$name" in
		-lmalloc|-lc|-lc_s)	;;
		*)		new_lib="$new_lib $name"
		esac
	done
	MEEXTRALIB="$new_lib"
fi		

if [ "$do_opt" = yes ] ; then MEEXTRAFLAGS="-O $MEEXTRAFLAGS" ; fi
if [ "$do_strip" = yes ] ; then MEEXTRAFLAGS="$MEEXTRAFLAGS -s" ; fi
if [ "$do_debug" = yes ] ; then MEEXTRAFLAGS="$MEEXTRAFLAGS -g" ; fi
if [ "$do_profile" = yes ] ; then MEEXTRAFLAGS="$MEEXTRAFLAGS -p" ; fi
if [ "$do_pipe" = yes ] ; then MEEXTRAFLAGS="$MEEXTRAFLAGS -pipe" ; fi

if [ "$do_dry_run" = yes ] ; then make_opts="$make_opts -n" ; fi

if [ "$do_jobs" != no ]
then
	num="$(grep processor /proc/cpuinfo | tail -1 | awk '{ print $3 }')"
	case "$num" in
	0) ;;
	1|2) make_opts="$make_opts -j2" ;;
	3) make_opts="$make_opts -j4" ;;
	[1-9]*) make_opts="$make_opts -j$num" ;;
	*) if [ "$do_jobs" = yes ] ; then echo "$0: Unknown cpu count $num" ; make_opts="$make_opts -j2" ; fi ;;
	esac
fi

export MEEXTRAFLAGS MEEXTRALIB

if [ -n "$DEF_CC" ]
then
	CC="$DEF_CC"
	export CC
fi

if [ "$do_quiet" != yes ]
then
	echo "Building emacs with $MAKE $make_opts"
	echo "Compiler: $CC"
	echo "Compile flags: $MEEXTRAFLAGS"
	echo "Link flags: $MEEXTRALIB"
fi

if [ $do_clean = yes ]
then
	$MAKE -f mymakefile clean
fi

if [ "$do_quiet" != yes ]
then
	echo
	echo "Warnings OK for"
	echo "display.c: 1905: mlwrite has variable arguments"
	echo "fileio.c: 383: long/short mismatch in argument to open"
	echo "termio.c: 242: long/short mismatch in argument to kill"
	echo
fi

# shellcheck disable=SC2086
$MAKE $make_opts -f mymakefile

EMACS=emacs
case "$OSTYPE" in
cygwin) EMACS="${EMACS}.exe" ;;
esac

if [ "$do_quiet" != yes ]
then
	ls -l "$EMACS"
fi

if [ "$do_strip" = yes ] && [ -x emacs ] && [ "$is_bgcc" = no ]
then
	if [ -x /usr/bin/strip ] || [ -x /usr/ccs/bin/strip ]
	then
		if [ "$do_quiet" != yes ]
		then
			echo "Stripping..."
		fi
		if [ "$do_dry_run" = yes ]
		then
			echo "strip $EMACS"
		else
			strip "$EMACS"
		fi
		if [ "$do_quiet" != yes ]
		then
			ls -l "$EMACS"
		fi
	fi
	if [ "$do_mcs" != no ] && { [ -x /usr/bin/mcs ] || [ -x /usr/ccs/bin/mcs ] ; }
	then
		if [ "$do_quiet" != yes ]
		then
			echo "Removing comments..."
		fi
		if [ "$do_dry_run" = yes ]
		then
			echo "mcs -d $EMACS"
		else
			mcs -d "$EMACS"
		fi
		if [ "$do_quiet" != yes ]
		then
			ls -l "$EMACS"
		fi
	fi
fi

if [ "$do_install" = yes ] && [ -f emacs ]
then
	$MAKE -f mymakefile install
fi
