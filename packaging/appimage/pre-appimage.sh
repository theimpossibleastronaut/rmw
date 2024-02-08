#!/bin/bash

set -ev

WORKSPACE=${WORKSPACE:-$ACTION_WORKSPACE}

if [ -z "$WORKSPACE" ]; then
  echo "You need to set the WORKSPACE directory."
  exit 1
fi

# Check if the path starts with a slash (/)
if [[ "$WORKSPACE" != /* ]]; then
  echo "The workspace path must absolute"
  exit 1
fi

test -d "$WORKSPACE"

sudo apt install -y \
  gettext \
  liblua5.1-0-dev \
  libphysfs-dev \
  libsdl2-dev \
  libsdl2-mixer-dev \
  libsdl2-ttf-dev

export APPDIR=${APPDIR:-"$WORKSPACE/AppDir"}

if [ -d "$APPDIR" ]; then
  rm -rf "$APPDIR"
fi

BUILD_DIR="$WORKSPACE/appimage_build"

if [ -d "$APPDIR" ] && [ "$CLEAN_BUILD" = "true" ]; then
  rm -rf "$APPDIR"
fi

meson setup $BUILD_DIR \
  -Dbuildtype=release \
  -Dstrip=true \
  -Db_sanitize=none \
  -Dprefix=/usr

cd $BUILD_DIR
ninja
meson install --destdir=$APPDIR --skip-subprojects
