#ifndef __FORT_WEAP_H__
#define __FORT_WEAP_H__

#include "../../include/libdg_x.h"
#include "../../include/util.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

extern int  FRT_HzdSegLineCheck( HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 ) ;
extern void FRT_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p ) ;
extern int  FRT_HzdCheck( HZX_GROUP_ID hzx, FVECTOR *pos, FVECTOR *pos_v,
			  float min, FVECTOR *bounce, float sphere ) ;
extern int  FRT_HzdOnlineCheck( HZX_GROUP_ID hzx, FVECTOR *pos, FVECTOR *pos_v,
				float min, FVECTOR *bounce ) ;

extern void *NewFortWeaponBarriar( int mode, FVECTOR *fort ) ;            /* weapon/fort_bar.c */


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
static inline void RotateMatrixX( FMATRIX *out, FMATRIX *in, short rot )
{
    int r ;
    r = rot & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixY( FMATRIX *out, FMATRIX *in, short rot )
{
    int r ;
    r = rot & 0x0fff ;
    _sceVu0RotMatrixY( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void RotateMatrixZ( FMATRIX *out, FMATRIX *in, short rot )
{
    int r ;
    r = rot & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos )
{
    mtx->m[W][X] += pos->vx ;
    mtx->m[W][Y] += pos->vy ;
    mtx->m[W][Z] += pos->vz ;
}

static inline void CopyRMatrix( FMATRIX *dst, FMATRIX *src )
{
#ifdef BP_PSX2_ASM
    asm volatile ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x10(%1)
    lqc2        vf3,0x20(%1)
    sqc2        vf1,0x00(%0)
    sqc2        vf2,0x10(%0)
    sqc2        vf3,0x20(%0)
    " : : "r"(dst), "r"(src) ) ;
#else
	_sceVu0CopyVector( (FVECTOR*)dst->m[0], (FVECTOR*)src->m[0] ) ;
	_sceVu0CopyVector( (FVECTOR*)dst->m[1], (FVECTOR*)src->m[1] ) ;
	_sceVu0CopyVector( (FVECTOR*)dst->m[2], (FVECTOR*)src->m[2] ) ;
#endif
}


#endif
