#!/bin/sh

set -ev

EXT="${1##*.}"
echo ": $EXT"

if test "$EXT" = "sh"; then
  export VALGRIND="valgrind --error-exitcode=1 --leak-check=full"
  $1
else
  valgrind --error-exitcode=1 --leak-check=full "$1"
fi
