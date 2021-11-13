rem ztcemlib.bat -- build a library of emacs modules
rem
del *.lib
del v:*.lib

%MYLIB% v:wemacs /c + wmod + main + basic + bind + buffer + display + file;
%MYLIB% v:wemacs + fileio + line + random + region + search + mespawn + termio;
%MYLIB% v:wemacs + window + word + meexec + eval + isearch + input + crypt + memenu;
xcopy v:wemacs.lib .
