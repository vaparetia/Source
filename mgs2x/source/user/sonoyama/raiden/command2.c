//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   command2.c
   プレイヤー／シナリオリンク関数（その２）
   
   2000/10/06 M.Sonoyama
   $Id: command2.c,v 1.1.1.3 2002/11/19 11:50:54 Yoshizawa1 Exp $
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
#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

#include "BP_Misc.h"

/* プレイヤー状態取得 */
enum {
	PFLAG_NORMAL = 			0x0000,
	PFLAG_INTRUDE =			0x0008,		/* イントルード */
	PFLAG_SUBJECT =			0x0010,		/* 主観 */
	PFLAG_CAUTION =			0x0020,		/* 張り付き */
	PFLAG_ELUDE =			0x0040,		/* エルード */
	PFLAG_LOCKER =			0x0080,		/* ロッカー */
	PFLAG_LADDER = 			0x0100,		/* はしご */
	PFLAG_ENEMY_PULL =		0x0200,		/* 死体引き摺り */
	PFLAG_WATER =			0x0400,		/* 水中 */
	PFLAG_BEHIND =			0x0800,		/* ビハインドカメラ中 */
	PFLAG_DIAZEPAM =		0x1000,		/* ジアゼパム効果中 */
	PFLAG_PEEP_R =			0x2000,		/* 覗き込み右最大 */
	PFLAG_PEEP_L =			0x4000,		/* 覗き込み左最大 */
	PFLAG_BLOOD =			0x8000,		/* 流血 */
	PFLAG_WALK =			0x00010000,	/* 歩き */
	PFLAG_RUN =				0x00020000,	/* 走り */
	PFLAG_BOX =				0x00040000,	/* ダンボール */
	PFLAG_WATER_SURFACE =	0x00080000,	/* 水中モードでカメラは水面上 */
	PFLAG_NO_BREATH =		0x00100000,	/* 無呼吸状態 */
	PFLAG_SNAKE =			0x00200000,	/* スネーク */
	PFLAG_ROLLING =			0x00400000,	/* ローリング */
	PFLAG_DAMAGED =			0x00800000,	/* ダメージＡＮＤダウン中 */
	PFLAG_ELUDING =			0x01000000,	/* エルード静止ＯＲ左右移動 */
	PFLAG_SQUAT =			0x02000000,	/* しゃがみ中 */
	PFLAG_GROUND =			0x04000000,	/* ほふく中ＯＲダウン中 */
	PFLAG_ENEMY_HANG =		0x08000000,	/* 首締め中 */
	PFLAG_NIKITA_ALIVE =	0x10000000,	/* ニキータ飛行中 */
	PFLAG_ELUDE_FALL =		0x20000000,	/* エルード落下中 */
} ;

#define	PFLAG_NORMAL_MASK	(PFLAG_DIAZEPAM|PFLAG_PEEP_R|PFLAG_PEEP_L|\
							 PFLAG_BLOOD|PFLAG_SNAKE|PFLAG_NIKITA_ALIVE)

int		NewGetPlayerStateFlag( void )
{
	int			flag ;
	PlayerWork	*work ;

	if ( GM_PlayerWork == NULL ) return 0 ;
	work = GM_PlayerWork ;
	flag = 0 ;
	if ( GM_CheckPlayerStatus( PLAYER_INTRUDE ) ) flag |= PFLAG_INTRUDE ;
	if ( GM_CheckPlayerStatus( PLAYER_WATCH ) ) flag |= PFLAG_SUBJECT ;	
	if ( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) flag |= PFLAG_CAUTION ;
	if ( GM_CheckPlayerStatus( PLAYER_BEYOND ) ) flag |= PFLAG_ELUDE ;	
	if ( GM_CheckPlayerStatus( PLAYER_LOCKER ) ) flag |= PFLAG_LOCKER ;
	if ( GM_CheckPlayerStatus( PLAYER_LADDER ) ) flag |= PFLAG_LADDER ;
	if ( GM_CheckPlayerStatus( PLAYER_IN_THE_WATER ) ) flag |= PFLAG_WATER ;
	if ( GM_CheckPlayerStatus( PLAYER_BEHIND ) ) flag |= PFLAG_BEHIND ;
	if ( GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) flag |= PFLAG_BLOOD ;
	if ( GM_DiazepamCount > 0 ) flag |= PFLAG_DIAZEPAM ;
	if ( PL_Flag( FLAG_BEHIND_PEEP_R_MAX ) ) flag |= PFLAG_PEEP_R ;
	if ( PL_Flag( FLAG_BEHIND_PEEP_L_MAX ) ) flag |= PFLAG_PEEP_L ;
	if ( GM_CheckPlayerStatus( PLAYER_WALK ) ) flag |= PFLAG_WALK ;
	if ( GM_CheckPlayerStatus( PLAYER_DASH ) ) flag |= PFLAG_RUN ;
	if ( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ) flag |= PFLAG_BOX ;
	if ( GM_CheckPlayerStatus( PLAYER_WATER_SURFACE ) ) flag |= PFLAG_WATER_SURFACE ;
	if ( GM_CheckPlayerStatus( PLAYER_NO_BREATH ) ) flag |= PFLAG_NO_BREATH ;
	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) flag |= PFLAG_SNAKE ;
	if ( GM_CheckPlayerStatus( PLAYER_ROLLING ) ) flag |= PFLAG_ROLLING ;
	if ( GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ) flag |= PFLAG_DAMAGED ;
	if ( GM_PlayerWork->act_name == ELUDE_STILL_MODE ) flag |= PFLAG_ELUDING ;
	if ( GM_CheckPlayerStatus( PLAYER_SQUAT ) ) flag |= PFLAG_SQUAT ;
	if ( GM_CheckPlayerStatus( PLAYER_GROUND ) ) flag |= PFLAG_GROUND ;
	if ( GM_CheckPlayerStatus( PLAYER_ENEMY_PULL ) ) flag |= PFLAG_ENEMY_PULL ;
	if ( GM_CheckPlayerStatus( PLAYER_ENEMY_HANG ) || 
		 work->act_name == 8741186 ) flag |= PFLAG_ENEMY_HANG ;
	if ( GM_WeaponAlive & WP_ALIVE_NIKITA ) flag |= PFLAG_NIKITA_ALIVE ;
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ) flag |= PFLAG_ELUDE_FALL ;

	if ( GCL_GetOption( 'd' ) != NULL ) return flag ;

	flag &= ~PFLAG_NORMAL_MASK ;
	return flag ;
}

static	TARGET			Target ;
static 	POWER_TARGET	Power ;
extern int PL_LastKillTime ;

/* プレイヤー瞬殺 */
int		NewKillPlayer( void )
{
	int			name ;
	TARGET			*t ;
	POWER_TARGET	*p ;
	FVECTOR		force = { 0.0F, 0.0F, 300.0F } ;

	if ( GV_Time == PL_LastKillTime ) return 0 ;
	PL_LastKillTime = GV_Time ;
	if ( GM_PlayerControl == NULL || GM_PlayerTarget == NULL ) return 0 ;
	name = GCL_GetOptionValue( 'n', 0 ) ;
	if ( name != GM_PlayerControl->evt.chara ) return 0 ;

	t = &Target ;
	p = &Power ;
	GM_SetTarget( t, TARGET_OFFENSE, 0, PLAYER_SIDE, 
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	if ( GCL_GetOption( 'b' ) != NULL ) {
		/* 吹っ飛び */
#ifdef DEBUG_MODE
		GM_SetTargetWeaponType( t, WP_BLOW | WP_NOGUARD | WP_NORECOVER ) ;
		GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, GM_VitalityMax, &force ) ;
//		GM_SetTargetWeaponType( t, WP_BLOW ) ;
//		GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, 25, &force ) ;
#else
		GM_SetTargetWeaponType( t, WP_BLOW | WP_NOGUARD | WP_NORECOVER ) ;
		GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, GM_VitalityMax, &force ) ;
#endif
	} else {
		/* のけぞり */
#ifdef DEBUG_MODE
//		GM_SetTargetWeaponType( t, WP_USP | WP_NOGUARD | WP_NORECOVER ) ;
//		GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, GM_VitalityMax, &DG_ZeroVector ) ;		
		GM_SetTargetWeaponType( t, WP_USP ) ;
		GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, 1, &DG_ZeroVector ) ;		
#else
		GM_SetTargetWeaponType( t, WP_USP | WP_NOGUARD | WP_NORECOVER ) ;
		GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, GM_VitalityMax, &DG_ZeroVector ) ;		
#endif
	}
	GM_PutTarget( t ) ;
	GM_TargetSetDirectAttack( t, GM_PlayerTarget ) ;
	return 1 ;
}

#if 0
/* プログラム呼び即死 */
int		PL_KillPlayer( void )
{
	TARGET			*t ;
	POWER_TARGET	*p ;

	if ( GM_PlayerControl == NULL || GM_PlayerTarget == NULL ) return 0 ;
	t = &Target ;
	p = &Power ;
	GM_SetTarget( t, TARGET_OFFENSE, 0, PLAYER_SIDE, 
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_USP ) ;
	GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, 255, &DG_ZeroVector ) ;
	GM_PutTarget( t ) ;
	GM_TargetSetDirectAttack( t, GM_PlayerTarget ) ;
	return 1 ;
}
#endif

/* ジャンプ重力の設定 */
int		PL_COM_SetJumpGravity( void )
{
	PL_JumpGravity = ( float )GCL_GetOptionValue( 'g', 0 ) ;

   if ( BP_IsPAL()==TRUE )
	   PL_JumpGravity *= 1.20F ;

   return 0 ;
}

/* 最後に立っていた床の高さをゲット */
int		PL_COM_FloorHeight( void )
{
	ASSERT( GM_PlayerWork != NULL ) ;
	if ( GCL_GetOption( 'c' ) == NULL ) {
		return ( int )PL_PlayerFloorHeight ;
	} else {
		return ( int )GM_PlayerControl->levels[ 0 ] ;
	}
}

/* シナリオからライフ値操作 */
int		PL_COM_VitalityAdjust( void )
{
	int		type, mode ;

	mode = GCL_GetOptionValue( 'm', 0 ) ;
	type = GCL_GetOptionValue( 't', 0 ) ;
	if ( mode == 0 ) {
		if ( type == 0 ) {	/* 死ぬこともある */
			GM_VitalityAdjust += GCL_GetOptionValue( 'v', 0 ) ;
		} else {			/* 死なない */
			GM_VitalityAdjustNoDead += GCL_GetOptionValue( 'v', 0 ) ;
		}
	} else {
		/* 直接いじる */
		GM_PlayerWork->power.vital += GCL_GetOptionValue( 'v', 0 ) ;
		if ( type != 0 && GM_PlayerWork->power.vital <= 0 ) {
			GM_PlayerWork->power.vital = 1 ;
		}
	}
	return 0 ;
}

/* 死体上で匍匐しているときは立たせる */
int		PL_COM_OnCorpseStand( void )
{
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ON_CORPSE_STAND ) ;
	return 0 ;
}

/* しゃがみ張付きＯＮＬＹ指定 */
int		PL_COM_CautionSquatOnly( void )
{
	GM_SetPlayerStatusEX( I64(0), PLAYER2_SQUAT_ONLY ) ;
	return 0 ;
}
