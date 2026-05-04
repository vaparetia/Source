//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  blast.c
  汎用爆発
   
  1999/12/02 M.Sonoyama
  2000/01/25 Revised by T.Morita
  $Id: fort_blast.c,v 1.1.1.3 2002/11/19 11:46:18 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

typedef	struct	{
    GV_ACT       actor   ;
    TARGET       target  ;
    TARGET       target2 ;
    POWER_TARGET damage  ;
    POWER_TARGET faint   ;
    int          flag    ;
} Work ;

extern void *NewBombEffect( FVECTOR *vec, int mode ) ;
extern void *NewExplosionControl( FVECTOR *pos, float size, int alpha ) ;


/* ターゲットセット */
static void SetTarget( TARGET *t, POWER_TARGET *p,
		       FVECTOR *vec, int side,
		       float range, float damage, float faint,
		       int wp )
{
    FVECTOR size, force ;

    size.vx = size.vy = size.vz = ( float )range / 2.0F ;
    force.vx = size.vx / 8.0F ;
    GM_SetTarget( t, TARGET_OFFENSE, 0, side, &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << wp ) ;
    GM_SetPowerTarget( t, p, (int)POWER_EXPLODE, 255, (int)faint, (int)damage, &force ) ;
    GM_MoveTargetMap( t, vec, GM_CurrentMap | GM_CurrentStageMap ) ;
}

static void Act( Work *work )
{
    if ( work->flag )
    {
	GM_PutTarget( &work->target  ) ;
	GM_PutTarget( &work->target2 ) ;

	work->flag = 0 ;
    }
    else
	GV_DestroyActor( work ) ;
}

void *NewFortBlast( FVECTOR *vec, int side,
		    float range1, float range2, float damage, float faint,
		    int wp, int mode )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, NULL ) ;

	/*ターゲットをセット*/
	SetTarget( &work->target , &work->damage, vec, side, range1, damage,     0, wp ) ;
	SetTarget( &work->target2, &work->faint , vec, side, range2, damage, faint, wp ) ;

	/*エフェクト*/
	if ( mode != 3 ){
	    NewBombEffect( vec, mode ) ;
	}else{
	    NewExplosionControl( vec, 1200.0f, 60 ) ;
#if 0 //BP_PS2 def PSX2 - If it's good enough for XBOX, it's good enough for us (and we're hurting performance!)
	    NewExplosionControl( vec, 2000.0f, 40 ) ;
#endif
	}

	/* 2種類の音ランダム */
	GM_SeSetMode( (GV_Time&1 ? SD_W_EXPLOS01 : SD_W_EXPLOS02), vec, GM_SEMODE_BOMB ) ;

	work->flag = 1 ;
    }
    return work ;    
}
