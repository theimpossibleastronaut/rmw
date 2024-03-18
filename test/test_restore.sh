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

echo $SEPARATOR
echo "Initialize"
$RMW_TEST_CMD_STRING

echo $SEPARATOR
echo "If the mrl file doesn't exist yet..."
cmp_substr "$($RMW_TEST_CMD_STRING -u)" \
  "no items in the list"

echo $SEPARATOR
echo " Creating some files for testing..."
cd ${RMW_FAKE_HOME}
create_some_files

echo $SEPARATOR
echo "Try to restore files that aren't in a Waste/files folder"
cmp_substr "$($RMW_TEST_CMD_STRING -z ${RMW_FAKE_HOME}/somefiles/* 2>&1 && exit 1)" \
  "not in a Waste directory"

echo $SEPARATOR
echo "ReMove files and then restore them by using -u"
$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/somefiles/*
echo $SEPARATOR
output=$($RMW_TEST_CMD_STRING -uvv | grep Waste)
echo $SEPARATOR
echo "OUTPUT:"
echo "---"
echo "$output"
echo "---"
test "$output" = "+'${RMW_FAKE_HOME}/.Waste/files/read_only_file' -> '${RMW_FAKE_HOME}/somefiles/read_only_file'
-${RMW_FAKE_HOME}/.Waste/info/read_only_file.trashinfo
+'${RMW_FAKE_HOME}/.Waste/files/topdir' -> '${RMW_FAKE_HOME}/somefiles/topdir'
-${RMW_FAKE_HOME}/.Waste/info/topdir.trashinfo"

echo $SEPARATOR
echo "Show result when no undo file exists..."
output=$(${RMW_TEST_CMD_STRING} -u)
test "${output}" = "There are no items in the list - please check back later."

echo $SEPARATOR
echo "restore using wildcard pattern, but be in the trash directory"
$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/somefiles/topdir -v
cd $PRIMARY_WASTE_DIR/files
$RMW_TEST_CMD_STRING -z topd*
test -e "${RMW_FAKE_HOME}/somefiles/topdir"

echo $SEPARATOR
echo "Try restoring a file that doesn't exist"
$RMW_TEST_CMD_STRING -z nonexistent_fil* && exit 1

# This test is inaccurate when run with superuser privileges.

#echo $SEPARATOR
#echo "What happens when trying to ReMove file inside dir with no write permissions..."
#mkdir ${RMW_FAKE_HOME}/no_write_perms
#touch ${RMW_FAKE_HOME}/no_write_perms/test.tmp
#chmod -R -w ${RMW_FAKE_HOME}/no_write_perms
#$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/no_write_perms
#test_result_want_fail $?

echo $SEPARATOR
echo "Symlinks"
ln -s ${RMW_FAKE_HOME} ${RMW_FAKE_HOME}/link_test
# broken link
ln -s broken_symlink_test ${RMW_FAKE_HOME}/link_test2
$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/link_test ${RMW_FAKE_HOME}/link_test2
test -h "${PRIMARY_WASTE_DIR}/files/link_test"

${RMW_TEST_CMD_STRING} -u
test -h "${RMW_FAKE_HOME}/link_test2"

# test using relative path and dotfiles
cd ${RMW_FAKE_HOME}
mkdir tmpfoo
for t in "foo" "bar" ".boo" ".far"; do
  touch tmpfoo/$t
  ${RMW_TEST_CMD_STRING} tmpfoo/$t
  cat ${PRIMARY_WASTE_DIR}/info/$t.trashinfo
  cd ${PRIMARY_WASTE_DIR}
  ${RMW_TEST_CMD_STRING} -z files/$t
  cd ${RMW_FAKE_HOME}
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

if [ -n "${TERM}" ] && [ "${TERM}" != "dumb" ]; then
  echo q | ${RMW_TEST_CMD_STRING} -s
fi

# This test will only work on Andy's workstation.
# The media root, /home/andy/src is on a different partition than /home/andy
# It's mounted with 'bind' and therefore has a different device id
#
# /dev/sda7 on /mnt/sda7 type ext4 (rw,relatime)
# /dev/sda7 on /home/andy/src type ext4 (rw,relatime)

if test -d /mnt/918375c2; then
  test_file="media_root_test"
  PREV_RMW_FAKE_HOME=${RMW_FAKE_HOME}
  # needs to be unset so rmw will use $HOME instead
  unset RMW_FAKE_HOME
  test_file_path=${PREV_RMW_FAKE_HOME}/$test_file
  if test -f $test_file_path; then
    rm $test_file_path
  fi
  if test -f /home/andy/src/rmw-project/.Trash-1000/files/$test_file; then
    rm /home/andy/src/rmw-project/.Trash-1000/files/$test_file
  fi
  if test -f /home/andy/src/rmw-project/.Trash-1000/info/$test_file.trashinfo; then
    rm /home/andy/src/rmw-project/.Trash-1000/info/$test_file.trashinfo
  fi
  touch $test_file_path
  $BIN_DIR/rmw -c /home/andy/.config/rmwrc $test_file_path
  echo $SEPARATOR
  output=$(grep Path /home/andy/src/rmw-project/.Trash-1000/info/$test_file.trashinfo)
  echo $SEPARATOR
  # There should be no leading '/' in the filename.
  path_expected=$(echo ${MESON_BUILD_ROOT} | sed -e "s/\/home\/andy\/src\/rmw-project\///g")
  echo $path_expected
  test "$output" = "Path=${path_expected}/test/rmw-tests-home/test_restore.sh_dir/media_root_test"
  echo $SEPARATOR
  output=$($BIN_DIR/rmw -uvv -c /home/andy/.config/rmwrc | grep media_root_test)
  echo $SEPARATOR
  test "$output" = "+'/home/andy/src/rmw-project/.Trash-1000/files/media_root_test' -> '${MESON_BUILD_ROOT}/test/rmw-tests-home/test_restore.sh_dir/media_root_test'
-/home/andy/src/rmw-project/.Trash-1000/info/media_root_test.trashinfo"
fi
echo $SEPARATOR
echo "restore tests passed"

exit 0
