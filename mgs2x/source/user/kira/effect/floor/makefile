#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.3 2002/02/13 04:35:42 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = floor.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = -D_DEBUG_

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


floor.o:	floor.c floor.h
