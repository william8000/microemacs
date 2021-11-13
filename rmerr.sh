:
# rmerr.sh -- filter out gcc errors

grep -v "warning: return-type defaults to \`int\'" |
grep -v "warning: type of \`f\' defaults to \`int\'" |
grep -v "warning: type of \`n\' defaults to \`int\'" |
grep -v "warning: implicit declaration of function" |
grep -v "was previously declared within a block" |
grep -v "At top level:"
