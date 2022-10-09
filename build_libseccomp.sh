VERSION=v2.5.4
if [ -d ./libseccomp ]; then exit 0; fi

set -x

git clone https://github.com/seccomp/libseccomp.git &>/dev/null
cd libseccomp || exit 1
git checkout $VERSION &>/dev/null
./autogen.sh &>/dev/null || exit 1
./configure --enable-shared=no &>/dev/null || exit 1
make -j &>/dev/null || exit 1
