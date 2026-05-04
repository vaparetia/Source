#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/11/09 K.Kano (Modified)
# $Id: makefile,v 1.18 2002/02/14 04:00:44 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = debugcam.o prekey.o human_preview2.o va_preview.o \
	prefog.o human.o object.o prenullobj.o precamera.o prefile.o premove.o pretex.o \
	skeltest.o fobj.o preparticle.o
XOBJECTS = skeltest.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

debugcam.o : debugcam.c preview.h
prekey.o : prekey.c prekey.h
human_preview.o : human_preview.c human_preview.h
human_preview2.o : human_preview2.c human_preview.h
va_preview.o : va_preview.c va_preview.h
prefog.o : prefog.c prefog.h
prenullobj.o : prenullobj.c
human.o : human.c human.h
precamera.o : precamera.c precamera.h
pretex.o : pretex.c pretex.h
preparticle.o : preparticle.c preparticle.h
