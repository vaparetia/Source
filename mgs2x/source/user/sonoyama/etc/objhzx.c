//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   objhzx.c
   オブジェクトに自動で壁をつける
   
   2000/08/04 M.Sonoyama
   $Id: objhzx.c,v 1.1.1.3 2002/11/19 11:50:44 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

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

typedef struct {
    GV_ACT_EX 		actor ;
    HZX_D_SEGMENT	*segments[ 4 ] ;
	HZX_D_FLOOR		*floors[ 2 ] ;
    int				atr ;
	int				map ;
} Work;

/*------------------------------------------------------------*/

static	void	UpdateHazard( Work *work, FMATRIX *world )
{
	int			i ;
	IVECTOR		shift ;

	shift.vx = ( int )world->m[ 3 ][ 0 ] ;
	shift.vy = ( int )world->m[ 3 ][ 1 ] ;
	shift.vz = ( int )world->m[ 3 ][ 2 ] ;
	for ( i = 0; i < 4; i ++ ) {
		HZX_RotateDynamicSegment2( work->segments[ i ], &DG_ZeroIVector, world ) ;
		HZX_ShiftDynamicSegment( work->segments[ i ], &shift ) ;
	}
	for ( i = 0; i < 2; i ++ ) {
		HZX_RotateDynamicFloor2( work->floors[ i ], &DG_ZeroIVector, world ) ;
		HZX_ShiftDynamicFloor( work->floors[ i ], &shift ) ;
	}
}

/*------------------------------------------------------------*/

static 	void 	Act( work )
Work		*work ;
{
#ifdef DEBUG_MODE
#if 0
	GM_SetCurrentMap( work->map ) ;
	HZX_ViewDynamicSegment( work->segments[ 0 ] ) ;
	HZX_ViewDynamicSegment( work->segments[ 1 ] ) ;
	HZX_ViewDynamicSegment( work->segments[ 2 ] ) ;
	HZX_ViewDynamicSegment( work->segments[ 3 ] ) ;
	HZX_ViewDynamicFloor( work->floors[ 0 ], 4 ) ;
	HZX_ViewDynamicFloor( work->floors[ 1 ], 4 ) ;
#endif
#endif
}

static	void	Die( work )
Work		*work ;
{
	int		i ;

	for ( i = 0; i < 4; i ++ ) {
		if ( work->segments[ i ] != NULL ) {
			HZX_RemoveDynamicSegment( work->segments[ i ] ) ;
		}
	}
	for ( i = 0; i < 2; i ++ ) {
		if ( work->floors[ i ] != NULL ) {
			HZX_RemoveDynamicFloor( ( HZX_D_FLOOR * )work->floors[ i ] ) ;
		}
	}
}

static	int	GetResources( work, hzx_id, objs, seg_atr, flr_atr, disp, scale )
Work			*work ;
HZX_GROUP_ID	hzx_id ;
DG_OBJS			*objs ;
u_int			seg_atr, flr_atr, disp ;
FVECTOR			*scale ;
{
	IVECTOR			v[ 4 ] ;
	DG_DEF			*def ;
	int				i, h ;

	def = objs->def ;
	if ( seg_atr & HZX_SEG_RECOIL_TYPE ) {
		seg_atr &= ~HZX_SEG_RECOIL_TYPE ;
		GV_InitVec3( &v[ 0 ], 
					objs->objs[ 0 ].bound_min.vx, 
					objs->objs[ 0 ].bound_min.vy, 
					objs->objs[ 0 ].bound_min.vz ) ;
		GV_InitVec3( &v[ 1 ], 
					objs->objs[ 0 ].bound_max.vx, 
					objs->objs[ 0 ].bound_min.vy, 
					objs->objs[ 0 ].bound_min.vz ) ;
		GV_InitVec3( &v[ 2 ], 
					objs->objs[ 0 ].bound_min.vx, 
					objs->objs[ 0 ].bound_min.vy, 
					objs->objs[ 0 ].bound_max.vz ) ;
		GV_InitVec3( &v[ 3 ], 
					objs->objs[ 0 ].bound_max.vx, 
					objs->objs[ 0 ].bound_max.vy, 
					objs->objs[ 0 ].bound_max.vz ) ;
	} else {
		GV_InitVec3( &v[ 0 ], def->lx, def->ly, def->lz ) ;
		GV_InitVec3( &v[ 1 ], def->ux, def->ly, def->lz ) ;	
		GV_InitVec3( &v[ 2 ], def->lx, def->ly, def->uz ) ;	
		GV_InitVec3( &v[ 3 ], def->ux, def->uy, def->uz ) ;
	}
	if ( scale != NULL ) {
		v[ 0 ].vx = ( int )( ( float )v[ 0 ].vx * scale->vx ) ;
		v[ 0 ].vz = ( int )( ( float )v[ 0 ].vz * scale->vz ) ;
		v[ 1 ].vx = ( int )( ( float )v[ 1 ].vx * scale->vx ) ;
		v[ 1 ].vz = ( int )( ( float )v[ 1 ].vz * scale->vz ) ;
		v[ 2 ].vx = ( int )( ( float )v[ 2 ].vx * scale->vx ) ;
		v[ 2 ].vz = ( int )( ( float )v[ 2 ].vz * scale->vz ) ;
		v[ 3 ].vx = ( int )( ( float )v[ 3 ].vx * scale->vx ) ;
		v[ 3 ].vy = ( int )( ( float )v[ 3 ].vy * scale->vy ) ;
		v[ 3 ].vz = ( int )( ( float )v[ 3 ].vz * scale->vz ) ;
	}
	
	h = v[ 3 ].vy - v[ 0 ].vy ; v[ 3 ].vy = v[ 0 ].vy ;
	for ( i = 0; i < 4; i ++ ) v[ i ].vw = h ;

	work->segments[ 0 ] = HZX_AddDynamicSegment( hzx_id, &v[ 0 ], &v[ 1 ], seg_atr ) ;
	work->segments[ 1 ] = HZX_AddDynamicSegment( hzx_id, &v[ 0 ], &v[ 2 ], seg_atr ) ;
	work->segments[ 2 ] = HZX_AddDynamicSegment( hzx_id, &v[ 3 ], &v[ 1 ], seg_atr ) ;
	work->segments[ 3 ] = HZX_AddDynamicSegment( hzx_id, &v[ 3 ], &v[ 2 ], seg_atr ) ;

	if ( disp ) {
		HZX_DynamicSegmentSetAttribute( work->segments[ 0 ], seg_atr ) ;
		HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], seg_atr ) ;
		HZX_DynamicSegmentSetAttribute( work->segments[ 2 ], seg_atr ) ;
		HZX_DynamicSegmentSetAttribute( work->segments[ 3 ], seg_atr ) ;
	}
	
	work->floors[ 0 ] = HZX_AddDynamicFloor( hzx_id, &v[ 0 ], &v[ 1 ], &v[ 3 ], &v[ 2 ],
											 4, flr_atr ) ;
	for ( i = 0; i < 4; i ++ ) v[ i ].vy += h ;
	work->floors[ 1 ] = HZX_AddDynamicFloor( hzx_id, &v[ 0 ], &v[ 1 ], &v[ 3 ], &v[ 2 ],
											 4, flr_atr ) ;	

	UpdateHazard( work, &objs->world ) ;

	work->map = GM_CurrentMap ;

	GV_SleepActor( work, GV_CLASS_WAITING ) ;

	return 0 ;
}

/* オブジェクトのバウンディングに沿ってハザード生成 
   起動時はＸＺ回転は０でなければならない。*/
void 			*NewMakeObjectBoundHazard3( hzx_id, objs, seg_atr, flr_atr, disp, scale )
HZX_GROUP_ID	hzx_id ;
DG_OBJS			*objs ;
u_int			seg_atr, flr_atr, disp ;
FVECTOR			*scale ;
{
    Work *work;

    work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, hzx_id, objs, seg_atr, flr_atr, disp, scale ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;	    
		}
    }
    return ( void * )work ;
}

void 			*NewMakeObjectBoundHazard( hzx_id, objs, seg_atr, flr_atr )
HZX_GROUP_ID	hzx_id ;
DG_OBJS			*objs ;
u_int			seg_atr, flr_atr ;
{
	return NewMakeObjectBoundHazard3( hzx_id, objs, seg_atr, flr_atr, 0, NULL ) ;
}

void 			*NewMakeObjectBoundHazard2( hzx_id, objs, seg_atr, flr_atr, disp )
HZX_GROUP_ID	hzx_id ;
DG_OBJS			*objs ;
u_int			seg_atr, flr_atr ;
int				disp ;
{
	return NewMakeObjectBoundHazard3( hzx_id, objs, seg_atr, flr_atr, disp, NULL ) ;
}

/*--------------------------------------------------------------------------*/

void			PL_UpdateObjectBoundHazard( void *work, FMATRIX *world )
{
	UpdateHazard( ( Work * )work, world ) ;
}


void	PL_ObjectBoundHazardSetAttribute( void *ptr, int seg_atr, int flr_atr )
{
	int			i ;
	Work		*work ;

	work = ( Work * )ptr ;

	for ( i = 0; i < 4; i ++ ) {
		HZX_DynamicSegmentSetAttribute( work->segments[ i ], 
									    work->segments[ i ]->atr | seg_atr ) ;
	}
	for ( i = 0; i < 2; i ++ ) {
		HZX_DynamicFloorSetAttribute( work->floors[ i ], 
									  work->floors[ i ]->atr | flr_atr ) ;
	}
}

void	PL_ObjectBoundHazardResetAttribute( void *ptr, int seg_atr, int flr_atr )
{
	int			i ;
	Work		*work ;

	work = ( Work * )ptr ;

	for ( i = 0; i < 4; i ++ ) {
		HZX_DynamicSegmentSetAttribute( work->segments[ i ], 
									    work->segments[ i ]->atr & ~seg_atr ) ;
	}
	for ( i = 0; i < 2; i ++ ) {
		HZX_DynamicFloorSetAttribute( work->floors[ i ], 
									  work->floors[ i ]->atr & ~flr_atr ) ;
	}
}

void	PL_ObjectBoundHazardSetTarget( void *ptr, TARGET *target ) 
{
	int			i ;
	Work		*work ;

	work = ( Work * )ptr ;

	for ( i = 0; i < 4; i ++ ) {
		GM_SetTargettoDynamicHazard( work->segments[ i ], target, HZX_TYPE_SEGMENT ) ;
	}
	for ( i = 0; i < 2; i ++ ) {
		GM_SetTargettoDynamicHazard( work->floors[ i ], target, HZX_TYPE_FLOOR ) ;
	}	
}
