#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 1999/09/21 K.Kano (Modified)
# $Id: makefile,v 1.6 2002/02/04 06:36:16 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = corpse.o free_corpse2.o pull_corpse.o
XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

corpse.o : corpse.c corpse.h
pull_corpse.o : pull_corpse.c corpse.h
free_corpse.o : free_corpse.c corpse.h
free_corpse2.o : free_corpse2.c corpse.h
