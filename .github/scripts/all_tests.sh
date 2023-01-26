#!/bin/sh

set -ev

echo ${GITHUB_REF_NAME}
export VERSION="${GITHUB_REF_NAME##v}"
BUILDDIR="$PWD/builddir"

sudo apt-get install -y gettext

if [ "${GITHUB_JOB}" = "ubuntu-focal" ]; then
  sudo apt-get install -y python3-pip python3-setuptools
fi

if [ "$CC" = "gcc-12" ]; then
  sudo apt-get install -y meson $CC
else
   sudo -H python3 -m pip install meson ninja
fi

meson setup $BUILDDIR
cd $BUILDDIR

if [ "${GITHUB_REF_TYPE}" != "tag" ]; then
  ninja -v
  ninja dist

  # fake media root
  meson test --setup=fake_media_root --suite rmw

  # nls disabled
  meson configure -Dnls=false
  ninja -v
  meson test -v

  # curses disabled
  meson configure -Dwithout-curses=true
else
  meson configure -Dbuildtype=release -Dstrip=true -Dprefix=/usr
  DESTDIR=AppDir ninja install
  cd -
  wget -nv https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage
  chmod +x linuxdeploy-${ARCH}.AppImage
  ./linuxdeploy-${ARCH}.AppImage  \
    -d packaging/rmw.desktop \
    --icon-file=packaging/rmw_icon_32x32.png \
    --icon-filename=rmw \
    --executable ${BUILDDIR}/AppDir/usr/bin/rmw \
    --appdir ${BUILDDIR}/AppDir \
    --output appimage
  sha256sum rmw-${VERSION}-${ARCH}.AppImage > rmw-${VERSION}-${ARCH}.AppImage.sha256sum
fi
