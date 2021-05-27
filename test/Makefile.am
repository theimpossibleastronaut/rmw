include $(srcdir)/../src/include.mk
AM_CFLAGS += -I$(top_srcdir)/src

check_PROGRAMS = \
	test_strings	\
	test_config_test \
	test_utils	\
	test_trashinfo	\
	test_rmdir_recursive	\
	test_select_restore

test_strings_SOURCES = $(top_srcdir)/src/strings_rmw.c
test_config_test_SOURCES = $(top_srcdir)/src/config_rmw.c
test_utils_SOURCES = $(top_srcdir)/src/utils_rmw.c
test_trashinfo_SOURCES = $(top_srcdir)/src/trashinfo_rmw.c
test_rmdir_recursive_SOURCES = $(top_srcdir)/src/purging_rmw.c
test_select_restore_SOURCES = $(top_srcdir)/src/restore_rmw.c

noinst_HEADERS = test.h

test_scripts =	\
	test_basic.sh	\
	test_purging.sh	\
	test_restore.sh

@VALGRIND_CHECK_RULES@
#VALGRIND_SUPPRESSIONS_FILES = @srcdir@/default.supp

AUTOMAKE_OPTIONS = parallel-tests
TEST_EXTENSIONS = .sh
LOG_COMPILER = $(VALGRIND)

TESTS=$(check_PROGRAMS) $(test_scripts)
TESTS_ENVIRONMENT = VALGRIND='$(VALGRIND)'

@top_builddir@/src/librmw.a: subsystem

DEFS = -DTEST_LIB -DHOME_TEST_DIR=\"$(HOME_TEST_DIR)\"
LDADD = $(top_builddir)/src/librmw.a @LIBS@

EXTRA_DIST=	\
	README	\
	COMMON \
	rmw.testrc	\
	rmw.alt.testrc	\
	rmw.purge_disabled.testrc

check-local:
	if test -e $(builddir)/rmdir_test; then	\
		$(RM) -rf $(builddir)/rmdir_test;	\
	fi

clean-local:
	dir=$(top_builddir)/test/rmw-tests-home; \
	if test -d $$dir;	then	\
		rm -rf $$dir;	\
	fi
	if test -d $(top_builddir)/test/rmdir_test; then	\
		rm -rf $(top_builddir)/test/rmdir_test;	\
	fi

subsystem:
	$(MAKE) librmw.a -C @top_builddir@/src;	\
	$(MAKE) @top_builddir@/rmw -C @top_builddir@/src;
