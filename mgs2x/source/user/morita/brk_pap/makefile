#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2000/04/25 T.Morita Revised
# $Id: makefile,v 1.6 2002/04/24 04:12:41 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS = brk_paper.c \
	brk_pap_act.c brk_pap_ini.c brk_pap_clb.c \
	brk_pap_msg.c 
OBJS = $(SRCS:.c=.o)
XOBJECTS= $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS): brk_paper.h ../brk_hzd/brk_hazard.h

