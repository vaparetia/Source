#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.32 2002/07/08 05:18:09 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = satoyoshi.a

# オブジェクトリスト
OBJECTS = har_main.o har_sdmng.o har_command.o har_command2.o \
	har_kasacka.o bos_kasacka.o har_kascomm.o \
	har_global.o

XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト
LOCAL_INCLUDES = 
LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

har_main.o :\
	harrier.def   har_initfst.h\
	har_inline.c har_message.c \
	har_target.c  har_debug.c   har_parts.c    har_move.c	\
	har_flare.c   har_pmove.c   har_seeffect.c har_pirots.c	\
	har_missile.c har_claster.c har_mpod.c     har_vulc.c	\
	har_pirots.h  har_missile.h har_claster.h  har_mpod.h	\
	harrier.h     harp_norm.c   har_acrobat.c  har_bakudan.c\
	har_burn.c    har_hover.c    har_mpmv.c   \
	harp_amram.c  har_damage.c  har_sound.c	   har_think.c  \
	har_command.c har_mphov.c   har_start.c    har_vibration.c \
	fly_data.c

har_kasacka.o :\
	harrier.def   har_initfst.h\
	harrier.h     har_snkgrn.c  har_kastgt.c   har_snake.c	\
	har_inline.c \
	har_kasmove.c har_kakitem.c har_dbg.c      har_kastart.c\
	har_kasacka.h

bos_kasacka.o :\
	har_initfst.h\
	har_inline.c \
	boss.def bos_kastgt.c	bos_kasmove.c	bos_kastart.c	har_kasacka.h



har_main.obj :\
	harrier.def   har_initfst.h\
	har_inline.c har_message.c \
	har_target.c  har_debug.c   har_parts.c    har_move.c	\
	har_flare.c   har_pmove.c   har_seeffect.c har_pirots.c	\
	har_missile.c har_claster.c har_mpod.c     har_vulc.c	\
	har_pirots.h  har_missile.h har_claster.h  har_mpod.h	\
	harrier.h     harp_norm.c   har_acrobat.c  har_bakudan.c\
	har_burn.c    har_hover.c    har_mpmv.c   \
	harp_amram.c  har_damage.c  har_sound.c	   har_think.c  \
	har_command.c har_mphov.c   har_start.c    har_vibration.c \
	fly_data.c

har_kasacka.obj :\
	harrier.def   har_initfst.h\
	harrier.h     har_snkgrn.c  har_kastgt.c   har_snake.c	\
	har_inline.c \
	har_kasmove.c har_kakitem.c har_dbg.c      har_kastart.c\
	har_kasacka.h

bos_kasacka.obj :\
	har_initfst.h\
	har_inline.c \
	boss.def bos_kastgt.c	bos_kasmove.c	bos_kastart.c	har_kasacka.h
