/*
   brk_box_hzx.c
   じゃがいも壊れ 当たり判定用ルーチン

   1999/12/15 T. Morita
   $Id: brk_box_hzd.c,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_box.h"



void BRK_BOX_HzdCheck( Work *work, HZX_SEG *seg, HZX_FLR *flr, FVECTOR *pos )
{
    int atr[2] ;

    if ( ((pos->vx >= seg[0].p2.x || pos->vx <= seg[0].p1.x)  &&
	  (pos->vz >= seg[0].p2.z || pos->vz <= seg[0].p1.z)) ||
	 (seg[0].p1.y == seg[0].p1.h && seg[0].p2.y == seg[0].p2.h ) )
	if ( HZX_NearHazardCheck( work->hzx, pos,
				   BRK_HZX_SPHERE,
				   HZX_CHK_ALL,
				   HZX_SEG_NO_PLAYER,
				   BRK_HZX_SPHERE ) )
	    HZX_GetNearHazard( seg, atr ) ;

    if ( ((pos->vx >= flr[0].b2.x || pos->vx <= flr[0].b1.x)  &&
	  (pos->vz >= flr[0].b2.z || pos->vz <= flr[0].b1.z)) ||
	 (!flr[0].p1.h && !flr[0].p2.h && !flr[0].p3.h ) )
	if ( HZX_LevelHazardCheck( work->hzx, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	    HZX_GetLevelHazard( flr, atr ) ;
}

int BRK_BOX_HzdSegLineCheck( Work *work, HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    float px, pz, sx, sz, tx, tz ;
    float r, s, rd ;

    if ( seg->p1.y == seg->p1.h && seg->p2.y == seg->p2.h )
	return 0 ;

    px = p2->vx    - p1->vx    ; pz = p2->vz    - p1->vz    ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*pz - sz*px) == 0.0f )				 
	return 0 ;

    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if (  r > 1.0f || r < 0.0f )		 
	return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
	return  0 ;
    return 1 ;
}


int BRK_BOX_HzdBoxLineCheck( Work *work, BOX *b, FVECTOR *p1, FVECTOR *p2 )
{
    return 1 ;
}


void BRK_BOX_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;
}

void BRK_BOX_HzdPotatoCheck( Work *work, float rad )
{
    int     i, j ;
    float   f, d ;
    FVECTOR v ;
    BOX *p, *o ;

    for ( i=work->n_box, p=work->box ; --i>=0 ; p++ )
	for ( j=i, o=p+1 ; --j>=0 ; o++ )
	{
	    SubVector( &v, &p->pos, &o->pos ) ;
	    d = v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ;
	    f = p->target.size.vx + o->target.size.vx ;
	    if ( d <= f*f )
	    {
		if ( o->act != BRK_BOX_ActOnFoot )
		    o->act = BRK_BOX_Act ;
		if ( p->act != BRK_BOX_ActOnFoot )
		    p->act = BRK_BOX_Act ;

		f = ( (p->pos_v.vx - o->pos_v.vx) * v.vx +
		      (p->pos_v.vy - o->pos_v.vy) * v.vy + 
		      (p->pos_v.vz - o->pos_v.vz) * v.vz) / d ;
		f -= 0.1f ;/* 重なり防止 */

		_sceVu0ScaleVector( &v, &v, f ) ;
		_sceVu0AddVector( &o->pos_v, &o->pos_v, &v ) ;
		_sceVu0SubVector( &p->pos_v, &p->pos_v, &v ) ;
	    }
	}
}


