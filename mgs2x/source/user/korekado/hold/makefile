#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2000/03/23 Y.Korekado
# $Id: makefile,v 1.10 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = holdene.o checkene.o rootene.o camera.o
 
XOBJECTS = holdene.obj checkene.obj rootene.obj camera.obj
 
# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

holdene.o : holdene.c holdene.h
checkene.o : checkene.c chthink.c checkene.h holdene.h
rootene.o : rootene.c rtthink.c rootene.h holdene.h

holdene.obj : holdene.c holdene.h
checkene.obj : checkene.c chthink.c checkene.h holdene.h
rootene.obj : rootene.c rtthink.c rootene.h holdene.h

$(OBJECTS) : ../enemy/enemy.h ../action/action.h
