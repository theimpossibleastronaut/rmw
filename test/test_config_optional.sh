#!/bin/sh
# Configuration is optional (since 0.10.0): with no configured waste folder,
# rmw does not exit; it moves a home-filesystem file to the home trash
# (~/.local/share/Trash), created on demand. First run also creates a
# comment-only config and prints a one-time notice.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

mkdir -p "$RMW_FAKE_HOME"
HOME_TRASH="$RMW_FAKE_HOME/.local/share/Trash"
CONFIG="$RMW_FAKE_HOME/optional.rc"   # does not exist yet -> first run creates it

cd "$RMW_FAKE_HOME"

# --- first run: no config present, no waste configured ---
echo data > victim
out=$("$BIN_DIR"/rmw -c "$CONFIG" victim 2>&1)
echo "$out"

# A config was created, and it carries no active WASTE line (comments only).
test -f "$CONFIG"
if grep -Eq '^[[:space:]]*WASTE' "$CONFIG"; then
  echo "FAIL: a freshly created config must have no active WASTE line"
  exit 1
fi

# The one-time first-run notice was shown.
cmp_substr "$out" "moves each file to the trash"

# The file went to the home trash, created on demand, with an absolute Path.
test ! -e victim
test -f "$HOME_TRASH/files/victim"
test -f "$HOME_TRASH/info/victim.trashinfo"
grep -q '^Path=/' "$HOME_TRASH/info/victim.trashinfo"

# --- second run: config now exists, notice must not repeat ---
echo data > victim2
out2=$("$BIN_DIR"/rmw -c "$CONFIG" victim2 2>&1)
echo "$out2"
if cmp_substr "$out2" "moves each file to the trash"; then
  echo "FAIL: first-run notice must not repeat once the config exists"
  exit 1
fi
test -f "$HOME_TRASH/files/victim2"

echo "PASS: config optional — home file trashed to home trash, notice shown once"
exit 0
