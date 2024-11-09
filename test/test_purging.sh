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
echo " == rmw --empty command should completely purge the waste folders,"
echo " == no matter when they were deleted"

$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/somefiles/*

echo $SEPARATOR
echo '  == purging disabled should output a message that purging is disabled'
output=`$RMW_PURGE_DISABLED_CMD --empty -f`
expected=`echo "purging is disabled ('expire_age' is set to '0')" | cut -b1-20`
output=`echo $output | cut -b1-20`
test "${output}" = "${expected}"

# Should not work if '-f' isn't used"
substring=
cmp_substr "$(echo y | $RMW_ALT_TEST_CMD_STRING --purge --empty)" \
  "purge has been skipped"

echo " == Should not work if 'Y' or 'y' is not supplied."
echo "yfw" | $RMW_TEST_CMD_STRING --purge --empty

echo "Yfw" | $RMW_TEST_CMD_STRING --purge --empty

echo "n" | $RMW_TEST_CMD_STRING --purge --empty

echo "Nqer" | $RMW_TEST_CMD_STRING --purge --empty

echo " == (files should still be present)"
test -e $PRIMARY_WASTE_DIR/files/read_only_file
test -e $PRIMARY_WASTE_DIR/files/topdir

echo $SEPARATOR
echo " == Make sure the correct string (filename) is displayed when using -vvg"
output="$($RMW_TEST_CMD_STRING -vvg)"
echo $output
cmp_substr "$output"  "'read_only_file' will be purged in 90$(locale decimal_point)"
cmp_substr "$output" "'topdir' will be purged in 90$(locale decimal_point)"

echo $SEPARATOR
echo " == Empty works with force empty (-fe)"
echo "y" | $RMW_TEST_CMD_STRING -v --empty -ff
test ! -e $PRIMARY_WASTE_DIR/files/read_only_file
test ! -e $PRIMARY_WASTE_DIR/files/topdir
test ! -e $PRIMARY_WASTE_DIR/info/read_only_file.trashinfo
test ! -e $PRIMARY_WASTE_DIR/info/topdir.trashinfo

echo " == Should work with 'Y' or 'y'"
create_temp_files
$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/* && echo "y" | $RMW_TEST_CMD_STRING --empty

create_temp_files
$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/*
test -e $PRIMARY_WASTE_DIR/files/abc
test -e $PRIMARY_WASTE_DIR/files/123
test -e $PRIMARY_WASTE_DIR/files/xyz
echo "Y" | $RMW_TEST_CMD_STRING --empty

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

RMW_FAKE_YEAR=true $RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/*
cmp_substr "$(cat $PRIMARY_WASTE_DIR/info/abc.trashinfo)" \
  "DeletionDate=1999"

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
echo " == 1 f, this should pass"
$RMW_TEST_CMD_STRING -f --purge

echo
echo
echo " == Show that the files are really gone"

test ! -e $PRIMARY_WASTE_DIR/files/somefiles
test ! -e $PRIMARY_WASTE_DIR/info/somefiles.trashinfo

$RMW_TEST_CMD_STRING -o

echo " == Test 'show_purge_stats' == "
create_temp_files
RMW_FAKE_YEAR=True $RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/tmp-files/*
output=$($RMW_TEST_CMD_STRING -g)
echo $output
cmp_substr "$output" "Purging files based"
cmp_substr "$output" "3 items purged"

# filenames with parens
# THIS TEST DOESN'T WORK.

#cd $RMW_FAKE_HOME
#foo_file="foo (1)"
#touch "$foo_file"
#RMW_FAKE_YEAR=True $RMW_TEST_CMD_STRING -v "$foo_file"
#
# When rmw is invoked from here, the file gets
# purged, when invoked from the command line, rmw fails with error
# sh: -c: line 1: syntax error near unexpected token `('
#$RMW_TEST_CMD_STRING -v -g
#test ! -e "$PRIMARY_WASTE_DIR/files/$foo_file"
#
# But I've patched rmw so the error won't happen anymore.

exit 0
