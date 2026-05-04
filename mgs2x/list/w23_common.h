//
//	w23_common.h
//
//	written by H.Yoshiike 2000.Oct
//
//	$Id: w23_common.h,v 1.18 2001/10/29 12:06:15 usr03379 Exp $

//	ステージ名表記用
#define		WN_W23A

block .use r_plt0
block .nocache
block .cache

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		wp_plant.h


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
	//	天狗兵とウォークマン兵のモーションはいらない
#define 	ENE_MAR '-DDEL_TNG=DUMMY -DDEL_WALKMAN=DUMMY'

//	影用モデル
#define		ENE_SHADOW

//	キャラクタ
#include	"ene_plant.h"


// 強制モーション
mtn		rai_w23a.mls

//	アタリ
hazard		w23a.hzx

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
itembox

//	パッドデモ
	//	PAL版
#ifdef MGS2_VMODE_PAL
rpd			w23a_paddemo_n00_pal.rpd
	//	それ以外
#else
rpd			w23a_paddemo_n00.rpd
rpd			w23a_paddemo_n01.rpd
rpd			w23a_paddemo_n02.rpd
#endif

// デバッグ用
// 小さいサイズのテクスチャはまとめておく

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

// リネーム
