#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.6 2002/02/13 04:35:42 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = cylinder.o blink.o spin_model.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


cylinder.o:	cylinder.c cylinder.h blink.h
blink.o:	blink.c blink.h cylinder.h
spin_model.o:	spin_model.c spin_model.h cylinder.h
