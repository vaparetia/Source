#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.2 2002/11/04 11:43:42 takaki Exp $
#

# ベースディレクトリ

BASEDIR = ..

# オブジェクトリスト

#MODULES =  korekado sigeno sonoyama takabe okajima morita kano uehara nobu shibata kira mode satoyoshi okuta skoba kunibe yano yamashita
MODULES =  sigeno sonoyama takabe okajima morita kano uehara nobu shibata kira mode satoyoshi okuta skoba kunibe yano yamashita korekado

# コマンド

.PHONY : all lib $(MODULES) link clean

all : $(MODULES) link
lib : $(MODULES)

$(MODULES) : 
	@$(MAKE) lib -C $@

link : 
	cd $(BASEDIR); make link

clean :
	-(for i in $(MODULES); do $(MAKE) clean -C $$i; done)

.DEFAULT:
	cd $(BASEDIR); make $@
