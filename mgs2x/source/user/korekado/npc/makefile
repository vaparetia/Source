#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2001/02/08 Y.Korekado
# $Id: makefile,v 1.4 2002/02/01 08:50:22 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = korekado.a

# オブジェクトリスト
OBJECTS = npc.o npcact.o sample.o 

XOBJECTS = npc.obj npcact.obj sample.obj 

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../conv
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

npc.o : npc.c npc.h
npcact.o : npcact.c npc.h
sample.o : sample.c smpact.c smppre.c smpthink.c

npc.obj : npc.c npc.h
npcact.obj : npcact.c npc.h
sample.obj : sample.c smpact.c smppre.c smpthink.c
