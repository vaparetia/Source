#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.18 2002/02/14 10:30:54 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

OBJECTS =	gm_over.o title_trial.o option_trial.o tobcon.o ctrl_exp.o \
			rankenc.o clear.o sp_menu.o sp_menu_eng.o lowpoly_ag.o \
			sp_menu_page.o

XOBJECTS =	gm_over.obj title_trial.obj option_trial.obj tobcon.obj ctrl_exp.obj \
			rankenc.obj clear.obj sp_menu.obj sp_menu_eng.obj lowpoly_ag.obj \
			sp_menu_page.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
gm_over.o : ../util/dma_set.h ../util/dma_set.o
tobcon.o : ../util/dma_set.h ../util/dma_set.o

gm_over.obj : ../util/dma_set.h ../util/dma_set.obj
tobcon.obj : ../util/dma_set.h ../util/dma_set.obj
