//
//	ene_teng.h	
//
// $Id: ene_tng.h,v 1.11 2002/05/13 09:07:55 usr03379 Exp $
// タンカー編での敵兵

// キャラクタ 

// マルチウェイト



// 上記以外のノーマルモデル
//pack_all	gbs_gba_def_nm.tri \
//	effect/gradation/gradation_alp_ovl.bmp \
//	item/radio \
//	item/shield \
//	item/gbs_nv_goggle \
//	item/sougan 
// gbsとgbaのテクスチャ重複はスネークほどでは
// ないが、gbsの方でマルチテクスチャとノーマル
// の両方があるので、その部分の重複を回避する
// ために一つに。(00.07.12)
//pack_all	gbss.tri \
//		human/gbs_def
pack_trnall katana_eft.tri \
	debug/shibata/staffroll/alpha03_alp_ovl.bmp

//pack_all	tng.tri \
//		human/tng_def

human	tng_def/tng_high.kms
human	tng_def/tng_saya.kms
human	tng_def/tng_magazine.kms
human	tng_def/tng_strap.kms
cvd		human/tng_def/tng_high.cv2 vn



weapon/p90
weapon	tbl/tbl_katana.kms
item	sougan 




mtn	gbs.mls '-DDEL_SHIELD=DUMMY -DDEL_SHOTGUN=DUMMY \
	-DDEL_WALKMAN=DUMMY \
//	-DDEL_TNG=DUMMY \
//	-DDEL_RADIO=DUMMY \
	-DDEL_EROTICA=DUMMY \
	-DDEL_BOX=DUMMY \
//	-DDEL_GRD=DUMMY \ /*そのうちけずれる*/
//	-DDEL_NEAR_ATTACK=DUMMY \
//	-DDEL_HANG=DUMMY \
	-DDEL_PARTS=DUMMY \
//	-DDEL_CARRY=DUMMY \
	-DDEL_TEST=DUMMY'


//mtn	gbs.mls 

//rename	tng_a.mar gbs.mar
//rename	tng_a.sar gbs.sar
//rename	tng.mar gbs.mar
//rename	tng.sar gbs.sar

