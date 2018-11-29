AC_INIT([rmw], [0.4.06-dev], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])

# Don't add -g and -O2 by default
: ${CFLAGS=""}

AC_PROG_CC
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE

# This prevents problems such as those shown in
# https://github.com/theimpossibleastronaut/rmw/commit/90200c2df06b16f16b5d21d25c51966c0ee65b23#commitcomment-30291627
# and https://github.com/theimpossibleastronaut/rmw/issues/21
AM_MAINTAINER_MODE

AC_CHECK_LIB([ncursesw], [initscr], found_ncursesw=yes, found_ncursesw=no)
if test "x$found_curses" = xno; then
		AC_MSG_ERROR("required ncursesw library not found. Please install libncursesw5-dev")
fi

AC_CHECK_LIB([menuw], [post_menu], found_menuw=yes, found_menuw=no)
if test "x$found_menuw" = xno; then
		AC_MSG_ERROR("required menuw library not found. Please install libncursesw5-dev")
fi

AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION([0.19.8])

AC_CONFIG_FILES([	Makefile
			src/Makefile
			man/Makefile
			test/Makefile
			po/Makefile.in])
AC_OUTPUT
