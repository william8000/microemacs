rem set gccopt=-pedantic -O -Wall
set gccopt=-O
rem set gccopt=-g
rem set gccopt=-O -pg

set gccopt=%gccopt% %1 %2 %3 %4 %5 %6 %7 %8 -c

echo Starting compile %gccopt%
del *.o
del me.a
gcc %gccopt% ansi.c
gcc %gccopt% basic.c
gcc %gccopt% bind.c
gcc %gccopt% buffer.c
gcc %gccopt% crypt.c
gcc %gccopt% display.c
gcc %gccopt% dolock.c
gcc %gccopt% emacs.c
gcc %gccopt% eval.c
gcc %gccopt% exec.c
gcc %gccopt% file.c
gcc %gccopt% fileio.c
gcc %gccopt% ibmpc.c
gcc %gccopt% input.c
gcc %gccopt% isearch.c
gcc %gccopt% lock.c
gcc %gccopt% main.c
gcc %gccopt% meline.c
gcc %gccopt% menu.c
gcc %gccopt% random.c
gcc %gccopt% region.c
gcc %gccopt% search.c
gcc %gccopt% spawn.c
gcc %gccopt% termio.c
gcc %gccopt% window.c
gcc %gccopt% word.c

gcclink  %1 %2 %3 %4 %5 %6 %7 %8
