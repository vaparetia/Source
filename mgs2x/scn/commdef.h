/*
	commdef.h
	    inf ファイルで用いるキャラ名の StrCode 定義ヘッダ

		MT_SetMotionSeTable で用いるキャラID

	2001/02/27 S.Hirano
	$Id: commdef.h,v 1.8 2002/05/09 13:07:48 usr01475 Exp $
*/
// ファイルが二重呼びされたときの対処
#ifndef COMMDEF_H
#define COMMDEF_H	1

#define SNAKE_INF_NAME		2677990		/* GV_StrCode("スネーク") */
#define OTACON_INF_NAME		4896105		/* GV_StrCode("オタコン") */
#define OLGA_INF_NAME		16138912	/* GV_StrCode("オルガ") */
#define OCELOT_INF_NAME		7666070		/* GV_StrCode("オセロット") */
#define MEILING_INF_NAME	10198823	/* GV_StrCode("メイリン") */
#define SCOTT_INF_NAME		9107709		/* GV_StrCode("スコット") */
#define EMMA_INF_NAME		5584254		/* GV_StrCode("エマ") */
#define PLIS_INF_NAME		6401406		/* GV_StrCode("プリスキン") */
#define ENESOL_INF_NAME		6633980		/* GV_StrCode("敵兵") */
#define ROSE_INF_NAME		2328243		/* GV_StrCode("ローズ") */
#define RAI_INF_NAME		10163495	/* GV_StrCode("ライデン") */
#define SEAL_INF_NAME		1808488		/* GV_StrCode("シールズ") */
#define SCOM_INF_NAME		15096511	/* GV_StrCode("シールズ隊長") */
#define PETE_INF_NAME		1765162		/* GV_StrCode("ピーター") */
#define PRES_INF_NAME		14061600	/* GV_StrCode("大統領") */
#define HOS_INF_NAME		6474817		/* GV_StrCode("人質") */
#define COL_INF_NAME		6599668		/* GV_StrCode("大佐") */
#define AMES_INF_NAME		10184878	/* GV_StrCode("エイムズ") */
#define PAROT_INF_NAME		16068308	/* GV_StrCode("オウム") */
#define SOL_INF_NAME		16444913	/* GV_StrCode("ソリダス") */
#define SEL_INF_NAME		701789		/* GV_StrCode("セルゲイ") */
#define VMP_INF_NAME		7063499		/* GV_StrCode("ヴァンプ") */
#define FORT_INF_NAME		11843538	/* GV_StrCode("フォーチュン") */
#define FAT_INF_NAME		1883539		/* GV_StrCode("ファットマン") */
#define MARI_INF_NAME		6040060		/* GV_StrCode("海兵") */

/*----------------------------------------------------------------
  MT_SetMotionSeTable のキャラID
----------------------------------------------------------------*/
#define MT_SETABLE_PLAYER	(0)		/* プレイヤー */
#define MT_SETABLE_ENE_0	(1)		/* 敵兵０ */
#define MT_SETABLE_ENE_1	(2)		/* 敵兵１ */
#define MT_SETABLE_ENE_2	(3)		/* 敵兵２ */
#define MT_SETABLE_ENE_3	(4)		/* 敵兵３ */

#define MT_SETABLE_GLL_0	(5)		// ゴルルゴン
#define MT_SETABLE_GLL_1	(6)		// メカゲノラ
#define MT_SETABLE_GLL_2	(7)		// ゲノラ
#define MT_SETABLE_GLL_3	(8)		// 怪獣予備

#define MT_SETABLE_NPC_EMMA	(32)	/* NPCエマ */
#define MT_SETABLE_NPC_SNA	(33)	/* NPCスネーク */

#define MT_SETABLE_ORGA		(34)	/* オルガ */
#define MT_SETABLE_FORT		(35)	/* フォーチュン */
#define MT_SETABLE_FAT		(36)	/* ファットマン */
#define MT_SETABLE_VAMP		(37)	/* バンプ */
#define MT_SETABLE_SOL		(38)	/* ソリダス */
#define MT_SETABLE_OCELOT	(39)	/* オセロット */

#else
#endif
