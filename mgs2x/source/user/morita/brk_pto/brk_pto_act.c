//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_act.c
   じゃがいも壊れ アクト

   1999/12/13 T. Morita
   2000/10/16 1.20 T.Morita 
   $Id: brk_pto_act.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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

#include "brk_potato.h"
#include "../brk_utl/brk_utl.x"



FVECTOR BRK_PTO_Bounce  = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
FVECTOR BRK_PTO_Size[]  = {
    { BRK_HZX_SPHERE, BRK_HZX_SPHERE, BRK_HZX_SPHERE, 0 },
    { 70.0f         , 70.0f         , 70.0f         , 0 },
} ;
FVECTOR BRK_PTO_PartSize[] = {
    { 25.0f, 25.0f, 25.0f, 0 },
    { 30.0f, 30.0f, 30.0f, 0 },
} ;

int BRK_PTO_ActPartBreaking( Work *work, PART *p )
{
    p->rot_x += p->rot_vx ;
    p->rot_y += p->rot_vy ;
    p->pos_v.vy -= BRK_GRAVITY ;
    RotateMatrixXY( &p->obj->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;

    switch( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			     &BRK_PTO_Bounce, &BRK_PTO_PartSize[work->type] ) )
    {
    case 3:
    case 1:
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
	if ( p->pos_v.vy < BRK_GRAVITY && !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	    p->act = NULL ;
	p->rot_vx = (short)(p->rot_vx * (BRK_PROT_R-1) / BRK_PROT_R * (-BRK_BOUNCE)) ;
	p->rot_vy = (short)(p->rot_vy * (BRK_PROT_R-1) / BRK_PROT_R                ) ;
	break ;
    case 2:
	break ;
    case 0:
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( &p->obj->world, &p->pos ) ;

    return 1 ;
}




int BRK_PTO_ActOnFoot( Work *work, POTATO *p )
{
    _sceVu0SubVector( &p->pos_v, &p->pos, p->parts.foot ) ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, 0.001f ) ;
    p->pos = *p->parts.foot ;

    return BRK_PTO_HzdPotatoOneCheck( work, p, 200.0f ) ;
}

int BRK_PTO_ActNone( Work *work, POTATO *p )
{
    return 0 ;
}

int BRK_PTO_ActBreak( Work *work, POTATO *p )
{
    int   i ;
    PART *part = p->parts.prof ;
    int flag = 0 ;

    for ( i=p->n_parts ; --i>=0 ; part++ )
	/*for ( i=1 ; --i>=0 ; part++ )*/
	if ( part->act )
	    flag |= (*part->act)( work, part ) ;
    if ( !flag )
    {
	p->act = NULL ;
	p->n_parts = 0 ;
	GV_Free( p->parts.prof ) ;
	return 0 ;
    }
    return 1 ;
}



int BRK_PTO_Act( Work *work, POTATO *p )
{
    FVECTOR v ;

    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;

    p->pos_v.vy -= BRK_GRAVITY ;
    switch( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			     &BRK_PTO_Bounce, &BRK_PTO_Size[work->type] ) )
    {
    case 1:
    case 3:
	/*箱内の外に出るようにする*/
	if ( BRK_HZD_Hazard == work->box_hzd[0] ||
	     BRK_HZD_Hazard == work->box_hzd[1] ||
	     BRK_HZD_Hazard == work->box_hzd[2] ||
	     BRK_HZD_Hazard == work->box_hzd[3] ||
	     BRK_HZD_Hazard == work->box_hzd[4] )
	{
	    p->pos_v.vy += 10.0f ;
	    _sceVu0ScaleVector( &v,
				(FVECTOR*)work->box.objs->world.m[Y],
				10.0f+rnd()*5.0f ) ;
	    _sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
	}

	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;

	if ( p->n_bounce > 0 )
	    if ( p->pos_v.vy < BRK_GRAVITY*2 )
		p->pos_v.vy = 0.0f ;
	if ( p->pos_v.vy < BRK_GRAVITY )
	{
	    p->n_bounce = 3 ;
	    p->rot_vx = (short)(p->pos_v.vx * 40.0f) ;
	    p->rot_vy = (short)(p->pos_v.vz * 40.0f) ;
	    if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		if ( !BRK_UTL_PutCenterHazard( work->hzd,
					       &p->pos, &p->pos_v,
					       1000.0f, 25.0f ) )
		    p->act = BRK_PTO_ActNone ;
	}
	else
	    p->rot_vx *= -(short)(BRK_BOUNCE-1.0f) ;

	break ;
    case 0:
	if ( p->n_bounce > 0 )
	    p->n_bounce-- ;
	p->rot_x += (p->rot_vx = p->rot_vx * (BRK_ROT_R-1) / BRK_ROT_R) ;
	p->rot_y += (p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R) ;
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    p->parts.flag = 0 ; /* 取り敢えず すべて衝突無効 */
    TransMatrix( &p->objs->world, &p->pos ) ;
    GM_MoveTargetMap( &p->target, &p->pos, work->where ) ;

    return 1 ;
}
