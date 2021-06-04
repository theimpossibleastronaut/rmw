AM_CFLAGS = @CURSES_CFLAGS@ @EXTRA_CFLAGS@
AM_CFLAGS += -I$(top_srcdir)/external/canfigger

# The build fails on OpenBSD if NLS is requested and LIBINTL is not
# included here.
#LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@
