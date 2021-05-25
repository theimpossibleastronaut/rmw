#!/bin/sh
set -v

if [ -n "$COVERITY_SCAN_TOKEN" ]; then
  exit 0
fi

mkdir _build
cd _build
../configure ${CONFIGURE_OPTS}

# for 'rmw -s' in the restore test
export TERM=xterm

if [ -z "$DISTCHECK_ONLY" ]; then
  make V=1 || exit $?
  make V=1 check -j5
  r=$?
  if test "x$r" != x0; then
    cat ./test/test-suite.log
    exit $r
  fi
else
  make V=1 distcheck
  r=$?
  if test "x$r" != x0; then
    cat ./test/test-suite.log
    exit $r
  fi
  make distclean || exit $?
fi
