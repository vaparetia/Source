#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2000/03/23 Y.Korekado
# $Id: makefile,v 1.5 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = hostage.o skirt.o hostcomm.o
 
XOBJECTS = hostage.obj skirt.obj hostcomm.obj
 
# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../action ../enemy ../conv ../npc
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

hostage.o : hostage.c hstgpre.c hstgthink.c hstgact.c hostage.h
hostcomm.o : hostcomm.c hostage.h

hostage.obj : hostage.c hstgpre.c hstgthink.c hstgact.c hostage.h
hostcomm.obj : hostcomm.c hostage.h

