#!/bin/bash

set -ev

cd /rmw/src

if [ ! -f src/main.c ]; then
  echo "Your local rmw src directory must be mounted in \
    the container at /rmw/src (e.g. '-v $PWD:/rmw/src'"
  exit 1
fi

ARGS2="--setup=fake_media_root"

meson setup ../builddir
cd ../builddir
ninja
meson test $ARGS
meson test $ARGS2 --suite=rmw
