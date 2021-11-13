rem ztcem.bat -- build a stand-alone version of emacs
rem
del *.obj
del *.lib
del *.log

%MYCC% -c *.c > emacs.log

type emacs.log

call ztcemlib

call ztcemln
