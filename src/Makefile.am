bin_PROGRAMS = $(top_builddir)/rmw
AM_CFLAGS = @CURSES_CFLAGS@ @EXTRA_CFLAGS@

include @top_srcdir@/Makefile.include

DEFS = -DLOCALEDIR=\"$(localedir)\"

__top_builddir__rmw_LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@
