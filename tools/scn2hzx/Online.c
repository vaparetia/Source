/*
   Online.c
   
   オンラインチェック関数
   2000/01/24 M.Sonoyama
   $Id: Online.c,v 1.6 2000/11/01 05:08:16 usr03635 Exp $
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

static	FVECTOR		From, To, Diff, Step, Normal ;
static	int		InsideBlock[ 256 ] ;
static	int		N_InsideBlocks ; 
static	FVECTOR		Bound1, Bound2 ;
static	FVECTOR		P1v, P2v ;
static	FVECTOR		P1_P2v, P1_FROMv, FROM_P1v ;
static	FVECTOR		V1v, V2v, V3v, V4v ;
static	int		Which ;
static	int		Hit ;
static	int		Where[ 4 ] ;

static	float		Minlen, Len ;
static	FVECTOR		Cross, Mincross ;

#define	FROM		(&From) 
#define	TO		(&To) 
#define	DIFF		(&Diff)
#define	STEP		(&Step)
#define	NORMAL		(&Normal)
#define	BMIN		(&Bound1)
#define	BMAX		(&Bound2)
#define	P1		(&P1v)
#define	P2		(&P2v)
#define	V1		(&V1v)
#define	V2		(&V2v)
#define	V3		(&V3v)
#define	V4		(&V4v)
#define	P1_P2		(&P1_P2v)
#define	P1_FROM		(&P1_FROMv)
#define	FROM_P1		(&FROM_P1v)
#define	WHICH		(&Which)
#define	HIT		(&Hit)

#define	LEN		(&Len)
#define	CROSS		(&Cross)
#define	MINLEN		(&Minlen)
#define	MINCROSS	(&Mincross)

#define	MAX_DISTANCE	(1000000.0F)

int		OnlineDebugFlag = 0 ;

static	void	DumpVec( v )
FVECTOR		*v ;
{
    printf( "%.0f %.0f %.0f %.0f\n", v->vx, v->vy, v->vz, v->vw ) ;
}

/* 始点→終点ベクトルを、水平成分＝1.0F
   になるようスケーリング */
static	float	MakeStepXZ( void )
{
    float		len ;

    STEP->vx = TO->vx - FROM->vx ;
    STEP->vy = TO->vy - FROM->vy ;
    STEP->vz = TO->vz - FROM->vz ;
    len = sqrtf( STEP->vx * STEP->vx +	 
				STEP->vy * STEP->vy +	 
				STEP->vz * STEP->vz ) ;
    if ( len == 0.0F ) return 0.0F ;
    STEP->vx = STEP->vx / len ;
    STEP->vy = STEP->vy / len ;
    STEP->vz = STEP->vz / len ;
    return len ;
}

/* オンライン壁チェック */

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
    if ( d1 > BMAX->vx || d2 < BMIN->vx ) return 0 ;
    if ( P1->vz > P2->vz ) {
		d1 = P2->vz ; d2 = P1->vz ;
    } else {
		d2 = P2->vz ; d1 = P1->vz ;
    }
    if ( d1 > BMAX->vz || d2 < BMIN->vz ) return 0 ;
    y = BMIN->vy ;	
    y1 = P1->vy ;  
    y2 = P2->vy ; 
    if ( y1 > y && y2 > y ) return 0 ;
    y = BMAX->vy ;	
    y1 += P1->vw ;
    y2 += P2->vw ;
    if ( y1 < y && y2 < y ) return 0 ;    
    return 1 ;
}

/* 壁を直線として交点までの長さを計算 */
static	float	SegmentDistance( void ) 
{
    float	hxv, dxh, len ;
    
    P1_P2->vx = P2->vx - P1->vx ;
    P1_P2->vz = P2->vz - P1->vz ;	
    hxv = P1_P2->vx * STEP->vz - P1_P2->vz * STEP->vx ;
    if ( hxv == 0.0F ) return MAX_DISTANCE ;
    P1_FROM->vx = FROM->vx - P1->vx ;
    P1_FROM->vz = FROM->vz - P1->vz ;
    dxh = P1_FROM->vx * P1_P2->vz - P1_FROM->vz * P1_P2->vx ;
    len = dxh / hxv ;
    if ( len < 0.0F ) return MAX_DISTANCE ;
    return len ;
}

/* 壁を線分として、交差するかチェック */
static	int	CheckSegmentCross( len )
float		len ;
{
    float	cross ;

    CROSS->vx = FROM->vx + STEP->vx * len ;
    CROSS->vy = FROM->vy + STEP->vy * len ;
    CROSS->vz = FROM->vz + STEP->vz * len ;
    if ( P1_P2->vx != 0 ) {
		cross = CROSS->vx ;
		if ( cross < P1->vx ||
			cross > P2->vx ) return 0 ;
		return 1 ;
    } else {
		cross = CROSS->vz ;
		if ( cross < P1->vz ||
			cross > P2->vz ) return 0 ;
		return 2 ;
    }
}

/* 傾斜壁の交点での高さを計算 */
static	void	DiagonalSegmentHeight( axis ) 
int		axis ;
{
    float	y, h, depth ;

    if ( axis == 1 ) depth = ( CROSS->vx - P1->vx ) / P1_P2->vx ;
    else	     depth = ( CROSS->vx - P1->vz ) / P1_P2->vz ;
    y = P1->vy + depth * ( P2->vy - P1->vy ) ;
    h = P1->vw + depth * ( P2->vw - P1->vw ) ;
    P1->vy = y ; P1->vw = h ;
}

/* 1ストリップチェック */
static	void	CheckOneSegmentStrip( b, seg, n )
HZD_BLOCK	*b ;
HZD_VuSEG	*seg ;
int		n ;
{
    int		i, n_segments, axis ;
    SVECTOR	*v, *v1, *v2 ;
    float	len, cross ;

    n_segments = seg->b1.vw - 1 ;
    v = seg->verts ;
    for ( i = 0; i < n_segments; i ++, v ++ ) {
		v1 = v ; v2 = v + 1 ;
		P1->vx = v1->vx + b->tx ;
		P1->vy = v1->vy + b->ty ;
		P1->vz = v1->vz + b->tz ;
		P1->vw = v1->vw ;
		P2->vx = v2->vx + b->tx ;
		P2->vy = v2->vy + b->ty ;
		P2->vz = v2->vz + b->tz ;
		P2->vw = v2->vw ;	
		if ( !CheckSegmentConflict() ) continue ;
		len = SegmentDistance() ;
		axis = CheckSegmentCross( len ) ;
		if ( axis == 0 ) continue ;
		DiagonalSegmentHeight( axis ) ;
		cross = CROSS->vy - P1->vy ;
		if ( cross < 0.0F || cross > P1->vw ) continue ;
		if ( len > *MINLEN ) continue ;
		/* 当たった */
		*HIT = *HIT + 1 ;
		*MINLEN = len ;
		*MINCROSS = *CROSS ;
		*WHICH = 1 ;
		Where[ 0 ] = b->block_no ;
		Where[ 1 ] = n ;
		Where[ 2 ] = i ;
		
    }
}

/* １ブロックの壁をチェック */
static	void	CheckOneBlockSeg( b, seg, n_segs, seg_flag )
HZD_BLOCK	*b ;
HZD_VuSEG	*seg ;
int		n_segs ;
int		seg_flag ;
{
    int		i ;

    for ( i = 0; i < n_segs; i ++ ) {
		/* フラグチェック */
		if ( seg->atr & seg_flag ) goto online_check_skip_strip ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx + b->tx ) > BMAX->vx ||
			( float )( seg->b1.vy + b->ty ) > BMAX->vy ||
			( float )( seg->b1.vz + b->tz ) > BMAX->vz ||
			( float )( seg->b2.vx + b->tx ) < BMIN->vx ||
			( float )( seg->b2.vy + b->ty ) < BMIN->vy ||
			( float )( seg->b2.vz + b->tz ) < BMIN->vz ) {
			goto online_check_skip_strip ;
		}
		CheckOneSegmentStrip( b, seg, i ) ;
		online_check_skip_strip :
		seg ++ ;
    }    
}

/*----------------------------------------------------------------------------*/

/* 床チェック */
static	inline	void	SVtoFV_Trans( b, sv, fv )
HZD_BLOCK	*b ;
SVECTOR		*sv ;
FVECTOR		*fv ;
{
    fv->vx = ( float )sv->vx + ( float )b->tx ;
    fv->vy = ( float )sv->vy + ( float )b->ty ;
    fv->vz = ( float )sv->vz + ( float )b->tz ;
    fv->vw = ( float )sv->vw ;
}

static	inline	void	IVtoFV( iv, fv )
IVECTOR		*iv ;
FVECTOR		*fv ;
{
    fv->vx = iv->vx ;
    fv->vy = iv->vy ;
    fv->vz = iv->vz ;
    fv->vw = iv->vw ;
}

/* 交点までの距離を計算 */
static	int	FloorDistance( type )
int		type ;
{
    float	depth, x, y, z ;
    float	fa_n, ft_n, tmp ;

    if ( type & HZX_FLOOR_FLAT ) { /* FLAT_FLOOR */
		if ( FROM->vy == TO->vy ) return -1 ;
		depth = ( V1->vy - FROM->vy ) / ( TO->vy - FROM->vy ) ;
		if ( depth < 0.0F || depth > 1.0F ) return -1 ;
		CROSS->vx = FROM->vx + ( TO->vx - FROM->vx ) * depth ;
		CROSS->vz = FROM->vz + ( TO->vz - FROM->vz ) * depth ;
		CROSS->vy = V1->vy ;
    } else {					/* SLOPE_FLOOR */
		FROM_P1->vx = V1->vx - FROM->vx ;
		FROM_P1->vy = V1->vy - FROM->vy ;
		FROM_P1->vz = V1->vz - FROM->vz ;
		/* 法線 */
		NORMAL->vx = V1->vw ;
		NORMAL->vy = V2->vw ;
		NORMAL->vz = V3->vw ;
		ft_n = DIFF->vx * NORMAL->vx + DIFF->vy * NORMAL->vy + DIFF->vz * NORMAL->vz ;
		fa_n = FROM_P1->vx * NORMAL->vx + FROM_P1->vy * NORMAL->vy + FROM_P1->vz * NORMAL->vz ;
		tmp = fa_n * ft_n ;
		if ( tmp <= 0.0F ) return -1 ;
		fa_n = fa_n / ft_n ;
		//	if ( fa_n <= 0.0F ) return -1 ;
		CROSS->vx = FROM->vx + DIFF->vx * fa_n ;
		CROSS->vy = FROM->vy + DIFF->vy * fa_n ;
		CROSS->vz = FROM->vz + DIFF->vz * fa_n ;
    }
    x = CROSS->vx - FROM->vx ; if ( x < 0.0F ) x = -x ; 
    y = CROSS->vy - FROM->vy ; if ( y < 0.0F ) y = -y ;
    z = CROSS->vz - FROM->vz ; if ( z < 0.0F ) z = -z ;
    *LEN = x + y + z ;
    return 1 ;
}

/* 交点が床に含まれるかチェック */
static	int	CheckInsideFloor( type, n_v )
int		type, n_v ;
{
    float	opz, ip ;
    FVECTOR	p1, p2, p3, p4, min, max, op1, op2 ;

	if ( V1->vx < V3->vx ) {
		min.vx = V1->vx ; max.vx = V3->vx ;
	} else {
		min.vx = V3->vx ; max.vx = V1->vx ;
	}
	if ( V1->vz < V3->vz ) {
		min.vz = V1->vz ; max.vz = V3->vz ;
	} else {
		min.vz = V3->vz ; max.vz = V1->vz ;
	}
	if ( CROSS->vx < min.vx || CROSS->vx > max.vx ||
		CROSS->vz < min.vz || CROSS->vz > max.vz ) return -1 ;

	if ( type & HZX_FLOOR_RECT ) return 1 ;

    p1.vx = V2->vx - V1->vx ;
    p1.vy = V2->vy - V1->vy ;
    p1.vz = V2->vz - V1->vz ;
    p2.vx = CROSS->vx - V1->vx ;
    p2.vy = CROSS->vy - V1->vy ;
    p2.vz = CROSS->vz - V1->vz ;
	p3.vx = V3->vx - V1->vx ;
    p3.vy = V3->vy - V1->vy ;
    p3.vz = V3->vz - V1->vz ;
	op1.vx = p1.vy * p2.vz - p2.vy - p1.vz ;
	op1.vy = p1.vz * p2.vx - p2.vz - p1.vx ;
	op1.vz = p1.vx * p2.vy - p2.vx - p1.vy ;
	op2.vx = p1.vy * p3.vz - p3.vy - p1.vz ;
	op2.vy = p1.vz * p3.vx - p3.vz - p1.vx ;
	op2.vz = p1.vx * p3.vy - p3.vx - p1.vy ;
	ip = op1.vx * op2.vx + op1.vy * op2.vy + op1.vz * op2.vz ;
	if ( ip < 0.0F ) return -1 ;

    p1.vx = V3->vx - V2->vx ;
    p1.vy = V3->vy - V2->vy ;
    p1.vz = V3->vz - V2->vz ;
    p2.vx = CROSS->vx - V2->vx ;
    p2.vy = CROSS->vy - V2->vy ;
    p2.vz = CROSS->vz - V2->vz ;
	if ( n_v == 3 ) {
		p3.vx = V1->vx - V2->vx ;
		p3.vy = V1->vy - V2->vy ;
		p3.vz = V1->vz - V2->vz ;
	} else {
		p3.vx = V4->vx - V2->vx ;
		p3.vy = V4->vy - V2->vy ;
		p3.vz = V4->vz - V2->vz ;
	}
	op1.vx = p1.vy * p2.vz - p2.vy - p1.vz ;
	op1.vy = p1.vz * p2.vx - p2.vz - p1.vx ;
	op1.vz = p1.vx * p2.vy - p2.vx - p1.vy ;
	op2.vx = p1.vy * p3.vz - p3.vy - p1.vz ;
	op2.vy = p1.vz * p3.vx - p3.vz - p1.vx ;
	op2.vz = p1.vx * p3.vy - p3.vx - p1.vy ;
	ip = op1.vx * op2.vx + op1.vy * op2.vy + op1.vz * op2.vz ;
	if ( ip < 0.0F ) return -1 ;

	if ( n_v == 3 ) {
		p1.vx = V1->vx - V3->vx ;
		p1.vy = V1->vy - V3->vy ;
		p1.vz = V1->vz - V3->vz ;
	} else {
		p1.vx = V4->vx - V3->vx ;
		p1.vy = V4->vy - V3->vy ;
		p1.vz = V4->vz - V3->vz ;
	}
    p2.vx = CROSS->vx - V2->vx ;
    p2.vy = CROSS->vy - V2->vy ;
    p2.vz = CROSS->vz - V2->vz ;
	if ( n_v == 3 ) {
		p3.vx = V2->vx - V3->vx ;
		p3.vy = V2->vy - V3->vy ;
		p3.vz = V2->vz - V3->vz ;
	} else {
		p3.vx = V1->vx - V3->vx ;
		p3.vy = V1->vy - V3->vy ;
		p3.vz = V1->vz - V3->vz ;
	}
	op1.vx = p1.vy * p2.vz - p2.vy - p1.vz ;
	op1.vy = p1.vz * p2.vx - p2.vz - p1.vx ;
	op1.vz = p1.vx * p2.vy - p2.vx - p1.vy ;
	op2.vx = p1.vy * p3.vz - p3.vy - p1.vz ;
	op2.vy = p1.vz * p3.vx - p3.vz - p1.vx ;
	op2.vz = p1.vx * p3.vy - p3.vx - p1.vy ;
	ip = op1.vx * op2.vx + op1.vy * op2.vy + op1.vz * op2.vz ;
	if ( ip < 0.0F ) return -1 ;

	if ( n_v == 3 ) return -1 ;

    p1.vx = V1->vx - V4->vx ;
    p1.vy = V1->vy - V4->vy ;
    p1.vz = V1->vz - V4->vz ;
    p2.vx = CROSS->vx - V4->vx ;
    p2.vy = CROSS->vy - V4->vy ;
    p2.vz = CROSS->vz - V4->vz ;
	p3.vx = V2->vx - V4->vx ;
    p3.vy = V2->vy - V4->vy ;
    p3.vz = V2->vz - V4->vz ;
	op1.vx = p1.vy * p2.vz - p2.vy - p1.vz ;
	op1.vy = p1.vz * p2.vx - p2.vz - p1.vx ;
	op1.vz = p1.vx * p2.vy - p2.vx - p1.vy ;
	op2.vx = p1.vy * p3.vz - p3.vy - p1.vz ;
	op2.vy = p1.vz * p3.vx - p3.vz - p1.vx ;
	op2.vz = p1.vx * p3.vy - p3.vx - p1.vy ;
	ip = op1.vx * op2.vx + op1.vy * op2.vy + op1.vz * op2.vz ;
	if ( ip < 0.0F ) return -1 ;	
	
#if 0
    p1.vx = CROSS->vx - V1->vx ;
    p1.vz = CROSS->vz - V1->vz ;
    p2.vx = V2->vx - V1->vx ;
    p2.vz = V2->vz - V1->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;
    p1.vx = CROSS->vx - V2->vx ;
    p1.vz = CROSS->vz - V2->vz ;
    p2.vx = V3->vx - V2->vx ;
    p2.vz = V3->vz - V2->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;
    if ( n_v == 3 ) {
		p1.vx = CROSS->vx - V3->vx ;
		p1.vz = CROSS->vz - V3->vz ;
		p2.vx = V1->vx - V3->vx ;
		p2.vz = V1->vz - V3->vz ;
		opz = p1.vz * p2.vx - p1.vx * p2.vz ;
		if ( opz < 0.0F ) return -1 ;
		return 1 ;
    }
    p1.vx = CROSS->vx - V3->vx ;
    p1.vz = CROSS->vz - V3->vz ;
    p2.vx = V4->vx - V3->vx ;
    p2.vz = V4->vz - V3->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;    
    p1.vx = CROSS->vx - V4->vx ;
    p1.vz = CROSS->vz - V4->vz ;
    p2.vx = V1->vx - V4->vx ;
    p2.vz = V1->vz - V4->vz ;
    opz = p1.vz * p2.vx - p1.vx * p2.vz ;
    if ( opz < 0.0F ) return -1 ;    
#endif
    return 1 ;
}


/* １ストリップチェック */
static	void	CheckOneFloorStrip( b, seg, n ) 
HZD_BLOCK	*b ;
HZD_VuSEG	*seg ;
int		n ;
{
    int		n_points, n_v, i ;
    int		type ;
    SVECTOR	*verts ;

    n_points = seg->b1.vw ;
    n_v = seg->b2.vw ;
    verts = seg->verts ;
    type = seg->atr ;
    for ( i = 0; i < n_points; i += n_v ) {
		SVtoFV_Trans( b, verts, V1 ) ;
		SVtoFV_Trans( b, verts + 1, V2 ) ;	
		SVtoFV_Trans( b, verts + 2, V3 ) ;
		SVtoFV_Trans( b, verts + 3, V4 ) ;
		if ( FloorDistance( type ) < 0 ) goto check_one_flr_skip ;
		if ( *LEN > *MINLEN ) goto check_one_flr_skip ;
		if ( CheckInsideFloor( type, n_v ) < 0 ) goto check_one_flr_skip ;
		*WHICH = 2 ;
		*MINCROSS = *CROSS ;
		*MINLEN = *LEN ;
		*HIT = *HIT + 1 ;
		Where[ 0 ] = b->block_no ;
		Where[ 1 ] = n ;
		Where[ 2 ] = i ;
check_one_flr_skip :
		verts += n_v ;
    }
}

/* １ブロックの床をチェック */
static	void	CheckOneBlockFlr( b, seg, n_segs, flag )
HZD_BLOCK	*b ;
HZD_VuSEG	*seg ;
int		n_segs ;
int		flag ;
{
    int		i ;

    for ( i = 0; i < n_segs; i ++ ) {
		/* フラグチェック */
		if ( seg->atr & flag ) goto online_check_flr_skip_strip_cpu ;
		/* バウンディングチェック */
		if ( ( float )( seg->b1.vx + b->tx ) > BMAX->vx ||
			( float )( seg->b1.vy + b->ty ) > BMAX->vy ||
			( float )( seg->b1.vz + b->tz ) > BMAX->vz ||
			( float )( seg->b2.vx + b->tx ) < BMIN->vx ||
			( float )( seg->b2.vy + b->ty ) < BMIN->vy ||
			( float )( seg->b2.vz + b->tz ) < BMIN->vz ) {
			goto online_check_flr_skip_strip_cpu ;
		}
		CheckOneFloorStrip( b, seg, i ) ;
		online_check_flr_skip_strip_cpu :
		seg ++ ;
    }
}

/*----------------------------------------------------------------------------*/

/* バウンディング作成 */
static	void	MakeBound( from, to )
FVECTOR		*from, *to ;
{
    if ( from->vx < to->vx ) {
		BMIN->vx = from->vx ; BMAX->vx = to->vx ;
    } else {
		BMAX->vx = from->vx ; BMIN->vx = to->vx ;
    }
    if ( from->vy < to->vy ) {
		BMIN->vy = from->vy ; BMAX->vy = to->vy ;
    } else {
		BMAX->vy = from->vy ; BMIN->vy = to->vy ;
    }
    if ( from->vz < to->vz ) {
		BMIN->vz = from->vz ; BMAX->vz = to->vz ;
    } else {
		BMAX->vz = from->vz ; BMIN->vz = to->vz ;
    }    
}

/* 始点から指定の点までのキョリを計算 */
static	float	DistanceTo( to )
FVECTOR		*to ;
{
    float		d, len ;

    len = to->vx - FROM->vx ;
    if ( len < 0.0F ) len = - len ;
    d = to->vy - FROM->vy ;
    if ( d < 0.0F ) d = - d ;
    len += d ;
    d = to->vz - FROM->vz ;
    if ( d < 0.0F ) d = - d ;
    len += d ;
    return len ;
}

/* ブロック進入チェック */
static	void	CheckBlockBound( grp ) 
HZD_GRP		*grp ;
{
    int		i, j, k, block, tmp ;
    int		fx, fy, fz, tx, ty, tz ;
    int		dx, dy, dz, n_blocks ;
    float	cx, cy, cz, sx, sy, sz ;
    float	bx, by, bz, mx, my, mz ;
    int		x1, x2, y1, y2, z1, z2 ;
    HZD_BLOCK	*b, *blk ;
    FVECTOR	from, to ;

    bx = ( float )grp->block_size_x ;
    by = ( float )grp->block_size_y ;
    bz = ( float )grp->block_size_z ;

    mx = ( float )grp->bound_min_x ;
    my = ( float )grp->bound_min_y ;
    mz = ( float )grp->bound_min_z ;

    dx = grp->div_x ;
    dy = grp->div_y ;
    dz = grp->div_z ;

    N_InsideBlocks = 0 ;

    n_blocks = grp->n_blocks ;
    b = grp->blocks ;

    from = *FROM ; to = *TO ;
    fx = x1 = ( int )( ( ( &from )->vx - mx ) / bx ) ;
    tx = x2 = ( int )( ( ( &to )->vx - mx ) / bx ) ;
    fz = z1 = ( int )( ( ( &from )->vz - mz ) / bz ) ;
    tz = z2 = ( int )( ( ( &to )->vz - mz ) / bz ) ;
    fy = y1 = ( int )( ( ( &from )->vy - my ) / by ) ;    
    ty = y2 = ( int )( ( ( &to )->vy - my ) / by ) ;

    if ( x1 > x2 ) { tmp = x1 ; x1 = x2 ; x2 = tmp ; }
    if ( z1 > z2 ) { tmp = z1 ; z1 = z2 ; z2 = tmp ; }
    if ( y1 > y2 ) { tmp = y1 ; y1 = y2 ; y2 = tmp ; }	

    for ( k = y1; k <= y2; k ++ ) {
		for ( j = z1; j <= z2; j ++ ) {
			for ( i = x1; i <= x2; i ++ ) {
				if ( i < 0 || j < 0 || k < 0 ||
					i >= dx || j >= dz || k >= dy ) continue ;
				block = dx * j + i + ( k * dx * dz ) ;
				if ( block < 0 || block >= n_blocks ) continue ;
				blk = b + block ;
				if ( blk->n_segs == 0 &&
					blk->n_bul_segs == 0 &&
					blk->n_flrs == 0 &&
					blk->n_bul_flrs == 0 ) continue ;
				InsideBlock[ N_InsideBlocks ] = block ;
				N_InsideBlocks ++ ;
			}
		}
    }
}

/* オンラインチェック */
int	HZD_OnlineHazardCheck( grp, from, to, chk_flag, seg_flag, flr_flag )
HZD_GRP		*grp ;
FVECTOR		*from, *to ;
int		chk_flag, seg_flag, flr_flag ;
{
    HZD_BLOCK	*blk, *b ;
    int		i, hit, n ;
    
    if ( grp->n_blocks == 0 ) return 0 ;

    *FROM = *from ;
    *TO = *to ;
    *WHICH = 0 ;
    *HIT = 0 ;
    *MINCROSS = *TO ;
    blk = grp->blocks ;

    /* バウンディング作成 */
    MakeBound( FROM, TO ) ;

    /* 検索直線ベクトル */
    DIFF->vx = TO->vx - FROM->vx ;
    DIFF->vy = TO->vy - FROM->vy ;
    DIFF->vz = TO->vz - FROM->vz ;    

    /* バウンドチェック */
    CheckBlockBound( grp ) ;

    /* 壁チェック */
    MakeStepXZ() ;
    *MINLEN = sqrtf( DIFF->vx * DIFF->vx +
					DIFF->vy * DIFF->vy +
					DIFF->vz * DIFF->vz ) ;
    for ( i = 0; i < N_InsideBlocks; i ++ ) {
		b = blk + InsideBlock[ i ] ;
		if ( b->n_segs > 0 ) 
			CheckOneBlockSeg( b, b->segs, b->n_segs, seg_flag ) ;
		if ( seg_flag & HZX_SEG_RECOIL_TYPE ) {
			if ( b->n_bul_segs > 0 ) {
				CheckOneBlockSeg( b, b->bul_segs, b->n_bul_segs, seg_flag ) ;
			}
		}		
    }

    /* 床チェック */
    MakeBound( FROM, MINCROSS ) ;
    *MINLEN = DistanceTo( MINCROSS ) ;
    for ( i = 0; i < N_InsideBlocks; i ++ ) {
		b = blk + InsideBlock[ i ] ;
		if ( b->n_flrs > 0 ) CheckOneBlockFlr( b, b->flrs, b->n_flrs, flr_flag ) ;
		if ( flr_flag & HZX_FLOOR_RECOILE_TYPE ) {
			if ( b->n_bul_flrs > 0 ) CheckOneBlockFlr( b, b->bul_flrs, b->n_bul_flrs, flr_flag ) ;
		}
    }
    return *HIT ;
}

/*----------------------------------------------------------------------------*/

void	HZD_GetOnlinePoint( v )
FVECTOR	*v ;
{
    v->vx = MINCROSS->vx ;
    v->vy = MINCROSS->vy ;
    v->vz = MINCROSS->vz ;
}

