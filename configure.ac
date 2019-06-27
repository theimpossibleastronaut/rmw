AC_INIT([rmw], [0.7.02], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects foreign no-dependencies])
AM_SILENT_RULES([yes])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])

# Don't add -g and -O2 by default
: ${CFLAGS=""}

# RELEASE TODO:
is_release=false
AM_CONDITIONAL([IS_RELEASE], [test "x$is_release" = xtrue])
case "${is_release}" in
  true) is_release=yes ;;
  no)  is_release=no ;;
esac
AX_CHECK_ENABLE_DEBUG([${is_release}])

AC_PROG_CC
AC_CONFIG_MACRO_DIRS([build-aux/m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE
AX_FUNC_SNPRINTF

# This prevents problems such as those shown in
# https://github.com/theimpossibleastronaut/rmw/commit/90200c2df06b16f16b5d21d25c51966c0ee65b23#commitcomment-30291627
# and https://github.com/theimpossibleastronaut/rmw/issues/21
AM_MAINTAINER_MODE

# For creating the static test library
AC_PROG_RANLIB

AX_WITH_CURSES
if test "x$ax_cv_curses" != xyes; then
	AC_MSG_ERROR([requires an X/Open-compatible Curses library])
fi

AX_WITH_CURSES_MENU
if test "x$ax_cv_menu" != xyes; then
# sometimes the AX_WITH_CURSES_MENU doesn't find the menu libary (reported on
# Manjaro and OpenBSD), so we'll do some more checking
	AC_CHECK_LIB(menu, free_menu, found_MENU=yes, found_MENU=no, [$CURSES_LIBS])
	if test "x$found_MENU" = xyes; then
		MENU_LIBS="-lmenu"
		AC_DEFINE(HAVE_MENU, 1, [define to 1 if the curses menu library is present])
		AC_CHECK_HEADER(menu.h, [found_MENU_H=yes], [found_MENU_H=no])
		if test "x$found_MENU_H" = xyes; then
			AC_DEFINE(HAVE_MENU_H, 1, [define to 1 if the menu header file is present])
		else
			AC_MSG_ERROR(Curses menu header not found)
		fi
	else

		AC_MSG_ERROR([requires the Curses menu library.])
	fi
fi

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
AC_CONFIG_FILES([test/buffer_overrun.sh], [chmod +x test/buffer_overrun.sh])
AC_CONFIG_FILES([test/create-some-files.sh], [chmod +x test/create-some-files.sh])

AC_OUTPUT
echo ""
