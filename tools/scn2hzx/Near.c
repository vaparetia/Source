/*
   Near.c
   
   ニアハザードチェックＣ版
   2000/01/24 M.Sonoyama
   $Id: Near.c,v 1.2 2000/10/28 04:42:55 usr02011 Exp $
*/

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<sys/types.h> 
#include	<math.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include	"Scene2HZX.h"
#include	"HZXStruct2.h"
#include	"fmt_hzx.h"

typedef	struct	{
    FVECTOR		nearest ;
    FVECTOR		edge1 ;
    FVECTOR		edge2 ;
    long		is_edge ;
    float		length ;
    int			g, b, s, v ;
} Nearest ;

static	FVECTOR		From ;
static	int		Sphere, R_Sphere ;
static	int		InsideBlock[ 256 ] ;
static	int		N_InsideBlocks ; 
static	FVECTOR		Bound1, Bound2 ;
static	int		N_Nears ;
static	Nearest		First, Second, This ;
static	FVECTOR		P1v, P2v ;
static	FVECTOR		P1_P2v, P1_FROMv ;
static	float		Hoh, Doh, Dxh ;

#define	FROM		(&From) 
#define	SPHERE		(&Sphere)
#define	R_SPHERE	(&R_Sphere)
#define	BOUND1		(&Bound1)
#define	BOUND2		(&Bound2)
#define	N_NEARS		(&N_Nears)
#define	THIS		(&This)
#define	FIRST		(&First)
#define	SECOND		(&Second)
#define	P1		(&P1v)
#define	P2		(&P2v)
#define	P1_P2		(&P1_P2v)
#define	P1_FROM		(&P1_FROMv)
#define	DOH		(&Doh)
#define	HOH		(&Hoh)
#define	DXH		(&Dxh)

/* 初期化 */
static	void	InitNearData( void )
{
    int		d ;

    /* バウンディング作成 */
    d = FROM->vx ;
    BOUND1->vx = d - *SPHERE ;
    BOUND2->vx = d + *SPHERE ;
    d = FROM->vz ;
    BOUND1->vz = d - *SPHERE ;
    BOUND2->vz = d + *SPHERE ;
    d = FROM->vy ;
    BOUND1->vy = BOUND2->vy = d ;    

    *N_NEARS = 0 ;
    FIRST->length = SECOND->length = (*SPHERE) * (*SPHERE) ;
}

/* １セグメントバウンドチェック */
static	int	CheckSegmentConflict( void )
{
    float	d1, d2, tmp ;
    float	y, y1, y2 ;

    if ( P1->vx > P2->vx ) {
		d1 = P2->vx ; d2 = P1->vx ;
    } else {
		d2 = P2->vx ; d1 = P1->vx ;
    }
    if ( d1 > BOUND2->vx || d2 < BOUND1->vx ) return 0 ;
    if ( P1->vz > P2->vz ) {
		d1 = P2->vz ; d2 = P1->vz ;
    } else {
		d2 = P2->vz ; d1 = P1->vz ;
    }
    if ( d1 > BOUND2->vz || d2 < BOUND1->vz ) return 0 ;
    y = BOUND1->vy ;	
    y1 = P1->vy ;  
    y2 = P2->vy ; 
    if ( y1 > y && y2 > y ) return 0 ;
    y = BOUND2->vy ;	
    y1 += P1->vw ;
    y2 += P2->vw ;
    if ( y1 < y && y2 < y ) return 0 ;    
    return 1 ;
}

/* ＦＲＯＭからの最近点を計算 */
static	float	SegmentNearest( void ) 
{
    float	doh, hoh, dxh ;

    P1_P2->vx = P2->vx - P1->vx ;
    P1_P2->vz = P2->vz - P1->vz ;
    P1_FROM->vx = FROM->vx - P1->vx ;
    P1_FROM->vz = FROM->vz - P1->vz ;
    /* 内積 */
    doh = P1_P2->vx * P1_FROM->vx + P1_P2->vz * P1_FROM->vz ;
    THIS->is_edge = 1 ;
    *HOH = 1.0F ;
    if ( doh < 0.0F ) {
		/* P1が最近点 */
		THIS->nearest.vx = P1->vx - FROM->vx ;
		THIS->nearest.vz = P1->vz - FROM->vz ;
		*DOH = 0.0F ;
    } else {
		hoh = P1_P2->vx * P1_P2->vx + P1_P2->vz * P1_P2->vz ;
		if ( doh > hoh ) {
			/* P2が最近点 */
			THIS->nearest.vx = P2->vx - FROM->vx ;
			THIS->nearest.vz = P2->vz - FROM->vz ;
			*DOH = 1.0F ;
		} else {
			/* 垂線の足が最近点 */
			/* 外積 */
			dxh = P1_P2->vx * P1_FROM->vz - P1_P2->vz * P1_FROM->vx ;
			*DOH = doh ;
			*HOH = hoh ;
			THIS->nearest.vx = -P1_P2->vz * dxh / hoh ;
			THIS->nearest.vz =  P1_P2->vx * dxh / hoh ;
			THIS->is_edge = 0 ;
			THIS->edge1.vx = P1->vx ;
			THIS->edge1.vz = P1->vz ;
			THIS->edge2.vx = P2->vx ;
			THIS->edge2.vz = P2->vz ;
		}
    }
    THIS->length = THIS->nearest.vx * THIS->nearest.vx + 
		THIS->nearest.vz * THIS->nearest.vz ;
    return THIS->length ;
}

/* 傾斜壁の交点での高さを計算 */
static	void	DiagonalSegmentHeight( void ) 
{
    float	y, h ;

    y = P1->vy + ( *DOH / *HOH ) * ( P2->vy - P1->vy ) ;
    h = P1->vw + ( *DOH / *HOH ) * ( P2->vw - P1->vw ) ;
    P1->vy = y ; P1->vw = h ;
}

/* １ストリップ検査 */
static	void	CheckOneStrip( b, seg, n )
HZD_BLOCK	*b ;
HZD_VuSEG	*seg ;
int		n ;
{
    int		n_segments, i, vn ;
    SVECTOR	*v, *v1, *v2 ; 
    float	len, cross ;

    n_segments = seg->b1.vw - 1 ;
    v = seg->verts ; vn = 0 ;
    for ( i = 0; i < n_segments; i ++ ) {
		v1 = v ; v2 = v + 1 ;
		P1->vx = v1->vx + b->tx ;
		P1->vy = v1->vy + b->ty ;
		P1->vz = v1->vz + b->tz ;
		P1->vw = v1->vw ;
		P2->vx = v2->vx + b->tx ;
		P2->vy = v2->vy + b->ty ;
		P2->vz = v2->vz + b->tz ;
		P2->vw = v2->vw ;	
		if ( !CheckSegmentConflict() ) goto check_skip ;
		len = SegmentNearest() ;
		if ( len >= SECOND->length ) goto check_skip ;
		DiagonalSegmentHeight() ;
		cross = FROM->vy - P1->vy ;
		if ( cross < 0 || cross > P1->vw ) goto check_skip ;
		THIS->b = b->block_no ;
		THIS->s = n ;
		THIS->v = vn ;
		if ( len < FIRST->length ) {
			*SECOND = *FIRST ;
			*FIRST = *THIS ;
		} else {
			if ( THIS->nearest.vx == FIRST->nearest.vx &&
				THIS->nearest.vz == FIRST->nearest.vz ) goto check_skip ;
			*SECOND = *THIS ;
		}
		( *N_NEARS ) ++ ;
		check_skip :
		v ++ ; vn ++ ;
    }
}

/* １ブロック検査 */
static	void	CheckOneBlock( b, seg_flag )
HZD_BLOCK	*b ;
int		seg_flag ;
{
    HZD_VuSEG	*seg ;
    int		n_segs, n ;
    SVECTOR	b1, b2, svfrom ;

    seg = b->segs ;
    n_segs = b->n_segs ;
    /* 自分のバウンディング作成 */	
    svfrom.vx = ( short )( FROM->vx - ( float )b->tx ) ;
    svfrom.vy = ( short )( FROM->vy - ( float )b->ty ) ;
    svfrom.vz = ( short )( FROM->vz - ( float )b->tz ) ;
    b1.vx = svfrom.vx - *SPHERE ;
    b1.vy = svfrom.vy - *SPHERE ;
    b1.vz = svfrom.vz - *SPHERE ;
    b2.vx = svfrom.vx + *SPHERE ;
    b2.vy = svfrom.vy + *SPHERE ;
    b2.vz = svfrom.vz + *SPHERE ;
    for ( n = 0; n < n_segs; n ++ ) {
		/* フラグチェック */
		if ( seg->atr & seg_flag ) goto near_check_skip_strip ;
		/* バウンディングチェック */
		if ( seg->b1.vx > b2.vx || seg->b2.vx < b1.vx ||
			seg->b1.vy > b2.vy || seg->b2.vy < b1.vy ||
			seg->b1.vz > b2.vz || seg->b2.vz < b1.vz ) goto near_check_skip_strip ;
		CheckOneStrip( b, seg, n ) ;
		near_check_skip_strip :
		seg ++ ;
    }
}

/* ブロック進入チェック */
static	void	CheckBlockBound( grp )
HZD_GRP		*grp ;
{
    int		i, j, k, x1, x2, y1, y2, z1, z2, block ;
    int		min, size, dx, dy, dz ;
    HZD_BLOCK	*blk ;

    blk = grp->blocks ;
    min = grp->bound_min_x ; size = grp->block_size_x ;
    x1 = ( ( int )FROM->vx - *SPHERE - min ) / size ;
    x2 = ( ( int )FROM->vx + *SPHERE - min ) / size ;
    min = grp->bound_min_z ; size = grp->block_size_z ;
    z1 = ( ( int )FROM->vz - *SPHERE - min ) / size ;
    z2 = ( ( int )FROM->vz + *SPHERE - min ) / size ;
    min = grp->bound_min_y ; size = grp->block_size_y ;
    y1 = ( ( int )FROM->vy - min ) / size ;
    y2 = y1 ;
    N_InsideBlocks = 0 ;
    dx = grp->div_x ; dz = grp->div_z ; dy = grp->div_y ;
    for ( j = z1; j <= z2; j ++ ) {
		for ( i = x1; i <= x2; i ++ ) {
			if ( i < 0 || j < 0 || i >= dx || j >= dz || k < 0 || k >= dy ) continue ;
			block = grp->div_x * j + i + ( y1 * grp->div_x * grp->div_z ) ;
			if ( block >= 0 && block < grp->n_blocks ) {
				if ( N_InsideBlocks && 
					( block <= InsideBlock[ N_InsideBlocks - 1 ] ) ) continue ;
				if ( ( blk + block )->n_segs == 0 ) continue ;
				InsideBlock[ N_InsideBlocks ] = block ;
				N_InsideBlocks ++ ;
			}
		}
    }
}

/* ニアハザードチェック */
int		HZD_NearHazardCheck( grp, from, sphere, r_sphere, chk_flag, seg_flag )
HZD_GRP		*grp ;
FVECTOR		*from ;
int		sphere, r_sphere ;
int		chk_flag, seg_flag ;
{
    int		hit, i ;
    HZD_BLOCK	*blk, *b ;

    *FROM = *from ;
    *SPHERE = sphere ;
    *R_SPHERE = r_sphere ;

    InitNearData() ;
    blk = grp->blocks ;
    CheckBlockBound( grp ) ;
    for ( i = 0; i < N_InsideBlocks; i ++ ) {
		b = blk + InsideBlock[ i ] ;
		CheckOneBlock( b, seg_flag ) ;
    }
    /* 重複頂点をチェック */
    if ( *N_NEARS <= 1 ) goto check_end ;
    *N_NEARS = 2 ;
    if ( !SECOND->is_edge ) goto check_end ;
    if ( FIRST->is_edge ) {
		if ( FIRST->nearest.vx != SECOND->nearest.vx ||
			FIRST->nearest.vz != SECOND->nearest.vz ) goto check_end ;
    } else {
		FVECTOR	tmp ;

		tmp.vx = FROM->vx + SECOND->nearest.vx ;
		tmp.vz = FROM->vz + SECOND->nearest.vz ;
		if ( ( tmp.vx != FIRST->edge1.vx || tmp.vz != FIRST->edge1.vz ) &&
			( tmp.vx != FIRST->edge2.vx || tmp.vz != FIRST->edge2.vz ) ) goto check_end ;
    }
    *N_NEARS = 1 ;
	check_end :
    return *N_NEARS ;
}

/*-----------------------------------------------------------------------*/

void	HZD_GetNearVector( vec_ptr )
FVECTOR		*vec_ptr ;
{
    vec_ptr[ 0 ].vx = FIRST->nearest.vx ;
    vec_ptr[ 0 ].vy = FIRST->nearest.vy ;
    vec_ptr[ 0 ].vz = FIRST->nearest.vz ;
    vec_ptr[ 1 ].vx = SECOND->nearest.vx ;
    vec_ptr[ 1 ].vy = SECOND->nearest.vy ;
    vec_ptr[ 1 ].vz = SECOND->nearest.vz ;
}
