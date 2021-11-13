if not exist %1.c echo %1 does not exist
if not exist %1.c goto done

del %1.obj

%MYCC% -c -DUSEWINMOD %1.c > emacs.log

type emacs.log

if not exist %1.obj echo "Compile errors"
if not exist %1.obj goto done

tlib wemacs -+ %1

call bccemlin

:done
