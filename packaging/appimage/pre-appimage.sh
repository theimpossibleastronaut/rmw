#!/bin/bash

# Set bash options:
# -e: Exit immediately if a command exits with a non-zero status.
# -v: Print each command to stderr before executing it.
set -ev

# Set default workspace if not provided
WORKSPACE=${WORKSPACE:-$(pwd)}
# Check if the workspace path is absolute
if [[ "$WORKSPACE" != /* ]]; then
  echo "The workspace path must be absolute"
  exit 1
fi
test -d "$WORKSPACE"

# Set default source root if not provided
SOURCE_ROOT=${SOURCE_ROOT:-$WORKSPACE}
# Check if the source root path is absolute
if [[ "$SOURCE_ROOT" != /* ]]; then
  echo "The source root path must be absolute"
  exit 1
fi
# Verify that you're in the source root
echo $SOURCE_ROOT
test -f "$SOURCE_ROOT/src/main.c"

# Define and create application directory if it doesn't exist
# This is the directory where your project will be installed to
# and an AppImage created
APPDIR=${APPDIR:-"/tmp/$USER-AppDir"}
if [ -d "$APPDIR" ]; then
  rm -rf "$APPDIR"
else
  mkdir -v -p "$APPDIR"
fi

# Install necessary dependencies
sudo apt install -y \
  gettext \
  liblua5.1-0-dev \
  libphysfs-dev \
  libsdl2-dev \
  libsdl2-mixer-dev \
  libsdl2-ttf-dev

# Set up build directory
BUILD_DIR="$SOURCE_ROOT/appimage_build"
cd "$SOURCE_ROOT"
BUILD_DIR="$SOURCE_ROOT/_build_prep_appdir"
# Clean build directory if specified and it exists
if [ "$CLEAN_BUILD" = "true" ] && [ -d "$BUILD_DIR" ]; then
  rm -rf "$BUILD_DIR"
fi

# Setup project for building
if [ ! -d "$BUILD_DIR" ]; then
  meson setup "$BUILD_DIR" \
    -Dbuildtype=release \
    -Dstrip=true \
    -Db_sanitize=none \
    -Dprefix=/usr
fi

# Build project
cd "$BUILD_DIR"
ninja
meson install --destdir=$APPDIR --skip-subprojects

# Set up output directory
OUT_DIR="$WORKSPACE/out"
if [ ! -d "$OUT_DIR" ]; then
  mkdir "$OUT_DIR"
fi
cd "$OUT_DIR"

# Set LinuxDeploy output version
LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
# Generate AppImage using linuxdeploy
linuxdeploy \
  --appdir="$APPDIR" \
  -d $SOURCE_ROOT/packaging/rmw.desktop \
  --icon-file=$SOURCE_ROOT/packaging/rmw_icon_32x32.png \
  --icon-filename=rmw \
  --executable=$APPDIR/usr/bin/rmw \
  -o appimage
