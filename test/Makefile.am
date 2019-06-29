AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = \
	strings	\
	config_test \
	multi_strlen	\
	buffer_overrun

strings_SOURCES = strings.c
config_test_SOURCES = config_test.c
multi_strlen_SOURCES = multi_strlen.c
buffer_overrun_SOURCES = buffer_overrun.c

test_scripts =	\
	basic.sh	\
	purging.sh	\
	restore.sh	\
	buffer_overrun.sh

TESTS=$(check_PROGRAMS) $(test_scripts)

LDADD = $(top_builddir)/src/librmw_test.a

EXTRA_DIST=	\
	README	\
	COMMON \
	rmw.testrc	\
	rmw.alt.testrc

clean-local:
	dir=$(top_builddir)/test/tmp-home; \
	if test -d $$dir;	then	\
		rm -rf $$dir;	\
	fi
