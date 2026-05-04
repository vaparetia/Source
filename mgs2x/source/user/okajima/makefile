#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.22 2002/02/01 04:43:47 usr01475 Exp $
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

MODULES = t_fig_raven t_effect t_tyoudan t_prev t_util t_debug t_irs t_flashlight demo_effect effect effect2 effect3 etc 


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
