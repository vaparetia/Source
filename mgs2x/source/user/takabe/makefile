#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:08 Yoshizawa1 Exp $
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

MODULES =  test effect1 object other pdray

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
