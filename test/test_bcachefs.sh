#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

BCACHEFS_MOUNTPOINT="/tmp/rmw-bcachefs-loop"
BCACHEFS_IMAGE="/tmp/rmw-bcachefs-test.img"

if ! command -v bcachefs >/dev/null 2>&1; then
  echo "bcachefs userspace tools not found; skipping."
  exit 0
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping bcachefs test."
  exit 0
fi

# Load the module if available, then confirm kernel support
sudo modprobe bcachefs 2>/dev/null || true
if ! grep -qw 'bcachefs' /proc/filesystems 2>/dev/null; then
  echo "bcachefs not supported by kernel; skipping."
  exit 0
fi

# Create a fresh bcachefs image
rm -f "$BCACHEFS_IMAGE"
truncate -s 32M "$BCACHEFS_IMAGE"
mkfs.bcachefs "$BCACHEFS_IMAGE"

if [ ! -d "$BCACHEFS_MOUNTPOINT" ]; then
  sudo mkdir "$BCACHEFS_MOUNTPOINT"
fi

LOOP=$(sudo losetup -f --show "$BCACHEFS_IMAGE")
sudo mount -t bcachefs "$LOOP" "$BCACHEFS_MOUNTPOINT"
sudo bcachefs subvolume create "$BCACHEFS_MOUNTPOINT/@two"
sudo chown "$(id -u)" -R "$BCACHEFS_MOUNTPOINT"

cd "$BCACHEFS_MOUNTPOINT"
BCACHEFS_RMW_CMD="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/bcachefs_img.testrc"

BCACHEFS_SUBVOLUME="$BCACHEFS_MOUNTPOINT/@two"
BCACHEFS_WASTE_DIR="$BCACHEFS_SUBVOLUME/Waste"

# --- Test: move a file across bcachefs subvolumes (issue #526) ---
echo "== Test: move a file across bcachefs subvolumes"
touch foo
$BCACHEFS_RMW_CMD foo
test ! -f foo
test -f "$BCACHEFS_WASTE_DIR/files/foo"
test -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"

echo "== Test: restore the moved file"
$BCACHEFS_RMW_CMD -u
test -f foo
test ! -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"

# --- Test: move a directory across bcachefs subvolumes ---
echo "== Test: move a directory across bcachefs subvolumes"
BCACHEFS_TEST_DIR="$BCACHEFS_MOUNTPOINT/test_dir"
mkdir "$BCACHEFS_TEST_DIR"
touch "$BCACHEFS_TEST_DIR/bar"
$BCACHEFS_RMW_CMD "$BCACHEFS_TEST_DIR"
test ! -d "$BCACHEFS_TEST_DIR"
test -d "$BCACHEFS_WASTE_DIR/files/test_dir"
test -f "$BCACHEFS_WASTE_DIR/files/test_dir/bar"
test -f "$BCACHEFS_WASTE_DIR/info/test_dir.trashinfo"

echo "== Test: restore the moved directory"
$BCACHEFS_RMW_CMD -u
test -d "$BCACHEFS_TEST_DIR"
test -f "$BCACHEFS_TEST_DIR/bar"
test ! -f "$BCACHEFS_WASTE_DIR/info/test_dir.trashinfo"

# --- Test: purge an expired file from bcachefs waste ---
echo "== Test: purge an expired file from bcachefs waste"
RMW_FAKE_YEAR=true $BCACHEFS_RMW_CMD foo
test -f "$BCACHEFS_WASTE_DIR/files/foo"
test -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"
$BCACHEFS_RMW_CMD -g
test ! -f "$BCACHEFS_WASTE_DIR/files/foo"
test ! -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"

cd

sudo umount "$BCACHEFS_MOUNTPOINT"
sudo losetup -d "$LOOP"
rm -f "$BCACHEFS_IMAGE"

exit 0
