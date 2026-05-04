//
//	w25a_harrier.h	
//
//	written by H.Satoyoshi
//
//	$Id: w25a_harrier.h,v 1.14 2001/09/25 10:48:13 usr03664 Exp $

//    ＊＊＊ハリア関連ヘッダファイル＊＊＊

pack_all	tt_cm.tri meca/harrier/hri_mspod_warhead_cm.dar meca/harrier/hri_clst_warhead_cm.dar \
		meca/kacatka_plant/kck_release_case_cm.dar meca/kacatka_plant/kck_release_case_cm.dar \
                          meca/kacatka_plant/kck_chair.dar\
                          meca/kacatka_plant/kck_cushion_iro.dar item/shield/shl_frg1.dar

pack_trnall	har_ef.tri /* ハリアー用エフェクト※文中のコメントはこの形にしてください */\
	effect/smoke/smoke_lp1_alp.bmp \
	effect/smoke/smoke_lp2_alp.bmp \
	effect/smoke/smoke_lp3_alp.bmp \
	effect/bakuha/bombgas1_alp.bmp \
	effect/bakuha/bombgas6_alp.bmp \
	effect/bonbori/svc_bonbori_r.bmp \
	effect/mark/kirari_alp_ovl.bmp \
        effect/splash/splash07_alp.bmp \
        effect/plant_sea/d_oil_sea_alp_ovl_mod0222.bmp \
        effect/plant_sea/entyou_oil_sea.bmp \
        effect/ray_effect/ray_eye_bonbori_alp.bmp

pack_all	har01.tri meca/harrier/hri_def.dar meca/harrier/hri_clst_math.dar \
                meca/harrier/hri_amraam.dar meca/harrier/hri_mspod.dar \
                meca/harrier/hri_gunpod_flash.dar meca/harrier/hri_pilot_vmp.dar \
                meca/harrier/hri_pilot_sol.dar meca/harrier/hri_canopy_frame_def.dar \
                weapon/m4/m4b_gl.dar goods/demo_m4_grn_sel/demo_m4_grn_bul.dar \
                meca/harrier/hri_brk1.dar \
                meca/kacatka_plant/kck_plant_mrot.dar \
                meca/kacatka_plant/kck_plant_trot.dar

pack_all	kak01.tri meca/kacatka_plant/kck_plant_mt.dar\
                meca/harrier/hri_canopy_high_brk_50hlf_mt.dar \
                meca/harrier/hri_canopy_def_mt.dar


meca	harrier/hri_def.kms
meca	harrier/hri_brk1.kms
meca	harrier/hri_clst_math.kms
meca	harrier/hri_amraam.kms
meca	harrier/hri_canopy_def_mt.kms
meca	harrier/hri_canopy_high_brk_50hlf_mt.kms
meca	harrier/hri_canopy_frame_def.kms
meca	harrier/hri_gunpod_flash.kms
meca	harrier/hri_mspod.kms
meca	harrier/hri_mspod_warhead_cm.kms
meca	harrier/hri_clst_warhead_cm.kms
meca	harrier/hri_pilot_vmp.kms
meca	harrier/hri_pilot_sol.kms
item	shield/shl_frg1.kms		// デブリ用

	//カサッカ
meca	kacatka_plant/kck_plant_mt.kms
meca	kacatka_plant/kck_plant_mrot.kms
meca	kacatka_plant/kck_plant_trot.kms
meca	kacatka_plant/kck_chair.kms
meca	kacatka_plant/kck_cushion_iro.kms

	//スネーク
human	iro_def/iro_def_mt.kms
meca	kacatka_plant/kck_release_case_cm.dar	//お助け箱
weapon	m4/m4b_gl.kms				//Ｍ４Ａ１
goods	demo_m4_grn_sel/demo_m4_grn_bul.kms	//グレネード弾頭
mtn	snkinkak.mls				//スネークモーション
mtn	snknglhako.mls				//スネークの投げる箱のモーション


