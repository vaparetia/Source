#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.7 2002/02/07 05:29:35 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト
OBJECTS = chim_smo.o blood.o breath.o shield_smoke.o spark.o inf_cam.o afterimage.o dust_cloud.o s_light_break.o radio_break.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

