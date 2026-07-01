#!/bin/sh
# test_restore_select_multifs.sh: 'rmw -s' (non-tty dump) must select the trash
# on the CURRENT filesystem when several wastes exist -- the behavior issue #532
# asked for. Exercised across two real mounts the test fully controls inside an
# unprivileged mount namespace (no sudo). SKIP where namespaces are unavailable.
#
# Everything lives on private tmpfs mounts that vanish when the namespace exits.

set -ve

# Re-exec inside a private mount namespace (mapped to root so mount works)
# before sourcing COMMON, so COMMON runs exactly once. If unprivileged
# user/mount namespaces are unavailable, skip (77).
if [ -z "$RMW_SELECT_NS" ]; then
  if ! unshare --mount --map-root-user true 2>/dev/null; then
    echo "unprivileged mount namespace unavailable; skipping."
    exit 77
  fi
  RMW_SELECT_NS=1 exec unshare --mount --map-root-user "$0" "$@"
fi

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# The -s dump lives in the ncurses restore-menu code; a build without curses
# just prints "built without menu support", so there is nothing to check.
if grep -q "DISABLE_CURSES" "$MESON_BUILD_ROOT/src/config.h" 2>/dev/null; then
  echo "built without curses; skipping -s dump test."
  exit "$SKIP"
fi

ROOT="/tmp/rmw-select-multifs"
A="$ROOT/a"                    # its own tmpfs (distinct device)
B="$ROOT/b"                    # a second tmpfs (another device)
rm -rf "$ROOT"
mkdir -p "$A" "$B"
mount -t tmpfs none "$A"
mount -t tmpfs none "$B"
mkdir -p "$A/waste" "$B/waste"

# A configured waste on each filesystem.
CONFIG="$RMW_FAKE_HOME/select-multifs.testrc"
mkdir -p "$RMW_FAKE_HOME"
printf 'WASTE = %s/waste\nWASTE = %s/waste\nexpire_age = 30\n' "$A" "$B" \
  > "$CONFIG"
RMW="$BIN_DIR/rmw -c $CONFIG"

# A file on each filesystem, removed to that filesystem's waste.
echo data > "$A/file_a"
echo data > "$B/file_b"
$RMW "$A/file_a"
$RMW "$B/file_b"

# From mount A, the active (nearest-by-device) waste is A's, listing file_a.
cd "$A"
out=$($RMW -s)
printf '%s\n' "$out"
test "$(printf '%s\n' "$out" | head -n 1)" = "$A/waste"
cmp_substr "$out" "file_a"

# From mount B, the active waste is B's, listing file_b. The configured order
# (A before B) does not override the current filesystem.
cd "$B"
out=$($RMW -s)
printf '%s\n' "$out"
test "$(printf '%s\n' "$out" | head -n 1)" = "$B/waste"
cmp_substr "$out" "file_b"

echo "PASS: -s selected the current filesystem's waste on each mount"
exit 0
