#!/bin/sh
# Regression test: discovery must skip mount points that are files, not
# directories. Containers (Docker) and some systemd setups bind-mount single
# files like /etc/resolv.conf; before the fix, discovery probed "<file>/.Trash"
# and printed ":error: lstat ...: Not a directory" for each one.
#
# Built inside an unprivileged mount namespace (no sudo). RMW_DISCOVERY=on
# opts discovery in for the command so it enumerates our file mount.

set -ve

# Re-exec inside a private mount namespace (mapped to root so mount works)
# before sourcing COMMON. Skip (77) if unprivileged userns is unavailable.
if [ -z "$RMW_FILEMNT_NS" ]; then
  if ! unshare --mount --map-root-user true 2>/dev/null; then
    echo "unprivileged mount namespace unavailable; skipping file-mount test."
    exit 77
  fi
  RMW_FILEMNT_NS=1 exec unshare --mount --map-root-user "$0" "$@"
fi

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

ROOT="/tmp/rmw-filemnt"
SRC="$ROOT/source_file"
MNT="$ROOT/etcfile"            # a regular file used as a bind-mount target
rm -rf "$ROOT"
mkdir -p "$ROOT"
echo src > "$SRC"
# Make it executable so the file mount passes rmw's access(R_OK|X_OK) mount
# filter and reaches the directory probe -- without sudo we can only make a
# tmpfs mount here, which rmw treats as an excluded fs; on a real disk (the
# Docker /etc/resolv.conf case) an eligible file mount reaches the same probe.
chmod 0755 "$SRC"
: > "$MNT"
mount --bind "$SRC" "$MNT"     # $MNT is now a *file* mount point
mountpoint -q "$MNT"           # sanity: it really is a mount

CONFIG="$RMW_FAKE_HOME/filemnt.rc"
mkdir -p "$RMW_FAKE_HOME"
printf 'expire_age = 30\n' > "$CONFIG"

cd "$RMW_FAKE_HOME"

# -l forces discovery to enumerate every mount, including the file mount.
out=$(RMW_DISCOVERY=on "$BIN_DIR"/rmw -c "$CONFIG" -l 2>&1)
echo "$out"

# The file mount must be skipped silently: no error, and its path must not
# appear in any "<path>/.Trash" probe.
if cmp_substr "$out" "Not a directory"; then
  echo "FAIL: discovery errored on a file bind-mount"
  exit 1
fi
if cmp_substr "$out" "$MNT"; then
  echo "FAIL: a file bind-mount must not appear in discovery output"
  exit 1
fi

# And normal operation still works with the file mount present.
echo data > victim
RMW_DISCOVERY=on "$BIN_DIR"/rmw -c "$CONFIG" victim
test ! -e victim
test -f "$RMW_FAKE_HOME/.local/share/Trash/files/victim"

echo "PASS: file bind-mount skipped by discovery; removal unaffected"
exit 0
