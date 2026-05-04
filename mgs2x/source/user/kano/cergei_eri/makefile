#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/09/21 K.Kano (Modified)
# $Id: makefile,v 1.3 2002/02/04 05:46:44 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = cergei_eri.o cergei_eri_act.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

cergei_eri.o : cergei_eri.c cergei_eri.h
cergei_eri_act.o : cergei_eri_act.c cergei_eri.h
