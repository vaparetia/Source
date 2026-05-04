#
# Makefile for MGS2 Sub Module
#	user/okajima/etc 用 makefile
#
# 1999/08/17 S.Okajima
# $Id: makefile,v 1.19 2002/02/14 02:37:49 usr03700 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト
OBJ1 = amb_set.o prl_set.o fog_set.o scr_shot.o ok_util.o makky_rain.o
OBJ2 = item_box.o scr_st_c.o shake_camera.o far_clip_set.o cb_wet.o
OBJ3 = randamizer.o ok_init.o
OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3)

XOBJECTS = amb_set.obj prl_set.obj fog_set.obj ok_util.obj makky_rain.obj \
			item_box.obj shake_camera.obj far_clip_set.obj cb_wet.obj randamizer.obj ok_init.obj temp_resolve.obj

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

