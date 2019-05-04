CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = \
	strings	\
	config_test

strings_SOURCES = strings.c
config_test_SOURCES = config_test.c

test_scripts =	\
	basic.sh	\
	purging.sh	\
	restore.sh

TESTS=$(check_PROGRAMS) $(test_scripts)

LDADD = $(top_builddir)/src/librmw.a

EXTRA_DIST=	\
	README	\
	COMMON \
	rmw.testrc

clean-local:
	dir=$(top_builddir)/test/tmp-home; \
	if test -d $$dir;	then	\
		rm -rf $$dir;	\
	fi
