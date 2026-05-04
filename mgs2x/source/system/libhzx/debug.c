//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   debug.c
   デバッグ用アクターあれこれ
   
   2000/02/10 M.Sonoyama
   $Id: debug.c,v 1.1.1.3 2002/11/19 11:42:46 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE

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
#define	PRIM_TYPE2	(DG_PRIM2_POLY | DG_PRIM2_SHADE | DG_PRIM2_ALPHA)
#define	PRIM_TYPE3	(DG_PRIM2_POLY | DG_PRIM2_SHADE)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

extern	void	DG_TransPersOne( FVECTOR *disp_pos, FVECTOR *pos ) ;

typedef	struct	{
    GV_ACT		actor ;
    DG_PRIM2	*prim ;
    DG_PRIM2	*prim2 ;	
	int			map ;
    int			time ;
	int			flag ;
} Work ;

static	void	Act( work )
Work		*work ;
{
    if ( work->time != GV_Time ) {
		GM_FreePrim2( work->prim ) ;
		GM_FreePrim2( work->prim2 ) ;
		GV_DestroyActor( work ) ;
		work->flag = 1 ;
    }
}

static	void	Die( work )
Work		*work ;
{
	if ( work->flag == 0 ) {
		GM_FreePrim2( work->prim ) ;
		GM_FreePrim2( work->prim2 ) ;
	}
}

static	int	GetResources( work, v, n, r, g, b, type, flag )
Work		*work ;
FVECTOR		*v ;
int		n ;
u_char		r, g, b ;
int		type ;
int		flag ;
{
    DG_PRIM2		*prim ;
    DG_PRIM2_UVRGB	*rgb ;
	DG_TEX			*tex ;
    int				j, i ;
    FVECTOR			*pos, *vbuf, tmp ;

	tex = DG_GetTexture( TXT_LSIGHT_LINE ) ;
    work->time = GV_Time ;
    vbuf = v ;

    prim = work->prim = GM_MakePrim2( PRIM_TYPE, 1, n * ( type + 1 ) ) ;
    if ( prim == NULL ) {
		printf( "debug prim making failed\n" ) ;
		return -1 ;
	}
    prim->raise = 1000 ;
	DG_ConfigPrim2Tex( prim, tex ) ; /* ダミー */
    DG_SwitchBuffPrim2( prim ) ;
	pos = prim->pos[ prim->buffer_clock ] ;
	rgb = prim->uvrgb[ prim->buffer_clock ] ;
	i = 0 ;
	for ( j = 0; j < n * ( type + 1 ); j ++ ) {
		rgb[ j ].r = r ;
		rgb[ j ].g = g ;
		rgb[ j ].b = b ;
		rgb[ j ].a = 128 ;
		rgb[ j ].q = 4096 ;
		if ( j % ( type + 1 ) == 0 ) rgb[ j ].f = 0x8fff ;
		else	 rgb[ j ].f = 0x0fff ;
		if ( i == type ) {
			pos[ j ] = v[ 0 ] ;
			i = 0 ; v += type ;
		} else {
			pos[ j ] = v[ i ] ;
			i ++ ;
		}
		pos[ j ].vw = 1.0F ;
	}

    if ( type < 3 ) return 0 ;

    v = vbuf ;
    prim = work->prim2 = GM_MakePrim2( PRIM_TYPE2, n, type ) ;
	DG_ConfigPrim2Tex( prim, tex ) ; /* ダミー */
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
    prim->raise = 1000 ;
    DG_SwitchBuffPrim2( prim ) ;
    pos = prim->pos[ prim->buffer_clock ] ;
    rgb = prim->uvrgb[ prim->buffer_clock ] ;
    if ( prim == NULL ) {
		printf( "debug prim making failed\n" ) ;
		return -1 ;
	}
    for ( i = 0; i < n; i ++ ) {
		for ( j = 0; j < type; j ++ ) {
			rgb[ j ].r = r ;
			rgb[ j ].g = g ;
			rgb[ j ].b = b ;
			rgb[ j ].a = 16 ;
			rgb[ j ].q = 4096 ;
			if ( j < 2 ) rgb[ j ].f = 0x8fff ;
			else	 rgb[ j ].f = 0x0fff ;
			pos[ j ] = v[ j ] ;
			pos[ j ].vw = 1.0F ;
		}
		tmp = pos[ 0 ] ; pos[ 0 ] = pos[ 1 ] ; pos[ 1 ] = tmp ;
		rgb += type ;
		pos += type ;
		v += type ;
    }

    return 0 ;
}

void	*NewLineView( verts, n_lines, r, g, b ) 
FVECTOR		*verts ;
int		n_lines ;
u_char		r, g, b ;
{
    Work		*work ;

    work = ( Work * )GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, verts, n_lines, r, g, b, 2, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}

void	*NewSquareView( verts, n_squares, r, g, b )
FVECTOR		*verts ;
int		n_squares ;
u_char		r, g, b ;
{
    Work		*work ;

    work = ( Work * )GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, verts, n_squares, r, g, b, 4, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}

void	*NewTriangleView( verts, n_tris, r, g, b )
FVECTOR		*verts ;
int		n_tris ;
u_char		r, g, b ;
{
    Work		*work ;

    work = ( Work * )GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, verts, n_tris, r, g, b, 3, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}

void	*NewPointView( FVECTOR *pos, u_char r, u_char g, u_char b )
{
	FVECTOR		verts[ 8 ], *v ;
	float		size = 16.0F ;
	int			i ;

	v = verts ;
	for ( i = 0; i < 8; i ++, v ++ ) {
		if ( i & 4 ) {
			v->vx = ( i & 1 ) ? pos->vx + size : pos->vx - size ;
			v->vy = ( i & 2 ) ? pos->vy + size : pos->vy - size ;
			v->vz = pos->vz ;
		} else {
			v->vz = ( i & 1 ) ? pos->vz + size : pos->vz - size ;
			v->vy = ( i & 2 ) ? pos->vy + size : pos->vy - size ;
			v->vx = pos->vx ;
		}
	}
	return NewSquareView( verts, 2, r, g, b ) ;
}

void		*NewBoundingBoxView( b1, b2, r, g, b )
FVECTOR		*b1, *b2 ;
u_char		r, g, b ;
{
    FVECTOR	v[ 24 ] ;

    v[ 0 ].vx = b1->vx ;
    v[ 0 ].vy = b1->vy ;
    v[ 0 ].vz = b1->vz ;
    v[ 8 ] = v[ 19 ] = v[ 0 ] ;
    v[ 1 ].vx = b1->vx ;
    v[ 1 ].vy = b1->vy ;
    v[ 1 ].vz = b2->vz ;
    v[ 12 ] = v[ 16 ] = v[ 1 ] ;
    v[ 2 ].vx = b2->vx ;
    v[ 2 ].vy = b1->vy ;
    v[ 2 ].vz = b2->vz ;    
    v[ 15 ] = v[ 23 ] = v[ 2 ] ;
    v[ 3 ].vx = b2->vx ;
    v[ 3 ].vy = b1->vy ;
    v[ 3 ].vz = b1->vz ;    
    v[ 11 ] = v[ 20 ] = v[ 3 ] ;
    v[ 4 ].vx = b1->vx ;
    v[ 4 ].vy = b2->vy ;
    v[ 4 ].vz = b1->vz ;    
    v[ 9 ] = v[ 18 ] = v[ 4 ] ;
    v[ 5 ].vx = b1->vx ;
    v[ 5 ].vy = b2->vy ;
    v[ 5 ].vz = b2->vz ;    
    v[ 13 ] = v[ 17 ] = v[ 5 ] ;
    v[ 6 ].vx = b2->vx ;
    v[ 6 ].vy = b2->vy ;
    v[ 6 ].vz = b2->vz ;    
    v[ 14 ] = v[ 22 ] = v[ 6 ] ;
    v[ 7 ].vx = b2->vx ;
    v[ 7 ].vy = b2->vy ;
    v[ 7 ].vz = b1->vz ;    
    v[ 10 ] = v[ 21 ] = v[ 7 ] ;
    return NewSquareView( v, 6, r, g, b ) ;
}

void	HZX_ViewSegment( seg )
HZX_SEG	*seg ;
{
    FVECTOR	v[ 4 ] ;

    if ( seg == NULL ) return ;

    v[ 0 ].vx = seg->p1.x ;
    v[ 0 ].vy = seg->p1.y ;
    v[ 0 ].vz = seg->p1.z ;
    v[ 1 ].vx = seg->p1.x ;
    v[ 1 ].vy = seg->p1.y + seg->p1.h ;
    v[ 1 ].vz = seg->p1.z ;
    v[ 2 ].vx = seg->p2.x ;
    v[ 2 ].vy = seg->p2.y + seg->p2.h ;
    v[ 2 ].vz = seg->p2.z ;
    v[ 3 ].vx = seg->p2.x ;
    v[ 3 ].vy = seg->p2.y ;
    v[ 3 ].vz = seg->p2.z ;
    NewSquareView( v, 1, 32, 32, 240 ) ;
}

void	HZX_ViewFloor( seg, n )
HZX_FLR	*seg ;
int	n ;
{
    FVECTOR	v[ 4 ] ;

    if ( seg == NULL ) return ;

    v[ 0 ].vx = seg->p1.x ;
    v[ 0 ].vy = seg->p1.y ;
    v[ 0 ].vz = seg->p1.z ;
    v[ 1 ].vx = seg->p2.x ;
    v[ 1 ].vy = seg->p2.y ;
    v[ 1 ].vz = seg->p2.z ;
    v[ 2 ].vx = seg->p3.x ;
    v[ 2 ].vy = seg->p3.y ;
    v[ 2 ].vz = seg->p3.z ;
    v[ 3 ].vx = seg->p4.x ;
    v[ 3 ].vy = seg->p4.y ;
    v[ 3 ].vz = seg->p4.z ;
    if ( n == 3 ) NewTriangleView( v, 1, 240, 32, 32 ) ;
    else	  NewSquareView( v, 1, 240, 32, 32 ) ;
}

void	HZX_ViewHazard( HZX_HZD *hzd )
{
	if ( hzd->type == HZX_TYPE_SEGMENT ) HZX_ViewSegment( hzd ) ;
	else HZX_ViewFloor( hzd, hzd->p4.h ) ;
}

void	HZX_ViewVuSegRGB( blk, seg, r, g, b )
HZX_BLOCK	*blk ;
HZX_VuSEG	*seg ;
u_char		r, g, b ;
{
    FVECTOR	*v, *verts ;
    SVECTOR	*sv ;
    int		i, n_prims, add ;

    verts = v = GV_Malloc( sizeof( FVECTOR ) * ( seg->b1.pad - 1 ) * 4 ) ;
    if ( v == NULL ) return ;
    sv = seg->verts ;
    for ( i = 0; i < seg->b1.pad - 1; i ++ ) {
		v[ 0 ].vx = sv->vx + blk->tx ;
		v[ 0 ].vy = sv->vy + blk->ty ;
		v[ 0 ].vz = sv->vz + blk->tz ;
		v[ 1 ].vx = sv->vx + blk->tx ;
		v[ 1 ].vy = sv->vy + sv->pad + blk->ty ;
		v[ 1 ].vz = sv->vz + blk->tz ;
		sv ++ ;
		v[ 2 ].vx = sv->vx + blk->tx ;
		v[ 2 ].vy = sv->vy + sv->pad + blk->ty ;
		v[ 2 ].vz = sv->vz + blk->tz ;
		v[ 3 ].vx = sv->vx + blk->tx ;
		v[ 3 ].vy = sv->vy + blk->ty ;
		v[ 3 ].vz = sv->vz + blk->tz ;
		v += 4 ;
    }
    n_prims = seg->b1.pad - 1 ;
    add = 0 ;
    while( n_prims > 0 ) {
		if ( n_prims > 12 ) NewSquareView( verts + add, 12, r, g, b ) ;
		else {
			NewSquareView( verts + add, n_prims, r, g, b ) ;
			break ;
		}
		n_prims -= 12 ; add += 12 * 4 ;
    }
    GV_Free( verts ) ;
}

void	HZX_ViewVuFlr( blk, seg )
HZX_BLOCK	*blk ;
HZX_VuSEG	*seg ;
{
    FVECTOR	*v, *verts ;
    SVECTOR	*sv ;
    int		i, type, n_prims, add ;

    type = seg->b2.pad ;
    verts = v = GV_Malloc( sizeof( FVECTOR ) * seg->b1.pad ) ;
    if ( v == NULL ) return ;
    sv = seg->verts ;
    for ( i = 0; i < seg->b1.pad ; i ++, v ++, sv ++ ) {
		v->vx = sv->vx + blk->tx ;
		v->vy = sv->vy + blk->ty ;
		v->vz = sv->vz + blk->tz ;
    }
    if ( type == 3 ) {
		n_prims = seg->b1.pad / 3 ;
		add = 0 ;	
		while( n_prims > 0 ) {
			if ( n_prims > 16 ) NewTriangleView( verts + add, 16, 240, 32, 32 ) ;
			else {
				NewTriangleView( verts + add, n_prims, 240, 32, 32 ) ;
				break ;
			}
			n_prims -= 16 ; add += 16 * 3 ;
		}
    } else {
		n_prims = seg->b1.pad / 4 ;
		add = 0 ;
		while( n_prims > 0 ) {
			if ( n_prims > 12 ) {
				NewSquareView( verts + add, 12, 240, 32, 32 ) ;
			} else {
				NewSquareView( verts + add, n_prims, 240, 32, 32 ) ;
				break ;
			}
			n_prims -= 12 ; add += 12 * 4 ;
		}
    }
    GV_Free( verts ) ;
}

void	HZX_ViewDynamicSegment( seg )
HZX_D_SEGMENT	*seg ;
{
    FVECTOR	v[ 4 ] ;

    if ( seg == NULL ) return ;

    v[ 0 ].vx = seg->p1.vx ;
    v[ 0 ].vy = seg->p1.vy ;
    v[ 0 ].vz = seg->p1.vz ;
    v[ 1 ].vx = seg->p1.vx ;
    v[ 1 ].vy = seg->p1.vy + seg->p1.vw ;
    v[ 1 ].vz = seg->p1.vz ;
    v[ 2 ].vx = seg->p2.vx ;
    v[ 2 ].vy = seg->p2.vy + seg->p2.vw ;
    v[ 2 ].vz = seg->p2.vz ;
    v[ 3 ].vx = seg->p2.vx ;
    v[ 3 ].vy = seg->p2.vy ;
    v[ 3 ].vz = seg->p2.vz ;
    NewSquareView( v, 1, 32, 32, 240 ) ;    
}

void	HZX_ViewDynamicFloor( seg, n )
HZX_D_FLOOR	*seg ;
int		n ;
{
    FVECTOR	v[ 4 ] ;

    if ( seg == NULL ) return ;

    v[ 0 ].vx = seg->p1.vx ;
    v[ 0 ].vy = seg->p1.vy ;
    v[ 0 ].vz = seg->p1.vz ;
    v[ 1 ].vx = seg->p2.vx ;
    v[ 1 ].vy = seg->p2.vy ;
    v[ 1 ].vz = seg->p2.vz ;
    v[ 2 ].vx = seg->p3.vx ;
    v[ 2 ].vy = seg->p3.vy ;
    v[ 2 ].vz = seg->p3.vz ;
    v[ 3 ].vx = seg->p4.vx ;
    v[ 3 ].vy = seg->p4.vy ;
    v[ 3 ].vz = seg->p4.vz ;
    if ( n == 3 ) NewTriangleView( v, 1, 240, 32, 32 ) ;
    else	  NewSquareView( v, 1, 240, 32, 32 ) ;
}

static	void	ViewZone( zone, r, g, b )
HZX_ZON		*zone ;
u_char		r, g, b ;
{
    FVECTOR	b1, b2 ;

    b1.vx = zone->x - ( float )zone->w ;
    b2.vx = zone->x + ( float )zone->w ;
    b1.vy = zone->y - 1000.0F ;
    b2.vy = zone->y + 2000.0F ;
    b1.vz = zone->z - ( float )zone->h ;
    b2.vz = zone->z + ( float )zone->h ;
#if 1
	if ( ( zone->flag & HZX_ZON_LINK ) && ( GV_PadData->press & PAD_L1 ) ) {
		DumpVec( &b1 ) ;
		DumpVec( &b2 ) ;
	}
#endif
    NewBoundingBoxView( &b1, &b2, r, g, b ) ;
}

void	HZX_ViewZone1( HZX_ZON *zon, u_char r, u_char g, u_char b ) 
{
	ViewZone( zon, r, g, b ) ;
}

void	HZX_ViewZone( hzx_id, addr )
HZX_GROUP_ID	hzx_id ;
int		addr ;
{
	int		i ;
    int		zno ;
    int		gno ;
    HZX_GRP	*grp ;
    HZX_ZON	*zone ;

    zno = addr & 255 ;
    gno = HZX_GetGroupNo( hzx_id ) ;
    grp = HZX_GetGroupFromNo( gno ) ;
    if ( zno >= grp->n_zones ) return ;
    zone = grp->zones + zno ;
    ViewZone( zone, 128, 128, 16 ) ;

	for ( i = 0; i < 6; i ++ ) {
		if ( ( zone->flag & HZX_ZON_LINK ) && i == 5 ) break ;
		zno = zone->nears[ i ] ;
		if ( zno == 255 ) break ;
		ViewZone( grp->zones + zno, 16, 16, 128 ) ;
	}

if ( GV_PadData->press & PAD_L1 ) printf( "this zone has %d nears\n", i ) ;

    if ( zone->flag & HZX_ZON_LINK ) {
		gno = zone->near_flag[ 5 ] ;
		grp = HZX_GetGroupFromNo( gno ) ;
		zno = zone->nears[ 5 ] ;
		zone = grp->zones + zno ;
		ViewZone( zone, 16, 128, 16 ) ;
    }
}

void	HZX_ViewMatrix( FMATRIX *world, float len )
{
	FVECTOR		v[ 6 ] ;

	v[ 0 ].vx = world->m[ 3 ][ 0 ] ;
	v[ 0 ].vy = world->m[ 3 ][ 1 ] ;
	v[ 0 ].vz = world->m[ 3 ][ 2 ] ;
	DG_COPY_VEC( &v[ 2 ], &v[ 0 ] ) ;
	DG_COPY_VEC( &v[ 4 ], &v[ 0 ] ) ;
	v[ 1 ].vx = v[ 0 ].vx + world->m[ 0 ][ 0 ] * len ;
	v[ 1 ].vy = v[ 0 ].vy + world->m[ 0 ][ 1 ] * len ;
	v[ 1 ].vz = v[ 0 ].vz + world->m[ 0 ][ 2 ] * len ;
	v[ 3 ].vx = v[ 0 ].vx + world->m[ 1 ][ 0 ] * len ;
	v[ 3 ].vy = v[ 0 ].vy + world->m[ 1 ][ 1 ] * len ;
	v[ 3 ].vz = v[ 0 ].vz + world->m[ 1 ][ 2 ] * len ;
	v[ 5 ].vx = v[ 0 ].vx + world->m[ 2 ][ 0 ] * len ;
	v[ 5 ].vy = v[ 0 ].vy + world->m[ 2 ][ 1 ] * len ;
	v[ 5 ].vz = v[ 0 ].vz + world->m[ 2 ][ 2 ] * len ;
	NewLineView( &v[ 0 ], 1, 255, 0, 0 ) ;
	NewLineView( &v[ 2 ], 1, 0, 255, 0 ) ;
	NewLineView( &v[ 4 ], 1, 0, 0, 255 ) ;
}

void	HZX_ViewSeNo( HZX_HZD *hzd, FVECTOR *pos )
{
	FVECTOR		disp_pos, pos2 ;

	if ( pos == NULL ) {
		pos = &pos2 ;
		if ( hzd->type == HZX_TYPE_SEGMENT ) {
			pos2.vx = ( hzd->p1.x + hzd->p2.x ) / 2.0F ;
			pos2.vz = ( hzd->p1.z + hzd->p2.z ) / 2.0F ;
			pos2.vy = ( 2.0F * hzd->p1.y + 2.0F * hzd->p2.y +
					    hzd->p1.h + hzd->p2.h ) / 4.0F ;
		} else {
			pos2.vx = hzd->p1.x + hzd->p2.x + hzd->p3.x ;
			pos2.vy = hzd->p1.y + hzd->p2.y + hzd->p3.y ;
			pos2.vz = hzd->p1.z + hzd->p2.z + hzd->p3.z ;
			if ( hzd->p4.h == 4 ) {
				pos2.vx += hzd->p4.x ;
				pos2.vy += hzd->p4.y ;
				pos2.vz += hzd->p4.z ;
			}
			_sceVu0ScaleVector( &pos2, &pos2, 1.0F / ( float )hzd->p4.h ) ;
		}
		pos2.vw = 1.0F ;
	}

	DG_TransPersOne( &disp_pos, pos ) ;
	MENU_Locate( ( int )disp_pos.vx, ( int )disp_pos.vy, 0 ) ;
	MENU_Color( 196, 196, 196, 128 ) ;
	MENU_Printf( "%d\n", ( hzd->attribute & 0xf0000000 ) >> 28 ) ;
}

/* デバッグ用 */
void	DumpSegment( seg )
HZX_SEG		*seg ;
{
    printf( "seg %.0f %.0f %.0f %.0f : ", seg->p1.x, seg->p1.y,
		   seg->p1.z, seg->p1.h ) ;
    printf( "%.0f %.0f %.0f %.0f : ", seg->p2.x, seg->p2.y,
		   seg->p2.z, seg->p2.h ) ;
    printf( "%x\n", seg->attribute ) ;
}

void	DumpFloor( flr )
HZX_FLR		*flr ;
{
    printf( "flr %.0f %.0f %.0f %.0f : ", flr->p1.x, flr->p1.y,
		   flr->p1.z, flr->p1.h ) ;
    printf( "%.0f %.0f %.0f %.0f\n", flr->p2.x, flr->p2.y,
		   flr->p2.z, flr->p2.h ) ;
    printf( "%.0f %.0f %.0f %.0f : ", flr->p3.x, flr->p3.y,
		   flr->p3.z, flr->p3.h ) ;
    printf( "%.0f %.0f %.0f %.0f\n", flr->p4.x, flr->p4.y,
		   flr->p4.z, flr->p4.h ) ;
	//    printf( "%.0f, %.0f %.0f\n", flr->p1.h, flr->p2.h, flr->p3.h ) ;
#if 0
    printf( "[%.0f %.0f %.0f %.0f : ", flr->b1.x, flr->b1.z,
		   flr->b1.y, flr->b1.h ) ;
    printf( "%.0f %.0f %.0f %.0f\n]", flr->b2.x, flr->b2.z,
		   flr->b2.y, flr->b2.h ) ;
#endif
	printf( "atr %x\n", flr->attribute ) ;
}

void		DumpHzxVec( v )
HZX_VEC		*v ;
{
    printf( "%f %f %f\n", v->x, v->y, v->z ) ;
}

void		DumpVec( v )
FVECTOR		*v ;
{
    printf( "%f %f %f %f\n", v->vx, v->vy, v->vz, v->vw ) ;
}

void		DumpSVec( v )
SVECTOR		*v ;
{
    printf( "%d %d %d %d\n", v->vx, v->vy, v->vz, v->pad ) ;
}

void		DumpIVec( v )
IVECTOR		*v ;
{
    printf( "%d %d %d %d\n", v->vx, v->vy, v->vz, v->vw ) ;
}

void		DumpTrap( trp, n )
HZX_TRP		*trp ;
int		n ;
{
    while( -- n >= 0 ) {
		printf( "trap[ %d ]\n", n ) ;
		DumpVec( ( FVECTOR * )&( trp->b1 ) ) ;
		DumpVec( ( FVECTOR * )&( trp->b2 ) ) ;
		printf( "id %d\n", trp->name_id ) ;
		trp ++ ;
    }
}

/* Ｖｕ０デバッグ用 */
void		DumpFloat( adr )
void		*adr ;
{
    float	*f ;

    f = ( float * )adr ;
    printf( "%.5f %.5f %.5f %.5f\n", f[ 0 ], f[ 1 ], f[ 2 ], f[ 3 ] ) ;
}

void		DumpInt( adr )
void		*adr ;
{
    int		*f ;

    f = ( int * )adr ;
    printf( "%d %d %d %d\n", f[ 0 ], f[ 1 ], f[ 2 ], f[ 3 ] ) ;
}

/* そのた */
void		DumpMatrix( m )
FMATRIX		*m ;
{
    printf( "%.2f, %.2f, %.2f %.2f\n", m->m[ 0 ][ 0 ], m->m[ 0 ][ 1 ], 
		   m->m[ 0 ][ 2 ], m->m[ 0 ][ 3 ] ) ;
    printf( "%.2f, %.2f, %.2f %.2f\n", m->m[ 1 ][ 0 ], m->m[ 1 ][ 1 ], 
		   m->m[ 1 ][ 2 ], m->m[ 1 ][ 3 ] ) ;
    printf( "%.2f, %.2f, %.2f %.2f\n", m->m[ 2 ][ 0 ], m->m[ 2 ][ 1 ], 
		   m->m[ 2 ][ 2 ], m->m[ 2 ][ 3 ] ) ;
    printf( "%.2f, %.2f, %.2f %.2f\n", m->m[ 3 ][ 0 ], m->m[ 3 ][ 1 ], 
		   m->m[ 3 ][ 2 ], m->m[ 3 ][ 3 ] ) ;
}

void		DumpVuSEGS( segs, n_segs )
HZX_VuSEG	*segs ;
int		n_segs ;
{
    int		i ;
    SVECTOR	*v ;

    if ( segs == NULL ) { 
		printf( "nul\n" ) ; 
		return ; 
    }
    printf( "%d strips\n", n_segs ) ;
    while( -- n_segs >= 0 ) {
		printf( "n_point %d\n", segs->b1.pad ) ;
		printf( "atr %x\n", segs->atr ) ;
		printf( "%d %d %d - %d %d %d\n",
			   segs->b1.vx, segs->b1.vy, segs->b1.vz,
			   segs->b2.vx, segs->b2.vy, segs->b2.vz ) ;
		v = segs->verts ;
		for ( i = 0; i < segs->b1.pad; i ++, v ++ ) {
			DumpSVec( v ) ;
		}
		segs ++ ;
    }
}

void		DumpVuSEGS2( blk, segs, n_segs )
HZX_BLOCK	*blk ;
HZX_VuSEG	*segs ;
int		n_segs ;
{
    int		i ;
    SVECTOR	*v ;

    if ( segs == NULL ) { 
		printf( "nul\n" ) ; 
		return ; 
    }
    printf( "%x : %d strips\n", segs->verts, n_segs ) ;
    while( -- n_segs >= 0 ) {
		printf( "n_point %d\n", segs->b1.pad ) ;
		printf( "atr %x\n", segs->atr ) ;
		printf( "%d %d %d - %d %d %d\n",
			   segs->b1.vx + blk->tx, segs->b1.vy + blk->ty, segs->b1.vz + blk->tz,
			   segs->b2.vx + blk->tx, segs->b2.vy + blk->ty, segs->b2.vz + blk->tz ) ;
		v = segs->verts ;
		for ( i = 0; i < segs->b1.pad; i ++, v ++ ) {
			printf( "%d %d %d %d\n", v->vx + blk->tx, v->vy + blk->ty,
				   v->vz + blk->tz, v->pad ) ;
		}
		segs ++ ;
    }
}

void	DumpVuSEGS3( hzx, group, block, seg, which )
HZX_HDL	*hzx ;
int	group, block, seg, which ;
{
    HZX_BLOCK	*b ;
    HZX_VuSEG	*s ;

    b = ( hzx->def->groups + group )->blocks + block ;
    if ( which == 0 ) {
		if ( b->n_segs < seg ) {
			printf( "not exist\n" ) ; return ;
		}
		s = b->segs + seg ;
    } else {
		if ( b->n_flrs < seg ) {
			printf( "not exist\n" ) ; return ;
		}
		s = b->flrs + seg ;
    }
    DumpVuSEGS2( b, s, 1 ) ;
}

void	DumpBehind( b )
HZX_BEHIND	*b ;
{
    DumpVec( &( b->b1 ) ) ;
    DumpVec( &( b->b2 ) ) ;
}

void	ViewFromTo( from, to, r, g, b )
FVECTOR	*from, *to ;
u_char	r, g, b ;
{
	FVECTOR		v[ 2 ] ;

	DG_COPY_VEC( &v[ 0 ], from ) ;
	DG_COPY_VEC( &v[ 1 ], to ) ;
	NewLineView( v, 1, r, g, b ) ;
}
#endif

