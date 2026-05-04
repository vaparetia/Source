/*
   dynamic.c
   動的ハザード
   
   1999/10/08 M.Sonoyama
   $Id: dynamic.c,v 1.1.1.3 2002/11/19 11:42:47 Yoshizawa1 Exp $
   */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <devgif.h>

#include	"gameheader.h"

/* 動的ハザードコールバックを登録 */
HZX_D_CALLBACK		*HZX_SetDynamicCallback( hzd, cb, func, work1, work2 )
void				*hzd ;
HZX_D_CALLBACK		*cb ;
HZX_CALLBACK_FUNC	func ;
void				*work1, *work2 ;
{
	HZX_D_SEGMENT		*seg ;
	HZX_D_FLOOR			*flr ;
	HZX_D_CALLBACK 		*pre ;
	
	cb->callback = func ;
	cb->work1 = work1 ;
	cb->work2 = work2 ;

	if ( hzd == NULL ) return cb ;
	seg = ( HZX_D_SEGMENT * )hzd ;
	if ( seg->tag[ 0 ] != 2 ) {	/* 床 */
		flr = ( HZX_D_FLOOR * )seg ;
		pre = flr->callback ;
		flr->callback = cb ;
		cb->next = pre ;
	} else {					/* 壁 */
		pre = seg->callback ;		
		seg->callback = cb ;
		cb->next = pre ;
	}
	return cb ;
}

/* コールバックを削除 */
void		HZX_RemoveDynamicCallback( hzd, cb )
void				*hzd ;
HZX_D_CALLBACK		*cb ;
{
	HZX_D_SEGMENT		*seg ;
	HZX_D_FLOOR			*flr ;	
	HZX_D_CALLBACK		*list, *pre ;
	int					type ;

	if ( hzd == NULL ) return ;
	seg = ( HZX_D_SEGMENT * )hzd ;
	if ( seg->tag[ 0 ] != 2 ) {	/* 床 */
		flr = ( HZX_D_FLOOR * )seg ;
		list = flr->callback ;
		type = HZX_TYPE_FLOOR ;
	} else {					/* 壁 */
		flr = NULL ;
		list = seg->callback ;
		type = HZX_TYPE_SEGMENT ;
	}
	pre = NULL ;
	while( list != NULL ) {
		if ( list == cb ) {
			if ( pre == NULL ) {
				if ( type == HZX_TYPE_SEGMENT ) seg->callback = list->next ;
				else			 				flr->callback = list->next ;
				break ;
			} else {
				pre->next = list->next ;
				break ;
			}
		}
		pre = list ;
		list = list->next ;
	} 
}

/* コールバックを実行 */
static	void	HZX_ExecCallback( hzd, list, flag )
void			*hzd ;
HZX_D_CALLBACK	*list ;
int				flag ;
{
	void	( *func )( void *, void *, void *, int ) ;

	while( list != NULL ) {
		func = list->callback ;
		( *func )( hzd, list->work1, list->work2, flag ) ;
		list = list->next ;
	}
}

/* ある壁のコールバックを実行 */
void	HZX_DynamicSegmentExecCallback( HZX_D_SEGMENT *seg, int flag )
{
	HZX_ExecCallback( seg, seg->callback, HZX_TYPE_SEGMENT | flag ) ;
}

/* ある床のコールバックを実行 */
void	HZX_DynamicFloorExecCallback( HZX_D_FLOOR *seg, int flag )
{
	HZX_ExecCallback( seg, seg->callback, HZX_TYPE_FLOOR | flag ) ;
}

/*-----------------------------------------------------------------*/

static	ALIGN16_PRE int	SegmentTag[] ALIGN16_POST = {
    2, -1, 0, 0
} ;

static	ALIGN16_PRE int	FloorTag[] ALIGN16_POST = {
    4, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
} ;

static	inline	float	MaxFloat( float a, float b )
{
    return ( a > b ) ? a : b ;
}

static	inline	float	MinFloat( float a, float b )
{
    return ( a < b ) ? a : b ;
}

static	inline	int	MaxInt( a, b )
int		a, b ;
{
    return ( a > b ) ? a : b ;
}

static	inline	int	MinInt( a, b )
int		a, b ;
{
    return ( a < b ) ? a : b ;
}

static	void	AddIVec( v3, v1, v2 )
IVECTOR			*v3, *v1, *v2 ;
{
	v3->vx = v1->vx + v2->vx ;
	v3->vy = v1->vy + v2->vy ;
	v3->vz = v1->vz + v2->vz ;
	v3->vw = v1->vw ;
}

/* 現在の位置で座標固定化 */
void		HZX_FixDynamicSegment( HZX_D_SEGMENT *seg )
{
    seg->def[ 0 ] = seg->p1 ;
    seg->def[ 1 ] = seg->p2 ;
	DG_COPY_MAT( &seg->world, &DG_UnitMatrix ) ;
	DG_COPY_MAT( &seg->world2, &DG_UnitMatrix ) ;
}

/* 動的壁移動 */
void		HZX_MoveDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *p1, IVECTOR *p2 )
{
	FVECTOR			diff ;

	if ( seg == NULL ) return ;

    if ( p1->vx == p2->vx ) {
		if ( p1->vz < p2->vz ) {
			seg->p1 = *p1 ;
			seg->p2 = *p2 ;
		} else {
			seg->p1 = *p2 ;
			seg->p2 = *p1 ;
		}
    } else {
		if ( p1->vx < p2->vx ) {
			seg->p1 = *p1 ;
			seg->p2 = *p2 ;
		} else {
			seg->p1 = *p2 ;
			seg->p2 = *p1 ;
		}	
    }    

    seg->b1.vx = MinInt( p1->vx, p2->vx ) ;
    seg->b1.vy = MinInt( p1->vy, p2->vy ) ;
    seg->b1.vz = MinInt( p1->vz, p2->vz ) ;
    seg->b2.vx = MaxInt( p1->vx, p2->vx ) ;
    seg->b2.vy = MaxInt( p1->vy + p1->vw, p2->vy + p2->vw ) ;
    seg->b2.vz = MaxInt( p1->vz, p2->vz ) ;

	seg->world2.m[ 3 ][ 0 ] = seg->world.m[ 3 ][ 0 ] ;
	seg->world2.m[ 3 ][ 1 ] = seg->world.m[ 3 ][ 1 ] ;
	seg->world2.m[ 3 ][ 2 ] = seg->world.m[ 3 ][ 2 ] ;
	diff.vx = ( float )( p1->vx - seg->def[ 0 ].vx ) ;
	diff.vy = ( float )( p1->vy - seg->def[ 0 ].vy ) ;
	diff.vz = ( float )( p1->vz - seg->def[ 0 ].vz ) ;
	GV_VecToMat( &diff, &seg->world ) ;

	*( HZX_D_SEGMENT * )GV_UNCACHE( seg ) = *seg ;
	HZX_ExecCallback( seg, seg->callback, HZX_TYPE_SEGMENT ) ;
}

/* 動的壁回転 */
/* Ｙ軸回転のみ */
void 	HZX_RotateDynamicSegment( seg, axis, rot )
HZX_D_SEGMENT	*seg ;
IVECTOR		*axis ; /* 回転中心 */
SVECTOR		*rot ;
{
    SVECTOR	rot2 ;
    FVECTOR	p[ 2 ], pole ;
    IVECTOR	new[ 2 ] ;

	if ( seg == NULL ) return ;

    rot2 = DG_ZeroSVector ; rot2.vy = rot->vy ;
    GV_IVtoFV( axis, &pole, 3 ) ;
    GV_IVtoFV( seg->def, &p[ 0 ], 8 ) ;
    _sceVu0SubVector( &p[ 0 ], &p[ 0 ], &pole ) ;
    _sceVu0SubVector( &p[ 1 ], &p[ 1 ], &pole ) ;

    DG_SetPos2( &pole, &rot2 ) ;
    DG_PutVector( p, p, 2 ) ;
    GV_FVtoIV( &p[ 0 ], &new[ 0 ], 7 ) ;
    new[ 0 ].vw = seg->def[ 0 ].vw ;
    new[ 1 ].vw = seg->def[ 1 ].vw ;

	DG_COPY_MAT( &seg->world2, &seg->world ) ;
	GV_MatToVec( &seg->world, &pole ) ;
	DG_GetPos( &seg->world ) ;	
	GV_VecToMat( &pole, &seg->world ) ;	

    HZX_MoveDynamicSegment( seg, &new[ 0 ], &new[ 1 ] ) ;
}

void 	HZX_RotateDynamicSegment2( seg, axis, mat )
HZX_D_SEGMENT	*seg ;
IVECTOR		*axis ; /* 回転中心 */
FMATRIX		*mat ;
{
    FVECTOR		p[ 2 ], pole ;
    IVECTOR		new[ 2 ] ;
	FMATRIX		m ;

	if ( seg == NULL ) return ;

	if ( axis == NULL ) {
		pole.vx = ( int )mat->m[ 3 ][ 0 ] ;
		pole.vy = ( int )mat->m[ 3 ][ 1 ] ;
		pole.vz = ( int )mat->m[ 3 ][ 2 ] ;
	} else {
		GV_IVtoFV( axis, &pole, 3 ) ;
	}
    GV_IVtoFV( seg->def, &p[ 0 ], 8 ) ;
    _sceVu0SubVector( &p[ 0 ], &p[ 0 ], &pole ) ;
    _sceVu0SubVector( &p[ 1 ], &p[ 1 ], &pole ) ;

	DG_COPY_MAT( &m, mat ) ;
	GV_VecToMat( &pole, &m ) ;
    DG_SetPos( &m ) ;
    DG_PutVector( p, p, 2 ) ;
    GV_FVtoIV( &p[ 0 ], &new[ 0 ], 7 ) ;
    new[ 0 ].vw = seg->def[ 0 ].vw ;
    new[ 1 ].vw = seg->def[ 1 ].vw ;

	DG_COPY_MAT( &seg->world2, &seg->world ) ;
	GV_MatToVec( &seg->world, &pole ) ;
	DG_GetPos( &seg->world ) ;	
	GV_VecToMat( &pole, &seg->world ) ;	

    HZX_MoveDynamicSegment( seg, &new[ 0 ], &new[ 1 ] ) ;
}

/* 動的壁シフト */
void	HZX_ShiftDynamicSegment( seg, shift )
HZX_D_SEGMENT		*seg ;
IVECTOR				*shift ;
{
	IVECTOR			v[ 2 ] ;

	if ( seg == NULL ) return ;

	AddIVec( &v[ 0 ], &seg->p1, shift ) ;
	AddIVec( &v[ 1 ], &seg->p2, shift ) ;
    HZX_MoveDynamicSegment( seg, &v[ 0 ], &v[ 1 ] ) ;	
}

/* 動的壁登録 */
HZX_D_SEGMENT	*HZX_AddDynamicSegment( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2, u_int atr )
{
    HZX_HDL		*hzd ;
    HZX_GRP		*grp ;
    HZX_D_SEGMENT	*seg ;
    int			group ;

    hzd = HZX_GetCurrentHzx();
    group = HZX_GetGroupNo( id );
    grp = hzd->grp + group ;
    seg = GV_Malloc( sizeof( HZX_D_SEGMENT ) ) ;
    if ( seg == NULL ) {
		printf( "warning : cannot create new dynamic segment\n" ) ;
		return NULL ;
    }

    seg->def[ 0 ] = *p1 ;
    seg->def[ 1 ] = *p2 ;
	seg->callback = NULL ;
	seg->target = NULL ;
	DG_COPY_MAT( &seg->world, &DG_UnitMatrix ) ;
	DG_COPY_MAT( &seg->world2, &DG_UnitMatrix ) ;

    seg->atr = atr | HZX_SEG_DYNAMIC | HZX_SEG_NO_DISP_RADAR ; /* デフォルトではレーダーに映らない */
    memcpy( seg->tag, SegmentTag, sizeof( int ) * 4 ) ;
    seg->tag[ 2 ] = grp->dynamics->n_segs ;
    seg->tag[ 3 ] = group ;
    seg->next = grp->dynamics->segs ;

    HZX_MoveDynamicSegment( seg, p1, p2 ) ;

    grp->dynamics->segs = seg ;
    grp->dynamics->n_segs ++ ;

    return seg ;
}

/* 動的壁削除 */
void HZX_RemoveDynamicSegment( HZX_D_SEGMENT *seg ) 
{
    HZX_HDL		*hzd ;
    HZX_DYNAMICS	*dym ;
    HZX_D_SEGMENT	*segs, *pre ;
    int			group, i ;

	if ( seg == NULL ) return ;

	/* 破壊をコールバックにて通知 */
	HZX_ExecCallback( seg, seg->callback, HZX_TYPE_SEGMENT | HZX_CALLBACK_FLAG_DESTROY ) ;

    hzd = HZX_GetCurrentHzx() ;
    group = seg->tag[ 3 ] ;
    dym = ( hzd->grp + group )->dynamics ;
    segs = pre = dym->segs ;
    if ( segs == seg ) {
		dym->segs = seg->next ;
		dym->n_segs -- ;
		GV_DelayedFree( seg ) ;
		goto dseg_renumber_proc ;
    }
    while( segs != NULL ) {
		if ( segs == seg ) {
			pre->next = seg->next ;
			dym->n_segs -- ;
			GV_DelayedFree( seg ) ;
			break ;
		}
		pre = segs ;
		segs = segs->next ;
    }
dseg_renumber_proc :
	/* ナンバーの振り直し */
	segs = dym->segs ;
	for ( i = dym->n_segs - 1; i >= 0; i -- ) {
		segs->tag[ 2 ] = i ;
		segs = segs->next ;
	}
}

/* 長方形チェック */
static	int		CheckRect( v1, v2, v3, v4 )
IVECTOR			*v1, *v2, *v3, *v4 ;
{
    if ( v1->vx == v2->vx && v3->vx == v4->vx &&
		v1->vz == v4->vz && v2->vz == v3->vz ) return HZX_FLOOR_RECT ;
    if ( v1->vz == v2->vz && v3->vz == v4->vz &&
		v1->vx == v4->vx && v2->vx == v3->vx ) return HZX_FLOOR_RECT ;
    return 0 ;
}

/* フラットチェック */
static	int		CheckFlat3( v1, v2, v3 )
IVECTOR			*v1, *v2, *v3 ;
{
    if ( v1->vy == v2->vy &&
		v2->vy == v3->vy ) return HZX_FLOOR_FLAT ;
    return 0 ;
}

static	int		CheckFlat4( v1, v2, v3, v4 )
IVECTOR			*v1, *v2, *v3, *v4 ;
{
    if ( v1->vy == v2->vy &&
		v2->vy == v3->vy && 
		v3->vy == v4->vy ) return HZX_FLOOR_FLAT ;
    return 0 ;
}

/* 法線長さ32000 */
static	void 	GetNormal( res, v1, v2, v3, v4, n )
IVECTOR		*res ;
IVECTOR		*v1, *v2, *v3, *v4 ;
int		n ;
{
    FVECTOR	normal ;
    IVECTOR	tmp ;
    float	len ;

    normal.vx = ( v3->vy - v2->vy ) * ( v2->vz - v1->vz ) -
		( v2->vy - v1->vy ) * ( v3->vz - v2->vz ) ;
    normal.vy = ( v3->vz - v2->vz ) * ( v2->vx - v1->vx ) -
		( v2->vz - v1->vz ) * ( v3->vx - v2->vx ) ;
    normal.vz = ( v3->vx - v2->vx ) * ( v2->vy - v1->vy ) -
		( v2->vx - v1->vx ) * ( v3->vy - v2->vy ) ;
    if ( normal.vy < 0.0F ) {
		normal.vx *= -1.0F ;
		normal.vy *= -1.0F ;
		normal.vz *= -1.0F ;
		/* 反転 */
		if ( n == 3 ) {
			tmp = *v1 ; *v1 = *v3 ; *v3 = tmp ;
		} else {
			tmp = *v1 ; *v1 = *v4 ; *v4 = tmp ;
			tmp = *v2 ; *v2 = *v3 ; *v3 = tmp ;
		}
    }
    len = bp_sqrtf( normal.vx * normal.vx +  //BP_MATH - emulate PS2 sqrtf
				normal.vy * normal.vy +
				normal.vz * normal.vz ) ;
    res->vx = ( int )( normal.vx * 32000.0F / len ) ;
    res->vy = ( int )( normal.vy * 32000.0F / len ) ;
    res->vz = ( int )( normal.vz * 32000.0F / len ) ;
}

/* 現在の位置で座標固定化 */
void		HZX_FixDynamicFloor( HZX_D_FLOOR *seg )
{
	if ( seg == NULL ) return ;
    seg->def[ 0 ] = seg->p1 ;
    seg->def[ 1 ] = seg->p2 ;
    seg->def[ 2 ] = seg->p3 ;
    seg->def[ 3 ] = seg->p4 ;
	DG_COPY_MAT( &seg->world, &DG_UnitMatrix ) ;
	DG_COPY_MAT( &seg->world2, &DG_UnitMatrix ) ;
}

/* 動的床移動 */
void		HZX_MoveDynamicFloor( HZX_D_FLOOR *flr, IVECTOR *p1, IVECTOR *p2,
								 IVECTOR *p3, IVECTOR *p4 ) 
{
    int		n ;
    IVECTOR	normal, p1buf ;
	FVECTOR	diff ;

	if ( flr == NULL ) return ;
    n = flr->tag[ 0 ] ;

	p1buf.vx = p1->vx ;
	p1buf.vy = p1->vy ;
	p1buf.vz = p1->vz ;
    GetNormal( &normal, p1, p2, p3, p4, n ) ;

    flr->p1 = *p1 ;
    flr->p2 = *p2 ;
    flr->p3 = *p3 ;
    flr->p4 = *p4 ;
    flr->p1.vw = normal.vx ;
    flr->p2.vw = normal.vy ;
    flr->p3.vw = normal.vz ;
    flr->atr &= ~( HZX_FLOOR_RECT | HZX_FLOOR_FLAT ) ;
    if ( n == 4 ) {
		flr->b1.vx = MinInt( MinInt( p1->vx, p2->vx ), MinInt( p3->vx, p4->vx ) ) ;
		flr->b1.vy = MinInt( MinInt( p1->vy, p2->vy ), MinInt( p3->vy, p4->vy ) ) ;
		flr->b1.vz = MinInt( MinInt( p1->vz, p2->vz ), MinInt( p3->vz, p4->vz ) ) ;
		flr->b2.vx = MaxInt( MaxInt( p1->vx, p2->vx ), MaxInt( p3->vx, p4->vx ) ) ;
		flr->b2.vy = MaxInt( MaxInt( p1->vy, p2->vy ), MaxInt( p3->vy, p4->vy ) ) ;
		flr->b2.vz = MaxInt( MaxInt( p1->vz, p2->vz ), MaxInt( p3->vz, p4->vz ) ) ;
		flr->atr |= CheckRect( p1, p2, p3, p4 ) | CheckFlat4( p1, p2, p3, p4 ) ;
		flr->tag[ 5 ] = flr->atr & HZX_FLOOR_RECT ;
		flr->tag[ 6 ] = flr->atr & HZX_FLOOR_FLAT ;
    } else {
		flr->b1.vx = MinInt( MinInt( p1->vx, p2->vx ), p3->vx ) ;
		flr->b1.vy = MinInt( MinInt( p1->vy, p2->vy ), p3->vy ) ;
		flr->b1.vz = MinInt( MinInt( p1->vz, p2->vz ), p3->vz ) ;
		flr->b2.vx = MaxInt( MaxInt( p1->vx, p2->vx ), p3->vx ) ;
		flr->b2.vy = MaxInt( MaxInt( p1->vy, p2->vy ), p3->vy ) ;
		flr->b2.vz = MaxInt( MaxInt( p1->vz, p2->vz ), p3->vz ) ;
		flr->atr |= CheckFlat3( p1, p2, p3, p4 ) ;
		flr->tag[ 6 ] = flr->atr & HZX_FLOOR_FLAT ;
    }
    flr->tag[ 8 ] = flr->b1.vy ;
    flr->tag[ 9 ] = flr->b2.vy ;

	flr->world2.m[ 3 ][ 0 ] = flr->world.m[ 3 ][ 0 ] ;
	flr->world2.m[ 3 ][ 1 ] = flr->world.m[ 3 ][ 1 ] ;
	flr->world2.m[ 3 ][ 2 ] = flr->world.m[ 3 ][ 2 ] ;
	diff.vx = ( float )( p1buf.vx - flr->def[ 0 ].vx ) ;
	diff.vy = ( float )( p1buf.vy - flr->def[ 0 ].vy ) ;
	diff.vz = ( float )( p1buf.vz - flr->def[ 0 ].vz ) ;
	GV_VecToMat( &diff, &flr->world ) ;

	*( HZX_D_FLOOR * )GV_UNCACHE( flr ) = *flr ;
	HZX_ExecCallback( flr, flr->callback, HZX_TYPE_FLOOR ) ;
}

/* 動的床回転 */
void 	HZX_RotateDynamicFloor( seg, axis, rot )
HZX_D_FLOOR	*seg ;
IVECTOR		*axis ; /* 回転中心 */
SVECTOR		*rot ;
{
    FVECTOR	p[ 4 ], pole ;
    IVECTOR	new[ 4 ] ;

	if ( seg == NULL ) return ;
    GV_IVtoFV( axis, &pole, 3 ) ;
    GV_IVtoFV( &seg->def, &p[ 0 ], 16 ) ;
    _sceVu0SubVector( &p[ 0 ], &p[ 0 ], &pole ) ;
    _sceVu0SubVector( &p[ 1 ], &p[ 1 ], &pole ) ;
    _sceVu0SubVector( &p[ 2 ], &p[ 2 ], &pole ) ;
    _sceVu0SubVector( &p[ 3 ], &p[ 3 ], &pole ) ;
    DG_SetPos2( &pole, rot ) ;
    DG_PutVector( p, p, 4 ) ;
    GV_FVtoIV( &p[ 0 ], &new[ 0 ], 16 ) ;

	DG_COPY_MAT( &seg->world2, &seg->world ) ;
	GV_MatToVec( &seg->world, &pole ) ;
	DG_GetPos( &seg->world ) ;	
	GV_VecToMat( &pole, &seg->world ) ;	

    HZX_MoveDynamicFloor( seg, &new[ 0 ], &new[ 1 ], &new[ 2 ], &new[ 3 ] ) ;
}

/* 動的床回転２ */
void 	HZX_RotateDynamicFloor2( seg, axis, mat )
HZX_D_FLOOR	*seg ;
IVECTOR		*axis ; /* 回転中心 */
FMATRIX		*mat ;
{
	FMATRIX	m ;
    FVECTOR	p[ 4 ], pole ;
    IVECTOR	new[ 4 ] ;

	if ( seg == NULL ) return ;
	if ( axis == NULL ) {
		pole.vx = ( int )mat->m[ 3 ][ 0 ] ;
		pole.vy = ( int )mat->m[ 3 ][ 1 ] ;
		pole.vz = ( int )mat->m[ 3 ][ 2 ] ;
	} else {
		GV_IVtoFV( axis, &pole, 3 ) ;
	}
    GV_IVtoFV( &seg->def, &p[ 0 ], 16 ) ;
    _sceVu0SubVector( &p[ 0 ], &p[ 0 ], &pole ) ;
    _sceVu0SubVector( &p[ 1 ], &p[ 1 ], &pole ) ;
    _sceVu0SubVector( &p[ 2 ], &p[ 2 ], &pole ) ;
    _sceVu0SubVector( &p[ 3 ], &p[ 3 ], &pole ) ;
	DG_COPY_MAT( &m, mat ) ;
	GV_VecToMat( &pole, &m ) ;
    DG_SetPos( &m ) ;	
    DG_PutVector( p, p, 4 ) ;
    GV_FVtoIV( &p[ 0 ], &new[ 0 ], 16 ) ;

	DG_COPY_MAT( &seg->world2, &seg->world ) ;
	GV_MatToVec( &seg->world, &pole ) ;
	DG_GetPos( &seg->world ) ;	
	GV_VecToMat( &pole, &seg->world ) ;	

    HZX_MoveDynamicFloor( seg, &new[ 0 ], &new[ 1 ], &new[ 2 ], &new[ 3 ] ) ;
}

/* 動的床シフト */
void	HZX_ShiftDynamicFloor( seg, shift )
HZX_D_FLOOR		*seg ;
IVECTOR			*shift ;
{
	IVECTOR		v[ 4 ] ;

	if ( seg == NULL ) return ;
	AddIVec( &v[ 0 ], &seg->p1, shift ) ;
	AddIVec( &v[ 1 ], &seg->p2, shift ) ;
	AddIVec( &v[ 2 ], &seg->p3, shift ) ;
	AddIVec( &v[ 3 ], &seg->p4, shift ) ;
    HZX_MoveDynamicFloor( seg, &v[ 0 ], &v[ 1 ], &v[ 2 ], &v[ 3 ] ) ;
}

/* 動的床登録 */
HZX_D_FLOOR	*HZX_AddDynamicFloor( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2,
								 IVECTOR *p3, IVECTOR *p4, int n, u_int atr )
{
    HZX_D_FLOOR		*flr ;
    HZX_HDL		*hzd ;
    HZX_GRP		*grp ;
    int			group ;

    ASSERT( n == 3 || n == 4 ) ;
    hzd = HZX_GetCurrentHzx() ;
    group = HZX_GetGroupNo( id );

    grp = hzd->grp + group ;
    flr = GV_Malloc( sizeof( HZX_D_FLOOR ) ) ;
    if ( flr == NULL ) {
		printf( "warning : cannot create new dynamic floor\n" ) ;
		return NULL ;
    }

    flr->atr = atr | HZX_FLOOR_DYNAMIC ;
    memcpy( flr->tag, FloorTag, sizeof( int ) * 12 ) ;
    flr->tag[ 0 ] = n ;
    flr->tag[ 2 ] = grp->dynamics->n_flrs ;
    flr->tag[ 3 ] = group ;
    flr->tag[ 4 ] = n - 3 ;

    flr->def[ 0 ] = *p1 ;
    flr->def[ 1 ] = *p2 ;
    flr->def[ 2 ] = *p3 ;
    flr->def[ 3 ] = *p4 ;
	DG_COPY_MAT( &flr->world, &DG_UnitMatrix ) ;
	DG_COPY_MAT( &flr->world2, &DG_UnitMatrix ) ;
	flr->callback = NULL ;
	flr->target = NULL ;

    flr->next = grp->dynamics->flrs ;

    HZX_MoveDynamicFloor( flr, p1, p2, p3, p4 ) ;

    grp->dynamics->flrs = flr ;
    grp->dynamics->n_flrs ++ ;

    return flr ;
}

HZX_D_FLOOR	*HZX_AddDynamicFloorF( HZX_GROUP_ID id, FVECTOR *p, int n, u_int atr )
{
	IVECTOR		iv[ 4 ] ;

	GV_FVtoIV( p, iv, 16 ) ;
	return HZX_AddDynamicFloor( id, &iv[ 0 ], &iv[ 1 ], &iv[ 2 ], &iv[ 3 ], n, atr ) ;
}

/* 動的床削除 */
void		HZX_RemoveDynamicFloor( HZX_D_FLOOR *flr ) 
{
    HZX_HDL		*hzd ;
    HZX_DYNAMICS	*dym ;
    HZX_D_FLOOR		*flrs, *pre ;
    int			group, i ;

	if ( flr == NULL ) return ;
	/* 破壊をコールバックにて通知 */
	HZX_ExecCallback( flr, flr->callback, HZX_TYPE_FLOOR | HZX_CALLBACK_FLAG_DESTROY ) ;

    hzd = HZX_GetCurrentHzx() ;
    group = flr->tag[ 3 ] ;
    dym = ( hzd->grp + group )->dynamics ;
    flrs = pre = dym->flrs ;
    if ( flrs == flr ) {
		dym->flrs = flr->next ;
		dym->n_flrs -- ;
		GV_DelayedFree( flr ) ;
		goto dflr_renumber_proc ;
    }
    while( flrs != NULL ) {
		if ( flrs == flr ) {
			pre->next = flr->next ;
			dym->n_flrs -- ;
			GV_DelayedFree( flr ) ;
			break ;
		}
		pre = flrs ;
		flrs = flrs->next ;
    }
dflr_renumber_proc :
	flrs = dym->flrs ;
	for ( i = dym->n_flrs - 1; i >= 0; i -- ) {
		flrs->tag[ 2 ] = i ;
		flrs = flrs->next ;
	}
}

/*-----------------------------------------------------------------*/

/* ハザードがダイナミックかどうか */
int		HZX_IsDynamic( HZX_HZD *hzd )
{
	if ( ( hzd->type == HZX_TYPE_SEGMENT ) &&
		 ( hzd->attribute & HZX_SEG_DYNAMIC ) ) return 1 ;
	if ( ( hzd->type == HZX_TYPE_FLOOR ) &&
		 ( hzd->attribute & HZX_FLOOR_DYNAMIC ) ) return 1 ;
	return 0 ;
}

/*-----------------------------------------------------------------*/

/* コンビニエンスコールバック関数 */

/* 位置を更新 */
void	HZX_CallbackUpdatePos( hzd, old, new )
void		*hzd ;
FVECTOR		*old, *new ;
{
	HZX_D_SEGMENT		*seg ;
	HZX_D_FLOOR			*flr ;
	FMATRIX				*pre, *now, t ;
	FVECTOR				base, v1, v2 ;
	
	seg = ( HZX_D_SEGMENT * )hzd ;
	if ( seg->tag[ 0 ] != 2 ) {
		flr = ( HZX_D_FLOOR * )seg ;
		pre = &flr->world2 ;
		now = &flr->world ;
		GV_IVtoFV( &flr->def[ 0 ], &base, 3 ) ;
	} else {
		pre = &seg->world2 ;
		now = &seg->world ;
		GV_IVtoFV( &seg->def[ 0 ], &base, 3 ) ;
	}
	GV_MatToVec( pre, &v1 ) ;
	_sceVu0SubVector( &v2, old, &v1 ) ;
	_sceVu0SubVector( &v2, &v2, &base ) ;
	_sceVu0TransposeMatrix( &t, pre ) ;
	DG_SetPos( &t ) ;
	DG_RotVector( &v2, &v2, 1 ) ;
	DG_SetPos( now ) ;
	DG_RotVector( &v2, &v2, 1 ) ;
	_sceVu0AddVector( &v2, &v2, &base ) ;
	GV_MatToVec( now, &v1 ) ;
	_sceVu0AddVector( new, &v2, &v1 ) ;
}

void	HZX_CallbackUpdateMatrix( void *hzd, FMATRIX *old, FMATRIX *new )
{
	HZX_D_SEGMENT		*seg ;
	HZX_D_FLOOR			*flr ;
	FMATRIX				pre, now, this, t ;
	FVECTOR				old_pos, new_pos ;
	
	seg = ( HZX_D_SEGMENT * )hzd ;
	if ( seg->tag[ 0 ] != 2 ) {
		flr = ( HZX_D_FLOOR * )seg ;
		DG_COPY_MAT( &pre, &flr->world2 ) ;
		DG_COPY_MAT( &now, &flr->world ) ;
	} else {
		DG_COPY_MAT( &pre, &seg->world2 ) ;
		DG_COPY_MAT( &now, &seg->world ) ;
	}
	GV_VecToMat( &DG_ZeroVector, &pre ) ;
	GV_VecToMat( &DG_ZeroVector, &now ) ;

	DG_COPY_MAT( &this, old ) ;
	GV_VecToMat( &DG_ZeroVector, &this ) ;	

	_sceVu0TransposeMatrix( &t, &pre ) ;
	_sceVu0MulMatrix( &this, &t, &this ) ;
	_sceVu0MulMatrix( &this, &now, &this ) ;

	GV_MatToVec( old, &old_pos ) ;
	HZX_CallbackUpdatePos( hzd, &old_pos, &new_pos ) ;

	DG_COPY_MAT( new, &this ) ;
	GV_VecToMat( &new_pos, new ) ;
}

/*----------------------------------------------------------------*/

#ifdef HZX_DTRP
/* 動的トラップ */

/* 動的トラップ登録 */
HZX_D_TRP	*HZX_AddDynamicTrap( HZX_GROUP_ID hzx_id,
								 int   name_id,
								 float min_x, float min_y, float min_z,
								 float max_x, float max_y, float max_z, 
								 int atr )
{
	HZX_GRP 		*grp ;
	HZX_D_TRP		*trp, *trps ;

	trp = GV_Malloc( sizeof( HZX_D_TRP ) ) ;
	if ( trp == NULL ) {
		printf( "Warning : make dynamic trap failed\n" ) ;
		return NULL ;
	}
	
	grp = HZX_GetGroup( hzx_id ) ;
	trps = grp->dynamics->traps ;

	trp->name_id = name_id ;
	trp->attribute = atr ;
	GV_SetVec3( &trp->b1, min_x, min_y, min_z ) ;
	GV_SetVec3( &trp->b2, max_x, max_y, max_z ) ;
	
	trp->next = trps ;
	grp->dynamics->traps = trp ;
	grp->dynamics->n_traps ++ ;

	return trp ;
}

/* 動的トラップ削除 */
void		HZX_RemoveDynamicTrap( HZX_GROUP_ID hzx_id, HZX_D_TRP *trp )
{
	HZX_GRP 		*grp ;
	HZX_D_TRP		*trps, *prev ;	

	grp = HZX_GetGroup( hzx_id ) ;
	trps = grp->dynamics->traps ;
	prev = NULL ;

	while( trps != NULL ) {
		if ( trps == trp ) {
			if ( prev == NULL ) {
				grp->dynamics->traps = trp->next ;
			} else {
				prev->next = trp->next ;
			}
			grp->dynamics->n_traps -- ;
			GV_DelayedFree( trp ) ;
			break ;
		}
		prev = trps ;
		trps = trps->next ;
	}
}
#endif
