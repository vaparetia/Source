//
//	raiden_d.h	
//	潜水服ライデン
//
//	written by H.Yoshiike 1999.Oct
//  $Id: raiden_d.h,v 1.14 2002/07/22 06:38:57 usr04098 Exp $

// 潜水服ライデン
pack_all	rai_tex_mt.tri \
			human/rai_diver/rai_diver_mt.dar \ 
			human/rai_diver/rai_diver_sh_mt.dar \ 
			human/rai_diver/rai_diver_mh_mt.dar 

// ライデン装備品
pack_all	rai_tex_nm.tri \
            human/rai_def/rai_mag.dar \
            human/rai_def/rai_hlst.dar \
            human/rai_def/rai_grip.dar \
            human/rah_raiden_hand/rah_def_dummy.dar \
            debug/shibata/brk_screen/brk_scr07_full.dar

pack_all	rai_tex_mw.tri \
            //human/snh_snake_hands/snh_def_mh_mt.dar \
            human/rah_raiden_hand/rah_def_mh_mt.dar


// キャラクタモデル 
human		rai_diver/rai_hair_diver_mh_mt.evm
human		rai_diver/rai_diver_bounding.kms
human		rai_diver/rai_diver_mt.kms
human		rai_diver/rai_diver_sh_mt.kms
human		rai_diver/rai_diver_mh_mt.evm
human		rai_def/rai_mag.kms
human		rai_def/rai_hlst.kms
human		rai_def/rai_grip.kms

// 主観腕
human	    rah_raiden_hand/rah_def_mh_mt.evm
human	    rah_raiden_hand/rah_def_dummy.kms
//human		snh_snake_hands/snh_def_mh_mt.evm
//human		snh_snake_hands/snh_def_dummy.kms

// 影
goods		shadow
human		rai_def/rai_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

// カメラ水
debug     shibata/awa_model/awa_model2.kms
cvd       debug/shibata/awa_model/awa_model2.cv2

// 頂点データ 
cvd			human/rai_diver/rai_diver_mt.cv2

// モーション
mtn		raiden.mls
mtn		rai_arm_mh.mls

// リネーム
rename		rai_diver_mt.kms	rai_def.kms
rename		rai_diver_sh_mt.kms	rai_def_sh_mt.kms
rename		rai_diver_mt.cv2	rai_def.cv2
