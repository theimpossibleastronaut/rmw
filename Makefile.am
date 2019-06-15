packextrasdir=@docdir@
SUBDIRS = src man test po
EXTRA_DIST = ABOUT-NLS README.md ChangeLog AUTHORS.md THANKS COPYING NEWS rmwrc

packextras_DATA=$(EXTRA_DIST)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rmw.pc

ACLOCAL_AMFLAGS = --install -I build-aux/m4
