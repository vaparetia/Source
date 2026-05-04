/*******************************************************************************
 * marker - marker.h
 * ＶＲマーカー
 * 2002/03/26 S.Yamashita
 * $Id: marker.h,v 1.1.1.3 2002/11/19 11:51:50 Yoshizawa1 Exp $
 */

#ifndef __INC_MARKER__
#define __INC_MARKER__

/*******************************************************************************
 * include
 */

#include "gameheader.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define ALPHA_1      (16)							/* テクスチャ１のアルファ値 */
#define ALPHA_2      (24)							/* テクスチャ２のアルファ値 */
#define SPEED        (15)							/* スピードパラメータ */
#define HIDE_PSTATUS (PLAYER_WATCH|PLAYER_INTRUDE)	/* 表示をＯＦＦにするプレイヤー状態 */
#define HIDE_FLRFLAG (HZX_FLOOR_NO_BLOOD)			/* 表示をＯＦＦにする床フラグ */

#define F_TEXTURE_1  "floor_marker2_add_alp"		/* 床マーカーテクスチャ１ */
#define F_TEXTURE_2  "floor_marker1_add_alp"		/* 床マーカーテクスチャ２ */
#define POLYSIZE     (1000)							/* 床マーカーポリゴンサイズ */
#define START_DIST   (0)							/* 床マーカー初期距離 */

#define W_TEXTURE_1   "wall_marker2_add_alp"		/* 壁マーカーテクスチャ１ */
#define W_TEXTURE_2   "wall_marker1_add_alp"		/* 壁マーカーテクスチャ２ */
#define POLY_WIDTH    (1000)						/* 壁マーカーポリゴン幅 */
#define POLY_HEIGHT   (2000)						/* 壁マーカーポリゴン高さ */
#define SEARCH_DIST   (500)							/* 壁マーカー壁検索距離 */
#define Y_SEARCH      (-200)						/* 壁マーカー壁検索高さ(腰からの相対位置) */
#define Y_POS         (1000)						/* 壁マーカー表示高さ(足元からの相対位置) */
#define ALPHA_FADE    (15)							/* 壁マーカーアルファ値のフェードフレーム数 */
#define ADJ_SHIFT     (6)							/* シフト値 */

/*******************************************************************************
 * functions
 */

void *NewVRFloorMarker(CONTROL *control, float scale);
void *NewVRFloorMarker2(FVECTOR *mov, float scale);
void *NewVRWallMarker(CONTROL *control);

/*******************************************************************************
 */

#endif	/* __INC_MARKER__ */
