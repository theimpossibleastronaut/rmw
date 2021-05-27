#!/bin/sh
set -ve

if [ -n "$COVERITY_SCAN_TOKEN" ]; then
  exit 0
fi

if test -e ./_build; then
  rm -rf ./_build;
fi

mkdir _build
cd _build

../configure ${CONFIGURE_OPTS}

# for 'rmw -s' in the restore test
export TERM=xterm

r=0
if [ -z "$DISTCHECK_ONLY" ]; then
  make V=1;
  make V=1 check -j5 || r=$?
  if test "x$r" != "x0"; then
    cat ./test/test-suite.log
  fi
else
  make V=1 distcheck || r=$?
  if test "x$r" != "x0"; then
    cat ./test/test-suite.log
  fi
fi
if test "x$r" = "x0"; then
  make distclean
else
  rm -rf ./test;
fi
