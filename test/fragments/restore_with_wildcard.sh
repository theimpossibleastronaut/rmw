create_some_files

$RMW_TEST_CMD_STRING ${RMW_FAKE_HOME}/somefiles/topdir -v
cd $PRIMARY_WASTE_DIR/files
$RMW_TEST_CMD_STRING -z topd*
test -e "${RMW_FAKE_HOME}/somefiles/topdir"
