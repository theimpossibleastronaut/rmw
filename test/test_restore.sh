#!/bin/sh
# restore.sh: tests rmw's restore features
#
# This script can be used to demonstrate the restore features of rmw to
# new users, and to test for bugs when making code changes
#
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

echo "$SEPARATOR"
echo "Initialize"
$RMW_TEST_CMD_STRING

echo "$SEPARATOR"
echo "If the mrl file doesn't exist yet..."
cmp_substr "$($RMW_TEST_CMD_STRING -u)" \
  "no items in the list"

echo "$SEPARATOR"
echo " Creating some files for testing..."
cd "${RMW_FAKE_HOME}"
create_some_files

echo "$SEPARATOR"
echo "Try to restore files that aren't in a Waste/files folder"
cmp_substr "$($RMW_TEST_CMD_STRING -z "${RMW_FAKE_HOME}"/somefiles/* 2>&1 && exit 1)" \
  "not in a Waste directory"

echo "$SEPARATOR"
echo "ReMove files and then restore them by using -u"
$RMW_TEST_CMD_STRING "${RMW_FAKE_HOME}"/somefiles/*
echo "$SEPARATOR"
output=$($RMW_TEST_CMD_STRING -uvv | grep Waste)
echo "$SEPARATOR"
echo "OUTPUT:"
echo "---"
echo "$output"
echo "---"
test "$output" = "+'"${RMW_FAKE_HOME}"/.Waste/files/read_only_file' -> '"${RMW_FAKE_HOME}"/somefiles/read_only_file'
-"${RMW_FAKE_HOME}"/.Waste/info/read_only_file.trashinfo
+'"${RMW_FAKE_HOME}"/.Waste/files/topdir' -> '"${RMW_FAKE_HOME}"/somefiles/topdir'
-"${RMW_FAKE_HOME}"/.Waste/info/topdir.trashinfo"

echo "$SEPARATOR"
echo "Show result when no undo file exists..."
output=$(${RMW_TEST_CMD_STRING} -u || true)
test "${output}" = "There are no items in the list - please check back later."

echo "$SEPARATOR"
echo "restore using wildcard pattern, but be in the trash directory"
$RMW_TEST_CMD_STRING "${RMW_FAKE_HOME}"/somefiles/topdir -v
cd "$PRIMARY_WASTE_DIR"/files
$RMW_TEST_CMD_STRING -z topd*
test -e "${RMW_FAKE_HOME}"/somefiles/topdir

echo "$SEPARATOR"
echo "Try restoring a file that doesn't exist"
$RMW_TEST_CMD_STRING -z nonexistent_fil* && exit 1

# This test is inaccurate when run with superuser privileges.

#echo "$SEPARATOR"
#echo "What happens when trying to ReMove file inside dir with no write permissions..."
#mkdir "${RMW_FAKE_HOME}"/no_write_perms
#touch "${RMW_FAKE_HOME}"/no_write_perms/test.tmp
#chmod -R -w "${RMW_FAKE_HOME}"/no_write_perms
#$RMW_TEST_CMD_STRING "${RMW_FAKE_HOME}"/no_write_perms
#test_result_want_fail $?

echo "$SEPARATOR"
echo "Symlinks"
ln -s ${RMW_FAKE_HOME} "${RMW_FAKE_HOME}"/link_test
# broken link
ln -s broken_symlink_test "${RMW_FAKE_HOME}"/link_test2
$RMW_TEST_CMD_STRING "${RMW_FAKE_HOME}"/link_test "${RMW_FAKE_HOME}"/link_test2
test -h "${PRIMARY_WASTE_DIR}/files/link_test"

${RMW_TEST_CMD_STRING} -u
test -h "${RMW_FAKE_HOME}"/link_test2

# test using relative path and dotfiles
cd "${RMW_FAKE_HOME}"
mkdir tmpfoo
for t in "foo" "bar" ".boo" ".far"; do
  touch tmpfoo/$t
  ${RMW_TEST_CMD_STRING} tmpfoo/$t
  cat ${PRIMARY_WASTE_DIR}/info/$t.trashinfo
  cd ${PRIMARY_WASTE_DIR}
  ${RMW_TEST_CMD_STRING} -z files/$t
  cd "${RMW_FAKE_HOME}"
  test -f tmpfoo/$t
done

# Now try dotfiles with wildcards
for t in ".boo" ".far"; do
  touch tmpfoo/$t
  ${RMW_TEST_CMD_STRING} tmpfoo/.*
  ${RMW_TEST_CMD_STRING} -z ${PRIMARY_WASTE_DIR}/files/$t
  test -f tmpfoo/$t
done

# a dot dir
cmp_substr "$(${RMW_TEST_CMD_STRING} -z ${PRIMARY_WASTE_DIR}/files/. && exit 1)" \
  "refusing to process"

# I don't want to force anyone to install Xvfb for this single test
# so I'll only run it if it's already installed
if [ -n "$(command -v Xvfb)" ] && [ ! $(grep "DISABLE_CURSES" "$MESON_BUILD_ROOT/src/config.h") ]; then
  # Start Xvfb on display :99
  Xvfb :99 &
  XVFB_PID=$!

  # Save the current DISPLAY value and set it to use the virtual display
  OLD_DISPLAY="$DISPLAY"
  export DISPLAY=:99

  # This may be needed to prevent a failure on OpenBSD:
  # Error opening terminal: unknown.
  export TERM=xterm

  cd "$RMW_FAKE_HOME"
  for file in "foo(1)far" "far side" "clippity-clap"; do
    touch "$file"
    ${RMW_TEST_CMD_STRING} "$file"
  done
  # No visual test here, but when used with llvm sanitize or valgrind,
  # the chances of spotting any memory leaks are pretty good.
  # https://github.com/theimpossibleastronaut/rmw/issues/464
  # rmw -s in some cases, when built using _FORTIFY=3, results in an immediate crash
  ${RMW_TEST_CMD_STRING} -s &
  RMW_PID=$!
  sleep 1 && kill $RMW_PID # OpenBSD sleep doesn't accept '1s'
  kill $XVFB_PID

  # Restore the original DISPLAY value if it was set
  if [ -n "$OLD_DISPLAY" ]; then
    export DISPLAY="$OLD_DISPLAY"
  else
    unset DISPLAY
  fi
fi

exit 0
