packextrasdir=@docdir@
SUBDIRS = src man test po
EXTRA_DIST = ABOUT-NLS README.md ChangeLog AUTHORS.md THANKS COPYING ReleaseNotes rmwrc.example

packextras_DATA=$(EXTRA_DIST)

ACLOCAL_AMFLAGS = --install -I build-aux/m4

clean-local:
	if test -f @builddir@/po/stamp-po; then	\
		rm @builddir@/po/stamp-po;	\
	fi
