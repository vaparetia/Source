//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sneeze.c
   くしゃみプラグイン

   2000/12/23 M.Sonoyama
   $Id: sneeze.c,v 1.1.1.3 2002/11/19 11:50:50 Yoshizawa1 Exp $
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

static	PL_PollingSet	PollingSet ;
//static	int				OrgMotion ;
static	int				SnzMotion ;
static	int				Sneezing = 0 ;
//static	int				SneezeTime = 0 ;
static	int				SneezeCount = 0 ;

extern	void			*New_ACHUU( FMATRIX *world ) ;
static	void			Sneeze( PlayerWork *work, int time ) ;

/*---------------------------------------------------------------*/

static	void			Sneeze( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, SnzMotion, 6 ) ;
	}
	SetFlag( FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_MENU_DISABLE ) ;
	if ( PL_CheckMotionTime( work, 14 ) &&
		 !PL_AttackDisable() &&
		 PL_PadEnable() ) {
		GV_SetActorChild( work, New_ACHUU( &work->body.objs->objs[ HUMAN21_ATAMA ].world ) ) ;
		GM_SeSetMode( SD_V_RAISNZ01, &work->control.mov, GM_SEMODE_BOMB ) ;
		//GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;
		work->data = 1 ;
	}
	if ( work->data > 0 && !PL_AttackDisable() && PL_PadEnable() ) {
		if ( ++ work->data >= DIRECT_TICK( 60 ) ) {
			GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;				
			work->data = 0 ;
		}
	}

	if ( EndMotion( work ) || PL_AttackDisable() || !PL_PadEnable() ) {
		if ( EndMotion( work ) && work->data != 0 ) {
			GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;
		}
		if ( Status( PLAYER_CAUTION ) ) {
			SetMode( work, PL_StandCautionStill ) ;
		} else {
			SetMode( work, PL_StillMode[ work->stance ] ) ;
		}
		Sneezing = 0 ;
		SneezeCount = 0 ;
	}
}

/*---------------------------------------------------------------*/

static	int			CheckSneeze( PlayerWork *work )
{
	int				act ;
	PL_MOTION_SET	*ms ;
	extern	void	PL_PlayerSetCold( void ) ;

	if ( GM_PlayerCold == 1 && !Status( PLAYER_COLD ) ) {
		PL_PlayerSetCold() ;
	}

	if ( Sneezing == 1 && work->action != Sneeze ) {	
		/* くしゃみが中断された */
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}

	if ( !Status( PLAYER_COLD ) || Sneezing == 1 ) return 0 ;
	if ( work->action2 != NULL ) {
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}
	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}
	act = work->act_name ;
	if ( act != STAND_STILL &&
		 //act != SQUAT_STILL &&	/* しゃがみは無しにしてみる */
		 act != GROUND_STILL &&
		 act != STAND_CAUTION_STILL ) {
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}

	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}

	ms = PL_MotionSet ;
	if ( act == STAND_STILL && 
		 work->motion1 != ms->change[ Mstand ] ) {
		Sneezing = 0 ;
		SneezeCount = 0 ;
		return 0 ;
	}

	if ( Sneezing == 0 ) {
		if ( ++ SneezeCount >= GM_PlayerSneezeTime ) {
			SetFlag( FLAG_CANNOT_CHANGE ) ;
			SetStatus( PLAYER_MENU_DISABLE ) ;
			if ( act == STAND_CAUTION_STILL ) {
				SnzMotion = ms->shared[ Msnz_caution ] ;
			} else {
				SnzMotion = ms->shared[ Msnz_idle + work->stance ] ;
			}
			work->trigger = TRIG_FALL ;	/* グレネード落とし */
			SetMode2( work, NULL ) ;
			SetMode( work, Sneeze ) ;
			Sneezing = 1 ;
#if 0
			OrgMotion = work->motion1 ;
			if ( act == STAND_CAUTION_STILL ) {
				SnzMotion = ms->shared[ Msnz_caution ] ;
			} else {
				SnzMotion = ms->shared[ Msnz_idle + work->stance ] ;
			}
			SetAction( work, SnzMotion, 6 ) ;
			work->motion1 = OrgMotion ;
			Sneezing = 1 ;
#endif
			return 1 ;
		}
	} else {
#if 0
		if ( work->motion1 != OrgMotion ||
			 ( work->motion1 == OrgMotion && EndMotion( work ) ) ) {
			if ( work->motion1 == OrgMotion ) {
				work->motion1 = -1 ;
				SetAction( work, OrgMotion, 6 ) ;
			}
			Sneezing = 0 ;
			SneezeCount = 0 ;			
		}
#endif
	}
	return 0 ;
}

/* プラグイン登録 */
int		NewPluginSneeze( void )
{
	PL_AddPollingFunc( &PollingSet, CheckSneeze ) ;
	Sneezing = 0 ;
	SneezeCount = 0 ;
	return 0 ;
}

/*---------------------------------------------------------------*/

/* 風邪を引かす */
void	PL_PlayerSetCold( void )
{
	GM_SetPlayerStatus( PLAYER_COLD ) ;	
	if ( GM_PlayerCold == 0 ) GM_PlayerColdStartTime = GM_PlayTime ;
	GM_PlayerCold = 1 ; 
	if ( GM_PlayerSneezeTime <= 0 ) {
		printf( "風邪くしゃみ間隔をセットしてください\n" ) ;
	}
}

/* シナリオ */
int		NewPlayerSetCold( void )
{
	/* くしゃみ間隔 */
	if ( GCL_GetOption( 't' ) != NULL ) {
		GM_PlayerSneezeTime = GCL_GetNextInt() * ( 300 / TIME_BASE ) ;
	}
	PL_PlayerSetCold() ;
	return 0 ;
}

/* このタイミングでくしゃみを出す */
int		PL_COM_Sneeze( void )
{
	SneezeCount = GM_PlayerSneezeTime ;
	return 0 ;
}

/* 風邪を治す */
int		NewPlayerRecoverCold( void )
{
	GM_ResetPlayerStatus( PLAYER_COLD ) ;
	GM_PlayerCold = 0 ;
	GM_PlayerColdCount = 0 ;
	return 0 ;
}



