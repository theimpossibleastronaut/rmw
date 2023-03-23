create_some_files

echo "Try to restore files that aren't in a Waste/files folder"
cmp_substr "$($RMW_TEST_CMD_STRING -z ${RMW_FAKE_HOME}/somefiles/* 2>&1 && exit 1)" \
  "not in a Waste directory"
