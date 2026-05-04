//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_lgt.c
   壊れ用 汎用ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_lgt.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
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

/*

  DG_GetLightMatrixを用い光源計算
  mapから現在のライトデータからの環境光,および平行光をとりだし,
  lightマトリックスに反映させる。

 */
void BRK_UTL_GetLightMatrix( FVECTOR *pos, FMATRIX *lights, float intense, int where )
{
    LIT_DEF *lit_def ;
    MAP *map ;
    FVECTOR *lit_vec = (FVECTOR*)lights ;
    float    p_intense = 1.0f ;

    DG_GetLightMatrixFix( pos, lights ) ;

    if ( intense != 1.0f )
    {
	_sceVu0ScaleVector( lit_vec+Z, lit_vec+Z, intense ) ;
	_sceVu0ScaleVector( lit_vec+Y, lit_vec+Y, intense ) ;
	_sceVu0ScaleVector( lit_vec+X, lit_vec+X, intense ) ;
	p_intense = sceVu0Sqrt( _sceVu0InnerProduct( lit_vec+Z, lit_vec+Z ) ) ;
	if ( p_intense == 0.0f )
	    p_intense = sceVu0Sqrt( _sceVu0InnerProduct( lit_vec+Y, lit_vec+Y ) ) ;
	if ( p_intense == 0.0f )
	    p_intense = sceVu0Sqrt( _sceVu0InnerProduct( lit_vec+X, lit_vec+X ) ) ;
    }
    if ( where )
	if ( (map = GM_GetMap( where )) )
	    if ( (lit_def = map->light) )
	    {
		/*平行光の設定*/
		_sceVu0ScaleVector( (FVECTOR*)lights[0].m[2], &lit_def->dir, p_intense ) ;
		lights[1].m[2][R] = lit_def->color.r ;
		lights[1].m[2][G] = lit_def->color.g ;
		lights[1].m[2][B] = lit_def->color.b ;
		/*環境光の設定*/
		lights[1].m[3][R] = lit_def->ambient.r ;
		lights[1].m[3][G] = lit_def->ambient.g ;
		lights[1].m[3][B] = lit_def->ambient.b ;
	    }

#ifdef KP_WINDOWS
    if( DG_CheckLightMatrixNormalize() ){ DG_NormalizeLightMatrix(lights) ; }
#endif
}


/*

  位置より彩色を計算する関数

  影響度により変化を強さを変えることが可能。

*/
void BRK_UTL_PosColor( FVECTOR *pos, FVECTOR *dir, IVECTOR *col, float intense, int where )
{
    FVECTOR c = { 0.5f, 0.5f, 0.5f, 1.0f } ;
    FMATRIX lgt[2] ;

    BRK_UTL_GetLightMatrix( pos, lgt, intense, where ) ;

    c.vx = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[X], dir ) ;
    c.vy = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[Y], dir ) ;
    c.vz = _sceVu0InnerProduct( (FVECTOR*)&lgt[0].m[Z], dir ) ;
    c.vx = !(int)(c.vx/0.2f) ? 0.2f : c.vx>0.0f ? c.vx : -c.vx ;
    c.vy = !(int)(c.vy/0.2f) ? 0.2f : c.vy>0.0f ? c.vy : -c.vy ;
    c.vz = !(int)(c.vz/0.2f) ? 0.2f : c.vz>0.0f ? c.vz : -c.vz ;

    _sceVu0ApplyMatrix( &c, &lgt[1], &c ) ;
    col->vx = (int)(c.vx>168 ? 168 : c.vx) ;
    col->vy = (int)(c.vy>168 ? 168 : c.vy) ;
    col->vz = (int)(c.vz>168 ? 168 : c.vz) ;
    col->vw = 128  ; 
}

/*

  コモデルの彩色関数

  影響度により変化を強さを変えることが可能。

*/
void  BRK_UTL_ComdlColor( DG_COMDL_POS *p, float intense, int where )
{
    BRK_UTL_PosColor( (FVECTOR *)p->world.m[3], (FVECTOR *)p->world.m[1],
		      &p->color, intense, where ) ;
}

void  BRK_UTL_ComdlColor2( DG_COMDL_POS *p, int axis )
{
    FMATRIX lgt[2] ;
    FVECTOR c ;

    /* 色の更新 */
    DG_GetLightMatrixFix( (FVECTOR *)p->world.m[W], lgt ) ;

    _sceVu0ApplyMatrix( &c, &lgt[0], p->world.m[axis] ) ;
    c.vx = fpu_Abs( c.vx ) ;
    c.vy = fpu_Abs( c.vy ) ;
    c.vz = fpu_Abs( c.vz ) ;
    c.vw = 1.0f ;
    _sceVu0ApplyMatrix( &c, &lgt[1], &c ) ;

    p->color.vx += ((int)(c.vx<0 ? 0 : c.vx) - p->color.vx)/2 ;
    p->color.vy += ((int)(c.vy<0 ? 0 : c.vy) - p->color.vy)/2 ;
    p->color.vz += ((int)(c.vz<0 ? 0 : c.vz) - p->color.vz)/2 ;
    p->color.vw = 128 ;

    if ( p->color.vx > 255 )
	p->color.vx = 255 ;
    if ( p->color.vy > 255 )
	p->color.vy = 255 ;
    if ( p->color.vz > 255 )
	p->color.vz = 255 ;
}
