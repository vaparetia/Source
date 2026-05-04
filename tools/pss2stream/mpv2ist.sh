#! /bin/tcsh -f

set ipu = `echo $1 | sed -e "s/\.mpv/.ipu/g"`

set frame = `ps2str i $1 | grep "frame rate" | awk '{ print $4 }'`

ps2str convert -o -video $1 $ipu
if ( $status ) then
   exit 1
endif

ipu2stream $ipu $2 $frame
rm -f $ipu


