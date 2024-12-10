/* Stub for the main emacscmd function in wemod.o in library wemacs.a */

#include "wedum.h"

static const char wemacsdum_what[]
#if (defined(__GCC__) || defined(__GNUC__))
		__attribute__((used))
#endif
		= "@(#)! --- WEmacs stubs";

int emacscmd( char *line, int len )
{
    ((void)line);
    ((void)len);
    return -1;
}

