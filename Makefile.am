packextrasdir=@docdir@
SUBDIRS = src etc man po
EXTRA_DIST=         \
BUGS                \
rmwrc_config.example \
README.md            \
ChangeLog           \
AUTHORS             \
THANKS	            \
COPYING			\
INSTALL             \
NEWS                \
TODO								\
ABOUT-NLS

packextras_DATA=$(EXTRA_DIST)

PACKAGE = @PACKAGE@
VERSION = @VERSION@

ACLOCAL = true
AUTOCONF = true
AUTOHEADER = true
AUTOMAKE = true

ACLOCAL_AMFLAGS = --install -I m4

distdir = $(PACKAGE)-$(VERSION)
dist: Makefile
	rm -fr $(distdir)
	mkdir $(distdir)
	chmod 777 $(distdir)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) 2>/dev/null || cp -p $$file $(distdir); \
	done
	for subdir in $(SUBDIRS); do \
	  mkdir $(distdir)/$$subdir || exit 1; \
	  chmod 777 $(distdir)/$$subdir; \
	  (cd $$subdir && $(MAKE) $@) || exit 1; \
	done
	tar chozf $(distdir).tar.gz $(distdir)
	rm -fr $(distdir)
