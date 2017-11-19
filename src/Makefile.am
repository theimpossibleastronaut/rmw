bin_PROGRAMS=rmw
AM_CFLAGS= -std=gnu99 -Wall -I./functions  -DSYSCONFDIR=\"${sysconfdir}\" -DLOCALEDIR=\"${localedir}\"
rmw_SOURCES=			\
primary_funcs.c			\
main.c 				\
./functions/restore_rmw.c 	\
./functions/config_rmw.c	\
./functions/trivial_rmw.c 	\
./functions/strings_rmw.c 	\
./functions/purging_rmw.c	\
./functions/messages_rmw.c

PACKAGE = @PACKAGE@
VERSION = @VERSION@
top_srcdir = @top_srcdir@
subdir = src

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

LIBS = @LIBINTL@ @LIBS@

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist: Makefile $(DISTFILES)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) 2>/dev/null || cp -p $$file $(distdir) || exit 1; \
	done

rmw_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
rmw_LDADD = @LIBINTL@
