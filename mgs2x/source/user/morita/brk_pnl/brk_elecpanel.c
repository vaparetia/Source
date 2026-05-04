//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spotlgt.c 
   プットライトオブジェ

   1999/12/26 T.Morita
   $Id: brk_elecpanel.c,v 1.1.1.3 2002/11/19 11:45:41 Yoshizawa1 Exp $
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

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#define  MAX_N_TARGET 2 

typedef struct work_t
{
    GV_ACT       actor  ;

    int          where  ;
    int          name   ;
    int          proc   ;
    int          break_tic ;

    TARGET       def_t[MAX_N_TARGET] ;
    POWER_TARGET def_p[MAX_N_TARGET] ;
    TARGET       off_t[MAX_N_TARGET] ;
    POWER_TARGET off_p[MAX_N_TARGET] ;
} Work ;


static void BRK_PNL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work*)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_WEAPONCORE|WP_BLAST) )
	{
	    work->break_tic = 8 ;

	    GM_FreeTarget( &work->def_t[0] ) ;
	    GM_FreeTarget( &work->def_t[1] ) ;
	}
    }
}

static void Act( Work *work )
{
    if ( work->break_tic >=0 )
	if ( !GM_IsGameOver() )
	{
	    if ( GM_GameLevel >= GM_LEVEL_HARD )
	    {
		GM_PutTarget( &work->off_t[0] ) ;
		GM_PutTarget( &work->off_t[1] ) ;
	    }
	    if ( --work->break_tic == 0 )
		if ( work->proc )
		    GCL_ExecProc( work->proc, NULL ) ;
	}
}

static void Die( Work *work )
{
    GM_FreeTarget( &work->def_t[0] ) ;
    GM_FreeTarget( &work->def_t[1] ) ;
}

static void InitDefTarget( Work *work, int i, FVECTOR *lv, FVECTOR *uv )
{
    FVECTOR t_size, t_pos ;

    _sceVu0SubVector( &t_size, lv, uv ) ;
    t_size.vx = fpu_Abs( t_size.vx ) ;
    t_size.vy = fpu_Abs( t_size.vy ) ;
    t_size.vz = fpu_Abs( t_size.vz ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, lv, uv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;

    /* 防御ターゲット */
    GM_SetTarget( &work->def_t[i],
		  TARGET_DEFENSE|TARGET_POWER|TARGET_CALL_CALLBACK_THROUGH_HIT,
		  work->where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( &work->def_t[i], &work->def_p[i],
		       POWER_ONCE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->def_t[i], BRK_PNL_TargetCallBack, work ) ;
    GM_PutTarget( &work->def_t[i] ) ;

    /* エクストリーム用攻撃ターゲット */
    t_size.vx += 1000.0f ;
    t_size.vz += 1000.0f ;
    GM_SetTarget( &work->off_t[i], TARGET_OFFENSE|TARGET_POWER,
		  work->where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetTargetWeaponType( &work->off_t[i], WP_NIKITA ) ;
    GM_SetPowerTarget( &work->off_t[i], &work->off_p[i],
		       POWER_ONCE, GM_Vitality,
		       1000, 1000, &DG_ZeroVector ) ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    FVECTOR lv, uv ;
    int     i, opt ;

    work->name    = name  ;
    work->where   = where ;
    work->break_tic = -1  ;

    /* 防御ターゲット */
    opt = 1 ;
    for ( i=0 ; i<MAX_N_TARGET && opt ; i++ )
	switch( (opt = GCL_GetNextOption()) )
	{
	case 't':
	    GCL_GetNextIV( (int *)&buf ) ;
	    vu0_IV0toFV( &buf, &lv ) ;
	    GCL_GetNextIV( (int *)&buf ) ;
	    vu0_IV0toFV( &buf, &uv ) ;
	    InitDefTarget( work, i, &lv, &uv ) ;
	}

    work->proc = GCL_GetOptionValue( 'p', 0 ) ;

#ifdef DEBUG_MODE
    if ( GCL_GetOptionValue( 'f', 0 ) )
	for ( ; i>=0 ; i-- )
	{
	    NewTargetView( &work->off_t[i],  50, 32, 200 ) ;
	    NewTargetView( &work->def_t[i],  200, 50, 32 ) ;
	}
#endif


    return 0 ;
}

void *NewTargetProc( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
