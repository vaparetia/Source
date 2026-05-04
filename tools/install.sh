#!/bin/tcsh

#
# 園山ツールのインストールシェル 
#

cd libMDU ; make ; make install ; cd ..
cd dar ; make ; make install ; cd ..
cd etc ; make ; make install ; cd ..
cd id_change ; make ; make install ; cd ..
cd id_check ; make ; make install ; cd ..
cd makecm2 ; make ; make install ; cd ..
cd mdl2kms ; make ; make install ; cd ..
cd txm ; make ; make install ; cd ..
cd hzx_conv ; make ; make install ; cd ..
cd hzx_conv2 ; make ; make install ; cd ..

