packextrasdir=@docdir@
SUBDIRS = src etc man po
EXTRA_DIST=         \
BUGS                \
README.md            \
ChangeLog           \
AUTHORS.md           \
THANKS	            \
COPYING			\
INSTALL             \
NEWS                \
TODO

packextras_DATA=$(EXTRA_DIST)

PACKAGE = @PACKAGE@
VERSION = @VERSION@

ACLOCAL = true
AUTOCONF = true
AUTOHEADER = true
AUTOMAKE = true

ACLOCAL_AMFLAGS = --install -I m4

distdir = $(PACKAGE)-$(VERSION)
