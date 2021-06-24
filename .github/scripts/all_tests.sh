#!/bin/sh

set -ev

meson builddir
cd builddir
ninja -v
ninja dist

# valgrind
meson test --setup=valgrind

# fake media root
meson test --setup=fake_media_root

# nls disabled
meson configure -Dnls=false
ninja -v
