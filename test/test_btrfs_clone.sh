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

if ! grep -qw 'btrfs' /proc/filesystems 2>/dev/null; then
  echo "btrfs not supported by kernel; skipping."
  exit $SKIP
fi

LOOP=""
# shellcheck disable=SC2329
cleanup() {
  cd /
  if mountpoint -q "$BTRFS_MOUNTPOINT" 2>/dev/null; then
    sudo umount "$BTRFS_MOUNTPOINT"
  fi
  if [ -n "$LOOP" ]; then
    sudo losetup -d "$LOOP" 2>/dev/null || true
  fi
}
trap cleanup EXIT

# Clean up any stale state from a previous run
if mountpoint -q "$BTRFS_MOUNTPOINT" 2>/dev/null; then
  echo "btrfs mountpoint already mounted from a previous run; cleaning up."
  sudo umount "$BTRFS_MOUNTPOINT"
fi
STALE_LOOP=$(sudo losetup -j "$BTRFS_IMAGE" -O NAME --noheadings 2>/dev/null)
if [ -n "$STALE_LOOP" ]; then
  echo "btrfs image already on loop device $STALE_LOOP from a previous run; cleaning up."
  sudo losetup -d "$STALE_LOOP"
fi

if [ ! -d "$BTRFS_MOUNTPOINT" ]; then
  sudo mkdir "$BTRFS_MOUNTPOINT"
fi

LOOP=$(sudo losetup -f --show "$BTRFS_IMAGE")
sudo mount -t btrfs "$LOOP" "$BTRFS_MOUNTPOINT"
sudo chown "$(id -u)" -R "$BTRFS_MOUNTPOINT"

cd "$BTRFS_MOUNTPOINT"
BTRFS_RMW_CMD="$BIN_DIR/rmw -c ${MESON_SOURCE_ROOT}/test/conf/btrfs_img.testrc"

BTRFS_SUBVOLUME="$BTRFS_MOUNTPOINT/@two"
BTRFS_WASTE_DIR="$BTRFS_SUBVOLUME/Waste"

rm -rf "$BTRFS_WASTE_DIR"

# --- Test: move a directory with contents across btrfs subvolumes ---
echo "== Test: move a directory with contents across btrfs subvolumes"
BTRFS_TEST_DIR="$BTRFS_MOUNTPOINT/test_dir"
rm -rf "$BTRFS_TEST_DIR"
mkdir "$BTRFS_TEST_DIR"
touch "$BTRFS_TEST_DIR/bar"
# shellcheck disable=SC2086
strace_check -t "ioctl" "FICLONE" $BTRFS_RMW_CMD "$BTRFS_TEST_DIR"
test ! -d "$BTRFS_TEST_DIR"
test -d "$BTRFS_WASTE_DIR/files/test_dir"
test -f "$BTRFS_WASTE_DIR/files/test_dir/bar"
test -f "$BTRFS_WASTE_DIR/info/test_dir.trashinfo"

echo "== Test: restore the moved directory"
# shellcheck disable=SC2086
strace_check -t "ioctl" "FICLONE" $BTRFS_RMW_CMD -u
test -d "$BTRFS_TEST_DIR"
test -f "$BTRFS_TEST_DIR/bar"
test ! -f "$BTRFS_WASTE_DIR/info/test_dir.trashinfo"

# --- Test: move a deeply nested directory across btrfs subvolumes ---
echo "== Test: move a deeply nested directory across btrfs subvolumes"
BTRFS_NESTED_DIR="$BTRFS_MOUNTPOINT/nested_dir"
rm -rf "$BTRFS_NESTED_DIR"
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

# --- Test: move a directory containing a symlink across btrfs subvolumes ---
echo "== Test: move a directory containing a symlink across btrfs subvolumes"
BTRFS_SYM_DIR="$BTRFS_MOUNTPOINT/sym_dir"
rm -rf "$BTRFS_SYM_DIR"
mkdir "$BTRFS_SYM_DIR"
touch "$BTRFS_SYM_DIR/real_file"
ln -s real_file "$BTRFS_SYM_DIR/link_file"
$BTRFS_RMW_CMD "$BTRFS_SYM_DIR"
test ! -d "$BTRFS_SYM_DIR"
test -d "$BTRFS_WASTE_DIR/files/sym_dir"
test -f "$BTRFS_WASTE_DIR/files/sym_dir/real_file"
test -L "$BTRFS_WASTE_DIR/files/sym_dir/link_file"
test -f "$BTRFS_WASTE_DIR/info/sym_dir.trashinfo"

echo "== Test: restore directory containing a symlink"
$BTRFS_RMW_CMD -u
test -d "$BTRFS_SYM_DIR"
test -f "$BTRFS_SYM_DIR/real_file"
test -L "$BTRFS_SYM_DIR/link_file"
test ! -f "$BTRFS_WASTE_DIR/info/sym_dir.trashinfo"

FS_MOUNTPOINT="$BTRFS_MOUNTPOINT"
FS_RMW_CMD="$BTRFS_RMW_CMD"
FS_WASTE_DIR="$BTRFS_WASTE_DIR"
. "${MESON_SOURCE_ROOT}/test/test_ficlone_safeguards.sh"

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

# --- Test: move a symlink across btrfs subvolumes ---
echo "== Test: move a symlink across btrfs subvolumes"
[ -L sym_link ] && rm sym_link
ln -s foo sym_link
$BTRFS_RMW_CMD sym_link
test ! -L sym_link
test -L "$BTRFS_WASTE_DIR/files/sym_link"
test -f "$BTRFS_WASTE_DIR/info/sym_link.trashinfo"

echo "== Test: restore the moved symlink"
$BTRFS_RMW_CMD -u
test -L sym_link
test ! -f "$BTRFS_WASTE_DIR/info/sym_link.trashinfo"

# --- Test: dangling symlink on btrfs is not followed by is_ficlone_fs ---
# If is_ficlone_fs followed the symlink it would crash (ENOENT) or return false,
# and the symlink would not reach the btrfs waste dir.
echo "== Test: dangling symlink on btrfs goes to btrfs waste (symlink not followed)"
ln -s nonexistent_ficlone_target dangling_link
$BTRFS_RMW_CMD dangling_link
test ! -L dangling_link
test -L "$BTRFS_WASTE_DIR/files/dangling_link"
test -f "$BTRFS_WASTE_DIR/info/dangling_link.trashinfo"

echo "== Test: restore the dangling symlink"
$BTRFS_RMW_CMD -u
test -L dangling_link
test ! -f "$BTRFS_WASTE_DIR/info/dangling_link.trashinfo"
rm dangling_link

# --- Test: xattr preservation across btrfs subvolumes ---
if command -v setfattr >/dev/null 2>&1 && command -v getfattr >/dev/null 2>&1; then
  echo "== Test: xattr preserved after ficlone move"
  touch xattr_file
  setfattr -n user.rmwtest -v hello xattr_file
  $BTRFS_RMW_CMD xattr_file
  test ! -f xattr_file
  test -f "$BTRFS_WASTE_DIR/files/xattr_file"
  getfattr -n user.rmwtest --only-values "$BTRFS_WASTE_DIR/files/xattr_file" | grep -qx hello

  echo "== Test: xattr preserved after ficlone restore"
  $BTRFS_RMW_CMD -u
  test -f xattr_file
  getfattr -n user.rmwtest --only-values xattr_file | grep -qx hello
else
  echo "setfattr/getfattr not available; skipping xattr test."
fi

# --- Test: purge an expired file from btrfs waste ---
echo "== Test: purge an expired file from btrfs waste"
RMW_FAKE_YEAR=true $BTRFS_RMW_CMD foo
test -f "$BTRFS_WASTE_DIR/files/foo"
test -f "$BTRFS_WASTE_DIR/info/foo.trashinfo"
# shellcheck disable=SC2086
strace_check "unlink" $BTRFS_RMW_CMD -g
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

exit 0
