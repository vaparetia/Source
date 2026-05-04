#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 2000/05/17 T.Shibata
# $Id: makefile,v 1.15 2002/02/07 07:16:19 usr01749 Exp $
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

#MODULES = 	radio_break util headmark demo etc effect\
#			t_headmark t_flour anime \
#			sea_slater test scr_brk 

MODULES = 	radio_break util headmark demo etc effect \
			anime t_headmark t_flour sea_slater vamp


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
