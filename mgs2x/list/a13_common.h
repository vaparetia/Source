//
//	a13_common.h（アナザーミッション用）
//
//	written by S.Mukaide 2002.Jan
//
//	$Id: a13_common.h,v 1.12 2002/07/18 05:01:15 usr03379 Exp $

//	ステージ名表記用
#define WN_W13A
// アナザーミッション明示
#define		WN_ANOTHER

block .use r_vr_s
block .nocache
block .cache

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"

//	オルタナティブ追加エフェクト
#include	"alt_effect.h"

//	かもめ
//include		kamome.h	// modified by Okuta 2001/05/10


// 敵兵タイプ
	//	警備兵
#define		ENE_P_GOL
	//	通常アタッカー
#define		ENE_ATTACKER


#ifndef	TALES_E_STAGE	//	テイルズＥ以外

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

#endif


//	影用モデル
#define		ENE_SHADOW

//	キャラクタ
#include	"ene_plant.h"

// 強制モーション
mtn		rai_a13a.mls
mtn		sna_a13a.mls

//	アタリ
#ifndef	TALES_E_STAGE	//	テイルズＥ以外
	hazard		a13a.hzx
#endif

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
#ifndef	TALES_E_STAGE	//	テイルズＥ以外
	itembox
#endif

// デバッグ用
// 小さいサイズのテクスチャはまとめておく
debug		mogi/floor_sound.kms

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

// リネーム
