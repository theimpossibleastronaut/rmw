bin_PROGRAMS=rmw
AM_CFLAGS = @GLOBAL_CFLAGS@
AM_CFLAGS += @CURSES_CFLAGS@

if IS_RELEASE_FALSE
git_rev := $(shell \
	if test -f $(top_srcdir)/.git/refs/heads/master; then	\
		cat $(top_srcdir)/.git/refs/heads/master | cut -b1-7;	\
	else	\
		echo 'unknown';	\
	fi)
else
git_rev = ""
endif

$(shell	\
if test -n $(git_rev); then	\
	echo '#define GIT_REV "'$(git_rev)'"' > $(builddir)/git_rev.h; \
else	\
	echo '/* #undef GIT_REV */' > $(builddir)/git_rev.h;	\
fi)
$(shell touch $(srcdir)/trivial_rmw.c)

rmw_CFLAGS = $(AM_CFLAGS)

rmw_SOURCES=	\
	rmw.c	\
	restore_rmw.c	\
	config_rmw.c	\
	trivial_rmw.c	\
	strings_rmw.c	\
	purging_rmw.c	\
	messages_rmw.c	\
	trashinfo_rmw.c	\
	utils_rmw.c	\
	bst.c	\
	rmw.h	\
	restore_rmw.h 	\
	config_rmw.h	\
	trivial_rmw.h	\
	strings_rmw.h	\
	purging_rmw.h	\
	messages_rmw.h	\
	trashinfo_rmw.h	\
	utils_rmw.h	\
	bst.h	\
	gettext.h

DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALEDIR=\"$(localedir)\"

# static library used by 'make check'
check_LIBRARIES = librmw_test.a
librmw_test_a_SOURCES = $(rmw_SOURCES)
librmw_test_a_CFLAGS = $(AM_CFLAGS) -DTEST_LIB -DBUILD_LIBRARY

#
# On OpenBSD, without @LIBINTL@ present here, the result is a lot of
# undefined reference errors when building. That doesn't happen when
# building on Debian
#
rmw_LDADD = @LIBINTL@ @MENU_LIBS@ @CURSES_LIBS@

all-local:
	cp $(top_builddir)/src/rmw $(top_builddir)/rmw;

clean-local:
	for file in $(top_builddir)/rmw $(builddir)/git_rev.h; do	\
		if test -f $$file; then	\
			rm $$file;	\
		fi	\
	done;
