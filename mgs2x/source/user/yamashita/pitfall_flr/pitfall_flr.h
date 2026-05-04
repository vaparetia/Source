/*******************************************************************************
 * pitfall_flr - pitfall_flr.h
 * 落とし穴の床ヘッダー
 * 2002/03/13 S.Yamashita
 * $Id: pitfall_flr.h,v 1.1.1.3 2002/11/19 11:51:53 Yoshizawa1 Exp $
 */

#ifndef __INC_PITFALL_FLR__
#define __INC_PITFALL_FLR__

/*******************************************************************************
 * definitions and typedefs and structures
 */

/* 順序変更禁止！！！ */
enum
{
	BRK_FLR_ModeNone = 0,		/* 制御停止                                    */
	BRK_FLR_ModeReset,			/* 初期状態                                    */
	BRK_FLR_ModeShakeStart,		/* 消滅開始      value: 消滅までのフレーム数   */
	BRK_FLR_ModeShake,			/* (使用禁止)                                  */
	BRK_FLR_ModeSwingStart,		/* (使用禁止)                                  */
	BRK_FLR_ModeSwing,			/* (使用禁止)                                  */
	BRK_FLR_ModeFallStart,		/* (使用禁止)                                  */
	BRK_FLR_ModeFall,			/* (使用禁止)                                  */
	BRK_FLR_ModeLaydownStart,	/* (使用禁止)                                  */
	BRK_FLR_ModeLaydown,		/* (使用禁止)                                  */

	BRK_FLR_ModeReapperStart,	/* 再表示開始    value: 再表示までのフレーム数 */
	BRK_FLR_ModeReapper,		/* (使用禁止)                                  */

	BRK_FLR_SetSeMode,			/* (使用禁止)                                  */
	BRK_FLR_SetFallType,		/* (使用禁止)                                  */
};

/*******************************************************************************
 * functions
 */

void *NewPitfallFloor(FVECTOR *pos, SVECTOR *rot);

/*******************************************************************************
 */

#endif	/* __INC_PITFALL_FLR__ */
