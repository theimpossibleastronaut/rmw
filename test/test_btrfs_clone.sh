#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

BTRFS_MOUNTPOINT="/tmp/rmw-loop"
BTRFS_IMAGE="${MESON_SOURCE_ROOT}/test/rmw-btrfs-test.img"

if [ ! -f "$BTRFS_IMAGE" ]; then
  echo "Test image not found; skipping."
  exit $SKIP
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping btrfs test."
  exit $SKIP
fi

if [ ! -d "$BTRFS_MOUNTPOINT" ]; then
  sudo mkdir "$BTRFS_MOUNTPOINT"
fi

if ! mount | grep -q rmw-btrfs; then
  sudo mount -o loop "$BTRFS_IMAGE" "$BTRFS_MOUNTPOINT"
  sudo chown "$(id -u)" -R "$BTRFS_MOUNTPOINT"
fi

cd "$BTRFS_MOUNTPOINT"
BTRFS_RMW_CMD="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/btrfs_img.testrc"

BTRFS_SUBVOLUME="$BTRFS_MOUNTPOINT/@two"
BTRFS_WASTE_DIR="$BTRFS_SUBVOLUME/Waste"

if [ -d "$BTRFS_WASTE_DIR" ]; then
  rm -rf "$BTRFS_WASTE_DIR"
fi

# --- Test: move a directory with contents across btrfs subvolumes ---
echo "== Test: move a directory with contents across btrfs subvolumes"
BTRFS_TEST_DIR="$BTRFS_MOUNTPOINT/test_dir"
if [ -d "$BTRFS_TEST_DIR" ]; then
  rm -rf "$BTRFS_TEST_DIR"
fi
mkdir "$BTRFS_TEST_DIR"
touch "$BTRFS_TEST_DIR/bar"
$BTRFS_RMW_CMD "$BTRFS_TEST_DIR"
test ! -d "$BTRFS_TEST_DIR"
test -d "$BTRFS_WASTE_DIR/files/test_dir"
test -f "$BTRFS_WASTE_DIR/files/test_dir/bar"
test -f "$BTRFS_WASTE_DIR/info/test_dir.trashinfo"

echo "== Test: restore the moved directory"
$BTRFS_RMW_CMD -u
test -d "$BTRFS_TEST_DIR"
test -f "$BTRFS_TEST_DIR/bar"
test ! -f "$BTRFS_WASTE_DIR/info/test_dir.trashinfo"

# --- Test: move a deeply nested directory across btrfs subvolumes ---
echo "== Test: move a deeply nested directory across btrfs subvolumes"
BTRFS_NESTED_DIR="$BTRFS_MOUNTPOINT/nested_dir"
if [ -d "$BTRFS_NESTED_DIR" ]; then
  rm -rf "$BTRFS_NESTED_DIR"
fi
mkdir -p "$BTRFS_NESTED_DIR/a/b/c"
touch "$BTRFS_NESTED_DIR/a/b/c/deep_file"
$BTRFS_RMW_CMD "$BTRFS_NESTED_DIR"
test ! -d "$BTRFS_NESTED_DIR"
test -d "$BTRFS_WASTE_DIR/files/nested_dir"
test -f "$BTRFS_WASTE_DIR/files/nested_dir/a/b/c/deep_file"
test -f "$BTRFS_WASTE_DIR/info/nested_dir.trashinfo"

echo "== Test: restore the nested directory"
$BTRFS_RMW_CMD -u
test -d "$BTRFS_NESTED_DIR/a/b/c"
test -f "$BTRFS_NESTED_DIR/a/b/c/deep_file"
test ! -f "$BTRFS_WASTE_DIR/info/nested_dir.trashinfo"

# --- Test: move a file across btrfs subvolumes ---
echo "== Test: move a file across btrfs subvolumes"
touch foo
$BTRFS_RMW_CMD foo
test -f "$BTRFS_WASTE_DIR/files/foo"
test -f "$BTRFS_WASTE_DIR/info/foo.trashinfo"
test ! -f foo

echo "== Test: restore the moved file"
$BTRFS_RMW_CMD -u
test -f foo
test ! -f "$BTRFS_WASTE_DIR/info/foo.trashinfo"

# --- Test: purge an expired file from btrfs waste ---
echo "== Test: purge an expired file from btrfs waste"
RMW_FAKE_YEAR=true $BTRFS_RMW_CMD foo
test -f "$BTRFS_WASTE_DIR/files/foo"
test -f "$BTRFS_WASTE_DIR/info/foo.trashinfo"
$BTRFS_RMW_CMD -g
test ! -f "$BTRFS_WASTE_DIR/files/foo"
test ! -f "$BTRFS_WASTE_DIR/info/foo.trashinfo"

# --- Test: cross-subvolume move from a non-btrfs home directory ---
echo "== Test: move file from non-btrfs home to btrfs waste"
cd "$RMW_FAKE_HOME"
touch foo
$BTRFS_RMW_CMD foo -v
test ! -f foo

echo "== Test: restore file to non-btrfs home from btrfs waste"
$BTRFS_RMW_CMD -u
test -f foo

cd

if mount | grep -q rmw-btrfs; then
  sudo umount "$BTRFS_MOUNTPOINT"
fi

exit 0
