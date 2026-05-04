// w20bcd_common.h
// Ｅ脚・ヘリポート
// 2001/05/06 S.Yamashita
// $Id: w20bcd_common.h,v 1.29 2002/05/31 11:51:49 usr02774 Exp $



// エフェクト切り分け
#ifdef WN_W20C
	#define EFFECT_CUSTUM

	//	#define	E_GLASS_DANKON			// グラス系弾痕
		#define	E_KOGETUKI				// 壁への焦げ付き
	//	#define	E_FOOTSTAMP				// 足跡
		#define	E_ENEMY					// 敵兵系
		#define	E_ENEMY_LIGHT			// 敵兵ライト、スティンガー、ニキータ、剣の火花、しょんべん兵
		#define	E_SLING					// スリング
	//	#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
	//	#define	E_MANHATTAN_LIGHT		// マンハッタンライト
		#define	E_C4BOMB				// Ｃ４爆弾、赤外線センサー
		#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷
	//	#define	E_CROSS_FADE_LIGHT		// クロスフェード（甲板のライトで使用）
	//	#define	E_CHARA_DENTOU			// chara 懐中電灯、フォーチュン戦
		#define	E_WATER					// 水関係
		#define	E_CHARA_HAMON			// chara 波紋
	//	#define	E_BROKEN_OBJECT			// 壊れ物
	//	#define	E_BROKEN_PAPEROBJECT	// 壊れ物紙
	//	#define	E_BROKEN_PLATE			// 壊れ物皿
		#define	E_HIMATU				// 水飛沫
	//	#define	E_CHARA_MADOAME			// chara 窓雨
	//	#define	E_STEAM					// 粉 & 水蒸気
	//	#define	E_POWDER				// 小麦粉
		#define	E_SKY_NORMAL			// 空
	//	#define	E_LINEAR_GUN			// リニアガン
		#define	E_TENKYUU				// 天球ドーム
		#define	E_SUN					// 太陽
		#define	E_DOOR_LUMP				// ドアランプ
		#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
		#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
		#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
		#define	BONBORI					// ぼんぼり
	//	#define	NODE_LAMP				// ノードディスプレイランプ
#endif



// 各ステージ共通要素
#include	"common.h"
#include	"c_effect.h"
#include	"goverpic.h"
//include		wp_plant.h



// キャラクタ

#ifdef WN_W20C
	//----- w20c
	// ファットマン
	human		fat_def/fat_def_sh_mt.kms
	human		fat_def/fat_def_mh_mt.evm
	human		fat_def/fat_nonc_dead_mh_mt.evm		// ファットマン死体
	mtn			fatman.mls
	mtn			corps.mls '-DDEL_BACK=DUMMY'		// ファットマン死体
	far			fat_boss.far
	cvd			human/org_def/org_sgl.cv2
	weapon		glock

	// アニメテクスチャ
	anmtex		fat_no_ooze_blood.rat effect/fat_ooze_blood/fat_ooze_blood00_alp.bmp

	// ファットマンドックタグ
	pack_trnall	kirari.tri \
					effect/mark/kirari_alp_ovl.bmp
	goods		dogtag/dogtag_fat.kms

	// かもめ
//	human		kmo_def/kmo_noalp.kms
//	human		kmo_def/kmo_hane.kms
//	mtn			kamome.mls
	#include	"kamome.h"

    //死体下Ｃ４
    goods		c4_kaitai_c/c4_kaitai_c1.kms
    goods		c4_kaitai_c/c4_kaitai_c1_frost_cm.kms
#else
	//----- w20b
	//----- w20d
	#include	"ene_plant.h"
#endif



// 強制モーション
mtn			rai_w20b.mls





// ワールド
// マルチテクスチャモデル
// それ以外のモデル
option makexti6 -l 256
pack_all	oil.tri \
				effect/plant_sea/d_oil_sea_alp_ovl_mod0222.bmp \
				effect/plant_sea/d_oil_ref_add_alp_ovl_mod1120.bmp
option makexti6

pack_trnall	oil2.tri \
				effect/plant_sea/d_oil_sea_alp_ovl_mod0222.bmp

pack_all	zobject.tri \
				world/w20b/w20b/w20b_dammy_sea.dar \
				world/w20b/w20b/w20b_horo.dar \
				world/door/w20b_dr00.dar

cvd			world/w20b/w20b/w20b_horo.cv2 vn
world		door/w20b_dr00.kms


world		w20b/other

#ifdef WN_W20B
	//----- w20b
	world		w20b/w20b
	hazard		w20b.hzx
	lt2			w20b.lt2
#else
	//----- w20c
	//----- w20d
	world		w20b/w20b/w20b_dammy_sea.kms
	world		w20b/w20b/w20b_horo.kms

	world		w20c/w20c.kms
	hazard		w20c.hzx
	lt2			w20c.lt2
#endif





// オブジェクト
// 小さいサイズのテクスチャはまとめておく

// 解体Ｃ４
goods		c4_kaitai_a

// 電灯虫（廃止）
//pack_all	obj_cm.tri \
//				goods/fly/fly_cm.dar
//
//goods		fly/fly_cm.kms

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
//					itembox/item/box2_ibox.dar \
//					itembox/item/box2_ibox_sh.dar \
//					itembox/item/cbx_label.dar \
//					itembox/weapon/grenade_ibox.dar \
//					itembox/weapon/grenade_ibox_sh.dar \
//					itembox/weapon/sgr_label.dar \
//					itembox/weapon/clm_label.dar
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
//	// ダンボールＣ
//	itembox		item/box2_ibox.kms				// モデル：箱本体
//	itembox		item/box2_ibox_sh.kms			// モデル：箱影
//	itembox		item/cbx_label.kms				// モデル：ラベル
//	// スタン
//	itembox		weapon/grenade_ibox.kms			// モデル：箱本体
//	itembox		weapon/grenade_ibox_sh.kms		// モデル：箱影
//	itembox		weapon/sgr_label.kms			// モデル：ラベル
//	// クレイモア
////	itembox		weapon/grenade_ibox.kms			// モデル：箱本体
////	itembox		weapon/grenade_ibox_sh.kms		// モデル：箱影
//	itembox		weapon/clm_label.kms			// モデル：ラベル





//	振動
vib			etc/drop_sea.vib


// リネーム
rename		org_sgl.cv2		fat_def_sh_mt.cv2
texrename	w20b_24a.kms	w10c_cntw04.bmp		w10c_cntw04_ren.bmp	// ID Conflict のため

#ifdef WN_W20C
	//----- w20c
	rename		dogtag_fat.kms		dogtag_org.kms
#endif
