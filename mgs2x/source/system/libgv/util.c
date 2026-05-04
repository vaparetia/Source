//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   util.c 
   システムが完成するまでの一時的なユーティリティ関数置き場
   
   1999/07/07 M.Sonoyama
   $Id: util.c,v 1.1.1.3 2002/11/19 11:42:45 Yoshizawa1 Exp $			
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

#include	"gameheader.h"
#endif //PSX2

#ifdef KP_XBOX
#include <xtl.h>

#include "libgv.h"

#include	"gameheader.h"	/// まだないので
#endif //KP_XBOX

#include "bp_matrix.h"

static	void	SetPItoPI( v )
short		*v ;
{
    *v &= 4095 ;
    if ( *v >= 2048 ) *v -= 4096 ;
}

void		GV_ItoFVector( ps2, ps )
FVECTOR		*ps2 ;
SVECTOR		*ps ;
{
    SVECTOR	v ;
    
    v = *ps ;
    SetPItoPI( &v.vx ) ;
    SetPItoPI( &v.vy ) ;
    SetPItoPI( &v.vz ) ;
    ps2->vx = ( float )v.vx * TPI / 4096.0F ;
    ps2->vy = ( float )v.vy * TPI / 4096.0F ;
    ps2->vz = ( float )v.vz * TPI / 4096.0F ;
    ps2->vw = 0.0F ;
}

void		GV_FtoIVector( sv, fv )
SVECTOR		*sv ;
FVECTOR		*fv ;
{
    sv->vx = ( short )( fv->vx * 2048.0F / PI ) ;
    sv->vy = ( short )( fv->vy * 2048.0F / PI ) ;
    sv->vz = ( short )( fv->vz * 2048.0F / PI ) ;
}

/*----------------------------------------------------------------*/

void	GV_IVtoFV( src, dst, n )
void	*src ;
void	*dst ;
int     n ;
{
    int		*iv ;
    float	*fv ;

    iv = ( int * )src ;
    fv = ( float * )dst ;
    while( -- n >= 0 ) {
		*( fv ) = ( float )( *iv ) ;
		fv ++ ; iv ++ ;
    }
}

void	GV_FVtoIV( src, dst, n )
void	*src ;
void	*dst ;
int     n ;
{
    int		*iv ;
    float	*fv ;

    fv = ( float * )src ;
    iv = ( int * )dst ;
    while( -- n >= 0 ) {
		*( iv ) = ( int )( *fv ) ;
		fv ++ ; iv ++ ;
    }
}
#if defined(PSX2) && !defined(BP_VITA)
inline
#endif
void	GV_MatToVec( m, v )
FMATRIX	*m ;
FVECTOR	*v ;
{
    v->vx = m->m[ 3 ][ 0 ] ;
    v->vy = m->m[ 3 ][ 1 ] ;
    v->vz = m->m[ 3 ][ 2 ] ;
}

#if defined(PSX2) && !defined(BP_VITA)
inline
#endif
void	GV_VecToMat( v, m )
FVECTOR	*v ;
FMATRIX	*m ;
{
    m->m[ 3 ][ 0 ] = v->vx ;
    m->m[ 3 ][ 1 ] = v->vy ;
    m->m[ 3 ][ 2 ] = v->vz ;
}

/*----------------------------------------------------------------*/

int		GV_VecLen3( vec )
SVECTOR		*vec ;
{
    FVECTOR		tmp ;
    float		f ;

    tmp.vx = vec->vx ;
    tmp.vy = vec->vy ;
    tmp.vz = vec->vz ;
    tmp.vw = 0.0F ;
    f = _sceVu0InnerProduct( &tmp, &tmp ) ;
#ifdef PSX2	
    return ( int )sceVu0Sqrt( f ) ;
#else
    return ( int )bp_sqrtf( f ) ;   //BP_MATH - emulate PS2 sqrtf
#endif	
}

float		GV_VecLen3F( vec )
FVECTOR		*vec ;
{
    FVECTOR		fv0 ;
    float 		f ;

	DG_COPY_VEC( &fv0, vec ) ;
    fv0.vw = 0.0F ;
    f = _sceVu0InnerProduct( &fv0, &fv0 ) ;
	if ( f == 0.0F ) return 0.0F ;
#ifdef PSX2	
    return sceVu0Sqrt( f ) ;
#else
	return bp_sqrtf( f ) ;  //BP_MATH - emulate PS2 sqrtf
#endif	
}

float		GV_VecLen3F2( v1, v2 )
FVECTOR		*v1, *v2 ;
{
    FVECTOR	diff ;

    _sceVu0SubVector( &diff, v1, v2 ) ;
    return GV_VecLen3F( &diff ) ;
}

void		GV_LenVec3( vec1, vec2, len1, len2 )
SVECTOR		*vec1, *vec2 ;
int		len1, len2 ;
{
    FVECTOR	tmp, tmp2 ;

    ASSERT( len1 > 0 ) ;
    tmp.vx = vec1->vx ;
    tmp.vy = vec1->vy ;
    tmp.vz = vec1->vz ;
    tmp.vw = 0.0F ;
    _sceVu0Normalize( &tmp, &tmp ) ;
    _sceVu0ScaleVector( &tmp2, &tmp, ( float )len2 ) ;
    vec2->vx = (short)tmp2.vx ;
    vec2->vy = (short)tmp2.vy ;
    vec2->vz = (short)tmp2.vz ;
}

void		GV_SubVec3( vec1, vec2, vec3 )
SVECTOR		*vec1, *vec2, *vec3 ;
{
    vec3->vx = vec1->vx - vec2->vx ;
    vec3->vy = vec1->vy - vec2->vy ;
    vec3->vz = vec1->vz - vec2->vz ;
}

void		GV_SubVec3F( vec1, vec2, vec3 )
FVECTOR		*vec1, *vec2, *vec3 ;
{
    _sceVu0SubVector( vec3, vec1, vec2 ) ;
}

void		GV_LenVec3F( 
		FVECTOR		*vec1, FVECTOR *vec2 ,
		float		len1, float len2 )
{
    FVECTOR	tmp ;

    _sceVu0Normalize( &tmp, vec1 ) ;
    _sceVu0ScaleVector( vec2, &tmp, len2 ) ;
}

short		GV_VecDir2( vec )
FVECTOR		*vec ;
{
    float	dir ;
    short	res ;

    dir = atan2f( vec->vx, vec->vz ) ;
    res = 4095 & ( short )( ( dir * 2048.0F / PI ) + 0.5F ) ; /* 四捨五入 */
    return res ;
}

short		GV_VecDir2X( vec )
FVECTOR		*vec ;
{
	FVECTOR		v ;
	short		dir ;

	v.vx = vec->vy ;
	v.vz = bp_sqrtf( vec->vx * vec->vx + vec->vz * vec->vz ) ;  //BP_MATH - emulate PS2 sqrtf
	dir = - GV_VecDir2( &v ) ;
	return dir ;
}

short		GV_VecDir2FromTo( FVECTOR *from, FVECTOR *to )
{
	FVECTOR		diff ;

	_sceVu0SubVector( &diff, to, from ) ;
	return GV_VecDir2( &diff ) ;
}

void		GV_VecToRot( vec, rot )
FVECTOR		*vec ;
SVECTOR		*rot ;
{
	rot->vy = GV_VecDir2( vec ) ;
	rot->vx = GV_VecDir2X( vec ) ;
	rot->vz = 0 ;
}

void		GV_DirVec3( dir, len, vec )
SVECTOR		*dir ;		/* 方向		*/
int		len ;		/* 長さ		*/
FVECTOR		*vec ;		/* ベクトル	*/
{
    FMATRIX	m0 ;
    FVECTOR	rot ;

    GV_ItoFVector( &rot, dir ) ;
    _sceVu0UnitMatrix( &m0 ) ;
	//    _sceVu0RotMatrixZ( &m0, &m0, rot.vz ) ;
    _sceVu0RotMatrixX( &m0, &m0, rot.vx ) ;
    _sceVu0RotMatrixY( &m0, &m0, rot.vy ) ;
    vec->vx = ( float )len * m0.m[ 2 ][ 0 ] ;
    vec->vy = ( float )len * m0.m[ 2 ][ 1 ] ;
    vec->vz = ( float )len * m0.m[ 2 ][ 2 ] ;
}

int		GV_DiffDirU( from, to )
int		from, to ;
{
    return 4095 & ( to - from ) ;
}

int		GV_DiffDirS( from, to )
int		from, to ;
{
    int		diff ;

    diff = 4095 & ( to - from ) ;
    return ( diff <= 2048 ) ? diff : diff - 4096 ;
}

int		GV_DiffDirAbs( from, to )
int		from, to ;
{
    int		diff ;

    diff = 4095 & ( to - from ) ;
    return ( diff <= 2048 ) ? diff : 4096 - diff ;
}

/* 点と直線の距離 */
float		GV_PointToVectorLen( p, v1, v2 )
FVECTOR		*p, *v1, *v2 ;
{
    FVECTOR	d1, d2, d3, d4 ;

    _sceVu0SubVector( &d2, p, v1 ) ;
    if ( v1->vx == v2->vx &&
		v1->vy == v2->vy &&
		v1->vz == v2->vz ) {
		return GV_VecLen3F( &d2 ) ; 
    }
    _sceVu0SubVector( &d1, v2, v1 ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0OuterProduct( &d3, &d1, &d2 ) ;
    _sceVu0OuterProduct( &d4, &d1, &d3 ) ;
    return GV_VecLen3F( &d4 ) ;
}

/* ある位置から、指定ベクトルだけシフトした座標を計算する
   ベクトルは指定角度に回転させる */
void			GV_GetShiftPos(
		FVECTOR			*res ,
		FVECTOR			*pos ,
		SVECTOR			*rot ,
		float			vx, float vy, float vz )
{
	res->vx = vx ;
	res->vy = vy ;
	res->vz = vz ;
	DG_SetPos2( pos, rot ) ;
	DG_PutVector( res, res, 1 ) ;
}
