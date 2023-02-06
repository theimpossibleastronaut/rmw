#!/usr/bin/env -S bash --posix

set -ev

SRC_DIR=/rmw/src
cd $SRC_DIR

if [ ! -f src/main.c ]; then
  echo "Your local rmw src directory must be mounted in \
    the container at /rmw/src (e.g. '-v $PWD:/rmw/src'"
  exit 1
fi

BUILD_DIR="$SRC_DIR/../builddir"

meson setup $BUILD_DIR
cd $BUILD_DIR
ninja
meson test $RMW_V
meson test $RMW_V --setup=fake_media_root --suite=rmw
