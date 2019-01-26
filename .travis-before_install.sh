#!/bin/bash
# script for use with travis and on linux only
#
# Copyright (c) 2015-2016 MegaGlest Team under GNU GPL v3.0+
#
# Copyright (c) 2018
# modified for rmw <https://github.com/theimpossibleastronaut/rmw/wiki> by Andy Alt

Compiler_name="$1"; Compiler_version="$2"
Compiler_version_grep="$(echo "$Compiler_version" | sed 's/\./\\./g')"

set -x
if [ "$Compiler_version" != "" ] && [ "$Compiler_version" != "default" ]; then
  # UPDATE REPOS
  sudo apt-get update -qq
  sudo apt-get install -y -qq

  set +x
  if [ "$Compiler_name" = "gcc" ]; then
    VersionAvByDefault="$(apt-cache search ^g[c+][c+]-[0-9] | grep -v '[0-9]-[a-zA-Z]' | grep "^gcc-$Compiler_version_grep")"
  elif [ "$Compiler_name" = "clang" ]; then
    VersionAvByDefault="$(apt-cache search ^clang-[0-9] | grep -v '[0-9]-[a-zA-Z]' | grep "^clang-$Compiler_version_grep")"
  fi
  set -x
  if [ "$VersionAvByDefault" = "" ]; then
    # ubuntu test toolchain needed for more recent version of gcc and clang-6.0
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6B05F25D762E3157 &&
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  fi
fi

if [ "$VersionAvByDefault" = "" ]; then
  # UPDATE REPOS
  sudo apt-get update -qq
  sudo apt-get install -y -qq
fi
set -e

if [ "$Compiler_version" != "" ] && [ "$Compiler_version" != "default" ]; then
  if [ "$Compiler_name" = "gcc" ]; then
    set +ex
    Gcc_AvSepGpp="$(apt-cache search ^g[c+][c+]-[0-9] | grep -v '[0-9]-[a-zA-Z]' | grep "^g++-$Compiler_version_grep")"
    set -ex
    if [ "$Gcc_AvSepGpp" = "" ]; then
      sudo apt-get install -qq -y gcc-${Compiler_version}
    else
      sudo apt-get install -qq -y gcc-${Compiler_version} g++-${Compiler_version}
    fi
  elif [ "$Compiler_name" = "clang" ]; then
    # llvm-toolchain needed for more recent version of clang
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo -E apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-6.0 main"
    sudo -E apt-get -yq update &>> ~/apt-get-update.log
    sudo -E apt-get -yq --no-install-suggests \
        --no-install-recommends $TRAVIS_APT_OPTS install clang-${Compiler_version}
  fi
fi

# what available versions we can use
set +x
apt-cache search ^g[c+][c+]-[0-9] | grep -v '[0-9]-[a-zA-Z]'
apt-cache search ^clang-[0-9] | grep -v '[0-9]-[a-zA-Z]'
set -x
