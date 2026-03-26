#!/bin/sh
# If a waste folder is in $topdir of a partition or device, the
# Path should be relative
#
#https://specifications.freedesktop.org/trash-spec/1.0/#id-1.6.10.1 The key
#"Path" contains the original location of the file/directory, as either an
#absolute pathname (starting with the slash character "/") or a relative
#pathname (starting with any other character). A relative pathname is to be
#from the directory in which the trash directory resides (for example, from
#$XDG_DATA_HOME for the "home trash" directory); it MUST not include a ".."
#directory, and for files not "under" that directory, absolute pathnames must
#be used. The system SHOULD support absolute pathnames only in the "home trash"
#directory, not in the directories under $topdir.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# This test requires /tmp to be a top-level mount point on its own device so
# that rmw will write a relative Path in the trashinfo. Check /proc/mounts
# (Linux); on macOS/BSD where it doesn't exist the grep fails and we skip.
if ! grep -q '^[^ ]* /tmp ' /proc/mounts 2>/dev/null; then
  echo "/tmp is not a top-level mount point; skipping"
  exit 0
fi

# /tmp is on a different device — use it as the simulated media root.
TRASH_DIR="/tmp/.Trash-$(id -u)"
TEST_DIR="/tmp/rmw-media-root-test"
test_file="media_root_test"
test_file_path="$TEST_DIR/$test_file"

cleanup() {
  rm -rf "$TRASH_DIR" "$TEST_DIR"
}
trap cleanup EXIT

# Clean up any leftovers from a previous run
rm -rf "$TRASH_DIR" "$TEST_DIR"

# Create test config pointing the waste folder to /tmp
mkdir -p "$RMW_FAKE_HOME"
TEST_CONFIG="$RMW_FAKE_HOME/media-root.testrc"
printf 'WASTE = /tmp/.Trash-%s, removable\nexpire_age = 90\n' "$(id -u)" > "$TEST_CONFIG"

# Create waste dir manually (required because it is marked removable)
mkdir -p "$TRASH_DIR/files" "$TRASH_DIR/info"
mkdir -p "$TEST_DIR"

touch "$test_file_path"
"$BIN_DIR"/rmw -c "$TEST_CONFIG" "$test_file_path"

test -f "$TRASH_DIR/info/$test_file.trashinfo"
test -f "$TRASH_DIR/files/$test_file"
test ! -f "$test_file_path"

# The Path must be relative (no leading '/') since the waste folder is at
# the topdir of /tmp's partition.
output=$(grep '^Path=' "$TRASH_DIR/info/$test_file.trashinfo")
echo "trashinfo: $output"
case "$output" in
  Path=/*)
    echo "FAIL: Path is absolute, expected relative"
    exit 1
    ;;
  Path=*)
    echo "PASS: Path is relative"
    ;;
  *)
    echo "FAIL: Path line not found"
    exit 1
    ;;
esac

# Restore and verify
"$BIN_DIR"/rmw -uvv -c "$TEST_CONFIG"
test -f "$test_file_path"
test ! -f "$TRASH_DIR/info/$test_file.trashinfo"

rm -f "$TEST_CONFIG"

exit 0
