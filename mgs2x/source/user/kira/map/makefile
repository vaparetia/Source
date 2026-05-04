#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.7 2002/04/22 08:28:28 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kira.a

# オブジェクトリスト
OBJECTS = allmap.o map_3d.o

#yano DMAPACK関連で map_3d.c 外しました。
XOBJECTS = $(OBJECTS:.o=.obj)
#XOBJECTS = allmap.obj


# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = -D_DEBUG_

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


allmap.o:	allmap.c map_config.h map_action.h
allmap.obj:	allmap.c map_config.h map_action.h

map_action.h:	map_action.lst
	perl ./listcnv.pl MAP_ < $< > $@


map_3d.o:	map_3d.c map_3d.h
map_3d.obj:	map_3d.c map_3d.h
