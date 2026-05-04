//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_glass.c
   ガラス壊れ用のモジュール

   1999/11/26 T. Morita
   $Id: brk_utl_glass.c,v 1.1.1.3 2002/11/19 11:45:51 Yoshizawa1 Exp $
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



#define BRK_BGLS_SCAR_W 350.0f
#define FTOI12(_f) ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

void BRK_UTL_PutGlassWeb( TARGET *t, DG_TEX *tex,
			  FVECTOR *hit_pos, float offset, float size,
			  FVECTOR        *v0, FVECTOR        *v1,
			  DG_PRIM2_UVRGB *u0, DG_PRIM2_UVRGB *u1  )
{
    FMATRIX  mat, inv  ;
    FVECTOR  hit, ofst ;
    float    uv_b[4] = { 0.5f, 0.5f, 0.5f, 0.5f } ;
    int      u[4], v[5] ;
    float    w, h, l ;
    int      i ;

    if ( hit_pos )
	_sceVu0CopyVector( &hit, hit_pos ) ;
    else
	_sceVu0CopyVector( &hit, &t->hit ) ;
    if ( t->class & TARGET_ROTATE )
    {
	hit.vw = 1.0f ;
	_sceVu0CopyMatrix( &mat, &t->world ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)mat.m[3], &t->center ) ;
	_sceVu0InversMatrix( &inv, &mat ) ;
	_sceVu0ApplyMatrix( &hit, &inv, &hit ) ;
    }

    /* 取りあえずプリミティブの形を作る 表 */
    v0[0].vx = v0[2].vx = hit.vx - size ;
    v0[1].vx = v0[3].vx = hit.vx + size ;
    v0[0].vy = v0[1].vy = hit.vy - size ;
    v0[2].vy = v0[3].vy = hit.vy + size ;
    v0[0].vz = v0[1].vz = v0[2].vz = v0[3].vz = t->offset.vz + offset ;
    v0[4].vx = v0[0].vx ; 
    v0[4].vy = v0[0].vy ;
    v0[4].vz = t->offset.vz + offset*1000.0f ;
    v0[0].vw = v0[1].vw = v0[2].vw = v0[3].vw = v0[4].vw = 1.0f ;
    /* 取りあえずプリミティブの形を作る 裏 カリングを考えて逆回しにする */
    if ( v1 )
    {
	v1[0].vx = v1[1].vx = hit.vx - size ;
	v1[2].vx = v1[3].vx = hit.vx + size ;
	v1[0].vy = v1[2].vy = hit.vy - size ;
	v1[1].vy = v1[3].vy = hit.vy + size ;
	v1[0].vz = v1[1].vz = v1[2].vz = v1[3].vz = t->offset.vz - offset ;
	v1[4].vx = v1[0].vx ; 
	v1[4].vy = v1[0].vy ;
	v1[4].vz = t->offset.vz - offset*1000.0f ;
	v1[0].vw = v1[1].vw = v1[2].vw = v1[3].vw = v1[4].vw = 1.0f ;
    }

    /* ガラスからはみ出ていないかを見る */
    _sceVu0SubVector( &ofst, &hit, &t->offset ) ;
    if ( ofst.vx >  (t->size.vx - size) )/* 横がはみ出た 右側？ */
    {
	w = t->offset.vx + t->size.vx ;
	uv_b[0] *= 1.0f - (v0[1].vx - w)/size ;
	v0[1].vx = v0[3].vx = w ;
	if ( v1 )
	    v1[2].vx = v1[3].vx = w ;
    }
    if ( ofst.vx < -(t->size.vx - size) )/* 横がはみ出た 左側？ */
    {
	w = t->offset.vx - t->size.vx ;
	uv_b[1] *= 1.0f - (w - v0[0].vx)/size ;
	v0[0].vx = v0[2].vx = w ;
	if ( v1 )
	    v1[0].vx = v1[1].vx = w ;
    }

    if ( ofst.vy >  (t->size.vy - size) )/* 縦がはみ出た 上側？ */
    {
	h = t->offset.vy + t->size.vy ;
	uv_b[2] *= 1.0f - (v0[2].vy - h)/size ;
	v0[2].vy = v0[3].vy = t->offset.vy + t->size.vy ;
	if ( v1 )
	    v1[1].vy = v1[3].vy = v0[2].vy ;
    }
    if ( ofst.vy < -(t->size.vy - size) )/* 縦がはみ出た 下側？ */
    {
	h = t->offset.vy - t->size.vy ;
	uv_b[3] *= 1.0f - (h - v0[0].vy)/size ;
	v0[0].vy = v0[1].vy = h ;
	if ( v1 )
	    v1[0].vy = v1[2].vy = h ;
    }

    /* 回転属性があれば回転に合わせて表示される */
    if ( t->class & TARGET_ROTATE )
    {
	for ( i=5 ; --i>=0 ; )
	    _sceVu0ApplyMatrix( &v0[i], &mat, &v0[i] ) ;
	if ( v1 )
	    for ( i=5 ; --i>=0 ; )
		_sceVu0ApplyMatrix( &v1[i], &mat, &v1[i] ) ;
    }

    /* テクスチャーのUVを計算（はみ出たら,それを考慮する） */
    if ( u0 || u1 )
    {
	l = M_PI * frnd() ;
	w = 0.707f * cosf( l ) ;
	h = 0.707f * sinf( l ) ;/*ランダムに回転*/
	u[0] = FTOI12( (0.5f-w*uv_b[1]+h*uv_b[3]) * tex->u_scale + tex->u_offset ) ;
	v[0] = FTOI12( (0.5f-h*uv_b[1]-w*uv_b[3]) * tex->v_scale + tex->v_offset ) ;
	u[1] = FTOI12( (0.5f+w*uv_b[0]+h*uv_b[3]) * tex->u_scale + tex->u_offset ) ;
	v[1] = FTOI12( (0.5f+h*uv_b[0]-w*uv_b[3]) * tex->v_scale + tex->v_offset ) ;
	u[2] = FTOI12( (0.5f-w*uv_b[1]-h*uv_b[2]) * tex->u_scale + tex->u_offset ) ;
	v[2] = FTOI12( (0.5f-h*uv_b[1]+w*uv_b[2]) * tex->v_scale + tex->v_offset ) ;
	u[3] = FTOI12( (0.5f+w*uv_b[0]-h*uv_b[2]) * tex->u_scale + tex->u_offset ) ;
	v[3] = FTOI12( (0.5f+h*uv_b[0]+w*uv_b[2]) * tex->v_scale + tex->v_offset ) ;
	if ( u0 )
	{
	    u0[0].u = u[0] ;	u0[0].v = v[0] ;
	    u0[1].u = u[1] ;	u0[1].v = v[1] ;
	    u0[2].u = u[2] ;	u0[2].v = v[2] ;
	    u0[3].u = u[3] ;	u0[3].v = v[3] ;
	    u0[0].a = u0[1].a = u0[2].a = u0[3].a = 128 ;
	}
	if ( u1 )
	{
	    u1[0].u = u[0] ;	u1[0].v = v[0] ;
	    u1[2].u = u[1] ;	u1[2].v = v[1] ;
	    u1[1].u = u[2] ;	u1[1].v = v[2] ;
	    u1[3].u = u[3] ;	u1[3].v = v[3] ;
	    u1[0].a = u1[1].a = u1[2].a = u1[3].a = 128 ;
	}
    }
}
