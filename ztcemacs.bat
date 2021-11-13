rem ztcemacs.bat -- build the wemacs library and its test main program
rem
del *.obj
del *.lib
del *.log

%MYCC% -c -DUSEWINMOD *.c > emacs.log

type emacs.log

call ztcemlib

call ztcemlin
