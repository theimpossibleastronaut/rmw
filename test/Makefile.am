AM_CFLAGS = -I$(top_srcdir)/src @CURSES_CFLAGS@

check_PROGRAMS = \
	strings	\
	config_test \
	multi_strlen	\
	buffer_overrun	\
	url_encoding	\
	trashinfo

strings_SOURCES = strings.c
config_test_SOURCES = config_test.c
multi_strlen_SOURCES = multi_strlen.c
buffer_overrun_SOURCES = buffer_overrun.c
url_encoding_SOURCES = url_encoding.c
trashinfo_SOURCES = trashinfo.c

test_scripts =	\
	basic.sh	\
	purging.sh	\
	restore.sh	\
	buffer_overrun.sh

TESTS=$(check_PROGRAMS) $(test_scripts)

LDADD = @LIBINTL@ $(top_builddir)/src/librmw_test.a @MENU_LIBS@ @CURSES_LIBS@

EXTRA_DIST=	\
	README	\
	COMMON \
	rmw.testrc	\
	rmw.alt.testrc	\
	rmw.purge_disabled.testrc

clean-local:
	dir=$(top_builddir)/test/rmw-tests-home; \
	if test -d $$dir;	then	\
		rm -rf $$dir;	\
	fi
