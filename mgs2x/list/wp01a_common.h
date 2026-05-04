//
//	wp01a_common.h
//	武器訓練ハンドガン１面２面共通
//	written by S.Mukaide 2002.Feb
//
//	$Id:

block .use r_vr_r
block .nocache
block .cache

// 敵兵関連define
// 敵兵タイプ
#define ENE_P_GOL		// w22aは都市迷彩攻撃兵
#define ENE_ATTACKER	// 攻撃兵
#define ENE_SHIELD		// 盾兵
#define ENE_SHOT		// ショットガン兵
#define ENE_HIGH_TECH	// 人質イベント後はハイテク兵

// 特殊兵のモーション設定
#define ENE_MAR '-DDEL_TNG=DUMMY -DDEL_WALKMAN=DUMMY'

// ＶＲ的関連define
#define VR_TGT_TRIANGLE	1	// 三角的
// ＶＲ的
#include	"vrstage.h"
// ＶＲエフェクト関連define
#include	"vr_def.h"

// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"
//	ＶＲ追加エフェクト
#include	"vr_effect.h"

// キャラクタ
#include	"ene_plant.h"

// ワールド
// マルチテクスチャモデル
// それ以外のモデル
option makexti6 -l 1024 -f PAL8		//	ＸＢＯＸ用劣化無しテクスチャ用
pack_all	ws01a.tri \
	world/vr_weapon/ws01a
option makexti6						//	ＸＢＯＸ用劣化無しテクスチャ用

world		vr_weapon/ws01a
hazard		ws01a.hzx

rol			world/vr_weapon/ws01a/ws01a.rol

// オブジェクト

// 消火器

// アイテム
itembox

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
