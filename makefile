CFLAGS=		-Otacl -CSON

OFILES=		ansi.o basic.o bind.o buffer.o display.o file.o \
		fileio.o hp150.o meline.o lock.o main.o random.o region.o \
		search.o mespawn.o tcap.o termio.o vt52.o window.o word.o \
		meexec.o eval.o isearch.o input.o crypt.o emacs.o memenu.o

CFILES=		ansi.c basic.c bind.c buffer.c display.c  file.c \
		fileio.c hp150.c meline.c lock.c main.c random.c region.c \
		search.c mespawn.c tcap.c termio.c vt52.c window.c word.c \
		meexec.c eval.c isearch.c input.c crypt.c emacs.c memenu.c

HFILES=		estruct.h edef.h efunc.h epath.h ebind.h evar.h

emacs:		$(OFILES)
		$(CC) $(CFLAGS) $(OFILES) -s -lmalloc -ltermcap -lc -lx -o emacs

$(OFILES):	$(HFILES)
