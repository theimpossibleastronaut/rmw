bin_PROGRAMS=rmw
AM_CFLAGS = -std=c99 -Wall
AM_CFLAGS +=  @CURSES_CFLAGS@

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
lib_OBJECTS =	\
	rmw.o	\
	restore_rmw.o	\
	config_rmw.o	\
	trivial_rmw.o	\
	strings_rmw.o	\
	purging_rmw.o	\
	messages_rmw.o	\
	trashinfo_rmw.o	\
	utils_rmw.o	\
	bst.o

# shared library
lib_LTLIBRARIES = librmw.la
librmw_la_SOURCES = $(lib_OBJECTS)
librmw_la_CFLAGS = $(lib_CFLAGS)

# static library
lib_LIBRARIES = librmw.a
librmw_a_SOURCES = $(lib_OBJECTS)
librmw_a_CFLAGS = $(lib_CFLAGS)

# static library used by 'make check'
check_LIBRARIES = librmw_test.a
# some conditions in the source are changed when TEST_LIB is defined, therefore
# we won't use pre-compiled objects for this lib.
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
