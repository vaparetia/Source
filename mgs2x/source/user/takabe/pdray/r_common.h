/*
	r_common.h
	量産型ＲＡＹ関連共通定義ヘッダ

	2001/04/16 K.Takabe
	$Id: r_common.h,v 1.1.1.3 2002/11/19 11:51:26 Yoshizawa1 Exp $

*/

#ifndef __R_COMMON_H__
#define __R_COMMON_H__


/* サーバー側から各ＲＡＹに対する要求コマンド */
enum {
	RAY_COMMAND_ATTACK1			= 0x00000001,	/* 場外からの攻撃（攻撃方法自動判断） */
	RAY_COMMAND_ATTACK2			= 0x00000002,	/* ステージ上に乗って攻撃（ＡＩ自動戦闘） */
	RAY_COMMAND_ATTACK3			= 0x00000004,	/*  */
	RAY_COMMAND_ATTACK4			= 0x00000008,	/*  */
	RAY_COMMAND_ATTACK5			= 0x00000010,	/*  */
	RAY_COMMAND_NULL			= 0x00000000,	/*  */
	RAY_COMMAND_ADJUST			= 0x00000100,	/* 立ち位置補正 */
	RAY_COMMAND_ROAR			= 0x00000200,	/* 叫び要求 */
	RAY_COMMAND_DRINK			= 0x00000400,	/* 水のみ要求 */
	RAY_COMMAND_QUICKSTART		= 0x40000000,	/* 起動開始時に既に攻撃ポジションに移動 */
	RAY_COMMAND_ACTIVE			= 0x80000000,	/* 起動開始要求 */
	/* デバッグ用 */
	RAY_COMMAND_DEBUG_A1		= 0x00100001,	/* デバッグ攻撃１ */
	RAY_COMMAND_DEBUG_A2		= 0x00100001,	/* デバッグ攻撃２ */
	RAY_COMMAND_DEBUG_A3		= 0x00100001,	/* デバッグ攻撃３ */
	RAY_COMMAND_DEBUG_A4		= 0x00100001,	/* デバッグ攻撃４ */
};

/* ゲーム状態コンディション関連 */
/* （サーバーが各ＲＡＹの動作状態を監視するのに使用） */
enum {
	GAME_CONDITION_WAIT,			/* 開始待ち */
	GAME_CONDITION_STANDBY,			/* ステージ外で待機 */
	GAME_CONDITION_INATTACK,		/* ステージ内で攻撃 */
	GAME_CONDITION_OUTATTACK,		/* ステージ外で攻撃 */
	GAME_CONDITION_OUTATTACK_A,		/* ステージ外で攻撃Ａ位置 */
	GAME_CONDITION_OUTATTACK_B,		/* ステージ外で攻撃Ｂ位置 */
	GAME_CONDITION_OUTATTACK_C,		/* ステージ外で攻撃Ｃ位置 */
	GAME_CONDITION_SUPPORTATTACK,	/* 他のＲＡＹとの複合攻撃 */
	GAME_CONDITION_ESCAPE,			/* 撤退 */
	GAME_CONDITION_BREAK,			/* 大破 */
};

/* ゲーム進行シグナル */
/* （各ＲＡＹからサーバーに対して送るのに使用） */
enum {
	GAME_SIGNAL_START_IN_ATTACK,	/* ステージ内攻撃開始通知 */
	GAME_SIGNAL_END_IN_ATTACK,		/* ステージ内攻撃終了通知 */
	GAME_SIGNAL_LOCKON_ENABLE,		/* カメラロックオン有効化通知 */
	GAME_SIGNAL_LOCKON_DISABLE,		/* カメラロックオン無効化通知 */
	GAME_SIGNAL_DEMAND_NEXT_PHASE,	/* 次フェーズ要求 */
	GAME_SIGNAL_DEMAND_NEXT_STEP,	/* 次ステップ要求 */
	GAME_SIGNAL_GET_CAM_DIFF_ANGLE,	/* カメラとＲＡＹの角度差 */
	GAME_SIGNAL_CHANGE_CAMERA,		/* カメラモード変更 */
	GAME_SIGNAL_NOTICE,				/* ＲＡＹからサーバーに対して通知 */
	GAME_SIGNAL_DAMAGE,				/* ダメージを受けたことを通知 */
};

enum {
	GAME_NOTICE_RAY_DAMAGE,			/* ステージに乗っているＲＡＹがダメージを受けた */
	GAME_NOTICE_DUMMY,				/* ダミー */
};

/* ＲＡＹ共通ステータス（） */
enum {
	CSTATUS_DAMAGE1		= 0x00000001,		/* ステージ内のＲＡＹがダメージを受けた */
	CSTATUS_DAMAGE2		= 0x00000002,		/* ステージ外のＲＡＹがダメージを受けた */
	CSTATUS_SDAMAGE1	= 0x00000004,		/* ステージ内のＲＡＹが足ダメージを受けた */
	CSTATUS_SDAMAGE2	= 0x00000008,		/* ステージ外のＲＡＹが足ダメージを受けた */
};



/* ---------------------------------------------------------------- */
#define NUMBER_TRI		(2151908)			/* "pdray_other" */
#define NUMBER_MODEL_R2	(7644705)			/* "pdray_number_a01_r" */
#define NUMBER_MODEL_L2	(7644699)			/* "pdray_number_a01_l" */
#define NUMBER_MODEL_R1	(7645729)			/* "pdray_number_a02_r" */
#define NUMBER_MODEL_L1	(7645723)			/* "pdray_number_a02_l" */
#define NUMBER_MODEL_R0	(7646753)			/* "pdray_number_a03_r" */
#define NUMBER_MODEL_L0	(7646747)			/* "pdray_number_a03_l" */
#define NUMBER_MODEL_R3	(7647777)			/* "pdray_number_a04_r" */
#define NUMBER_MODEL_L3	(7647771)			/* "pdray_number_a04_l" */
#define NUMBER_MODEL_R4	(7648801)			/* "pdray_number_a05_r" */
#define NUMBER_MODEL_L4	(7648795)			/* "pdray_number_a05_l" */
#define NUMBER_MODEL_R5	(8693281)			/* "pdray_number_b01_r" */
#define NUMBER_MODEL_L5	(8693275)			/* "pdray_number_b01_l" */
#define NUMBER_TEXTURE_02 (5951528)		/* "pdray_number_a01_alp_ovl" */
#define NUMBER_TEXTURE_01 (6017064)		/* "pdray_number_a02_alp_ovl" */
#define NUMBER_TEXTURE_00 (6082600)		/* "pdray_number_a03_alp_ovl" */
#define NUMBER_TEXTURE_03 (6148136)		/* "pdray_number_a04_alp_ovl" */
#define NUMBER_TEXTURE_04 (6213672)		/* "pdray_number_a05_alp_ovl" */
#define NUMBER_TEXTURE_05 (5951532)		/* "pdray_number_b01_alp_ovl" */
#define NUMBER_TEXTURE_06 (6017068)		/* "pdray_number_b02_alp_ovl" */
#define NUMBER_TEXTURE_07 (6082604)		/* "pdray_number_b03_alp_ovl" */
#define NUMBER_TEXTURE_08 (6148140)		/* "pdray_number_b04_alp_ovl" */
#define NUMBER_TEXTURE_09 (6213676)		/* "pdray_number_b05_alp_ovl" */
#define NUMBER_TEXTURE_10 (5951536)		/* "pdray_number_c01_alp_ovl" */
#define NUMBER_TEXTURE_11 (6017072)		/* "pdray_number_c02_alp_ovl" */
#define NUMBER_TEXTURE_12 (6082608)		/* "pdray_number_c03_alp_ovl" */
#define NUMBER_TEXTURE_13 (6148144)		/* "pdray_number_c04_alp_ovl" */
#define NUMBER_TEXTURE_14 (6213680)		/* "pdray_number_c05_alp_ovl" */
#define NUMBER_TEXTURE_15 (5951540)		/* "pdray_number_d01_alp_ovl" */
#define NUMBER_TEXTURE_16 (6017076)		/* "pdray_number_d02_alp_ovl" */
#define NUMBER_TEXTURE_17 (6082612)		/* "pdray_number_d03_alp_ovl" */
#define NUMBER_TEXTURE_18 (6148148)		/* "pdray_number_d04_alp_ovl" */
#define NUMBER_TEXTURE_19 (6213684)		/* "pdray_number_d05_alp_ovl" */
#define NUMBER_TEXTURE_20 (5951544)		/* "pdray_number_e01_alp_ovl" */
#define NUMBER_TEXTURE_21 (6017080)		/* "pdray_number_e02_alp_ovl" */
#define NUMBER_TEXTURE_22 (6082616)		/* "pdray_number_e03_alp_ovl" */
#define NUMBER_TEXTURE_23 (6148152)		/* "pdray_number_e04_alp_ovl" */
#define NUMBER_TEXTURE_24 (6213688)		/* "pdray_number_e05_alp_ovl" */

/* ---------------------------------------------------------------- */
	/*
		サーバーとＲＡＹの情報交換用構造体
	*/
typedef struct _ray_info {
	/* ＲＡＹからサーバーへ */
	int				ray_life ;			/* ＲＡＹ残りライフ */
	int				status ;			/* ステータス */
	int				condition ;			/* ＲＡＹのコンディション */
	int				n_water_tank ;		/* 水圧カッター残り使用回数 */
	FVECTOR			pos ;				/* 座標 */
	SVECTOR			rot ;				/* 方向 */
	int				adjust_rot ;		/* 補正方向 */

	/* サーバーからＲＡＹへ */
	int				command ;			/* ＲＡＹ行動コマンド */
	int				color_id ;			/* カラーＩＤ */
	FVECTOR			standby_pos ;		/* 待機状態立ち位置 */
} RAY_INFO ;


/* ---------------------------------------------------------------- */
/* 共通変数関連 */
extern int		RAYSERVER_CommonStatus ;
extern int		RAYSERVER_GameLevel ;			/* ゲームレベル（1,2,3,4,5） */
extern int		RAYSERVER_GameLevelOffset ;		/* ゲームレベルオフセット（-2,-1,0,1,2） */
extern int		RAYSERVER_GameDownLevel ;		/* ゲーム難易度低下レベル（2,1,0,0,0） */
extern int		RAYSERVER_GameUpLevel ;			/* ゲーム難易度上昇レベル（0,0,0,1,2） */
/* ---------------------------------------------------------------- */

extern int RAYSERVER_SetGameSignal( int signal, int param );
extern int RAYSERVER_AddRayControl( void *ray_work, RAY_INFO *info, CONTROL *ctrl );
extern int RAYSERVER_GetRayMovePosition( void *ray_work, FVECTOR *pos );
extern int RAYSERVER_GetRayJob( void *ray_work );
extern int RAYSERVER_GetRayCommand( void *ray_work );
extern void RAYSERVER_GetPlayerSpeed( FVECTOR *vec );
extern void RAYSERVER_GetStingerVelocity( FVECTOR *vec );
extern int RAYSERVER_GetNumberModel( DG_OBJS *body );
extern void RAYSERVER_FreeNumberModel( int handle );

/* ---------------------------------------------------------------- */
#if 0
	/*
		デバッグ機能用
		キーボードマニア専用ＵＳＢキーボード読み込みルーチン
	*/
#ifdef DEBUG_MODE
static int GetUsbPad( void )
{
#define SIF_SYSREG_USBKBD	9		// SIFのSYSREGを使用
	extern unsigned int sceSifGetSreg(int);
	int		data ;

#ifdef PSX2
	data = sceSifGetSreg( SIF_SYSREG_USBKBD );
#else
	data = 0 ;
#endif
	return ( data );
}
/* キーボードマニア専用ＵＳＢキーボード用マッピング */
#define USB_KBM_ST		(0x00400000)
#define USB_KBM_SL		(0x00004000)
#define USB_KBM_C1		(0x00000001)
#define USB_KBM_CD1		(0x00000002)
#define USB_KBM_D1		(0x00000004)
#define USB_KBM_DE1		(0x00000008)
#define USB_KBM_E1		(0x00000010)
#define USB_KBM_F1		(0x00000020)
#define USB_KBM_FG1		(0x00000040)
#define USB_KBM_G1		(0x00000100)
#define USB_KBM_GA1		(0x00000200)
#define USB_KBM_A1		(0x00000400)
#define USB_KBM_AB1		(0x00000800)
#define USB_KBM_B1		(0x00001000)
#define USB_KBM_C2		(0x00002000)
#define USB_KBM_CD2		(0x00010000)
#define USB_KBM_D2		(0x00020000)
#define USB_KBM_DE2		(0x00040000)
#define USB_KBM_E2		(0x00080000)
#define USB_KBM_F2		(0x00100000)
#define USB_KBM_FG2		(0x00200000)
#define USB_KBM_G2		(0x01000000)
#define USB_KBM_GA2		(0x02000000)
#define USB_KBM_A2		(0x04000000)
#define USB_KBM_AB2		(0x08000000)
#define USB_KBM_B2		(0x10000000)
#define USB_KBM_HU		(0x20000000)
#define USB_KBM_HD		(0x40000000)
#endif

#endif



#endif




