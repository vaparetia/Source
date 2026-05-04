#!/bin/sh

SearchPath=$1
IpuConvertCmd=`pwd`/ipu2mpeg.elf
echo $IpuConvertCmd

echo "Searching for ipu movie files in" $SearchPath

IpuFiles=`find $SearchPath -name *.ipu -print`
for file in $IpuFiles
do

   BaseFileName=`basename $file .ipu`
   OutputDir="out/"$BaseFileName
   echo "Extracting frames to" $OutputDir

   mkdir -p $OutputDir
   pushd . > /dev/null
   cd $OutputDir
   
   #dsedb -r run $IpuConvertCmd $file
   #ffmpeg -i frame%d.tga -vcodec rawvideo -pix_fmt bgr24 $BaseFileName.avi
   TargetDir=$SearchPath/$BaseFileName/$BaseFileName.avi

   cp -f $BaseFileName.avi $TargetDir

   popd > /dev/null
   
done
