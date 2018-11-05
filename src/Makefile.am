bin_PROGRAMS=rmw
AM_CFLAGS= -std=c99 -Wall

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
	bst.c

noinst_HEADERS = \
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

rmw_LDADD = -lcurses -lmenu

all-local:
	cp $(top_builddir)/src/rmw $(top_builddir)/rmw;

clean-local:
	if test -f $(top_builddir)/rmw; then	\
		rm $(top_builddir)/rmw;	\
	fi
