//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	missile.c
	
	2002/7/16 Y.Korekado
	$Id: dummytrg.c,v 1.3 2002/11/23 12:42:26 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
/*----------------------------------------------------------------*/
#define PRIO	0x40
#define TARGET_CLASS	(TARGET_LOCKON|TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	TARGET		deftrg ;	/* 防御ターゲット */
	FVECTOR		pos ;
	FVECTOR		size ;
	int			end_proc ;
	int			status ;
} Work ;

/*----------------------------------------------------------------*/
#define DUMMY_TRG_STATUS_BULLET		0x00000001
#define DUMMY_TRG_STATUS_VISIBLE	0x00000002
/*----------------------------------------------------------------*/
static	int	Damage( Work *work )
{
	TARGET		*trg  ;
	
	trg = &work->deftrg ;

	if ( TARGET_POWER & trg->damaged ) {
		if ( trg->weapon_type  & (WP_BLAST) ) {
			if ( work->end_proc != 0 ) {
				GM_ExecProc( work->end_proc, NULL ) ;
			}
			trg->weapon_type = 0 ;
			trg->damaged = 0 ;
			return 1 ;
		}
		if ( trg->weapon_type  & (WP_BULLET|WP_M92) ) {
			if( work->status & DUMMY_TRG_STATUS_BULLET ) {
				if ( work->end_proc != 0 ) {
					GM_ExecProc( work->end_proc, NULL ) ;
				}
				trg->weapon_type = 0 ;
				trg->damaged = 0 ;
				return 1 ;
			}
		}
		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if ( Damage( work ) ) {
		GV_DestroyActor( work ) ;
		return ;
	}
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeTarget( &work->deftrg ) ;
}
/*----------------------------------------------------------------*/
static void	InitTarget( Work *work )
{
	TARGET		*trg  ;
	FVECTOR 	shift ;

	trg = &work->deftrg ;

	shift.vx = 0.0 ;	shift.vy = 0.0 ;	shift.vz = 0.0 ;

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &work->size, &shift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
	GM_PutTarget( trg ) ;

	GM_MoveTarget( trg, &work->pos ) ;
#ifdef DEBUG_MODE
	if ( work->status & DUMMY_TRG_STATUS_VISIBLE ) {
		NewTargetView( trg, 200, 34, 184 ) ;
	}
#endif
}

static	int	GetResources( Work *work )
{
	int	vec[ 3 ] ;

	if ( GCL_GetOption( 'p' ) != NULL ) {
		if ( GCL_NextStr() != NULL ){
			GCL_GetNextIV( vec ) ;
			work->pos.vx = (float)vec[0] ;
			work->pos.vy = (float)vec[1] ;
			work->pos.vz = (float)vec[2] ;
		} else {
			return -1 ;
		}
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		if ( GCL_NextStr() != NULL ){
			GCL_GetNextIV( vec ) ;
			work->size.vx = (float)vec[0] ;
			work->size.vy = (float)vec[1] ;
			work->size.vz = (float)vec[2] ;
		} else {
			work->size.vx = 1000.0 ;
			work->size.vy = 1000.0 ;
			work->size.vz = 1000.0 ;
		}
	}
	work->status = 0 ;
	if ( GCL_GetOption( 'b' ) != NULL ) {
		work->status |= DUMMY_TRG_STATUS_BULLET ;
	}
	if ( GCL_GetOption( 'v' ) != NULL ) {
		work->status |= DUMMY_TRG_STATUS_VISIBLE ;
	}

	work->end_proc = GCL_GetOptionValue( 'e', 0 ) ;

	InitTarget( work ) ;

	return 0 ;
}

void	*NewDummyTarget( name, where )
{
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
