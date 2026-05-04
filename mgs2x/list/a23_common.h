//
//	a23_common.h（アナザーミッション用）
//
//	written by H.Yoshiike 2002.Feb
//
//	$Id: a23_common.h,v 1.8 2002/06/28 10:48:16 usr03379 Exp $

//	ステージ名表記用
#define		WN_W23A
// アナザーミッション明示
#define		WN_ANOTHER

block .use r_vr_s
block .nocache
block .cache

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_plant.h

//	オルタナティブ追加エフェクト
#include	"alt_effect.h"

// 敵兵タイプ
	//	警備兵
#define		ENE_P_GOL
	//	通常アタッカー
#define		ENE_ATTACKER
	//	ハイテク兵
#define		ENE_HIGH_TECH


// 特殊兵
	//	ショットガン兵セット
#define		ENE_SHOT
	//	盾兵セット
#define		ENE_SHIELD

// 特殊兵のモーション設定
	//	天狗兵とウォークマン兵とロッカーとトイレのモーションはいらない
#define 	ENE_MAR '-DDEL_TNG=DUMMY -DDEL_WALKMAN=DUMMY -DDEL_LOCKER=DUMMY -DDEL_TOILET=DUMMY'

//	影用モデル
#define		ENE_SHADOW

//	キャラクタ
#include	"ene_plant.h"


// 強制モーション
mtn		rai_a23a.mls
mtn		sna_a23a.mls

//	アタリ
hazard		a23a.hzx


//	ダミー
goods		null/null.kms

//	ゲームオーバー
//pack_all	gmov.tri 2D/game_over/dekisi_alp_ovl.bmp
#include "goverpic.h"

//	ドア
pack_all	w23_dr.tri	world/door/w23a_dr00.dar \
						world/door/w23a_dr01.dar \
						world/door/w23a_dr02.dar

world		door/w23a_dr00.kms
world		door/w23a_dr01.kms
world		door/w23a_dr02.kms

// オブジェクト
#define		NORM_CYP
#include	"cypher.h"

// アイテム
//itembox
#include	"a_item.h"

// デバッグ用
// 小さいサイズのテクスチャはまとめておく

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

// リネーム
