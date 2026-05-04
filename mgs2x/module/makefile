#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.14 2002/05/13 02:18:48 usr01475 Exp $
#

# ベースディレクトリ

BASEDIR = ..

# オブジェクトリスト

MODULES =  jsifman usbfs usbkbd cdbios sound mts zlibdec loader inetfs usbhmd

# コマンド

.PHONY : all $(MODULES) clean inc_se

all : $(MODULES) inc_se

$(MODULES) : 
	$(MAKE) -C $@

clean :
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)
	rm -f include/*.h irx/*.irx lib/*.a

data :
	cp -r /u/develop/mj001b/exports/sound ../cdrom.img/.

inc_se :
	cp -f inc_se/se_defin.h include/se_defin.h

.DEFAULT:
	make -C .. $@
