#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/11/09 K.Kano (Modified)
# $Id: makefile,v 1.25 2002/08/13 09:44:10 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = titlescr.o selectscr.o gamesel.o newgame.o option.o loadgame.o bgscr.o portsel.o \
		konami_logo.o kcej_logo.o savegame.o mcchkscr.o portsel_for_photosave.o special.o frame.o \
		bossrush.o theater.o language.o util.o tr_option_frame.o

XOBJECTS= titlescr.obj selectscr.obj gamesel.obj newgame.obj option_x.obj loadgame.obj bgscr.obj portsel.obj \
		konami_logo.obj kcej_logo.obj savegame.obj mcchkscrx.obj portsel_for_photosave.obj special.obj frame.obj \
		bossrush_x.obj theater.obj language.obj util.obj tr_option_frame.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : titlescr.h
util.o	: ../../yamashita/2D/msn.h ../../../../scn/msn_stage_id2.h
mcchkscrx.obj : ../xmcman/xmcman.h
option_x.obj : titlescr_x.h
