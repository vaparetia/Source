/*******************************************************************************
 * vr_def.h
 * ＶＲ系定義
 * 2002/05/29 S.Yamashita
 * $Id: vr_def.h,v 1.1.1.3 2002/11/19 11:51:43 Yoshizawa1 Exp $
 */

#ifndef __INC_VR_DEF__
#define __INC_VR_DEF__

/******************************************************************************
 * defines
 */

/* 体験版ステージ */
#define VRDEF_TRIAL_1 (VRM_SNK_SNK_03_SNA)	/* 体験版１面 */
#define VRDEF_TRIAL_2 (VRM_WPN_HGN_01_SNA)	/* 体験版２面 */
#define VRDEF_TRIAL_3 (VRM_SNK_ELM_08_RAI)	/* 体験版３面 */
#define VRDEF_TRIAL_4 (VRM_WPN_HFB_03_RAI)	/* 体験版４面 */
#define VRDEF_TRIAL_5 (VRM_VRT_05_SNA    )	/* 体験版５面 */

/* スクリーン */
extern int BP_FRAMES_PER_SEC();
#define VRDEF_FRAMERATE  (BP_FRAMES_PER_SEC())			/* フレームレート */

#define VRDEF_SCREEN_X   (512)			/* スクリーン幅 */
#define VRDEF_SCREEN_Y   (384)			/* スクリーン高さ */

/* カーソル */
#define VRDEF_CUR_R (160)
#define VRDEF_CUR_G (180)
#define VRDEF_CUR_B (170)
#define VRDEF_CUR_A (128)
#define VRDEF_CUR_W (14)
#define VRDEF_CUR_H (8)

/* フォントの色 */

/* 体験版で使用 */
#define VRDEF_HIDE       (0x00000000)	/* 非表示時の文字の色ＡＢＧＲ */

#define VRDEF_ACTIVE      (0x80808080)	/* アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_ACTIVE_R    (128)			/* アクティブ時の文字の色Ｒ */
#define VRDEF_ACTIVE_G    (128)			/* アクティブ時の文字の色Ｇ */
#define VRDEF_ACTIVE_B    (128)			/* アクティブ時の文字の色Ｂ */
#define VRDEF_ACTIVE_A    (128)			/* アクティブ時の文字の色Ａ */
#define VRDEF_INACTIVE    (0x40404040)	/* 非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_INACTIVE_R  ( 64)			/* 非アクティブ時の文字の色Ｒ */
#define VRDEF_INACTIVE_G  ( 64)			/* 非アクティブ時の文字の色Ｇ */
#define VRDEF_INACTIVE_B  ( 64)			/* 非アクティブ時の文字の色Ｂ */
#define VRDEF_INACTIVE_A  ( 64)			/* 非アクティブ時の文字の色Ａ */
#define VRDEF_INACTIVE2   (0x60606060)	/* 非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_INACTIVE2_R ( 96)			/* 非アクティブ時の文字の色Ｒ */
#define VRDEF_INACTIVE2_G ( 96)			/* 非アクティブ時の文字の色Ｇ */
#define VRDEF_INACTIVE2_B ( 96)			/* 非アクティブ時の文字の色Ｂ */
#define VRDEF_INACTIVE2_A ( 96)			/* 非アクティブ時の文字の色Ａ */

#define VRDEF_A_1STPLAC   (0x80004080)	/* アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_A_1STPLAC_R (128)			/* アクティブ時の文字の色Ｒ */
#define VRDEF_A_1STPLAC_G ( 64)			/* アクティブ時の文字の色Ｇ */
#define VRDEF_A_1STPLAC_B (  0)			/* アクティブ時の文字の色Ｂ */
#define VRDEF_A_1STPLAC_A (128)			/* アクティブ時の文字の色Ａ */
#define VRDEF_I_1STPLAC   (0x50002850)	/* 非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_I_1STPLAC_R ( 80)			/* 非アクティブ時の文字の色Ｒ */
#define VRDEF_I_1STPLAC_G ( 40)			/* 非アクティブ時の文字の色Ｇ */
#define VRDEF_I_1STPLAC_B (  0)			/* 非アクティブ時の文字の色Ｂ */
#define VRDEF_I_1STPLAC_A ( 80)			/* 非アクティブ時の文字の色Ａ */

#define VRDEF_A_CLEARED   (0x80107080)	/* アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_A_CLEARED_R (128)			/* アクティブ時の文字の色Ｒ */
#define VRDEF_A_CLEARED_G (112)			/* アクティブ時の文字の色Ｇ */
#define VRDEF_A_CLEARED_B ( 16)			/* アクティブ時の文字の色Ｂ */
#define VRDEF_A_CLEARED_A (128)			/* アクティブ時の文字の色Ａ */
#define VRDEF_I_CLEARED   (0x500a4650)	/* 非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_I_CLEARED_R ( 80)			/* 非アクティブ時の文字の色Ｒ */
#define VRDEF_I_CLEARED_G ( 70)			/* 非アクティブ時の文字の色Ｇ */
#define VRDEF_I_CLEARED_B ( 10)			/* 非アクティブ時の文字の色Ｂ */
#define VRDEF_I_CLEARED_A ( 80)			/* 非アクティブ時の文字の色Ａ */

#define VRDEF_A_UNPABLE   (0x80404040)	/* アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_A_UNPABLE_R ( 64)			/* アクティブ時の文字の色Ｒ */
#define VRDEF_A_UNPABLE_G ( 64)			/* アクティブ時の文字の色Ｇ */
#define VRDEF_A_UNPABLE_B ( 64)			/* アクティブ時の文字の色Ｂ */
#define VRDEF_A_UNPABLE_A (128)			/* アクティブ時の文字の色Ａ */
#define VRDEF_I_UNPABLE   (0x50282828)	/* 非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_I_UNPABLE_R ( 40)			/* 非アクティブ時の文字の色Ｒ */
#define VRDEF_I_UNPABLE_G ( 40)			/* 非アクティブ時の文字の色Ｇ */
#define VRDEF_I_UNPABLE_B ( 40)			/* 非アクティブ時の文字の色Ｂ */
#define VRDEF_I_UNPABLE_A ( 80)			/* 非アクティブ時の文字の色Ａ */

#define VRDEF_WINTEXT    (0x60808080)	/* ＶＲウィンドウの文字の色ＡＢＧＲ */



/* ＳＵＢＳＴＡＮＣＥで使用 */
/*
VRDEF_1ST_PLC_A   ( 96  48   0 128)
VRDEF_2ND_3RD_A   ( 96  84  12 128)
VRDEF_CLEARED_A   (110 125 118 128)
VRDEF_PLAYABL_A   ( 80  90  85 128)
VRDEF_UNPLAYA_A   ( 40  45  43 128)

VRDEF_1ST_PLC_I   ( 72  36   0  64)
VRDEF_2ND_3RD_I   ( 72  63   9  64)
VRDEF_CLEARED_I   (110 125 118  64)
VRDEF_PLAYABL_I   ( 60  68  64  64)
VRDEF_UNPLAYA_I   ( 30  34  32  64)
*/
#define VRDEF_1ST_PLC_A (0x80003060)	/* １位			アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_2ND_3RD_A (0x800c5460)	/* ２，３位		アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_CLEARED_A (0x80767d6e)	/* クリア		アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_PLAYABL_A (0x80555a50)	/* プレイ可		アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_UNPLAYA_A (0x802b2d28)	/* プレイ不可	アクティブ時の文字の色ＡＢＧＲ */

#define VRDEF_1ST_PLC_I (0x40002448)	/* １位			非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_2ND_3RD_I (0x40093f48)	/* ２，３位		非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_CLEARED_I (0x40767d6e)	/* クリア		非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_PLAYABL_I (0x4040443c)	/* プレイ可		非アクティブ時の文字の色ＡＢＧＲ */
#define VRDEF_UNPLAYA_I (0x4020221e)	/* プレイ不可	非アクティブ時の文字の色ＡＢＧＲ */

/*******************************************************************************
 */

#endif	/* __INC_VR_DEF__ */
