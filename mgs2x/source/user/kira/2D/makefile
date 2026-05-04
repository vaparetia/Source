#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.7 2002/02/21 11:34:50 usr02774 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = sprite_2d.o sprite_dma.o test_2d.o
XOBJECTS = sprite_2d.obj xsprite_dma.obj test_2d.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

sprite_2d.o:	sprite_2d.c sprite_dma.h
sprite_dma.o:	sprite_dma.c sprite_dma.h sprite_dma_defaults.c
