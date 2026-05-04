#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.4 2002/02/26 01:21:14 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = face.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = -D_DEBUG_

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


face.o:		face.c face.h
