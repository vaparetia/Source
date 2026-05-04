#!/bin/tcsh

cd ../source
#make cd
#ee-strip ../cdrom.img/mgs2.elf -o ../disc/image/SLPM_670.02

setenv MGS2_DISC 1
cd ../scn
rm *.cdo *.gco
make

cd ../cdrom.img
mkcdimg -l ../list/stagegm.lst -s stage -o stagegm.dat -z

setenv MGS2_DISC 2
cd ../scn
rm *.gco
make

cd ../cdrom.img
mkcdimg -l ../list/stagevr.lst -s stage -o stagevr.dat -z

