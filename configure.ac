AC_PREREQ(2.69)
AC_INIT([rmw], [0.7.09-dev], [https://github.com/theimpossibleastronaut/rmw/issues],,[https://remove-to-waste.info/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall subdir-objects foreign no-dependencies])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([src/config.h:config.in])

AX_IS_RELEASE([git-directory])

# Also provide an argument to enable a release build manually
AC_ARG_ENABLE([release],
    AS_HELP_STRING([--enable-release], [Enable release build (Default: disabled unless .git directory is present]))
AS_IF([test "x$enable_release" = "xyes"], [ax_is_release=yes])

# A "yes" value would be overridden by the value of $ax_is_release, returned by AX_IS_RELEASE()
# or set using --enable-release.
AX_CHECK_ENABLE_DEBUG([yes])

AC_PROG_CC
AX_CFLAGS_WARN_ALL

AC_CONFIG_MACRO_DIRS([build-aux/m4])
AC_PROG_INSTALL
AC_HEADER_STDBOOL
AC_SYS_LARGEFILE

# This prevents problems such as those shown in
# https://github.com/theimpossibleastronaut/rmw/commit/90200c2df06b16f16b5d21d25c51966c0ee65b23#commitcomment-30291627
# and https://github.com/theimpossibleastronaut/rmw/issues/21
AM_MAINTAINER_MODE

# For creating the static test library
AC_PROG_RANLIB
AM_PROG_AR

EXTRA_CFLAGS="-Wextra -fno-common"

AX_CHECK_COMPILE_FLAG(-Werror=format-security, [ax_gcc_flag_option=yes], [ax_gcc_flag_option=no], [-Werror])
if test "x$ax_gcc_flag_option" = "xyes"; then
        EXTRA_CFLAGS+=" -Werror=format-security"
fi

AX_CHECK_COMPILE_FLAG(-Wformat-overflow=2, [ax_gcc_flag_option=yes], [ax_gcc_flag_option=no], [-Werror])
if test "x$ax_gcc_flag_option" = "xyes"; then
        EXTRA_CFLAGS+=" -Wformat-overflow=2"
fi

AX_CHECK_COMPILE_FLAG(-fanalyzer, [ax_gcc_flag_option=yes], [ax_gcc_flag_option=no], [-Werror])
if test "x$ax_gcc_flag_option" = "xyes"; then
        EXTRA_CFLAGS+=" -Werror=format-security"
fi

if test "x$ax_is_release" = "xyes"; then
        AX_CHECK_COMPILE_FLAG(-fstack-protector-strong, [ax_gcc_flag_option=yes], [ax_gcc_flag_option=no], [-Werror])
        if test "x$ax_gcc_flag_option" = "xyes"; then
                EXTRA_CFLAGS+=" -fstack-protector-strong"
        fi
        AX_CHECK_COMPILE_FLAG(-D_FORTIFY_SOURCE=2, [ax_gcc_flag_option=yes], [ax_gcc_flag_option=no], [-Werror])
        if test "x$ax_gcc_flag_option" = "xyes"; then
                EXTRA_CFLAGS+=" -D_FORTIFY_SOURCE=2 -O2"
        fi
fi

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

if test "x$ac_pwd" = x; then
        AC_MSG_ERROR(while determining top build directory)
fi
AC_SUBST(HOME_TEST_DIR, [$ac_pwd/test/rmw-tests-home])

AC_CONFIG_FILES([       Makefile
                        src/Makefile
                        man/Makefile
                        test/Makefile
                        po/Makefile.in
                        test/VARS])

AC_CONFIG_FILES([test/test_basic.sh], [chmod +x test/test_basic.sh])
AC_CONFIG_FILES([test/test_purging.sh], [chmod +x test/test_purging.sh])
AC_CONFIG_FILES([test/test_restore.sh], [chmod +x test/test_restore.sh])
AC_CONFIG_FILES([test/test_buffer_overrun.sh], [chmod +x test/test_buffer_overrun.sh])
AC_CONFIG_FILES([test/create-some-files.sh], [chmod +x test/create-some-files.sh])

AC_OUTPUT
echo ""
echo "    ==[ Options ]=="
echo "Installation prefix:                  $prefix"
echo "Debug enabled:                        $enable_debug"
echo "Build for release:                    $ax_is_release"
echo ""
