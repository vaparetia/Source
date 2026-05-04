#
# Makefile for MGS2 Sub Module
#	中間ディレクトリ／その下のディレクトリを探索する
#
# 2002/03/12 S.Yamashita
# $Id: makefile,v 1.1.1.3 2002/11/19 11:51:38 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# オブジェクトリスト
MODULES =	sy_util \
			test \
			outline \
			pitfall_flr \
			marker \
			vr_sky \
			vr_wall_scar \
			panel \
			2D \
			vr_spark 

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
