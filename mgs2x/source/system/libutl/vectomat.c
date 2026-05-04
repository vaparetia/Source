//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    ベクトルから マトリックスを求める。

    2000/02/10  H.TANAKA
    2000/04/20  T.Morita Revised
    $Id: vectomat.c,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


void  BIG_VectoMat(FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode)
{
    FVECTOR   base = {0.0F, -1.0F, 0.0F,0.0F} ;
    FVECTOR   ftmp[3] ;
    int       tmp ;

    /* 1つめのベクトル */
    _sceVu0Normalize(&ftmp[0],force) ;
    
    /* force と base の外せきを求める */
    _sceVu0OuterProduct(&ftmp[1],&base,force) ;
    /* 2つめのベクトル */
    _sceVu0Normalize(&ftmp[1],&ftmp[1]) ;
    /* 3つめのベクトル */
    _sceVu0OuterProduct(&ftmp[2],&ftmp[1],&ftmp[0]) ;

    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[2]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[1]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[0]) = ftmp[tmp] ; 

    *((FVECTOR *)world->m[3]) = *hit ;
      world->m[3][3] = 1.0F ;
}


/* Added by T.Morita*/
void  UTL_VectoMat( FMATRIX *world, FVECTOR *force, FVECTOR *hit, int axis )
{
    enum { X=0, Y, Z, XY=Z, W, XYZ=W, XYZW } ;
    FVECTOR   base = { 0.0F, 0.0F, 0.0F, 0.0F } ;
    FVECTOR  *ftmp[] = { (FVECTOR *)world->m[X], (FVECTOR *)world->m[Y], (FVECTOR *)world->m[Z] } ;

    /* 基準ベクトルを決める */
    ((float*)&base)[axis&XYZ] = (axis & 0x10) ? -1.0f : 1.0f ;
    _sceVu0Normalize( ftmp[X], force ) ;/*大きさを1にする*/

    /* force と 基準ベクトルの外せきを求める */
    _sceVu0OuterProduct( ftmp[Y], &base, force ) ;
    _sceVu0Normalize( ftmp[Y], ftmp[Y] ) ;
    _sceVu0OuterProduct( ftmp[Z], ftmp[Y], ftmp[X] ) ;

    if ( hit )
	*((FVECTOR *)world->m[W]) = *hit ;
    world->m[W][W] = 1.0F ;
}
