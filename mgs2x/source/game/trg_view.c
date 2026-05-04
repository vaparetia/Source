//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   trg_view.c 
   デバッグ用ターゲットビュワー
   
   1999/07/23 M.Sonoyama
   $Id: trg_view.c,v 1.1.1.3 2002/11/19 11:41:57 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE

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

#define	PRIM_TYPE	(DG_PRIM2_LINE | DG_PRIM2_SHADE)
#define	N_PRIMS		(1)
#define	N_VERTS		(24)

typedef	struct	{
    GV_ACT_EX		actor ;
    TARGET		*targ ;
    DG_PRIM2		*prim ;
    int			mode ;
	int			clock ;
} Work ;

static	void	UpdatePos( work )
Work		*work ;
{
    TARGET		*t ;
    DG_PRIM2		*prim ;
    DG_PRIM2_PACKET	*pkt ;
    float		loc[ 6 ] ;
    FVECTOR		*pos ;
    FVECTOR		center, offset ;
    FMATRIX		mat ;
    int			i ;

    prim = work->prim ;
    DG_SwitchBuffPrim2( prim ) ;
    pos = prim->pos[ prim->buffer_clock ] ;
    pkt = prim->packet[ prim->buffer_clock ] ;
    t = work->targ ;
    GM_GroupPrim2( prim, t->map ) ;

    if ( t->class & TARGET_ROTATE ) {
		_sceVu0ApplyMatrix( &offset, &( t->world ), &( t->offset ) ) ;
    } else {
		offset = t->offset ;
    }

    center.vx = t->center.vx + offset.vx ;
    center.vy = t->center.vy + offset.vy ;
    center.vz = t->center.vz + offset.vz ;

    loc[ 0 ] = center.vx - t->size.vx ;
    loc[ 1 ] = center.vx + t->size.vx ;
    loc[ 2 ] = center.vy - t->size.vy ;
    loc[ 3 ] = center.vy + t->size.vy ;
    loc[ 4 ] = center.vz - t->size.vz ;
    loc[ 5 ] = center.vz + t->size.vz ;
    
    pos[ 0 ].vx = pos[ 7 ].vx = loc[ 0 ] ; 
    pos[ 0 ].vy = pos[ 7 ].vy = loc[ 2 ] ; 
    pos[ 0 ].vz = pos[ 7 ].vz = loc[ 4 ] ;
    pos[ 1 ].vx = pos[ 2 ].vx = loc[ 1 ] ; 
    pos[ 1 ].vy = pos[ 2 ].vy = loc[ 2 ] ; 
    pos[ 1 ].vz = pos[ 2 ].vz = loc[ 4 ] ;
    pos[ 3 ].vx = pos[ 4 ].vx = loc[ 1 ] ; 
    pos[ 3 ].vy = pos[ 4 ].vy = loc[ 2 ] ; 
    pos[ 3 ].vz = pos[ 4 ].vz = loc[ 5 ] ;
    pos[ 5 ].vx = pos[ 6 ].vx = loc[ 0 ] ; 
    pos[ 5 ].vy = pos[ 6 ].vy = loc[ 2 ] ; 
    pos[ 5 ].vz = pos[ 6 ].vz = loc[ 5 ] ;

    pos += 8 ;
    pos[ 0 ].vx = pos[ 7 ].vx = loc[ 0 ] ; 
    pos[ 0 ].vy = pos[ 7 ].vy = loc[ 3 ] ; 
    pos[ 0 ].vz = pos[ 7 ].vz = loc[ 4 ] ;
    pos[ 1 ].vx = pos[ 2 ].vx = loc[ 1 ] ; 
    pos[ 1 ].vy = pos[ 2 ].vy = loc[ 3 ] ; 
    pos[ 1 ].vz = pos[ 2 ].vz = loc[ 4 ] ;
    pos[ 3 ].vx = pos[ 4 ].vx = loc[ 1 ] ; 
    pos[ 3 ].vy = pos[ 4 ].vy = loc[ 3 ] ; 
    pos[ 3 ].vz = pos[ 4 ].vz = loc[ 5 ] ;
    pos[ 5 ].vx = pos[ 6 ].vx = loc[ 0 ] ; 
    pos[ 5 ].vy = pos[ 6 ].vy = loc[ 3 ] ; 
    pos[ 5 ].vz = pos[ 6 ].vz = loc[ 5 ] ;

    pos += 8 ;
    pos[ 0 ].vx = loc[ 0 ] ; 
    pos[ 0 ].vy = loc[ 2 ] ; 
    pos[ 0 ].vz = loc[ 4 ] ;
    pos[ 1 ].vx = loc[ 0 ] ; 
    pos[ 1 ].vy = loc[ 3 ] ; 
    pos[ 1 ].vz = loc[ 4 ] ;

    pos[ 2 ].vx = loc[ 1 ] ; 
    pos[ 2 ].vy = loc[ 2 ] ; 
    pos[ 2 ].vz = loc[ 4 ] ;
    pos[ 3 ].vx = loc[ 1 ] ; 
    pos[ 3 ].vy = loc[ 3 ] ; 
    pos[ 3 ].vz = loc[ 4 ] ;

    pos[ 4 ].vx = loc[ 1 ] ; 
    pos[ 4 ].vy = loc[ 2 ] ; 
    pos[ 4 ].vz = loc[ 5 ] ;
    pos[ 5 ].vx = loc[ 1 ] ; 
    pos[ 5 ].vy = loc[ 3 ] ; 
    pos[ 5 ].vz = loc[ 5 ] ;

    pos[ 6 ].vx = loc[ 0 ] ; 
    pos[ 6 ].vy = loc[ 2 ] ; 
    pos[ 6 ].vz = loc[ 5 ] ;
    pos[ 7 ].vx = loc[ 0 ] ; 
    pos[ 7 ].vy = loc[ 3 ] ; 
    pos[ 7 ].vz = loc[ 5 ] ;

    pos = prim->pos[ prim->buffer_clock ] ;
    for ( i = 0; i < N_VERTS; i ++, pos ++ ) {
		_sceVu0SubVector( pos, pos, &center ) ;
    }
    if ( t->class & TARGET_ROTATE ) {
		_sceVu0CopyMatrix( &mat, &( t->world ) ) ;
    } else {
		mat = DG_UnitMatrix ;
    }
    mat.m[ 3 ][ 0 ] = center.vx ;
    mat.m[ 3 ][ 1 ] = center.vy ;
    mat.m[ 3 ][ 2 ] = center.vz ;
    DG_SetPos( &mat ) ;
    pos = prim->pos[ prim->buffer_clock ] ;
    DG_PutVector( pos, pos, N_VERTS ) ;
}

static	void	Act( work )
Work		*work ;
{
    if ( work->mode == 0 ) {
		UpdatePos( work ) ;
//		DG_SetPos( &DG_UnitMatrix ) ;
//		DG_PutPrim2( work->prim ) ;
		DG_COPY_MAT( &work->prim->world, &DG_UnitMatrix ) ;
		DG_VisiblePrim2( work->prim ) ;
    } else {
		if ( work->clock != GV_Time ) {
			GM_FreePrim2( work->prim ) ;
			work->clock = -1 ;
			GV_DestroyActor( work ) ;
		}
    }
}

static	void	Die( work )
Work		*work ;
{
	if ( work->clock > 0 ) GM_FreePrim2( work->prim ) ;
}

static	int	GetResources( work, targ, r, g, b )
Work		*work ;
TARGET		*targ ;
u_char		r, g, b ;
{
    DG_PRIM2		*prim ;
    DG_PRIM2_UVRGB	*rgb ;
    int			i, j, n ;

	GM_SetCurrentMap( targ->map ) ;
    prim = work->prim = GM_MakePrim2( PRIM_TYPE, N_PRIMS, N_VERTS ) ;
    if ( prim == NULL ) return -1 ;
    for ( i = 0; i < 2; i ++ ) {
		rgb = prim->uvrgb[ i ] ;
		for ( n = 0; n < N_PRIMS; n ++ ) {
			for ( j = 0; j < N_VERTS; j ++ ) {
				rgb[ n * N_VERTS + j ].r = r ;
				rgb[ n * N_VERTS + j ].g = g ;
				rgb[ n * N_VERTS + j ].b = b ;
				rgb[ n * N_VERTS + j ].a = 128 ;
				rgb[ n * N_VERTS + j ].q = 4096 ;
				if ( j & 1 ) rgb[ n * N_VERTS + j ].f = 0x0fff ;
				else	 rgb[ n * N_VERTS + j ].f = 0x8fff ;
			}
		}
    }
    work->targ = targ ;
    UpdatePos( work ) ;
    prim->world = DG_UnitMatrix ;
    return 0 ;
}

void	*NewTargetView2( targ, r, g, b )
TARGET			*targ ;
u_char			r, g, b ;
{
    Work		*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, targ, r, g, b ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->mode = 1 ;
		work->clock = GV_Time ;
    }
    return work ;
}

void	*NewTargetView( targ, r, g, b )
TARGET			*targ ;
u_char			r, g, b ;
{
    Work		*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, targ, r, g, b ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->mode = 0 ;
		work->clock = GV_Time ;
    }
    return work ;
}

#endif
