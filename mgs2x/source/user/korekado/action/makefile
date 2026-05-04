#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.5 2002/02/01 08:50:21 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = action.o

XOBJECTS = action.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

action.o : action.c action.h
action.obj : action.c action.h
$(OBJECTS) : ../enemy/enemy.h

