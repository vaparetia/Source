#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.13 2002/05/09 06:11:07 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = watcher.o wcsneak.o wcalert.o wcavoid.o wcress.o wccomm.o  combat.o

XOBJECTS = watcher.obj wcsneak.obj wcalert.obj wcavoid.obj wcress.obj wccomm.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

watcher.o : watcher.c wcprepro.c wcthink.c wcaction.c watcher.h wcgetres.c
wcsneak.o : wcsneak.c wcsneact.c modechng.c
wcalert.o : wcalert.c wcaleact.c
wcavoid.o : wcavoid.c wcavoact.c modechng.c

watcher.obj : watcher.c wcprepro.c wcthink.c wcaction.c watcher.h wcgetres.c
wcsneak.obj : wcsneak.c wcsneact.c modechng.c
wcalert.obj : wcalert.c wcaleact.c
wcavoid.obj : wcavoid.c wcavoact.c modechng.c
$(OBJECTS) : ../enemy/enemy.h ../action/action.h
