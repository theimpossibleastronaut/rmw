#!/bin/sh

set -ev

if [ -z "$WORKSPACE" ]; then
  echo "This script needs to be called from make-appimage.sh"
  exit 1
fi

if [ -z "$VERSION" ]; then
  echo "VERSION is set from make-appimage.sh or the GitHub workflow."
  exit 1
fi

OUTPUT_DIR="$WORKSPACE/packaging/appimage"
test -d "$OUTPUT_DIR"
APPDIR="$WORKSPACE/packaging/appimage/AppDir"
BUILD_DIR="$WORKSPACE/packaging/appimage/builddir"

cd "$WORKSPACE"
meson setup "$BUILD_DIR" -Dbuildtype=release -Dstrip=true -Db_sanitize=none -Dprefix=/usr

rm -rf "$APPDIR"
DESTDIR="$APPDIR" ninja -C "$BUILD_DIR" install
cd "$OUTPUT_DIR"
if [ ! -e "linuxdeploy-${ARCH}.AppImage" ]; then
  curl -LO https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20220822-1/linuxdeploy-${ARCH}.AppImage
  chmod +x linuxdeploy-${ARCH}.AppImage
  ./linuxdeploy-${ARCH}.AppImage --appimage-extract
fi

"$OUTPUT_DIR/squashfs-root/AppRun"  \
  -d "$WORKSPACE/packaging/rmw.desktop" \
  --icon-file="$WORKSPACE/packaging/rmw_icon_32x32.png" \
  --icon-filename=rmw \
  --executable "$APPDIR/usr/bin/rmw" \
  --appdir "$APPDIR" \
  --output appimage
sha256sum rmw-${VERSION}-${ARCH}.AppImage > rmw-${VERSION}-${ARCH}.AppImage.sha256sum
