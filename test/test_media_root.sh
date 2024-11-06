#!/bin/sh
# If a waste folder is in $topdir of a partition or device, the
# Path should be relative
#
#https://specifications.freedesktop.org/trash-spec/1.0/#id-1.6.10.1 The key
#“Path” contains the original location of the file/directory, as either an
#absolute pathname (starting with the slash character “/”) or a relative
#pathname (starting with any other character). A relative pathname is to be
#from the directory in which the trash directory resides (for example, from
#$XDG_DATA_HOME for the “home trash” directory); it MUST not include a “..”
#directory, and for files not “under” that directory, absolute pathnames must
#be used. The system SHOULD support absolute pathnames only in the “home trash”
#directory, not in the directories under $topdir.

set -ve

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# This test will only work on Andy's workstation.
# The media root, /home/andy/src is on a different partition than /home/andy
# It's mounted with 'bind' and therefore has a different device id
#
# /dev/sda7 on /mnt/sda7 type ext4 (rw,relatime)
# /dev/sda7 on /home/andy/src type ext4 (rw,relatime)

if test ! -d /mnt/918375c2; then
  exit 0
fi

test_file="media_root_test"
PREV_RMW_FAKE_HOME=${RMW_FAKE_HOME}
# needs to be unset so rmw will use $HOME instead
unset RMW_FAKE_HOME
mkdir -p "$PREV_RMW_FAKE_HOME"
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

output=$(grep Path /home/andy/src/rmw-project/.Trash-1000/info/$test_file.trashinfo)

# There should be no leading '/' in the filename.
path_expected=$(echo ${MESON_BUILD_ROOT} | sed -e "s/\/home\/andy\/src\/rmw-project\///g")
echo $path_expected
test "$output" = "Path=${path_expected}/test/rmw-tests-home/test_media_root.sh_dir/media_root_test"

output=$($BIN_DIR/rmw -uvv -c /home/andy/.config/rmwrc | grep media_root_test)

test "$output" = "+'/home/andy/src/rmw-project/.Trash-1000/files/media_root_test' -> '${MESON_BUILD_ROOT}/test/rmw-tests-home/test_media_root.sh_dir/media_root_test'
-/home/andy/src/rmw-project/.Trash-1000/info/media_root_test.trashinfo"

exit 0
