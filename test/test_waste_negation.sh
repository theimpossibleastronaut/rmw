#!/bin/sh
# "!WASTE = /path" marks a waste folder that never receives files when
# removing, but still participates in listing, restoring, and purging.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

WASTE_NEG="$RMW_FAKE_HOME/.WasteNeg"
WASTE_OK="$RMW_FAKE_HOME/.Waste"
TEST_CONFIG="$RMW_FAKE_HOME/negation.testrc"

mkdir -p "$RMW_FAKE_HOME"
printf '!WASTE = %s\nWASTE = %s\nexpire_age = 1\n' \
  "$WASTE_NEG" "$WASTE_OK" > "$TEST_CONFIG"
RMW="$BIN_DIR/rmw -c $TEST_CONFIG"

# Plant two items in the negated folder: one to restore, one (long expired)
# to purge.
mkdir -p "$WASTE_NEG/files" "$WASTE_NEG/info"
echo old > "$WASTE_NEG/files/purgeme"
echo old > "$WASTE_NEG/files/restoreme"
for f in purgeme restoreme; do
  cat > "$WASTE_NEG/info/$f.trashinfo" << TRASHINFO
[Trash Info]
Path=$RMW_FAKE_HOME/$f
DeletionDate=2020-01-01T00:00:00
TRASHINFO
done

cd "$RMW_FAKE_HOME"

# --- removals skip the negated folder, even listed first on the same fs ---
touch newfile
$RMW newfile
test ! -e newfile
test -f "$WASTE_OK/files/newfile"
test ! -e "$WASTE_NEG/files/newfile"

# --- plain -l stays a bare, pipeable list of paths ---
out=$($RMW -l)
echo "$out"
cmp_substr "$out" "$WASTE_NEG"
cmp_substr "$out" "$WASTE_OK"
if cmp_substr "$out" "!$WASTE_NEG"; then
  echo "FAIL: '!' marker must not appear in the bare -l list"
  exit 1
fi

# --- with -v, the negated folder is marked with the config's '!' syntax ---
outv=$($RMW -l -v)
echo "$outv"
cmp_substr "$outv" "!$WASTE_NEG"

# --- restore from the negated folder works ---
$RMW -z "$WASTE_NEG/files/restoreme"
test -f "$RMW_FAKE_HOME/restoreme"
test ! -e "$WASTE_NEG/files/restoreme"
test ! -e "$WASTE_NEG/info/restoreme.trashinfo"

# --- purge reaches into the negated folder ---
$RMW -g
test ! -e "$WASTE_NEG/files/purgeme"
test ! -e "$WASTE_NEG/info/purgeme.trashinfo"
# the file just removed today is not expired and must survive the purge
test -f "$WASTE_OK/files/newfile"

echo "PASS: !WASTE folder skipped for removals, honored for list/restore/purge"
exit 0
