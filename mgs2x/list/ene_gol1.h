//
//	ene_gol1.h
//
//	written by S.Nojiri 2001.Apl
//

// ゴル兵の基本装備。タンカー編メインで使用（ene_tank.h,ene_plant_gol1.hにインクルード）
#ifdef ENE_T_SHOTGUN_GOL
	#define ENE_T_SHOTGUN_GOL_DAR	weapon/spas12
#else
	#define ENE_T_SHOTGUN_GOL_DAR	human/gbs_hand		// ダミー
#endif

// キャラクタ

// マルチウェイト

//（シングルウェイト）マルチテクスチャ
//pack_all	gbs_def_mt.tri \
//	human/gbs_def/gbs_def_mt.dar

// 共有モデル（暫定的に手動で切り分け。後に自動化予定）
pack_all	gbs_def_cm.tri \
	item/shield/shl_frg1.dar

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
	human/gba_def/gba_knif.dar \
	ENE_T_SHOTGUN_GOL_DAR \
	weapon/aks/aks.dar \
	weapon/aks/aks_sp.dar
//	human/gbs_def \
//	human/gba_def \

	pack_trnall	kirari.tri \
		effect/mark/kirari_alp_ovl.bmp

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
goods	dogtag/dogtag_gps.kms			/* ドッグタグ */
human	gbs_def/gbs_bp.kms 				/* バックパック */
human	gbs_def/gbs_hlst.kms 				/* ホルスター */
//human	gbs_def/gbs_mag.kms 				/* マガジン */
//human	gbs_def/gbs_knif.kms 				/* 警備兵のナイフ */
human	gbs_def/gbs_sling.kms 			/* 警備兵のスリング */
human	gbs_def/gbs_kanagu.kms 			/* スリングを留める金具 */
human	gbs_def/gbs_def.kms

weapon		aks/aks.kms
weapon		aks/aks_sp.kms

#ifdef ENE_T_SHOTGUN_GOL
weapon		spas12			// ショットガン
#endif

cvd			human/gbs_def
human		gba_def
cvd			human/gba_def
item		radio
item		shield
item		gbs_nv_goggle
item		sougan

//フィンガーサイン用
human		gbs_hand/gbs_hand_def.kms
cvd			human/gbs_hand/gbs_hand_def.cv2 vn
//cvd			human/gbs_hand/gbs_hand0.cv2  vn
cvd			human/gbs_hand/gbs_hand1.cv2  vn
cvd			human/gbs_hand/gbs_hand2.cv2  vn
cvd			human/gbs_hand/gbs_hand3.cv2  vn
cvd			human/gbs_hand/gbs_hand4.cv2  vn
cvd			human/gbs_hand/gbs_hand5.cv2  vn


//口パク
/*
human		gbs_face/gbs_face_def.kms
cvd			human/gbs_face/gbs_face_def.kms vn
cvd			human/gbs_face/gbs_face0.kms vn
cvd			human/gbs_face/gbs_face1.kms vn
*/


