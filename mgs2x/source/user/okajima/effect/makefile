#
# Makefile for MGS2 Sub Module
#	各ソースがおかれたディレクトリ用 makefile
#
# 1999/07/08 K.Uehara
# $Id: makefile,v 1.112 2002/02/19 11:43:49 usr01475 Exp $
#

# ベースディレクトリ
BASEDIR = ../../..

# ライブラリ名

LIBNAME = okajima.a

# オブジェクトリスト

# ripple.o flame.o sun_set.o particle.o poly_dst.o test1.o 
# mask_ply.o scrn_tex.o pichpich.o g_light.o shading.o rain_run.o
# blood.o rain_gas.o


ifneq ($(MGS2_XBOX),1)  # ----------------- PS2版設定

OBJ1 = rain.o rain_fix.o rain_parts.o objs_fall.o rain_cm_window.o bomb_gas.o
OBJ2 = blood2.o blood_cm.o blood_wl.o blood_dp.o blood_sp.o hako.o slowdown.o
OBJ3 = wind_man.o blur.o blur_dpt.o c_light_spot.o fade_in_out.o bomb.o
OBJ4 = c_light.o z_man.o body_sdw.o blood_part1.o blood_part2.o bb_dan.o
OBJ5 = mdl_anim.o mdl_p_am.o lit_man.o body_sph.o set_tex.o wind_local.o
OBJ6 = ft_splsh.o rain_cm.o mdl_p2.o off_man.o bomb_wall.o
OBJ7 = set_t_fw.o set_t_fc.o set_t_dr.o shield_e.o shield_crsh.o bomb_kasu.o
OBJ8 = weapon_sph.o weapon_sph2.o rain_gas_pers.o ripple.o line_sph.o
OBJ9 = flush_man.o bokashi.o steam_smoke.o achuu.o achuu_parts.o bomb_chaff.o
OBJ10= crush_dust.o lesson.o saa_smoke.o blood_m9.o tear_drops.o rot_y_object.o
OBJ11= projector_light.o bomb_chaff_screen.o flying_smoke2.o bomb_stan.o
OBJ12= foot_print.o blur_timer.o spherelight.o george_bonbori.o cv2_view.o
OBJ13= rain_slow.o c_box_parts.o c_box_body.o cigar.o extinguisher.o blood_bio.o
OBJ14= cb_splash.o cb_hole_light.o splash_rot.o c_box_dust.o extinguisher_mini.o
OBJ15= extinguisher_jet.o steam_cm.o flush_screen.o bomb2.o rain_gas_pers_demo.o
OBJ16= flying_smoke_no_hzd.o flying_smoke.o cotton.o nikita_ef.o snake_breath_cm.o
OBJ17= twin_dr.o chair_hit.o sofa_hit.o wind_local2.o set_t_plant.o

OBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15) $(OBJ16) $(OBJ17)

else
# ----------------- Xbox版設定
OBJ1 = rain.obj rain_fix.obj rain_parts.obj objs_fall.obj rain_cm_window.obj bomb_gas.obj
OBJ2 = blood2.obj blood_cm.obj blood_wl.obj blood_dp.obj blood_sp.obj hako.obj slowdown.obj
OBJ3 = wind_man.obj blur_dpt.obj c_light_spot.obj fade_in_out.obj bomb.obj
OBJ4 = c_light.obj z_man.obj body_sdw.obj blood_part1.obj blood_part2.obj bb_dan.obj
OBJ5 = mdl_anim.obj mdl_p_am.obj lit_man.obj body_sph.obj set_tex.obj wind_local.obj
OBJ6 = ft_splsh.obj rain_cm.obj mdl_p2.obj off_man.obj bomb_wall.obj
OBJ7 = set_t_fw.obj set_t_fc.obj set_t_dr.obj shield_e.obj shield_crsh.obj bomb_kasu.obj
OBJ8 = weapon_sph.obj weapon_sph2.obj rain_gas_pers.obj ripple.obj line_sph.obj
OBJ9 = flush_man.obj steam_smoke.obj achuu.obj achuu_parts.obj bomb_chaff.obj
OBJ10= crush_dust.obj lesson.obj saa_smoke.obj blood_m9.obj tear_drops.obj rot_y_object.obj
OBJ11= projector_light.obj bomb_chaff_screen.obj flying_smoke2.obj bomb_stan.obj
OBJ12= foot_print.obj spherelight.obj george_bonbori.obj cv2_view.obj
OBJ13= rain_slow.obj c_box_parts.obj c_box_body.obj cigar.obj blood_bio.obj
OBJ14= cb_splash.obj cb_hole_light.obj splash_rot.obj c_box_dust.obj 
OBJ15= extinguisher_jet.obj steam_cm.obj flush_screen.obj bomb2.obj rain_gas_pers_demo.obj
OBJ16= flying_smoke_no_hzd.obj flying_smoke.obj cotton.obj nikita_ef.obj snake_breath_cm.obj
OBJ17= twin_dr.obj chair_hit.obj sofa_hit.obj wind_local2.obj set_t_plant.obj


OBJ_DMA= blur.obj bokashi.obj blur_timer.obj 


# PERROR
# 

OBJ_19= extinguisher.obj extinguisher_mini.obj

XOBJECTS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15) $(OBJ16) $(OBJ17) $(OBJ_19) $(OBJ_DMA)

#$(OBJ_DMA)

endif




# ここでのローカルフラグリスト


# foot_print.cで落ちることを確認したので、最適化オプションを無効にいたしました。
LOCAL_INCLUDES =
LOCAL_DEFINES = 
LOCAL_OPTIONS =

# 設定の実体

include $(BASEDIR)/makefile.sub

rain_cm.obj:	./rain_cm.h

blood_part1.obj:	./blood.h
blood_part2.obj:	./blood.h
blood2.obj:	./blood.h
blood_cm.obj:	./blood.h
blood_wl.obj:	./blood.h
blood_dp.obj:	./blood.h
blood_sp.obj:	./blood.h

lit_man.obj:	./lit_man.h
body_sdw.obj:	./lit_man.h


bomb.obj:		./bomb_vibration.h
bomb_chaff.obj:	./bomb_vibration.h
bomb_stan.obj:	./bomb_vibration.h
bomb2.obj:	./bomb_vibration.h
