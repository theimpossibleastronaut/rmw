include $(srcdir)/include.mk

noinst_LIBRARIES = librmw.a
librmw_a_CFLAGS = $(AM_CFLAGS)
librmw_a_SOURCES =	\
	globals.c	\
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
	globals.h	\
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

bin_PROGRAMS = $(top_builddir)/rmw
DEFS = -DLOCALEDIR=\"$(localedir)\"
__top_builddir__rmw_SOURCES=	\
	main.c	\
	main.h	\
	gettext.h

__top_builddir__rmw_LDADD = librmw.a $(top_builddir)/external/canfigger/libcanfigger.a @LIBS@

@top_builddir@/external/canfigger/libcanfigger.a:
	make -C $(top_srcdir)/external/canfigger
