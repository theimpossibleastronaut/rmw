#!/bin/bash

set -ev

cd /rmw/src

if [ ! -f src/main.c ]; then
  echo "Your local rmw src directory must be mounted in \
    the container at /rmw/src (e.g. '-v $PWD:/rmw/src'"
  exit 1
fi

meson ../builddir
cd ../builddir
ninja
meson test --setup=valgrind
meson test --setup=valgrind_fake_media_root --suite=rmw
