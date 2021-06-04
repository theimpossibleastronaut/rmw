packextrasdir=@docdir@
SUBDIRS = external/canfigger src man test po
EXTRA_DIST =	\
	README.md	\
	ChangeLog	\
	AUTHORS.md	\
	COPYING	\
	ReleaseNotes	\
	CODE_OF_CONDUCT.md

packextras_DATA=$(EXTRA_DIST)

ACLOCAL_AMFLAGS = --install -I build-aux/m4

clean-local:
	if test -f @builddir@/po/stamp-po; then	\
		rm @builddir@/po/stamp-po;	\
	fi
