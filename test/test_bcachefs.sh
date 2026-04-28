#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

BCACHEFS_MOUNTPOINT="/tmp/rmw-bcachefs-loop"
BCACHEFS_IMAGE="${MESON_SOURCE_ROOT}/test/rmw-bcachefs-test.img"

if ! command -v bcachefs >/dev/null 2>&1; then
  echo "bcachefs userspace tools not found; skipping."
  exit $SKIP
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping bcachefs test."
  exit $SKIP
fi

# Load the module if available, then confirm kernel support
sudo modprobe bcachefs 2>/dev/null || true
if ! grep -qw 'bcachefs' /proc/filesystems 2>/dev/null; then
  echo "bcachefs not supported by kernel; skipping."
  exit $SKIP
fi

if [ ! -f "$BCACHEFS_IMAGE" ]; then
  echo "bcachefs test image not found at $BCACHEFS_IMAGE; skipping."
  exit $SKIP
fi

LOOP=""
# shellcheck disable=SC2329
cleanup() {
  cd /
  if mountpoint -q "$BCACHEFS_MOUNTPOINT" 2>/dev/null; then
    sudo umount "$BCACHEFS_MOUNTPOINT"
  fi
  if [ -n "$LOOP" ]; then
    sudo losetup -d "$LOOP" 2>/dev/null || true
  fi
}
trap cleanup EXIT

# Clean up any stale state from a previous run
if mountpoint -q "$BCACHEFS_MOUNTPOINT" 2>/dev/null; then
  echo "bcachefs mountpoint already mounted from a previous run; cleaning up."
  sudo umount "$BCACHEFS_MOUNTPOINT"
fi
STALE_LOOP=$(sudo losetup -j "$BCACHEFS_IMAGE" -O NAME --noheadings 2>/dev/null)
if [ -n "$STALE_LOOP" ]; then
  echo "bcachefs image already on loop device $STALE_LOOP from a previous run; cleaning up."
  sudo losetup -d "$STALE_LOOP"
fi

if [ ! -d "$BCACHEFS_MOUNTPOINT" ]; then
  sudo mkdir "$BCACHEFS_MOUNTPOINT"
fi

LOOP=$(sudo losetup -f --show "$BCACHEFS_IMAGE")
sudo mount -t bcachefs "$LOOP" "$BCACHEFS_MOUNTPOINT"
if [ ! -d "$BCACHEFS_MOUNTPOINT/@two" ]; then
  sudo bcachefs subvolume create "$BCACHEFS_MOUNTPOINT/@two"
fi
sudo chown "$(id -u)" -R "$BCACHEFS_MOUNTPOINT"

cd "$BCACHEFS_MOUNTPOINT"
BCACHEFS_RMW_CMD="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/bcachefs_img.testrc"

BCACHEFS_SUBVOLUME="$BCACHEFS_MOUNTPOINT/@two"
BCACHEFS_WASTE_DIR="$BCACHEFS_SUBVOLUME/Waste"

rm -rf "$BCACHEFS_WASTE_DIR"

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

# --- Test: move a symlink across bcachefs subvolumes ---
echo "== Test: move a symlink across bcachefs subvolumes"
ln -s foo sym_link
$BCACHEFS_RMW_CMD sym_link
test ! -L sym_link
test -L "$BCACHEFS_WASTE_DIR/files/sym_link"
test -f "$BCACHEFS_WASTE_DIR/info/sym_link.trashinfo"

echo "== Test: restore the moved symlink"
$BCACHEFS_RMW_CMD -u
test -L sym_link
test ! -f "$BCACHEFS_WASTE_DIR/info/sym_link.trashinfo"

# --- Test: move a directory across bcachefs subvolumes ---
echo "== Test: move a directory across bcachefs subvolumes"
BCACHEFS_TEST_DIR="$BCACHEFS_MOUNTPOINT/test_dir"
rm -rf "$BCACHEFS_TEST_DIR"
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

# --- Test: move a deeply nested directory across bcachefs subvolumes ---
echo "== Test: move a deeply nested directory across bcachefs subvolumes"
BCACHEFS_NESTED_DIR="$BCACHEFS_MOUNTPOINT/nested_dir"
rm -rf "$BCACHEFS_NESTED_DIR"
mkdir -p "$BCACHEFS_NESTED_DIR/a/b/c"
touch "$BCACHEFS_NESTED_DIR/a/b/c/deep_file"
$BCACHEFS_RMW_CMD "$BCACHEFS_NESTED_DIR"
test ! -d "$BCACHEFS_NESTED_DIR"
test -d "$BCACHEFS_WASTE_DIR/files/nested_dir"
test -f "$BCACHEFS_WASTE_DIR/files/nested_dir/a/b/c/deep_file"
test -f "$BCACHEFS_WASTE_DIR/info/nested_dir.trashinfo"

echo "== Test: restore the nested directory"
$BCACHEFS_RMW_CMD -u
test -d "$BCACHEFS_NESTED_DIR/a/b/c"
test -f "$BCACHEFS_NESTED_DIR/a/b/c/deep_file"
test ! -f "$BCACHEFS_WASTE_DIR/info/nested_dir.trashinfo"

# --- Test: move a directory containing a symlink across bcachefs subvolumes ---
echo "== Test: move a directory containing a symlink across bcachefs subvolumes"
BCACHEFS_SYM_DIR="$BCACHEFS_MOUNTPOINT/sym_dir"
rm -rf "$BCACHEFS_SYM_DIR"
mkdir "$BCACHEFS_SYM_DIR"
touch "$BCACHEFS_SYM_DIR/real_file"
ln -s real_file "$BCACHEFS_SYM_DIR/link_file"
$BCACHEFS_RMW_CMD "$BCACHEFS_SYM_DIR"
test ! -d "$BCACHEFS_SYM_DIR"
test -d "$BCACHEFS_WASTE_DIR/files/sym_dir"
test -f "$BCACHEFS_WASTE_DIR/files/sym_dir/real_file"
test -L "$BCACHEFS_WASTE_DIR/files/sym_dir/link_file"
test -f "$BCACHEFS_WASTE_DIR/info/sym_dir.trashinfo"

echo "== Test: restore directory containing a symlink"
$BCACHEFS_RMW_CMD -u
test -d "$BCACHEFS_SYM_DIR"
test -f "$BCACHEFS_SYM_DIR/real_file"
test -L "$BCACHEFS_SYM_DIR/link_file"
test ! -f "$BCACHEFS_WASTE_DIR/info/sym_dir.trashinfo"

# --- Test: purge an expired file from bcachefs waste ---
echo "== Test: purge an expired file from bcachefs waste"
RMW_FAKE_YEAR=true $BCACHEFS_RMW_CMD foo
test -f "$BCACHEFS_WASTE_DIR/files/foo"
test -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"
$BCACHEFS_RMW_CMD -g
test ! -f "$BCACHEFS_WASTE_DIR/files/foo"
test ! -f "$BCACHEFS_WASTE_DIR/info/foo.trashinfo"

exit 0
