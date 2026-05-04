//
//	a15_common.h（アナザーミッション用）
//
//	written by Y.Matsuhana 2002.Jan
//
//	$Id: a15_common.h,v 1.9 2002/08/03 13:17:27 usr03682 Exp $

//	ステージ名表記用
#define WN_W15A
// アナザーミッション明示
#define		WN_ANOTHER

block .use r_vr_s
block .nocache
block .cache

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"
//include		item_box_tank.h
//include		gover.h
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
	//	天狗兵とウォークマン兵のモーションはいらない
#define 	ENE_MAR '-DDEL_TNG=DUMMY -DDEL_WALKMAN=DUMMY'

//	影用モデル
#define		ENE_SHADOW

//	キャラクタ
#include	"ene_plant.h"

// 強制モーション
mtn		rai_a15a.mls
mtn		sna_a15a.mls

//死体捨て用強制モーション
mtn			corpse_w15a.mls

//	海

/*
pack_all	oil.tri \
		debug/takabe/tex/oil_alp_ovl_mod0222.bmp \
		debug/takabe/tex/sky_day_add_alp_ovl_mod1120.bmp
*/

//	アタリ
hazard		a15a.hzx

//	ダミー
goods		null/null.kms

//	ゲームオーバー
//pack_all	gmov.tri 2D/game_over/dekisi_alp_ovl.bmp
#include "goverpic.h"

//	海
//world		sea


//	ドア
pack_all door.tri \
		world/door/w15a_dr00.dar \
		world/door/w15a_dr00x.dar
world/door	w15a_dr00.kms w15a_dr00x.kms

// オブジェクト
#define		NORM_CYP
#include	"cypher.h"

// 解体Ｃ４セット(一番最後に描画してもらうためにzからはじめる)
pack_all	z_object.tri \
			goods/c4_kaitai_a \
			goods/c4_kaitai_b

goods		c4_kaitai_a
goods		c4_kaitai_b


//meca		cypher/cyp.kms

// アイテム
itembox

//	パッドデモ
rpd			w15a_paddemo_n00.rpd

// デバッグ用
// 小さいサイズのテクスチャはまとめておく


//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib

// リネーム

