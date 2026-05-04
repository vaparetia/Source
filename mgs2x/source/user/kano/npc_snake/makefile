#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# 2001/02/05 K.Kano (Modified)
# $Id: makefile,v 1.19 2002/07/08 10:53:32 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kano.a

# オブジェクトリスト
OBJECTS = npc_snake.o npc_common.o npc_command.o \
	snake_main.o snake_situation.o \
	snake_decide.o snake_act.o snake_pad.o snake_life.o snake_weapon.o \
	snake_w45a.o snake_w32a.o c_box.o snake_status.o
XOBJECTS= $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
snake_pad.o : snake_pad.c snake_pad2.c
$(OBJECTS): npc_snake.h args.h npc_snake.mh ../../korekado/npc/npc.h ../../kira/effect/floor/floor.h
