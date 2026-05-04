//
//	w31c_game.h
//	第二濾過室デモ共通部分データ
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w31c_common.h,v 1.14 2002/08/29 08:10:54 usr02774 Exp $

// c_effect用define
#define	EFFECT_CUSTUM			// c_effectのデフォルトdefineを使用しない
#define	E_GLASS_DANKON			// グラス系弾痕
#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_ENEMY					// 敵兵系
#define	E_ENEMY_LIGHT			// 敵兵ライト、体から出る影
#define	E_FOOTSTAMP				// 足跡
#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷
#define	E_WATER					// 水関係
#define	E_CHARA_HAMON			// chara 波紋
#define	E_BROKEN_OBJECT			// 壊れ物
#define	E_HIMATU				// 水飛沫
#define	E_CHARA_MADOAME			// chara 窓雨
#define	E_DOOR_LUMP				// ドアランプ
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
#define	NODE_LAMP				// ノードディスプレイランプ


// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"

// 強制モーション
// 横開きロッカー
mtn		locker_sl.mls

// 追加エフェクト
// 水面照り返し
pack_trnall	effect.tri \
	effect/water_reflection/water_reflection_alp_01.bmp \
	effect/water_reflection/water_reflection_alp_02.bmp

// ワールド
option makexti6 -l 1024 -f PAL8
pack_all	w31c.tri \
			world/w31c/w31c0 \
			world/w31c/w31c1 \
			world/door/dr_lv4_00.dar \
			world/door/dr_free.dar
option makexti6

pack_all	water.tri world/w31c/w31c_water

world		w31c/w31c0
world		w31c/w31c1
world		w31c/w31c_water
world		door/dr_lv4_00.kms
world		door/dr_free.kms
world		w31b/sea.kms
lt2			w31c.lt2
hazard		w31c.hzx

// テクスチャアニメデータ
anmtex	w31c.rat \
	effect/w31_hakei

// 水中処理用頂点データ
cvd		human/sna_skl/sna_skl3.cvd vn



// オブジェクト
pack_all	object.tri \
			goods/watermine/watermine.dar /* 水中機雷 */ \
			goods/null /* 壊れライトダミー用 */ \
			goods/moai/moai.dar /* モアイ */


goods		watermine/watermine.kms
goods		null
goods		moai/moai.kms

// 水中ゴミ
pack_all	dust.tri \
				goods/dust/kan.dar \
				goods/dust/wtd_vul_figure/wtd_vul_figure.dar \
				goods/dust/wtd_hiyoko/wtd_hiyoko.dar \
				goods/dust/wtd_kan/wtd_kan.dar \
				goods/dust/wtd_papercup/wtd_papercup.dar

// 気泡
//debug	kimura/temp/scr_kihou03.kms
//cvd		debug/kimura/temp/scr_kihou03.cv2 vn
effect	scr_effect/scr_kihou01.kms
cvd		effect/scr_effect/scr_kihou01.cv2 vn

pack_all	dust_mt.tri \
				goods/dust/wtd_petbottle_mt.dar \
				goods/dust/wtd_smallpet_mt.dar

// 紙屑
pack_all	paper.tri \
				goods/dust/paper_a.dar \
				goods/dust/paper_b.dar

goods	dust/paper_a.kms
goods	dust/paper_b.kms

cvd		goods/dust/paper_a.cv2 vn
cvd		goods/dust/paper_b.cv2 vn



goods		dust/kan.kms
goods		dust/wtd_vul_figure/wtd_vul_figure.kms
goods		dust/wtd_hiyoko/wtd_hiyoko.kms
goods		dust/wtd_kan/wtd_kan.kms
goods		dust/wtd_papercup/wtd_papercup.kms

goods		dust/wtd_petbottle_mt.kms
goods		dust/wtd_smallpet_mt.kms

#ifdef MGS2_XBOX_1
	debug takabe/raw_ipux/reflact02.rix
#else
	debug takabe/raw_ipu/reflact02.rim
#endif

// デバッグ用
// 小さいサイズのテクスチャはまとめておく
