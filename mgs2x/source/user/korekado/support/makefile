#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.8 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = support.o  spalert.o spavoid.o spcautio.o

XOBJECTS = support.obj  spalert.obj spavoid.obj spcautio.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../watcher ../enemy ../conv ../action
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

support.o : support.c spprepro.c spthink.c spaction.c support.h ../watcher/wcgetres.c
spcautio.o : spcautio.c spcauact.c
spalert.o : spalert.c spaleact.c
spavoid.o : spavoid.c spavoact.c

support.obj : support.c spprepro.c spthink.c spaction.c support.h ../watcher/wcgetres.c
spcautio.obj : spcautio.c spcauact.c
spalert.obj : spalert.c spaleact.c
spavoid.obj : spavoid.c spavoact.c
$(OBJECTS) : ../enemy/enemy.h ../action/action.h
