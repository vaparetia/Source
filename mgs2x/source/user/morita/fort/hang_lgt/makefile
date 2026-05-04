#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/01/20 T.Morita Revised
# $Id: makefile,v 1.8 2002/04/24 04:12:45 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = fort_hang.c fort_lgt.c fort_wall_lgt.c fort_wind.c fort_lgt_src.c
OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

