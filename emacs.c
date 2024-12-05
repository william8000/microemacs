/*
 * The routine in this file
 * is just a main program.  For the
 * callable interface, the main program must be in its own file.
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	CALLABLE

int main(argc, argv)
int argc;
char *argv[];	{

#if	1
	/* normal entry and exit */
	exit( emacspgm(argc, argv) );
#else
	/* allow -? for testing ram usage with emacsone and RAMSHOW */

	char name[NFILEN];
	char *fgets();

	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == '?') {
		for(;;)	{
			printf("$_Emacs_File:\t");
			if (fgets(name, sizeof(name), stdin) == NULL) exit();
			if (*name == '\0') exit();
			emacsone(name);
			}
		}
	else	{
		exit( emacspgm(argc, argv) );
		}
#endif
}

#endif
