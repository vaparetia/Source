//
//	raiden.h	
//
//	written by H.Yoshiike 1999.Oct
//  $Id: raiden_photo.h,v 1.3 2002/07/24 04:24:30 usr04249 Exp $

// ライデン

pack_all	rai_tex_mt.tri \
            human/rai_def/rai_def_mt.dar \ 
            human/rai_def/rai_def_sh_mt.dar 

pack_all	rai_shadow.tri \
            human/rai_def/rai_hair_shadow_mt.dar \
            human/rai_def/rai_hair_shadow_mugen.dar \
            human/rai_def/rai_hair_shadow_blue.dar \
            human/rai_def/rai_hair_shadow_red.dar

pack_all 	rai_tex_nm.tri \
            human/rai_def/rai_mag.dar \
            human/rai_def/rai_hlst.dar \
            human/rai_def/rai_grip.dar \
            human/rah_raiden_hand/rah_def_dummy.dar \
            debug/shibata/brk_screen/brk_scr07_full.dar \
            human/rai_def/rai_hair_bounding.dar

pack_all	rai_tex_mw.tri \
            human/rah_raiden_hand/rah_def_mh_mt.dar \
            human/rai_def/rai_zura_mugen_mh_mt.dar \
            human/rai_def/rai_zura_blue_mh_mt.dar \
            human/rai_def/rai_zura_red_mh_mt.dar \
            human/rai_def/rai_hair_mh_mt.dar

// キャラクタ 
human		rai_def/rai_def_mt.kms
human		rai_def/rai_def_sh_mt.kms
human		rai_def/rai_mag.kms
human		rai_def/rai_hlst.kms
human		rai_def/rai_grip.kms

// 髪の毛
human		rai_def/rai_hair_mh_mt.evm
human		rai_def/rai_hair_shadow_mt.kms

// かつらせっと
human		rai_def/rai_zura_mugen_mh_mt.evm 
//human		rai_def/rai_zura_blue_mh_mt.evm
//human		rai_def/rai_zura_red_mh_mt.evm

// 髪の毛用バウンディングボックス
human		rai_def/rai_hair_bounding.kms

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

//顔のキズ
//human		rai_def/rai_kizu_face_mh.evm

// 頂点データ 
cvd			human/rai_def/rai_def_mt.cv2

// モーション
// ニキータとＲＧＢ６を削除
#define RAI_MAR '-DDEL_A_RIFLE -DDEL_NIKITA=DUMMY -DDEL_GRENADE -DDEL_C4 -DDEL_CLAYMORE -DDEL_RGB6=DUMMY -DDEL_MIC'

mtn		raiden.mls RAI_MAR
mtn		rai_arm_mh.mls

// リネーム
//rename		rai_def_sh_mt.kms	rai_def.kms
//rename		rai_def_mt.kms	rai_def_mt2.kms
rename		rai_def_mt.kms	rai_def.kms
rename		rai_def_mt.cv2	rai_def.cv2

