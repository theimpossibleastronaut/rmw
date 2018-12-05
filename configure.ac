AC_INIT([rmw], [0.7.01], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])

# Don't add -g and -O2 by default
: ${CFLAGS=""}

# IS-RELEASE can be used to change the 'enable-debug' default to ’no’
# when making a release. Set IS-RELEASE to ’yes’ or ’no’ as
# appropriate. By default, it uses the value of $ax_is_release, so if
# you are using the AX_IS_RELEASE macro, there is no need to pass this
# parameter.
AX_CHECK_ENABLE_DEBUG([no])

AC_PROG_CC
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE
AX_FUNC_SNPRINTF

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
			po/Makefile.in])
AC_OUTPUT
