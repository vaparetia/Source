#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.6 2002/04/24 04:12:48 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =	vamp.c vamp_ini.c vamp_act.c vamp_thk.c \
	vamp_msg.c vamp_rcg.c vamp_dsp.c \
	vamp_utl.c 
OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
vamp_thk.o : $(wildcard think/*.h)
vamp_act.o : $(wildcard action/*.h)
$(OBJS): include/vamp.h  include/w32a_vamp.mh  ../../korekado/npc/npc.h

