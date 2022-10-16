#!/bin/bash

set -ev

cd /rmw/src

if [ ! -f src/main.c ]; then
  echo "Your local rmw src directory must be mounted in \
    the container at /rmw/src (e.g. '-v $PWD:/rmw/src'"
  exit 1
fi

ARGS2="--setup=fake_media_root"

if [ -n "$USE_VALGRIND" ]; then
  ARGS="--setup=valgrind"
  ARGS2="--setup=valgrind_fake_media_root"
fi

meson ../builddir
cd ../builddir
ninja
meson test $ARGS
meson test $ARGS2 --suite=rmw
