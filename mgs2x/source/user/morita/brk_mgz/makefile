#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/06/19 T.Morita Revised
# $Id: makefile,v 1.7 2002/04/24 04:12:41 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_magazine.c \
	brk_mgz_act.c brk_mgz_ini.c brk_mgz_clb.c \
	brk_mgz_msg.c
OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): brk_magazine.h ../brk_hzd/brk_hazard.h

