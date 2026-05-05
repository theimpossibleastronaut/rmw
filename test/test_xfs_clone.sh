#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

XFS_MOUNTPOINT="/tmp/rmw-xfs-loop"
XFS_IMAGE="${MESON_SOURCE_ROOT}/test/rmw-xfs-test.img"

if [ ! -f "$XFS_IMAGE" ]; then
  echo "Test image not found; skipping."
  exit $SKIP
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping xfs test."
  exit $SKIP
fi

sudo modprobe xfs 2>/dev/null || true
if ! grep -qw 'xfs' /proc/filesystems 2>/dev/null; then
  echo "xfs not supported by kernel; skipping."
  exit $SKIP
fi

LOOP=""
# shellcheck disable=SC2329
cleanup() {
  cd /
  if mountpoint -q "$XFS_MOUNTPOINT" 2>/dev/null; then
    sudo umount "$XFS_MOUNTPOINT"
  fi
  if [ -n "$LOOP" ]; then
    sudo losetup -d "$LOOP" 2>/dev/null || true
  fi
}
trap cleanup EXIT

# Clean up any stale state from a previous run
if mountpoint -q "$XFS_MOUNTPOINT" 2>/dev/null; then
  echo "xfs mountpoint already mounted from a previous run; cleaning up."
  sudo umount "$XFS_MOUNTPOINT"
fi
STALE_LOOP=$(sudo losetup -j "$XFS_IMAGE" -O NAME --noheadings 2>/dev/null)
if [ -n "$STALE_LOOP" ]; then
  echo "xfs image already on loop device $STALE_LOOP from a previous run; cleaning up."
  sudo losetup -d "$STALE_LOOP"
fi

if [ ! -d "$XFS_MOUNTPOINT" ]; then
  sudo mkdir "$XFS_MOUNTPOINT"
fi

LOOP=$(sudo losetup -f --show "$XFS_IMAGE")
sudo mount -t xfs "$LOOP" "$XFS_MOUNTPOINT"
sudo chown "$(id -u)" -R "$XFS_MOUNTPOINT"

cd "$XFS_MOUNTPOINT"
XFS_RMW_CMD="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/xfs_img.testrc"

XFS_WASTE_DIR="$XFS_MOUNTPOINT/Waste"
rm -rf "$XFS_WASTE_DIR"

# --- Test: move a file on xfs ---
echo "== Test: move a file on xfs"
touch foo
$XFS_RMW_CMD foo
test ! -f foo
test -f "$XFS_WASTE_DIR/files/foo"
test -f "$XFS_WASTE_DIR/info/foo.trashinfo"

echo "== Test: restore the moved file"
$XFS_RMW_CMD -u
test -f foo
test ! -f "$XFS_WASTE_DIR/info/foo.trashinfo"

# --- Test: move a symlink on xfs ---
echo "== Test: move a symlink on xfs"
[ -L sym_link ] && rm sym_link
ln -s foo sym_link
$XFS_RMW_CMD sym_link
test ! -L sym_link
test -L "$XFS_WASTE_DIR/files/sym_link"
test -f "$XFS_WASTE_DIR/info/sym_link.trashinfo"

echo "== Test: restore the moved symlink"
$XFS_RMW_CMD -u
test -L sym_link
test ! -f "$XFS_WASTE_DIR/info/sym_link.trashinfo"

# --- Test: move a directory with contents on xfs ---
echo "== Test: move a directory with contents on xfs"
XFS_TEST_DIR="$XFS_MOUNTPOINT/test_dir"
rm -rf "$XFS_TEST_DIR"
mkdir "$XFS_TEST_DIR"
touch "$XFS_TEST_DIR/bar"
$XFS_RMW_CMD "$XFS_TEST_DIR"
test ! -d "$XFS_TEST_DIR"
test -d "$XFS_WASTE_DIR/files/test_dir"
test -f "$XFS_WASTE_DIR/files/test_dir/bar"
test -f "$XFS_WASTE_DIR/info/test_dir.trashinfo"

echo "== Test: restore the moved directory"
$XFS_RMW_CMD -u
test -d "$XFS_TEST_DIR"
test -f "$XFS_TEST_DIR/bar"
test ! -f "$XFS_WASTE_DIR/info/test_dir.trashinfo"

# --- Test: move a deeply nested directory on xfs ---
echo "== Test: move a deeply nested directory on xfs"
XFS_NESTED_DIR="$XFS_MOUNTPOINT/nested_dir"
rm -rf "$XFS_NESTED_DIR"
mkdir -p "$XFS_NESTED_DIR/a/b/c"
touch "$XFS_NESTED_DIR/a/b/c/deep_file"
$XFS_RMW_CMD "$XFS_NESTED_DIR"
test ! -d "$XFS_NESTED_DIR"
test -d "$XFS_WASTE_DIR/files/nested_dir"
test -f "$XFS_WASTE_DIR/files/nested_dir/a/b/c/deep_file"
test -f "$XFS_WASTE_DIR/info/nested_dir.trashinfo"

echo "== Test: restore the nested directory"
$XFS_RMW_CMD -u
test -d "$XFS_NESTED_DIR/a/b/c"
test -f "$XFS_NESTED_DIR/a/b/c/deep_file"
test ! -f "$XFS_WASTE_DIR/info/nested_dir.trashinfo"

# --- Test: move a directory containing a symlink on xfs ---
echo "== Test: move a directory containing a symlink on xfs"
XFS_SYM_DIR="$XFS_MOUNTPOINT/sym_dir"
rm -rf "$XFS_SYM_DIR"
mkdir "$XFS_SYM_DIR"
touch "$XFS_SYM_DIR/real_file"
ln -s real_file "$XFS_SYM_DIR/link_file"
$XFS_RMW_CMD "$XFS_SYM_DIR"
test ! -d "$XFS_SYM_DIR"
test -d "$XFS_WASTE_DIR/files/sym_dir"
test -f "$XFS_WASTE_DIR/files/sym_dir/real_file"
test -L "$XFS_WASTE_DIR/files/sym_dir/link_file"
test -f "$XFS_WASTE_DIR/info/sym_dir.trashinfo"

echo "== Test: restore directory containing a symlink"
$XFS_RMW_CMD -u
test -d "$XFS_SYM_DIR"
test -f "$XFS_SYM_DIR/real_file"
test -L "$XFS_SYM_DIR/link_file"
test ! -f "$XFS_WASTE_DIR/info/sym_dir.trashinfo"

# --- Test: purge an expired file from xfs waste ---
echo "== Test: purge an expired file from xfs waste"
RMW_FAKE_YEAR=true $XFS_RMW_CMD foo
test -f "$XFS_WASTE_DIR/files/foo"
test -f "$XFS_WASTE_DIR/info/foo.trashinfo"
# shellcheck disable=SC2086
strace_check "unlink" $XFS_RMW_CMD -g
test ! -f "$XFS_WASTE_DIR/files/foo"
test ! -f "$XFS_WASTE_DIR/info/foo.trashinfo"

# --- Test: move file from non-xfs home to xfs waste ---
echo "== Test: move file from non-xfs home to xfs waste"
cd "$RMW_FAKE_HOME"
touch foo
$XFS_RMW_CMD foo -v
test ! -f foo

echo "== Test: restore file to non-xfs home from xfs waste"
$XFS_RMW_CMD -u
test -f foo

exit 0
