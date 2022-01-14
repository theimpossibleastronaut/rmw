#!/bin/sh

set -ev

sudo apt-get install -y gettext

USE_VALGRIND=""

if [ "$CC" = "gcc-11" ]; then
  USE_VALGRIND=1
  sudo apt-get install -y valgrind
fi

if [ "$MATRIX_OS" = "ubuntu-18.04" ]; then
  sudo apt-get install -y python3-pip python3-setuptools
fi

sudo -H python3 -m pip install meson ninja

meson builddir
cd builddir
ninja -v
ninja dist

if [ -n "$USE_VALGRIND" ]; then
  meson test --setup=valgrind
fi

# fake media root
meson test --setup=fake_media_root --suite rmw

# nls disabled
meson configure -Dnls=false
# 'meson test' appears to rebuild but doesn't necessarily show errors or
# if the binary was not rebuilt because of errors
ninja clean
ninja build
meson test -v

ninja clean
# curses disabled
meson configure -Dwithout-curses=true
if [ -n "$USE_VALGRIND" ]; then
  ninja clean
  ninja build
  meson test --setup=valgrind
fi
