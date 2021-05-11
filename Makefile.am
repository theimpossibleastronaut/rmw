packextrasdir=@docdir@
SUBDIRS = src man test po
EXTRA_DIST =	\
	ABOUT-NLS	\
	README.md	\
	ChangeLog	\
	AUTHORS.md	\
	COPYING	\
	ReleaseNotes	\
	rmwrc.example	\
	CODE_OF_CONDUCT.md

packextras_DATA=$(EXTRA_DIST)

ACLOCAL_AMFLAGS = --install -I build-aux/m4

clean-local:
	if test -f @builddir@/po/stamp-po; then	\
		rm @builddir@/po/stamp-po;	\
	fi

make check-selectrestore:
	if test $$(command -v valgrind); then	\
		echo q | RMWTEST_HOME=@HOME_TEST_DIR@ valgrind --leak-check=full -v --error-exitcode=1 @top_builddir@/rmw -s -c @top_srcdir@/test/rmw.testrc;	\
	else	\
		echo "Valgrind not installed";	\
	fi
