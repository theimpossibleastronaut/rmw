bin_PROGRAMS=rmw
AM_CFLAGS= -std=c99 -Wall -pedantic -I./functions
rmw_SOURCES=			\
rmw.c 				\
restore_rmw.c 	\
config_rmw.c	\
trivial_rmw.c 	\
strings_rmw.c 	\
purging_rmw.c	\
messages_rmw.c		\
trashinfo_rmw.c		\
utils_rmw.c

noinst_HEADERS = 						\
rmw.h												\
restore_rmw.h 	\
config_rmw.h		\
trivial_rmw.h 	\
strings_rmw.h 	\
purging_rmw.h		\
messages_rmw.h	\
trashinfo_rmw.h	\
utils_rmw.h			\
gettext.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@
top_srcdir = @top_srcdir@
subdir = src

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
sysconfdir = @sysconfdir@
DEFS = -DLOCALEDIR=\"$(localedir)\" -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@


LIBS = @LIBINTL@ @LIBS@
rmw_LDADD = @LIBINTL@
rmw_LDFLAGS = -lcurses -lmenu

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist: Makefile $(DISTFILES)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) 2>/dev/null || cp -p $$file $(distdir) || exit 1; \
	done
