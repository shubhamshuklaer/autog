#!/bin/bash

# ./make_package.sh autog-VERSION UBUNTU_VERSION ARCHITECTURE
# ./make_package.sh autog-0.9.27 focal amd64

set -x
set -e
echo $#
if [ $# -ne 3 ]
then
    echo "Please pass proper arguments"
    exit -1
fi

rm -rf build
mkdir build
cp -R ../src build/$1
cd build/$1

mkdir qtlibs
if [ $3 == "i386" ]
then
    cp -R ../../qtlibs/i386/* qtlibs
elif [ $3 == "amd64" ]
then
    cp -R ../../qtlibs/amd64/* qtlibs
fi

export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

tee -a autog.pro << 'EOF'

# Install the QT libs under the application folder and make autog use that.
# This helps with not having to install QT in the host.
# Note that the compile is still done using the host libs.. this is for runtime only.

QMAKE_RPATHDIR += $$PREFIX/lib/autog
QMAKE_RPATHDIR += $$PREFIX/lib/autog/platforms

libfiles.path=$$PREFIX/lib/autog
libfiles.files +=qtlibs/libQt5*
INSTALLS +=libfiles

platform_plugins.path=$$PREFIX/lib/autog/platforms
platform_plugins.files += qtlibs/platforms/*
INSTALLS +=platform_plugins

EOF

qmake
make all
make clean

echo "Creating tar"
tar -czvf ../$1.tar.gz .

DEBEMAIL="shubhamshuklaerssss@gmail.com" DEBFULLNAME="Shubham Shukla" dh_make -c gpl -s -f ../$1.tar.gz

cd debian
rm *ex *EX README* control copyright rules compat || true
cp ../../../control ../../../copyright ../../../rules ../../../compat .
chmod +x rules
sed -i "1 s/unstable/$2/" "changelog" #1 only replaces the first line
sed -i "s/^Architecture.*/Architecture: $3/" "control"
cd ..

debuild -S
