#
# Makefile for MGS2 Sub Module
#	user/okajima/etc 用 makefile
#
# 1999/08/17 S.Okajima
# $Id: makefile,v 1.32 2002/02/07 10:16:15 usr03875 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト
OBJ1 = d_blood.o d_blood1.o d_blood2.o d_splash.o d_splash_parts.o
OBJ2 = d_body_plasma_dot.o d_plasma_line.o d_rain_cm.o d_splash_ripple.o
OBJ3 = d_gas_fixed_pers.o d_gas_fixed_world.o d_gas_on_camera.o d_vol_shadow.o
OBJ4 = d_plasma_poly.o d_thunder.o d_thunder_parts.o d_set_hola.o dive_splash.o
OBJ5 = d_splash_floor.o d_splash_camera.o d_fog_set.o d_splash_motion.o
OBJ6 = d_splash_parts_slow.o d_splash_fall.o d_inter_poly.o gw_lines_model.o
OBJ7 = d_fade_io.o d_fade_io_prog.o d_contrast.o common_smoke.o camera_crush.o
OBJ8 = d_inter_poly_grenade.o d_fade_io_prog_strong.o
OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)


XOBJ1 = d_blood.obj d_blood1.obj d_blood2.obj d_splash.obj d_splash_parts.obj
XOBJ2 = d_body_plasma_dot.obj d_plasma_line.obj d_rain_cm.obj d_splash_ripple.obj
XOBJ3 = d_gas_fixed_pers.obj d_gas_fixed_world.obj d_gas_on_camera.obj d_vol_shadow.obj
XOBJ4 = d_plasma_poly.obj d_thunder.obj d_thunder_parts.obj d_set_hola.obj dive_splash.obj
XOBJ5 = d_splash_floor.obj d_splash_camera.obj d_fog_set.obj d_splash_motion.obj
XOBJ6 = d_splash_parts_slow.obj d_splash_fall.obj d_inter_poly.obj gw_lines_model.obj
XOBJ7 = d_fade_io.obj d_fade_io_prog.obj d_contrast.obj common_smoke.obj camera_crush.obj
XOBJ8 = d_inter_poly_grenade.obj d_fade_io_prog_strong.obj
XOBJECTS = $(XOBJ1) $(XOBJ2) $(XOBJ3) $(XOBJ4) $(XOBJ5) $(XOBJ6) $(XOBJ7) $(XOBJ8)



# ここでのローカルフラグリスト

LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS = 

# 設定の実体

include $(BASEDIR)/makefile.sub

d_blood.c:	../effect/blood.h
d_blood1.c:	../effect/blood.h
d_blood2.c:	../effect/blood.h

