#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/08/30 K.Uehara
# $Id: makefile,v 1.12 2002/02/22 08:16:52 usr04098 Exp $
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

ifneq ($(MGS2_XBOX),1)
MODULES =  codec demo menu node phototerm mobile
else
MODULES =  codec demo menu node phototerm mobile
endif

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
