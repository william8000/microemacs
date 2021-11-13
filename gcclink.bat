ar uv me.a ansi.o basic.o bind.o buffer.o crypt.o display.o dolock.o emacs.o
ar uv me.a eval.o exec.o file.o fileio.o ibmpc.o input.o isearch.o line.o
ar uv me.a lock.o menu.o random.o region.o search.o spawn.o termio.o
ar uv me.a window.o word.o

gcc %1 %2 %3 %4 %5 %6 %7 %8 main.o me.a -lpc
