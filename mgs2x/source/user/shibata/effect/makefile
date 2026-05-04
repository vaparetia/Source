#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.33 2002/02/13 05:17:58 usr01749 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = shibata.a

# オブジェクトリスト

OBJECTS = 	gnrl_sprt.o 2d_sprt.o win_rain.o scr_crack.o \
			puddle.o scr_water.o line_smoke.o \
			plant_sky.o plant_sun.o \
			ray_eye.o gnrl_sprt2.o \
			water_con.o blood_water.o \
			flow_paper.o cam_lamp.o put_light.o \
			lens_flr_gm.o blade_eft.o sky_util.o \
			ray_laser.o scr_sight.o scr_drop.o gnrl_poly.o \
			scr_goggles.o wc_flush.o scr_shimmer.o scr_bubble.o \
			scr_waterfilm.o scr_trans.o 2d_sprt_pause.o scr_hex.o

XOBJECTS = 	gnrl_sprt.obj 2d_sprt.obj win_rain.obj scr_crack.obj \
			puddle.obj scr_water.obj line_smoke.obj \
			plant_sky.obj plant_sun.obj \
			ray_eye.obj gnrl_sprt2.obj \
			water_con.obj blood_water.obj \
			flow_paper.obj cam_lamp.obj put_light.obj \
			lens_flr_gm.obj blade_eft.obj sky_util.obj \
			ray_laser.obj scr_sight.obj scr_drop.obj gnrl_poly.obj \
			scr_goggles.obj wc_flush.obj scr_shimmer.obj scr_bubble.obj \
			scr_waterfilm.obj scr_trans.obj 2d_sprt_pause.obj scr_hex.obj


# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト
scr_crack.o : ../util/dma_set.h ../util/dma_set.c
scr_water.o : ../util/dma_set.h ../util/dma_set.c

scr_crack.obj : ../util/dma_set.h ../util/dma_set.c
scr_water.obj : ../util/dma_set.h ../util/dma_set.c
