AM_CFLAGS = @CURSES_CFLAGS@ @EXTRA_CFLAGS@

# The build fails on OpenBSD if NLS is requested and LIBINTL is not
# included here.
#LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@
