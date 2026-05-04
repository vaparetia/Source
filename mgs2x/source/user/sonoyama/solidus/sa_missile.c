//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sa_missile.c
   ソリダス／蛇手ミサイル

   2001/03/29	M.Sonoyama
   $Id: sa_missile.c,v 1.1.1.3 2002/11/19 11:51:05 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"sol_common.h"

#include "BP_Misc.h"

/*----------------------------------------------------------------*/

extern	void 	*NewRisingSmoke( FVECTOR* pos, float size ) ;
extern	int		PL_SlashNow( void ) ;
extern void *NewBladeSpark( FMATRIX *mat );

extern	void	*NewSnakeArmMissileSlashed( FMATRIX *world, FVECTOR *step, int map, int which ) ;

/*----------------------------------------------------------------*/

#define	BODY_NAME		GV_StrCode( "sol_msl" )
#define	BODY_FLAG		(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

typedef	struct	{
	GV_ACT_EX			actor ;
	CONTROL_NOEVT		control ;
	OBJECT				body ;
	FMATRIX				lights[ 2 ] ;
	FVECTOR				speed_def0 ;
	FVECTOR				speed_def ;
	FVECTOR				aim_pos ;
	TARGET				defense ;
	TARGET				offense ;
	int					count ;
	int					time ;
	int					flag ;
	float				aim_len ;
	int					phase ;

	int					smoke_flag ;
} Work ;

enum {
	FLAG_NOTHING		=		0x0000,
	FLAG_HIT			=		0x0001,
	FLAG_HITTED			=		0x0002,
} ;

/*----------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;
	if ( PL_SlashNow() ) work->flag |= FLAG_HITTED ;
	else {
		if ( !GM_CheckPlayerStatus( PLAYER_ROLLING ) ) work->flag |= FLAG_HIT ;
	}
}

static	void	Hitted( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	if ( off->weapon_type & ( WP_BLADE | WP_BLADEFAINT ) ) {
		work = ( Work * )ptr ;
		work->flag |= FLAG_HITTED ;
		GM_SeSetMode( SD_A_SWORDHIT, &work->control.mov, GM_SEMODE_BOMB ) ;
		NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work ) 
{
	CONTROL		*ctrl ;
	FVECTOR		from, diff ;
	SVECTOR		rot ;

	ctrl = ( CONTROL * )&work->control ;
	DG_COPY_VEC( &from, &ctrl->mov ) ;

	if ( work->flag & FLAG_HIT ) {
		extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;
		int				damage = DMG_BLAST ;

		/* 難易度別 */
		switch( SOL_GameLevel ) {
		case GM_LEVEL_VERYEASY :
			damage /= 2 ;
			break ;
		case GM_LEVEL_EASY :
			damage /= 2 ;
			break ;
		case GM_LEVEL_HARD :
			damage += 5 ;
			break ;
		case GM_LEVEL_EXTREME :
			damage += 5 ;
			break ;
		case GM_LEVEL_NORMAL :
		default :
		  ;
		}
		if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
		if ( damage <= 0 ) damage = 1 ;		 

		NewBlast( &from, PLAYER_SIDE, 1000, 2000, damage, FNT_BLAST, WP_C4Bomb ) ;
		work->smoke_flag = 1;
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( work->flag & FLAG_HITTED ) {
		work->smoke_flag = 1;
		NewSnakeArmMissileSlashed( &work->body.objs->world, 
								   &ctrl->step, work->body.map_name, 0 ) ;
		NewSnakeArmMissileSlashed( &work->body.objs->world, 
								   &ctrl->step, work->body.map_name, 1 ) ;
		NewBladeSpark( &work->body.objs->world );
		GV_DestroyActor( work ) ;
		return ;		
	}
	
	GM_ActControl( ctrl ) ;
	GM_ActObject( &work->body ) ;
	DG_GetLightMatrix( &ctrl->mov, work->lights ) ;

	if ( ctrl->n_touches || -- work->count <= 0 ) {
		work->flag |= FLAG_HIT ;
	}
	GM_MoveTarget( &work->defense, &ctrl->mov ) ;
	//NewTargetView2( &work->defense, 32, 232, 23 ) ;

	GM_MoveOnlineTarget( &work->offense, &from, &ctrl->mov ) ;
	GM_PutTarget( &work->offense ) ;

	switch( work->phase ) {
	case 0 :
		if ( ++ work->time >= DIRECT_TICK( 48 ) ) {

			NewSolidusMissileSmoke( &work->control.mov, 
									&work->control.step,
									200.0F, &work->smoke_flag );

			GM_SeSetMode( SD_W_MISILE03, &ctrl->mov, GM_SEMODE_BOMB ) ;
			work->control.turn.vx -= 160 ;
			work->phase = 1 ;
		}
		break ;
	case 1 :
		_sceVu0SubVector( &diff, &GM_PlayerFindPos, &ctrl->mov ) ;
		work->aim_len = GV_VecLen3F( &diff ) ;
		GV_VecToRot( &diff, &rot ) ;	
		if ( GV_DiffDirAbs( ctrl->rot.vy, rot.vy ) < 1024 ) {
			int		speed = 8 ;
			switch( SOL_GameLevel ) {
			case GM_LEVEL_VERYEASY :
			case GM_LEVEL_EASY :
				speed = 6 ;
				break ;
			case GM_LEVEL_HARD :
				speed = 16 ;
				break ;
			case GM_LEVEL_EXTREME :
				speed = 32 ;
				break ;
			case GM_LEVEL_NORMAL :
			default :
			  ;
			}
			work->control.turn.vy = GV_NearSpeedP( work->control.turn.vy, rot.vy, speed ) ;
			if ( GV_DiffDirS( work->control.turn.vx - 1024, rot.vx ) > 0 ) {
				work->control.turn.vx = GV_NearSpeedP( work->control.turn.vx, rot.vx + 1024, 16 ) ;
			}
		}
		if ( GV_DiffDirS( work->control.turn.vx, 1000 + 1024 ) < 0 ) {
			work->control.turn.vx = 1000 + 1024 ;
		}
		DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
		DG_RotVector( &work->speed_def, &ctrl->step, 1 ) ;

		DG_SetTmpLight2( &ctrl->mov, 1000.0F, 2000.0F, 
						 32 | ( 96 << 8 )  | ( 232 << 16 ), 
						 LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
	}

//printf( "----\n" ) ;
//DumpVec( &ctrl->step ) ;
//DumpSVec( &ctrl->rot ) ;

}

static	void	Die( Work *work )
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->defense ) ;
}

/*----------------------------------------------------------------*/

static	int	InitControl( Work *work, FMATRIX *world, int map )
{
	CONTROL		*ctrl ;
	FVECTOR		dir = { 0.0F, 10.0F, 0.0F } ;
	FVECTOR		mov ;
	SVECTOR		rot ;

	GV_SetVec3( &work->speed_def0, 0.0F, 12.0F, 0.0F ) ;
	GV_SetVec3( &work->speed_def, 0.0F, 112.0F, 0.0F ) ;
   if ( BP_IsPAL()==TRUE )
   {
      work->speed_def0.vy *= 1.20F ;
	   work->speed_def.vy *= 1.20F ;
   }

	ctrl = ( CONTROL * )&work->control ;
	if ( GM_InitControl( ctrl, GV_StrCode( "蛇手ミサイル" ), map ) < 0 ) return -1 ;
	DG_SetPos( world ) ;
	DG_RotVector( &dir, &dir, 1 ) ;
	DG_RotVector( &work->speed_def0, &ctrl->step, 1 ) ;
	ctrl->step.vy = -6.0F ;
	//DG_RotVector( &work->speed_def, &ctrl->step, 1 ) ;

	GV_VecToRot( &dir, &rot ) ;
	GV_MatToVec( world, &mov ) ;
	rot.vx += 1024 ;
	GM_ConfigControlPosition( ctrl, &mov, &rot ) ;
	GM_ConfigControlHazard( ctrl, 200, 500, 500 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_IK ;
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	return 0 ;
}

static	int	InitObject( Work *work, FMATRIX *world, int map )
{
	OBJECT	   	*body ;

	body = &work->body ;
	GM_InitObject( body, BODY_NAME, BODY_FLAG ) ;
	if ( body->objs == NULL ) return -1 ;
	GM_ConfigObjectLight( body, work->lights ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActObject( body ) ;
	return 0 ;
}

static	void	InitTarget( Work *work, FMATRIX *world, int map )
{
	TARGET		*t ;
	FVECTOR		size ;

	t = &work->defense ;
	GV_SetVec3( &size, 350.0F, 400.0F, 350.0F ) ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER, map, ENEMY_SIDE,
				  &size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, Hitted, work ) ;
	GM_MoveTarget3Map( t, world, map ) ;
	GM_PutTarget( t ) ;

	t = &work->offense ;
	GM_SetTarget( t, TARGET_OFFENSE|TARGET_ONLINE|TARGET_ONLINE_MIN|
				  TARGET_POWER|TARGET_THROUGH, map, PLAYER_SIDE, 
				  &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
}

static	int	GetResources( Work *work, FMATRIX *world, int map )
{
	GM_SetCurrentMap( map ) ;
	if ( InitControl( work, world, map ) < 0 ) return -1 ;
	if ( InitObject( work, world, map ) < 0 ) return -1 ;
	InitTarget( work, world, map ) ;

	DG_COPY_VEC( &work->aim_pos, &GM_PlayerFindPos ) ;
	work->aim_len = GV_VecLen3F2( &work->control.mov, &work->aim_pos ) ;

	work->count = 5 * ( 300 / TIME_BASE ) ;
	work->time = 0 ;

	GM_SeSetMode( SD_E_TMISSTA1, &work->control.mov, GM_SEMODE_BOMB ) ;

#if 1
	GV_SetActorChild( work, NewSolidusMissileFire( 
						      (FVECTOR *)work->body.objs->world.m[3],
							  (FVECTOR *)work->body.objs->world.m[1],
							  &work->phase ) ) ;
#else
	GV_SetActorChild( work, NewRisingSmoke( &work->control.mov, 250.0F ) ) ;
#endif

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewSnakeArmMissile( FMATRIX *world, int map )
{
	Work			*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, world, map ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}




