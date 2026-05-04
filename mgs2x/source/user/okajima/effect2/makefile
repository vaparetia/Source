#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.53 2002/07/18 02:35:13 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima2.a

# オブジェクトリスト

OBJ1  = dynamic_flow.o bubble_many.o bubble_line.o prism_bullet.o splush_tidal.o
OBJ2  = window_light.o splush_man.o ripple_man.o spot_blur_line.o
OBJ3  = bubble_cm.o dust_cm.o ripple_bubble.o line_brust.o side_flare.o
OBJ4  = splush_plane.o water_front.o water_front_prim.o # gas_line.o
OBJ5  = liner_gun_plasma.o floor_light_man.o blow_line.o water_drops.o
OBJ6  = drop_body_splush.o splush_surface_man.o splush_big.o water_wind.o
OBJ7  = splush_surface_gravity_man.o sonic_wave.o check_water_level.o
OBJ8  = slit_light.o short_spark.o str_disp.o bubble_motion.o inter_line_blade.o
OBJ9  = splush_tidal_parts.o metal_blow_water.o rain_cm_con.o wall_tidal.o
OBJ10 = splush_tidal_parts2.o point_tidal.o blood_strip.o blood_strip_parts.o
OBJ11 = circle_splush.o auto_splush.o gll_splush.o splush_tidal_parts3.o ripple_stable.o 
OBJ12 = drop_body_splush_prog.o slit_light2.o liner_gun_init.o
OBJ13 = liner_plasma_small.o liner_gun_plasma_flush.o crush_parts.o
OBJ14 = delay_tmplight.o splush_tidal_parts4.o ray_sphere.o dust_area.o
OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14)

XOBJECTS = $(OBJECTS:.o=.obj)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

bubble_many.o:	./bubble.h
bubble_cm.o:	./bubble.h
bubble_line.o:	./bubble.h
dust_cm.o:	./bubble.h

