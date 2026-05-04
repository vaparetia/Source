#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/01/20 T.Morita Revised
# $Id: makefile,v 1.8 2002/04/24 04:12:39 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_computer.c brk_com_w24d.c brk_com_w12b.c \
	efct_plasma.c efct_blink.c efct_flash.c efct_bomb.c efct_monitor.c
OBJS = $(SRCS:.c=.o)
XOBJECTS= $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
$(OBJS) : brk_computer.h
