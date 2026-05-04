#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.4 2002/04/15 11:16:43 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = mode.a

# オブジェクトリスト
OBJECTS = 	mobile.o mob_panel.o
XOBJECTS = 	mobile.obj mob_panel.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト


mobile.o:	mobile.c mobile.h mobact.h
mobile.obj:	mobile.c mobile.h mobact.h
mob_panel.o:	mob_panel.c mob_panel.h mobile.h mobact.h
mob_panel.obj:	mob_panel.c mob_panel.h mobile.h mobact.h

mobact.h:	mobact.lst
	macrobld.pl < $< > $@
