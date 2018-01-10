#!/bin/sh
# purging.sh: tests rmw's purging facilities
#
# This script can be used to demonstrate the purging feature of rmw to
# new users, and for testing for bugs
#

# include VARS file
. ./VARS

test_result() {

set +x

if [ $1 != 0 ]; then
  echo "\n  --:Test failure:--"
  exit $1
fi

}

echo "== On first run, directories should get created"
# show commands that are run
set -x

$BIN_DIR/rmw -c $CONFIG

test_result $?

# Make some temporary files using a fake-year
mkdir tmp-home/tmp-files
cd tmp-home/tmp-files

set +x

echo "\n\n == creating temporary files to be rmw'ed"
set -x
for file in abc 123 xyz
do
  touch $file
done

set +x
cd $HOME/..
echo "\n\n == use a built-in environmental variable to write a"
echo " == fake year to the .trashinfo files when running rmw"
echo "-----------------------------------------------------\n"

set -x
RMWTRASH=fake-year $BIN_DIR/rmw --verbose -c $CONFIG $HOME/tmp-files/*
test_result $?
set +x

echo
echo
echo " == displaying a trashinfo; it shows a DeletionDate of 1999"

set -x
cat $HOME/.trash.rmw/info/abc.trashinfo
test_result $?

set +x
echo
echo
echo " == The trashinfo records that these files were rmw'ed in 1999"
echo " == So they will be purged now."

set -x
$BIN_DIR/rmw --verbose --force --purge -c $CONFIG
test_result $?

set +x
echo
echo
echo " == Show that the files are really gone"

set -x
ls -l $HOME/.trash.rmw/files
ls -l $HOME/.trash.rmw/info

set +x
echo
echo
echo " using 'cp' to copy files from test/somefiles"
cp -av somefiles $HOME

echo
echo
echo " == rmw should be able to purge directories and subdirectories"
echo " == even if some of the dirs are read-only"

set -x
RMWTRASH=fake-year $BIN_DIR/rmw --verbose -c $CONFIG $HOME/somefiles
test_result $?

$BIN_DIR/rmw --verbose -ff --purge -c $CONFIG
test_result $?

set +x
echo
echo
echo " == Show that the files are really gone"

set -x
ls -l $HOME/.trash.rmw/files
ls -l $HOME/.trash.rmw/info

set +x
echo
echo
echo " using 'cp' to copy files from test/somefiles for RMWTRASH=empty test"
cp -av somefiles $HOME
test_result $?

echo
echo
echo " == preceding RMWTRASH=empty with the rmw command should completely"
echo " == purge the waste folders, no matter when they were deleted"

set -x
$BIN_DIR/rmw --verbose -c $CONFIG $HOME/somefiles
test_result $?

RMWTRASH=empty $BIN_DIR/rmw --verbose -ff --purge -c $CONFIG
test_result $?

set +x
echo
echo
echo " == Show that the files are really gone"

set -x
ls -l $HOME/.trash.rmw/files
ls -l $HOME/.trash.rmw/info

set +x
echo "Purging tests passed"

exit 0
