bin_PROGRAMS=rmw
AM_CFLAGS = -std=c99 -Wall
AM_CFLAGS +=  @CURSES_CFLAGS@

git_rev := $(shell \
if test -f @top_srcdir@/.git/refs/heads/master; then	\
	expr substr `cat @top_srcdir@/.git/refs/heads/master` 1 7;	\
else	\
	echo "unknown";	\
fi)
$(shell echo '#define GIT_REV  ".$(git_rev)"' > git_rev.h)

rmw_CFLAGS = $(AM_CFLAGS)

rmw_SOURCES=	\
	rmw.c	\
	restore_rmw.c	\
	config_rmw.c	\
	trivial_rmw.c	\
	strings_rmw.c	\
	purging_rmw.c	\
	messages_rmw.c	\
	trashinfo_rmw.c	\
	utils_rmw.c	\
	bst.c	\
	rmw.h	\
	restore_rmw.h 	\
	config_rmw.h	\
	trivial_rmw.h	\
	strings_rmw.h	\
	purging_rmw.h	\
	messages_rmw.h	\
	trashinfo_rmw.h	\
	utils_rmw.h	\
	bst.h	\
	gettext.h



DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALEDIR=\"$(localedir)\"

lib_CFLAGS = $(AM_CFLAGS) -DBUILD_LIBRARY

if BUILD_LIBS
# shared library
lib_LTLIBRARIES = librmw.la
librmw_la_SOURCES = $(rmw_SOURCES)
librmw_la_CFLAGS = $(lib_CFLAGS)

# static library, don't install (actually it installs anyway,
# see issue https://github.com/theimpossibleastronaut/rmw/issues/227
noinst_LIBRARIES = librmw.a
librmw_a_SOURCES = $(rmw_SOURCES)
librmw_a_CFLAGS = $(lib_CFLAGS)
endif

# static library used by 'make check'
check_LIBRARIES = librmw_test.a
librmw_test_a_SOURCES = $(rmw_SOURCES)
librmw_test_a_CFLAGS = $(lib_CFLAGS) -DTEST_LIB

#
# On OpenBSD, without @LIBINTL@ present here, the result is a lot of
# undefined reference errors when building. That doesn't happen when
# building on Debian
#
rmw_LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@

all-local:
	cp $(top_builddir)/src/rmw $(top_builddir)/rmw;

clean-local:
	if test -f $(top_builddir)/rmw; then	\
		rm $(top_builddir)/rmw;	\
	fi
