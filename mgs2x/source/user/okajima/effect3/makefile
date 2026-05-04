#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 2001/04/18 S.Okajima
# $Id: makefile,v 1.54 2002/02/14 07:16:11 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima3.a

# オブジェクトリスト

OBJ1  = pool_water.o slow_man.o flying_smoke_slow.o pool_water_stable.o
OBJ2  = vamp_drop_splash.o bomb_in_water.o bomb_sphere.o water_reflect.o
OBJ3  = smoke_in_water.o bomb_bubble.o water_surface.o water_surface2.o
OBJ4  = fog_control.o elevator_lamp.o w_mine_lamp.o door_lamp.o gas_in_water.o
OBJ5  = water_reflect_wall.o splush_vertical.o auto_splush_vertical.o
OBJ6  = gas_pers_fast.o fluid_flow_man.o fluid_flow_parts.o raiden_mask_bubble.o
OBJ7  = dynamic_scar.o view_3d.o breath_demo.o blood_drop_demo.o bubble_bullet.o
OBJ8  = blood_strip_demo.o ray_missile_shower.o hitech_face_blood.o sprt_hola.o
OBJ9  = vamp_knife_fix.o strip_dome.o kirari_cm.o drops_many_cm.o strip_hola.o
OBJ10 = mountain_splush.o kirari_water_cm.o splush_rsprt.o vamp_hola.o
OBJ11 = sprt_hola_upper.o poly_hola_upper.o vapor.o death_dive_splush.o
OBJ12 = dive_flow.o ripple_strip.o kirari_water_sun.o ray_missile_launcher.o
OBJ13 = explosion_parts.o explosion_controler.o node_lamp.o dust_cone.o
OBJ14 = peter_blood.o kirari_water_sun2.o mountain_splush_spread.o smoke_strip.o
OBJ15 = smoke2_strip.o gas2_pers_fast.o smoke_strip_control.o splush_rsprt_hari.o
OBJ16 = smoke_mitsukoshi.o smoke_mitsukoshi_man.o
OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15) $(OBJ16)


XOBJ1  = pool_water.obj slow_man.obj flying_smoke_slow.obj pool_water_stable.obj
XOBJ2  = vamp_drop_splash.obj bomb_in_water.obj bomb_sphere.obj water_reflect.obj
XOBJ3  = smoke_in_water.obj bomb_bubble.obj water_surface.obj water_surface2.obj
XOBJ4  = fog_control.obj elevator_lamp.obj w_mine_lamp.obj door_lamp.obj gas_in_water.obj
XOBJ5  = water_reflect_wall.obj splush_vertical.obj auto_splush_vertical.obj
XOBJ6  = gas_pers_fast.obj fluid_flow_man.obj fluid_flow_parts.obj raiden_mask_bubble.obj
XOBJ7  = dynamic_scar.obj view_3d.obj breath_demo.obj blood_drop_demo.obj bubble_bullet.obj
XOBJ8  = blood_strip_demo.obj ray_missile_shower.obj hitech_face_blood.obj sprt_hola.obj
XOBJ9  = vamp_knife_fix.obj strip_dome.obj kirari_cm.obj drops_many_cm.obj strip_hola.obj
XOBJ10 = mountain_splush.obj kirari_water_cm.obj splush_rsprt.obj vamp_hola.obj
XOBJ11 = sprt_hola_upper.obj poly_hola_upper.obj vapor.obj death_dive_splush.obj
XOBJ12 = dive_flow.obj ripple_strip.obj kirari_water_sun.obj ray_missile_launcher.obj
XOBJ13 = explosion_parts.obj explosion_controler.obj node_lamp.obj dust_cone.obj
XOBJ14 = peter_blood.obj kirari_water_sun2.obj mountain_splush_spread.obj smoke_strip.obj
XOBJ15 = smoke2_strip.obj gas2_pers_fast.obj smoke_strip_control.obj splush_rsprt_hari.obj
XOBJ16 = smoke_mitsukoshi.obj smoke_mitsukoshi_man.obj
XOBJECTS = $(XOBJ1) $(XOBJ2) $(XOBJ3) $(XOBJ4) $(XOBJ5) $(XOBJ6) $(XOBJ7) $(XOBJ8) $(XOBJ9) $(XOBJ10) $(XOBJ11) $(XOBJ12) $(XOBJ13) $(XOBJ14) $(XOBJ15) $(XOBJ16)

# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub


explosion_controler.o:	./explosion.h
explosion_parts.o:	./explosion.h

