/*
 * compat.h
 * Local prototype definitions for functions put together in this library.
 * We don't have the full OpenBSD system headers, so use this header file
 * to be a placeholder.
 */


/* Windows compatibility */
/* This is experimental
 * Windows compatibility headers
 */
#if defined __MINGW32__ || defined _MSC_VER
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <windows.h>
#endif


/*
 * Reference from Apple's archived OS X (now macOS documentation)
 * we need to import this else we are going to get a "declaration expected at
 * line 29"
 *
 * including types.h allows us to fix errors in the mget declaration
 *
 */

#if defined __APPLE__
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

/* General imports for non-Apple platforms */
#if defined(__linux__) || defined(__FreeBSD__)
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

//! Fix a warning that complains about extern declarations
#ifdef HAVE___PROGNAME
extern const char *__progname;
#else
extern const char *__progname;
#endif

// Use whereami library
#ifdef USE_LIBWHEREAMI
#include "whereami.h"
#define IS_USING_WHEREAMI_LIBRARY
void setprogname(const char *progname);
const char *getprogname(void);
#endif

/* sys/types.h */
#if defined __MINGW32__ || defined _MSC_VER
typedef int gid_t;
typedef int uid_t;
typedef unsigned char      u_char;

typedef unsigned int u_int;
#pragma push_macro("u_long")
#undef u_long
typedef unsigned long u_long;
#pragma pop_macro("u_long")
#endif

/* Primarily musl... */
#ifndef u_long
typedef unsigned long u_long;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

/* sys/stat.h */
#if defined __MINGW32__|| defined _MSC_VER
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

#define S_IFSOCK 0140000
#define S_IFLNK    0120000 /* Symbolic link */
#endif

#if defined __MINGW32__ || defined _MSC_VER
struct passwd {
	char *pw_name;
	char *pw_passwd;
	char *pw_gecos;
	char *pw_dir;
	char *pw_shell;
	uid_t pw_uid;
	gid_t pw_gid;
};

int		 getpwuid_r (uid_t, struct passwd *, char *,
			size_t, struct passwd **);
int 		 getpwnam_r (const char *, struct passwd *,
			char *, size_t , struct passwd **);
#endif

#ifdef __MINGW32__
long sysconf(int name);
#endif

#if defined __MINGW32__ || defined _MSC_VER
#define NAME_MAX _MAX_FNAME
#endif

#ifdef __MINGW32__
struct group {
	char *gr_name;
	char *gr_passwd;
	gid_t gr_gid;
	char **gr_mem;
};
#endif

#if defined __MINGW32__ || defined _MSC_VER
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

#ifdef _MSC_VER
#define mode_t int
#endif


/* setmode.c */
#ifndef __MINGW32__
mode_t getmode (const void *, mode_t);
void *setmode (const char *);
#endif

#ifdef __MINGW32__
unsigned long stroul(const char*, char **, int);
#endif

/* strtonum.c */
long long strtonum (const char *, long long, long long, const char **);

/* strmode.c */
void strmode (int, char *);

/* pwcache.c */
/* Darwin (OSX/macOS) requires the nouser and nogroup
to be added */

#ifndef __APPLE__
const char *user_from_uid (uid_t, int);
const char *group_from_gid (gid_t, int);
#endif

int uid_from_user (const char *, uid_t *);
int gid_from_group (const char *, gid_t *);

/* fmt_scaled.c */
int scan_scaled (char *, long long *);
int fmt_scaled (long long, char *);

/* getbsize.c */
char *getbsize (int *, long *);

/* devname.c */
#ifndef __MINGW32__
char *devname (dev_t, mode_t);
#endif

#ifdef __MINGW32__
char *devname_mingw(dev_t, mode_t);
#define devname devname_mingw;
#endif

/* merge.c */
int mergesort (void *, size_t, size_t, int (*) (const void *, const void *));

/* heapsort.c */
int heapsort (void *, size_t, size_t, int (*) (const void *, const void *));

/* recallocarray.c */
void *recallocarray (void *, size_t, size_t, size_t);

/* reallocarray.c */
#if defined __APPLE__
void *reallocarray (void *ptr, size_t nmemb, size_t size);
#endif

/* strlcat.c */
#if defined (__linux__)
size_t strlcat (char *, const char *, size_t);
#endif

/* strlcpy.c */
#if defined __linux__|| defined __MINGW32__
size_t strlcpy (char *, const char *, size_t);
#endif

/*
 * MAXBSIZE does not exist on Linux  however, since Darwin is an OS
 * that derives from FreesBD this does exist on Darwin, so we dont
 * need to get oursevels, an extra warning for redefining a macro,
 * however this is the explainaition for Linux because filesystem block size
 * limits are per filesystem and not consistently enforced across
 * the different filesystems.  If you look at e2fsprogs and its
 * header files, you'll see the max block size is defined as 65536
 * via (1 << EXT2_MAX_BLOCK_LOG_SIZE) where EXT2_MAX_BLOCK_LOG_SIZE
 * is 16.  On OpenBSD, MAXBSIZE is simply (64 * 1024), which is
 * 65536.  So we'll just define that here so as to avoid having
 * bsdutils depend on e2fsprogs to compile.
 */
#if defined (__linux__) || defined (__MINGW32__)
#define MAXBSIZE (64 * 1024)
#endif

/*
 * fmt_scaled(3) specific flags.
 * This comes from lib/libutil/util.h in the OpenBSD source.
 */
#define FMT_SCALED_STRSIZE 7 /* minus sign, 4 digits, suffix, null byte */

/* Buffer sizes */
#if defined (__linux__) || defined (__APPLE__) || defined (__FreeBSD__)
#define _PW_BUF_LEN sysconf (_SC_GETPW_R_SIZE_MAX)
#define _GR_BUF_LEN sysconf (_SC_GETGR_R_SIZE_MAX)
#endif

#if defined __MINGW32__
#define _SC_GETPW_R_SIZE_MAX				0x0048
#define _PW_BUF_LEN _SC_GETPW_R_SIZE_MAX
#define _GR_BUF_LEN _SC_GETPW_R_SIZE_MAX
#endif

/* Linux spelling differences
 * And Windows too :p
*/
#if defined (__linux__) || defined (__MINGW32__)
#define S_ISTXT S_ISVTX
#endif

#if defined __linux__
#define D_TAPE 1
#endif

/* This are extracted from libbsd, define missing symbols
 * and operations necessary on timespec structs */
#ifndef timespeccmp
#define	timespeccmp(tsp, usp, cmp)					\
	(((tsp)->tv_sec == (usp)->tv_sec) ?				\
	    ((tsp)->tv_nsec cmp (usp)->tv_nsec) :			\
	    ((tsp)->tv_sec cmp (usp)->tv_sec))
#endif

#ifndef timespecclear
#define	timespecclear(tsp)		(tsp)->tv_sec = (tsp)->tv_nsec = 0
#endif

void explicit_bzero(void *, size_t );

/* we use SIGUSR1 in place of SIGINFO */
#define SIGINFO SIGUSR1

int signame_to_signum(const char *sig);
const char *signum_to_signame(int signum);
int get_signame_by_idx(size_t idx, const char **signame, int *signum);

