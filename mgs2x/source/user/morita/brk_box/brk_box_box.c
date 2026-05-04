/*
   brk_box_box.c
   じゃがいも壊れの箱

   2000/01/15 T. Morita
   $Id: brk_box_box.c,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
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



static void StickPartsOnFloor( BOX *b, FVECTOR *min )
{
    int         i ;
    DG_MDLPACK *o = b->def->models->packs ;
    SVECTOR    *s ;
    FVECTOR     f ;

    min->vy = 60000000.0f ;
    for ( i=o->n_verts, s=(SVECTOR*)o->verts ; --i>=0 ; s++ )
    {
	vu0_SV0toFV( s, &f ) ;
	_sceVu0ApplyMatrix( &f, &b->objs->world, &f ) ;
	if ( f.vy < min->vy )
	    *min = f ;
    }
}

void BRK_BOX_ActBox( Work *work )
{
    FVECTOR v ;
    BOX    *b = &work->box ;

    if ( b->rot_z/1024 )
	b->rot_z /= 1024, b->rot_z *= 1024 ;
    else if ( b->rot_z/512 )
	b->rot_z += b->rot_vz += b->rot_vz/8 ;
    else
    {
	if ( b->rot_z/25 || b->rot_vz/5 )
	    b->rot_z += b->rot_vz -= (b->rot_vz + b->rot_z)/8 ;
	else
	    b->rot_z = 0 ;

	if ( b->rot_x/25 || b->rot_vx/5 )
	    b->rot_x += b->rot_vx -= (b->rot_vx + b->rot_x)/8 ;
	else
	    b->rot_x = 0 ;
    }
    b->pos.vy -= 50.0f ;
    RotateMatrixXZ( &b->objs->world, &DG_UnitMatrix, b->rot_x, b->rot_z ) ;
    StickPartsOnFloor( b, &v ) ;
    if ( (v.vy += b->pos.vy) < b->floor )
	b->pos.vy -= v.vy - b->floor ;
    TransMatrix( &b->objs->world, &b->pos ) ;
}
