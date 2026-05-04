#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/02/10 T.Morita Revised
# $Id: makefile,v 1.12 2002/05/30 04:09:32 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_bottle.c \
       brk_btl_act.c brk_btl_ini.c brk_btl_clb.c brk_btl_hzd.c brk_btl_msg.c \
       brk_wine.c brk_winefall.c
OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): brk_bottle.h ../brk_hzd/brk_hazard.h
$(XOBJECTS): brk_bottle.h ../brk_hzd/brk_hazard.h

