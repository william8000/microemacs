/*
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if	USG | BSD | (MSDOS & (MSC | GNUC | ZORTECH))
#include	<sys/types.h>
#endif

#if	USG | BSD | (MSDOS & (MSC | TURBO | GNUC | ZORTECH))
#include	<sys/stat.h>
#endif

#if	MSDOS & (C86)
#include <stat.h>
#endif

#if	VMS
#include <stat.h>
#include <fab.h>
#include <rab.h>
#include <rmsdef.h>

/* names for record attributes and record formats */
CONST int rms_maxrat = 3;
CONST char *rms_rat[] = {"ftn", "cr", "prn", "blk"};
CONST int rms_maxrfm = 6;
CONST char *rms_rfm[] =
		/* {"udf",  "fix",  "var", "vfc", "stm", "stmlf", "stmcr"} */
		{"stmlf","stmlf","var", "vfc", "stm", "stmlf", "stmcr"};

#define successful(s)	((s) & 1)
#define unsuccessful(s) (!successful(s))

static struct FAB fab;		/* a file access block */
static struct RAB rab;		/* a record access block */
#endif

#if	USG | FREEBSD2
#include	<fcntl.h>
#endif

#if	FREEBSD2 || defined(__linux__)
#include	<unistd.h>
#endif

#ifndef	O_RDONLY
#define O_RDONLY	0
#endif

#ifndef	O_WRONLY
#define O_WRONLY	1
#endif

#ifndef	O_APPEND
#define O_APPEND	0010
#endif

/* Make getc and putc macroes if they are missing */

#ifndef getc
#define	getc(f)	fgetc(f)
#endif

#ifndef putc
#define	putc(c,f)	fputc(c,f)
#endif

#if defined(_IOFBF) && defined(BUFSIZ) && (defined(M_UNIX) || !defined(M_XENIX))
#define	HAS_SETVBUF	1
#else
#define	HAS_SETVBUF	0
#endif

static FILE *ffp;		/* File pointer, all functions. */
static int eofflag;		/* end-of-file flag */

#define	isbinary		((curbp->b_mode&MDBINARY) != 0)

#if SYMEXPAND

/* expand $ symbols in file names
 *
 * name = { segment }
 * segment = <any character but $>
 *	% maps to the character
 *         | "$$"
 *	% maps to a single "$"
 *	   | "$" name | "$(" name ")" | "${" name "}"
 *	% maps to the environment value of 'name'
 *	% do not count on treatment of trailing blanks in environment symbols
 * name = { <letter or digit or _> }
 *
 * char *symexpand(name)
 * char *name;
 *
 * For an indirect symbol replacement with passes > 1, the first symbol
 * may need to have the second symbol inside $() to produce the desired results.
 *
 * name is left unchanged.
 * symexpand returns a pointer to a static area.
 *
 * For example,
 *   str = symexpand("$(bin)name");
 * where
 *   bin = $(home)bin/
 *   home = /u/william/
 *
 */

static char *symexpand PP((CONSTA char *inname));

static char *symexpand(inname)
CONSTA char *inname;
{
	int i, j, len, addlen, save, more, passes, maxlen;
	char paren;
	char sym[NFILEN];
	CONST char *value;
	static char FAR name[NFILEN];

	maxlen = NFILEN - 1;
	bytecopy(name, inname, maxlen-1);
	passes = 20;
	more = 1;
	while (passes-- > 0 && more) {
		more = 0;
		for(i = 0; i < maxlen && name[i] != '\0'; i++) {
			if (name[i] == '$') {
				more = 1;
				save = i;
				i++;
				if (name[i] == '$') {
					for(j=i; j+1 < maxlen && name[j+1]; j++)
						name[j] = name[j+1];
					name[j] = '\0';
					--i;
					continue;
				}
				j = 0;
				if (name[i] == '(') {paren = ')'; i++;}
				else if (name[i] == '{') {paren = '}'; i++;}
				else paren = 0;
				while (name[i] != '\0' && name[i] != paren &&
					(paren ||
					 isletter(name[i]) ||
					 (name[i] >= '0' && name[i] <= '9') ||
					 name[i] == '_')) {
					if (j < (int) sizeof(sym))
						sym[j++] = name[i];
					i++;
				}
				if (paren != 0 && name[i] == paren)
					i++;
				if (j < (int) sizeof(sym))
					sym[j] = '\0';
				value = getenv(sym);
				if (value == NULL)
					value = "";
				len = (int) strlen(value);
				if (i + len >= maxlen)
					len = maxlen - i - 1;
				addlen = len - (i - save);
				if (addlen > 0) {
					for(j = maxlen-addlen-1; j >= i; j--)
						name[j + addlen] = name[j];
				} else if (addlen < 0) {
					addlen = -addlen;
					for(j = i-addlen; j+addlen < maxlen; j++)
						name[j] = name[j + addlen];
					name[j] = '\0';
				}
				i = save;
				for(j = 0; j < len; j++)
					name[i++] = value[j];
				--i;
			}
		}
		if (i >= maxlen) i = maxlen - 1;
		name[i] = '\0';
	}
	return(name);
}
#endif

/*
 * Open a file for reading.
 */
int ffropen(fn, getstat)
CONSTA char    *fn;
int getstat;
{
#if	VMS | USG | BSD
	int	ffd;		/* File descriptor */
	struct stat s;
#if	VMS
	unsigned long status;
	int maxlen;
#endif

#if	SYMEXPAND
	fn = symexpand(fn);
#endif
#if	VMS
	if (!isbinary) {
		/* initialize structures */
		fab=cc$rms_fab;
		rab=cc$rms_rab;

		fab.fab$l_fna = fn;
		fab.fab$b_fns = strlen(fn);
		fab.fab$b_fac = FAB$M_GET;
		fab.fab$b_shr = FAB$M_SHRGET;
		fab.fab$l_fop = FAB$M_SQO;

		rab.rab$l_fab = &fab;
		rab.rab$l_rop = RAB$M_RAH;	/* read-ahead for multibuffering */

		status=SYS$OPEN(&fab);
		if (status==RMS$_FLK)
		{
			/*
			 * File locking problem:
			 * Add the SHRPUT option, allowing shareability
			 * with other writers. This lets us read batch
			 * logs and stuff like that. I don't turn it on
			 * automatically since adding this sharing
			 * eliminates the read-ahead
			 */
			fab.fab$b_shr |= FAB$M_SHRPUT;
			status=SYS$OPEN(&fab);
		}

		if (unsuccessful(status))
			return(FIOFNF);

		if (unsuccessful(SYS$CONNECT(&rab)))
		{
			SYS$CLOSE(&fab);
			return(FIOFNF);
		}

		if (getstat) {
			if (stat(fn, &s) == 0)
				curbp->b_fmode = s.st_mode;
			curbp->b_fab_rfm = fab.fab$b_rfm;
			curbp->b_fab_rat = fab.fab$b_rat;
			curbp->b_fab_fsz = fab.fab$b_fsz;
			curbp->b_fab_mrs = fab.fab$w_mrs;
		}

		maxlen = fab.fab$w_mrs?fab.fab$w_mrs+1:32768;
		if (maxlen < NSTRING) maxlen = NSTRING;
		if (fline == NULL || flen < maxlen) {
			if (fline != NULL) {
				free(fline);
				fline = NULL;
			}
			fline = malloc(flen = maxlen);
		}

		return(FIOSUC);
	}
#endif

	ffd = open(fn, O_RDONLY);
	if (getstat && fstat(ffd, &s) == 0) {
		curbp->b_fmode = s.st_mode;
#if	VMS
		curbp->b_fab_rfm = s.st_fab_rfm;
		curbp->b_fab_rat = s.st_fab_rat;
		curbp->b_fab_fsz = s.st_fab_fsz;
		curbp->b_fab_mrs = s.st_fab_mrs;
#endif
	}
	if (ffd < 0 || (ffp=fdopen(ffd, "r")) == NULL)
#else
#if MSDOS
	if ((ffp=fopen(fn, (isbinary? "rb": "r"))) == NULL)
#else
	if ((ffp=fopen(fn, "r")) == NULL)
#endif
#endif
                return (FIOFNF);

#if HAS_SETVBUF
	setvbuf(ffp, NULL, _IOFBF, (BUFSIZ > 4096? BUFSIZ: 4096));
#endif

	eofflag = FALSE;
        return (FIOSUC);
}

/*
 * Open a file for writing. Return TRUE if all is well, and FALSE on error
 * (cannot create).
 */
int ffwopen(fn, mode)
CONSTA char    *fn;
CONSTA char *mode;	/* mode to open file for */
{
#if	VMS | USG | BSD
	int	ffd;		/* File descriptor */

#if     VMS
	int i, n;
	char ratstr[80], rfmstr[80];
	unsigned long status;

	if (!isbinary) {
		/* initialize structures */
		fab=cc$rms_fab;
		rab=cc$rms_rab;

		fab.fab$l_fna = fn;
		fab.fab$b_fns = strlen(fn);
		fab.fab$b_fac = FAB$M_PUT;	/* writing this file */
		fab.fab$b_shr = FAB$M_NIL;	/* no other writers */
		fab.fab$l_fop = FAB$M_SQO;	/* sequential ops only */
		fab.fab$b_rat = curbp->b_fab_rat;
		fab.fab$b_rfm = curbp->b_fab_rfm;
		fab.fab$b_fsz = curbp->b_fab_fsz;
		if (fab.fab$b_rfm == FAB$C_FIX)
			fab.fab$w_mrs = curbp->b_fab_mrs;

		rab.rab$l_fab = &fab;
		rab.rab$l_rop = RAB$M_WBH;	/* write behind - multibuffer */

		if (*mode == 'a')
		{
			/* append mode */
			rab.rab$l_rop = RAB$M_EOF;
			status=SYS$OPEN(&fab);
			if (status == RMS$_FNF)
				status=SYS$CREATE(&fab);
		}
		else	/* *mode == 'w' */
		{
			/* write mode */
			fab.fab$l_fop |= FAB$M_MXV; /* always make a new version */
			status=SYS$CREATE(&fab);
		}

		if (successful(status))
		{
			status=SYS$CONNECT(&rab);
			if (unsuccessful(status)) SYS$CLOSE(&fab);
		}

		if (unsuccessful(status)) {
			mlwrite("Cannot open file for writing [RMS %d]",status);
			return(FIOERR);
		}
		return(FIOSUC);
	}

	n = curbp->b_fab_rfm;
	if (n < 0 || n > rms_maxrfm) n = 2;
	sprintf(rfmstr, "rfm=%s", rms_rfm[ n ]);

	n = 0;
	strcpy(ratstr, "rat=");
	for(i = 0; i <= rms_maxrat; i++)
		if (curbp->b_fab_rat & (1 << i))	{
			if (n++ > 0) strcat(ratstr, ",");
			strcat(ratstr, rms_rat[i]);
		}
	if (n == 0) strcat(ratstr, rms_rat[1]);

	ffd = creat(fn, curbp->b_fmode, rfmstr, ratstr);
#else

#if	SYMEXPAND
	fn = symexpand(fn);
#endif
	ffd = (*mode == 'w'?
		creat(fn, curbp->b_fmode):
		open(fn, (int) (O_WRONLY|O_APPEND), curbp->b_fmode));
#endif
	if (ffd < 0 || (ffp=fdopen(ffd, mode)) == NULL) {
#else
#if MSDOS
	char	newmode[3];

	newmode[0] = mode[0];
	newmode[1] = (isbinary? 'b': '\0');
	newmode[2] = '\0';
	if ((ffp=fopen(fn, newmode)) == NULL) {
#else
	if ((ffp=fopen(fn, mode)) == NULL) {
#endif
#endif
                mlwrite("Cannot open file for writing");
                return (FIOERR);
        }

#if HAS_SETVBUF
	setvbuf(ffp, NULL, _IOFBF, (BUFSIZ > 4096? BUFSIZ: 4096));
#endif

        return (FIOSUC);
}

/*
 * Close a file. Should look at the status in all systems.
 */

static int ffclose PP(( int for_writing ));

static int ffclose(for_writing)
int for_writing;
{
#if	VMS
	unsigned long status;
#endif

	/* free this since we do not need it anymore */
	if (fline) {
		free(fline);
		fline = NULL;
	}

#if	MSDOS & CTRLZ
	if (for_writing) {
		putc(26, ffp);		/* add a ^Z at the end of the file */
	}
#endif

#if	VMS
	if (!isbinary) {
		status = SYS$DISCONNECT(&rab);
		if (successful(status)) status = SYS$CLOSE(&fab);
		else SYS$CLOSE(&fab);

		if (unsuccessful(status)) {
			mlwrite("Error closing file [RMS %d]", status);
			return(FIOERR);
		}
		return(FIOSUC);
	}
#endif
	
#if     V7 | USG | BSD | (MSDOS & (LATTICE | MSC | TURBO | C86 | GNUC | ZORTECH))
        if (fclose(ffp) != FALSE) {
                mlwrite("Error closing file");
                return(FIOERR);
        }

        if (for_writing) {
#if	MSDOS
#if	!GNUC
		bdos(0x0D,0,0);		/* force MSDOS file system update */
#endif
#endif
#if	V7 | USG | BSD
#if	defined(__sun)
		/* skip sync */
#else
#if	defined(__INTERIX) || defined(__MINGW32__)
		/* sync not present */
#else
        if ( dosync ) {
		sync();			/* force Unix file system update */
		sync();
        }
#endif
#endif
#endif
	}
        return(FIOSUC);
#else
        fclose(ffp);
        return (FIOSUC);
#endif
}

/*
 * Close a file opened for reading
 */
int ffrclose()
{
	return ffclose( 0 );
}

/*
 * Close a file opened for writing
 */
int ffwclose()
{
	return ffclose( 1 );
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
int ffputline(buf, nbuf)
char    *buf;
int	nbuf;
{
        register int    i;
        char *obuf;

	obuf = buf;

#if	CRYPT
	if (cryptflag) {
		/* get a reasonable buffer */
		if (fline && flen < nbuf)
		{
			free(fline);
			fline = NULL;
		}

		if (fline == NULL &&
		    (fline=malloc(flen = nbuf+NSTRING)) == NULL)
				return(FIOMEM);

		/* copy data */
		for(i = 0; i < nbuf; i++)
			fline[i] = buf[i];

		/* repoint output buffer */
		obuf=fline;

		me_crypt(obuf, nbuf);
	}
#endif

#if	VMS
	if (!isbinary) {
		/* set output buffer */
		rab.rab$l_rbf = obuf;
		rab.rab$w_rsz = nbuf;

		i = SYS$PUT(&rab);
		if (unsuccessful(i)) {
			mlwrite("Write I/O error [RMS %d]", i);
			return(FIOERR);
		}

		return(FIOSUC);
	}
#endif

#if USG | BSD | MSDOS
	if (nbuf > 0)
		fwrite(obuf, 1, nbuf, ffp);
#else
	for (i = 0; i < nbuf; ++i)
		putc(obuf[i]&0xFF, ffp);
#endif

#if	ST520
        putc('\r', ffp);
#endif        
        putc('\n', ffp);

        if (ferror(ffp)) {
                mlwrite("Write I/O error");
                return (FIOERR);
        }

        return (FIOSUC);
}

/*
 * Read a line from a file, and store the bytes in the supplied buffer. The
 * "nbuf" is the length of the buffer. Complain about long lines and lines
 * at the end of the file that don't have a newline present. Check for I/O
 * errors too. Return status.
 */
int ffgetline()
{
        register int c;		/* current character read */
        register int i;		/* current index into fline */
	register char *tmpline;	/* temp storage for expanding line */
	int tmplen;
	FILE *fp;
#if	VMS
	unsigned long status;
#endif

	frlen = 0;

	fp = ffp;		/* avoid reference to extern variable */

#if	VMS
	if (!isbinary) {
		if (fline == NULL)
			return(FIOMEM);

		/* read the line in */
		rab.rab$l_ubf=fline;
		rab.rab$w_usz=flen;

		status=SYS$GET(&rab);
		if (status == RMS$_EOF) return(FIOEOF);
		if (unsuccessful(status)) {
			mlwrite("File read error [RMS %d]", status);
			return(FIOERR);
		}

		/* terminate and decrypt the string */
		frlen = rab.rab$w_rsz;
		fline[frlen] = 0;
#if	CRYPT
		if (cryptflag)
			me_crypt(fline, frlen);
#endif
		return(FIOSUC);
	}
#endif

	/* if we are at the end...return it */
	if (eofflag)
		return(FIOEOF);

	/* dump fline if it ended up too big */
	if (sizeof(int) < 4 && flen > 2 * NSTRING && fline != NULL) {
		free(fline);
		fline = NULL;
	}

	/* if we don't have an fline, allocate one */
	if (fline == NULL)
		if ((fline = malloc(flen = NSTRING)) == NULL)
			return(FIOMEM);

	/* read the line in */
        i = 0;

#if defined(M_UNIX)
	/* stdio hack, probably not worth the effort... */
	if ((fp->__flag & (_IORW | _IOREAD)) == 0) {
#if !defined(_SCO_DS)
		errno = _EBADF;
#endif
		c = EOF;
	} else {
		tmplen = fp->__cnt;
		if (tmplen >= flen) tmplen = flen-1;
		while (i < tmplen && fp->__ptr[i] != '\n') {
			i++;
		}
		if (i > 0) {
			memcpy(fline, fp->__ptr, i);
			fp->__cnt -= i;
			fp->__ptr += i;
		}
		if (i < tmplen) {
			c = '\n';
			fp->__cnt--;
			fp->__ptr++;
		} else {
			while (--fp->__cnt < 0 ?
				((c = _filbuf(fp)) != EOF && c != '\n') :
				((c = (int) *fp->__ptr++) != '\n'))
#else
	while ((c = getc(fp)) != EOF && c != '\n')
#endif
        {
                fline[i++] = (char) c;
		/* if it's longer, get more room */
                if (i >= flen) {
			tmplen = (sizeof(int) < 4? flen+NSTRING: flen+flen);
			if ((tmpline = malloc(tmplen)) == NULL)
				return(FIOMEM);
			if (flen > 0) memcpy(tmpline, fline, flen);
			i = flen;
			flen = tmplen;
			free(fline);
			fline = tmpline;
                }
        }
#if defined(M_UNIX)
	}
	}
#endif

#if	ST520
	if(fline[i-1] == '\r')
		i--;
#endif

	/* test for any errors that may have occured */
        if (c == EOF) {
                if (ferror(fp)) {
                        mlwrite("File read error");
                        return(FIOERR);
                }

                if (i != 0)
			eofflag = TRUE;
		else
			return(FIOEOF);
        }

	/* terminate and decrypt the string */
	frlen = i;
        fline[i] = 0;

#if	CRYPT
	if (cryptflag)
		me_crypt(fline, frlen);
#endif
        return(FIOSUC);
}

int fexist(fname)	/* does <fname> exist on disk? */

char *fname;		/* file to check for existance */

{
	FILE *fp;

#if	SYMEXPAND
	fname = symexpand(fname);
#endif

	/* try to open the file for reading */
	fp = fopen(fname, "r");

	/* if it fails, just return false! */
	if (fp == NULL)
		return(FALSE);

	/* otherwise, close it and report true */
	fclose(fp);
	return(TRUE);
}

#if	AZTEC & MSDOS
#undef	fgetc
/*	a1getc:		Get an ascii char from the file input stream
			but DO NOT strip the high bit
*/

int a1getc(fp)

FILE *fp;

{
	int c;		/* translated character */

	c = getc(fp);	/* get the character */

	/* if its a <LF> char, throw it out  */
	while (c == 10)
		c = getc(fp);

	/* if its a <RETURN> char, change it to a LF */
	if (c == '\r')
		c = '\n';

	/* if its a ^Z, its an EOF */
	if (c == 26)
		c = EOF;

	return(c);
}
#endif

#if	ABACKUP

/*
 * Test if a file is newer than its auto-saved version
 */

int ffisnewer(fn)
char    *fn;
{
#if	VMS | USG | BSD | (MSDOS & (MSC | TURBO | C86 | GNUC | ZORTECH))
	char asvname[NFILEN + 4];
#if	C86
	struct fbuf asvstat, fnstat;
#else
	struct stat asvstat, fnstat;
#endif

	makeasvname(asvname, fn);
	if (stat(asvname, &asvstat) != 0) return(TRUE);
	if (stat(fn, &fnstat) != 0) return(FALSE);
	if (asvstat.st_ctime > fnstat.st_ctime) return(FALSE);
#endif
	return(TRUE);
}

#endif

#ifdef NEEDCOMPL
#undef NEEDCOMPL
#endif

#define NEEDCOMPL	1

/*	FILE Directory routines		*/

#if	(BCOMPL & (USG | BSD))

#undef NEEDCOMPL
#define NEEDCOMPL	0

#if USG | FREEBSD2
#   if USGV3 || FREEBSD2 || defined(__AUX__) || defined(sun)
#       include	<dirent.h>
#       define	direct	dirent
#    else
#       include	<sys/ndir.h>
#    endif
#else
#   include	<sys/dir.h>
#endif

static DIR *dirptr = NULL; /* pointer to the current directory being searched */

static char FAR path[NFILEN];	/* path of file to find */
static char FAR rbuf[NFILEN];	/* return file buffer */
static char *nameptr;		/* ptr past end of path in rbuf */
static int namelen;		/* length we can write into nameptr */
static int zapdot;		/* true if ./ must be removed */
#if SYMEXPAND
static int pathlen;		/* length of expanded path */
#endif
static int in_arg;
static char FAR argbuf[NFILEN];
static int arglen;

/*	do a wild card directory search (for file name completion) */

char *getffile(fspec, match_len, exact, is_arg)

CONSTA char *fspec;	/* pattern to match */
int match_len;
int exact;
int is_arg;

{
	register int index;		/* index into various strings */

	/* first parse the file path off the file spec */
	bytecopy(path, fspec, NFILEN-2);
	in_arg = is_arg;
	if (is_arg) {
		/* only for testing expandarg under unix */
		index = 0;
		while (path[index] != '\0' && path[index] != '*')
			index++;
		path[index] = '\0';
		strcpy(argbuf, path);
		arglen = index;
	}
	index = (int) (strlen(path) - 1);
	while (index >= 0 && path[index] != '/' &&
			path[index] != '\\' && path[index] != ':')
		--index;
	zapdot = (index < 0);
	if (zapdot) { index = 0; path[0] = '.'; }
	if (path[index] == '/') {
		if (index > 0) --index;
		else { zapdot = 1; index = 1; path[1] = '.'; }
	}
	path[index+1] = '\0';

	if (path[0] == '/' && path[1] == '/')
		return(NULL);

	/* open the directory pointer */
	if (dirptr != NULL) {
		closedir(dirptr);
		dirptr = NULL;
	}
	strcpy(rbuf, path);
#if SYMEXPAND
	strcpy(path, symexpand(path));
#endif
	dirptr = opendir(path);
	if (dirptr == NULL)
		return(NULL);

	rbuf[index+1] = '/';
	nameptr = &rbuf[index+2];
	namelen = NFILEN - 4 - index;
#if SYMEXPAND
	pathlen = (int) strlen(path);
	path[pathlen] = '/';
	if (pathlen > index+1) namelen = NFILEN - 3 - pathlen;
#endif
	if (namelen <= 0)
		return(NULL);

	/* and call for the first file */
	return(getnfile(fspec, match_len, exact));
}

/*
** Find the next name
**	match_name, match_len and exact are only suggestions to reduce the
**		number of calls to stat().
**	The caller is responsible for doing the full comparisons.
*/

char *getnfile(match_name, match_len, exact)
CONSTA char *match_name;
int match_len;
int exact;

{
	register struct direct *dp;	/* directory entry pointer */
	register int index;		/* index into various strings */
	struct stat fstatus;
	char *rptr;

	if (match_len > 0 && match_name[ match_len-1 ] == '/') {
		match_len--;
	}

	/* and call for the next file */
nxtdir:	dp = readdir(dirptr);
	if (dp == NULL)
		return(NULL);

	/* check to make sure we skip directory entries */
	bytecopy(nameptr, dp->d_name, namelen-1);

#if SYMEXPAND
	bytecopy(&path[pathlen+1], dp->d_name, namelen-1);
#endif

	/* strip leading './' or '/.' */
	rptr = rbuf;
	if (zapdot != 0 && rptr[0] != '\0' && rptr[1] != '\0') rptr += 2;

	if (in_arg && arglen > 0) {
		if (strncmp(rptr, argbuf, arglen) != 0)
			goto nxtdir;
	}

	if (match_len > 0) {
		if (exact) {
			if (strncmp(rptr, match_name, match_len) != 0) {
				goto nxtdir;
			}
		} else {
			if (strcompare(rptr, match_name, match_len) != 0) {
				goto nxtdir;
			}
		}
	}
#if SYMEXPAND
	if (stat(path, &fstatus) == -1)
		goto nxtdir;
#else
	if (stat(rbuf, &fstatus) == -1)
		goto nxtdir;
#endif
	if ((fstatus.st_mode & S_IFMT) == S_IFDIR) {
		index = (int) strlen(nameptr);
		nameptr[index] = '/';
		nameptr[index+1] = '\0';
	} else if ((fstatus.st_mode & S_IFMT) != S_IFREG) {
		goto nxtdir;
	}

	/* return the next file name! */
	return(rptr);
}
#endif

#if	(BCOMPL & (TURBO | GNUC | ZORTECH))

#undef NEEDCOMPL
#define NEEDCOMPL	0

#if ZORTECH
struct FIND *fileblock;	/* structure for directory searches */
#else
#include <dir.h>
struct ffblk fileblock;	/* structure for directory searches */
#endif

char path[NFILEN];	/* path of file to find */
char rbuf[NFILEN];	/* return file buffer */

char *getffile(fspec, match_len, exact, is_arg)

CONSTA char *fspec;	/* pattern to match */
int match_len;
int exact;
int is_arg;

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(path) - 1;
	while (index >= 0 && (path[index] != '/' &&
				path[index] != '\\' && path[index] != ':'))
		--index;
	path[index+1] = 0;

	/* check for an extension */
	point = strlen(fspec) - 1;
	extflag = FALSE;
	while (point >= 0) {
		if (fspec[point] == '.') {
			extflag = TRUE;
			break;
		}
		point--;
	}

	/* construct the composite wild card spec */
	strcpy(fname, path);
	strcat(fname, &fspec[index+1]);
	if (!is_arg) strcat(fname, "*");
	if (extflag == FALSE)
		strcat(fname, ".*");

	/* and call for the first file */
#if	ZORTECH
	fileblock = findfirst(fname, 0);
	if (!fileblock)
		return(NULL);
#else
	if (findfirst(fname, &fileblock, 0) == -1)
		return(NULL);
#endif

	/* return the first file name! */
	strcpy(rbuf, path);
#if	ZORTECH
	strcat(rbuf, fileblock->name);
#else
	strcat(rbuf, fileblock.ff_name);
#endif
	mklower(rbuf);
	return(rbuf);
}

char *getnfile(match_name, match_len, exact)
CONSTA char *match_name;
int match_len;
int exact;

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* and call for the next file */
#if	ZORTECH
	fileblock = findnext();
	if (!fileblock)
		return(NULL);
#else
	if (findnext(&fileblock) == -1)
		return(NULL);
#endif

	/* return the file name! */
	strcpy(rbuf, path);
#if	ZORTECH
	strcat(rbuf, fileblock->name);
#else
	strcat(rbuf, fileblock.ff_name);
#endif
	mklower(rbuf);
	return(rbuf);
}
#endif

#if	(BCOMPL & VMS)

#undef NEEDCOMPL
#define NEEDCOMPL	0

#include <descrip.h>
#include <rmsdef.h>

static char FAR path[NFILEN];	/* path of file to find */
static char FAR rbuf[NFILEN];	/* return file buffer */
static char FAR patbuf[NFILEN];	/* buffer for LIB$FIND_FILE pattern */
static char FAR resbuf[NFILEN];	/* buffer for LIB$FIND_FILE result */
static struct dsc$descriptor_s	patdesc, resdesc;
static long context = 0;
static int namelen;		/* length in rbuf after path */
static int verflag;		/* does the file have a version? */
static int matchcount;		/* count of files matched */
static int directflag;		/* in directory part? */
static int zapzeros;		/* zap added zeros */

static int in_arg;

char *getffile(orgfspec, match_len, exact, is_arg)

CONSTA char *orgfspec;	/* pattern to match */
int match_len;
int exact;
int is_arg;

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	int lbflag, rbflag;		/* check for left and right brackets */
	int i;
	char fspec[NFILEN];		/* local copy of fspec */
	char *p;

	in_arg = is_arg;
	if (context) {
		lib$find_file_end(&context);
		context = 0;
	}
	/* copy fspec, and leave some room */
	bytecopy(fspec, orgfspec, NFILEN-16);
	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(path) - 1;
	while (index >= 0 && (path[index] != ']' && path[index] != ':'))
		--index;
	path[index+1] = 0;

	namelen = NFILEN - index - 2;
	matchcount = 0;

	/* check for an extension */
	i = strlen(fspec) - 1;
	point = 0;
	extflag = verflag = lbflag = rbflag = zapzeros = FALSE;
	while (i >= 0) {
		if (fspec[i] == ';' && !rbflag)
			verflag = TRUE;
		if (fspec[i] == '.' && !extflag && !rbflag) {
			point = i;
			extflag = TRUE;
		}
		if (fspec[i] == ':')
			break;
		if (fspec[i] == ']')
			rbflag = TRUE;
		if (fspec[i] == '[') {
			lbflag = TRUE;
			break;
		}
		i--;
	}

	/* construct the composite wild card spec */
	directflag = (lbflag && !rbflag);
	if (directflag && extflag) fspec[point] = ']';
	strcpy(patbuf, path);
	strcat(patbuf, &fspec[index+1]);
	if (directflag) {
		if (!extflag) {
			/* convert log:[x -> log:[000000]x */
			p = strchr(patbuf, '[');
			if (p != NULL) {
				strcpy(resbuf, p+1);
				strcpy(p+1, "000000]");
				strcat(patbuf, resbuf);
				zapzeros = TRUE;
			}
		}
		strcat(patbuf, "*.dir;1");
	} else {
		if (!is_arg) strcat(patbuf, "*");
		if (extflag == FALSE) strcat(patbuf, ".*");
		if (verflag == FALSE) strcat(patbuf, ";0");
	}

	/*d printf("pattern %s\n", patbuf); */
	patdesc.dsc$a_pointer = patbuf;
	patdesc.dsc$w_length = strlen(patbuf);
	patdesc.dsc$b_dtype = DSC$K_DTYPE_T;
	patdesc.dsc$b_class = DSC$K_CLASS_S;

	resdesc.dsc$a_pointer = resbuf;
	resdesc.dsc$w_length = sizeof(resbuf) - 2;
	resdesc.dsc$b_dtype = DSC$K_DTYPE_T;
	resdesc.dsc$b_class = DSC$K_CLASS_S;

	/* and call for the first file */
	return(getnfile(orgfspec, match_len, exact));
}

char *getnfile(match_name, match_len, exact)
CONSTA char *match_name;
int match_len;
int exact;

{
	char *p, *s;
	int stat, len;

	stat = lib$find_file(&patdesc, &resdesc, &context);
	if ((stat != RMS$_NORMAL && (++matchcount == 1 || stat != RMS$_SYN)) ||
	    context == 0)
		return(NULL);

	if (in_arg) {
		strcpy(rbuf, resbuf);
		goto checksemi;
	}
	strcpy(rbuf, path);
	resbuf[namelen-1] = '\0';
	p = strchr(resbuf, ']');
	if (directflag && p != NULL) {
		/* convert log:[a.b]c.dir;n -> [a.b.c] */
		*p = '.';
		p = strchr(p+1, '.');
		if (p != NULL)	{ *p = ']'; *(p+1) = '\0'; }
		if (zapzeros) {
			/* convert log:[000000.x] -> log:[x] */
			p = strchr(resbuf, '[');
			if (p != NULL && strlen(p) > 8) {
				do {
					p++;
					*p = *(p+7);
				} while (*p != '\0');
			}
		}
		p = NULL;
	}
	if (p == NULL) p = strchr(resbuf, ':');
	if (p == NULL)
		strcat(rbuf, resbuf);
	else
		strcat(rbuf, p + 1);
checksemi:
	if (verflag == FALSE && directflag == FALSE) {
		p = strchr(rbuf, ';');
		if (p != NULL)
			*p = '\0';
	}

	mklower(rbuf);
	/*d printf("ret %s ", rbuf); */
	return(rbuf);
}
#endif

#if NEEDCOMPL
char *getffile(fspec, match_len, exact, is_arg)

CONSTA char *fspec;	/* file to match */
int match_len;
int exact;
int is_arg;

{
	return(NULL);
}

char *getnfile(match_name, match_len, exact)
CONSTA char *match_name;
int match_len;
int exact;

{
	return(NULL);
}
#endif

#if	VMS && defined(vax11c)
static char *strstr(a, b)
CONSTA char *a;
CONSTA char *b;
{
	int len;

	if (!a || !b) return((char *)0);
	if (!*b) return(a);
	len = strlen(b);
	while (*a) {
		if (!strncmp(a, b, len)) return(a);
		a++;
	}
	return((char *)0);
}		
#endif

#if	VMS | MSDOS
/***********************************************************
* FUNCTION - addspec - utility function for expandargs
***********************************************************/

#if	VMS
#define ADDSPEC_INCREMENT 100
#else
#define ADDSPEC_INCREMENT 20
#endif

static void PASCAL NEAR addspec(arg, arglen, pargc, pargv, pargcapacity)

char *arg;
int arglen;		/* len of -1 means no alloc needed */
int *pargc;
char ***pargv;
int *pargcapacity;
{
    char *s;

    /* reallocate the argument array if necessary */

    if (*pargc >= *pargcapacity) {
	*pargcapacity += ADDSPEC_INCREMENT;

        if (*pargv)
            *pargv = (char **) realloc(*pargv, sizeof(**pargv)*(*pargcapacity));
        else
            *pargv = (char **) malloc(sizeof(**pargv)*(*pargcapacity));
    }

    if (!*pargv)
	return;

    if (arglen >= 0) {
	/* allocate new argument */
	s = malloc(arglen + 1);
	if (!s)
	    return;
	strncpy(s, arg, arglen);
	s[arglen] = 0;
    } else {
	s = arg;
    }

    /* put into array */
    (*pargv)[(*pargc)++] = s;
}

/***********************************************************
* FUNCTION - expandargs - massage argc and argv to expand wildcards
***********************************************************/

VOID PASCAL NEAR expandargs(pargc, pargv)

int *pargc;
char ***pargv;
{
    int argc;
    char **argv;
    int nargc;
    char **nargv;
    int nargcapacity;
    char *name, *filespec;

    argc = *pargc;
    argv = *pargv;

    nargc = 0;
    nargv = NULL;
    nargcapacity = 0;

    /* loop over all arguments */
    while (argc--)
    {
    	name = *argv;

	/* should check for wildcards: %, *, and "..." */
#if	VMS
	if (*name != '-' && (strchr(name,'%') || strchr(name,'*') ||
			 	strstr(name,"...")))
#else
	if (*name != '-' && (strchr(name,'?') || strchr(name,'*')))
#endif
	{
	    filespec = getffile(name, 0, FALSE, TRUE);
	    if (filespec == NULL) {
	    	addspec(name, -1, &nargc, &nargv, &nargcapacity);
	    } else {
	    	do {
		    addspec(filespec, strlen(filespec),
				&nargc, &nargv, &nargcapacity);
		    filespec = getnfile(NULL, 0, FALSE);
		} while (filespec != NULL);
	    }
	} else {
	    addspec(name, -1, &nargc, &nargv, &nargcapacity);
	}
        argv++;
    }

    *pargc = nargc;
    *pargv = nargv;
}

#else

VOID PASCAL NEAR expandargs(pargc, pargv)
int *pargc;
char ***pargv;
{
    UNUSED_ARG(pargc);
    UNUSED_ARG(pargv);
    /* nothing */
}
#endif

