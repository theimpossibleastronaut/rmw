#!/bin/sh
# Discovery against an *eligible* filesystem on its own device, which a bind
# mount or tmpfs can't provide. Covers the two branches the tmpfs discovery
# test can't reach:
#   * an eligible mount not in the config is offered as a candidate (-l -v)
#   * a file on that mount, with no matching configured WASTE, gets a fresh
#     $topdir trash created on its own mount and lands there
#
# The image is generated at run time (no committed blob); needs passwordless
# sudo + losetup + mkfs.ext4, otherwise SKIP.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

if ! sudo -n true 2>/dev/null; then
  echo "sudo not available without password; skipping loopback discovery test."
  exit "$SKIP"
fi
if ! command -v mkfs.ext4 >/dev/null 2>&1; then
  echo "mkfs.ext4 not available; skipping."
  exit "$SKIP"
fi

UID_NUM=$(id -u)
IMG="/tmp/rmw-disc-ext4.img"
MNT="/tmp/rmw-disc-mnt"
TRASH="$MNT/.Trash-$UID_NUM"
LOOP=""

# shellcheck disable=SC2329
cleanup() {
  cd /
  if mountpoint -q "$MNT" 2>/dev/null; then
    sudo umount "$MNT"
  fi
  if [ -n "$LOOP" ]; then
    sudo losetup -d "$LOOP" 2>/dev/null || true
  fi
  sudo rm -rf "$MNT" "$IMG"
}
trap cleanup EXIT

# Clear stale state from an interrupted run.
if mountpoint -q "$MNT" 2>/dev/null; then
  sudo umount "$MNT"
fi
STALE_LOOP=$(sudo losetup -j "$IMG" -O NAME --noheadings 2>/dev/null)
if [ -n "$STALE_LOOP" ]; then
  sudo losetup -d "$STALE_LOOP"
fi
sudo rm -rf "$MNT" "$IMG"

# A fresh ext4 on a loop device is its own (eligible) filesystem, distinct
# from the home device.
truncate -s 16M "$IMG"
mkfs.ext4 -qF "$IMG"
sudo mkdir -p "$MNT"
LOOP=$(sudo losetup -f --show "$IMG")
sudo mount "$LOOP" "$MNT"
sudo chown "$UID_NUM" "$MNT"

TEST_CONFIG="$RMW_FAKE_HOME/disc-loop.testrc"
mkdir -p "$RMW_FAKE_HOME"
printf 'WASTE = %s/.Waste\nexpire_age = 30\n' "$RMW_FAKE_HOME" > "$TEST_CONFIG"
RMW="$BIN_DIR/rmw -c $TEST_CONFIG"

# --- with no trash yet, the eligible mount is offered as a candidate ---
out=$(RMW_CHECK_DISCOVERY=1 $RMW -l -v 2>&1)
echo "$out"
cmp_substr "$out" "$TRASH"
test ! -e "$TRASH"

# --- a file on the loopback fs has no matching configured WASTE (different
# device than home), so the fallback creates a trash on its own mount ---
echo data > "$MNT/victim"
RMW_CHECK_DISCOVERY=1 $RMW -v "$MNT/victim"
test ! -e "$MNT/victim"
test -f "$TRASH/files/victim"
test -f "$TRASH/info/victim.trashinfo"

# --- now that the trash exists, plain -l discovers and lists it ---
out=$(RMW_CHECK_DISCOVERY=1 $RMW -l 2>&1)
cmp_substr "$out" "$TRASH"

echo "PASS: eligible separate-device mount offered as candidate and auto-created"
exit 0
