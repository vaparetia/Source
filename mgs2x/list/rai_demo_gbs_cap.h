//
//	rai_demo_gbs_cap.h
//	ライデン／ポリデモ用
//	ゴルルコ兵装（キャップあり）時常駐データ
//
//	2001/07/29	M.Sonoyama
//	$Id: rai_demo_gbs_cap.h,v 1.4 2002/07/22 06:38:57 usr04098 Exp $

// ライデン

pack_all 	rai_tex_nm.tri \
            human/rai_def/rai_mag.dar \
            human/rai_def/rai_hlst.dar \
            human/rai_def/rai_grip.dar \
            debug/shibata/brk_screen/brk_scr07_full.dar 

pack_all	rai_tex_mw.tri \
            human/rai_gbs/rai_gbs_gbshead_mh_mt.dar \
            human/rai_gbs/rai_gbs_body_mh_mt.dar

// マルチウェイト
human		rai_gbs/rai_gbs_gbshead_mh_mt.evm
human		rai_gbs/rai_gbs_body_mh_mt.evm

// キャラクタ 

//wp_plant.h で入る
//human		rai_gbs/rai_gbs_mt.kms
//human		rai_gbs/rai_gbs_sh_mt.kms

human		rai_def/rai_mag.kms
human		rai_def/rai_hlst.kms
human		rai_def/rai_grip.kms

// 主観腕
// （不要）

// 影
goods		shadow
human		rai_def/rai_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

//顔のキズ
//human		rai_def/rai_kizu_face_mh.evm

// 頂点データ 
cvd			human/rai_gbs/rai_gbs_mt.cv2

// モーション
//（不要）

// リネーム
rename		rai_gbs_mt.kms 		rai_def.kms
rename		rai_gbs_mt.cv2		rai_def.cv2
rename		rai_gbs_sh_mt.kms	rai_def_sh_mt.kms

