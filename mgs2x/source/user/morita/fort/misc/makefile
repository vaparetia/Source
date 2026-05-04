#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.14 2002/04/24 04:12:45 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  fort_com.c fort_jnt.c \
	fort_ceil.c fort_ceil_dust.c fort_motion.c \
	fort_dbg.c fort_dbgcnf.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

fort_com.o : ../include/fort_flag.h  ../include/fort.h
fort_dbg.o : ../include/fort_flag.h  ../include/fort.h

