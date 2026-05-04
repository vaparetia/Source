#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.5 2002/02/01 04:43:17 usr01475 Exp $
#

# ベースディレクトリ

BASEDIR = ..

# オブジェクトリスト

MODULES =  libdg libfs libgv libhzx libmt libgcl libutl

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
