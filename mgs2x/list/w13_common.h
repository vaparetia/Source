//
//	w13_common.h
//
//	written by Y.Matsuhana 2000.May
//
//	$Id: w13_common.h,v 1.19 2001/10/29 11:17:25 usr03379 Exp $

//	ステージ名表記用
#define WN_W13A

block .use r_plt0
block .nocache
block .cache

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"

//	かもめ
include		kamome.h	// modified by Okuta 2001/05/10


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
mtn		rai_w13a.mls

//	アタリ
hazard		w13a.hzx

//	ダミー
goods		null/null.kms

//	ゲームオーバー
//pack_all	gmov.tri 2D/game_over/dekisi_alp_ovl.bmp
#include "goverpic.h"

//	ドア
pack_all	w13_dr.tri	world/door/w13a_dr00.dar \
						world/door/w13a_dr01.dar
world/door	w13a_dr00.kms
world/door	w13a_dr01.kms

// アイテム
itembox

//	パッドデモ
	//	PAL版
#ifdef MGS2_VMODE_PAL
rpd			w13a_paddemo_00_pal.rpd
	//	それ以外
#else
rpd			w13a_paddemo_00.rpd
#endif

// デバッグ用
// 小さいサイズのテクスチャはまとめておく
debug		mogi/floor_sound.kms

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

// リネーム

