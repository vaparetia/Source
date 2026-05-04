/*
   raiden.h
   ライデン／ヘッダファイル

   1999/07/07 M.Sonoyama
   $Id: raiden.h,v 1.1.1.3 2002/11/19 11:51:02 Yoshizawa1 Exp $			
*/

#ifndef __raiden_h__
#define	__raiden_h__

#ifdef DEBUG_MODE
//#define	BLOOD_EVERY
//#define	PAD_PRESSURE_CHECK 
//#define	PAD_MEMORY
//#define	NO_SUBJECT
//#define	NEAR_SHOOT_OK
#endif

#if 0
#if ( defined( HIGHRESO_FFI ) || defined( HIGHRESO_I ) )
#define	Y_ADJ	(2)
#else
#define	Y_ADJ	(1)
#endif
#endif
#define	Y_ADJ	(1)

/* システム用設定 */
#define	RAIDEN_NAME	(10163495) /* GV_StrCode( "ライデン" ) */
#define	MOTION_NAME	(4595192)  /* GV_StrCode( "raiden" ) */
#define	MODEL_NAME	GV_StrCode( "raiden" )
#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE|\
					 DG_FLAG_IRREACTION)
//#define	OBJECT_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)

#define	CAMERA_TURN

#define	N_UNITS		(21)
#define	HEAD_UNIT	(HUMAN21_ATAMA)
#define	RIGHT_HAND_UNIT	(HUMAN21_MIGI_TE)

/* トラップ */
#define	HIDDEN_TRAP	(12819940) 	/* "hidden" */
#define	LOCKER_TRAP	(2308593)  	/* "locker" */
#define	DARK_TRAP	(3379883)	/* "dark" */
#define	ENHIDDEN_TRAP	(13033828)	/* "enhidden" */
#define	NO_CROUCH_TRAP	(13300053)	/* "no_crouch" */
#define	WALLTO_TRAP		(4702179)	/* "wallto" */
#define	WALLTO_HIDDEN_TRAP	(12767202)	/* "wallto_hidden" */

/* はりつき可能状態検索マージン */
#define	LIE_ANGLE_MARGIN	(128)

/* はりつき終了ウェイト */
#define	END_CAUTION_WAIT	(4)

/* 張り付きめり込み防止距離 */
/* 後述の「覗き込むために止まる位置」より小であること */
#define	CAUTION_SINK		(440.0F)	

/* 主観可視待ち時間 */
#define	SUBJECT_WAIT_TIME_IN		(6)
#define	SUBJECT_WAIT_TIME_OUT		(2)

/* ビハインドに入るまでの待ち時間 */
#define	BEHIND_IN_DELAY		(DIRECT_TICK(24))

/* ビハインド覗き待ち時間 */
#define	BEHIND_PEEP_IN_TIME	(16)	/* メニュークイックと調整すること */

/* 覗き込むために止まる位置 */
#define	BEHIND_PEEP_START_LEN	(450.0F)

/* はりつきで飛び出せる位置 */
#define	BEHIND_ATTACK_ENABLE_LEN (650.0F)

/* ビハインド飛び出し位置 */
//#define	BEHIND_ATTACK_LEN	(750.0F)
//#define	BEHIND_ATTACK_LEN_SQ	(600.0F)
#define	BEHIND_ATTACK_LEN		(550.0F)
#define	BEHIND_ATTACK_LEN_SQ	(600.0F)

/* 武器ボタン押し判定 */
//#define	NEW_HG			/* ハンドガン新操作 */

#define	PL_PAD_WEAPON_TH		(24)
#define	PL_PAD_WEAPON_TH2		(60)	/* マシンガン用 */
#define	PL_PAD_WEAPON_TH3		(180)	/* 新ハンドガン用 */

#define		WALK_PATTERN_A

/* 覗き込みレベル */
#define	PEEP_LEVEL		(750.0F)
//#define	PEEP_LEVEL_Y	(450.0F)
#define	PEEP_LEVEL_Y_RAI	(700.0F)
#define	PEEP_LEVEL_Y_SNA	(450.0F)

/* ホーミング */
#define	HOMING_LEN		(HomingLen)
#define	HOMING_RANGE	(HomingRange)

#define	LOOK_LEN	(4500.0F)
#define	LOOK_RANGE	(640)

/* 静止回転用 */
#define	TURN_MARGIN			(256)
#define	TURN_COUNT_VALUE	(1024)

/* 重力 */

extern float BP_AdjustTick3(float);
#define	GRAVITY		(BP_AdjustTick3(96.0F))

/* イントルード設定 */
#define	INTRUDE_CAMERA_HEIGHT_STILL	(150)
#define	INTRUDE_CAMERA_HEIGHT_MOVE	(250)

/* 匍匐時カメラ高さ調整 */
#define	GROUND_CAMERA_HEIGHT_STILL	(250)

/* ぽたぽた血時間 */
#define	BLOOD_DROP_TIME 	(DIRECT_TICK(20))

/* 無敵時間 */
#define	INVINCIBLE_TIME		(DIRECT_TICK(60))

/* 出血状態からライフ回復カウンタ */
#define	BLOOD_DROP_RECOVER_TIME		(DIRECT_TICK(48))

/* 出血でライフ減少カウンタ */
#define	BLOOD_DROP_DECREASE_TIME	(DIRECT_TICK(180))

/* ビハインド覗き方向 */
enum {
    BEHIND_LEFT = 0,
    BEHIND_RIGHT,
} ;

#define	SubjectHoming( work ) 	Homing( work )

/*-------------------------------------------------------------------------*/

/* 外部変数宣言 */
extern	FVECTOR	ArmBodyPosition ;

/* 外部関数宣言 */    
extern	void	*NewWeaponManager( void ) ;
extern 	void	*NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int ) ;
extern 	void	*NewBloodDrops( FVECTOR *, int, int, int ) ;
extern	MAR_HEADER	*MT_GetMotionArchives( int ) ;
extern	void	*NewBodySplash( DG_OBJS *, CONTROL *, int ) ;
extern	void	*NewFootSplash( OBJECT *, CONTROL * ) ;
extern	void	*NewShadow( DG_OBJ *, DG_OBJ *, CONTROL *, FMATRIX *, int * ) ;
extern	void	*NewDropShadow( OBJECT *, CONTROL *, FMATRIX *, int * ) ;
extern	void	*NewBodyShadow( DG_OBJS * ) ;
extern	void	*NewRaidenArm( OBJECT *, int *, int *, int *, int * ) ;

extern	void	*NewWeaponChangeTest( void ) ;
extern	void	*NewItemChangeTest( void ) ;

extern	void	*NewGageDaemon( void ) ;

extern	void 	*NewFootPrint( OBJECT *body, CONTROL *control, int *flag, int time ) ;

extern	void	*NewScrCrack( int, int ) ;
extern	void	*NewWeaponChange1( int ) ;
extern	void	*NewWeaponChange2( int ) ;
extern	void	*NewItemChange1( int ) ;
extern	void	*NewItemChange2( int ) ;

extern	void 	*NewIntrudeInDuct2D_2( int mode , int camera_chanl ) ;

extern	void	*NewDebugCamera( int ) ;
extern	void	*NewDebugCameraForce( int ) ;

#ifdef DEBUG_MODE
extern	void	GM_DumpControlList( void ) ;
extern	void	*NewPadRecorder( int ) ;

extern	int		PlayerDebugBehindType ;
#endif

/* その他 */

/*-----------------------------------------------------------*/

/* ワーク定義ファイルをインクルード */
#include	"pl_define.h"
#include	"pl_work.h"

#endif
