//
//	wp71a_common.h
//	武器訓練ブレード１面２面共通
//	written by S.Mukaide 2002.Apr
//
//	$Id:

block .use r_vr_r
block .nocache
block .cache


// ＶＲ的関連define
#define VR_TGT_STRAW	1	// 刀用
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


// ワールド
// マルチテクスチャモデル
// それ以外のモデル
option makexti6 -l 1024 -f PAL8		//	ＸＢＯＸ用劣化無しテクスチャ用
pack_all	ws71a.tri \
	world/vr_weapon/ws71a
option makexti6						//	ＸＢＯＸ用劣化無しテクスチャ用

world		vr_weapon/ws71a
hazard		ws71a.hzx

rol			world/vr_weapon/ws71a/ws71a.rol

// オブジェクト

// 消火器

// アイテム
itembox

// 刀セット一式
#include	"blade.h"

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
