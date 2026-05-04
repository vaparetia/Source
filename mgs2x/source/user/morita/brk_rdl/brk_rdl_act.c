//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_rdl_act.c
   レードル揺れ アクト

   2000/09/08 T. Morita
   $Id: brk_rdl_act.c,v 1.1.1.3 2002/11/19 11:45:43 Yoshizawa1 Exp $
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

#include "../brk_utl/brk_utl.x"
#include "brk_radle.h"


/*
  基本的に動いている方が,止まっている方と当たっているかどうかを見る。
  動いている物同士の場合,2度チェックが入らないように,飛ばしている。
*/
void BRK_RDL_CheckCollide( Work *work )
{
    int     i, j ;
    FVECTOR p, q ;
    RADLE  *ri, *rj ;
    float   d ;

    for ( j=work->n_radle ; --j>=0 ; )
    {
	rj = &work->radle[j] ;
	if ( rj->mode )
	    for ( ri=work->radle, i=work->n_radle ; --i>=0 ; )
	    {
		if ( i==j || rj->col_id==i || ri->col_id==j || (ri->mode && i>j) )
		    continue ;
		ri = &work->radle[i] ;
		/*各揺れ物の最近点をqに計算する
		 つまり,2点を結ぶベクトルをモデルのローカルに射影する。*/
		_sceVu0SubVector( &p, &rj->col_p, &ri->col_p ) ;
		q.vx = _sceVu0InnerProduct( (FVECTOR *)ri->objs->world.m[X], &p ) ;
		q.vy = _sceVu0InnerProduct( (FVECTOR *)ri->objs->world.m[Y], &p ) ;
		q.vz = q.vw = 0.0f ;
		_sceVu0Normalize( &q, &q ) ;                      /*射影された単位ベクトル*/
		_sceVu0ScaleVector( &q, &q, ri->target.size.vx ) ;/*これで最近点になる*/
		_sceVu0ApplyMatrix( &q, &ri->objs->world, &q ) ;  /*ワールド座標に変換*/
		_sceVu0AddVector( &q, &q, &ri->col_p ) ;
		/*半径size.vxの円柱にはいっているかどうかの判定*/
		d = BRK_UTL_DistPointToLine( &q, &rj->col_p, (FVECTOR*)&rj->objs->world.m[Z] ) ;
		if ( rj->target.size.vx * 0.8f > d )
		{
#if MAKING
		    extern void *AN_Test_Eye2( FVECTOR *, int ) ;
		    AN_Test_Eye3( &q ) ;
		    //AN_Test_Eye3( &ri->col_p ) ;
#endif
		    /*厚さに入っているかどうか*/
		    d = _sceVu0InnerProduct( (FVECTOR *)&ri->objs->world.m[Z], &p ) ;
		    if ( !(int)(d/rj->target.size.vz/0.9f) )
		    {
			//BRK_RDL_SetSound( rj, (FVECTOR *)rj->objs->world.m[W] ) ;

			/*当たった時の挙動計算*/
#if 0
			ri->col_id = j ;/*二重計算しないように当たったら覚えておく*/
			ri->rot_vx = rj->rot_vx *  0.85f ;
			ri->rot_vz = rj->rot_vz *  0.85f ;
			if ( ri->rot_vx && ri->rot_vz )
			    ri->mode = BRK_MOD_MOVE ;
			rj->col_id = i ;
			rj->rot_vx = rj->rot_vz * -0.85f ;
			rj->rot_vz = rj->rot_vz * -0.85f ;
			if ( rj->rot_vx && rj->rot_vz )
			    rj->mode = BRK_MOD_MOVE ;
#else

#if MAKING
			AN_Test_Eye2( &q, 1 ) ;
#endif
			_sceVu0Normalize( &p, &p ) ;
			_sceVu0ScaleVector( &p, &p, BRK_RDL_MOVE_FORCE ) ;/*ちからの向きを計算*/
			ri->col_id = j ;/*二重計算しないように当たったら覚えておく*/
			ri->rot_vx = (short)(ri->rot_vx*0.8f + p.vz) ;
			ri->rot_vz = (short)(ri->rot_vz*0.8f - p.vx) ;
			if ( ri->rot_vx && ri->rot_vz )
			    ri->mode = BRK_MOD_MOVE ;
			rj->col_id = i ;
			rj->rot_vx = (short)(rj->rot_vx*0.8f - p.vz) ;
			rj->rot_vz = (short)(rj->rot_vz*0.8f + p.vx) ;
			if ( rj->rot_vx && rj->rot_vz )
			    rj->mode = BRK_MOD_MOVE ;
#endif
		    }
		}
	    }
    }
}

void BRK_RDL_Act( Work *work )
{
    int      j ,i, r ;
    RADLE   *radle = work->radle ;
    FMATRIX *mat ;
    int      flag  = 0 ;

    for ( j=work->n_radle ; --j>=0 ; flag|=radle->mode, radle++ )
	if ( radle->mode )
	{
	    /*各軸の揺れ計算*/
	    r = radle->rot_vx + radle->rot_x - radle->rot_dx ;
	    for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	    radle->rot_x += radle->rot_vx -= r ;

	    r = radle->rot_vz + radle->rot_z - radle->rot_dz ;
	    for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	    radle->rot_z += radle->rot_vz -= r ;
	    /* 揺れがデフォルトになったかどうか */
	    if ( (radle->rot_x==radle->rot_dx) && (!radle->rot_vx) &&
		 (radle->rot_z==radle->rot_dz) && (!radle->rot_vz) )
		radle->mode = BRK_MOD_FINISHED ;
	    /* ワールドを計算 */
	    mat = &radle->objs->world ;
	    MakeObjWorld( mat, radle, work->where ) ;

	    /* レードル同士での当たりの中心 */
	    radle->col_p = radle->target.offset ;
	    radle->col_p.vw = 1.0f ;
	    radle->col_p.vy += radle->col_p.vy + radle->target.size.vx ;
	    _sceVu0ApplyMatrix( &radle->col_p, mat, &radle->col_p ) ;
	    /*非当たりIDをクリア*/
	    radle->col_id = -1 ;
	}
    if ( !flag )
	work->n_radle |= BRK_RDL_INACTIVE ;
}
