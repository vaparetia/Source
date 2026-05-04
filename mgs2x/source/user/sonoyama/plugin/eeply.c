//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   eeply.c
   手繋ぎライデン

   2001/01/29 M.Sonoyama
   $Id: eeply.c,v 1.1.1.3 2002/11/19 11:50:47 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"
#include	"../raiden/motion.h"
#include	"../../morita/emma/include/emma.mh"
#include	"../../morita/emma/include/emma_com.h"


/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/

static	PL_PollingSet	PollingSet ;
static	int				Motion ;

static	int				EE_NextMotion ;
static	int				EE_Trigger ;
static	FVECTOR			EE_Pos ;

static	int				GraspMode = 0 ;
//static  int                             VibTime   = 0 ;

/*---------------------------------------------------------------*/

enum {
	EEidle			= 0 ,
	EEwalk,
	EErun,
	EEsquat,
	EEcomeon,
} ;

/*---------------------------------------------------------------*/

static	void	GraspIdle( PlayerWork *work, int time ) ;
static	void	GraspRun( PlayerWork *work, int time ) ;
static	void	GraspSquatIdle( PlayerWork *work, int time ) ;

/*---------------------------------------------------------------*/
void PLY_EE_ResetGraspModeAndTrigger()
{
    EE_Trigger = 0 ;
    GraspMode = 0 ;
}

/* ダメージコールバック */
static	void	DamageCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	if ( work->current_mar == Motion ) {
		PL_ChangeMotionArc( work, work->org_motion ) ;
	}
	//GM_ResetMenuStatus( MENU_MENU_OFF ) ;
	PLY_EE_ResetGraspModeAndTrigger() ;
	work->dmg_callback = NULL ;
}

/* 手繋ぎ共通 */
static	void	GraspAct( PlayerWork *work )
{
	SetFlag( FLAG_CANNOT_CHANGE/*|FLAG_SPECIAL_MODE*/ ) ;
	//GM_SetMenuStatus( MENU_MENU_OFF ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
}

/* 手繋ぎ終了 */
static	void	EndGrasp( PlayerWork *work )
{
	PLY_EE_ResetGraspModeAndTrigger() ; 
	if ( work->dmg_callback == DamageCallback ) work->dmg_callback = NULL ;
	//GM_ResetMenuStatus( MENU_MENU_OFF ) ;
	PL_ReturnMotionArc( work ) ;
}

/* 終了チェック */
static	int		CheckEndGrasp( PlayerWork *work )
{
	if ( !( work->pad->status & PL_PAD_ACTION ) ||
		 EE_Trigger == 0 ) {
		return 1 ;
	}
	return 0 ;
}

/*---------------------------------------------------------------*/



/* 手繋ぎ静止 */
static	void	GraspIdle( PlayerWork *work, int time ) 
{
	GraspAct( work ) ;
	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, EEidle, 6 ) ;
		GraspMode = 1 ;
		EE_Trigger = 1 ;
		work->dmg_callback = DamageCallback ;
		if ( work->data == 0 ) PL_FootPrintForce( work->foot_work, 3 ) ;
		work->data = 0 ;
	}
	if ( CheckEndGrasp( work ) ) {
		EndGrasp( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	if ( work->pad->press & PL_PAD_SQUAT ) {
		SetMode( work, GraspSquatIdle ) ;
		work->data = 1 ;
		return ;
	}

	/* プレーヤーが目を合わせる */
	{
	    extern void EMA_PlayerEyeControl( void ) ;
	    //EMA_PlayerEyeControl()  ;
	}

	if ( Status( PLAYER_WATCH ) ) {
	    PL_SubjectTurn( work ) ;
	} else {
	    if ( EMA_CommandIsEnableLinkMove() )
		if ( work->pad->status & PAD_UDLR )
			SetMode( work, GraspRun ) ;
	}
}


#if 0

/* コイコイ静止 */
static	void	GraspComeon( PlayerWork *work, int time ) 
{
    GraspAct( work ) ;
    if ( time == 0 ) {
	PL_ChangeStance( work, SQUAT ) ;
	PL_ChangeMotionArc( work, Motion ) ;
	PL_SetAction( work, EEcomeon, 6 ) ;
	GraspMode = 1 ;
	EE_Trigger = 1 ;
	work->dmg_callback = DamageCallback ;
    }

    if ( work->pad->status & PAD_UDLR  ||
	 work->pad->press  & (PL_PAD_SQUAT|PL_PAD_CHANGE_WEAPON) ||
	 CheckEndGrasp( work ) )
    {
	EndGrasp( work ) ;
	PL_SetMode( work, PL_StillMode[ STAND ] ) ;
	return ;
    }

    /* プレーヤーが目を合わせる */
    {
	extern void EMA_PlayerEyeControl( void ) ;
	//EMA_PlayerEyeControl()  ;
    }

    if ( GV_VecLen3F2( &work->control.mov, &EE_Pos ) < 1000.0F ||
	 GM_PlayerBody->m_ctrl->mt3_ctrl->loop >= 2 )
	SetMode( work, GraspIdle ) ;

    if ( Status( PLAYER_WATCH ) )
	PL_SubjectTurn( work ) ;
}
#endif


/* 手繋ぎしゃがみ静止 */
static	void	GraspSquatIdle( PlayerWork *work, int time ) 
{
	GraspAct( work ) ;
	if ( time == 0 ) {
		PL_ChangeStance( work, SQUAT ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, EEsquat, 6 ) ;
		GraspMode = 1 ;
		EE_Trigger = 4 ;
		work->dmg_callback = DamageCallback ;
		if ( work->data == 1 ) PL_FootPrintForce( work->foot_work, 3 ) ;
		work->data = 0 ;
	}

	if ( CheckEndGrasp( work ) ) {
		EndGrasp( work ) ;
		PL_SetMode( work, PL_StillMode[ SQUAT ] ) ;
		return ;
	}

	if ( work->pad->press & PL_PAD_SQUAT ) {
		SetMode( work, GraspIdle ) ;
		work->data = 1 ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	} else {

	}
}

/* 手繋ぎ走り */
static	void	GraspRun( PlayerWork *work, int time ) 
{
	int			max ;

	GraspAct( work ) ;
	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		GraspMode = 1 ;
		work->dmg_callback = DamageCallback ;
		PL_FootPrintAct( work->foot_work, 1, -1, -1 ) ;
	}

	if ( CheckEndGrasp( work ) ) {
		EndGrasp( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	max = PL_MoveLevel( work ) ;
	if ( max != 0 ) {
		if ( max == 1 ) {
			PL_SetAction( work, EEwalk, 6 ) ;
			PL_FootPrintAct( work->foot_work, 0, 44, 94 ) ;
			EE_Trigger = 2 ;
		} else {
			PL_SetAction( work, EErun, 6 ) ;
			PL_FootPrintAct( work->foot_work, 0, 55, 16 ) ;
			EE_Trigger = 3 ;
		}
	}

	if ( work->pad->press & PL_PAD_SQUAT ) {
		SetMode( work, GraspSquatIdle ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, GraspIdle ) ;
	} else {
		if ( !( work->pad->status & PAD_UDLR ) ) {
			SetMode( work, GraspIdle ) ;
			return ;
		}
//		work->control.turn.vy = PadTo ;
		PL_SeekTurn( work ) ;
	}
}

/*---------------------------------------------------------------*/

/* ポーリング関数 */
static	int CheckGraspEE( PlayerWork *work )
{
    float dist ;
    FVECTOR diff ;

//    if ( GraspMode == 0 && work->dmg_callback == DamageCallback ) {
//		work->dmg_callback = NULL ;
//	}

    if ( Status( PLAYER_DAMAGED|PLAYER_FORCE ) )
    {
		PLY_EE_ResetGraspModeAndTrigger() ;
		if ( work->dmg_callback == DamageCallback ) work->dmg_callback = NULL ;
		return 0 ;
    }
    if ( work->weapon != WP_None || GraspMode != 0 )
	return 0 ;
    if ( work->pad->press & PL_PAD_ACTION )
	if ( (work->act_name == STAND_STILL ||
	      work->act_name == STAND_RUN ) &&
	     EMA_CommandIsEnableLink() &&
	     EE_NextMotion < DAMG_DOWN )
	{
	    _sceVu0SubVector( &diff, &work->control.mov, &EE_Pos ) ;
	    dist = _sceVu0InnerProduct( &diff, &diff ) ;
	    if ( dist < /*3500.0F*/ 1500.0f*1500.0f )
	    {
#if 0
		HZX_ZON *zon ;

		zon = HZX_GetZone( work->control.hzx_id,
				   work->control.addr & 0xff ) ;
		if ( zon ? !(zon->flag & HZX_ZONE_ZINTRPT) : 1 )
#endif
		    /* 距離が 250cm以内で 腰の高さが500cm以上 差がないこと */
		    if ( fpu_Abs(work->control.mov.vy - EE_Pos.vy) < 1500.0f )
		    {
			work->control.turn.vy =
			    2048.0f/M_PI*atan2f( diff.vx, diff.vz ) ;
			//PL_SetMode( work, GraspComeon ) ;
			PL_SetMode( work, GraspIdle ) ;
			SetFlag( FLAG_CANNOT_CHANGE ) ;
			return 1 ;
		    }
	    }
	}
    return 0 ;
}

/*---------------------------------------------------------------*/

/* プラグイン登録 */
int		NewPluginGraspEE( void )
{
	//void *ee_work ;
	void EMA_SendMessageGetPosition( FVECTOR *pos ) ;
	void EMA_SendMessageSetTrigger( int *trigger ) ;
	void EMA_SendMessageGetMotionNum( int *motion ) ;

	PL_AddPollingFunc( &PollingSet, CheckGraspEE ) ;
	Motion = GCL_GetOptionValue( 'm', 0 ) ;

	EE_NextMotion = 0 ;
	PLY_EE_ResetGraspModeAndTrigger() ;

	EMA_SendMessageGetPosition( &EE_Pos ) ;
	EMA_SendMessageSetTrigger( &EE_Trigger ) ;
	EMA_SendMessageGetMotionNum( &EE_NextMotion ) ;

	//ee_work = NewEETest( &EE_NextMotion, &EE_Trigger, &EE_Pos ) ;

	return 0 ;
}

/*---------------------------------------------------------------*/



