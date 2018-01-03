#!/bin/sh

. ./VARS

# show commands that are run
set -x

../src/rmw -c $CONFIG -l

err=$?

set +x

if [ $err != 0 ]; then
  echo "Test failure"
  exit $err
fi

exit 0
