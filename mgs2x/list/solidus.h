//
//	solidus.h
//	ソリダス戦に必要なものリスト
//
//	2001/04/13	M.Sonoyama
//	$Id: solidus.h,v 1.12 2001/09/01 06:42:35 usr02011 Exp $

pack_trnall	sol_eff.tri \
	debug/morita/w11c_test/w11c2_fire3a_alp.bmp \
	debug/morita/w11c_test/w11c2_fire3b_alp.bmp \	
	debug/morita/w11c_test/w11c2_smoke1_alp.bmp \
	debug/morita/w11c_test/w11c2_smoke2_alp.bmp \
    effect/bonbori/w_bonbori.bmp \
    effect/wave/wave12_alp_ovl.bmp 

pack_all	sol_mh_tex.tri	\
	human/sol_def/sol_def_face_mh_mt.dar \
	human/sol_def/sol_snakearm_mh_mt.dar

pack_all	sol_tex.tri \
	human/sol_def/sol_vanim_0_sh_mt.dar 

pack_all	sol_sword_tex.tri \
	weapon/minsyutou/mst_l_mt.dar \
	weapon/kyowatou/kwt_r_mt.dar \
    human/sol_def/sol_gantai_obj.dar \
    goods/demo_katana_sol/demo_kwt_sht.dar \
    goods/demo_katana_sol/demo_mst_sht.dar \
    human/sol_def/sol_faceguard.dar

human	sol_def/sol_def_face_mh_mt.evm
human	sol_def/sol_snakearm_mh_mt.evm
human	sol_def/sol_vanim_0_sh_mt.kms
human	sol_def/sol_gantai_obj.kms

weapon	minsyutou/mst_l_mt.kms
weapon	kyowatou/kwt_r_mt.kms

cvd		human/sol_def/sol_vanim_0_sh_mt.cv2 
cvd		human/sol_def/sol_vanim_1_sh_mt.cv2

mtn		solidus.mls
mtn		sol_snakearm.mls
mtn		rai_solidus.mls

pack_all	missile.tri \
        weapon/snakearm_missile/sol_msl.dar \
        weapon/snakearm_missile/sol_msl_r.dar \
        weapon/snakearm_missile/sol_msl_l.dar 

pack_all	sol_comdl.tri \
        weapon/kyowatou/kwt_hl_cm.dar \
        weapon/minsyutou/mst_hl_cm.dar

weapon	snakearm_missile/sol_msl.kms
weapon	snakearm_missile/sol_msl_r.kms
weapon	snakearm_missile/sol_msl_l.kms

weapon	kyowatou/kwt_hl_cm.kms
weapon	minsyutou/mst_hl_cm.kms

goods	demo_katana_sol/demo_kwt_sht.kms
goods	demo_katana_sol/demo_mst_sht.kms
human	sol_def/sol_faceguard.kms
