//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_ini.c
   壊れ用 汎用初期化ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_ini.c,v 1.1.1.3 2002/11/19 11:45:51 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_utl.h"

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/*

  プリミティブの初期化

*/

void BRK_UTL_InitSPR( DG_PRIM2 *p, int i, int rgba, DG_TEX *t, float size )
{
    DG_PRIM2_UVRGBWH *u0, *u1 ;
    FVECTOR          *p0, *p1 ;
    int       r,g,b,a ;

    r = (rgba>> 0)&0xff ;
    g = (rgba>> 8)&0xff ;
    b = (rgba>>16)&0xff ;
    a = (rgba>>24)&0xff ;

    u0 = p->uvrgb[0] ;
    u1 = p->uvrgb[1] ;
    p0 = p->pos  [0] ;
    p1 = p->pos  [1] ;
    for ( ; --i>=0 ; u0++, u1++, p0++, p1++ )
    {
	u0->u0 = u1->u0 = FTOI12( 0.0f * t->u_scale + t->u_offset ) ;
	u0->v0 = u1->v0 = FTOI12( 0.0f * t->v_scale + t->v_offset ) ;
	u0->u1 = u1->u1 = FTOI12( 1.0f * t->u_scale + t->u_offset ) ;
	u0->v1 = u1->v1 = FTOI12( 1.0f * t->v_scale + t->v_offset ) ;
	u0->q0 = u1->q0 = u0->q1 = u1->q1 = 4096 ;
	u0->f0 = u1->f0 = u0->f1 = u1->f1 = 0x0fff ;

	u0->w = u1->w = u0->h = u1->h = (short)size ;

        u0->r = u1->r = r ;
	u0->g = u1->g = g ;
	u0->b = u1->b = b ;
        u0->a = u1->a = a ;

	_sceVu0CopyVector( p0, &DG_ZeroVector ) ;
	_sceVu0CopyVector( p1, &DG_ZeroVector ) ;
    }
}

DG_PRIM2* BRK_UTL_MakeSPRT( int n_prim, int tex_id, u_long64 alpha )
{
    DG_PRIM2 *p ;
    DG_TEX   *t ;
    int       rgba = 0x007f7f7f ;

    if ( !(p = GM_MakePrim2( DG_PRIM2_SPRT| DG_PRIM2_TEX| DG_PRIM2_ALPHA, n_prim, 1 )) )
	return NULL ;
    DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;
    BRK_UTL_InitSPR( p, n_prim, rgba, t, 0.0f ) ;

    return p ;
}

DG_PRIM2* BRK_UTL_MakeSPRTWH( int n_prim, int tex_id, u_long64 alpha, float size, int rgba )
{
    DG_PRIM2 *p ;
    DG_TEX   *t ;

    if ( !(p = GM_MakePrim2( DG_PRIM2_SPRT| DG_PRIM2_TEX| DG_PRIM2_ALPHA, n_prim, 1 )) )
	return NULL ;
    DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;
    BRK_UTL_InitSPR( p, n_prim, rgba, t, size ) ;

    return p ;
}

DG_PRIM2* BRK_UTL_MakeRSPRTWH( int n_prim, int tex_id, u_long64 alpha, float size, int rgba )
{
    DG_PRIM2 *p ;
    DG_TEX   *t ;

    if ( !(p = GM_MakePrim2( DG_PRIM2_RSPRT| DG_PRIM2_TEX| DG_PRIM2_ALPHA, n_prim, 1 )) )
	return NULL ;
    DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;
    BRK_UTL_InitSPR( p, n_prim, rgba, t, size ) ;

    return p ;
}








void BRK_UTL_InitPOLY( DG_PRIM2 *p, int i, int rgba, DG_TEX *t )
{
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    int       r,g,b,a ;

    r = (rgba>> 0)&0xff ;
    g = (rgba>> 8)&0xff ;
    b = (rgba>>16)&0xff ;
    a = (rgba>>24)&0xff ;

    u0 = p->uvrgb[0] ;
    u1 = p->uvrgb[1] ;
    p0 = p->pos  [0] ;
    p1 = p->pos  [1] ;
    for ( ; --i>=0 ; u0+=4, u1+=4, p0+=4, p1+=4 )
    {
	if ( t )
	{
	    u0[0].u = u0[2].u = u1[0].u = u1[2].u = FTOI12( 0.0f * t->u_scale + t->u_offset ) ;
	    u0[1].u = u0[3].u = u1[1].u = u1[3].u = FTOI12( 1.0f * t->u_scale + t->u_offset ) ;
	    u0[0].v = u0[1].v = u1[0].v = u1[1].v = FTOI12( 0.0f * t->v_scale + t->v_offset ) ;
	    u0[2].v = u0[3].v = u1[2].v = u1[3].v = FTOI12( 1.0f * t->v_scale + t->v_offset ) ;
	}
	u0[0].q = u0[1].q = u0[2].q = u0[3].q =
	    u1[0].q = u1[1].q = u1[2].q = u1[3].q = 4096 ;
	u0[0].f = u0[1].f = u1[0].f = u1[1].f = 0x8fff ;
	u0[2].f = u0[3].f = u1[2].f = u1[3].f = 0x0fff ;

	u0[0].r = u0[1].r = u0[2].r = u0[3].r =
	    u1[0].r = u1[1].r = u1[2].r = u1[3].r = r ;
	u0[0].g = u0[1].g = u0[2].g = u0[3].g =
	    u1[0].g = u1[1].g = u1[2].g = u1[3].g = g ;
	u0[0].b = u0[1].b = u0[2].b = u0[3].b =
	    u1[0].b = u1[1].b = u1[2].b = u1[3].b = b ;
	u0[0].a = u0[1].a = u0[2].a = u0[3].a =
	    u1[0].a = u1[1].a = u1[2].a = u1[3].a = a ;


	_sceVu0CopyVector( &p0[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p1[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p0[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p1[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p0[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p1[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p0[3], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p1[3], &DG_ZeroVector ) ;
    }
}

DG_PRIM2* BRK_UTL_MakePOLYFlag( int n_prim, int flag,
				int tex_id, u_long64 alpha, int rgba )
{
    DG_PRIM2 *p ;
    DG_TEX   *t ;

    if ( tex_id )
	t = DG_GetTexture( tex_id ) ;
    else
	t = NULL ;
    if ( t )
	flag |= DG_PRIM2_TEX ;
    if ( !(p = GM_MakePrim2( flag, n_prim, 4 )) )
	return NULL ;
    if ( t )
	DG_ConfigPrim2Tex( p, t ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;

    BRK_UTL_InitPOLY( p, n_prim, rgba, t ) ;

    return p ;
}

DG_PRIM2* BRK_UTL_MakePOLY( int n_prim, int tex_id, u_long64 alpha, int rgba )
{
    return BRK_UTL_MakePOLYFlag( n_prim,
				 DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
				tex_id, alpha, rgba ) ;
}

DG_PRIM2* BRK_UTL_MakePOLYFOG( int n_prim, int tex_id, u_long64 alpha, int rgba )
{
    return BRK_UTL_MakePOLYFlag( n_prim,
				 DG_PRIM2_POLY|DG_PRIM2_ALPHA|
				 DG_PRIM2_SHADE|DG_PRIM2_FOG,
				 tex_id, alpha, rgba ) ;
}







#define  LINE_FLG (DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_SHADE)
DG_PRIM2* BRK_UTL_MakeLINE2( int n_prim, int n_verts, int tex_id, u_long64 alpha, int rgba0, int rgba1 )
{
    DG_PRIM2 *p ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;
    DG_TEX   *t = NULL ;
    int       i, j ;
    int       r0,g0,b0,a0, r1,g1,b1,a1 ;
    float     r ;

    r0 = (rgba0>> 0)&0xff ; r1 = (rgba1>> 0)&0xff ;
    g0 = (rgba0>> 8)&0xff ; g1 = (rgba1>> 8)&0xff ;
    b0 = (rgba0>>16)&0xff ; b1 = (rgba1>>16)&0xff ;
    a0 = (rgba0>>24)&0xff ; a1 = (rgba1>>24)&0xff ;

    if ( !(p = GM_MakePrim2( (tex_id ? LINE_FLG|DG_PRIM2_TEX : LINE_FLG), n_prim, n_verts )) )
	return NULL ;
    if ( tex_id )
	DG_ConfigPrim2Tex( p, t=DG_GetTexture( tex_id ) ) ;
    if ( alpha )
	DG_SetPrim2Alpha( p, alpha ) ;

    u0 = p->uvrgb[0] ;
    u1 = p->uvrgb[1] ;
    p0 = p->pos  [0] ;
    p1 = p->pos  [1] ;
    for ( i=n_prim ; --i>=0 ; )
	for ( j=0 ; j<n_verts ; j++ )
	{
	    r = (float)j / (float)(n_verts-1) ;
	    if ( t )
	    {
		u0->u = u1->u = FTOI12( (float)r * t->u_scale + t->u_offset ) ;
		u0->v = u1->v = FTOI12( (float)r * t->v_scale + t->v_offset ) ;
	    }
	    u0->q = u1->q = 4096 ;
	    u0->f = u1->f = j ? 0x0fff : 0x8fff ;

	    u0->r = u1->r = (int)(r*r0 + (1.0f-r)*r1) ;
	    u0->g = u1->g = (int)(r*g0 + (1.0f-r)*g1) ;
	    u0->b = u1->b = (int)(r*b0 + (1.0f-r)*b1) ;
	    u0->a = u1->a = (int)(r*a0 + (1.0f-r)*a1) ;
	    u0++, u1++ ;

	    _sceVu0CopyVector( p0, &DG_ZeroVector ) ;
	    _sceVu0CopyVector( p1, &DG_ZeroVector ) ;
	    p0++, p1++ ;
	}
    return p ;
}

DG_PRIM2* BRK_UTL_MakeLINE( int n_prim, int tex_id, u_long64 alpha, int rgba0, int rgba1 )
{
    return BRK_UTL_MakeLINE2( n_prim, 2, tex_id, alpha, rgba0, rgba1 ) ;
}



FMATRIX BRK_UTL_MatrixTable[16][16] ;

int BRK_UTL_InitMatrixTable()
{
    int      x, z ;
    FMATRIX *m ;

#if 1
    for ( z=16 ; --z>=0 ; )
	for ( x=16 ; --x>=0 ; )
	{
	    m = &BRK_UTL_MatrixTable[x][z] ;
	    _sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)(2.0f * (float)M_PI * x /16 - (float)M_PI) ) ;
	    _sceVu0RotMatrixY( m, m             , (float)(2.0f * (float)M_PI * z /16 - (float)M_PI) ) ;
	}
#else
    static int flag  = 1 ;

    if ( flag )
	for ( z=16 ; --z>=0 ; )
	    for ( x=16 ; --x>=0 ; )
	    {
		m = &BRK_UTL_MatrixTable[x][z] ;
		_sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)M_PI * ( (float)x/8.0f - 1.0f ) ) ;
		_sceVu0RotMatrixY( m, m             , (float)M_PI * ( (float)z/8.0f - 1.0f ) ) ;
	    }
    flag = 0 ;
#endif

    return 0 ;
}
