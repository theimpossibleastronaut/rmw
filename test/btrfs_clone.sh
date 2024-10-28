#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# This script is only designed to work on Andy's workstation.
if [ ! -d "/mnt/btrfs_part" ]; then
  exit 0
fi

# subvolume
if [ ! -d "/mnt/btrfs_part/Waste" ]; then
  exit 0
fi

$RMW_TEST_CMD_STRING

cd "$RMW_FAKE_HOME"
for i in "/mnt/btrfs_part" "/mnt/btrfs_part/bar"; do
  touch "$i/foo"
  for j in 01 02 03 04; do
    echo "$j"
    $RMW_TEST_CMD_STRING -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs$j.rc" "$i/foo"
    test ! -f "$i/foo"
    $RMW_TEST_CMD_STRING -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs$j.rc" -u
    test -f "$i/foo"
  done
done

touch /mnt/btrfs_part/bar/foo
$RMW_TEST_CMD_STRING /mnt/btrfs_part/bar/foo \
  -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs04.rc"
test ! -f /mnt/btrfs_part/bar/foo
cd /mnt/btrfs_part/wasteLink/files
$RMW_TEST_CMD_STRING -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs04.rc" -z foo
test -f /mnt/btrfs_part/bar/foo

cd /mnt/btrfs_part/bar
touch ../foo
$RMW_TEST_CMD_STRING ../foo \
  -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs04.rc"
test -f /mnt/btrfs_part/wasteLink/files/foo
$RMW_TEST_CMD_STRING -c "${MESON_SOURCE_ROOT}/test/rmw.btrfs04.rc" -u
test -f ../foo

exit 0
