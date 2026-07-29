#!/bin/sh

# Build a portable AppImage of rmw with the "AnyLinux" method from
# pkgforge-dev (https://github.com/pkgforge-dev/Anylinux-AppImages):
# sharun + uruntime + DwarFS. It bundles glibc and the dynamic linker, so
# the result runs on any Linux distro, including musl ones such as Alpine
# and hosts with a much older glibc than the build machine.
#
# Meant to run on an Arch Linux base (see ../../.github/workflows/release.yml).
# Build dependencies are installed by the workflow with pacman. This script
# builds rmw, installs it into the container's /usr, then bundles the
# installed binary with quick-sharun.

set -eux

ARCH="$(uname -m)"

# VERSION is exported by CI; fall back for local runs.
VERSION="${VERSION:-snapshot}"

# quick-sharun is fetched from pkgforge-dev rather than vendored, so the
# bundling logic always tracks upstream.
SHARUN="https://raw.githubusercontent.com/pkgforge-dev/Anylinux-AppImages/refs/heads/main/useful-tools/quick-sharun.sh"

# Source root is two levels up from this script (packaging/appimage/).
unset CDPATH
SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
SOURCE_ROOT=$(cd -- "$SCRIPT_DIR/../.." && pwd)
test -f "$SOURCE_ROOT/meson.build"

WORKSPACE="${WORKSPACE:-$SOURCE_ROOT}"
BUILD_DIR="$SOURCE_ROOT/_build_appdir"
APPDIR="${APPDIR:-/tmp/rmw-AppDir}"
OUTPATH="$WORKSPACE/out"
WORKDIR="$SOURCE_ROOT/packaging/appimage"

rm -rf "$APPDIR" "$BUILD_DIR"
mkdir -p "$APPDIR" "$OUTPATH"

# --- build rmw and install into the container's /usr -----------------------
# Install to the real /usr (no DESTDIR): quick-sharun deploys rmw the way it
# deploys any /usr-prefixed app. Its path patcher rewrites the binary's
# compiled-in LOCALEDIR (/usr/share/locale) so the bundled translations
# resolve at runtime with no wrapper script setting RMW_LOCALEDIR.
meson setup "$BUILD_DIR" \
  -Dbuildtype=release \
  -Dstrip=true \
  -Db_sanitize=none \
  -Dprefix=/usr

ninja -C "$BUILD_DIR"
meson install -C "$BUILD_DIR" --skip-subprojects

# --- bundle with sharun and pack the AppImage ------------------------------
export APPDIR
# Staged under the name the .desktop entry's Icon= key expects; quick-sharun
# copies $ICON into the AppDir keeping its basename.
cp "$SOURCE_ROOT/packaging/rmw_icon_128x128.png" /tmp/rmw.png
export ICON="/tmp/rmw.png"
export DESKTOP="$SOURCE_ROOT/packaging/rmw.desktop"
export OUTPATH
export OUTNAME="rmw-$VERSION-$ARCH.AppImage"
export VERSION
export UPINFO="gh-releases-zsync|andy5995|rmw|latest|*$ARCH.AppImage.zsync"

cd "$WORKDIR"

# Arch is rolling, so refresh the package DB before quick-sharun resolves any
# of its own pacman dependencies against it.
pacman -Syu --noconfirm

wget --retry-connrefused --tries=30 "$SHARUN" -O "$WORKDIR/quick-sharun"
chmod +x "$WORKDIR/quick-sharun"

# quick-sharun copies /usr/share/terminfo when it sees libncursesw among the
# dependencies, so the restore menu gets a terminal database of its own
# instead of depending on the host having one.
./quick-sharun /usr/bin/rmw

# rmw is a terminal program: the single icon it needs comes from $ICON, so the
# unconditional copy of /usr/share/icons/hicolor is dead weight. On a developer
# machine that directory holds every installed application's icons.
rm -rf "$APPDIR/share/icons"

./quick-sharun --make-appimage

# Catches a missing bundled library ("error while loading shared libraries",
# "symbol lookup error") before the artifact is uploaded.
./quick-sharun --simple-test "$OUTPATH/$OUTNAME" --version

ls -lh "$OUTPATH"
