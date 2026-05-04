//
//	w03b_game.src
//
//	written by H.Yoshiike 1999.Oct
//
//	$Id: w03b_game.h,v 1.13 2002/10/02 02:33:43 usr04249 Exp $



// 各ステージ共通要素
#define	EFFECT_CUSTUM

#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_ENEMY					// 敵兵系
#define	E_SLING					// スリング
#define	E_ENEMY_LIGHT			// 敵兵ライト
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_BROKEN_OBJECT			// 壊れ物
#define	E_STEAM					// 粉 & 水蒸気
#define	E_POWDER				// 小麦粉
#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷
#define	E_CROSS_FADE_LIGHT		// クロスフェード（甲板のライトで使用）
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト

#define	E_SKY_NORMAL			// 刀のエフェクトで使用しているらしい

#include		"t_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_tank.h
//include		item_box_tank.h
itembox

// キャラクタ
//include		snake.h
include		ene_tank.h
//include		ene_w03a.h

//タンカークルーモデル
//human	tnc_def

// ワールド
//pack_all	w03bd.tri world/w03a/w03b world/w03a/w03d
//world		w03a/w03a
world		w03b/w03b0
world		w03b/w03b1
world		w03b/w03b2
//world		w03a/w03e
lt2		w03b.lt2
//lt2		debug
hazard		w03b.hzx

// デバッグ用
//pack_all	object.tri world/w03 world/door/w03bsdr_handle world/door/w03bsdr_body goods/sling_test goods/corpse_a goods/corpse_u

world		w03
world		door/w03bsdr_handle.kms
world		door/w03bsdr_body.kms
//goods		sling_test
//goods		corpse_a.kms
//goods		corpse_u.kms


// 消火器のモデル
debug	okajima/hose_test/hose_cone.kms
debug	okajima/hose_test/hose_new2.kms
cvd		debug/okajima/hose_test/hose_new2.cvd vn

// 水密ドア開閉用モーション
mtn			sna_hatch.mls
mtn			rai_wt_hatch.mls
mtn			hatch_door_sna.mls
mtn			hatch_handle_sna.mls
// 開かない水密ドア用の透明モデル
world		w01d/dining/w01d0tray.kms


//デバッグ用箱
world	w00b/obs_box3.kms


//敵兵用
rename	gbs_w03a.mar gbs.mar

#ifndef WN_ANOTHER
// ボスラッシュ用シナリオ
gcx		boss
#endif

