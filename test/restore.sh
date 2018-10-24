#!/bin/sh
# restore.sh: tests rmw's Restore features
#
# This script can be used to demonstrate the restore features of rmw to
# new users, and to test for bugs when making code changes
#

. ${1}
. ${TESTS_DIR}/COMMON

echo $SEPARATOR
echo "Initialize"
$RMW_TEST_CMD_STRING
test_result $?

echo $SEPARATOR
echo "Copying pre-existing files to temporary HOME dir..."
cp -a ${TESTS_DIR}/somefiles $HOME
test_result $?

echo $SEPARATOR
echo "ReMove files and then restore them by using -u"
set -x
$RMW_TEST_CMD_STRING $HOME/somefiles/*
$RMW_TEST_CMD_STRING -u
test_result $?

echo $SEPARATOR
echo "Show result when no undo file exists..."
$RMW_TEST_CMD_STRING -u
test_result $?

echo $SEPARATOR
echo "Restore by using the basename, from anywhere"
$RMW_TEST_CMD_STRING $HOME/somefiles/topdir
$RMW_TEST_CMD_STRING -z topdir

echo $SEPARATOR
echo "Try restoring with -z using a wildcard (will fail)"
$RMW_TEST_CMD_STRING $HOME/somefiles/topdir
$RMW_TEST_CMD_STRING -z topd*
test_result $?

echo $SEPARATOR
echo "Restore using wildcard pattern, but be in the trash directory"
set -x
cd $HOME/.trash.rmw/files
$RMW_TEST_CMD_STRING -z topd*
test_result $?

echo $SEPARATOR
echo "Try restoring a file that doesn't exist"
$RMW_TEST_CMD_STRING -z nonexistent_fil*
test_result $?

echo $SEPARATOR
echo "What happens when trying to ReMove file inside dir with no write permissions..."
set -x
mkdir $HOME/no_write_perms
touch $HOME/no_write_perms/test.tmp
chmod -R -w $HOME/no_write_perms
$RMW_TEST_CMD_STRING $HOME/no_write_perms

# can't test_result here because it should return 1, and we want this script
# to continue. Instead, just show what's returned.
echo error $? returned;
set +x

# change permissions back to writable so the dir gets removed before the
# next test (done in COMMON)
chmod +w -R $HOME/no_write_perms

echo $SEPARATOR
echo "Restore tests passed"

exit 0
