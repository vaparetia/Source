#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.9 2002/04/24 04:12:46 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  fort_lnr.c fort_amo.c fort_hzd.c fort_bul.c fort_bar.c fort_blast.c \
	fort_laser.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

fort_amo.o: fort_wep.h ../include/fort_se.h
fort_bar.o: fort_wep.h ../include/fort_se.h
fort_hzd.o: fort_wep.h ../include/fort_se.h
fort_m9.o : fort_wep.h ../include/fort_se.h
fort_sgr.o: fort_wep.h ../include/fort_se.h
fort_ssk.o: fort_wep.h ../include/fort_se.h
fort_usp.o: fort_wep.h ../include/fort_se.h
