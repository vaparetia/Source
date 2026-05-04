/*
   brk_utlh
   壊れ用 定義ファイル

   2000/09/14 T. Morita
   $Id: brk_utl.h,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
*/


#ifndef _RBK_UTIL_H_
#define _RBK_UTIL_H_

enum
{
    R=0, G, B, A, RGBA
};
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

static inline float BackFaceCulling( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a ;

    _sceVu0OuterProduct( &a, v1, v2 ) ;
    return _sceVu0InnerProduct( &a, v3 ) ;
}

#if 0
static inline void ApplyMatrixXYZ( FVECTOR *a, FMATRIX *m, FVECTOR *v )
{
#ifdef PSX2
    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    vmulax.xyz  ACC,vf2,vf1x
    vmadday.xyz ACC,vf3,vf1y
    vmaddz.xyz  vf2,vf4,vf1z
    sqc2        vf2,0x00(%2)
    " : : "r"(m), "r"(v), "r"(a) );
#else
	vu0_ApplyMatrixROT( a, m, v ) ;
#endif
}
#endif


static inline void RotateMatrix( FMATRIX *out, FMATRIX *in, SVECTOR *rot  )
{
    int r ;
    r = rot->vx & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vy & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot->vz & 0x0fff ;
    _sceVu0RotMatrixZ( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixXZ( FMATRIX *out, FMATRIX *in, short rot_x, short rot_z )
{
    int r ;
    r = rot_z & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
   r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0AddVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W], pos ) ;
    mtx->m[W][W] = 1.0f ;
}

#endif /* _RBK_UTIL_H_*/
