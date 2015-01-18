#!/bin/bash
echo $#
if [ $# -ne 2 ]
then
    echo "Please pass proper arguments"
    exit -1
fi

cp -R grader_improved launchpad/$1
cd launchpad/$1

mkdir data
mv COPYING.txt data/COPYING.txt
mv icon.png data/icon.png
cp ../../default_desktop_file data/autog.desktop

export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

qmake
make clean

echo "Creating tar"
tar -czvf ../$1.tar.gz .

dh_make -c gpl -s -f ../$1.tar.gz

cd debian
rm *ex *EX README*
rm control
rm copyright
rm rules
cp ../../../default_control_file control
cp ../../../default_copyright_file copyright
cp ../../../default_rules_file rules
chmod +x rules
sed -i "1 s/unstable/$2/" "changelog" #1 only replaces the first line
cd ..
debuild -S
cd ..
NEW=`echo $1|tr "-" "_"`
echo $NEW;
dput autog-ppa $NEW"-1_source.changes" 
