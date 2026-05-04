#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.10 2002/02/14 05:15:16 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

#OBJECTS = test_slater.o test_slater2.o test_slater3.o test_slater4.o

OBJECTS = prev_test.o emm_slater.o trample_slater.o check_slater.o \
			demo_slater.o cam_slater.o all_slater.o sea_slater.o \
			body_slater.o

XOBJECTS = emm_slater.obj trample_slater.obj check_slater.obj \
			demo_slater.obj cam_slater.obj all_slater.obj sea_slater.obj \
			body_slater.obj

#prev_test.obj はプレビュー用なのでとりあえずあとで

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

all_slater.o: sea_slater.h
sea_slater.o: sea_slater.h
body_slater.o: body_route.h

all_slater.obj: sea_slater.h
sea_slater.obj: sea_slater.h
body_slater.obj: body_route.h



