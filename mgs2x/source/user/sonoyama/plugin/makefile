#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:49 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = sonoyama.a

# オブジェクトリスト
OBJECTS = wt_door.o ladder.o stance.o lockerd.o \
		  shadowhzx.o spothzx.o run.o jump.o water.o \
		  sneeze.o beltply.o bladeply.o eeply.o eetest.o \
		  ee_swim.o elevator.o fall.o kageshibari.o \
	 	  superblow.o

XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES = ../raiden
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : ../raiden/pl_work.h ../raiden/pl_inline.h ../raiden/pl_inline.c \
			 ../raiden/pl_define.h ../../../game/player.h

wt_door.o : wt_door.c wt_door2.c
wt_door.obj : wt_door.c wt_door2.c
lockerd.o : lockerd.c locker2.c lckcnct.c ../../../game/locker.h
lockerd.obj : lockerd.c locker2.c lckcnct.c ../../../game/locker.h
eeply.o   : ../../morita/emma/include/emma.mh
eeply.obj   : ../../morita/emma/include/emma.mh
