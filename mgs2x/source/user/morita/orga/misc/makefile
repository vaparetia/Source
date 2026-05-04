#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.17 2002/04/24 04:12:47 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  orga_hid.c orga_har.c orga_lgt.c orga_hzx.c \
	orga_rai.c orga_com.c orga_obj.c orga_lnz.c \
	orga_box.c orga_sph.c orga_rpl.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

orga_box.o : ../include/orga_se.h
orga_com.o : ../include/orga.h
orga_hid.o : ../include/orga.h
orga_hol2.o: ../include/orga_se.h
orga_hol.o : ../include/orga_se.h
orga_lgt.o : ../include/orga_se.h
orga_obj.c : ../include/orga.h

