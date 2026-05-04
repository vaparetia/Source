#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.1.1.3 2002/11/19 11:50:35 Yoshizawa1 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = skoba.a

# オブジェクトリスト
OBJ1 = mazzle.o mazzleflash.o mazzleflash_sub.o mazzleflash_sub2.o mazzleflash2.o gunsmoke_single.o gunsmoke_single2.o
OBJ2 = mdl_cartridge.o sps_amo.o sps_mazzle.o smoke.o sps_smoke.o
OBJ3 = stg_fall_parts.o stg_spark.o stg_blur.o stg_smoke_front.o
OBJ4 = stg_smoke_back.o wpeffect.o blowback.o crtrg.o crtrg_ft.o
OBJ5 = cartridge.o connect_smoke.o connect_smoke_skoba.o
OBJ6 = LineSmokeMng.o clay_bomb_smoke.o 
OBJ7 = sub_mazzle.o sub_mazzle_mng.o
OBJ8 = line_box.o smoke2.o emb_control.o emb_call.o

OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)

XOBJ1 = mazzle.obj mazzleflash.obj mazzleflash_sub.obj mazzleflash_sub2.obj mazzleflash2.obj gunsmoke_single.obj gunsmoke_single2.obj
XOBJ2 = mdl_cartridge.obj sps_amo.obj sps_mazzle.obj smoke.obj sps_smoke.obj
XOBJ3 = stg_fall_parts.obj stg_spark.obj  stg_blur.obj stg_smoke_front.obj
XOBJ4 = stg_smoke_back.obj wpeffect.obj blowback.obj crtrg.obj crtrg_ft.obj
XOBJ5 = cartridge.obj connect_smoke.obj connect_smoke_skoba.obj
XOBJ6 = LineSmokeMng.obj clay_bomb_smoke.obj 
XOBJ7 = sub_mazzle.obj sub_mazzle_mng.obj
XOBJ8 = line_box.obj smoke2.obj emb_control.obj emb_call.obj

XOBJECTS = $(XOBJ1) $(XOBJ2) $(XOBJ3) $(XOBJ4) $(XOBJ5) $(XOBJ6) $(XOBJ7) $(XOBJ8)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

$(OBJECTS) : ../../../game/player.h
emb_control.o : emb_control.h
emb_call.o : emb_control.h