#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/04/25 T.Morita Revised
# $Id: makefile,v 1.15 2002/04/24 04:12:42 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_utl_lgt.c brk_utl_ini.c brk_utl_act.c brk_utl_aim.c \
       brk_utl_mtx.c brk_utl_obj.c brk_utl_clb.c \
       brk_utl_glass.c brk_utl_dbg.c \
       wireframe.c linepursuit.c playerwire.c circle.c
OBJS = $(SRCS:.c=.o)
XOBJECTS= $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


