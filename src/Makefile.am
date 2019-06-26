bin_PROGRAMS=rmw
AM_CFLAGS = -std=c99 -Wall
AM_CFLAGS +=  @CURSES_CFLAGS@

git_rev := $(shell \
if test -f @top_srcdir@/.git/refs/heads/master; then	\
	expr substr `cat @top_srcdir@/.git/refs/heads/master` 1 7;	\
else	\
	echo "unknown";	\
fi)

$(shell touch @top_srcdir@/src/trivial_rmw.c)

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



DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALEDIR=\"$(localedir)\" -DGIT_REV=\".$(git_rev)\"

# static library used by 'make check'
check_LIBRARIES = librmw_test.a
librmw_test_a_SOURCES = $(rmw_SOURCES)
librmw_test_a_CFLAGS = -DTEST_LIB -DBUILD_LIBRARY

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
