//
//	w21_common.h
//
//	written by H.Yoshiike 2000.Oct
//
//	$Id: w21_common.h,v 1.21 2001/10/29 13:11:38 usr03379 Exp $

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

//	キャラクタ
#include	"ene_plant.h"

// 強制モーション
mtn		rai_w21a.mls

//	海
/*
pack_all	oil.tri \
		debug/takabe/tex/oil_alp_ovl_mod0222.bmp \
		debug/takabe/tex/sky_day_add_alp_ovl_mod1120.bmp
*/

//	アタリ
hazard		w21a.hzx

//	ダミー
goods		null/null.kms

//	ゲームオーバー
//pack_all	gmov.tri 2D/game_over/dekisi_alp_ovl.bmp
#include "goverpic.h"

//	ドア
pack_all	w21_dr.tri	world/door/w21a_dr00.dar \
						world/door/w21a_dr01.dar

world		door/w21a_dr00.kms
world		door/w21a_dr01.kms


// オブジェクト
	//	ガンサイファー
#define		GUN_CYP

#include	"cypher.h"


// アイテム
itembox


//	パッドデモ
	//	PAL版
#ifdef MGS2_VMODE_PAL
rpd			w21a_paddemo_00_pal.rpd
	//	それ以外
#else
rpd			w21a_paddemo_00.rpd
#endif

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

//	落し穴
include		pit_effect.h

//	かもめ
include		kamome.h

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
