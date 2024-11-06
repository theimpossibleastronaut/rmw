#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

BTRFS_IMAGE_MOUNTPOINT="/tmp/rmw-loop"
IMAGE_PATH="${MESON_SOURCE_ROOT}/test/rmw-btrfs-test.img"

if [ ! -f "$IMAGE_PATH" ]; then
  echo "Test image not found, but exiting with zero anyway."
  exit 0
fi

if [ ! -d "$BTRFS_IMAGE_MOUNTPOINT" ]; then
  sudo mkdir "$BTRFS_IMAGE_MOUNTPOINT"
fi
IS_BTRFS_MOUNTED="$(mount | grep rmw-btrfs)" || true
if [ -z "$IS_BTRFS_MOUNTED" ]; then
  sudo mount -o loop "$IMAGE_PATH" \
    "$BTRFS_IMAGE_MOUNTPOINT"
  sudo chown $(id -u) -R "$BTRFS_IMAGE_MOUNTPOINT"
fi

cd "$BTRFS_IMAGE_MOUNTPOINT"
RMW_TEST_CMD_STRING="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/btrfs_img.testrc"
WASTE_USED="/tmp/rmw-loop/three/Waste"

touch foo
$RMW_TEST_CMD_STRING foo
test -f "$WASTE_USED/files/foo"
test -f "$WASTE_USED/info/foo.trashinfo"
test ! -f foo
$RMW_TEST_CMD_STRING -u
test -f foo

RMW_FAKE_YEAR=true $RMW_TEST_CMD_STRING foo
test -f "$WASTE_USED/files/foo"
$RMW_TEST_CMD_STRING -g
test ! -f "$WASTE_USED/files/foo"

cd "$RMW_FAKE_HOME"
touch foo
$RMW_TEST_CMD_STRING foo -v
test ! -f foo
$RMW_TEST_CMD_STRING -u
test -f foo

cd

if [ -n "$(mount | grep rmw-btrfs)" ]; then
  sudo umount "$BTRFS_IMAGE_MOUNTPOINT"
fi

exit 0
