#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = skoba.a

# オブジェクトリスト
OBJECTS = wp_mng.o usp.o psg.o grenade.o cfr.o nkt.o stg.o \
	 	  spray.o blade.o clay_line.o lattice_mng.o  \
		  arms_con.o stg_layout.o ray_layout.o vtr_layout.o \
		  dg_cam_layout.o psg_layout.o thermal_layout.o \
		  night_layout.o nikita_layout.o tdg_cam_layout.o \
		  clay_eff.o clay_mng.o dg_cam_shuter_layout.o \
		  ray_uv_layout.o scope_layout.o book.o stg_uv_layout.o \
		  demo_dg_cam_layout.o demo_dg_cam_shuter_layout.o \
		  equip_layout.o ai_ray_layout.o

XOBJECTS = $(OBJECTS:.o=.obj) usp_dmy1.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

wp_mng.o usp.o psg.0 grenade.o \
cfr.o nkt.o stg.o spray.o blade.o : ../../../game/player.h
