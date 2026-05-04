#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/08/30 K.Uehara
# $Id: 
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

MODULES = fatman conv prez kamome parrot dbgutil effect

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
