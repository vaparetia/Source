#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.15 2002/04/24 04:12:44 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../../..

# ライブラリ名

LIBNAME = morita.a

# オブジェクトリスト
SRCS =  efct_shadow.c efct_drop.c efct_spark.c efct_sidedust.c efct_smoke.c \
	efct_heatiron.c efct_oil.c efct_flame.c efct_bdy_flame.c efct_elec.c \
	efct_flow.c efct_elv_btn.c efct_bullet.c efct_b_line.c \
	efct_oilstain.c \
	efct_falldust.c efct_wallscar.c \
	efct_vib.c

OBJS = $(SRCS:.c=.o)
XOBJECTS = $(OBJS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../include
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


