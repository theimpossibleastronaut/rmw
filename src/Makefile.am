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
	utils_rmw.c

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
	gettext.h

DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALEDIR=\"$(localedir)\"

rmw_LDADD = -lcurses -lmenu

check-local:
	vars=$(top_builddir)/test/VARS;	\
	tests_dir=$(top_srcdir)/test;	\
	echo HOME=$(top_builddir)/test > $$vars;	\
	echo TESTS_DIR=$$tests_dir >> $$vars;	\
	echo CONFIG=$(top_srcdir)/etc/rmwrc >> $$vars;	\
	echo BIN_DIR=$(top_builddir)/src >> $$vars;	\
	$$tests_dir/basic.sh $$vars;	\
	$$tests_dir/purging.sh $$vars;
