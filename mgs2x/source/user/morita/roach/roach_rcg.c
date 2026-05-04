/*
  rch_act.c
  ゴキブリ 行動関数

  2000/04/23 T. Morita
  $Id: roach_rcg.c,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "roach.h"

static float CheckDestanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}

float RCH_SqLength2D( FVECTOR *a, FVECTOR *b )
{
    float x = a->vx - b->vx ;
    float z = a->vz - b->vz ;

    return x*x + z*z ;
}

void RCH_Recognize( Work *work, Roach *r, int id )
{
    static void (*next)( Work *, Roach *, int ) ;

    if ( RCH_SqLength2D( &r->pos, &GM_PlayerControl->mov ) < RCH_DANGER_RAD * RCH_DANGER_RAD )
    {
	next = NULL ;
	if ( CheckDestanceLineAndPoint( (FVECTOR*)&GM_PlayerArmBody->objs->objs[4].world.m[W],
					(FVECTOR*)&GM_PlayerArmBody->objs->objs[4].world.m[Y],
					&r->pos ) < RCH_AIMED_RAD )
	    next = NULL ; //RCH_ActWowowFly ;
	else
	    next = NULL ; //RCH_ActWowowRun ;
	//if ( next && r->act==RCH_ActPause )
	    r->act = next ;
    }
}

