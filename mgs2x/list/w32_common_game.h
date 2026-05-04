//
//	w32_common_game.h
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w32_common_game.h,v 1.28 2002/05/31 11:04:13 usr02774 Exp $

//	ステージ名表記用
#define WN_W32A
//#define		EVENING_SKY		//	夕空
#define			W32A_EVENING_SKY		//	夕空

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_plant.h

// キャラクタ
	//	エマ
#define		W32A
#include	"emma.h"
//	かもめ
human              kmo_def
mtn    '-DWN_W32A' kamome.mls

	//	ＮＰＣスネーク
//human		sna_def/sna_def.kms
human		iro_def/iro_def_mt.kms


//	モーション
	//	強制モーション
mtn		rai_w32a.mls

	//	ＮＰＣスネーク
mtn		'-DM9_MOTION=DUMMY -DM4_MOTION=DUMMY -DNORMAL_MOTION=DUMMY -DW17A_MOTION=DUMMY -DW14A_MOTION=DUMMY' npc_snake.mls
mtn		'-DCAPTURE_MOTION=DUMMY' npc_snake_cap.mls
mtn		'-DDRAG_MOTION=DUMMY' npc_snake_drag.mls

// ワールド
// マルチテクスチャモデル
// それ以外のモデル 
world		w32a/w32a
world		w32a/other
lt2			w32a.lt2
hazard		w32a.hzx

//	ダミー
goods		null/null.kms

//	海とオイルフェンス
/*
pack_all oil.tri \
	debug/takabe/tex/oil_alp_ovl_mod0222.bmp \
	debug/takabe/tex/sky_evening_add_alp_ovl_mod1120.bmp
*/
//debug takabe/oilfence/oilfence.kms

option makexti6 -l 256
pack_all	oil.tri \
		effect/plant_sea/s_oil_sea_alp_ovl_mod0022.bmp \
		effect/plant_sea/s_oil_ref_add_alp_ovl_mod1120.bmp
option makexti6




// アイテム
//itembox	//アイテムボックスは独自よび
	//	ＰＳＧ１系
itembox		amo/rifle_amo_ibox.kms
itembox		amo/rifle_amo_ibox_sh.kms

	//	ＰＳＧ１ラベル
itembox		amo/psg_amo_label.kms
	//	ＰＳＧ１-Ｔラベル
itembox		amo/psgT_amo_label.kms

	//	レーション
itembox		item/ration_ibox.kms
itembox		item/ration_ibox_sh.kms

	//	レーションラベル
itembox		item/rtn_label.kms

	//	ジアゼパム
itembox		item/medicine_ibox.kms
itembox		item/medicine_ibox_sh.kms

	//	ジアゼパムラベル
#ifdef MGS2_LANG_ENGLISH
	itembox		 item/dzp2_label.kms
#else
	itembox		 item/dzp_label.kms
#endif

	//	サーマル
itembox		item/goggle_ibox.kms
itembox		item/goggle_ibox_sh.kms

	//	サーマルラベル
itembox		item/tgl_label.kms

// ドッグタグ
pack_trnall	kirari.tri \
		effect/mark/kirari_alp_ovl.bmp
goods	dogtag/dogtag_sna.kms


// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
texrename w32a_24a.kms w10c_cntw04.bmp w10c_cntw04aaaa.bmp

