del *.obj
del *.lib
del *.log

%MYCC% -c -DUSEWINMOD *.c > emacs.log

type emacs.log

call bccemlib

call bccemlin
