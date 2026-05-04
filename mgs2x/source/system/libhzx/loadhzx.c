//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   loadhzx.c
   ハザードデータローダ
   
   1999/07/07 M.Sonoyama
   $Id: loadhzx.c,v 1.4 2002/11/23 11:48:44 Yoshizawa1 Exp $			
   */

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "bp_vector.h"

#include "BP_EndianSupport.h"

#include <stddef.h>

#define		MAX_HZX_CACHES	(1)
int		HZX_CurrentGroup ;

static int		N_HzxCaches ;
static HZX_HDL		*HzxCache[ MAX_HZX_CACHES ] ;

static void EndianSwapHzxDef( HZX_DEF *pData)
{
   BP_LE_SwapUShort_Inp( &pData->version );
   BP_LE_SwapUShort_Inp( &pData->n_groups );
   BP_LE_SwapUShort_Inp( &pData->n_patrols );
   BP_LE_SwapUShort_Inp( &pData->n_clears );
   BP_LE_SwapUShortArray_Inp( pData->vr_pat_offset, 8 );
   BP_LE_SwapUShortArray_Inp( pData->vr_clr_offset, 8 );

   BP_LE_SwapPtr_Inp( &pData->patrols );
   BP_LE_SwapPtr_Inp( &pData->points );
   BP_LE_SwapPtr_Inp( &pData->cle_areas );
   BP_LE_SwapPtr_Inp( &pData->cle_roots );
   BP_LE_SwapPtr_Inp( &pData->cle_points );

   BP_LE_SwapPtr_Inp( &pData->groups );
}

static void EndianSwapHzxPtp( HZX_PTP *pData )
{
   BP_LE_SwapFloatArray_Inp( &pData->x, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->ax, 3 );
   BP_LE_SwapSShort_Inp( &pData->act );
   BP_LE_SwapSShort_Inp( &pData->time );
   BP_LE_SwapSShort_Inp( &pData->dir );
   BP_LE_SwapSShort_Inp( &pData->pad );
   BP_LE_SwapSInt_Inp( &pData->flag );
   BP_LE_SwapSInt_Inp( &pData->group_id );
}

static void EndianSwapHzxPtpArray( HZX_PTP *pData, int count )
{
   int i;
   for ( i = 0; i < count; ++i )
   {
      EndianSwapHzxPtp( pData + i );
   }
}

static void EndianSwapHzxClePtp( HZX_CLE_PTP *pData )
{
   BP_LE_SwapFloatArray_Inp( &pData->x, 3 );
   BP_LE_SwapFloatArray_Inp( &pData->ax, 3 );
   BP_LE_SwapSShort_Inp( &pData->act );
   BP_LE_SwapSShort_Inp( &pData->time );
   BP_LE_SwapSShort_Inp( &pData->dir );
   BP_LE_SwapSShort_Inp( &pData->pad );
   BP_LE_SwapSInt_Inp( &pData->group_id );
}

static void EndianSwapHzxClePtpArray( HZX_CLE_PTP *pData, int count )
{
   int i;
   for ( i = 0; i < count; ++i )
   {
      EndianSwapHzxClePtp( pData + i );
   }
}

static void EndianSwapHzxCleRoot( HZX_CLE_ROOT *pData )
{
   BP_LE_SwapSShort_Inp( &pData->n_points );
   BP_LE_SwapSShort_Inp( &pData->pad );
   BP_LE_SwapSInt_Inp( &( pData->align_16[0] ) );
   BP_LE_SwapSInt_Inp( &( pData->align_16[1] ) );
   BP_LE_SwapPtr_Inp( &pData->points );
}

static void EndianSwapHzxCleArea( HZX_CLE_AREA *pData )
{
   BP_LE_SwapFloatArray_Inp( &pData->b1.vx, 4 );
   BP_LE_SwapFloatArray_Inp( &pData->b2.vx, 4 );
   BP_LE_SwapSInt_Inp( &pData->name );
   BP_LE_SwapUShort_Inp( &pData->go_time );
   BP_LE_SwapUShort_Inp( &pData->n_root );
   BP_LE_SwapPtr_Inp( &pData->roots );
   BP_LE_SwapSInt_Inp( &pData->align_16 );
}

static void EndianSwapHzxPat( HZX_PAT *pData )
{
   BP_LE_SwapSShort_Inp( &pData->n_points );
   BP_LE_SwapSShort_Inp( &pData->pad );
   BP_LE_SwapPtr_Inp( &pData->points );
   BP_LE_SwapSInt_Inp( &pData->flag );
}

/*-----------------------------------------------------------------------*/

/* 巡回データの初期化 */
static void	SetupPatrols( HZX_PAT *pat, int n, void *def )
{
    while( -- n >= 0 ){
      EndianSwapHzxPat( pat );

		pat->points = ( HZX_PTP * )( ( char * )def + ( u_int )( pat->points ) ) ;

      EndianSwapHzxPtpArray( pat->points, pat->n_points );

		pat++;
    }
}

/* クリアリングデータ初期化 */
static	void	SetupClearings( HZX_CLE_AREA *area, int n, void *def ) 
{
    HZX_CLE_ROOT	*root ;
    int			m, i, j ;

    i = 0 ;
    j = 0 ;
    while( -- n >= 0 ) {
      EndianSwapHzxCleArea( area );

      area->roots = ( HZX_CLE_ROOT * )( ( char * )def + ( u_int )area->roots ) ; 
		m = area->n_root ;
		root = area->roots ;
		while( -- m >= 0 ) {
         EndianSwapHzxCleRoot( root );
			root->points = ( HZX_CLE_PTP * )( ( char * )def + ( u_int )root->points ) ;
         EndianSwapHzxClePtpArray( root->points, root->n_points );
			j += root->n_points ;
			root ++ ;
		}
		area ++ ;
		i += m ;
    }
}

/*-----------------------------------------------------------------------*/

/* 当たりハンドラー取得 */
HZX_HDL		*HZX_GetHandler( id )
int		id ;
{
    int		i ;
    HZX_HDL	*hdl ;

    hdl = NULL ;
    id |= ( ( 'h' - 'a' ) << 24 ) ;
    for ( i = 0; i < N_HzxCaches; i ++ ) {
		hdl = HzxCache[ i ] ;
		if ( hdl->id == id ) return hdl ;
    }
    printf( "hzx %d not found\n", id ) ;
    return NULL ;
}

/* 当たりハンドラーの廃棄 */
void		HZX_FreeHandler( void )
{
    HZX_GRP	*grp ;
	HZX_HDL	*hdl ;

	if ( ( hdl = HZX_CurrentHzx ) == NULL ) return ;
	grp = hdl->def->groups ;
	if ( grp->dynamics != NULL ) GV_Free( grp->dynamics ) ;
	if ( hdl->tag != NULL ) GV_Free( hdl->tag ) ;
	if ( HZX_MapLink != NULL ) GV_Free( HZX_MapLink ) ;
	GV_Free( hdl ) ;
}

/* 初期化 */
void	HZX_InitHzxCache( void )
{
    N_HzxCaches = 0 ;
}

/*-----------------------------------------------------------------------*/

/* マップリンク壁の生成 */
static void	MakeMapLinkSegment( HZX_HDL *hdl, HZX_MAPLINK *maplink )
{
	HZX_GRP	*g1, *g2 ;
	HZX_ZON	*z1, *z2 ;	
	HZX_HZD	*seg ;
	FVECTOR	p[ 5 ], q[ 5 ] ;
	FVECTOR	v1, v2, v3 ;		
	int		i, j, found ;
	float	ip1, ip2 ;
	float	ftmp, ftmp2 ;
	float	ftmp3, ftmp4 ;

	seg = &maplink->segment ;

	// 初期化していなかったので、以下のコードを追加 2002/02/21 K.Uehara
	v1 = DG_ZeroVector;
	v2 = DG_ZeroVector;
	v3 = DG_ZeroVector;

	g1 = hdl->def->groups + GV_GetNo( maplink->hzx_id1 ) ;
	g2 = hdl->def->groups + GV_GetNo( maplink->hzx_id2 ) ;

	z1 = g1->zones + maplink->z1 ;
	z2 = g2->zones + maplink->z2 ;

	p[ 0 ].vx = z1->x - z1->w ;
	p[ 0 ].vz = z1->z - z1->h ;
	p[ 1 ].vx = z1->x + z1->w ;
	p[ 1 ].vz = z1->z - z1->h ;
	p[ 2 ].vx = z1->x + z1->w ;
	p[ 2 ].vz = z1->z + z1->h ;
	p[ 3 ].vx = z1->x - z1->w ;
	p[ 3 ].vz = z1->z + z1->h ;
	p[ 4 ].vx = z1->x - z1->w ;
	p[ 4 ].vz = z1->z - z1->h ;

	q[ 0 ].vx = z2->x - z2->w ;
	q[ 0 ].vz = z2->z - z2->h ;
	q[ 1 ].vx = z2->x + z2->w ;
	q[ 1 ].vz = z2->z - z2->h ;
	q[ 2 ].vx = z2->x + z2->w ;
	q[ 2 ].vz = z2->z + z2->h ;
	q[ 3 ].vx = z2->x - z2->w ;
	q[ 3 ].vz = z2->z + z2->h ;
	q[ 4 ].vx = z2->x - z2->w ;
	q[ 4 ].vz = z2->z - z2->h ;

	found = 0 ;
	for ( i = 0; i < 4; i ++ ) {
		v1.vx = p[ i + 1 ].vx - p[ i ].vx ;
		v1.vz = p[ i + 1 ].vz - p[ i ].vz ;
		_sceVu0Normalize( &v1, &v1 ) ;
		for ( j = 0; j < 4; j ++ ) {
			v2.vx = q[ j ].vx - p[ i ].vx ;
			v2.vz = q[ j ].vz - p[ i ].vz ;			
			_sceVu0Normalize( &v2, &v2 ) ;
			ip1 = _sceVu0InnerProduct( &v1, &v2 ) ;
			v3.vx = q[ j + 1 ].vx - p[ i ].vx ;
			v3.vz = q[ j + 1 ].vz - p[ i ].vz ;
			_sceVu0Normalize( &v3, &v3 ) ;
			ip2 = _sceVu0InnerProduct( &v1, &v3 ) ;
			if ( ( ( DG_FABS( v2.vx ) > 0.01F || DG_FABS( v2.vz ) > 0.01F ) 
				  && DG_FABS( ip1 ) < 0.99F ) ||
				 ( ( DG_FABS( v3.vx ) > 0.01F || DG_FABS( v3.vz ) > 0.01F ) 
				  && DG_FABS( ip2 ) < 0.99F ) ) continue ;

			ftmp = DG_MIN( p[ i ].vx, p[ i + 1 ].vx ) ;
			ftmp2 = DG_MAX( p[ i ].vx, p[ i + 1 ].vx ) ;
			ftmp3 = DG_MIN( q[ j ].vx, q[ j + 1 ].vx ) ;
			ftmp4 = DG_MAX( q[ j ].vx, q[ j + 1 ].vx ) ;
			if ( DG_FABS( ftmp - ftmp2 ) > 0.10F &&
				 ( ftmp >= ftmp4 || ftmp3 >= ftmp2 ) ) {
				continue ;
			}
			ftmp = DG_MIN( p[ i ].vz, p[ i + 1 ].vz ) ;
			ftmp2 = DG_MAX( p[ i ].vz, p[ i + 1 ].vz ) ;
			ftmp3 = DG_MIN( q[ j ].vz, q[ j + 1 ].vz ) ;
			ftmp4 = DG_MAX( q[ j ].vz, q[ j + 1 ].vz ) ;
			if ( DG_FABS( ftmp - ftmp2 ) > 0.10F &&
				( ftmp >= ftmp4 || ftmp3 >= ftmp2 ) ) {
				continue ;
			}
			ftmp = DG_MIN( p[ i ].vx, p[ i + 1 ].vx ) ;
			ftmp = DG_MIN( ftmp, q[ j ].vx ) ;
			ftmp = DG_MIN( ftmp, q[ j + 1 ].vx ) ;
			seg->p1.x = ftmp ;
			ftmp = DG_MAX( p[ i ].vx, p[ i + 1 ].vx ) ;
			ftmp = DG_MAX( ftmp, q[ j ].vx ) ;
			ftmp = DG_MAX( ftmp, q[ j + 1 ].vx ) ;
			seg->p2.x = ftmp ;
			ftmp = DG_MIN( p[ i ].vz, p[ i + 1 ].vz ) ;
			ftmp = DG_MIN( ftmp, q[ j ].vz ) ;
			ftmp = DG_MIN( ftmp, q[ j + 1 ].vz ) ;
			seg->p1.z = ftmp ;
			ftmp = DG_MAX( p[ i ].vz, p[ i + 1 ].vz ) ;
			ftmp = DG_MAX( ftmp, q[ j ].vz ) ;
			ftmp = DG_MAX( ftmp, q[ j + 1 ].vz ) ;
			seg->p2.z = ftmp ;
			
			seg->p1.y = seg->p2.y = DG_MIN( z1->y, z2->y ) - 500.0F ;
			seg->p1.h = seg->p2.h = DG_MAX( z1->y, z2->y ) + 2000.0F - seg->p1.y ;
			
			seg->attribute = 0 ;
			seg->type = HZX_TYPE_SEGMENT ;

			found = 1 ;
			break ;
		}
		if ( found ) break ;
	}
}

void		HZX_MakeMapLink( HZX_HDL *hzd ) 
{
	int				n_maplinks, i, j, k ;
	int				z1, z2 ;
	u_char			*links ;
	HZX_GRP			*grp ;
	HZX_ZON			*zon ;
	HZX_GROUP_ID	hzx_id1, hzx_id2 ;
	int				z1s[ 64 ], z2s[ 64 ] ;
	HZX_GROUP_ID	hzxid1s[ 64 ], hzxid2s[ 64 ] ;
	HZX_MAPLINK		*maplink ;

	n_maplinks = 0 ;
	grp = hzd->def->groups ;
	for ( i = 0; i < hzd->def->n_groups; i ++, grp ++ ) {
		hzx_id1 = GV_GetBit( i ) ;
		links = grp->link_zone ;
		for ( j = 0; j < (int)grp->n_link_zones; j ++, links += 2 ) {
			z1 = links[ 1 ] ;
			zon = grp->zones + z1 ;
			hzx_id2 = GV_GetBit( ( int )zon->near_flag[ 5 ] ) ;
			z2 = zon->nears[ 5 ] ;
			for ( k = 0; k < n_maplinks; k ++ ) {
				if ( ( hzx_id1 | hzx_id2 ) == ( hzxid1s[ k ] | hzxid2s[ k ] ) &&
					 ( ( z1 == z1s[ k ] && z2 == z2s[ k ] ) || 
					   ( z2 == z1s[ k ] && z1 == z2s[ k ] ) ) ) {
					break ;
				}
			}
			/* 縦方向リンクのときは作らない */
			{
				HZX_ZON		*zon1, *zon2 ;
				HZX_GRP		*hgrp2 ;

				zon1  = zon ;
				hgrp2 = hzd->def->groups + GV_GetNo( hzx_id2 ) ;
				zon2  = hgrp2->zones + z2 ;
				if ( DG_FABS( zon1->y - zon2->y ) > 2400.0F ) continue ;
			}
			if ( k == n_maplinks ) {
				ASSERT( n_maplinks < 64 ) ;
				hzxid1s[ n_maplinks ] = hzx_id1 ;
				hzxid2s[ n_maplinks ] = hzx_id2 ;
				z1s[ n_maplinks ] = z1 ;
				z2s[ n_maplinks ] = z2 ;
				n_maplinks ++ ;
			}
		}
	}

	if ( n_maplinks == 0 ) return ;

	maplink = GV_Malloc( sizeof( HZX_MAPLINK ) * n_maplinks ) ;
	ASSERT( maplink != NULL ) ;
	GV_ZeroMemory( maplink, sizeof( HZX_MAPLINK ) * n_maplinks ) ;

	HZX_MapLink = maplink ; 
	HZX_N_MapLinks = n_maplinks ;

	for ( i = 0; i < n_maplinks; i ++, maplink ++ ) {
		maplink->hzx_id1 = hzxid1s[ i ] ;
		maplink->hzx_id2 = hzxid2s[ i ] ;
		maplink->z1 = z1s[ i ] ;
		maplink->z2 = z2s[ i ] ;
		MakeMapLinkSegment( hzd, maplink ) ;
	}
}

/* グループセッティング */
void		HZX_SetupGroup( hzx )
HZX_HDL		*hzx ;
{
    HZX_DYNAMICS	*dym ;
    int	 		n_groups ;
    HZX_GRP		*grp;
	
    hzx->grp = grp = hzx->def->groups ;
    hzx->block = grp->blocks ;
#if 0
    n_groups = hzx->def->n_groups ;
    while( -- n_groups >= 0 ) {
		/* ルートセッティング */
		//	if ( grp->route == NULL ) 
		{
			int		n_zones ;
			n_zones = grp->n_zones ;
			if( n_zones > 1 ){
				int		route_size ;
				u_char 		*route ;

				route_size = ( ( n_zones - 1 ) * ( n_zones - 2 ) ) / 2 
					+ ( n_zones - 1 );
				route = GV_Malloc( route_size );
				HZX_MakeRoute( grp, route ) ;
				grp->route = route ;
				if ( n_groups == 2 )
				{
					int		i ;
					printf( "route size %d\n", route_size ) ;		
					for ( i = 0; i < route_size; i ++ ) {
						printf( "%d:", route[ i ] ) ;
					}
					printf( "\n" ) ;
				}
			}
		}
		grp ++ ;
	}
	/* 後で修正する */
#endif
	hzx->route = hzx->def->groups->route ;
	
	/* 動的ハザードセッティング */
	grp = hzx->def->groups ;
	n_groups = hzx->def->n_groups ;
	dym = GV_Malloc( sizeof( HZX_DYNAMICS ) * n_groups ) ;
	ASSERT ( dym != NULL ) ;
	while( -- n_groups >= 0 ) {    
		dym->segs = NULL ;
		dym->flrs = NULL ;
		dym->traps = NULL ;
		dym->n_segs = 0 ;
		dym->n_flrs = 0 ;
		dym->n_traps = 0 ;
		grp->dynamics = dym ;
		dym ++ ; grp ++ ;
	}

	/* マップリンク壁の生成 */
	HZX_MakeMapLink( hzx ) ;
}	

/* 当たりハンドラー作成 */
static	HZX_HDL		*HZX_MakeHandler( def, id, no )
HZX_DEF			*def ;
int			id, no ;
{
	HZX_HDL		*hdl ;

	hdl = ( HZX_HDL * )GV_Malloc( sizeof( HZX_HDL ) ) ;
	if ( hdl != NULL ) {
		hdl->def = def ;
		hdl->map = 1 ;
		hdl->id = id ;
		HZX_SetupGroup( hdl ) ;
		return hdl ;
	}
	return NULL ;
}

/* タグデータを生成 */
static	void	MakeSegTag( tag, blk, seg, no, grp_no ) 
int		*tag ;
HZX_BLOCK	*blk ;
HZX_VuSEG	*seg ;
int		no ;
int		grp_no ;
{
	seg->tag = tag ;
	seg->size = ( sizeof( SVECTOR ) * seg->b1.pad + 15 ) / 16 ;
	tag[ 0 ] = seg->b1.pad ;
	tag[ 1 ] = blk->block_no ;
	tag[ 2 ] = no ;
	tag[ 3 ] = grp_no ;
}

static	void	MakeFlrTag( tag, blk, seg, no, grp_no ) 
int		*tag ;
HZX_BLOCK	*blk ;
HZX_VuSEG	*seg ;
int		no ;
int		grp_no ;
{
	seg->size = ( sizeof( SVECTOR ) * seg->b1.pad + 15 ) / 16 ;
	seg->tag = tag ;
	tag[ 0 ] = seg->b1.pad ; 
	tag[ 1 ] = blk->block_no ;
	tag[ 2 ] = no ;
	tag[ 3 ] = grp_no ;
	tag[ 4 ] = seg->b2.pad - 3 ;
	tag[ 5 ] = ( seg->atr & HZX_FLOOR_RECT ) ;
	tag[ 6 ] = ( seg->atr & HZX_FLOOR_FLAT ) ;
	tag[ 7 ] = 0 ;
	tag[ 8 ] = seg->b1.vy + blk->ty ;
	tag[ 9 ] = seg->b2.vy + blk->ty ;
}

static void EndianSwapHzxGrp( HZX_GRP *pData )
{
   // BP - JM - Being lazy here.  Everything up to div_x is 4 byte swaps.

   int div_x_offset = offsetof( HZX_GRP, div_x );
   ASSERT( ( div_x_offset & 3 ) == 0 );
   BP_LE_SwapSIntArray_Inp( (int *) pData, div_x_offset / 4 );

   // Now we're swapping from div_x onward
   BP_LE_SwapUShort_Inp( &pData->n_blocks );
   BP_LE_SwapUShort_Inp( &pData->n_x_blocks );
   BP_LE_SwapPtr_Inp( &pData->blocks );
   BP_LE_SwapPtr_Inp( &pData->dynamics );
}

static void EndianSwapHzxGrp_PostPtrFixup( HZX_GRP *pData )
{
   // BP - JM - Got algorithms from MGS2/tools/scn2hzx/Scn2Hzx.c

   // Route Courses
   if ( pData->n_zones > 1 )
   {
      int num_route_shorts = ( ( pData->n_zones - 1 ) * ( pData->n_zones - 2 ) ) / 2 +
         ( pData->n_zones - 1 );

      // route_course entries can be duped in the array
      BP_LE_SwapUShortArray_Inp(  pData->route_course[0], num_route_shorts );
      if ( pData->route_course[1] != pData->route_course[0] )
      {
         BP_LE_SwapUShortArray_Inp( pData->route_course[1], num_route_shorts );
      }

      // no need to check against [1], it's impossible according to the export tool
      if ( pData->route_course[2] != pData->route_course[0] )
      {
         BP_LE_SwapUShortArray_Inp( pData->route_course[2], num_route_shorts );
      }

      if ( 
         pData->route_course[3] != pData->route_course[0] &&
         pData->route_course[3] != pData->route_course[1] &&
         pData->route_course[3] != pData->route_course[2] )
      {
         BP_LE_SwapUShortArray_Inp( pData->route_course[3], num_route_shorts );
      }
   }

   // Online_info
   {
      int online_info_size = ( ( ( pData->n_zones * pData->n_zones - pData->n_zones ) / 2 ) + 
         ( sizeof( u_int ) * 8 - 1 ) ) / ( sizeof( u_int ) * 8 ) ;

      BP_LE_SwapSIntArray_Inp( pData->online_info, online_info_size );
   }
}

static void EndianSwapHzxBlock( HZX_BLOCK *pData )
{
   BP_LE_SwapSIntArray_Inp( &pData->tx, 4 );
   BP_LE_SwapSInt_Inp( &pData->extension );
   BP_LE_SwapUShort_Inp( &pData->n_segs );
   BP_LE_SwapUShort_Inp( &pData->n_flrs );
   BP_LE_SwapUShort_Inp( &pData->n_bul_segs );
   BP_LE_SwapUShort_Inp( &pData->n_bul_flrs );
   BP_LE_SwapUShort_Inp( &pData->n_traps );
   BP_LE_SwapUShort_Inp( &pData->block_no );

   BP_LE_SwapPtr_Inp( &pData->segs );
   BP_LE_SwapPtr_Inp( &pData->flrs );
   BP_LE_SwapPtr_Inp( &pData->bul_segs );
   BP_LE_SwapPtr_Inp( &pData->bul_flrs );
   BP_LE_SwapPtr_Inp( &pData->traps );
}

static void EndianSwapHzxVuSeg( HZX_VuSEG *pData )
{
   BP_LE_SwapSShortArray_Inp( &pData->b1.vx, 4 );
   BP_LE_SwapSShortArray_Inp( &pData->b2.vx, 4 );
   BP_LE_SwapPtr_Inp( &pData->verts );
   BP_LE_SwapUInt_Inp( &pData->atr ); // BP: TODO fixup verts array
   BP_LE_SwapUInt_Inp( &pData->size );
   BP_LE_SwapPtr_Inp( &pData->tag );
}

static void EndianSwapHzxVuSeg_Verts( HZX_VuSEG *pData )
{
   int totalVertCount = pData->b1.pad; // yes, b1.pad
   if ( totalVertCount )
   {
      BP_LE_SwapSShortArray_Inp( &( pData->verts[0].vx ), 4 * totalVertCount );
   }
}

static void EndianSwapHzxTrp( HZX_TRP *pData )
{
   BP_LE_SwapSShortArray_Inp( &pData->b1.vx, 4 );
   BP_LE_SwapSShortArray_Inp( &pData->b2.vx, 4 );
   BP_LE_SwapSInt_Inp( &pData->name_id );
}

static void EndianSwapHzxBehind( HZX_BEHIND *pData )
{
   BP_LE_SwapFloatArray_Inp( &pData->b1.vx, 4 );
   BP_LE_SwapFloatArray_Inp( &pData->b2.vx, 4 );
   BP_LE_SwapFloatArray_Inp( &( pData->v[0] ), 6 );
   BP_LE_SwapFloatArray_Inp( &( pData->right[0] ), 6 );
   BP_LE_SwapFloatArray_Inp( &( pData->left[0] ), 6 );
   BP_LE_SwapSInt_Inp( &pData->value );
   BP_LE_SwapSInt_Inp( &pData->flag );
}

static void EndianSwapHzxZon( HZX_ZON *pData )
{
   BP_LE_SwapFloatArray_Inp( &pData->x, 3 );
#ifdef	HW_SHORT_TO_FLOAT
   BP_LE_SwapFloatArray_Inp( &pData->w, 2 );
#else
   BP_LE_SwapSShortArray_Inp( &pData->w, 2 );
#endif
   BP_LE_SwapSShort_Inp( &pData->flag );
   BP_LE_SwapUIntArray_Inp( &( pData->safe_types[0] ), HZX_MAX_SAFEZONE_NUM );
}

/* ＨＺＸファイル初期化 */
static	void	LoadInitHzx( def, id )
HZX_DEF		*def ;
int		id ;
{
	int		i, j, n, head ;
	HZX_GRP	*grp ;
	HZX_HDL	*hdl ;
	HZX_BLOCK	*blk ;
	HZX_VuSEG	*seg ;
	int		*tag, *stag, n_flrs, n_segs ;
	int		*tag_start ;

   EndianSwapHzxDef( def );

	printf( "hzx version %d\n", def->version ) ;
	ASSERT( def->version == HZX_VERSION ) ;

	/* ファイル内情報の調整 */
	head = ( u_int )def ;
	def->patrols = ( HZX_PAT * )( head + ( u_int )( def->patrols ) ) ;
	def->points = ( HZX_PTP * )( head + ( u_int )( def->points ) ) ;
	def->cle_areas = ( HZX_CLE_AREA * )( head + ( u_int )( def->cle_areas ) ) ;
	def->cle_roots = ( HZX_CLE_ROOT * )( head + ( u_int )( def->cle_roots ) ) ;
	def->cle_points = ( HZX_CLE_PTP * )( head + ( u_int )( def->cle_points ) ) ;
	def->groups = ( HZX_GRP * )( head + ( u_int )( def->groups ) ) ;

	SetupPatrols( def->patrols, def->n_patrols, def ) ;
	SetupClearings( def->cle_areas, def->n_clears, def ) ;

	i = def->n_groups ;
	grp = def->groups ;

	n_flrs = n_segs = 0 ;
	while( -- i >= 0 ) {
      EndianSwapHzxGrp( grp );
		if ( grp->n_behinds > 0 ) 
      {
		   int behindIndex;
         grp->behinds = ( HZX_BEHIND * )( head + ( u_int )grp->behinds ) ;

         for ( behindIndex = 0; behindIndex < grp->n_behinds; ++behindIndex )
         {
            EndianSwapHzxBehind( grp->behinds + behindIndex );
         }
      }
		if ( grp->n_zones > 0 ) 
      {
         int zoneIndex;

			grp->zones = ( HZX_ZON * )( head + ( u_int )grp->zones ) ;

         for ( zoneIndex = 0; zoneIndex < grp->n_zones; ++zoneIndex )
         {
            EndianSwapHzxZon( grp->zones + zoneIndex );
         }
      }
		if ( grp->n_zones > 1 ) 
			grp->route = ( u_char * )( head + ( u_int )grp->route ) ;
		if ( grp->n_zones > 1 ) 
			grp->online_info = ( u_int * )( head + ( u_int )grp->online_info ) ;
#ifdef ROUTE_COURSE_TEST
		if ( grp->n_zones > 1 ) {
			grp->route_course[0] = ( u_short * )( head + ( u_int )grp->route_course[0] ) ;
			grp->route_course[1] = ( u_short * )( head + ( u_int )grp->route_course[1] ) ;
			grp->route_course[2] = ( u_short * )( head + ( u_int )grp->route_course[2] ) ;
			grp->route_course[3] = ( u_short * )( head + ( u_int )grp->route_course[3] ) ;
		}
#endif

		if ( grp->n_link_zones > 0 ) 
			grp->link_zone = ( u_char * )( head + ( u_int )grp->link_zone ) ;
		if ( grp->n_blocks > 0 ) 
			grp->blocks = ( HZX_BLOCK * )( head + ( u_int )grp->blocks ) ;

      EndianSwapHzxGrp_PostPtrFixup( grp );
      
      j = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- j >= 0 ) {
            int trapIndex;
            EndianSwapHzxBlock( blk );

			if ( blk->n_segs > 0 ) 
				blk->segs = ( HZX_VuSEG * )( head + ( u_int )blk->segs ) ;
			else	
				blk->segs = NULL ;
			if ( blk->n_flrs > 0 ) 
				blk->flrs = ( HZX_VuSEG * )( head + ( u_int )blk->flrs ) ;
			else	
				blk->flrs = NULL ;
			if ( blk->n_bul_segs > 0 ) 
				blk->bul_segs = ( HZX_VuSEG * )( head + ( u_int )blk->bul_segs ) ;
			else	
				blk->bul_segs = NULL ;
			if ( blk->n_bul_flrs > 0 ) 
				blk->bul_flrs = ( HZX_VuSEG * )( head + ( u_int )blk->bul_flrs ) ;
			else	
				blk->bul_flrs = NULL ;
			if ( blk->n_traps > 0 ) {
				blk->traps = ( HZX_TRP * )( head + ( u_int )blk->traps ) ;
			} else	
				blk->traps = NULL ;
			n = blk->n_segs ;
			seg = blk->segs ;
			n_segs += n ;
			while( -- n >= 0 ) {
            EndianSwapHzxVuSeg( seg );
				seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
            EndianSwapHzxVuSeg_Verts( seg );
				seg ++ ;
			}
			seg = blk->flrs ;
			n = blk->n_flrs ;
			n_flrs += n ;
			while( -- n >= 0 ) {
            EndianSwapHzxVuSeg( seg );
				seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
            EndianSwapHzxVuSeg_Verts( seg );
				seg ++ ;
			}
			n = blk->n_bul_segs ;
			seg = blk->bul_segs ;
			n_segs += n ;
			while( -- n >= 0 ) {
            EndianSwapHzxVuSeg( seg );
				seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
            EndianSwapHzxVuSeg_Verts( seg );
				seg ++ ;
			}
			seg = blk->bul_flrs ;
			n = blk->n_bul_flrs ;
			n_flrs += n ;
			while( -- n >= 0 ) {
            EndianSwapHzxVuSeg( seg );
				seg->verts = ( SVECTOR * )( head + ( u_int )seg->verts ) ;
            EndianSwapHzxVuSeg_Verts( seg );
				seg ++ ;
			}

         for ( trapIndex = 0; trapIndex < blk->n_traps; ++trapIndex )
         {
            EndianSwapHzxTrp( blk->traps + trapIndex );
         }

			blk ++ ;
		}
		//	grp->route = NULL ;
		grp->dynamics = NULL ;
		grp ++ ;
	}
	/* 壁床のタグ作成 */
	i = def->n_groups ;
	grp = def->groups ;
	if ( n_flrs + n_segs > 0 ) {
		tag = GV_Malloc( sizeof( int ) * 12 * n_flrs + sizeof( int ) * 4 * n_segs ) ;
		tag_start = tag ;
		stag = tag + 12 * n_flrs ;
	} else {
		tag = tag_start = NULL ;
		stag = NULL ;
	}
	for ( i = 0; i < def->n_groups; i ++ ) {
		j = grp->n_blocks ;
		blk = grp->blocks ;
		while( -- j >= 0 ) {
			seg = blk->segs ;
			for ( n = 0; n < blk->n_segs; n ++, seg ++ ) {
				MakeSegTag( stag, blk, seg, n, i ) ;
				stag += 4 ;
			}	    
			seg = blk->flrs ;
			for ( n = 0; n < blk->n_flrs; n ++, seg ++ ) {
				MakeFlrTag( tag, blk, seg, n, i ) ;
				tag += 12 ;
			}	    
			seg = blk->bul_segs ;
			for ( n = 0; n < blk->n_bul_segs; n ++, seg ++ ) {
				MakeSegTag( stag, blk, seg, n + HZX_RECOIL_TYPE_SHIFT, i ) ;
				stag += 4 ;
			}	    
			seg = blk->bul_flrs ;
			for ( n = 0; n < blk->n_bul_flrs; n ++, seg ++ ) {
				MakeFlrTag( tag, blk, seg, n + HZX_RECOIL_TYPE_SHIFT, i ) ;
				tag += 12 ;
			}	    
			blk ++ ;
		}
		grp ++ ;
	}
#ifdef PSX2	
	FlushCache( 0 ) ;
#endif	
	/* ハンドラーの作成 */
	hdl = HZX_MakeHandler( def, id, 0 ) ;
	hdl->tag = tag_start ;
	HzxCache[ N_HzxCaches ] = hdl ;
	N_HzxCaches ++ ;
#ifdef DEBUG_MODE
	if ( HZX_CurrentHzx != NULL ) {
		printf( "hzx must be only one file\n" ) ;
		ASSERT( 0 ) ;
	}
#endif
	HZX_CurrentHzx = hdl ;
}

/* ローダ */
int			HZX_LoadHzx( hzx, id )
HZX_DEF		*hzx ;
int		id ;
{
	LoadInitHzx( hzx, id ) ;
	return 1 ;
}
