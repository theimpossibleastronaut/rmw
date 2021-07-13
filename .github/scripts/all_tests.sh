#!/bin/sh

set -ev

sudo apt-get install -y gettext

if [ "$MATRIX_OS" = "ubuntu-latest" && "$MATRIX_ARCH" = "x86" ]; then
  VALGRIND=1
  sudo apt-get install -y valgrind
fi

if [ "$MATRIX_OS" = "ubuntu-18.04" ]; then
  sudo apt-get install -y python3-pip
fi

sudo python3 -m pip install meson ninja

meson builddir
cd builddir
ninja -v
ninja dist

if [ -v VALGRIND ]; then
  meson test --setup=valgrind
fi

# fake media root
meson test --setup=fake_media_root

# nls disabled
meson configure -Dnls=false
ninja -v
