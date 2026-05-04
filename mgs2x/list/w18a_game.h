//
//	w18a.src
//
//	written by Y.Matsuhana 2000.May
//
//	$Id: w18a_game.h,v 1.27 2002/09/11 12:01:07 usr03682 Exp $

#define		WN_W18A		1

#define E_WATER

// 各ステージ共通要素
#include		"p_common.h"
#include		"common.h"
#include		"c_effect.h"

// 敵兵関連define
// 敵兵タイプ
#define ENE_P_GOL		// w18aは都市迷彩攻撃兵
#define ENE_ATTACKER	// 攻撃兵
#define ENE_HIGH_TECH	// ハイテク兵は人質イベント後
#define ENE_SHIELD		// 盾兵

// 特殊兵のモーション設定
#define ENE_MAR '-DDEL_SHOTGUN=DUMMY -DDEL_TNG=DUMMY'

// キャラクタ
#include	"ene_plant.h"

// 強制モーション
mtn		rai_w18a.mls
mtn		rai_swim.mls
mtn		rai_hatch.mls

// ワールド
// 水面
pack_all	water.tri world/w18a/w18a_water/w18a_water_alp_ovl.bmp
anmtex		w18ahakei.rat \
			effect/w18_hakei/w18_hakei00_alp.bmp \
			effect/w18_hakei/w18_hakei01_alp.bmp \
			effect/w18_hakei/w18_hakei02_alp.bmp \
			effect/w18_hakei/w18_hakei03_alp.bmp \
			effect/w18_hakei/w18_hakei04_alp.bmp \
			effect/w18_hakei/w18_hakei05_alp.bmp \
			effect/w18_hakei/w18_hakei06_alp.bmp \
			effect/w18_hakei/w18_hakei07_alp.bmp \
			effect/w18_hakei/w18_hakei08_alp.bmp \
			effect/w18_hakei/w18_hakei09_alp.bmp \
			effect/w18_hakei/w18_hakei10_alp.bmp \
			effect/w18_hakei/w18_hakei11_alp.bmp \
			effect/w18_hakei/w18_hakei12_alp.bmp \
			effect/w18_hakei/w18_hakei13_alp.bmp \
			effect/w18_hakei/w18_hakei14_alp.bmp \
			effect/w18_hakei/w18_hakei15_alp.bmp \
			effect/w18_hakei/w18_hakei16_alp.bmp \
			effect/w18_hakei/w18_hakei17_alp.bmp \
			effect/w18_hakei/w18_hakei18_alp.bmp \
			effect/w18_hakei/w18_hakei19_alp.bmp \
			effect/w18_hakei/w18_hakei20_alp.bmp \
			effect/w18_hakei/w18_hakei21_alp.bmp \
			effect/w18_hakei/w18_hakei22_alp.bmp \
			effect/w18_hakei/w18_hakei23_alp.bmp \
			effect/w18_hakei/w18_hakei24_alp.bmp \
			effect/w18_hakei/w18_hakei25_alp.bmp \
			effect/w18_hakei/w18_hakei26_alp.bmp \
			effect/w18_hakei/w18_hakei27_alp.bmp \
			effect/w18_hakei/w18_hakei28_alp.bmp \
			effect/w18_hakei/w18_hakei29_alp.bmp \
			effect/w18_hakei/w18_hakei30_alp.bmp \
			effect/w18_hakei/w18_hakei31_alp.bmp 

option makexti6 -l 1024 -f PAL8
world		w18a
option makexti6

world		door/w18a_dr00.kms
world		door/w18a_dr01.kms
world		door/w18a_dr02.kms
lt2			w18a.lt2
hazard		w18a.hzx

cvd		human/sna_skl/sna_skl3.cvd vn

// ゲームオーバー画面
#include	"goverpic.h"

//死体捨て用強制モーション
mtn			corpse_w18a.mls

// デバッグ用
goods		fly/fly_cm.kms
goods		c4_kaitai_a
goods		null

// アイテム
itembox

//	振動
vib		etc/drop_sea.vib
vib		program/explosion.vib
// リネーム

