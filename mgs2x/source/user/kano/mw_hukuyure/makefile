#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/09/21 K.Kano (Modified)
# $Id: makefile,v 1.4 2002/02/04 07:55:17 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = mw_hukuyure.o mw_hukuyure_act.o
XOBJECTS= $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

mw_hukuyure.o : mw_hukuyure.c mw_hukuyure.h
mw_hukuyure_act.o : mw_hukuyure_act.c mw_hukuyure.h