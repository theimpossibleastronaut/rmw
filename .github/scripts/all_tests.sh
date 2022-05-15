#!/bin/sh

set -ev

sudo apt-get install -y gettext

USE_VALGRIND=""

if [ "$CC" = "gcc-11" ]; then
  USE_VALGRIND=1
  sudo apt-get install -y valgrind
fi

if [ "${GITHUB_JOB}" = "ubuntu-focal" ]; then
  sudo apt-get install -y python3-pip python3-setuptools
fi

if [ "$CC" = "gcc-12" ]; then
  sudo apt-get install -y meson $CC
else
   sudo -H python3 -m pip install meson ninja
fi

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
ninja -v
meson test -v

# curses disabled
meson configure -Dwithout-curses=true
if [ -n "$USE_VALGRIND" ]; then
  ninja -v
  meson test --setup=valgrind
fi
