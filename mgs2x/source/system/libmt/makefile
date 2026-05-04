#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.15 2002/02/14 06:51:11 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../..

# ライブラリ名

LIBNAME = libmt.a

# オブジェクトリスト
OBJECTS = quat.o quat2.o quat3.o math_utl.o mt3_util.o motion.o mt_effct.o mar_load.o \
	actseq.o se_exchg.o hermite.o mtdec_vu.o faceanim.o facepack.o ik_sys.o
NO_USE_OBJECTS = mt3_dec.o 

XOBJECTS = actseq.obj hermite.obj mar_load.obj motion.obj mt_effct.obj quat2.obj quat3.obj \
	 se_exchg.obj math_utl.obj faceanim.obj ik_sys.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

# DO NOT DELETE
