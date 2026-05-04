#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/01/20 T.Morita Revised
# $Id: makefile,v 1.6 2002/08/22 07:08:38 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_monitor.c brk_mon_ini.c brk_mon_act.c brk_mon_clb.c efct_glass.c
OBJS = $(SRCS:.c=.o)
XOBJECTS= $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS) : brk_monitor.h
$(XOBJECTS) : brk_monitor.h
