// w12ac_common.h
// Ａ脚・屋上(共通)
// 2001/05/06 S.Yamashita
// $Id: w12ac_common.h,v 1.37 2002/05/31 11:51:49 usr02774 Exp $



// 各ステージ共通要素
#include	"common.h"
#include	"c_effect.h"
#include	"goverpic.h"
//include		wp_plant.h



// キャラクタ
#include	"ene_plant.h"
include		"kamome.h"



// 強制モーション
mtn			rai_w12a.mls





// ワールド
// マルチテクスチャモデル
// それ以外のモデル

world		w12_common/other
world		w12_common/w12_bg
world		w12_common/fence
hazard		w12a.hzx

#ifdef WN_W12A
	//----- w12a
option makexti6 -l 256
	pack_all	oil.tri \
					effect/plant_sea/m_oil_sea_alp_ovl_mod0222.bmp \
					effect/plant_sea/m_oil_ref_add_alp_ovl_mod1120.bmp
option makexti6

	// w12a_elvsk01、w12a_objdr00、w12a_objdr01、w12a_dr00、w12a_sdw00
	pack_all	zobject.tri \
					world/w12a/w12a/w12a_elvsk01.dar \
					world/w12a/w12a/w12a_objdr00.dar \
					world/w12a/w12a/w12a_objdr01.dar \
					world/door/w12a_dr00.dar

	world		door/w12a_dr00.kms	// Ａ脚内へのドア(左右共用)

	world		w12a/w12a
	lt2			w12a.lt2

#else
	//----- w12c
option makexti6 -l 256
	pack_all	oil.tri \
					effect/plant_sea/d_oil_sea_alp_ovl_mod0222.bmp \
					effect/plant_sea/d_oil_ref_add_alp_ovl_mod1120.bmp
option makexti6

	// w12c_elvsk01、w12c_elvsk02、w12a_dr00、w12c_sdw05d_alp.bmp、w12c_sdw05e_alp.bmp、w12c_sdw05f_alp.bmp
	pack_all	zobject.tri \
					world/w12c/w12c/w12c_elvsk01.dar \
					world/w12c/w12c/w12c_elvsk02.dar \
					world/door/w12c_dr00.dar

	pack_all	shimmer.tri \
					effect/haze/haze03_alp_ovl.bmp

	world		door/w12c_dr00.kms

	world		w12c/w12c
	lt2			w12c.lt2
#endif

// ダミー海
world		w20b/w20b/w20b_dammy_sea.kms





// オブジェクト
// 小さいサイズのテクスチャはまとめておく

// 解体Ｃ４
goods		c4_kaitai_a

// 昇降機
world		elevator/elv_1/elv_1.kms

// アイテム
itembox

//	pack_all	itembox.tri \
//					itembox/item/dogtag_ibox.dar \
//					itembox/item/dogtag_ibox_sh.dar \
//					itembox/item/dogtag_ibox.dar \
//					itembox/amo/handgun_amo_ibox.dar \
//					itembox/amo/handgun_amo_ibox_sh.dar \
//					itembox/amo/scm_amo_label.dar \
//					itembox/amo/m92_amo_label.dar \
//					itembox/item/ration_ibox.dar \
//					itembox/item/ration_ibox_sh.dar \
//					itembox/item/rtn_label.dar \
//					itembox/item/medicine_ibox.dar \
//					itembox/item/medicine_ibox_sh.dar \
//					itembox/item/dzp_label.dar \
//					itembox/item/dzp2_label.dar \
//					itembox/weapon/grenade_ibox.dar \
//					itembox/weapon/grenade_ibox_sh.dar \
//					itembox/weapon/cgr_label.dar \
//					itembox/item/sbs_label.dar
//
//	// ドッグタグ
//	itembox		item/dogtag_ibox.kms			// モデル：箱本体
//	itembox		item/dogtag_ibox_sh.kms			// モデル：箱影
//	itembox		item/dogtag_ibox.kms			// モデル：ラベル
//	// 弾薬_ソコム
//	itembox		amo/handgun_amo_ibox.kms		// モデル：箱本体
//	itembox		amo/handgun_amo_ibox_sh.kms		// モデル：箱影
//	itembox		amo/scm_amo_label.kms			// モデル：ラベル
//	// 弾薬_Ｍ９
////	itembox		amo/handgun_amo_ibox.kms		// モデル：箱本体
////	itembox		amo/handgun_amo_ibox_sh.kms		// モデル：箱影
//	itembox		amo/m92_amo_label.kms			// モデル：ラベル
//	// レーション
//	itembox		item/ration_ibox.kms			// モデル：箱本体
//	itembox		item/ration_ibox_sh.kms			// モデル：箱影
//	itembox		item/rtn_label.kms				// モデル：ラベル
//	// ジアゼパム
//	itembox		item/medicine_ibox.kms			// モデル：箱本体
//	itembox		item/medicine_ibox_sh.kms		// モデル：箱影
//	itembox		item/dzp_label.kms				// モデル：ラベル
//	itembox		item/dzp2_label.kms				// モデル：ラベル
//	// チャフ
//	itembox		weapon/grenade_ibox.kms			// モデル：箱本体
//	itembox		weapon/grenade_ibox_sh.kms		// モデル：箱影
//	itembox		weapon/cgr_label.kms			// モデル：ラベル
//	// 止血剤
////	itembox		item/medicine_ibox.kms			// モデル：箱本体
////	itembox		item/medicine_ibox_sh.kms		// モデル：箱影
//	itembox		item/sbs_label.kms				// モデル：ラベル





//	振動
vib			etc/drop_sea.vib
vib			etc/dock_ele_01.vib
vib			etc/dock_ele_02.vib
vib			etc/dock_ele_03.vib
vib			program/ptr_dead.vib	//ぴーたー死亡無線中死無怒宇





// リネーム
texrename w12a_24a.kms w10c_cntw04.bmp w10c_cntw04_ren.bmp	// ID Conflict のため
