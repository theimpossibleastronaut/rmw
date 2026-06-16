#!/bin/sh
# Same-device / different-mount (EXDEV) behavior, exercised on a small
# dedicated tmpfs ramdisk holding a bind mount.
#
# A bind mount shares the underlying filesystem with its source, so a file on
# the bind mount has the SAME st_dev as a WASTE folder elsewhere on that
# filesystem (rmw matches it by device) yet rename(2) returns EXDEV because it
# cannot cross the mount boundary.
#
# Covered here:
#   1. tmpfs is an excluded filesystem, so with no reachable WASTE rmw must
#      refuse (like desktop file managers do in /tmp) instead of creating a
#      $topdir trash there.
#   2. A configured WASTE on the file's own mount receives the file after the
#      cross-mount WASTE is skipped on EXDEV.
#   3. A pre-existing trash dir on an excluded filesystem is still honored:
#      discovery lists it and removals can reach it.
#
# Everything lives inside the ramdisk, which is unmounted on exit; the test
# never writes to the user's home or the host /tmp tree.

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
BIND_TRASH="$BIND/.Trash-$UID_NUM"

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

TEST_CONFIG="$RAM/exdev.testrc"
printf 'WASTE = %s\nexpire_age = 90\n' "$WASTE_DIR" > "$TEST_CONFIG"

echo "test data" > "$BIND/foo"

# The file and the configured WASTE share a device ...
test "$(stat -c %d "$BIND/foo")" = "$(stat -c %d "$RAM")"
# ... yet $BIND is a distinct (bind) mount, so renaming from it to a WASTE on
# $RAM crosses a mount boundary and rename(2) returns EXDEV. (stat's %m can't
# see bind mounts; mountpoint reads the real mount table and can.)
mountpoint -q "$BIND"

# --- 1: excluded fs, no reachable WASTE -> refuse, don't create a trash ---
out=$("$BIN_DIR"/rmw -c "$TEST_CONFIG" -v "$BIND/foo" 2>&1) || true
echo "$out"
cmp_substr "$out" "No WASTE folder defined"
# The file stays put and no trash dir was created on the excluded fs.
test -f "$BIND/foo"
test ! -e "$BIND_TRASH"
test ! -e "$WASTE_DIR/files/foo"

echo "PASS: excluded fs with no reachable WASTE refused cleanly"

# --- 2: a configured WASTE on the bind mount takes the EXDEV skip ---
WASTE_ON_BIND="$BIND/Waste2"
printf 'WASTE = %s\nWASTE = %s\nexpire_age = 90\n' \
  "$WASTE_DIR" "$WASTE_ON_BIND" > "$TEST_CONFIG"

"$BIN_DIR"/rmw -c "$TEST_CONFIG" -v "$BIND/foo"

test ! -e "$BIND/foo"
# Not in the skipped cross-mount WASTE ...
test ! -e "$WASTE_DIR/files/foo"
# ... but in the configured WASTE on the file's own mount.
test -f "$WASTE_ON_BIND/files/foo"
test -f "$WASTE_ON_BIND/info/foo.trashinfo"

echo "PASS: EXDEV skip reached the configured same-mount WASTE"

# --- 3: a pre-existing trash on an excluded fs is honored ---
# Plant a spec-shaped trash dir on the bind mount, return to the single
# cross-mount WASTE config, and verify discovery picks the planted trash up:
# it appears in -l output and a removal lands in it after the EXDEV skip.
#
# Discovery is suppressed under RMW_FAKE_HOME (it would see the host's real
# mounts), so opt in with RMW_CHECK_DISCOVERY. Discovery will list host topdir
# trashes too — the assertions are additive, and a host trash can never
# device-match a file on this private tmpfs, so removals can't escape it.
mkdir -p "$BIND_TRASH/files" "$BIND_TRASH/info"
printf 'WASTE = %s\nexpire_age = 90\n' "$WASTE_DIR" > "$TEST_CONFIG"

out=$(RMW_CHECK_DISCOVERY=1 "$BIN_DIR"/rmw -c "$TEST_CONFIG" -l 2>&1)
echo "$out"
cmp_substr "$out" "$BIND_TRASH"

echo "test data" > "$BIND/foo3"
RMW_CHECK_DISCOVERY=1 "$BIN_DIR"/rmw -c "$TEST_CONFIG" -v "$BIND/foo3"

test ! -e "$BIND/foo3"
test ! -e "$WASTE_DIR/files/foo3"
test -f "$BIND_TRASH/files/foo3"
test -f "$BIND_TRASH/info/foo3.trashinfo"

echo "PASS: pre-existing trash on an excluded fs was discovered and used"
exit 0
