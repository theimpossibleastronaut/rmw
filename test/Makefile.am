AM_CFLAGS = -I$(top_srcdir)/src @CURSES_CFLAGS@ @EXTRA_CFLAGS@ -DTEST_LIB

check_PROGRAMS = \
	test_strings	\
	test_config_test \
	test_multi_strlen	\
	test_buffer_overrun	\
	test_url_encoding	\
	test_trashinfo	\
	test_rmdir_recursive

test_strings_SOURCES = test_strings.c
test_config_test_SOURCES = test_config.c
test_multi_strlen_SOURCES = test_multi_strlen.c
test_buffer_overrun_SOURCES = test_buffer_overrun.c
test_url_encoding_SOURCES = test_url_encoding.c
test_trashinfo_SOURCES = test_trashinfo.c
test_rmdir_recursive = test_rmdir_recursive.c

test_scripts =	\
	test_basic.sh	\
	test_purging.sh	\
	test_restore.sh	\
	test_buffer_overrun.sh

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
	if test -d $(top_builddir)/test/rmdir_test; then	\
		rm -rf $(top_builddir)/test/rmdir_test;	\
	fi
