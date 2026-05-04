//
//	sna_bossrush.h
//	スネーク／ボスラッシュ用データセット
//	（モーションが違うだけ）
//  2001/09/17	M.Sonoyama
//	$Id: sna_bossrush.h,v 1.2 2002/07/22 06:38:57 usr04098 Exp $

// スネーク

// マルチウェイト
pack_all	sna_def_mw.tri \
    human/snh_snake_hands/snh_def_mh_mt.dar

// （シングルウェイト）マルチテクスチャ
pack_all	sna_def_mt.tri \
	human/sna_def/sna_def.dar \
	human/sna_def/sna_def_sh.dar
human		sna_def/sna_def.kms
human		sna_def/sna_def_sh.kms

// 共有モデル（co_モデル）
//pack_all	sna_def_cm.tri \

// 上記以外のノーマルモデル

pack_all	sna_def_nm.tri \
	human/sna_def/sna_bdn1.dar human/sna_def/sna_bdn2.dar \
	human/sna_def/sna_mugen_bdn1.dar human/sna_def/sna_mugen_bdn2.dar \
	human/sna_def/sna_mag1.dar human/sna_def/sna_mag2.dar \
	human/sna_def/sna_mag3.dar human/sna_def/sna_mag4.dar \
    human/snh_snake_hands/snh_def_dummy.dar \
    human/sna_def/sna_shadow.dar \
    debug/shibata/brk_screen/brk_scr07_full.dar

// バンダナ
human		sna_def/sna_bdn1.kms
human		sna_def/sna_bdn2.kms
human		sna_def/sna_mugen_bdn1.kms
human		sna_def/sna_mugen_bdn2.kms


// マガジン
human		sna_def/sna_mag1.kms
human		sna_def/sna_mag2.kms
human		sna_def/sna_mag3.kms
human		sna_def/sna_mag4.kms

// 主観時の腕
human	    snh_snake_hands/snh_def_mh_mt.evm
human		snh_snake_hands/snh_def_dummy.kms

// 影
goods		shadow
human		sna_def/sna_shadow.kms
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri effect/shdw/foot_shadow_alp.bmp  /*XBOX用の足影*/
#endif

//主観カメラわれ
debug		shibata/brk_screen/brk_scr07_full.kms

// 頂点データ
// ローポリシングルウエイトスネーク用
cvd		human/sna_def/sna_def.cv2
// ハイポリシングルウエイトスネーク用
cvd		human/sna_def/sna_def_sh.cv2

// モーション

// スネーク常駐モーション
mtn		sna_bossrush.mls
// 主観時の腕のモーション
mtn		sna_arm_bossrush.mls

// リネーム
rename	sna_bossrush.mar	snake.mar
rename	sna_bossrush.sar	snake.sar
rename	sna_bossrush.var	snake.var

rename	sna_arm_bossrush.mar	sna_arm_mh.mar

