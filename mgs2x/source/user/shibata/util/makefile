#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.13 2002/07/19 02:21:39 usr04761 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

OBJECTS = ts_util.o online_util.o ts_subwin.o vib_edit.o dma_set.o \
          prev_sky.o prev_sun.o online_util2.o 

XOBJECTS = ts_util.obj online_util.obj ts_subwin.obj vib_edit.obj dma_set.obj \
          prev_sky.obj prev_sun.obj online_util2.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

ts_subwin.o : ts_subwin.c ../../sigeno/display/radsize.h
vib_edit.o : ../../kano/preview/preview.h

//ts_subwin.obj : ts_subwin.c ../../sigeno/display/radsize.h
ts_subwin.obj : ts_subwin.c ../../kira/radar/radsize.h
vib_edit.obj : ../../kano/preview/preview.h

