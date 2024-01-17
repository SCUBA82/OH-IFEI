#!/bin/bash
zip=`which zip`
echo "Which version would you like to release? (i.e 0.3.1) "
read version
folder="./Releases/dcs-bios-arduino-library-"$version
if [ -d $folder ]; then 
	rm -rf $folder
fi
mkdir -p $folder 
cp -r ./examples $folder
cp -r ./src $folder
cp  ./keywords.txt $folder
cp  ./library.properties $folder
cp  ./LICENSE $folder
cp  ./*.md $folder
cd $folder 
zip dcs-bios-arduino-library-$version.zip * 
mv dcs-bios-arduino-library-$version.zip ../
cd ../..
rm -rf $folder
