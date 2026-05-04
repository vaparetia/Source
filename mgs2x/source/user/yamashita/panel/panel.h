/*******************************************************************************
 * panel - panel.h
 * ＶＲパネル
 * 2002/04/08 S.Yamashita
 * $Id: panel.h,v 1.1.1.3 2002/11/19 11:51:52 Yoshizawa1 Exp $
 */

#ifndef __INC_PANEL__
#define __INC_PANEL__

/*******************************************************************************
 * include
 */

#include "gameheader.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define F_TEXTURE     "vr2_wetpanel_alp"	/* テクスチャ */
#define POLYSIZE      (1000)				/* ポリゴンサイズ */
#define PUSH_UP       (5)					/* 浮かせ度 */
#define MARGIN        (10)					/* マージン */
#define ALPHA         (128)					/* アルファ値 */
#define ALPHA_FADEIN  (ALPHA / 2)			/* フェードイン時、１フレームのアルファ値の増加量 */
#define ALPHA_FADEOUT (ALPHA / 32)			/* フェードアウト時、１フレームのアルファ値の減少量 */
#define HIDE_PSTATUS  (PLAYER_ROLLING)		/* 表示をＯＦＦにするプレイヤー状態 */

/*******************************************************************************
 * macros
 */

#define FTOI12(_f) (DG_FTOI(((float)(_f) * 4096.0f)))
#define RND(n)     (((BP_PS2_rand() >> 16) * n) >> 15)

/*******************************************************************************
 * functions
 */

void *NewVRFloorPanel(FVECTOR *pos);

/*******************************************************************************
 */

#endif	/* __INC_PANEL__ */
