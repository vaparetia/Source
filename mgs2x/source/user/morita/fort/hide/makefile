#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.17 2002/04/24 04:12:45 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  fort_obj.c fort_obj_ini.c fort_obj_act.c fort_obj_clb.c fort_obj_hzx.c \
	brk_woodbox.c brk_ironbox.c brk_anydust.c \
	brk_gastank.c brk_gas_cap.c \
	brk_container.c brk_slide.c \
	brk_forklift.c brk_fl_fwheel.c brk_fl_bwheel.c \
	brk_fl_frame.c brk_fl_lift.c  brk_fl_stick.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJS) : fort_obj.h ../../brk_hzd/brk_hazard.h

