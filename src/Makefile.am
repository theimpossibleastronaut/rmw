bin_PROGRAMS=rmw
AM_CFLAGS = @CURSES_CFLAGS@

rmw_CFLAGS = $(AM_CFLAGS)

rmw_SOURCES=	\
	rmw.c	\
	main.c	\
	restore_rmw.c	\
	config_rmw.c	\
	parse_cli_options.c	\
	strings_rmw.c	\
	purging_rmw.c	\
	messages_rmw.c	\
	time_rmw.c	\
	trashinfo_rmw.c	\
	utils_rmw.c	\
	bst.c	\
	rmw.h	\
	main.h	\
	restore_rmw.h 	\
	config_rmw.h	\
	parse_cli_options.h	\
	strings_rmw.h	\
	purging_rmw.h	\
	messages_rmw.h	\
	time_rmw.h	\
	trashinfo_rmw.h	\
	utils_rmw.h	\
	bst.h	\
	gettext.h

DEFS = -DLOCALEDIR=\"$(localedir)\"

# static library used by 'make check'
check_LIBRARIES = librmw_test.a
librmw_test_a_SOURCES = $(rmw_SOURCES)
librmw_test_a_CFLAGS = $(AM_CFLAGS) -DTEST_LIB -DBUILD_LIBRARY

rmw_LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@
