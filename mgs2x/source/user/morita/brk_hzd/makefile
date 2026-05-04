#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/01/20 T.Morita Revised
# $Id: makefile,v 2.6 2002/04/24 04:12:40 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_hazard.c brk_hzd_check.c brk_hzd_data.c brk_hzd_ini.c trg_view.c
OBJS = $(SRCS:.c=.o)
XOBJECTS= $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): brk_hazard.h
