/*
	libutl.h
	各種ユーティリティライブラリヘッダ

	1999/08/16 K.Takabe
	$Id: libutl.h,v 1.1.1.3 2002/11/19 11:42:55 Yoshizawa1 Exp $

*/

#ifndef __LIBUTL_H__
#define __LIBUTL_H__


#include	"gameheader.h"

/* 99/10/27
   線形合同法ランダム関数 init_rnd(int seed) irnd() rnd()*/
#include        "rand.h"


/* 単位系変換 */
#define ONE_METER	1000.0f
#define ONE_CENTIMETER	(ONE_METER/100.0f)
#define ONE_MILLIMETER	(ONE_CENTIMETER/10.0f)
#define CVM2N(m)	((float)(m)*ONE_METER)
#define CVC2N(c)	((float)(c)*ONE_CENTIMETER)
#define CVMILLI2N(mm)	((float)(mm)*ONE_MILLIMETER)
#define CVMCM2N(m,c,mm)	((float)(m)*ONE_METER+(float)(c)*ONE_CENTIMETER+(float)(mm)*ONE_MILLIMETER)
#define CVCM2N(c,mm)	((float)(c)*ONE_CENTIMETER+(float)(mm)*ONE_MILLIMETER)

/* 重力加速度 */
extern float BP_P_GRAVITY;
#define P_GRAVITY (BP_P_GRAVITY)

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/

/* 各種構造体定義 */

/*----------------------------------------------------------------*/


/* 99/8/16 KanoKen
   vu0やfpuを使うためのインライン関数群の宣言 */

/* 内部ではこの形を用いているため、#defineを利用して変換 */
#define MATRIX FMATRIX
#define VECTOR FVECTOR

#include "vu0_pack.h"
#include "fpu_pack.h"
#include "vufpucmb.h"

#undef MATRIX
#undef VECTOR


/* 99/9/1 KanoKen
   パーティクル汎用制御ルーチン */
#include "particle.h"


/* 99/12/7 KanoKen
   モデルすり替えサブルーチン */
#include "objchange.h"


/* 99/11/30 KanoKen
   頂点アニメ系サブルーチン */
#include "vertex_animation.h"
#include "motion_animation.h"
#include "malti_matrix.h"
#include "partchg.h"


/* 00/02/25 T.Morita
   ボス・イベント用生体センサー */
extern void GM_VibrateSensor( CONTROL *ctrl, int *vib_time ) ;


/* 00/02/25 T.Morita
   lzsh 解凍ルーチン */
#include "utl_lzh.h"


/* 01/06/08 T.Morita
   エフェクトバウンドチェック ルーチン */
#include "utl_eft_bound.h"

/*----------------------------------------------------------------*/

/* プロトタイプ宣言読み込み */
#include	"libutl.x"
#include	"scratch.x"
#include	"makequat.x"
#include	"objcollision.x"
#include	"ctrlcollision.x"

/*----------------------------------------------------------------*/
	
#ifdef __cplusplus
};
#endif

#endif
