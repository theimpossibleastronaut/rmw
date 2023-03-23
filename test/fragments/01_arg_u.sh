create_some_files
$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/somefiles/*
output=$($RMW_TEST_CMD_STRING -uvv | grep Waste)
echo "OUTPUT:"
echo "---"
echo "$output"
echo "---"
test "$output" = "+'${RMW_FAKE_HOME}/.Waste/files/read_only_file' -> '${RMW_FAKE_HOME}/somefiles/read_only_file'
-${RMW_FAKE_HOME}/.Waste/info/read_only_file.trashinfo
+'${RMW_FAKE_HOME}/.Waste/files/topdir' -> '${RMW_FAKE_HOME}/somefiles/topdir'
-${RMW_FAKE_HOME}/.Waste/info/topdir.trashinfo"
