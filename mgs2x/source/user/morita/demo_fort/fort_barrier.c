//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_barrier.c
   デモ専用 弾除けキャラ

   2000/10/03 T. Morita
   $Id: fort_barrier.c,v 1.1.1.3 2002/11/19 11:45:56 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "../include/util.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT_EX actor ;

    TARGET       target ;
    POWER_TARGET power  ;

    float     front    ;
    float     vertical ;
    float     length   ;

    DG_OBJS  *body   ;
    int       where  ;
} Work ;

#define BODYPOS(a,b)   ((FVECTOR*)&((a)->objs[(b)].world.m[W]))


extern void *NewFortSplineBulletDemo( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid,
				      float front, float vertical ) ;

static void Die( Work *work )
{
    GM_FreeTarget( &work->target ) ;
}


static void Act( Work *work )
{
    GM_MoveTarget2Map( &work->target, &work->body->world, work->where ) ;
}


void FRT_BAR_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    FVECTOR shoot ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	/* 弾避けカットが呼ばれる */
        if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    /* 表示する弾の位置を計算 */
	    _sceVu0Normalize( &shoot, &off->power->force ) ;
	    _sceVu0ScaleVector( &shoot, &shoot, work->length ) ;
	    _sceVu0SubVector( &shoot, &def->hit, &shoot ) ;

	    NewFortSplineBulletDemo( &shoot,
				       &off->power->force,
				       (FVECTOR *)work->body->objs[HUMAN21_KOSHI].world.m[W],
				       work->front, work->vertical ) ;
	}
    }
}


int GetResources( Work *work, DG_OBJS *body, float length, float size, int where,
		  float front, float vertical, int flag )
{
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR t_size = { size,size,size, 0.0f } ;

    if ( !body )
	PERROR( "ERROR !!!!! No link, please link : NewFortBarrierDemo\n" ) ;

    work->body     = body     ;
    work->where    = where    ;
    work->length   = length   ;
    work->front    = front    ;
    work->vertical = vertical ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER, where, BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, FRT_BAR_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    GM_MoveTarget2Map( &work->target, &work->body->world, work->where ) ;

#if DEBUG_MODE
    if ( flag )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}


void *NewFortBarrierDemo( DG_OBJS *body,
			  float length, float front, float vertical, 
			  float size, int where, int flag ) 
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, body, length, size, where, front, vertical, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
