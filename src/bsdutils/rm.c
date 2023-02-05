// Modified from the original by the rmw project
// The original source is at
// https://github.com/dcantrell/bsdutils
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)rm.c	8.5 (Berkeley) 4/18/94";
#endif /* not lint */
#endif

#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <grp.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "compat.h"

static int dflag, eval, fflag, iflag, vflag, stdin_ok;
static int xflag;
static uid_t uid;
static volatile sig_atomic_t info;

static int	check(const char *, const char *, struct stat *);
static void	rm_tree(char * const *);
static void siginfo(int __attribute__((unused)));

/*
 * rm --
 *	This rm is different from historic rm's, but is expected to match
 *	POSIX 1003.2 behavior.	The most visible difference is that -f
 *	has two specific effects now, ignore non-existent files and force
 *	file removal.
 */
int
bsdutils_rm(const char *const argv, bool want_verbose)
{
	(void)setlocale(LC_ALL, "");

	iflag = dflag = fflag = 0;

	vflag = want_verbose ? true : false;

	uid = geteuid();

	(void)signal(SIGINFO, siginfo);

  char *const fake_fts_array[] = {
		(char *const)argv,
		NULL
	};

	rm_tree(fake_fts_array);

	return (eval);
}

static void
rm_tree(char * const * argv)
{
	FTS *fts;
	FTSENT *p;
	int needstat;
	int flags;
	int rval;

	/*
	 * Remove a file hierarchy.  If forcing removal (-f), or interactive
	 * (-i) or can't ask anyway (stdin_ok), don't stat the file.
	 */
	needstat = !uid || (!fflag && !iflag && stdin_ok);

	/*
	 * If the -i option is specified, the user can skip on the pre-order
	 * visit.  The fts_number field flags skipped directories.
	 */
#define	SKIPPED	1

	flags = FTS_PHYSICAL;
	if (!needstat)
		flags |= FTS_NOSTAT;
	if (xflag)
		flags |= FTS_XDEV;
	if (!(fts = fts_open(argv, flags, NULL))) {
		if (fflag && errno == ENOENT)
			return;
		err(1, "fts_open");
	}
	while (errno = 0, (p = fts_read(fts)) != NULL) {
		switch (p->fts_info) {
		case FTS_DNR:
			if (!fflag || p->fts_errno != ENOENT) {
				warnx("%s: %s",
				    p->fts_path, strerror(p->fts_errno));
				eval = 1;
			}
			continue;
		case FTS_ERR:
			errx(1, "%s: %s", p->fts_path, strerror(p->fts_errno));
		case FTS_NS:
			/*
			 * Assume that since fts_read() couldn't stat the
			 * file, it can't be unlinked.
			 */
			if (!needstat)
				break;
			if (!fflag || p->fts_errno != ENOENT) {
				warnx("%s: %s",
				    p->fts_path, strerror(p->fts_errno));
				eval = 1;
			}
			continue;
		case FTS_D:
			/* Pre-order: give user chance to skip. */
			if (!fflag && !check(p->fts_path, p->fts_accpath,
			    p->fts_statp)) {
				(void)fts_set(fts, p, FTS_SKIP);
				p->fts_number = SKIPPED;
			}
			continue;
		case FTS_DP:
			/* Post-order: see if user skipped. */
			if (p->fts_number == SKIPPED)
				continue;
			break;
		default:
			if (!fflag &&
			    !check(p->fts_path, p->fts_accpath, p->fts_statp))
				continue;
		}

		/*
		 * If we can't read or search the directory, may still be
		 * able to remove it.  Don't print out the un{read,search}able
		 * message unless the remove fails.
		 */
		switch (p->fts_info) {
		case FTS_DP:
		case FTS_DNR:
			rval = rmdir(p->fts_accpath);
			if (rval == 0 || (fflag && errno == ENOENT)) {
				if (rval == 0 && vflag)
					(void)printf("%s\n",
					    p->fts_path);
				if (rval == 0 && info) {
					info = 0;
					(void)printf("%s\n",
					    p->fts_path);
				}
				continue;
			}
			break;
		case FTS_NS:
			/*
			 * Assume that since fts_read() couldn't stat
			 * the file, it can't be unlinked.
			 */
			if (fflag)
				continue;
			/* FALLTHROUGH */
		case FTS_F:
		case FTS_NSOK:
		default:
			rval = unlink(p->fts_accpath);
			if (rval == 0 || (fflag && errno == ENOENT)) {
				if (rval == 0 && vflag)
					(void)printf("%s\n",
					    p->fts_path);
				if (rval == 0 && info) {
					info = 0;
					(void)printf("%s\n",
					    p->fts_path);
				}
				continue;
			}
		}
		warn("%s", p->fts_path);
		eval = 1;
	}
	if (!fflag && errno)
		err(1, "fts_read");
	fts_close(fts);
}


static int
check(const char *path, const char *name, struct stat *sp)
{
	int ch, first;
	char modep[15];
	struct passwd *pw = NULL;
	struct group *gr = NULL;

	/* Check -i first. */
	if (iflag)
		(void)fprintf(stderr, "remove %s? ", path);
	else {
		/*
		 * If it's not a symbolic link and it's unwritable and we're
		 * talking to a terminal, ask.  Symbolic links are excluded
		 * because their permissions are meaningless.  Check stdin_ok
		 * first because we may not have stat'ed the file.
		 */
		if (!stdin_ok || S_ISLNK(sp->st_mode) || !access(name, W_OK))
			return (1);
		strmode(sp->st_mode, modep);
		pw = getpwuid(sp->st_uid);
		if (pw == NULL)
			err(EXIT_FAILURE, "getpwuid");
		gr = getgrgid(sp->st_gid);
		if (gr == NULL)
			err(EXIT_FAILURE, "getgrgid");
		(void)fprintf(stderr, "override %s%s%s/%s for %s? ",
		    modep + 1, modep[10] == ' ' ? "" : " ",
		    pw->pw_name,
		    gr->gr_name,
		    path);
	}
	(void)fflush(stderr);

	first = ch = getchar();
	while (ch != '\n' && ch != EOF)
		ch = getchar();
	return (first == 'y' || first == 'Y');
}


static void
siginfo(int sig __attribute__((unused)))
{

	info = 1;
}
