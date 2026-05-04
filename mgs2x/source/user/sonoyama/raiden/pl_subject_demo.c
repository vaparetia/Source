//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_subject_demo.c
   主観デモ用プレイヤー

   2001/02/08	M.Sonoyama
   $Id: pl_subject_demo.c,v 1.1.1.3 2002/11/19 11:50:59 Yoshizawa1 Exp $
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
#include	"camera.h"

#include "BP_Misc.h"
#include "BP_Debug.h"//BP_GAMEPLAY

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)

/*----------------------------------------------------------------*/

extern	void 	*NewDebugCamera( int ) ;
extern	void	*NewDebugCameraForce( int ) ;

#ifdef DEBUG_MODE
extern	void	*NewPlayerDebugView( void * ) ;
#endif

#if defined(BP_VITA)
extern	void	*NewWeaponChangeVita( int ) ;
extern	void	*NewItemChangeVita( int ) ;
#else
extern void* NewWeaponChange1(int);
extern void* NewItemChange1(int);
#endif

/*----------------------------------------------------------------*/

typedef	struct t_Work_pl_subject_demo	{  //BP_GENERAL - added type so we can debug on PC
	GV_ACT_EX			actor ;
	CONTROL_NOEVT		control ;
	OBJECT				body ;
	FMATRIX				light[ 2 ] ;

	SVECTOR				rotate ;
	int					ry_max ;
	int					ry_min ;
	int					rx_max ;
	int					rx_min ;

	GM_GageSet			life ;
	int					gagetime ;

	GV_PAD				*pad ;
	int					motion ;
	int					flag ;
	GM_CameraSet		*camera ;

	short				subject_turn_accel[ 2 ] ;
	int					motion_flag ;
} Work ;

enum {
	MSG_POSITION =		0,
	MSG_ROTATE =		1,
	MSG_MOTION =		2,	
	MSG_RYLIMIT =		3,
	MSG_RXLIMIT =		4,
} ;

enum {
	MOTION_REPEAT =		0,
	MOTION_END_STOP =	1,
	MOTION_END_ZERO =	2,
} ;

/*----------------------------------------------------------------*/

/* 回転 */
/* プレイヤーと同じ */
#define	VALUE_ADJ	(127.0F - ANALOG_MARGIN_F)
static	void	SubjectTurn( Work *work ) 
{
    int		status, turn ;
    float	vstep, hstep ;
	int		vmax[ 2 ] ;
    u_char	*pressure ;
    short	*ac ;
    float	dx, dy ;

    ac = work->subject_turn_accel ;
    vstep = ( float )GM_SubjectVStep ;
    hstep = ( float )GM_SubjectHStep ;
    status = work->pad->status ;
    pressure = work->pad->pressure ;
    dx = dy = 0.0F ;

    if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		dx = -( ( float )work->pad->left_dx - 128.0F ) ;
		dx += ( dx > 0.0F ) ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;
		dy = ( float )work->pad->left_dy - 128.0F ;
		dy += ( dy > 0.0F ) ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;
    } else {
		if ( status & PAD_U ) 
			dy = - ( ( float )pressure[ PAD_PRESS_U ] * ( VALUE_ADJ * 2.0F / 255.0F ) ) ;
		else if ( status & PAD_D ) 
			dy = ( float )pressure[ PAD_PRESS_D ] * ( VALUE_ADJ * 2.0F / 255.0F ) ;
		if ( status & PAD_L ) 
			dx = ( float )pressure[ PAD_PRESS_L ] * ( VALUE_ADJ * 2.0F / 255.0F ) ;
		else if ( status & PAD_R ) 
			dx = -( ( float )pressure[ PAD_PRESS_R ] * ( VALUE_ADJ * 2.0F / 255.0F ) ) ;
		if ( dy < -VALUE_ADJ ) dy = -VALUE_ADJ ;
		else if ( dy > VALUE_ADJ ) dy = VALUE_ADJ ;
		if ( dx < -VALUE_ADJ ) dx = -VALUE_ADJ ;
		else if ( dx > VALUE_ADJ ) dx = VALUE_ADJ ;
    }

	/* 主観操作反転 */
	PL_ShukanReverse( &dy, NULL ) ;
	
	vstep *= 2.4F ; hstep *= 2.4F ;
	ac[ 0 ] = ac[ 1 ] = 0 ;

   if ( BP_IsPAL()==TRUE )//BP_FRAMERATE - adjust for runtime PAL
   {
	   vstep *= 1.20F ; hstep *= 1.20F ;
   }

   vmax[ 0 ] = work->rotate.vx + work->rx_max ;
	vmax[ 1 ] = work->rotate.vx + work->rx_min ;
	if ( vmax[ 0 ] >= 1023 ) vmax[ 0 ] = 1023 ;
	if ( vmax[ 1 ] <= -1023 ) vmax[ 1 ] = -1023 ;

    if ( status & ( PAD_U | PAD_D ) ) {
		turn = ( int )( vstep * dy / VALUE_ADJ ) ;
		if ( turn == 0 && vstep != 0.0F && dy != 0.0F ) {
			turn = ( dy > 0.0F ) ? 1 : -1 ;
		}
		turn += work->control.turn.vx ;
		if ( turn < vmax[ 1 ] ) turn = vmax[ 1 ] ;
		if ( turn > vmax[ 0 ] ) turn = vmax[ 0 ] ;
		if ( turn < -1023 ) turn = -1023 ;
		else if ( turn > 1023 ) turn = 1023 ;
		work->control.turn.vx = turn ;
    } else {
		if ( GM_CameraDir.vx > vmax[ 0 ] ) {
			GM_CameraDir.vx = vmax[ 0 ] ;
		} 
		if ( GM_CameraDir.vx < vmax[ 1 ] ) {
			GM_CameraDir.vx = vmax[ 1 ] ;
		}
		work->control.turn.vx = work->control.rot.vx = GM_CameraDir.vx ;

    }
    if ( status & ( PAD_L | PAD_R ) ) {
		turn = ( int )( hstep * dx / VALUE_ADJ ) ;
		if ( turn == 0 && hstep != 0.0F && dx != 0.0F ) {
			turn = ( dx > 0.0F ) ? 1 : -1 ;
		}	    
		work->control.turn.vy += turn ;
		if ( work->control.turn.vy > work->rotate.vy + work->ry_max ) {
			work->control.turn.vy = work->rotate.vy + work->ry_max ;
		} else if ( work->control.turn.vy < work->rotate.vy + work->ry_min ) {
			work->control.turn.vy = work->rotate.vy + work->ry_min ;
		}
    } else {
		if ( work->control.rot.vy > work->rotate.vy + work->ry_max ) {
			work->control.rot.vy = work->rotate.vy + work->ry_max ;
		} else if ( work->control.rot.vy < work->rotate.vy + work->ry_min ) {
			work->control.rot.vy = work->rotate.vy + work->ry_min ;
		}
		work->control.turn.vy = work->control.rot.vy ;
	}
}

static	void	UpdateCamera( Work *work ) 
{
	DG_COPY_VEC( &work->camera->position, &work->control.mov ) ;
	GM_CameraDir.vx = work->control.rot.vx ;
	GM_CameraDir.vy = work->control.rot.vy ;
}

/* メッセージ処理 */
static	void	CheckMessage( Work *work )
{
	int			n_msg, code ;
	FVECTOR		pos ;
	SVECTOR		pre_rot, diff_rot ;
	GV_MSG		*msg ;

	n_msg = work->control.n_msg ;
	if ( n_msg == 0 ) return ;
	msg = work->control.msg ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case MSG_POSITION :
			pos.vx = ( float )msg->message[ 1 ] ;
			pos.vz = ( float )msg->message[ 2 ] ;
			if ( msg->message_len >= 4 ) pos.vy = ( float )msg->message[ 3 ] ;
			else						 pos.vy = work->control.mov.vy ;
			GM_ResetControlPosition( ( CONTROL * )&work->control, &pos ) ;
			break ;
		case MSG_ROTATE :
			pre_rot = work->rotate ;
			work->rotate.vx = msg->message[ 1 ] ;
			work->rotate.vy = msg->message[ 2 ] ;
			diff_rot.vx = msg->message[ 1 ] - pre_rot.vx ;
			diff_rot.vy = msg->message[ 2 ] - pre_rot.vy ;
			if ( msg->message_len >= 4 && msg->message[ 3 ] > 0 ) {
				work->control.turn.vx = work->rotate.vx ;
				work->control.turn.vy = work->rotate.vy ;
				work->control.rot.vx = work->rotate.vx ;
				work->control.rot.vy = work->rotate.vy ;
				GM_CameraDir.vx = work->rotate.vx ;
				GM_CameraDir.vy = work->rotate.vy ;
			} else {
				work->control.turn.vx += diff_rot.vx ;
				work->control.turn.vy += diff_rot.vy ;
				work->control.rot.vx += diff_rot.vx ;
				work->control.rot.vy += diff_rot.vy ;
				GM_CameraDir.vx += diff_rot.vx ;
				GM_CameraDir.vy += diff_rot.vy ;
			}
			if ( work->control.turn.vx > 1023 ) work->control.turn.vx = 1023 ;
			else if ( work->control.turn.vx < -1023 ) work->control.turn.vx = -1023 ;
			if ( work->control.rot.vx > 1023 ) work->control.rot.vx = 1023 ;
			else if ( work->control.rot.vx < -1023 ) work->control.rot.vx = -1023 ;
			if ( GM_CameraDir.vx > 1023 ) GM_CameraDir.vx = 1023 ;
			else if ( GM_CameraDir.vx < -1023 ) GM_CameraDir.vx = -1023 ;
			break ;
		case MSG_MOTION :
			work->motion = msg->message[ 1 ] ;
			work->motion_flag = msg->message[ 2 ] ;
			work->body.m_ctrl->mt3_ctrl[ 0 ].flag &= ~MT3_SLEEP ;
			GM_ConfigObjectAction( &work->body, 0, work->motion, 0, 0xfffff, 0 ) ;
			work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
			break ;
		case MSG_RYLIMIT :
			work->ry_max = msg->message[ 1 ] ;
			work->ry_min = -msg->message[ 2 ] ;
			break ;
		case MSG_RXLIMIT :
			work->rx_max = msg->message[ 1 ] ;
			work->rx_min = -msg->message[ 2 ] ;
			break ;
		}
		msg ++ ;
	}
}

/*----------------------------------------------------------------*/

static	void	Action( Work *work )
{
	int			motion_flag ;

	/* メッセージチェック */
	CheckMessage( work ) ;

	/* モーション制御 */
	if ( MT_CHECK_LAST1( work->body.m_ctrl, 0 ) && 
		 !( work->body.m_ctrl->mt3_ctrl[ 0 ].flag & MT3_SLEEP ) ) {
		motion_flag = work->motion_flag ;
		switch( motion_flag ) {
		case MOTION_REPEAT :
			break ;
		case MOTION_END_STOP :
			work->body.m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
			break ;
		case MOTION_END_ZERO :
			GM_ConfigObjectAction( &work->body, 0, 0, 0, 0xfffff, 0 ) ;			
			work->motion_flag = MOTION_REPEAT ;
			work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
			break ;
		}
	}
}

static	void	UpdateLife( Work *work )
{
	int			p_vital ;

	p_vital = GM_Vitality ;

	if ( GM_CheckPlayerStatus( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) ||
		 GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
		if ( GM_VitalityAdjust < 0 ) GM_VitalityAdjust = 0 ;		
		/* NoDeadは無敵でも食らう */
		//if ( GM_VitalityAdjustNoDead < 0 ) GM_VitalityAdjustNoDead = 0 ;
	}
	if ( GM_CheckPlayerStatus( PLAYER_NORECOVER ) ) {
		if ( GM_VitalityAdjust > 0 ) GM_VitalityAdjust = 0 ;
		if ( GM_VitalityAdjustNoDead > 0 ) GM_VitalityAdjustNoDead = 0 ;
	}
	if ( !GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
		work->life.value += GM_VitalityAdjust ;
		/* 死なない調整値 */
		if ( GM_VitalityAdjustNoDead != 0 &&
			( work->life.value + GM_VitalityAdjustNoDead > 0 ) ) {
			work->life.value += GM_VitalityAdjustNoDead ;
		}
	}
#if 0
	/* ダメージ値カウント */
	if ( GM_VitalityAdjust < 0 ) GM_DamageCount += GM_VitalityAdjust ;
	if ( GM_VitalityAdjustNoDead < 0 ) GM_DamageCount += GM_VitalityAdjustNoDead ;
#endif
	GM_VitalityAdjust = 0 ;
	GM_VitalityAdjustNoDead = 0 ;
    if ( work->life.value > GM_VitalityMax ) work->life.value = GM_VitalityMax ;
    if ( work->life.value < 0 ) work->life.value = 0 ;

    GM_Vitality = work->life.value ;

    /* ゲージ表示／非表示 */
	if ( p_vital == GM_Vitality &&
		GM_Vitality > GM_VitalityMax / 3 &&
		GM_N_ActiveGages == 1 ) {
		if ( ++ work->gagetime >= 500 ) {
			GM_GageStatus = GM_GAGE_STATE_DISAPPEAR ;
		}
	} else {
		work->gagetime = 0 ;
		if ( !GM_CheckMenuStatus( MENU_MENU_OPEN ) ) {
			GM_GageStatus = GM_GAGE_STATE_APPEAR ;
		}
	}
	if ( GM_CheckMenuStatus( MENU_MENU_OPEN ) ) work->gagetime = 0 ;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	SubjectTurn( work ) ;
	GM_ActMotion( &work->body ) ;
	GM_ActControl( ( CONTROL * )&work->control ) ;
	GM_ActObject2( &work->body ) ;
	UpdateCamera( work ) ;
	DG_GetLightMatrix( &work->control.mov, work->light ) ;

	Action( work ) ;

	DG_COPY_VEC( &GM_PlayerPosition, &work->control.mov ) ;
	GM_PlayerMap = work->control.map ;
	GM_PlayerPosX = work->control.mov.vx ;
	GM_PlayerPosY = work->control.mov.vy ;
	GM_PlayerPosZ = work->control.mov.vz ;
	GM_PlayerDir = work->control.rot.vy & 4095 ;
	GM_PlayerMotion = work->motion ;
	GM_SubjectVMaxTmp[ 0 ] = 0 ;
	GM_SubjectVMaxTmp[ 1 ] = 0 ;
	GM_SetChanlTargetMap( GM_MAP_CHANL_MAIN, GM_PlayerMap ) ;

	UpdateLife( work ) ;
}

static	void	Die( Work *work )
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_RemoveGageSet( &work->life ) ;
	if ( GM_PlayerSubjectCamera[ 0 ] == work->camera ) {
		GM_PlayerSubjectCamera[ 0 ] = NULL ;
	}
	GM_PlayerControl = NULL ;
	GM_PlayerBody = NULL ;
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_SUBJECT_DEMO ) ;
}

/*----------------------------------------------------------------*/

static	void	InitCamera( Work *work )
{
	GM_CameraSet	*cam ;

	cam = work->camera = NewProgramCamera( 7532342 , //GV_StrCode( "主観カメラ" )
										   0,
										   GM_CAMERA_SUBJECT, SUBJECT_CAMERA_PRIO ) ;
	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE,
					  CAM_FLAG_DIR_TRACE | CAM_FLAG_PAD_ADJUST ) ;
	GM_SetCameraRotate( cam, &work->control.rot ) ;
	GM_SetCameraTrack( cam, 1000 ) ;
	GM_SetCameraAngle( cam, 2.0F ) ;	
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK, 0, 0 ) ;
	GM_PlayerSubjectCamera[ 0 ] = cam ;

	GM_CameraDir.vx = work->control.rot.vx ;
	GM_CameraDir.vy = work->control.rot.vy ;

	cam->on = 1 ;
}

static	void	InitGage( Work *work )
{
    GM_GageSet	*gs ;

    gs = &work->life ;
    GM_InitGageSet( gs, "LIFE", 16, 160, GM_DEFAULT_GAGE_HEIGHT, 
				   GM_Vitality, GM_VitalityMax, 0, 30, 0 ) ;
    GM_SetGageColor( gs, 0, 0, 0, 40, 128, 118, 110, 190, 118, 255, 0, 0 ) ;
    GM_AppendGageSet( gs ) ;
	GM_VisibleGage( gs ) ;
}

static	int		GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;

	ctrl = ( CONTROL * )&work->control ;
	body = &work->body ;
	GM_InitControlEx( ctrl, name, where, CTRL_ATR_NOCHK_HZX ) ;
	GM_ConfigControlMessageCheck( ctrl ) ;
	PL_GetOptionFV( 'p', &ctrl->mov ) ;
	PL_GetOptionSV( 'r', &ctrl->rot ) ;
	ctrl->turn = ctrl->rot ;
    GM_ConfigControlMapID( ( CONTROL * )&work->control ) ;
	GM_InitObject( body, GCL_GetOptionValue( 'k', 0 ), OBJECT_FLAG ) ;
	GM_ConfigObjectEvm( body, GCL_GetOptionValue( 'e', 0 ), 0 ) ;
	GM_ConfigObjectMotion( body, 1, GCL_GetOptionValue( 'm', 0 ), 0 ) ;
	GM_ConfigObjectStep( body, &ctrl->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;
	DG_SetLightMatrix( body->objs, work->light ) ;
	body->m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( body->objs ) ;
	GM_GroupObjs( body->objs, where ) ;

	InitCamera( work ) ;
	InitGage( work ) ;

	GM_PlayerControl = ctrl ;
	GM_PlayerBody = body ;

	work->pad = GV_PadData ;
#ifdef DEBUG_MODE
	NewPlayerDebugView( NULL ) ;
#endif

#if defined(BP_VITA)
	NewWeaponChangeVita( 0 ) ;
   NewItemChangeVita( 0 ) ;
#else
   NewWeaponChange1( 0 ) ;
   NewItemChange1( 0 ) ;
#endif

	work->rotate = ctrl->rot ;
	work->ry_max = 1024 ;
	work->ry_min = -1024 ;
	work->rx_max = 640 ;
	work->rx_min = -800 ;

	GM_SetPlayerStatusEX( I64(0), PLAYER2_SUBJECT_DEMO ) ;

	work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	work->body.objs->flag |= DG_FLAG_INVISIBLE ;

	return 0 ;
}

/*----------------------------------------------------------------*/

/* 起動 */
void	*NewSubjectDemoPlayer( int name, int where )
{
	Work			*work ;

   //BP_GAMEPLAY - override Solidus Boss choking to be easier
   {
      // Solidus Boss choking demo?
      const char* areaName = GM_GetArea();
      if( (name == 0x00ec76da) && (strcmp(areaName, "w51a") == 0) )
      {
         // Override
         const int life = BP_GetTweakValueFromDifficulty( gBP_Game_SolidusChoke_Life );
         GM_Vitality = GM_VitalityMax = life;
      }
   }
   //BP_GAMEPLAY - override Solidus Boss choking to be easier

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ARM_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, where ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}
