#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# 1999/08/17 K.Kano (Modified)
# $Id: makefile,v 1.38 2002/05/07 07:23:39 usr04098 Exp $
#

# ベースディレクトリ

BASEDIR = ..

# オブジェクトリスト

MODULES = particle ik rope waving_cloth corpse attachment vertex_animation objchange preview \
	malti_matrix us_soldier raincoat fhchg mw_hukuyure memtest hair memcard_checker nypd recmtn \
	npc_snake cergei_eri hostage mcman solmant handcuff titlescr resman late_control clearcode xmcman

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
