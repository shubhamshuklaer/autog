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
rm -rf build/$1/build
cd build/$1

qmake
make all
make distclean

echo "Creating tar"
tar -czvf ../$1.tar.gz .

if [ -z "$DEBEMAIL" ] || [ -z "$DEBFULLNAME" ]
then
	echo "DEBMAIL or DEBFULLNAME is empty using defaults"
	DEBEMAIL="shubhamshuklaerssss@gmail.com" DEBFULLNAME="Shubham Shukla" dh_make -c gpl -s -f ../$1.tar.gz
else
	dh_make -c gpl -s -f ../$1.tar.gz
fi

cd debian
rm *ex *EX README* control copyright rules compat || true
cp ../../../control ../../../copyright ../../../rules ../../../compat .
chmod +x rules
sed -i "1 s/unstable/$2/" "changelog" #1 only replaces the first line
sed -i "s/^Architecture.*/Architecture: $3/" "control"

cd ..

debuild -S
