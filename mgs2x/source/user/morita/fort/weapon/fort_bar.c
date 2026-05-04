//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_bar.c
   銃バリア（スネーク出し）

   2000/06/07 T.Morita
   $Id: fort_bar.c,v 1.1.1.3 2002/11/19 11:46:18 Yoshizawa1 Exp $
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
// for DG-LIB
#include "../../include/libdg_x.h"


#include "fort_wep.h"
#include "../include/fort_se.h"

static const FVECTOR FRT_BarriarPos = { -12000.0f, 14600.0f, -18674.0f, 1.0f } ;
static const FVECTOR FRT_DeckPos    = { -11500.0f, 12750.0f, -17500.0f, 1.0f } ;
typedef struct work_bar_t
{
    GV_ACT   actor  ;

    int      mode   ;
    FVECTOR  pos    ;
    FVECTOR *fort_pos ;
    int      tics   ;
    int      shoot  ;
} Work ;

static void Act( Work *work )
{
    extern void *NewBullet( FMATRIX *world, u_int type, u_int side, u_int size,
			    u_int damage, u_int length, u_int speed, int weapon ) ;
    extern void *NewFortBullet( FMATRIX *world, u_int type, u_int side, u_int size,
			    u_int damage, u_int length, u_int speed, int weapon ) ;
    FMATRIX dir ;
    FVECTOR pos ;

    if ( work->tics == 170 || work->tics == 164 || work->tics == 160 ||
	 work->tics ==  80 || work->tics ==  73 || work->tics ==  40 )
    {
	dir = DG_UnitMatrix ;
	_sceVu0CopyVector( &pos, &FRT_DeckPos ) ;
	pos.vx += 300.0f*frnd() ;
	pos.vz += 300.0f*frnd() ;
	_sceVu0SubVector( (FVECTOR*)&dir.m[Y], work->fort_pos, &pos ) ;
	dir.m[Y][W] = 0.0f ;
	_sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y]  ) ;
	dir.m[W][X] = pos.vx + dir.m[Y][X]*50.0f ;
	dir.m[W][Y] = pos.vy + dir.m[Y][Y]*50.0f ;
	dir.m[W][Z] = pos.vz + dir.m[Y][Z]*50.0f ;

	NewBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR, PLAYER_SIDE, 25, 5, 100, 50, WP_Usp ) ;
    }

    if ( work->tics <= 180 && !work->shoot )
    {
	if ( PL_PlayerResetInvincible() )
	{
	    work->shoot = 1 ;
	    dir = DG_UnitMatrix ;
	    _sceVu0CopyVector( (FVECTOR*)&dir.m[W], &FRT_BarriarPos ) ;
	    _sceVu0SubVector( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[W], &GM_PlayerControl->mov ) ;
	    dir.m[Y][W] = 0.0f ;
	    _sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y] ) ;
	    NewBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR, PLAYER_SIDE, 25, 5, 4000, 100, WP_Punch ) ;
	    if ( !work->mode )
	    {
		GV_DestroyActor( work ) ;
		return ;
	    }
	}
    }
    else if ( GM_PlayerStatus & PLAYER_DOWNED )
	work->pos = GM_PlayerControl->mov ;

    if ( work->tics )
	work->tics-- ;
    else if ( work->pos.vz > GM_PlayerControl->mov.vz + 50.0f && !(GM_PlayerStatus & PLAYER_DOWNED) )
	if ( work->mode )
	    work->shoot = 0, work->tics = 180 ;
}

static void Die( Work *work )
{
}

static int GetResources( Work *work, int mode, FVECTOR *fort_pos )
{
    work->mode = mode ;
    work->tics = mode ? 180 : 0 ;
    work->fort_pos = fort_pos ;
    work->shoot    = 0    ;
    return  0 ;
}

void *NewFortWeaponBarriar( int mode, FVECTOR *fort_pos )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, mode, fort_pos ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
