#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 1999/03/12 K.Uehara
# $Id: makefile,v 1.61 2002/07/06 05:45:04 usr04098 Exp $
#

# ベースディレクトリ

BASEDIR = ../..

# オブジェクトリスト

MODULES = shadow hit splash wall_scar wave \
	bullet_bar \
	brk_obj brk_lgt brk_b_gls brk_gls brk_btl brk_pto \
	brk_rdl brk_tre brk_spt brk_hzd brk_pap brk_plt brk_pnl \
	brk_hng_lgt brk_mgz brk_tv brk_ice brk_rol brk_utl brk_sch \
	brk_fil brk_mdl brk_vnd brk_swg brk_build brk_mon brk_com \
	brk_speech \
	put_elv put_sph put_anm put_stanm put_book put_flag put_snd \
	vanm_bllr demo_bul demo_frmcnt \
	demo_orga demo_meryl demo_fort demo_emma demo_arkms \
	demo_control demo_init \
	orga fort emma vamp

# コマンド

.PHONY : all $(MODULES) lib link clean

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
