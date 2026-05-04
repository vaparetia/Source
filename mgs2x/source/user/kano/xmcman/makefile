#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/08/17 K.Kano (Modified)
# $Id: makefile,v 1.8 2002/08/12 06:24:42 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
XOBJECTS= xmcman.obj xdevselect.obj xfilechk.obj xfilesel.obj xsaveload.obj xstrings.obj xmenustr.obj xl2dmorf.obj\
			slthread.obj xfileselp.obj xfileselg.obj xsavegame.obj xloadgame.obj xloaddog.obj xloadalbum.obj \
			xfileselp_load.obj xphotoedit.obj xsavest.obj xexistcheck.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES =
LOCAL_OPTIONS = /O1
//LOCAL_OPTIONS = /Od

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(XOBJECTS) : ../mcman/mcman.h xmcman.h
