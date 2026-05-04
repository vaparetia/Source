//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   eetest.c
   エマテスト

   2001/01/29 M.Sonoyama
   $Id: eetest.c,v 1.1.1.3 2002/11/19 11:50:47 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/

extern	void	Leg_IKcalc( CONTROL *, OBJECT * ) ;

/*----------------------------------------------------------------*/

#define		NTSC_TIME_BASE	(5)

#define		EE_NAME			GV_StrCode( "エマ" )
#define		EE_MOTION		GV_StrCode( "ee_test" )
#define		EE_MODEL		GV_StrCode( "ema_def_sh_mt" )
#define		OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)

#define		S_SPHERE			(500)
#define		R_SPHERE			(474)
#define		NEAR_HEIGHT_STAND	(748.0F)
#define		NEAR_HEIGHT_SQUAT	(248.0F)
#define		NEAR_HEIGHT_GROUND	(248.0F)

typedef	struct _Work {
	GV_ACT_EX			actor ;
	CONTROL				control ;
	OBJECT				body ;
	FMATRIX				lights[ 2 ] ;	
	TARGET				def_targ ;
	POWER_TARGET		power ;
	CAPTURE_TARGET		capture ;
	RADAR_CTRL			radar ;

	int					motion ;
	int					*next_motion ;
	int					*trigger ;
	float				hzx_height ;

	FVECTOR				*pos ;

	int					flag ;
	int					mode ;

	int					time ;
	int					ftime ;
	int					ftime_count ;
	int					data ;
	int					data2 ;
	void				( *action )( struct _Work *, int ) ;
} Work ;

typedef	void	( *ACTION )( Work *, int ) ;

enum {
	Mgrasp_idle,
	Mgrasp_walk,
	Mgrasp_run,
	Mgrasp_squat_idle,
} ;

enum {
	FLAG_NONE				= 0x0000,
	FLAG_NO_MOTION_STEP_XZ	= 0x0001,
} ;

static	FVECTOR	StartPos ;
static	SVECTOR	StartRot ;

static	Work	*EEWork ;

static	void	StandIdle( Work *work, int time ) ;
static	void	GraspIdle( Work *work, int time ) ;
static	void	GraspSquatIdle( Work *work, int time ) ;
static	void	GraspWalk( Work *work, int time ) ;
static	void	GraspRun( Work *work, int time ) ;

/*----------------------------------------------------------------*/

static	FVECTOR			Shifts[] = {
	{ -400.0F, 0.0F, -430.0F },		/* 静止シフト */
	{ -450.0F, 0.0F, -500.0F },		/* 歩きシフト */
	{ -500.0F, 0.0F, -800.0F }		/* 走りシフト */
} ;

/*----------------------------------------------------------------*/

static	void	SetAction( Work *work, int motion, int interp )
{
	int			rc ;

	if ( work->motion == motion ) return ;
	GM_ConfigObjectAction( &work->body, 0, motion, 0, 0xfffff, interp * NTSC_TIME_BASE ) ;
	if ( interp == 0 ) work->body.m_ctrl->interp_flag = 0 ;
	work->motion = motion ;
	if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
	}
}

static	void	SetMode( Work *work, ACTION action )
{
	work->time = 0 ;
	work->ftime_count = 0 ;
	work->action = action ;
	work->data = work->data2 = 0 ;
}

static	inline	void	SetFlag( int flag )
{
	EEWork->flag |= flag ;
}

static	inline	void	ResetFlag( int flag )
{
	EEWork->flag &= ~flag ;
}

static	inline	int		Flag( int flag )
{
	return ( EEWork->flag & flag ) ;
}

/*----------------------------------------------------------------*/

/* 位置あわせ */
static	void	SetPos2( Work *work, int mode )
{
	FVECTOR		root, pos, diff ;
	int			i ;

//	return ;

	GV_MatToVec( &GM_PlayerBody->objs->objs[ HUMAN21_MIGI_TE ].world, &root ) ;
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_TE ].world, &pos ) ;
	_sceVu0SubVector( &diff, &root, &pos ) ;

	for ( i = 0; i < 21; i ++ ) {
		work->body.objs->objs[ i ].world.m[ 3 ][ 0 ] += diff.vx ;
		work->body.objs->objs[ i ].world.m[ 3 ][ 2 ] += diff.vz ;
	}
	work->control.mov.vx += diff.vx ;
	work->control.mov.vz += diff.vz ;

	work->control.mov.vy = GM_PlayerControl->levels[ 0 ] + work->control.height ;
	work->control.hzx_base = work->control.mov.vy - work->control.height ;

	work->control.turn.vy = GM_PlayerControl->turn.vy ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;	
}

static	void	SetPos( Work *work, int mode )
{
	FVECTOR		shift ;
	FMATRIX		world ;

	return ;

	work->control.turn.vy = GM_PlayerControl->turn.vy ;

	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 1 ) ) {
		DG_COPY_VEC( &shift, &Shifts[ mode ] ) ;
		DG_COPY_MAT( &world, &GM_PlayerBody->objs->world ) ;
		DG_SetPos( &world ) ;
		DG_PutVector( &shift, &shift, 1 ) ;
		work->control.mov.vx = shift.vx ;
		work->control.mov.vz = shift.vz ;
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	}
}

/*----------------------------------------------------------------*/

/* 立ち静止 */
static	void	StandIdle( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mgrasp_idle, 6 ) ;
	}
}

/* 手繋ぎ静止 */
static	void	GraspIdle( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mgrasp_idle, 6 ) ;
	}
	SetPos( work, 0 ) ;
	SetPos2( work, 0 ) ;
}

/* 手繋ぎしゃがみ静止 */
static	void	GraspSquatIdle( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mgrasp_squat_idle, 6 ) ;
	}
	SetPos( work, 0 ) ;
	SetPos2( work, 0 ) ;
}

/* 手繋ぎ歩き */
static	void	GraspWalk( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mgrasp_walk, 6 ) ;
	}
	SetPos( work, 1 ) ;	
	SetPos2( work, 0 ) ;
}

/* 手繋ぎ走り */
static	void	GraspRun( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mgrasp_run, 6 ) ;
	}
	SetPos( work, 2 ) ;
	SetPos2( work, 0 ) ;
}

/*----------------------------------------------------------------*/

static	void	CheckGraspMode( Work *work )
{
	int			trig ;

	trig = *work->trigger ;
	if ( trig != work->mode ) {
		work->mode = trig ;
		switch( trig ) {
		case 0 :
			SetMode( work, StandIdle ) ;
			break ;
		case 1 :
			SetMode( work, GraspIdle ) ;
			break ;
		case 2 :
			SetMode( work, GraspWalk ) ;
			break ;
		case 3 :
			SetMode( work, GraspRun ) ;
			break ;
		case 4 :
			SetMode( work, GraspSquatIdle ) ;
			break ;
		default :
		  ;
		}
	}
}

/*----------------------------------------------------------------*/

static	void	Action( Work *work )
{
	int			time ;
	void		( *action )( Work *, int ) ;

	CheckGraspMode( work ) ;

	time = work->time ;
	work->time ++ ;
	work->ftime = work->ftime_count ;
	work->ftime_count ++ ;
	action = work->action ;
	if ( action != NULL ) ( *action )( work, time ) ;

	if ( GV_PadData->status & PAD_L1 ) {
		printf( "----\n" ) ;
		DumpVec( &GM_PlayerControl->mov ) ;
		DumpVec( &work->control.mov ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GM_ActMotion( &work->body ) ;
	work->control.height = work->body.height ;

	if ( Flag( FLAG_NO_MOTION_STEP_XZ ) ) {
		work->control.step.vx -= work->body.m_ctrl->step.vx ;
		work->control.step.vz -= work->body.m_ctrl->step.vz ;
	}

	ResetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

	GM_ActControl( &work->control ) ;
	Leg_IKcalc( &work->control, &work->body ) ;
	GM_ActObject2( &work->body ) ;
	DG_GetLightMatrix( &work->control.mov, work->lights ) ;
	if ( work->control.grounded & 1 ) work->control.step.vy = 0.0F ;
//	work->control.step.vy -= 96.0F ;

	DG_COPY_VEC( work->pos, &work->control.mov ) ;

	Action( work ) ;
}

/*----------------------------------------------------------------*/

static	void	Die( Work *work )
{
	GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
	EEWork = NULL ;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;

	ctrl = &work->control ;
	body = &work->body ;

	GM_InitControl( ctrl, EE_NAME, 0 ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlObject( ctrl, body ) ;
	GM_ConfigControlMapCheck( ctrl ) ;
	GM_ConfigControlHazard( ctrl, 1000, R_SPHERE, S_SPHERE ) ;
	GM_ConfigControlHzxHeight( ctrl, NEAR_HEIGHT_STAND, StartPos.vy ) ;
	GM_ConfigControlHzxCheckFlag( ctrl, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;	
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK | CTRL_SKIP_SEG_CHECK ;

	GM_InitObject( body, EE_MODEL, OBJECT_FLAG ) ;
	GM_ConfigObjectLight( body, work->lights ) ;
	GM_ConfigObjectMotion( body, 1, EE_MOTION, MT_FLAG_HUMAN2 ) ;
	GM_ConfigObjectStep( body, &ctrl->step ) ;
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 ) ;

//	GV_SetVec3( &ctrl->mov, 9500.0F, -4000.0F, -15000.0F ) ;
	StartPos.vy += 750.0F ;
	DG_COPY_VEC( &ctrl->mov, &StartPos ) ;
	ctrl->rot.vy = ctrl->turn.vy = StartRot.vy ;

	GM_ConfigControlMapID( ctrl ) ;

	work->motion = 0 ;

	SetMode( work, StandIdle ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewEETest( int	*next_motion, int *trigger, FVECTOR *pos )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->next_motion = next_motion ;
		work->trigger = trigger ;
		work->pos = pos ;
		EEWork = work ;
	}
	return work ;
}

/*----------------------------------------------------------------*/

int		NewComEETestSetStartPos( void )
{
	PL_GetOptionFV( 'p', &StartPos ) ;
	PL_GetOptionSV( 'r', &StartRot ) ;
	return 0 ;
}
