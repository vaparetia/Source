//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   trp_view.c 
   デバッグ用トラップビュワー
   
   1999/12/14 M.Sonoyama
   $Id: trp_view.c,v 1.1.1.3 2002/11/19 11:42:50 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#define	PRIM_TYPE	(DG_PRIM2_LINE | DG_PRIM2_SHADE)
#define	N_PRIMS		(HZX_MAX_TRAPS)
#define	N_VERTS		(24)

typedef	struct	{
    GV_ACT		actor ;
    CONTROL		*ctrl ;
    DG_PRIM2		*prim ;
    int			*sw ;
} Work ;

static	void	SetPrimPos( blk, trp, n, pos )
HZX_BLOCK	*blk ;
HZX_TRP		*trp ;
int		n ;
FVECTOR		*pos ;
{
    float	loc[ 6 ] ;

    loc[ 0 ] = trp->b1.vx + blk->tx ;
    loc[ 1 ] = trp->b2.vx + blk->tx ;
    loc[ 2 ] = trp->b1.vy + blk->ty ;
    loc[ 3 ] = trp->b2.vy + blk->ty ;
    loc[ 4 ] = trp->b1.vz + blk->tz ;
    loc[ 5 ] = trp->b2.vz + blk->tz ;
    
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

#ifdef HZX_DTRP
static	void	SetPrimPos2( trp, n, pos )
HZX_D_TRP		*trp ;
int				n ;
FVECTOR			*pos ;
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
#endif

static	void	UpdatePos( work )
Work		*work ;
{
    CONTROL	*ctrl ;
    DG_PRIM2	*prim ;
    DG_PRIM2_PACKET	*pkt ;
    FVECTOR	*pos, b1, b2 ;
    int		i, n_insides, n ;
    u_int	*inside ;
    HZX_BLOCK	*blk ;	
    HZX_TRP	*traps, *trp ;

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
    n_insides = ctrl->evt.n_inside ;
    inside = ctrl->evt.inside ;

    blk = HZX_GetInsideBlock( ctrl->hzx_id, &ctrl->mov ) ;
    //ASSERT( blk != NULL ) ;	
	if ( blk == NULL ) return ;
#if 0
	if ( blk == NULL ) {
		int			gNo ;
		HZX_GRP		*group ;

		gNo = GV_GetNo( ctrl->map ) ;
		group = HZX_CurrentHzx->def->groups + gNo ;
		printf( "GROUP %d\n", gNo ) ;
		printf( "MOV %.1f %.1f %.1f\n",
			   ctrl->mov.vx, ctrl->mov.vy, ctrl->mov.vz ) ;
		printf( "GRP BOUND MIN %d %d %d\n",
			   group->bound_min_x,	
			   group->bound_min_y,	
			   group->bound_min_z ) ;
		printf( "GRP BOUND MAX %d %d %d\n",
			   group->bound_min_x + 
			   group->block_size_x * 
			   group->div_x,
			   group->bound_min_y + 
			   group->block_size_y * 
			   group->div_y,
			   group->bound_min_z + 
			   group->block_size_z * 
			   group->div_z ) ;
		return ;
	}
#endif
    trp = NULL ; traps = blk->traps ;
    n = 0 ;
    DEBUG_Locate( 300, 120, 0 ) ;
    while( -- n_insides >= 0 ) {
		for ( i = 0; i < blk->n_traps; i ++, traps ++ ) {
			b1.vx = blk->tx + traps->b1.vx ;
			b1.vy = blk->ty + traps->b1.vy ;
			b1.vz = blk->tz + traps->b1.vz ;
			b2.vx = blk->tx + traps->b2.vx ;
			b2.vy = blk->ty + traps->b2.vy ;
			b2.vz = blk->tz + traps->b2.vz ;
			if ( traps->name_id == *inside ) {
				if ( ctrl->root_offset == NULL || 
					 ctrl->link_zone != NULL ) {
					if ( b1.vx > ctrl->mov.vx || ctrl->mov.vx > b2.vx ||
						 b1.vy > ctrl->mov.vy || ctrl->mov.vy > b2.vy ||
						 b1.vz > ctrl->mov.vz || ctrl->mov.vz > b2.vz ) {
						continue ;
					}
				} else {
					FVECTOR		chk_mov ;
					_sceVu0AddVector( &chk_mov, &ctrl->mov, ctrl->root_offset ) ;
					if ( b1.vx > chk_mov.vx || chk_mov.vx > b2.vx ||
						 b1.vy > chk_mov.vy || chk_mov.vy > b2.vy ||
						 b1.vz > chk_mov.vz || chk_mov.vz > b2.vz ) {
						continue ;
					}
				}
				DEBUG_Printf( "trap[ %d ] : %d\n", n, *inside ) ;
				if ( GV_PadData->press & PAD_L1 ) {
					printf( "%d %d %d - ", blk->tx + traps->b1.vx,
						   blk->ty + traps->b1.vy, blk->tz + traps->b1.vz ) ;
					printf( "%d %d %d\n", blk->tx + traps->b2.vx,
						   blk->ty + traps->b2.vy, blk->tz + traps->b2.vz ) ;
					printf( "\n" ) ;
				}
				trp = traps ;
				break ;
			}
		}	
		if ( trp != NULL ) {
			pkt[ n ].flag &= ~DG_PRIM2_INVISIBLE ;
			SetPrimPos( blk, trp, n, pos ) ;
			pos += N_VERTS ; n ++ ;
		} else {
#ifdef HZX_DTRP
			HZX_D_TRP	*dtrps, *dtrp ;
			FVECTOR		b1, b2 ;

			dtrp = NULL ;
			dtrps = HZX_GetGroup( ctrl->hzx_id )->dynamics->traps ;
			while( dtrps != NULL ) {
				DG_COPY_VEC( &b1, &dtrps->b1 ) ;
				DG_COPY_VEC( &b2, &dtrps->b2 ) ;
				if ( dtrps->name_id == *inside &&
					b1.vx <= ctrl->mov.vx && ctrl->mov.vx < b2.vx &&
					b1.vy <= ctrl->mov.vy && ctrl->mov.vy < b2.vy &&
					b1.vz <= ctrl->mov.vz && ctrl->mov.vz < b2.vz ) {
					DEBUG_Printf( "trap[ %d ] : %d\n", n, *inside ) ;
					dtrp = dtrps ;
					break ;
				}
				dtrps = dtrps->next ;
			}
			if ( dtrp != NULL ) {
				pkt[ n ].flag &= ~DG_PRIM2_INVISIBLE ;
				SetPrimPos2( dtrp, n, pos ) ;
				pos += N_VERTS ; n ++ ;
			}
#endif
		}
		inside ++ ; 
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

void	*NewTrapView( ctrl, r, g, b, sw )
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
