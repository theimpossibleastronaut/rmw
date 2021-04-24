packextrasdir=@docdir@
SUBDIRS = src man test po
EXTRA_DIST = ABOUT-NLS README.md ChangeLog AUTHORS.md THANKS COPYING ReleaseNotes rmwrc.example

packextras_DATA=$(EXTRA_DIST)

ACLOCAL_AMFLAGS = --install -I build-aux/m4

clean-local:
	if test -f @builddir@/po/stamp-po; then	\
		rm @builddir@/po/stamp-po;	\
	fi

make check-selectrestore:
	if test $$(command -v valgrind); then	\
		echo q | valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 @top_builddir@/rmw -s;	\
	else	\
		echo "Valgrind not installed";	\
	fi
