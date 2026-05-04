/*******************************************************************************
 * vr_wall_scar - vr_wall_scar.h
 * ＶＲ壁・床弾痕
 * 2002/03/28 S.Yamashita
 * $Id: vr_wall_scar.h,v 1.1.1.3 2002/11/19 11:51:56 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_WALL_SCAR__
#define __INC_VR_WALL_SCAR__

/*******************************************************************************
 * include
 */

#include "../../morita/include/libdg_x.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define WALLSCR_PRIM_FLAG (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_CULLPOLY|DG_PRIM2_ALPHA)
#define WLLSCR_TEXTURE_1  "vr2_dankon1_add_alp"		/* テクスチャ１ */
#define WLLSCR_TEXTURE_2  "vr2_dankon2_add_alp"		/* テクスチャ２ */
#define WLLSCR_ALPHA_1    (64)						/* アルファ１ */
#define WLLSCR_ALPHA_2    (64)						/* アルファ２ */
#define WLLSCR_SIZE       (125)						/* 弾痕サイズ */
#define WLLSCR_MAX_UNIT   (10)						/* デフォルト最大弾痕数 */
#define WLLSCR_MAX_DIFF   (5)						/* 少しずつフェードアウトする弾痕数 */
#define PUSH_UP           (3.0f)					/* 浮かす距離 */

#define SCAR_NODATA       (0x0001)					/* 初期状態 */
#define UNIT_PHASEMASK    (0x000000ff)				/* フェーズマスク */
#define UNIT_PHASE1       (0x00000001)				/* フェーズ１ */
#define UNIT_PHASE2       (0x00000002)				/* フェーズ２ */
#define UNIT_PHASE3       (0x00000003)				/* フェーズ３ */
#define UNIT_PHASE1_COUNT (15)						/* フェーズ１ フレーム数 */
#define UNIT_PHASE2_COUNT (30)						/* フェーズ２ フレーム数 */
#define UNIT_NOPHASE2     (0x00000100)				/* フェーズ２へ移行しない */

#define WLLSCR_ROTATE     (M_PI / UNIT_PHASE1_COUNT * 1)
#define WLLSCR_NEAR       (125)

/*******************************************************************************
 * functions
 */

void NewVRWallScar(FMATRIX *dir, HZX_SEG *seg, HZX_FLR *flr);

/*******************************************************************************
 */

#endif	/* __INC_VR_WALL_SCAR__ */
