#!/bin/sh
# Discovery of FreeDesktop $topdir trashes, exercised against mounts the test
# fully controls inside an unprivileged mount namespace (no sudo).
#
# The suite keeps discovery off by default (RMW_DISCOVERY=off); per command we
# set RMW_DISCOVERY=on so the binary scans the controlled mounts. The namespace
# inherits a copy of the host's mount table, so assertions are additive: we
# check our specific planted paths, not a closed list. Mounts vanish when the
# namespace exits.
#
# Covered: the excluded-filesystem rule (tmpfs is excluded) --
#   * an existing trash on an excluded fs IS discovered (evidence of intent)
#   * an excluded fs with no trash is NOT discovered or offered as a candidate

set -ve

# Re-exec the whole script inside a private mount namespace (mapped to root
# so mount works) before sourcing COMMON, so COMMON runs exactly once. If
# unprivileged user/mount namespaces are unavailable, skip (77, matching
# COMMON's SKIP).
if [ -z "$RMW_DISCOVERY_NS" ]; then
  if ! unshare --mount --map-root-user true 2>/dev/null; then
    echo "unprivileged mount namespace unavailable; skipping discovery test."
    exit 77
  fi
  RMW_DISCOVERY_NS=1 exec unshare --mount --map-root-user "$0" "$@"
fi

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# Inside --map-root-user the process (and rmw's getuid()) is uid 0, so the
# trash dir rmw looks for is .Trash-0; id -u agrees.
TRASH=".Trash-$(id -u)"

ROOT="/tmp/rmw-discovery"
HAVE="$ROOT/have"              # excluded fs (tmpfs) WITH a pre-existing trash
BARE="$ROOT/bare"             # excluded fs (tmpfs) with NO trash
rm -rf "$ROOT"
mkdir -p "$HAVE" "$BARE"
mount -t tmpfs none "$HAVE"
mount -t tmpfs none "$BARE"

mkdir -p "$HAVE/$TRASH/files" "$HAVE/$TRASH/info"

TEST_CONFIG="$RMW_FAKE_HOME/discovery.testrc"
mkdir -p "$RMW_FAKE_HOME"
printf 'WASTE = %s/.Waste\nexpire_age = 30\n' "$RMW_FAKE_HOME" > "$TEST_CONFIG"
RMW="$BIN_DIR/rmw -c $TEST_CONFIG"

# Discovery is opted in for this command with RMW_DISCOVERY=on.
out=$(RMW_DISCOVERY=on $RMW -l 2>&1)
echo "$out"

# The tmpfs with an existing trash is discovered and listed ...
cmp_substr "$out" "$HAVE/$TRASH"
# ... the bare tmpfs is not (no trash dir to honor).
if cmp_substr "$out" "$BARE/$TRASH"; then
  echo "FAIL: a trash-less excluded fs must not be discovered"
  exit 1
fi
# ... and it's not offered as a candidate either (excluded fs, -l -v).
outv=$(RMW_DISCOVERY=on $RMW -l -v 2>&1)
if cmp_substr "$outv" "$BARE/$TRASH"; then
  echo "FAIL: an excluded fs must not be a candidate trash location"
  exit 1
fi

# A removal reaches the discovered trash on the excluded fs.
echo data > "$HAVE/victim"
RMW_DISCOVERY=on $RMW -v "$HAVE/victim"
test ! -e "$HAVE/victim"
test -f "$HAVE/$TRASH/files/victim"
test -f "$HAVE/$TRASH/info/victim.trashinfo"

echo "PASS: existing trash on an excluded fs discovered; trash-less one ignored"
exit 0
