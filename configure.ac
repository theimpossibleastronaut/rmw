AC_INIT([rmw], [0.7.02-dev], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])

# Don't add -g and -O2 by default
: ${CFLAGS=""}

# RELEASE TODO: set this to "no"
AX_CHECK_ENABLE_DEBUG([yes])

AC_PROG_CC
AC_CONFIG_MACRO_DIRS([build-aux/m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE
AX_FUNC_SNPRINTF

AC_PROG_RANLIB

# This prevents problems such as those shown in
# https://github.com/theimpossibleastronaut/rmw/commit/90200c2df06b16f16b5d21d25c51966c0ee65b23#commitcomment-30291627
# and https://github.com/theimpossibleastronaut/rmw/issues/21
AM_MAINTAINER_MODE

AX_WITH_CURSES
AX_WITH_CURSES_MENU

AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION([0.19.8])

AC_CONFIG_FILES([	Makefile
			src/Makefile
			man/Makefile
			test/Makefile
			po/Makefile.in
			test/VARS])

AC_CONFIG_FILES([test/basic.sh], [chmod +x test/basic.sh])
AC_CONFIG_FILES([test/purging.sh], [chmod +x test/purging.sh])
AC_CONFIG_FILES([test/restore.sh], [chmod +x test/restore.sh])

AC_OUTPUT
