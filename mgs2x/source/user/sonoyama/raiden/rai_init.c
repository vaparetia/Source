//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   rai_init.c 
   ライデン／常駐関数・変数

   1999/07/07 M.Sonoyama
   $Id: rai_init.c,v 1.1.1.3 2002/11/19 11:51:01 Yoshizawa1 Exp $			
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"g_other.h"
#include	"raiden.h"

#include "BP_BuildDefines.h"

/* デバッグ用 */
int		PlayerDebugMenuStatus = 0 ;
#ifdef DEBUG_MODE
int		PlayerDebugBehindType ;
#endif

int		GM_PlayerDebugMode ;

int             PL_SubjectMove   = 0    ; /* 主観移動モード     */
int             PL_SubjectToggle = 0    ; /* 主観移動モード     */
float           PL_SubjectAngle  = 1.6f ; /* 主観移動モード角度 */
int             PL_ObjectFlag    = 0    ; /* オブジェクトフラグ */
int             PL_PadType       = 0    ; /* パッドタイプ       */
int             PL_PadTypePrev   = -1   ; /* パッドタイプ(前回) */


/* プレイヤーワーク */
PlayerWork	*GM_PlayerWork ;	/* ワーク */
CONTROL		*GM_PlayerControl ;	/* コントロール */
OBJECT		*GM_PlayerBody ;	/* オブジェクト */
OBJECT		*GM_PlayerArmBody ;	/* 腕オブジェクト */
OBJECT		*GM_PlayerWeaponBody ;	/* 武器オブジェクト */
OBJECT		*GM_PlayerSubWeaponBody ;/* 武器オブジェクト（主観） */
OBJECT		*GM_PlayerEquipBody ;	/* 装備品オブジェクト（ダンボール以外） */
OBJECT		*GM_PlayerBoxBody ;
int			GM_PlayerWeaponModel ;	/* 武器のモデル名 */

TARGET		*GM_PlayerTarget ;	/* ターゲット */
FVECTOR		GM_PlayerPosition ;	/* 位置 */
FVECTOR		GM_PlayerFirstPosition ;	/* ステージ初期位置 */
int			GM_PlayerAddress ;	/* ゾーン情報 */
long64		_GM_PlayerStatus ;	/* ステータス */
long64		_GM_PlayerStatus2 ;	/* ステータス（拡張） */
short		GM_PlayerCautionDirs[ 4 ] ;	/* 張り付き時移動方向 */
GM_CameraSet	*GM_PlayerSubjectCamera[ 4 ] ;	/* プレイヤー主観カメラ */

long64		GM_PlayerDamagedWeaponType ;	

PL_PluginSet	PL_PluginList ;	/* プラグインリスト */
PL_PollingSet	PL_PollingList ;/* ポーリングリスト */

PL_DeadMode		PL_DeadModeList ;	/* 特殊ゲームオーバーアクションリンク */

PL_TARGETCALLBACKFUNC	PL_TargetCallbackFunc ;	/* ターゲットコールバック内で呼ばれる 
												   特殊チェック関数 */

PL_DAMAGEFUNC	PL_DamageFunc ;		/* プラグイン用ダメージ関数 */
PL_OPERATE		PL_CheckAttackFunc ;	/* 武器独自の攻撃チェック */

PL_ACTION		PL_ReturnModes[ 3 ] ;	/* 特殊モードから戻るべきモードを指定 */

int				PL_ReturnModeMotionArc ;

/* プレイヤーのモーション再生スピード比率 */
float	PL_MotionVelocity = 1.0F ;

/* プレイヤーのいるマップ */
int		GM_PlayerMap ;

/* プレイヤーのいる画面（対戦モード用） */
int		GM_CurrentCameraChanl ;

/* プレイヤーの床高さ（設置時のみ更新） */
float	PL_PlayerFloorHeight ;

/* command2.cの初期か忘れ */
int		PL_LastKillTime = -1 ;

/* 敵関係 */
FVECTOR		GM_PlayerFindPos ;	/* 敵が発見する為の基本ポイント */
u_int		GM_PlayerFindObj ;	/* 敵がチェックする体の部位（ビットフラグ） */

/* 主観時カメラ制限 */
int		GM_SubjectVMax[] = { 640, -800 } ;
int		GM_SubjectHMax[] = { 1024, -1024 } ;
int		GM_SubjectVStep = 16 ;
int		GM_SubjectHStep = 16 ;

int		GM_SubjectVMaxTmp[] = { 0, 0 } ;
int		GM_SubjectHMaxTmp[] = { 0, 0 } ;
int		GM_SubjectVStepTmp = -1 ;
int		GM_SubjectHStepTmp = -1 ;
float	GM_SubjectAngleTmp = -1.0F ;

FVECTOR	PL_SubjectCameraShift ;

/* 武器関係 */
int		GM_WeaponChanged = 0 ;
int		GM_ItemChanged = 0 ;

signed char	GM_WeaponPrevChangedScn = -1 ;
signed char	GM_ItemPrevChangedScn = -1 ;

int		GM_MagazineMax = 0 ;
int		GM_Magazine = 0 ;

/* 装備している武器のＳＴＲＣＯＤＥ（素手は１） */
int		PL_EquipWeaponCode = 1 ;

/* 外部要因によるライフ調整値 */
int		GM_VitalityAdjust = 0 ;
int		GM_VitalityAdjustNoDead = 0 ;

/* 設置武器管理 */
u_char	GM_C4_Blast = 0 ;
u_char	GM_C4_BlastAll = 0 ;
u_char	GM_N_C4Bombs = 0 ;
u_char	GM_N_Claymores = 0 ;
u_char	GM_N_Books = 0 ;
u_char	GM_N_Grenades = 0 ;

/* ジアゼパムカウンタ */
int		GM_DiazepamCount = 0 ;

/* ニキータ関係 */
int			GM_NikitaAlive[ 1 ] ;	/* 1個にしちゃいました(2001/03/19) */
short		GM_NikitaLife ;			/* ニキータ爆発までのカウンター */
short		GM_NikitaLifeMax ;		/* ニキータ爆発までのカウンター最大 */
FVECTOR		GM_NikitaPosition[ 1 ] ;
void		*GM_NikitaMapConnection ;	/* ニキータとプレイヤーをつなぐマップ接合 */

/* スティンガー関係 */
FVECTOR		GM_StingerPosition ;

/* ミサイル系移動範囲設定 */
FVECTOR		GM_MissileBoundMin ;
FVECTOR		GM_MissileBoundMax ;

/* ＲＧＢ－６ */
int			GM_N_RGB6S ;
FVECTOR		GM_RGB6Position[ GM_CURRENT_RGB6_MAX ] ;

/* 特殊武器の存在フラグ */
int			GM_WeaponAlive ;
/* 武器発射トリガー */
/* 弾が発射された武器番号が入る */
signed char	GM_WeaponFire ;

/* 再生パッドデモバージョン */
int			GM_PadDemoVersion ;

/* スポットライト影チェック用 */
int								GC_ProjectionSpot_HitLights ;
GCT_ProjectionSpot_SpotParam	*GC_ProjectionSpot_SnakeCheckList ;

/* 影チェック位置 */
/* 最大４つまで */
int			GM_N_PlayerShadowPos ;
FVECTOR		GM_PlayerShadowPos[ GM_MAX_PLAYER_SHADOW_POS ] ;

/* デジカメチェック面数 */
int			PL_DGCam_N_CheckSquares = 0 ;
int			PL_DGCam_N_CheckCharas = 0 ;
int			PL_DGCam_ShutterProc = 0 ;

/* ＵＳＰライト */
int			PL_USP_LightOn = 0 ;

/* グレネード制御フラグ */
int			PL_Bul_Grenade_Flag = 0 ;

/* カレントボムリスト */
GM_BOMB		GM_BombList ;

/* 移動床関数 */
PL_MOVE_FLOOR_FUNC 	PL_MoveFloorFunc = NULL ;

/* 落下弾関数 */
PL_FALLBULLET_FUNC	PL_FallBulletFunc = NULL ;
PL_BLASTWATER_FUNC	PL_BlastWaterFunc = NULL ;

/* エルード妨害物チェック */
PL_ELUDEDISTURB_FUNC	PL_EludeDisturbFunc = NULL ;

/* 弾水面処理関数（水飛沫） */
PL_BULLETSPLASH_FUNC  	PL_BulletSplashFunc = NULL ;

/* 変更不可武器・アイテム */
long64		PL_MenuNoUseWeapon = I64(0) ;
long64		PL_MenuNoUseItem   = I64(0) ;
int			PL_MenuNoUseWeaponType = 0 ;
int			PL_MenuNoUseItemType = 0 ;

long64		PL_MenuNoUseWeaponScn = I64(0) ;
long64		PL_MenuNoUseItemScn   = I64(0) ;

/* ジャンプ重力制御 */
float		PL_JumpGravity ;

/* ゴル兵装 */
DG_OBJS		*PL_GolUniformObjs = NULL ;
DG_OBJS		*PL_GolCapObjs = NULL ;

/* 映り込み用髪の毛 */
DG_OBJS		*PL_ShadowHairObjs = NULL ;

/* プラグインメッセージプレチェック */
u_char		PL_PluginPreCheck = 0 ;

/* モーションブラー用DG_OBJS */
DG_OBJS		*PL_ActionBlurObjs ;
void		*PL_ActionBlurParent ;

/* ロックオンターゲット */
TARGET		*PL_LockonTarget ;

/* 現在の武器 */
int	PL_GetPlayerWeapon( void )
{
	if ( GM_PlayerWork == NULL ) return WP_None ;
//    ASSERT( GM_PlayerWork != NULL ) ;
    return GM_PlayerWork->weapon ;
}

/* 現在のアイテム */
int	PL_GetPlayerItem( void )
{
	if ( GM_PlayerWork == NULL ) return IT_None ;
//    ASSERT( GM_PlayerWork != NULL ) ;
    return GM_PlayerWork->item ;
}

#if 0
int	Ply_GetPlayerWeapon( void )
{
	return PL_GetPlayerWeapon() ;
}

int	Ply_GetPlayerItem( void )
{
	return PL_GetPlayerItem() ;
}
#endif

/* セット武器の初期化 */
void	PL_InitWeaponSystem( void )
{
	extern	void	*PL_C4Manager( void ) ;	/* bullet/bul_c4.c */

	GM_WeaponChanged = 0 ;
	GM_WeaponPrevChangedScn = -1 ;

	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;
	GM_PlayerWeaponModel = 0 ;

	GM_WeaponFire = -1 ;
    GM_C4_Blast = 0 ;
    GM_C4_BlastAll = 0 ;
    GM_N_C4Bombs = 0 ;
    GM_N_Claymores = 0 ;
	GM_N_Grenades = 0 ;
	GM_N_Books = 0 ;
	GM_N_RGB6S = 0 ;
	GM_NikitaLifeMax = DIRECT_TICK( 1200 ) ;
	GM_NikitaLife = 0 ;
	GM_NikitaAlive[ 0 ] = NKT_NONE ;

	GM_InitMapConnect() ;
	GM_NikitaMapConnection 
	  = NewMapConnectProg( 7245446,// GV_StrCode( "ニキータ用マップ接合" ),
						  GM_PlayerMap, GM_PlayerMap ) ;
	
	GM_WeaponAlive = 0 ;

	PL_USP_LightOn = 0 ;
	PL_Bul_Grenade_Flag = 0 ;
	/* Ｃ４管理起動 */
	PL_C4Manager() ;
	/* ボムリスト管理起動 */
	PL_BombListControl() ;

	PL_LockonTarget = NULL ;

	PL_BulletSplashFunc = NULL ;	/* 弾水飛沫 */

	GV_SetVec3( &GM_MissileBoundMin, -1000000.0F, -1000000.0F, -1000000.0F ) ;
	GV_SetVec3( &GM_MissileBoundMax, 1000000.0F, 1000000.0F, 1000000.0F ) ;

	/* 装備武器コードの初期化 */
	/* デモに反映させるのに必要 */
	/* シナリオで変えられた場合は知りません */
	{
		static	int		weapons[] = { WP_m92, WP_Usp, WP_Socom } ;
		static	int		bodys[] = { MDL_M92_SUB, MDL_USP_SUB, MDL_SOCOM_SUB } ;
		int				i ;

		PL_EquipWeaponCode = 1 ;		
		for ( i = 0; i < 3; i ++ ) {
			if ( GM_Weapon == weapons[ i ] ) {
				PL_EquipWeaponCode = bodys[ i ] ;
				break ;
			}
		}
		if ( PL_EquipWeaponCode == MDL_SOCOM_SUB ) {
			if ( GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ) {
				PL_EquipWeaponCode = MDL_SOCOM_SP_SUB ;
			}
		} else if ( PL_EquipWeaponCode == MDL_USP_SUB ) {
			if ( GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ) {
				PL_EquipWeaponCode = MDL_USP_SP_SUB ;
			}
		}
printf( "start weapon code %d\n", PL_EquipWeaponCode ) ;
	}
}

void	PL_InitEquipSystem( void )
{
	GM_PlayerEquipBody = NULL ;
	GM_PlayerBoxBody = NULL ;
	GM_ItemChanged = 0 ;
	GM_ItemPrevChangedScn = -1 ;

	PL_DGCam_N_CheckSquares = 0 ;
	PL_DGCam_N_CheckCharas = 0 ;
	PL_DGCam_ShutterProc = 0 ;

	PL_GolUniformObjs = NULL ;
	PL_GolCapObjs = NULL ;
	PL_ShadowHairObjs = NULL ;
}

/* 各種変数の初期化 */
void	PL_InitVariable( void )
{
	int		cont ;

	cont = 0 ;
	GM_InitPlayerStatus() ;
    GM_SetPlayerStatus( PLAYER_NORMAL | cont ) ;

	/* 有効アイテム配列をセット */
	if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
printf( "------------------------ tanker!!!\n" ) ;
		GM_SetCurrentItemSet( 0 ) ;
	} else {
printf( "------------------------ plant!!!\n" ) ;
		GM_SetCurrentItemSet( 1 ) ;
	}

	GC_ProjectionSpot_HitLights = 0 ; /* 侵入スポットライト数 */
    /* プラグインリスト初期化 */
    PL_PluginList.next = NULL ;
	PL_PollingList.next = NULL ;
	PL_DeadModeList.next = NULL ;
	PL_PluginPreCheck = 0 ;

	PL_TargetCallbackFunc = NULL ;
	PL_DamageFunc = NULL ;
	PL_CheckAttackFunc = NULL ;
	PL_ReturnModes[ 0 ] = NULL ;
	PL_ReturnModes[ 1 ] = NULL ;
	PL_ReturnModes[ 2 ] = NULL ;
	PL_ReturnModeMotionArc = -1 ;

	PL_InitWeaponSystem() ;
	PL_InitEquipSystem() ;
	PL_MenuNoUseWeapon = I64(0) ;
	PL_MenuNoUseItem   = I64(0) ;
	PL_MenuNoUseWeaponType = 0 ;
	PL_MenuNoUseItemType = 0 ;

	PL_MenuNoUseWeaponScn = I64(0) ;
	PL_MenuNoUseItemScn   = I64(0) ;

	PL_MotionVelocity = 1.0F ;
	PL_LastKillTime = -1 ;

	PL_PadTypePrev   = -1 ;/* パッド設定を行うための機構 */

	if ( GM_WeaponPrev < 0 || GM_WeaponPrev >= MAX_WEAPONS ) GM_WeaponPrev = WP_None ;
	if ( GM_ItemPrev < 0 || GM_ItemPrev >= MAX_ITEMS ) GM_ItemPrev = IT_None ;

	/* 関数ポインタ初期化 */
	PL_MoveFloorFunc = NULL ;
	PL_FallBulletFunc = NULL ;
	PL_BlastWaterFunc = NULL ;

	PL_EludeDisturbFunc = NULL ;

	DG_COPY_VEC( &PL_SubjectCameraShift, &DG_ZeroVector ) ;

	GM_PlayerWork = NULL ;
	GM_PlayerControl = NULL ;
	GM_PlayerBody = NULL ;
	GM_PlayerTarget = NULL ;
	GM_PlayerAddress = GM_INVALID_ADDR;

	GM_SubjectVMax[ 0 ] = 640 ;
	GM_SubjectVMax[ 1 ] = -800 ;
	GM_SubjectHMax[ 0 ] = 1024 ;
	GM_SubjectHMax[ 1 ] = -1024 ;
	GM_SubjectVStep = 16 ;
	GM_SubjectHStep = 16 ;

	GM_SubjectVMaxTmp[ 0 ] = 0 ;
	GM_SubjectVMaxTmp[ 1 ] = 0 ;
	GM_SubjectHMaxTmp[ 0 ] = 0 ;
	GM_SubjectHMaxTmp[ 1 ] = 0 ;

	GM_SubjectVStepTmp = -1 ;
	GM_SubjectHStepTmp = -1 ;
	GM_SubjectAngleTmp = -1.0F ;

	PL_ActionBlurObjs = NULL ;
	PL_ActionBlurParent = NULL ;

	GM_PlayerDamagedWeaponType = I64(0) ;

	/* ゲージ起動 */
    NewGageDaemon() ;


	{
		extern void 	*GM_SunGlassesManager( void ) ;
		GM_SunGlassesManager() ;
	}

#if BP_ENABLE_DEBUG_CAMERA()
   /* デバッグカメラ */

   NewDebugCamera( 0 ) ;
   NewDebugCameraForce( 0 ) ;
   NewDebugCamera( 1 ) ;
#endif

#ifdef DEBUG_MODE
	PlayerDebugBehindType = 0 ;
#endif
}

/* ステージ間で保持すべき状態をセーブ */
void	PL_SaveStatus( void )
{
	if ( GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) {
		GM_PlayerStateFlag |= PL_START_STATE_BLOOD ;
	}
}

/* 同じくロード */
void	PL_LoadStatus( void ) 
{
	short	flag ;

	flag = GM_PlayerStateFlag ;
	if ( flag & PL_START_STATE_BLOOD ) {
		GM_SetPlayerStatus( PLAYER_BLOOD_DROP ) ;
		GM_PlayerStateFlag &= ~PL_START_STATE_BLOOD ;
	}
}

/* 移動床関数の実行 */
int		PL_ExecMoveFloorFunc( FVECTOR *npos, HZX_GROUP_ID hzx_id, FVECTOR *pos, 
							  HZX_EVT *evt, int name )
{	
	int		( *func )( FVECTOR *, HZX_GROUP_ID, FVECTOR *, HZX_EVT *, int ) ;
	int		c ;

	if ( PL_MoveFloorFunc == NULL ) return 0 ;
	func = PL_MoveFloorFunc ;
	c = ( *func )( npos, hzx_id, pos, evt, name ) ;
	return c ;
}

/*----------------------------------------------------------------*/

/* パッドが生きているかチェック */
int				PL_PadEnable( void )
{
	if ( GM_PlayerWork == NULL ) return 0 ;
	if ( ( GM_CheckPlayerStatus( PLAYER_PAD_OFF ) ||
		  ( GV_PadData[ GM_PlayerWork->chanl ].flag & GV_PAD_RELEASE ) ) &&
		!GM_CheckGameStatus( STATE_PAD_DEMO ) ) return 0 ;
	return 1 ;
}

/* 攻撃の無効状態チェック */
/* 無効のとき１を返す */
int				PL_AttackDisable( void )
{
	if ( GM_PlayerWork == NULL ) return 0 ;
	/* ロード中は無効 */
	if ( GM_LoadRequest != 0 ) return 1 ;
	/* パッドデモ中は有効 */
	if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) return 0 ;
	/* ゲームオーバー処理、デモ中は無効 */
	if ( GM_IsGameOver() ||
		 GM_CheckGameStatus( STATE_PLAY_DEMO | STATE_GAMEOVER ) ) {
		return 1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

/* 関節も全部非表示＆表示 */
static	void	SetVisibleFlag( DG_OBJS *objs, int chanl, int mode )
{
	int				n ;
	DG_OBJ			*obj ;

	n = objs->n_models ;
	obj = objs->objs ;
	while( -- n >= 0 ) {
		if ( !mode ) obj->flag |= ( DG_FLAG_INVISIBLE0 << chanl ) ;
		else		 obj->flag &= ~( DG_FLAG_INVISIBLE0 << chanl ) ;
		obj ++ ;
	}
}

void	PL_VisibleObjsAll( DG_OBJS *objs, int chanl ) 
{
	SetVisibleFlag( objs, chanl, 1 ) ;
}

void	PL_InvisibleObjsAll( DG_OBJS *objs, int chanl ) 
{
	SetVisibleFlag( objs, chanl, 0 ) ;
}

/*----------------------------------------------------------------*/



