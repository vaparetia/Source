#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.14 2002/04/24 04:12:47 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  orga_usp.c orga_ssk.c orga_sgr.c orga_amo.c \
	orga_hzd.c orga_bul.c orga_bar.c orga_bul2.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

orga_amo.o: orga_wep.h ../include/orga_se.h
orga_bar.o: orga_wep.h ../include/orga_se.h
orga_hzd.o: orga_wep.h ../include/orga_se.h
orga_m9.o : orga_wep.h ../include/orga_se.h
orga_sgr.o: orga_wep.h ../include/orga_se.h
orga_ssk.o: orga_wep.h ../include/orga_se.h
orga_usp.o: orga_wep.h ../include/orga_se.h
