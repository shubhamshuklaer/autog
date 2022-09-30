#!/bin/bash

# ./make_package.sh autog-VERSION UBUNTU_VERSION ARCHITECTURE
# ./make_package.sh autog-0.9.27 focal amd64

set -x
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

cp ../../default_desktop_file data/autog.desktop

if [ $3 == "i386" ]
then
    cp -R data/i386/* data/
elif [ $3 == "amd64" ]
then
    cp -R data/amd64/* data/
fi

rm -rf data/amd64 data/i386

export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

qmake
make all
make clean

echo "Creating tar"
tar -czvf ../$1.tar.gz .

DEBEMAIL="shubhamshuklaerssss@gmail.com" DEBFULLNAME="Shubham Shukla" dh_make -c gpl -s -f ../$1.tar.gz

cd debian
rm *ex *EX README*
rm control
rm copyright
rm rules
rm compat
cp ../../../default_control_file control
cp ../../../default_copyright_file copyright
cp ../../../default_rules_file rules
cp ../../../default_compat compat
chmod +x rules
sed -i "1 s/unstable/$2/" "changelog" #1 only replaces the first line
sed -i "s/^Architecture.*/Architecture: $3/" "control"
cd ..

debuild -S