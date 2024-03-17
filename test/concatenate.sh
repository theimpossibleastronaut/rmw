#!/bin/sh

set -e

if [ -z "$BUILD_TEST_ROOT" ]; then
  echo "This script requires meson to be run"
  exit 1
fi

script="$BUILD_TEST_ROOT"/"$1".sh
cat script-head fragments/"$1".sh > "$script"
chmod +x "$script"
