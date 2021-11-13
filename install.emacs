:
# install.emacs -- install emacs

B=/usr/local/bin
E=emacs
U=$B/u$E
G=$B/g$E

if [ -x $E -a -x $U -a ! -x $B/$E ]
then
	if [ -f $U- ] ; then rm -f $U- ; fi
	mv $U $U-
	cp $E $U
	ls -l $E ${U}*
else
	$HOME/bin/install.sh -b . $E
	if [ -x $B/$E -a -x $U -a -x $G ]
	then
		if [ -f $U- ] ; then rm -f $U- ; fi
		mv $U $U-
		ln $B/$E $U
		ls -l ${U}*
	fi
fi
