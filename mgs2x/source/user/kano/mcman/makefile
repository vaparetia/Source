#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/08/17 K.Kano (Modified)
# $Id: makefile,v 1.12 2002/07/25 05:43:38 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = mcman.o mcman_step.o mcscr.o mcdata.o mcman_sjis.o
XOBJECTS= $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

mcman_sjis.o : mcman_sjis.c mcman.sjis
$(OBJECT) : mcman.h mcman_sjis.h mcmanh.sjis ../../yamashita/2D/msn.h

