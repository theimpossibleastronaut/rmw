#!/bin/sh
# A waste folder at the $topdir of a mount (media_root) must store a
# *relative* Path in the .trashinfo, per the FreeDesktop trash spec:
# https://specifications.freedesktop.org/trash-spec/1.0/#id-1.6.10.1
# (absolute pathnames are only for the home trash, not for $topdir trashes).
#
# The mount path here deliberately contains a space. create_trashinfo()
# strips media_root from the UNESCAPED real path before percent-escaping; an
# earlier bug stripped it from the escaped path by raw byte length, so a space
# (escaped to %20) desynced the offset and aborted. Testing a spaced mount
# therefore covers both the relative-Path rule and that escape regression.
#
# Self-contained: runs inside an unprivileged mount namespace (no sudo) with a
# dedicated tmpfs, so it never assumes the host's mount layout and never writes
# to the real /tmp. The mount vanishes when the namespace exits.

set -ve

if [ -z "$RMW_MEDIA_ROOT_NS" ]; then
  if ! unshare --mount --map-root-user true 2>/dev/null; then
    echo "unprivileged mount namespace unavailable; skipping."
    exit 77
  fi
  RMW_MEDIA_ROOT_NS=1 exec unshare --mount --map-root-user "$0" "$@"
fi

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

MNT="/tmp/rmw media root"        # mount path with a space
WASTE="$MNT/.Trash-$(id -u)"     # top-level waste -> media_root gets set
rm -rf "$MNT"
mkdir -p "$MNT"
mount -t tmpfs none "$MNT"

CONFIG="$RMW_FAKE_HOME/media-root.rc"
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

# Restore round-trip: the relative Path must resolve back to the original
# location, and the .trashinfo must be removed.
"$BIN_DIR"/rmw -u -c "$CONFIG"
test -f "$MNT/victim"
test ! -f "$WASTE/info/victim.trashinfo"

echo "PASS: media_root produced a correct relative Path and restored cleanly"
exit 0
