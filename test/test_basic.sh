#!/bin/sh
set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

echo "== On first run, directories should get created"
$VALGRIND $RMW_TEST_CMD_STRING

echo $SEPARATOR
echo "List the waste folders..."
echo "rmw should display folders on removable devices that are not mounted"
echo $SEPARATOR
output="$($RMW_TEST_CMD_STRING -l)"
echo "${output}"
test "${output}" = "/mnt/fs/Trash-$(id -u)
/mnt/sda10000/example_waste
"${MESON_BUILD_ROOT}"/test/rmw-tests-home/test_basic.sh_dir/.Waste
"${MESON_BUILD_ROOT}"/test/rmw-tests-home/test_basic.sh_dir/.local/share/Waste-2
"${MESON_BUILD_ROOT}"/test/rmw-tests-home/test_basic.sh_dir/.local/share/Waste-3"

echo $SEPARATOR

# Make some temporary files
mkdir ${RMW_FAKE_HOME}/tmp-files && cd ${RMW_FAKE_HOME}/tmp-files

echo "\n\n == creating temporary files to be deleted"
for file in 1 2 3; do
  touch $file
done
cd ${RMW_FAKE_HOME}/..

echo "\n\n == rmw should be able to operate on multiple files\n"
$RMW_TEST_CMD_STRING --verbose ${RMW_FAKE_HOME}/tmp-files/*

test -f "${PRIMARY_WASTE_DIR}/files/1"
test -f "${PRIMARY_WASTE_DIR}/files/2"
test -f "${PRIMARY_WASTE_DIR}/files/3"
test -f "${PRIMARY_WASTE_DIR}/info/1.trashinfo"
test -f "${PRIMARY_WASTE_DIR}/info/2.trashinfo"
test -f "${PRIMARY_WASTE_DIR}/info/3.trashinfo"

echo $SEPARATOR
echo "rmw should append a time_string to duplicate files..."
cd ${RMW_FAKE_HOME}/tmp-files
for file in 1 2 3; do
  touch $file
done
$RMW_TEST_CMD_STRING 1 2 3

echo "\n\n == Show contents of the files and info directories"

test -n "$(ls -A $PRIMARY_WASTE_DIR/files)"
test -n "$(ls -A $PRIMARY_WASTE_DIR/info)"

output="$(ls -A $PRIMARY_WASTE_DIR/files | wc -l | sed 's/ //g')"
test "$output" = "6"

echo $SEPARATOR
echo "  == rmw should refuse to move a waste folder or a file that resides within a waste folder"
output="$($RMW_TEST_CMD_STRING $PRIMARY_WASTE_DIR/info)"
test "${output}" = " :warning: $PRIMARY_WASTE_DIR/info resides within a waste folder and has been ignored
0 items were removed to the waste folder"

# If the file gets removed (which it shouldn't), then the test that follows it will fail
$RMW_TEST_CMD_STRING $PRIMARY_WASTE_DIR/info/1.trashinfo

echo "\n\n == Display contents of 1.trashinfo "
cat $PRIMARY_WASTE_DIR/info/1.trashinfo

echo $SEPARATOR
echo "  == Test -m option"
output=$($RMW_TEST_CMD_STRING --verbose -m)
substring=".Waste/files/1_"
test "${output}#*${substring}" != "${output}"
substring=".Waste/files/2_"
test "${output}#*${substring}" != "${output}"
substring=".Waste/files/3_"
test "${output}#*${substring}" != "${output}"

echo $SEPARATOR
echo "  == test undo/restore feature"

$RMW_TEST_CMD_STRING --verbose -u
$RMW_TEST_CMD_STRING --verbose -z $PRIMARY_WASTE_DIR/files/1
$RMW_TEST_CMD_STRING --verbose -z $PRIMARY_WASTE_DIR/files/2
$RMW_TEST_CMD_STRING --verbose -z $PRIMARY_WASTE_DIR/files/3

echo "\n\n == test that the files are restored to their previous locations"

test -f ${RMW_FAKE_HOME}/tmp-files/1
test -f ${RMW_FAKE_HOME}/tmp-files/2
test -f ${RMW_FAKE_HOME}/tmp-files/3

echo "\n\n == test that the .trashinfo files have been removed"

test ! -f $PRIMARY_WASTE_DIR/info/1.trashinfo
test ! -f $PRIMARY_WASTE_DIR/info/2.trashinfo
test ! -f $PRIMARY_WASTE_DIR/info/3.trashinfo

# Ignore dot directories
mkdir tmpdot
touch "tmpdot/.hello world"
touch "tmpdot/.hello earth"

output="$($RMW_TEST_CMD_STRING tmpdot/.*)"
test "${output}" = "refusing to ReMove '.' or '..' directory: skipping 'tmpdot/.'
refusing to ReMove '.' or '..' directory: skipping 'tmpdot/..'
2 items were removed to the waste folder"

expected="0 orphans found"
output="$($RMW_TEST_CMD_STRING -o)"
test "${output}" = "${expected}"

substring="invalid option"
output="$($RMW_ALT_TEST_CMD_STRING -l)"
test "${output}#*${substring}" != "${output}"

echo "Basic tests passed"
exit 0
