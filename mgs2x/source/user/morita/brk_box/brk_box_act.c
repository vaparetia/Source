/*
   brk_box_act.c
   じゃがいも壊れ アクト

   1999/12/13 T. Morita
   $Id: brk_box_act.c,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
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

#include "brk_box.h"


static void StickPartsOnFloor( BOX *p, FVECTOR *min )
{
    int         i ;
    DG_MDLPACK *o = p->objs->objs->packs ;
    SVECTOR    *s ;
    FVECTOR     f ;

    min->vy = 60000000.0f ;
    for ( i=o->n_verts, s=(SVECTOR*)o->verts ; --i>=0 ; s++ )
    {
	vu0_SV0toFV( s, &f ) ;
	_sceVu0ApplyMatrix( &f, &p->objs->world, &f ) ;
	if ( f.vy < min->vy )
	    *min = f ;
    }
}


int BRK_BOX_ActOnFoot( Work *work, BOX *p )
{
    FVECTOR prev ;

    prev = p->pos ;
    p->pos = *p->parts.foot ;
    _sceVu0SubVector( &p->pos_v, &p->pos, &prev ) ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.01f ) ;

    return 1 ;
    return 0 ;
}

int BRK_BOX_ActNone( Work *work, BOX *p )
{
    return 0 ;
}

int BRK_BOX_Act( Work *work, BOX *p )
{
    static FVECTOR g = { 0, -BRK_GRAVITY, 0, 0 } ; 
    FVECTOR v ;

    if ( !p->objs )
	return 0 ;

    _sceVu0AddVector( &p->pos_v, &p->pos_v, &g ) ;
    _sceVu0AddVector( &v, &p->pos  , &p->pos_v ) ;
    BRK_BOX_HzdCheck( work, p->seg, p->flr, &p->pos ) ;
    if ( BRK_BOX_HzdSegLineCheck( work, &p->seg[0], &p->pos, &v ) )
	BRK_BOX_HzdSegReactVector( &p->seg[0], &p->pos_v ) ;
    else
	_sceVu0CopyVector( &p->pos, &v ) ;
    p->rot_x += (p->rot_vx = p->rot_vx *(BRK_ROT_R-1) / BRK_ROT_R) ;
    p->rot_y += (p->rot_vy = p->rot_vy *(BRK_ROT_R-1) / BRK_ROT_R) ;

//printf( "pos(%f %f %f)  ", p->pos.vx,p->pos.vy,p->pos.vz ) ;
//printf( "vel(%f %f %f)\n", p->pos_v.vx,p->pos_v.vy,p->pos_v.vz ) ;

    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    StickPartsOnFloor( p, &v ) ;
    v.vy += p->pos.vy ;
    if ( v.vy < p->flr[0].p1.y )
    {
	p->pos.vy -= v.vy - p->flr[0].p1.y ;
	p->pos_v.vx = p->pos_v.vx *(BRK_VEL_R-1) / BRK_VEL_R ;
	p->pos_v.vz = p->pos_v.vz *(BRK_VEL_R-1) / BRK_VEL_R ;
	if ( (p->pos_v.vy *= -BRK_BOUNCE) < 2.0f )
	{
	    p->rot_x = (int)(p->pos.vx/p->target.size.vx) & 4095 ;
	    if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		p->act = BRK_BOX_ActNone ;
	    p->pos_v.vy = 0.0f ;
	}
	else
	    p->rot_vx *= -BRK_BOUNCE ;
    }
    TransMatrix( &p->objs->world, &p->pos ) ;
    GM_MoveTarget( &p->target, &p->pos ) ;

    return 1 ;
}

