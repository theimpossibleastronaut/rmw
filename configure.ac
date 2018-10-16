AC_INIT([rmw], [m4_esyscmd_s([cat ./.version])], [andy400-dev@yahoo.com],,['https://github.com/theimpossibleastronaut/rmw/wiki'])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])
AC_PROG_CC([cc])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE

# This prevents problems such as those shown in
# https://github.com/theimpossibleastronaut/rmw/commit/90200c2df06b16f16b5d21d25c51966c0ee65b23#commitcomment-30291627
# and https://github.com/theimpossibleastronaut/rmw/issues/21
AM_MAINTAINER_MODE

AC_CHECK_LIB([curses], [initscr], found_curses=yes, found_curses=no)
if test "x$found_curses" = xno; then
		AC_MSG_ERROR("required ncurses library not found. Please install libncurses5-dev")
fi

AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION([0.19.8])

AC_CONFIG_FILES([	Makefile
			src/Makefile
			etc/Makefile
			man/Makefile
			po/Makefile.in])
AC_OUTPUT
