#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.7 2002/02/07 07:16:19 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

OBJECTS = imagetrg.o ab_mind.o sleep.o discovery.o

XOBJECTS = imagetrg.obj ab_mind.obj sleep.obj discovery.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

camera_crack_test.o: crack_data.h
test_go.o: pix_data.h
test_go2.o: line_data.h

camera_crack_test.obj: crack_data.h
test_go.obj: pix_data.h
test_go2.obj: line_data.h

