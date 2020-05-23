AC_INIT([rmw], [0.7.05-dev], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall subdir-objects foreign no-dependencies])
AM_SILENT_RULES([yes])
AC_PREREQ(2.69)
AC_CONFIG_HEADERS([src/config.h:config.in])

AX_IS_RELEASE([git-directory])
# Define the variables listed in the second argument if debug is
# enabled, defaulting to no variables. Defines the variables listed in
# the third argument if debug is disabled, defaulting to NDEBUG. All
# lists of variables should be space-separated.
#
# A "yes" value will be overridden by the value returned by AX_IS_RELEASE()
AX_CHECK_ENABLE_DEBUG([yes])

AC_PROG_CC
AC_PROG_CC_STDC
AX_CFLAGS_WARN_ALL

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
AM_PROG_AR

EXTRA_CFLAGS="-fno-common"
AC_SUBST([EXTRA_CFLAGS])

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
                # These 2 paths are set so the headers can be found on DragonFlyBSD
                CFLAGS="$CFLAGS -I/usr/local/include -I/usr/local/include/ncurses"
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

AC_CONFIG_FILES([       Makefile
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
