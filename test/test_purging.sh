#!/bin/sh
# purging.sh: tests rmw's purging facilities
#
# This script can be used to demonstrate the purging feature of rmw to
# new users, and for testing for bugs
#
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

echo "== On first run, directories should get created"
# show commands that are run


$RMW_TEST_CMD_STRING

# Make some temporary files using a fake-year
mkdir ${RMW_FAKE_HOME}/tmp-files
cd ${RMW_FAKE_HOME}/tmp-files


create_temp_files() {
  cd ${RMW_FAKE_HOME}/tmp-files

  echo "\n\n == creating temporary files to be rmw'ed"

  for file in abc 123 xyz; do
    i=0
    while [ "$i" -lt 8 ]
    do
      echo "0000" >> $file
      i=`expr $i + 1`
    done
  done
}

echo
echo
echo " Creating some files for testing..."
cd ${RMW_FAKE_HOME}
create_some_files

echo
echo
echo " == rmw -e command should completely purge the waste folders,"
echo " == no matter when they were deleted"

$VALGRIND $RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/somefiles/*

echo $SEPARATOR
echo '  == purging disabled should output a message that purging is disabled'
output=`$RMW_PURGE_DISABLED_CMD -eff`
expected=`echo "purging is disabled ('expire_age' is set to '0')" | cut -b1-20`
output=`echo $output | cut -b1-20`
test "${output}" = "${expected}"

# Should not work if '-f' isn't used"
substring="purge has been skipped"
output=`echo "y" | $RMW_ALT_TEST_CMD_STRING --purge -e`
test "${output}#*${substring}" != "${output}"

echo " == Should not work if 'Y' or 'y' is not supplied."
echo "yfw" | $VALGRIND $RMW_TEST_CMD_STRING --purge -e

echo "Yfw" | $RMW_TEST_CMD_STRING --purge -e

echo "n" | $RMW_TEST_CMD_STRING --purge -e

echo "Nqer" | $RMW_TEST_CMD_STRING --purge -e

echo " == (files should still be present)"
test -e $PRIMARY_WASTE_DIR/files/read_only_file
test -e $PRIMARY_WASTE_DIR/files/topdir

echo $SEPARATOR
echo " == Make sure the correct string (filename) is displayed when using -vvg"
substring="'read_only_file' will be purged in 90."
output=$($VALGRIND $RMW_TEST_CMD_STRING -vvg)
test "${output}#*${substring}" != "${output}"

substring="'topdir' will be purged in 90."
test "${output}#*${substring}" != "${output}"

echo $SEPARATOR

echo " == Empty works with force empty (-ffe)"
echo "y" | $VALGRIND $RMW_TEST_CMD_STRING -ffe
test ! -e $PRIMARY_WASTE_DIR/files/read_only_file
test ! -e $PRIMARY_WASTE_DIR/files/topdir
test ! -e $PRIMARY_WASTE_DIR/info/read_only_file.trashinfo
test ! -e $PRIMARY_WASTE_DIR/info/topdir.trashinfo

echo " == Should work with 'Y' or 'y'"
create_temp_files
$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/* && echo "y" | $RMW_TEST_CMD_STRING -e

create_temp_files
$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/*
test -e $PRIMARY_WASTE_DIR/files/abc
test -e $PRIMARY_WASTE_DIR/files/123
test -e $PRIMARY_WASTE_DIR/files/xyz
echo "Y" | $RMW_TEST_CMD_STRING -e

echo
echo
echo " == Test that the files are really gone"

test ! -e $PRIMARY_WASTE_DIR/files/abc
test ! -e $PRIMARY_WASTE_DIR/info/abc.trashinfo
test ! -e $PRIMARY_WASTE_DIR/files/123
test ! -e $PRIMARY_WASTE_DIR/info/123.trashinfo
test ! -e $PRIMARY_WASTE_DIR/files/xyz
test ! -e $PRIMARY_WASTE_DIR/info/xyz.trashinfo

create_temp_files

cd ${RMW_FAKE_HOME}/..
echo "\n\n == use a built-in environmental variable to write a"
echo " == fake year to the .trashinfo files when running rmw"
echo "-----------------------------------------------------\n"

RMW_FAKE_YEAR=true $VALGRIND $RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/*
output=$(cat $PRIMARY_WASTE_DIR/info/abc.trashinfo)
substring="DeletionDate=1999"
test "${output}#*${substring}" != "${output}"

echo
echo
echo " == The trashinfo records that these files were rmw'ed in 1999"
echo " == So they will be purged now."
$RMW_TEST_CMD_STRING --verbose --purge
test ! -e $PRIMARY_WASTE_DIR/files/abc
test ! -e $PRIMARY_WASTE_DIR/info/abc.trashinfo
test ! -e $PRIMARY_WASTE_DIR/files/123
test ! -e $PRIMARY_WASTE_DIR/info/123.trashinfo
test ! -e $PRIMARY_WASTE_DIR/files/xyz
test ! -e $PRIMARY_WASTE_DIR/info/xyz.trashinfo

echo
echo
echo " Creating some files for testing..."
cd ${RMW_FAKE_HOME}
create_some_files

echo $SEPARATOR
echo " == rmw should be able to purge directories and subdirectories"
echo " == even if some of the dirs are read-only"

echo $SEPARATOR

RMW_FAKE_YEAR=True $RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/somefiles

echo $SEPARATOR
echo " == only one f, should fail"
$RMW_TEST_CMD_STRING -f --purge

echo $SEPARATOR
echo " == (files should still be present)"
test -e $PRIMARY_WASTE_DIR/files/somefiles
test -e $PRIMARY_WASTE_DIR/info/somefiles.trashinfo

echo $SEPARATOR
echo " == 2 f's, this should pass"
$RMW_TEST_CMD_STRING -ff --purge

echo
echo
echo " == Show that the files are really gone"

test ! -e $PRIMARY_WASTE_DIR/files/somefiles
test ! -e $PRIMARY_WASTE_DIR/info/somefiles.trashinfo

$RMW_TEST_CMD_STRING -o

exit 0
