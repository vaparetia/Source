#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#	ユーティリティ関連
# 2001/04/23 Masafumi Okuta
# $Id: makefile,v 1.6 2002/02/07 02:27:40 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okuta.a

# オブジェクトリスト
OBJECTS = algfunc.o maoutil.o memmot.o

XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
algfunc.o : algfunc.c algfunc.h

maoutii.o : maoutil.c maoutil.h

memmot.o : memmot.c memmot.h maoutil.h

