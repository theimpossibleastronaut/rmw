#!/bin/sh
# Regression test for the same-device / different-mount (EXDEV) fallback.
#
# A bind mount shares the underlying filesystem with its source, so a file on
# the bind mount has the SAME st_dev as a WASTE folder elsewhere on that
# filesystem (rmw matches it by device) yet rename(2) returns EXDEV because it
# cannot cross the mount boundary. rmw must skip that WASTE and fall back to
# the spec $topdir trash on the file's own mount.
#
# The whole scenario is built inside a small dedicated tmpfs ramdisk that is
# unmounted on exit, so the test never writes to the user's home or to the
# host /tmp tree -- only to its own throwaway mount.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# Creating mounts needs root; bow out cleanly when we can't sudo unattended.
if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping EXDEV fallback test."
  exit "$SKIP"
fi

UID_NUM=$(id -u)
RAM="/tmp/rmw-exdev-ram"          # dedicated tmpfs (its own device)
BIND="$RAM/mnt"                   # second mount of $RAM/src (same device)
WASTE_DIR="$RAM/waste/Waste"      # the only configured WASTE; on $RAM, not $BIND
FALLBACK_TRASH="$BIND/.Trash-$UID_NUM"

umount_if_mounted() {
  if mountpoint -q "$1" 2>/dev/null; then
    sudo umount "$1" || true
  fi
}

# shellcheck disable=SC2329
cleanup() {
  cd /
  umount_if_mounted "$BIND"       # nested mount: unmount before its parent
  umount_if_mounted "$RAM"
  sudo rm -rf "$RAM" 2>/dev/null || true
}
trap cleanup EXIT

# Clear any stale state left by an interrupted previous run.
umount_if_mounted "$BIND"
umount_if_mounted "$RAM"
sudo rm -rf "$RAM" 2>/dev/null || true

sudo mkdir -p "$RAM"
sudo mount -t tmpfs -o size=2m tmpfs "$RAM"
sudo chown "$UID_NUM" "$RAM"

mkdir -p "$RAM/src" "$BIND" "$RAM/waste"
sudo mount --bind "$RAM/src" "$BIND"
sudo chown "$UID_NUM" "$BIND"

# Only WASTE is on the ramdisk root: a different mount than $BIND but the same
# device. With no WASTE on $BIND, the file can reach a trash only via the
# $topdir fallback.
TEST_CONFIG="$RAM/exdev.testrc"
printf 'WASTE = %s\nexpire_age = 90\n' "$WASTE_DIR" > "$TEST_CONFIG"

echo "test data" > "$BIND/foo"

# The file and the configured WASTE share a device ...
test "$(stat -c %d "$BIND/foo")" = "$(stat -c %d "$RAM")"
# ... yet $BIND is a distinct (bind) mount, so renaming from it to a WASTE on
# $RAM crosses a mount boundary and rename(2) returns EXDEV. (stat's %m can't
# see bind mounts; mountpoint reads the real mount table and can.)
mountpoint -q "$BIND"

"$BIN_DIR"/rmw -c "$TEST_CONFIG" -v "$BIND/foo"

# Gone from its origin ...
test ! -e "$BIND/foo"
# ... did NOT land in the cross-mount WASTE ...
test ! -e "$WASTE_DIR/files/foo"
# ... and DID land in the $topdir fallback trash on its own mount.
test -f "$FALLBACK_TRASH/files/foo"
test -f "$FALLBACK_TRASH/info/foo.trashinfo"

echo "PASS: EXDEV bind-mount file fell back to its own-mount topdir trash"
exit 0
