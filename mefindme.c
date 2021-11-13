/* findme.c -- find callable microemacs through mail$edit */
/* returns 1 if OK, anything else for error */
/* This file may be used by vms application to call an editor */
/* Compile with -DTEST for a simple test main program */

#ifdef vms

#ifdef TEST
#include <stdio.h>
main()
{
	exit(editfile("junk.tmp"));
}
#endif

#include <descrip.h>

/* simplified versions of string functions so callers do not need vaxcrtl */
/* (not for general purpose use) */

static int mystrlen(s)
char *s;
{
	int len;
	len = 0;
	while (s && s[len]) len++;
	return(len);
}

static void mystrcpy(d, s)
char *d, *s;
{
	while( (*d++ = *s++) != '\0') ;
}

static void mystrcat(d, s)
char *d, *s;
{
	while (*d) d++;
	mystrcpy(d, s);
}

static int mystrncmp(a, b, n)
char *a, *b;
int n;
{
	while (n-- > 0)
		if (*a++ != *b++)
			return(1);
	return(0);
}

static int edit_first = 1;		/* tell if we must initialize */

/* Edit a file using a callable editor, defaults to EDT */

int editfile(namestr)
char *namestr;
{
	$DESCRIPTOR(edit, "CALL$EDIT");
	$DESCRIPTOR(mailedit, "MAIL$EDIT");
	$DESCRIPTOR(edittrn, "");
	$DESCRIPTOR(image, "");
	$DESCRIPTOR(symbol, "");
	$DESCRIPTOR(name, "");
	char imagestr[256], symbolstr[256], trnstr[256];
	int i;
	short len;
	static int (*editor)();
	static int is_tpu;
	int EDT$EDIT();

	if (edit_first) {
		editor = 0;
		is_tpu = 0;
		edit_first = 0;

		/* Look for CALL$EDIT */

		edittrn.dsc$a_pointer = trnstr;
		edittrn.dsc$w_length = sizeof(trnstr) - 1;
		i = lib$sys_trnlog(&edit, &len, &edittrn, 0, 0, 0);

		if (i != 1 || len < 10 || mystrncmp(trnstr, "CALLABLE_", 9)) {
			/* Look for MAIL$EDIT */
			edittrn.dsc$a_pointer = trnstr;
			edittrn.dsc$w_length = sizeof(trnstr) - 1;
			i = lib$sys_trnlog(&mailedit, &len, &edittrn, 0, 0, 0);
			if (i != 1)
				len = 0;
		}

		/* Build the name */

		if (len < 10 || len > sizeof(trnstr) ||
				mystrncmp(trnstr, "CALLABLE_", 9))
			len = 0;
		if (len == 0) {
			mystrcpy(trnstr, "CALLABLE_EDT");
			len = mystrlen(trnstr);
		}
		trnstr[len] = '\0';

		is_tpu = (len > 0 && mystrncmp(trnstr,"CALLABLE_TPU",12) == 0);
		mystrcpy(imagestr, &trnstr[9]);
		mystrcat(imagestr, "SHR");
		image.dsc$a_pointer = imagestr;
		image.dsc$w_length = mystrlen(imagestr);

		mystrcpy(symbolstr, &trnstr[9]);
		mystrcat(symbolstr, "$EDIT");
		symbol.dsc$a_pointer = symbolstr;
		symbol.dsc$w_length = mystrlen(symbolstr);

		/* Load the image and get the entry point */

		i = LIB$FIND_IMAGE_SYMBOL(&image, &symbol, &editor);
		if (i != 1 || !editor)
			editor = EDT$EDIT;
	}

	if (!editor)
		return(0);

	name.dsc$a_pointer = namestr;
	name.dsc$w_length = mystrlen(namestr);

	if (is_tpu)
		(*editor)(&name, &name);
	else
		(*editor)(&name, 0, 0, 0, 0, 0, 0, 0);

	return(1);
}

#endif
