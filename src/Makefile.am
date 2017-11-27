bin_PROGRAMS=rmw
AM_CFLAGS= -std=gnu99 -Wall -pedantic -I./functions  -DSYSCONFDIR=\"${sysconfdir}\" -DLOCALEDIR=\"${localedir}\"
rmw_SOURCES=			\
primary_funcs.c			\
main.c 				\
./functions/restore_rmw.c 	\
./functions/config_rmw.c	\
./functions/trivial_rmw.c 	\
./functions/strings_rmw.c 	\
./functions/purging_rmw.c	\
./functions/messages_rmw.c

noinst_HEADERS = 						\
rmw.h												\
primary_funcs.h							\
main.h 											\
gettext.h										\
./functions/restore_rmw.h 	\
./functions/config_rmw.h		\
./functions/trivial_rmw.h 	\
./functions/strings_rmw.h 	\
./functions/purging_rmw.h		\
./functions/messages_rmw.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@
top_srcdir = @top_srcdir@
subdir = src

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@

LIBS = @LIBINTL@ @LIBS@
rmw_LDADD = @LIBINTL@

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist: Makefile $(DISTFILES)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) 2>/dev/null || cp -p $$file $(distdir) || exit 1; \
	done
