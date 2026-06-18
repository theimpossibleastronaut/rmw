#!/bin/sh
# Regression test for is_ficlone_fs() on a mount point.
#
# is_ficlone_fs() must probe the path itself, not its parent directory: a mount
# point's dirname lives on the *parent* filesystem, so dirname() of a btrfs
# mount used to statfs the parent fs and mark a discovered $topdir trash as
# non-reflink. This is the one path where that misdetection changes the move
# destination -- two subvolumes of one btrfs, mounted separately, with a
# pre-existing discovered trash on subvol B and a file removed from subvol A.
# With the bug, A's file skips B's trash and the fallback puts it on A's own
# mount; fixed, it reflink-moves into B's discovered trash.
#
# Self-contained: builds a fresh btrfs loop image at run time (unique fsid, so
# it never clashes with the committed image or a parallel btrfs test). Needs
# passwordless sudo + losetup + mkfs.btrfs + btrfs kernel support; SKIP
# otherwise (so it runs locally even when CI has neither).

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping."
  exit "$SKIP"
fi
if ! command -v mkfs.btrfs >/dev/null 2>&1; then
  echo "mkfs.btrfs not available; skipping."
  exit "$SKIP"
fi
if ! grep -qw btrfs /proc/filesystems 2>/dev/null; then
  echo "btrfs not supported by kernel; skipping."
  exit "$SKIP"
fi

UID_NUM=$(id -u)
IMG="/tmp/rmw-disc-ficlone.img"
TOP="/tmp/rmw-disc-ficlone-top"          # top-level subvol mount (file lives here)
SUB="/tmp/rmw-disc-ficlone-sub"          # @two subvol mount (discovered trash here)
TRASH="$SUB/.Trash-$UID_NUM"
LOOP=""

# shellcheck disable=SC2329
cleanup() {
  cd /
  for m in "$SUB" "$TOP"; do
    if mountpoint -q "$m" 2>/dev/null; then
      sudo umount "$m"
    fi
  done
  if [ -n "$LOOP" ]; then
    sudo losetup -d "$LOOP" 2>/dev/null || true
  fi
  sudo rm -rf "$TOP" "$SUB" "$IMG"
}
trap cleanup EXIT

# Clear stale state from an interrupted run.
for m in "$SUB" "$TOP"; do
  if mountpoint -q "$m" 2>/dev/null; then
    sudo umount "$m"
  fi
done
STALE_LOOP=$(sudo losetup -j "$IMG" -O NAME --noheadings 2>/dev/null)
if [ -n "$STALE_LOOP" ]; then
  sudo losetup -d "$STALE_LOOP"
fi
sudo rm -rf "$TOP" "$SUB" "$IMG"

# Fresh btrfs with a second subvolume @two.
truncate -s 256M "$IMG"
mkfs.btrfs -q "$IMG"
sudo mkdir -p "$TOP" "$SUB"
LOOP=$(sudo losetup -f --show "$IMG")
sudo mount "$LOOP" "$TOP"
sudo btrfs subvolume create "$TOP/@two" >/dev/null
sudo chown "$UID_NUM" -R "$TOP"

# Mount @two as its own mount: a distinct st_dev AND its own mount-table entry,
# so discovery passes the bare mount point to is_ficlone_fs() -- the reading
# that used to be wrong.
sudo mount -o subvol=@two "$LOOP" "$SUB"
sudo chown "$UID_NUM" -R "$SUB"

# Pre-existing $topdir trash on the @two mount, for discovery to pick up.
mkdir -p "$TRASH/files" "$TRASH/info"

# The configured waste lives on the home device, so it never matches the btrfs
# file; only the discovered @two trash can.
TEST_CONFIG="$RMW_FAKE_HOME/disc-ficlone.testrc"
mkdir -p "$RMW_FAKE_HOME"
printf 'WASTE = %s/.Waste\nexpire_age = 30\n' "$RMW_FAKE_HOME" > "$TEST_CONFIG"
RMW="$BIN_DIR/rmw -c $TEST_CONFIG"

# A file on the top-level subvol: different device than the discovered @two
# trash, same btrfs -- only a correct is_ficlone_fs() on the discovered mount
# routes it there via reflink.
echo data > "$TOP/victim"

echo "== Test: file reflink-moves into the discovered cross-subvolume trash"
# shellcheck disable=SC2086
strace_check -t "ioctl" "FICLONE" env RMW_DISCOVERY=on $RMW -v "$TOP/victim"

test ! -e "$TOP/victim"
test -f "$TRASH/files/victim"
test -f "$TRASH/info/victim.trashinfo"
# The fallback trash on the top-level mount must NOT have been used.
test ! -e "$TOP/.Trash-$UID_NUM"

echo "PASS: discovered btrfs mount-point trash recognized as reflink-capable"
exit 0
