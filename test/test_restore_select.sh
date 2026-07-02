#!/bin/sh
# test_restore_select.sh: the non-interactive fallback for 'rmw -s'. When
# stdout is not a terminal (piped/redirected, as under 'meson test'), the
# ncurses menu can't run, so -s dumps the active waste -- the trash on the
# current filesystem -- and its contents instead. This is the scriptable,
# testable form of the "-s starts on the current filesystem" behavior (#532).
#
# Single filesystem, no privileges: this covers the dump plumbing and format.
# The cross-filesystem selection is covered by test_restore_select_multifs.sh.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# First run creates the waste directories.
$RMW_TEST_CMD_STRING

cd "${RMW_FAKE_HOME}"
echo data > alpha
echo data > beta
# Both files are on the home filesystem, so they land in the first waste
# configured there ($HOME/.Waste).
$RMW_TEST_CMD_STRING alpha beta

# stdout is captured (not a tty), so -s takes the dump path.
out=$($RMW_TEST_CMD_STRING -s)
printf '%s\n' "$out"

# Line 1 is the active waste: the trash on the current filesystem, not a
# removable or other-filesystem waste listed in the config.
active=$(printf '%s\n' "$out" | head -n 1)
test "$active" = "${RMW_FAKE_HOME}/.Waste"

# ...and its contents are listed.
cmp_substr "$out" "alpha"
cmp_substr "$out" "beta"

echo "PASS: -s without a tty dumped the current filesystem's waste"
exit 0
