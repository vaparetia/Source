#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.8 2002/09/19 01:43:00 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト
ifneq ($(MGS2_XBOX),1)
OBJECTS = equip.o dogtag.o dogtag3.o
else
XOBJECTS = equip.obj dogtag.obj dogtag3.obj
endif


# ここでのローカルフラグリスト
LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
equip.o : equip.c equip.h eq_list.h
equip.obj : equip.c equip.h eq_list.h
dogtag.o : dogtag.c
dogtag3.o : dogtag3.c

