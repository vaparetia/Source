//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_act.c
   壊れ用 汎用ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_act.c,v 1.1.1.3 2002/11/19 11:45:50 Yoshizawa1 Exp $
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
#include "../brk_hzd/brk_hazard.h"




/* 中心がハザードの上にあるか  ハザードを返す */
HZD_BOX * BRK_UTL_GetCenterHazard( void *hzd, FVECTOR *o_pos, FVECTOR *o_vel,
				   float length, float speed )
{
    FVECTOR  pos, pos_v ;
    HZD_BOX *prv, *get ;

    _sceVu0CopyVector( &pos, o_pos ) ;
    pos_v.vz = pos_v.vx = 0.0f ;
    pos_v.vy = -length ;

    prv = BRK_HZD_Hazard ;
    if ( hzd )
	BRK_CheckHazard( (HZD_BOX *)hzd, &pos, &pos_v, &BRK_HZD_NoBounce, &DG_ZeroVector ) ;
    else
	BRK_CheckHazardScr( &pos, &pos_v, &BRK_HZD_NoBounce, &DG_ZeroVector ) ;
    get = BRK_HZD_Hazard ;
    BRK_HZD_Hazard = prv ;

    return get ;
}



/* 中心がハザードの上にあるか */
int BRK_UTL_PutCenterHazard( void *hzd, FVECTOR *o_pos, FVECTOR *o_vel,
			     float length, float speed )
{
#if 1
    HZD_BOX *get ;

    if ( BRK_HZD_Hazard )
	if ( (get = BRK_UTL_GetCenterHazard( hzd, o_pos, o_vel, length, speed )) )
	{
	    if ( get == BRK_HZD_Hazard )
		return 0 ;
	    if ( o_vel )
	    {
		_sceVu0SubVector( o_vel, o_pos,
				  (FVECTOR *)BRK_HZD_Hazard->world.m[W] ) ;
		o_vel->vy = 0.0f ;
		_sceVu0Normalize( o_vel, o_vel ) ;
		_sceVu0ScaleVector( o_vel, o_vel, speed ) ;
	    }
	    return 1 ;
	}

#else
    FVECTOR  pos, pos_v ;
    HZD_BOX *prv ;

    if ( (prv = BRK_HZD_Hazard) )
    {
	_sceVu0CopyVector( &pos, o_pos ) ;
	pos_v.vz = pos_v.vx = 0.0f ;
	pos_v.vy = -length ;

	if ( BRK_CheckHazard( (HZD_BOX *)hzd, &pos, &pos_v, &BRK_HZD_NoBounce, &DG_ZeroVector ) )
	    if ( prv == BRK_HZD_Hazard )
		return  0 ;
	if ( o_vel )
	{
	    _sceVu0SubVector( o_vel, o_pos, (FVECTOR *)prv->world.m[W] ) ;
	    o_vel->vy = 0.0f ;
	    _sceVu0Normalize( o_vel, o_vel ) ;
	    _sceVu0ScaleVector( o_vel, o_vel, speed ) ;
	}
	BRK_HZD_Hazard = prv ;
	return 1 ;
    }
#endif
    return 0 ;
}

/* ハザードにそわして置く */
int BRK_UTL_PutOnHazard( FMATRIX *mat, FVECTOR *o_pos,
			 float width, int rx_msk, int ry_msk,
			 void *hzd, FVECTOR *size )
{
    FVECTOR  pos, pos_v ;
	int flag ;

    _sceVu0CopyVector( &pos, o_pos ) ;
    pos.vx += frnd() * width ;
    pos.vz += frnd() * width ;
    pos_v.vz = pos_v.vx = 0.0f ;
    pos_v.vy = -2000.0f ;

	flag = BRK_CheckHazard( (HZD_BOX *)hzd,
						   &pos, &pos_v,
						   &BRK_HZD_NoBounce, size ) ;
    if ( !(BRK_HZD_Flag & BRK_HZD_THROUGH) && (flag & 1) ) {
		if ( !BRK_UTL_PutCenterHazard( (HZD_BOX *)hzd,
									   &pos, NULL,
									   3000.0f, 0.0f ) ) {
			RotateMatrixXY( mat,
						   &DG_UnitMatrix,
						   irnd() & rx_msk,
						   irnd() & ry_msk ) ;
			TransMatrix( mat, &pos ) ;
			return 1 ;
		}
	}
    return 0 ;
}


void BRK_UTL_MinMaxOfMdlBound( DG_MDL *model, FMATRIX *m, FVECTOR *min, FVECTOR *max )
{
#ifdef BP_PSX2_ASM
    FVECTOR lv, uv ;

    lv.vx = model->lx ;
    lv.vy = model->ly ;
    lv.vz = model->lz ;

    uv.vx = model->ux ;
    uv.vy = model->uy ;
    uv.vz = model->uz ;

    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    lqc2        vf6, 0(%3)
    lqc2        vf5, 0(%4)
    lqc2        vf7,0x00(%2)
    vmulax.xyz  ACC,vf2,vf1x
    vmulax.xyz  ACC,vf2,vf7x
    vmadday.xyz ACC,vf3,vf1y
    vmadday.xyz ACC,vf3,vf7y
    vmaddz.xyz  vf2,vf4,vf1z
    vmaddz.xyz  vf1,vf4,vf7z
    vmini.xyz   vf6, vf6, vf2
    vmax.xyz    vf5, vf5, vf2
    vmini.xyz   vf6, vf6, vf1
    vmax.xyz    vf5, vf5, vf1
    sqc2        vf5, 0(%3)
    sqc2        vf6, 0(%4)
    " : : "r"(m), "r"(&lv), "r"(&uv), "r"(min), "r"(max) );
#else

    FVECTOR l ;

    l.vx = model->lx ;
    l.vy = model->ly ;
    l.vz = model->lz ;
    ApplyMatrixXYZ( &l, m, &l ) ;
    MinMaxVector( min, max, &l ) ;

    l.vx = model->ux ;
    l.vy = model->uy ;
    l.vz = model->uz ;
    ApplyMatrixXYZ( &l, m, &l ) ;
    MinMaxVector( min, max, &l ) ;
#endif

}

/*
  モデルの大きさを取得する

  回転マトリックスに掛けられるモデルの大きさを取得するための関数
  返戻はポインタで渡されたsizeに入る。
*/
void BRK_UTL_SizeOfMDL( DG_MDL *mdl, FMATRIX *m, FVECTOR *size )
{
    FVECTOR min, max ;

#if 0
    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    BRK_UTL_MinMaxOfMdlBound( mdl, m, &min, &max ) ;
#else
    DG_MDLPACK *o = mdl->packs ;
    FVECTOR     f ;
#ifdef PSX2
    SVECTOR    *s = o->verts ;
    int         j = o->n_verts ;
#else
	DG_VERTEX_KMSS *s = mdl->vbuff ;
    int         j = mdl->n_verts ;
#endif

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    for ( ; --j>=0 ; s++ )
    {
#ifdef PSX2
        SVector2FVector( &f, s ) ;
#else
		f.vx = s->vx ;
		f.vy = s->vy ;
		f.vz = s->vz ;
#endif
		f.vw = 0.0f ;
		_sceVu0ApplyMatrix( &f, m, &f ) ;
        //ApplyMatrixXYZ( &f, m, &f ) ;
        //MinMaxVector( &min, &max, &f ) ;
		if ( min.vx > f.vx ) min.vx = f.vx ;
		if ( min.vy > f.vy ) min.vy = f.vy ;
		if ( min.vz > f.vz ) min.vz = f.vz ;
		if ( max.vx < f.vx ) max.vx = f.vx ;
		if ( max.vy < f.vy ) max.vy = f.vy ;
		if ( max.vz < f.vz ) max.vz = f.vz ;
    }
#endif
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
}

void BRK_UTL_SizeOfBound( DG_OBJS *objs, FMATRIX *m, FVECTOR *size )
{
    int     i, j ;
    FVECTOR f, min, max ;
    DG_OBJ *o = objs->objs ;

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;

    f.vw = 0.0f ;
    for( j=objs->n_models ; --j>=0 ; o++ )
#if 1
	for ( i=8 ; --i>=0 ; )
	{
	    f.vx = i&1 ? o->model->ux : o->model->lx ;
	    f.vy = i&2 ? o->model->uy : o->model->ly ;
	    f.vz = i&4 ? o->model->uz : o->model->lz ;
	    _sceVu0ApplyMatrix( &f, m, &f ) ;
	    //ApplyMatrixXYZ( &f, m, &f ) ;
	    if ( min.vx > f.vx ) min.vx = f.vx ;
	    if ( min.vy > f.vy ) min.vy = f.vy ;
	    if ( min.vz > f.vz ) min.vz = f.vz ;
	    if ( max.vx < f.vx ) max.vx = f.vx ;
	    if ( max.vy < f.vy ) max.vy = f.vy ;
	    if ( max.vz < f.vz ) max.vz = f.vz ;
	    //MinMaxVector( &min, &max, &f ) ;
	}
#else
	BRK_UTL_MinMaxOfMdlBound( o->model, m, &min, &max ) ;
#endif
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
}

