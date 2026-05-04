#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.62 2002/05/07 07:00:05 usr04098 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = kunibe.a

# オブジェクトリスト
OBJECTS = landing_smoke.o running_smoke.o pipe_gas.o rising_smoke.o test_spiral2.o \
harrier_wing_smoke.o harrier_missile_fire.o harrier_pod_smoke.o harrier_vulcan_fire.o harrier_flare.o \
har_missile_smoke.o harrier_missile_smoke.o harrier_missile_smoke2.o harrier_vulcan.o harrier_missile_fire2.o \
point_debug.o harrier_claster_smoke.o harrier_clasterunit_smoke.o exp_smoke.o \
test_fire.o harrier_light.o harrier_claster_spark.o missile_exp.o test_fire2.o dogtag_flash.o e3_missile_exp.o \
harrier_vernier_smoke.o harrier_vernier.o test_fire3.o shower.o big_explosion.o \
solidas_dash_fire.o harrier_flash.o pee.o rising_smoke2.o fire_powder.o solidus_damage_plasma.o harrier_damage_smoke.o \
test_water_pillar.o solidus_missile_fire.o solidus_missile_smoke.o fatman_scratch.o solidus_dunk_smoke.o \
solidus_snakearm_flow.o solidus_snakearm_plasma.o blade_spark.o light_spark.o point_fire.o punch_smoke.o poly_plasma.o \
poly_plasma2.o sprt_plasma.o flying_warm.o stage_fire.o rainbow.o bathroom_pee.o electric_floor.o poly_plasma3.o \
damage_plasma.o sprt_plasma2.o door_pannel_spark.o separate_plasma.o body_fire.o body_smoke.o cypher_plasma.o \
power_pannel_spark.o light_spark2.o blade_spark2.o solidus_test_fire.o solidus_test_fire2.o running_smoke2.o \
cypher_debris.o crash_debris.o cypher_exp.o cypher_light.o solidus_mant_fire.o wine.o solidus_dash_light.o \
fatman_scratch_spark.o c_box_fire.o c_box_smoke.o harrier_flare_smoke.o harrier_claster_exp.o demo_sun.o \
harrier_stage_break.o harrier_tank_break.o demo_rising_smoke.o bridge_exp.o bridge_smoke.o fatman_scratch_smoke.o \
vamp_knife_aura.o solidus_missile_barn.o solidus_blade_light.o solidus_blade_wind.o solidus_energy_prim.o \
fatman_demo_scratch_smoke.o splash_blood.o book_fire.o point_splash_blood.o blood_smoke.o blood_smoke_inpact.o \
solidus_damage_spark.o ray_fall_blood.o demo_body_plasma.o demo_electric_floor.o blade_flow.o cypher_rising_smoke.o \
emma_body_fire.o demo_splash_blood.o point_dogtag_flash.o demo_dummypoint_blood.o demo_plasma_color.o \
tng_stage_monitor.o tng_monitor_control.o tng_monitor_control_42a.o tng_monitor_control_44a.o ray_blood_smoke.o \
demo_plasma_poly_color.o

XOBJECTS = $(OBJECTS:.o=.obj)


# ここでのローカルフラグリスト


#LOCAL_INCLUDES = ../../korekado/action ../../korekado/enemy \
#	../../korekado/conv 
#../attacker 
#LOCAL_INCLUDES = ../../korekado/enemy 

LOCAL_INCLUDES = 
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

# 依存関係のリスト

cypher.o : cypher.c eyecheck.h 
sv_camera.o : sv_camera.c eyecheck.h 
eyecheck.o : eyecheck.c eyecheck.h 
guncam.o : guncam.c eyecheck.h 
enemy.h : action.h 

