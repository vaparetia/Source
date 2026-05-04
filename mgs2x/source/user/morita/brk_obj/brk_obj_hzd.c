//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_obj_hzx.c
   じゃがいも壊れ 当たり判定用ルーチン

   1999/12/15 T. Morita
   $Id: brk_obj_hzd.c,v 1.1.1.3 2002/11/19 11:45:37 Yoshizawa1 Exp $
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

#include "brk_object.h"



int BRK_OBJ_HzdSegLineCheck( HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    float px, pz, sx, sz, tx, tz ;
    float r, s, rd ;

    px = p2->vx    - p1->vx    ; pz = p2->vz    - p1->vz    ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*pz - sz*px) == 0.0f )                                
      return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if ( r > 1.0f || r < 0.0f )               
      return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
      return  0 ;
    return 1 ;
}

void BRK_OBJ_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;
}

int BRK_OBJ_HzdCheck( Work *work,
		      FVECTOR *pos, FVECTOR *pos_v,
		      float min, FVECTOR *bounce, float sphere )
{
    int   atr[2] ;
    float flr[2] ;
    HZX_SEG seg[2] ;
    int flag = 0 ;
    FVECTOR new ;

    _sceVu0AddVector( &new, pos, pos_v ) ;
    if ( HZX_NearHazardCheck( work->hzx, pos, BRK_HZX_S_SPHERE, HZX_CHK_ALL, HZX_SEG_ALL, (int)sphere ) )
    {
	HZX_GetNearHazard( seg, atr ) ;
	if ( BRK_OBJ_HzdSegLineCheck( seg, pos, &new ) )
	    BRK_OBJ_HzdSegReactVector( seg, pos_v ), flag |= 2 ;
    }
    if ( HZX_LevelHazardCheck( work->hzx, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
	HZX_GetLevelHeight( flr ) ;
	if ( new.vy < (flr[0] -= min) )
	    pos->vy = flr[0], pos_v->vy *= bounce->vy, flag |= 1 ;
    }
    return flag ;
}


void BRK_OBJ_HzdObjectCheck( Work *work, float rad )
{
    int     i, j ;
    float   f, d ;
    FVECTOR v ;
    BRK_OBJ *p, *o ;

    for ( i=work->n_obj, p=work->obj ; --i>=0 ; p++ )
	if ( p->act )
	    for ( j=i, o=p+1 ; --j>=0 ; o++ )
		if ( o->act )
		{
		    SubVector( &v, &p->pos, &o->pos ) ;
		    d = v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ;
		    f = p->target.size.vx + o->target.size.vx ;
		    if ( d <= f*f )
		    {
			if ( o->act == BRK_OBJ_ActNone )
			    o->act = BRK_OBJ_ActMove ;
			if ( p->act == BRK_OBJ_ActNone )
			    p->act = BRK_OBJ_ActMove ;

			f = ( (p->pos_v.vx - o->pos_v.vx) * v.vx +
			      (p->pos_v.vy - o->pos_v.vy) * v.vy + 
			      (p->pos_v.vz - o->pos_v.vz) * v.vz) / d ;
			//f -= 0.1f ;/* 重なり防止 */
			//if ( f < 0 )
			{
			    _sceVu0ScaleVector( &v, &v, f ) ;
			    _sceVu0AddVector( &o->pos_v, &o->pos_v, &v ) ;
			    _sceVu0SubVector( &p->pos_v, &p->pos_v, &v ) ;
			}
		    }
		}
}


