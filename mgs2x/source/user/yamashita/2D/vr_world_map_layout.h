/******************************************************************************
 * vr_pause - vr_world_map_layout.h
 * ＶＲ全体マップ表示３Ｄのにぎやかし  *NewVRWorldMapLayout
 * 2002/07/08 S.Yamashita
 * $Id: vr_world_map_layout.h,v 1.1.1.3 2002/11/19 11:51:50 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_WORLD_MAP_LAYOUT__
#define __INC_VR_WORLD_MAP_LAYOUT__

/******************************************************************************
 * defines
 */

#define VRWML_STRCODE (14973458)	/* vr_world_map_layout */

/* シグナルタイプ */
enum
{
	VRWML_CLOSE = VRWML_STRCODE,	/* 終了 */
};

/*******************************************************************************
 * functions
 */

void *NewVRWorldMapLayout(Pos *pos, int *position, char mode, int *bomb_stage);

/*******************************************************************************
 */

#endif	/* __INC_VR_WORLD_MAP_LAYOUT__ */
