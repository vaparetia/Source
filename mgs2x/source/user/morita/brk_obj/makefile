#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/01/21 T.Morita Revised
# $Id: makefile,v 1.12 2002/04/24 04:12:41 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_object.c brk_obj_ini.c brk_obj_act.c brk_obj_hzd.c \
	brk_obj_msg.c brk_obj_clb.c efct_salt.c efct_kchp.c
OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): brk_object.h ../brk_hzd/brk_hazard.h

