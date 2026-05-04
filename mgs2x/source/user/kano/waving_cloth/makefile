#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/08/17 K.Kano (Modified)
# $Id: makefile,v 1.6 2002/02/04 07:55:18 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = waving_cloth.o waving_clothsub.o jacket.o
XOBJECTS= $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

waving_cloth.o : waving_cloth.c waving_cloth.h
waving_clothsub.o : waving_clothsub.c nest.c waving_cloth.h