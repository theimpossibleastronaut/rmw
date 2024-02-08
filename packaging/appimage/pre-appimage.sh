#!/bin/bash

set -ev

WORKSPACE=${WORKSPACE:-$ACTION_WORKSPACE}
if [ -z "$WORKSPACE" ]; then
  echo "You need to set the WORKSPACE directory."
  exit 1
fi
if [[ "$WORKSPACE" != /* ]]; then
  echo "The workspace path must be absolute"
  exit 1
fi
test -d "$WORKSPACE"

SOURCE_ROOT=${SOURCE_ROOT:-$ACTION_SOURCE_ROOT}
if [ -z "$SOURCE_ROOT" ]; then
  echo "You need to set the SOURCE_ROOT directory."
  exit 1
fi
if [[ "$SOURCE_ROOT" != /* ]]; then
  echo "The source root path must be absolute"
  exit 1
fi
test -d "$SOURCE_ROOT"

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

BUILD_DIR="$SOURCE_ROOT/appimage_build"

if [ -d "$APPDIR" ] && [ "$CLEAN_BUILD" = "true" ]; then
  rm -rf "$APPDIR"
fi

cd "$SOURCE_ROOT"
meson setup $BUILD_DIR \
  -Dbuildtype=release \
  -Dstrip=true \
  -Db_sanitize=none \
  -Dprefix=/usr

cd "$BUILD_DIR"
ninja
meson install --destdir=$APPDIR --skip-subprojects
