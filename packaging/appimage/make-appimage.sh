#!/bin/sh

if [ -z "$UID" ]; then
  echo "Could not detect UID."
  exit 1
fi

if [ ! -e "src/main.c" ];then
  echo "You need to be in the rmw source root directory when you run this script."
  exit 1
fi

export WORKSPACE="/rmw-src-root"
export VERSION=${VERSION:-"0.8.1"}

echo "Version is set to '$VERSION'"
echo "use 'VERSION=<version> $0' to change it."
echo "Waiting 10 seconds to start, hit CTRL-C now to cancel..."

read -t 10

set -ev

docker run -it --rm \
  -e VERSION=$VERSION  \
  -e ARCH=x86_64 \
  -e WORKSPACE \
  -e HOSTUID=$UID \
  -v $PWD:$WORKSPACE \
  andy5995/rmw-build-env:bionic \
    /bin/bash -c 'usermod -u $HOSTUID rmwbuilder \
    && su rmwbuilder --command "$WORKSPACE/packaging/appimage/workflow.sh"'
