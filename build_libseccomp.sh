#!/usr/bin/env bash

VERSION=v2.5.4
if [ -d ./libseccomp ]; then exit 0; fi

set -x

git clone https://github.com/seccomp/libseccomp.git >/dev/null 2>&1 || exit 1
cd libseccomp || exit 1
git checkout $VERSION >/dev/null 2>&1
./autogen.sh >/dev/null 2>&1 || exit 1
./configure --enable-shared=no >/dev/null 2>&1 || exit 1
make -j >/dev/null 2>&1 || exit 1
