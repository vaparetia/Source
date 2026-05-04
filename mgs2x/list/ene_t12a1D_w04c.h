//
//	ene_t12a1D_w04c.h	
//
//	written by H.Yoshiike 1999.Oct
//
//	$Id: ene_t12a1D_w04c.h,v 1.3 2001/06/04 10:50:13 usr02011 Exp $

// タンカー編での敵兵

// キャラクタ 

// マルチウェイト

//（シングルウェイト）マルチテクスチャ
//pack_all	gbs_def_mt.tri \
//	human/gbs_def/gbs_def_mt.dar 

// 共有モデル（暫定的に手動で切り分け。後に自動化予定）
pack_all	gbs_def_cm.tri \
	item/shield/shl_frg1.dar \
	item/shield/shl_frg2.dar \
	item/shield/shl_frg3.dar

// 上記以外のノーマルモデル
pack_all	gbs_gba_def_nm.tri \
	effect/gradation/gradation_alp_ovl.bmp \
	item/radio \
	item/shield \
	item/gbs_nv_goggle \
	item/sougan \
	human/gbs_hand \
	human/gbs_def/gbs_eye0.dar \
	human/gbs_def/gbs_eye1.dar \
	human/gbs_def/gbs_eye2.dar \
	human/gbs_def/gbs_eye3.dar \
	human/gbs_def/gbs_eye4.dar \
	human/gbs_def/gbs_bp.dar \
	human/gbs_def/gbs_hlst.dar \
	//human/gbs_def/gbs_mag.dar \
	//human/gbs_def/gbs_knif.dar \
	human/gbs_def/gbs_sling.dar \
	human/gba_def/gba_sling.dar \
	human/gbs_def/gbs_kanagu.dar \
	human/gba_def/gba_knif.dar
//	human/gbs_def \
//	human/gba_def \

// gbsとgbaのテクスチャ重複はスネークほどでは
// ないが、gbsの方でマルチテクスチャとノーマル
// の両方があるので、その部分の重複を回避する
// ために一つに。(00.07.12)
pack_all	gbss.tri \
	human/gba_def \
	human/gbs_def/gbs_def.dar 
//	effect/gradation/gradation_alp_ovl.bmp \
//	item/radio \
//	item/shield \
//	item/gbs_nv_goggle \
//	item/sougan \
//	human/gbs_hand 

human	gbs_def/gbs_eye0.kms
human	gbs_def/gbs_eye1.kms
human	gbs_def/gbs_eye2.kms
human	gbs_def/gbs_eye3.kms
human	gbs_def/gbs_eye4.kms
human	gbs_def/gbs_bp.kms 				/* バックパック */ 
human	gbs_def/gbs_hlst.kms 				/* ホルスター */ 
//human	gbs_def/gbs_mag.kms 				/* マガジン */ 
//human	gbs_def/gbs_knif.kms 				/* 警備兵のナイフ */ 
human	gbs_def/gbs_sling.kms 			/* 警備兵のスリング */ 
human	gbs_def/gbs_kanagu.kms 			/* スリングを留める金具 */
human	gbs_def/gbs_def.kms

cvd			human/gbs_def
human		gba_def
cvd			human/gba_def
item		radio 
item		shield
item		gbs_nv_goggle
item		sougan

//フィンガーサイン用
human		gbs_hand/gbs_hand_def.kms
cvd			human/gbs_hand vn

//口パク
/*
human		gbs_face/gbs_face_def.kms
cvd			human/gbs_face/gbs_face_def.kms vn
cvd			human/gbs_face/gbs_face0.kms vn
cvd			human/gbs_face/gbs_face1.kms vn
*/


