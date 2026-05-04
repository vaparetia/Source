//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bhd_view.c 
   デバッグ用ビハインドビュワー

   1999/12/14 M.Sonoyama
   $Id: bhd_view.c,v 1.1.1.3 2002/11/19 11:42:46 Yoshizawa1 Exp $
*/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef PSX2
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
#define	N_PRIMS		(2)
#define	N_VERTS		(24)

typedef	struct	{
    GV_ACT		actor ;
    CONTROL		*ctrl ;
    DG_PRIM2		*prim ;
    int			*sw ;
} Work ;

static	void	SetPrimPos( trp, n, pos )
HZX_BEHIND	*trp ;
int		n ;
FVECTOR		*pos ;
{
    float	loc[ 6 ] ;

    loc[ 0 ] = trp->b1.vx ;
    loc[ 1 ] = trp->b2.vx ;
    loc[ 2 ] = trp->b1.vy ;
    loc[ 3 ] = trp->b2.vy ;
    loc[ 4 ] = trp->b1.vz ;
    loc[ 5 ] = trp->b2.vz ;
    
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
}

static	void	UpdatePos( work )
Work		*work ;
{
    CONTROL	*ctrl ;
    DG_PRIM2	*prim ;
    DG_PRIM2_PACKET	*pkt ;
    FVECTOR	*pos ;
    int		i, n ;
    HZX_BEHIND	*bhs[ 2 ] ;
    int		n_bhs ;

    prim = work->prim ;
    DG_SwitchBuffPrim2( prim ) ;
    pos = prim->pos[ prim->buffer_clock ] ;
    pkt = prim->packet[ prim->buffer_clock ] ;
    for ( i = 0; i < N_PRIMS; i ++ ) {
	pkt[ i ].flag |= DG_PRIM2_INVISIBLE ;	
    }
    if ( *work->sw == 0 ) return ;

    ctrl = work->ctrl ;
    if ( ctrl == NULL ) {
	GV_DestroyActor( work ) ;
	return ;
    }

    GM_GroupPrim2( prim, ctrl->map ) ;
DEBUG_Locate( 64, 240, 0 );
    n_bhs = HZX_CheckBehind( ctrl->hzx_id, bhs, &ctrl->mov, &ctrl->rot ) ;
    for ( n = 0; n < n_bhs; n ++ ) {
	pkt[ n ].flag &= ~DG_PRIM2_INVISIBLE ;
DEBUG_Printf( "v%08X f%08X\n", bhs[ n ]->value, bhs[ n ]->flag );
	SetPrimPos( bhs[ n ], n, pos ) ;
	pos += N_VERTS ;
    }
}

static	void	Act( work )
Work		*work ;
{
    UpdatePos( work ) ;
    work->prim->as_world = DG_UnitMatrix ;
}

static	void	Die( work )
Work		*work ;
{
    GM_FreePrim2( work->prim ) ;
}

static	int	GetResources( work, ctrl, r, g, b )
Work		*work ;
CONTROL		*ctrl ;
u_char		r, g, b ;
{
    DG_PRIM2		*prim ;
    DG_PRIM2_UVRGB	*rgb ;
    int			i, j, n ;

    prim = work->prim = GM_MakePrim2( PRIM_TYPE, N_PRIMS, N_VERTS ) ;
    prim->raise = 1000 ;
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
    work->ctrl = ctrl ;
    UpdatePos( work ) ;
    return 0 ;
}

void	*NewBehindView( ctrl, r, g, b, sw )
CONTROL			*ctrl ;
u_char			r, g, b ;
int			*sw ;
{
    Work		*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	work->sw = sw ;
	if ( GetResources( work, ctrl, r, g, b ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
