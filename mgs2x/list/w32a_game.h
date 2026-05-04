//
//	w32a_game.h
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w32a_game.h,v 1.11 2001/07/12 02:27:18 usr03875 Exp $



#include "w32_common_game.h"

// キャラクタ
	//	特殊敵兵


//include		ene_plant_w32a.h
#define ENE_P_GOL
#define	ENE_MAR '-DDEL_WALKMAN=DUMMY -DDEL_SHIELD=DUMMY -DDEL_SHOTGUN=DUMMY -DDEL_TNG=DUMMY \
	-DDEL_HIGH=DUMMY -DDEL_RADIO=DUMMY -DDEL_EROTICA=DUMMY -DDEL_BOX=DUMMY -DDEL_GRD=DUMMY \
	-DDEL_NEAR_ATTACK=DUMMY -DDEL_LOCKER=DUMMY -DDEL_HANG=DUMMY -DDEL_CARRY=DUMMY \
	-DDEL_TOILET=DUMMY -DDEL_SHOUBEN=DUMMY '
#include	"ene_plant.h"


// オブジェクト
	//	サイファー

#define		 SNIPE_CYP
#include	"cypher.h"

//meca		gun_cypher/gcyp.kms


// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム

