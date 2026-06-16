#!/bin/sh
# Regression test: a top-level (media_root) waste folder whose mount path
# contains a space must produce a correct relative Path= in the .trashinfo,
# not a corrupt one or a crash. Before the fix, create_trashinfo() stripped
# media_root from the percent-ESCAPED path by raw byte length, so a space
# (escaped to %20) desynced the offset and aborted via diag_fatal.
#
# Built inside an unprivileged mount namespace (no sudo): a tmpfs mounted at
# a path containing a space, with a WASTE folder at its top level so
# media_root is set. The mount vanishes when the namespace exits.

set -ve

if [ -z "$RMW_MRSPACE_NS" ]; then
  if ! unshare --mount --map-root-user true 2>/dev/null; then
    echo "unprivileged mount namespace unavailable; skipping."
    exit 77
  fi
  RMW_MRSPACE_NS=1 exec unshare --mount --map-root-user "$0" "$@"
fi

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

MNT="/tmp/rmw mr space"          # mount path with a space
WASTE="$MNT/.Trash-$(id -u)"     # top-level waste -> media_root gets set
rm -rf "$MNT"
mkdir -p "$MNT"
mount -t tmpfs none "$MNT"

CONFIG="$RMW_FAKE_HOME/mrspace.rc"
mkdir -p "$RMW_FAKE_HOME"
printf 'WASTE = %s\nexpire_age = 30\n' "$WASTE" > "$CONFIG"

echo data > "$MNT/victim"

# Must not crash; the file must land in the spaced-path waste.
"$BIN_DIR"/rmw -c "$CONFIG" "$MNT/victim"
test ! -e "$MNT/victim"
test -f "$WASTE/files/victim"
test -f "$WASTE/info/victim.trashinfo"

# Path= must be the correct *relative* path (no leading '/', no %20 mangling).
path_line=$(grep '^Path=' "$WASTE/info/victim.trashinfo")
echo "$path_line"
test "$path_line" = "Path=victim"

echo "PASS: spaced media_root produced a correct relative Path="
exit 0
