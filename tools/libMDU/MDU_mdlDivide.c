/*
   MDU_mdlUtil.c

   モデルユーティリティ for MGS2 on linux

   by M.Sonoyama 1999.Sep.～ 
   $Id: MDU_mdlDivide.c,v 1.17 2000/03/28 01:09:38 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

/*----------------------------------------------------------

   モデル分割関数 
   KM3_DEF2		*MDU_DivideKm3( orgdef2, div )
   KM3_DEF2		*orgdef2 ;
   SVECTOR		*div ; 		 分割数 

   属性による分割 
   KM3_DEF2		*MDU_SplitKm3( orgdef2 )
   KM3_DEF2		*orgdef2 ;
   
------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "MDU_util.h"
#include "MDU_mdl.h"
#include "MDU_tex.h"

#define	MEM_SIZE	(8 * 1024)
extern size_t mallocblksize( void *memblock ) ;

/*--------------------------------------------------------------*/

/* モデル分割（関節モデルは不可） */
/* 元の頂点情報を壊します */

static	int	N_Types, N_Models ;
static	int	Type[ 256 ] ;
static	SVECTOR	*Div ;
static	FVECTOR	BoundMin, BoundMax, BlockSize ;
static	KM3_MDL	*Model ;

/* ポリゴン追加用 */
static	void	AddPolygon( dst, src, n )
KM3_MDL		*dst, *src ;
int		n ; /* src内 n番目のポリゴン */
{
    FVECTOR	v, *verts, *norms, *envs ;
    u_int	*tid, size ;
    TVECTOR	*uvs ;
    u_short	*vid, *nid ;

    /* 領域の再確保 */
    if ( dst->n_prims == 0 ) {
#ifdef MEM_SIZE
	verts = dst->verts = ( FVECTOR * )MDU_Alloc( MEM_SIZE ) ;
	envs = dst->envs = ( FVECTOR * )MDU_Alloc( MEM_SIZE ) ;
	norms = dst->norms = ( FVECTOR * )MDU_Alloc( MEM_SIZE ) ;
	uvs = dst->uvs = ( TVECTOR * )MDU_Alloc( MEM_SIZE ) ;
	tid = dst->tid = ( u_int * )MDU_Alloc( MEM_SIZE ) ;
	vid = dst->vid = ( u_short * )MDU_Alloc( MEM_SIZE ) ;
	nid = dst->nid = ( u_short * )MDU_Alloc( MEM_SIZE ) ;
#else
	verts = dst->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * 4 ) ;
	envs = dst->envs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * 4 ) ;
	norms = dst->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * 4 ) ;
	uvs = dst->uvs = ( TVECTOR * )MDU_Alloc( sizeof( TVECTOR ) * 4 ) ;
	tid = dst->tid = ( u_int * )MDU_Alloc( sizeof( u_int ) ) ;
	vid = dst->vid = ( u_short * )MDU_Alloc( sizeof( u_short ) * 4 ) ;
	nid = dst->nid = ( u_short * )MDU_Alloc( sizeof( u_short ) * 4 ) ;
#endif
//	vid[ 0 ] = nid[ 0 ] = 0 ;
//	vid[ 1 ] = nid[ 1 ] = 1 ;
//	vid[ 2 ] = nid[ 2 ] = 2 ;
//	vid[ 3 ] = nid[ 3 ] = 3 ;
	N_Models ++ ;
    } else {
#ifndef MEM_SIZE
	verts = ( FVECTOR * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( FVECTOR ) * 4 ) ;
	envs = ( FVECTOR * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( FVECTOR ) * 4 ) ;
	norms = ( FVECTOR * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( FVECTOR ) * 4 ) ;
	uvs = ( TVECTOR * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( TVECTOR ) * 4 ) ;
	tid = ( u_int * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( u_int ) ) ;
	vid = ( u_short * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( u_short ) * 4 ) ;
	nid = ( u_short * )MDU_Alloc( ( dst->n_prims + 1 ) * sizeof( u_short ) * 4 ) ;
	memcpy( verts, dst->verts, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	memcpy( envs, dst->envs, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	memcpy( norms, dst->norms, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	memcpy( uvs, dst->uvs, sizeof( TVECTOR ) * 4 * dst->n_prims ) ;
	memcpy( tid, dst->tid, sizeof( u_int ) * dst->n_prims ) ;
	memcpy( vid, dst->vid, sizeof( u_short ) * 4 * dst->n_prims ) ;	
	memcpy( nid, dst->nid, sizeof( u_short ) * 4 * dst->n_prims ) ;	
	MDU_Free( dst->verts ) ;
	MDU_Free( dst->envs ) ;
	MDU_Free( dst->norms ) ;
	MDU_Free( dst->uvs ) ;
	MDU_Free( dst->tid ) ;
	MDU_Free( dst->vid ) ;
	MDU_Free( dst->nid ) ;
	dst->verts = verts ;
	dst->envs = envs ;
	dst->norms = norms ;
	dst->uvs = uvs ;
	dst->tid = tid ;
	dst->vid = vid ;
	dst->nid = nid ;
#else
	size = ( dst->n_prims + 1 ) * sizeof( FVECTOR ) * 4 ;
	if ( mallocblksize( dst->verts ) < size ) {
	    size = mallocblksize( dst->verts ) + MEM_SIZE ;
	    verts = ( FVECTOR * )MDU_Alloc( size ) ;
	    envs = ( FVECTOR * )MDU_Alloc( size ) ; 
	    norms = ( FVECTOR * )MDU_Alloc( size ) ; 
	    uvs = ( TVECTOR * )MDU_Alloc( size ) ; 
	    tid = ( u_int * )MDU_Alloc( size ) ; 
	    vid = ( u_short * )MDU_Alloc( size ) ; 
	    nid = ( u_short * )MDU_Alloc( size ) ; 
	    memcpy( verts, dst->verts, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	    memcpy( envs, dst->envs, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	    memcpy( norms, dst->norms, sizeof( FVECTOR ) * 4 * dst->n_prims ) ;
	    memcpy( uvs, dst->uvs, sizeof( TVECTOR ) * 4 * dst->n_prims ) ;
	    memcpy( tid, dst->tid, sizeof( u_int ) * dst->n_prims ) ;
	    memcpy( vid, dst->vid, sizeof( u_short ) * 4 * dst->n_prims ) ;	
	    memcpy( nid, dst->nid, sizeof( u_short ) * 4 * dst->n_prims ) ;	
	    MDU_Free( dst->verts ) ;
	    MDU_Free( dst->envs ) ;
	    MDU_Free( dst->norms ) ;
	    MDU_Free( dst->uvs ) ;
	    MDU_Free( dst->tid ) ;
	    MDU_Free( dst->vid ) ;
	    MDU_Free( dst->nid ) ;
	    dst->verts = verts ;
	    dst->envs = envs ;
	    dst->norms = norms ;
	    dst->uvs = uvs ;
	    dst->tid = tid ;
	    dst->vid = vid ;
	    dst->nid = nid ;
	}
#endif
	verts = dst->verts + dst->n_prims * 4 ;
	envs = dst->envs + dst->n_prims * 4 ;
	norms = dst->norms + dst->n_prims * 4 ;
	uvs = dst->uvs + dst->n_prims * 4 ;
	tid = dst->tid + dst->n_prims ;
	vid = dst->vid + dst->n_prims * 4 ;
	nid = dst->nid + dst->n_prims * 4 ;
    }

    /* 頂点＆法線ＩＤ追加 */
    vid[ 0 ] = nid[ 0 ] = dst->n_prims * 4 ;
    vid[ 1 ] = nid[ 1 ] = dst->n_prims * 4 + 1 ;
    vid[ 2 ] = nid[ 2 ] = dst->n_prims * 4 + 2 ;
    vid[ 3 ] = nid[ 3 ] = dst->n_prims * 4 + 3 ;

    /* ポリゴン情報追加 */
    /* ついでにバウンド値更新 */
    v = *( src->verts + src->vid[ n * 4 ] ) ; 
    v.vx += src->tx ; v.vy += src->ty ; v.vz += src->tz ;
    v.vx -= dst->tx ; v.vy -= dst->ty ; v.vz -= dst->tz ;
    if ( v.vx < dst->lx ) dst->lx = v.vx ;
    if ( v.vy < dst->ly ) dst->ly = v.vy ;
    if ( v.vz < dst->lz ) dst->lz = v.vz ;
    if ( v.vx > dst->ux ) dst->ux = v.vx ;
    if ( v.vy > dst->uy ) dst->uy = v.vy ;
    if ( v.vz > dst->uz ) dst->uz = v.vz ;
    *verts = v ; verts ++ ;
    v = *( src->verts + src->vid[ n * 4 + 1 ] ) ; 
    v.vx += src->tx ; v.vy += src->ty ; v.vz += src->tz ;
    v.vx -= dst->tx ; v.vy -= dst->ty ; v.vz -= dst->tz ;
    if ( v.vx < dst->lx ) dst->lx = v.vx ;
    if ( v.vy < dst->ly ) dst->ly = v.vy ;
    if ( v.vz < dst->lz ) dst->lz = v.vz ;
    if ( v.vx > dst->ux ) dst->ux = v.vx ;
    if ( v.vy > dst->uy ) dst->uy = v.vy ;
    if ( v.vz > dst->uz ) dst->uz = v.vz ;
    *verts = v ; verts ++ ;
    v = *( src->verts + src->vid[ n * 4 + 2 ] ) ; 
    v.vx += src->tx ; v.vy += src->ty ; v.vz += src->tz ;
    v.vx -= dst->tx ; v.vy -= dst->ty ; v.vz -= dst->tz ;
    if ( v.vx < dst->lx ) dst->lx = v.vx ;
    if ( v.vy < dst->ly ) dst->ly = v.vy ;
    if ( v.vz < dst->lz ) dst->lz = v.vz ;
    if ( v.vx > dst->ux ) dst->ux = v.vx ;
    if ( v.vy > dst->uy ) dst->uy = v.vy ;
    if ( v.vz > dst->uz ) dst->uz = v.vz ;
    *verts = v ; verts ++ ;
    v = *( src->verts + src->vid[ n * 4 + 3 ] ) ; 
    v.vx += src->tx ; v.vy += src->ty ; v.vz += src->tz ;
    v.vx -= dst->tx ; v.vy -= dst->ty ; v.vz -= dst->tz ;
    if ( v.vx < dst->lx ) dst->lx = v.vx ;
    if ( v.vy < dst->ly ) dst->ly = v.vy ;
    if ( v.vz < dst->lz ) dst->lz = v.vz ;
    if ( v.vx > dst->ux ) dst->ux = v.vx ;
    if ( v.vy > dst->uy ) dst->uy = v.vy ;
    if ( v.vz > dst->uz ) dst->uz = v.vz ;
    *verts = v ; 
    *envs = *( src->envs + src->vid[ n * 4 ] ) ; envs ++ ;
    *envs = *( src->envs + src->vid[ n * 4 + 1 ] ) ; envs ++ ;
    *envs = *( src->envs + src->vid[ n * 4 + 2 ] ) ; envs ++ ;
    *envs = *( src->envs + src->vid[ n * 4 + 3 ] ) ;

#if 0
    {
	ENVDATA		*ed ;
	int		i ;
	verts = dst->verts + dst->n_prims * 4 ;
	envs = dst->envs + dst->n_prims * 4 ;

	for ( i = 0; i < 4; i ++ ) {
	    ed = ( ENVDATA * )&verts->vw ;
	    if ( ed->parent != 255 ) {
		printf( "%d : %.1f %.1f %.1f %.1f\n", 
		         ed->parent, envs->vx, envs->vy, envs->vz, envs->vw ) ;
	    }
	    verts ++ ; envs ++ ;
	}
    }
    {
	ENVDATA		*ed ;
	int		i ;

	for ( i = 0; i < 4; i ++ ) {
	    verts = src->verts + src->vid[ n * 4 + i ] ;
	    envs = src->envs + src->vid[ n * 4 + i ] ;
	    ed = ( ENVDATA * )&verts->vw ;
	    if ( ed->parent != 255 ) {
		printf( "%d : %.1f %.1f %.1f %.1f\n", 
		         ed->parent, envs->vx, envs->vy, envs->vz, envs->vw ) ;
	    }
	}
    }
#endif

    *norms = *( src->norms + src->nid[ n * 4 ] ) ; norms ++ ;
    *norms = *( src->norms + src->nid[ n * 4 + 1 ] ) ; norms ++ ;
    *norms = *( src->norms + src->nid[ n * 4 + 2 ] ) ; norms ++ ;
    *norms = *( src->norms + src->nid[ n * 4 + 3 ] ) ; 
    *uvs = *( src->uvs + n * 4 ) ; uvs ++ ;
    *uvs = *( src->uvs + n * 4 + 1 ) ; uvs ++ ;
    *uvs = *( src->uvs + n * 4 + 2 ) ; uvs ++ ;
    *uvs = *( src->uvs + n * 4 + 3 ) ;
    *tid = *( src->tid + n ) ;
    dst->n_verts += 4 ;
    dst->n_norms += 4 ;
    dst->n_prims ++ ;
}


/* 全体バウンディング、分割オブジェ中心、分割オブジェ数
   などを取得 */
static	void	GetDivideParam( def2 ) 
KM3_DEF2	*def2 ;
{
    KM3_MDL	*mdl ;
    FVECTOR	*verts, v ;
    int		n_verts, n_x_models ;
    int		i, j, k, t, n, type ; 

    N_Types = 0 ;
    BoundMin.vx = BoundMin.vy = BoundMin.vz = 10000000.0F ;
    BoundMax.vx = BoundMax.vy = BoundMax.vz = -10000000.0F ;
    
    n_x_models = def2->def->n_x_models ;
    mdl = def2->def->models ;
    for ( i = 0; i < n_x_models; i ++ ) {
	/* タイプチェック */
	type = mdl->type ;
	for ( j = 0; j < N_Types; j ++ ) {
	    if ( type == Type[ j ] ) break ;
	}
	if ( j == N_Types ) {
	    Type[ j ] = type ;
	    N_Types ++ ;
	}
	/* 全体バウンディング計算 */
	n_verts = mdl->n_verts ;
	verts = mdl->verts ;
	while( -- n_verts >= 0 ) {
	    v = *verts ;
	    v.vx += mdl->tx ;
	    v.vy += mdl->ty ;
	    v.vz += mdl->tz ;
	    if ( BoundMin.vx > v.vx ) BoundMin.vx = v.vx ;
	    if ( BoundMin.vy > v.vy ) BoundMin.vy = v.vy ;
	    if ( BoundMin.vz > v.vz ) BoundMin.vz = v.vz ;
	    if ( BoundMax.vx < v.vx ) BoundMax.vx = v.vx ;
	    if ( BoundMax.vy < v.vy ) BoundMax.vy = v.vy ;
	    if ( BoundMax.vz < v.vz ) BoundMax.vz = v.vz ;
	    verts ++ ;
	}
	mdl ++ ;
    }
    /* タイプ：オーバーレイを後ろに持ってくる */
#if 1
    {	
	int	type[ 256 ], n ;
	
	n = 0 ;
	for ( i = 0; i < N_Types; i ++ ) {
	    if ( !( Type[ i ] & ( DG_TYPE_OVERLAY0 | DG_TYPE_OVERLAY1 | 
				  DG_TYPE_OVERLAY2 ) ) ) {
		type[ n ] = Type[ i ] ; n ++ ;
	    }
	}
	for ( i = 0; i < N_Types; i ++ ) {
	    if ( Type[ i ] & DG_TYPE_OVERLAY0 ) {
		type[ n ] = Type[ i ] ; n ++ ;
	    }
	}
	for ( i = 0; i < N_Types; i ++ ) {
	    if ( Type[ i ] & DG_TYPE_OVERLAY1 ) {
		type[ n ] = Type[ i ] ; n ++ ;
	    }
	}
	for ( i = 0; i < N_Types; i ++ ) {
	    if ( Type[ i ] & DG_TYPE_OVERLAY2 ) {
		type[ n ] = Type[ i ] ; n ++ ;
	    }
	}
	for ( i = 0; i < N_Types; i ++ ) Type[ i ] = type[ i ] ;
    }
#endif

    printf( "Bound %f %f %f - ", BoundMin.vx, BoundMin.vy, BoundMin.vz ) ;
    printf( "%f %f %f\n", BoundMax.vx, BoundMax.vy, BoundMax.vz ) ;
#if 1
    BlockSize.vx = ( float )Div->vx ;
    BlockSize.vy = ( float )Div->vy ;
    BlockSize.vz = ( float )Div->vz ;
    Div->vx = ( int )( ( BoundMax.vx - BoundMin.vx ) / ( float )Div->vx ) + 1 ;
    Div->vy = ( int )( ( BoundMax.vy - BoundMin.vy ) / ( float )Div->vy ) + 1 ;
    Div->vz = ( int )( ( BoundMax.vz - BoundMin.vz ) / ( float )Div->vz ) + 1 ;
    printf( "div %d %d %d\n", Div->vx, Div->vy, Div->vz ) ;
#else
    BlockSize.vx = ( BoundMax.vx - BoundMin.vx ) / ( float )Div->vx ;
    BlockSize.vy = ( BoundMax.vy - BoundMin.vy ) / ( float )Div->vy ;
    BlockSize.vz = ( BoundMax.vz - BoundMin.vz ) / ( float )Div->vz ;
#endif
    /* 分配用モデル領域の確保 */
    Model = ( KM3_MDL * )MDU_Alloc( sizeof( KM3_MDL ) * N_Types *  Div->vx * 
				  Div->vy * Div->vz ) ;
    if ( Model == NULL ) return ;
    /* 中心座標の設定 */
    n = 0 ;
    for ( t = 0; t < N_Types; t ++ ) {
	for ( i = 0; i < Div->vz; i ++ ) {
	    for ( j = 0; j < Div->vy; j ++ ) {
		for ( k = 0; k < Div->vx; k ++ ) {
		    Model[ n ].type = Type[ t ] ;
		    Model[ n ].n_prims = 0 ;
		    Model[ n ].parent = -1 ;
		    Model[ n ].extend = -1 ;
		    Model[ n ].n_verts = 0 ;
		    Model[ n ].n_norms = 0 ;
		    Model[ n ].lx = Model[ n ].ly = Model[ n ].lz = 10000000.0F ;
		    Model[ n ].ux = Model[ n ].uy = Model[ n ].uz = -10000000.0F ;
//		    Model[ n ].tx = BoundMin.vx + BlockSize.vx * k + BlockSize.vx / 2.0F ;
//		    Model[ n ].ty = BoundMin.vy + BlockSize.vy * j + BlockSize.vy / 2.0F ;
//		    Model[ n ].tz = BoundMin.vz + BlockSize.vz * i + BlockSize.vz / 2.0F ;
#if 0
		    Model[ n ].tx = 0.0F ;
		    Model[ n ].ty = 0.0F ;
		    Model[ n ].tz = 0.0F ;
#else
		    /* 元モデル０番のトランスに合わせる */
		    Model[ n ].tx = def2->def->models[ 0 ].tx ;
		    Model[ n ].ty = def2->def->models[ 0 ].ty ;
		    Model[ n ].tz = def2->def->models[ 0 ].tz ;
#endif
		    n ++ ;
		}
	    }
	}
    }
}

/* ポリゴンがどのブロックに入るか調べる */
/* 各頂点の平均値を用いる */
static	int	CheckInsideBlock( mdl, n )
KM3_MDL		*mdl ;
int		n ;
{
    FVECTOR	v[ 4 ], c ;
    int		x, y, z ;
    int		res, n_models ;

    v[ 0 ] = *( mdl->verts + mdl->vid[ n * 4 ] ) ;
    v[ 1 ] = *( mdl->verts + mdl->vid[ n * 4 + 1 ] ) ; 
    v[ 2 ] = *( mdl->verts + mdl->vid[ n * 4 + 2 ] ) ;
    v[ 3 ] = *( mdl->verts + mdl->vid[ n * 4 + 3 ] ) ;
    v[ 0 ].vx += mdl->tx ;
    v[ 0 ].vy += mdl->ty ;
    v[ 0 ].vz += mdl->tz ;
    v[ 1 ].vx += mdl->tx ;
    v[ 1 ].vy += mdl->ty ;
    v[ 1 ].vz += mdl->tz ;
    v[ 2 ].vx += mdl->tx ;
    v[ 2 ].vy += mdl->ty ;
    v[ 2 ].vz += mdl->tz ;
    v[ 3 ].vx += mdl->tx ;
    v[ 3 ].vy += mdl->ty ;
    v[ 3 ].vz += mdl->tz ;
    if ( v[ 2 ].vx == v[ 3 ].vx &&
	 v[ 2 ].vy == v[ 3 ].vy &&
	 v[ 2 ].vz == v[ 3 ].vz ) {
	/* 三角形 */
	c.vx = ( v[ 0 ].vx + v[ 1 ].vx + v[ 2 ].vx ) / 3.0F ;
	c.vy = ( v[ 0 ].vy + v[ 1 ].vy + v[ 2 ].vy ) / 3.0F ;
	c.vz = ( v[ 0 ].vz + v[ 1 ].vz + v[ 2 ].vz ) / 3.0F ;
    } else {
	c.vx = ( v[ 0 ].vx + v[ 1 ].vx + v[ 2 ].vx + v[ 3 ].vx ) / 4.0F ;
	c.vy = ( v[ 0 ].vy + v[ 1 ].vy + v[ 2 ].vy + v[ 3 ].vy ) / 4.0F ;
	c.vz = ( v[ 0 ].vz + v[ 1 ].vz + v[ 2 ].vz + v[ 3 ].vz ) / 4.0F ;
    }
    x = ( int )( ( c.vx - BoundMin.vx ) / BlockSize.vx ) ;
    y = ( int )( ( c.vy - BoundMin.vy ) / BlockSize.vy ) ;
    z = ( int )( ( c.vz - BoundMin.vz ) / BlockSize.vz ) ;
    n_models = Div->vx * Div->vy * Div->vz ;
    res = x + y * Div->vx + z * ( Div->vx * Div->vy ) ;
    if ( res < 0 ) res = 0 ;
    if ( res >= n_models ) res = n_models - 1 ;
    return res ;
}

/* ポリゴン振り分け */
static	void	Distribute( def2 ) 
KM3_DEF2	*def2 ;
{
    int		i, n_x_models, n_prims, n, typeNo ;
    KM3_MDL	*src, *dst ;
    
    dst = Model ;
    src = def2->def->models ;
    n_x_models = def2->def->n_x_models ;
    while( -- n_x_models >= 0 ) {
	printf( "." ) ;
	n_prims = src->n_prims ;
	for ( i = 0; i < N_Types; i ++ ) {
	    if ( src->type == Type[ i ] ) break ;
	}
	if ( i == N_Types ) printf( "??????\n" ) ;
	typeNo = i ;
	for ( i = 0; i < n_prims; i ++ ) {
	    n = CheckInsideBlock( src, i ) ;
	    n += typeNo * Div->vx * Div->vy * Div->vz ;
	    AddPolygon( dst + n, src, i ) ;
	}
	src ++ ;
    }
    printf( "\n" ) ;
}

/* モデル分割関数 */
KM3_DEF2	*MDU_DivideKm3( orgdef2, div )
KM3_DEF2	*orgdef2 ;
SVECTOR		*div ; /* 分割数 */
{
    KM3_DEF2	*def2 ;
    KM3_DEF	*def ;
    KM3_MDL	*mdl ;
    int		i ;

    def2 = ( KM3_DEF2 * )MDU_Alloc( sizeof( KM3_DEF2 ) ) ;
    if ( def2 == NULL ) return NULL ;

    Div = div ; Model = NULL ;
    /* 一度コピー */
    memcpy( def2, orgdef2, sizeof( KM3_DEF2 ) ) ;
    /* 整数化しておく */
//    MDU_ModelVertsInt( def2->def ) ;
    /* 全体バウンディング、分割オブジェ中心、分割オブジェ数
       などを取得 */
    printf( "model divide start\n" ) ;
    GetDivideParam( def2 ) ;
    /* ポリゴン振り分け */
    N_Models = 0 ;
    Distribute( def2 ) ;
    /* 新KM3_DEF領域確保 */
    if ( N_Models == 0 ) return NULL ;
    def = ( KM3_DEF * )MDU_Alloc( sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * 
			       N_Models ) ;
    def->n_models = def->n_x_models = N_Models ;
    def->lx = BoundMin.vx - Model[ 0 ].tx ;
    def->ly = BoundMin.vy - Model[ 0 ].ty ;
    def->lz = BoundMin.vz - Model[ 0 ].tz ;
    def->ux = BoundMax.vx - Model[ 0 ].tx ;
    def->uy = BoundMax.vy - Model[ 0 ].ty ;
    def->uz = BoundMax.vz - Model[ 0 ].tz ;
    i = 0 ;	
    mdl = Model ;
    while( N_Models > 0 ) {
	if ( mdl->n_prims > 0 ) {
	    memcpy( &def->models[ i ], mdl, sizeof( KM3_MDL ) ) ;
	    i ++ ;
	    N_Models -- ;
	}
	mdl ++ ;
    }
    /* 頂点＆法線共有化 */
    MDU_JointModel( def ) ;
    /* 今までのdefをフリー */
    MDU_mdlFreeKm3Def2( orgdef2 ) ;
    /* 新しいdefを接続 */
    def2->def = def ;
    if ( Model != NULL ) MDU_Free( Model ) ;
    return def2 ;
}

/*--------------------------------------------------------------------------*/

/* 属性によるモデル分割 */
/* １：半透明オブジェに不透明テクスチャ → 新不透明オブジェ作成 
   ２：不透明オブジェに半透明テクスチャ → 新半透明オブジェ作成 
   ３：キャラモデルなら、半透明テクスチャ部分は拡張モデル化 */

/* ３番用分割処理 */
static	void	MakeSplitModel( def2 )
KM3_DEF2	*def2 ;
{
    int		n_extends, n_models, buf ;
    int		n_prims, i, n, c, m ;
    KM3_MDL	*mdl, *src, *dst, *ext ;
    u_int	*tid ;

    mdl = def2->def->models ;
    n_models = def2->def->n_models ;
    n_extends = 0 ;
    /* 拡張モデル数をカウント */
    while( -- n_models >= 0 ) {
	tid = mdl->tid ;
	n_prims = mdl->n_prims ;
	while( -- n_prims >= 0 ) {
	    if ( IsTransTexture( def2->texs[ *tid ] ) ) {
		n_extends ++ ;
		break ;
	    }
	    tid ++ ;
	}
	mdl ++ ;
    }
    if ( n_extends == 0 ) return ;
    printf( "n_extends %d\n", n_extends ) ;

    /* 領域確保 */
    N_Models += n_extends ;
    Model = ( KM3_MDL * )MDU_Alloc( sizeof( KM3_MDL ) * N_Models ) ;
    memset( Model, 0x00, sizeof( KM3_MDL ) * N_Models ) ;

    n_models = def2->def->n_models ;	   
    src = def2->def->models ;
    dst = Model ;
    ext = Model + n_models ;
    n_extends = 0 ; 
    m = 0 ;
    /* 拡張モデル作成 */
    while( -- n_models >= 0 ) {
	tid = src->tid ;
	n = c = 0 ;
	for ( i = 0; i < src->n_prims; i ++ ) {
	    if ( IsTransTexture( def2->texs[ *tid ] ) ) {
		if ( n == 0 ) {
		    n = 1 ;
		    ext->type = src->type | DG_TYPE_EXTEND | DG_TYPE_OVERLAY0 ;
		    ext->tx = src->tx ;
		    ext->ty = src->ty ;
		    ext->tz = src->tz ;
//		    ext->parent = src->parent ;
		    ext->parent = m ;
printf( "extend parent %d : type %x\n", ext->parent, ext->type ) ;
		    ext->extend = -1 ;
		    ext->n_prims = 0 ;
		}
//printf( "add extend %d\n", i ) ;
		buf = N_Models ;
		AddPolygon( ext, src, i ) ;
		N_Models = buf ;
	    } else {
		if ( c == 0 ) {
		    c = 1 ;
		    dst->type = src->type ;
		    dst->tx = src->tx ;
		    dst->ty = src->ty ;
		    dst->tz = src->tz ;
		    dst->parent = src->parent ;
		    dst->extend = -1 ;
		    dst->n_prims = 0 ;
		}
		buf = N_Models ;
		AddPolygon( dst, src, i ) ; 
		N_Models = buf ;
	    }
	    tid ++ ;
	}
	if ( dst->n_prims == 0 ) {
	    dst->type = src->type ;
	    dst->tx = src->tx ;
	    dst->ty = src->ty ;
	    dst->tz = src->tz ;
	    dst->parent = src->parent ;
	    dst->extend = -1 ;	    
	} 

	/* バウンディングはもとのまま */
	mdl = def2->def->models + m ;
	dst->lx = mdl->lx ;
	dst->lx = mdl->ly ;
	dst->lx = mdl->lz ;
	dst->ux = mdl->ux ;
	dst->uy = mdl->uy ;
	dst->uz = mdl->uz ;

	if ( ext->n_prims != 0 ) {
//	    dst->extend = n_extends ;
	    n_extends ++ ;
	    ext ++ ;
	}
	dst ++ ;
	src ++ ;
	m ++ ;
    }
}

/* 今は３番のみ */
KM3_DEF2	*MDU_SplitKm3( orgdef2 )
KM3_DEF2	*orgdef2 ;
{
    KM3_DEF2	*def2 ;
    KM3_DEF	*def ;
    KM3_MDL	*mdl ;
    int		i ;

    Model = NULL ;
    /* ポリゴン振り分け */
    N_Models = orgdef2->def->n_models ;
    MakeSplitModel( orgdef2 ) ; /* N_Modelsは総モデル数になる */
    /* 新KM3_DEF領域確保 */
    if ( N_Models == orgdef2->def->n_models ) {
	/* 分割の必要なし */
	printf( "split by transparent not needed\n" ) ;
	return orgdef2 ;
    }

    def2 = ( KM3_DEF2 * )MDU_Alloc( sizeof( KM3_DEF2 ) ) ;
    if ( def2 == NULL ) return NULL ;
    /* 一度コピー */
    memcpy( def2, orgdef2, sizeof( KM3_DEF2 ) ) ;
    def = ( KM3_DEF * )MDU_Alloc( sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * 
			       N_Models ) ;
    def->n_models = orgdef2->def->n_models ;
    def->n_x_models = N_Models ;
    def->lx = orgdef2->def->lx ;
    def->ly = orgdef2->def->ly ;
    def->lz = orgdef2->def->lz ;
    def->ux = orgdef2->def->ux ;
    def->uy = orgdef2->def->uy ;
    def->uz = orgdef2->def->ux ;

    i = 0 ;	
    mdl = Model ;
    while( N_Models > 0 ) {
	memcpy( &def->models[ i ], mdl, sizeof( KM3_MDL ) ) ;
	i ++ ;
	N_Models -- ;
	mdl ++ ;
    }

    /* 頂点＆法線共有化 */
    MDU_JointModel( def ) ;
    MDU_mdlFreeKm3Def2( orgdef2 ) ;
    /* 新しいdefを接続 */
    def2->def = def ;
    if ( Model != NULL ) MDU_Free( Model ) ;
    return def2 ;
}

