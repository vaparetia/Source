//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tre_act.c
   植物揺れ アクト

   1999/12/17 T. Morita
   $Id: brk_tre_act.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "brk_tree.h"


#if 0
void BRK_TRE_MakePosition( Work *work, PART *p, FVECTOR *pos )
{
    FVECTOR v ;
    DG_MDL *m = p->mdl ;
    int i ;

    *pos = DG_ZeroVector ;
    for ( i=m->parent ; i!=-1 ; i=m->parent )
    {
	v.vx = m->tx ;
	v.vy = m->ty ;
	v.vz = m->tz ;
	v.vw = 0.0f ;
        _sceVu0ApplyMatrix( &v, &work->objs->objs[i].world, &v ) ;
        _sceVu0AddVector( pos, pos, &v ) ;
        m = &work->objs->def->models[i] ;
    }
    _sceVu0AddVector( pos, pos, (FVECTOR*)&work->objs->world.m[3] ) ;
}
#else
void BRK_TRE_MakePosition( Work *work, PART *p )
{
    DG_OBJ  *o = p->obj ;
    FMATRIX *mtx ;

    TransMatrix( mtx = &o->world, (FVECTOR *)&o->trans ) ;
    if ( o->parent == -1 )
	_sceVu0MulMatrix( mtx, &work->objs->world                , mtx ) ;
    else
	_sceVu0MulMatrix( mtx, &work->objs->objs[o->parent].world, mtx ) ;

}
#endif


int BRK_TRE_ActShaking( Work *work, PART *p )
{
    if ( p->rot_x/work->tend || p->rot_vx )
	p->rot_x += p->rot_vx -= (p->rot_vx + p->rot_x)/work->tend ;
    else
	p->rot_x += p->rot_x>0 ? -1 : 1 ;
    if ( p->rot_z/work->tend || p->rot_vz )
	p->rot_z += p->rot_vz -= (p->rot_vz + p->rot_z)/work->tend ;
    else
	p->rot_z += p->rot_z>0 ? -1 : 1 ;

    if ( !(p->rot_x/2) && !(p->rot_z/2) && !p->rot_vx && !p->rot_vz )
	p->act = BRK_TRE_ActFinished ;
    RotateMatrixXZ( &p->obj->world, &DG_UnitMatrix, p->rot_x, p->rot_z ) ;
    BRK_TRE_MakePosition( work, p ) ;

    return 1 ;
}

int BRK_TRE_ActFinished( Work *work, PART *p )
{
    return 0 ;
}
