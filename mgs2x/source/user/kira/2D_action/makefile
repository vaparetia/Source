#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.6 2002/02/07 04:45:52 usr02774 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = layout_2d.o layout_conv.o layout_data.o l_util.o test.o 2d_preview.o
XOBJECTS = layout_2d.obj layout_conv.obj layout_data.obj l_util.obj test.obj 2d_preview.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


layout_2d.o:	layout_2d.c layout_2d.h layout_config.h layout_data.h 
layout_conv.o:	layout_conv.c layout_conv.h layout_data.h layout_config.h
layout_data.o:	layout_data.c layout_malloc.h layout_config.h layout_data.h layout_conv.h l_util.h
l_util.o:	l_util.c l_util.h
2d_preview.o:	2d_preview.c 2d_preview.h layout_2d.h
