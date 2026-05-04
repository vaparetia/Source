//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_search.c 
   投光器

   1999/12/26 T.Morita
   $Id: brk_search.c,v 1.1.1.3 2002/11/19 11:45:45 Yoshizawa1 Exp $
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
#include "../include/util.h"
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"

#define ORGA_N_VITAL 1

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


typedef struct lgtwork_t
{
    GV_ACT       actor  ;

    DG_DEF      *off    ;
    DG_DEF      *on     ;
    DG_OBJS     *objs   ;

    int         name    ;
    int         flag    ;
    int         bg_name ;
    int         lt_name ;
    int         pr_name ;

    LIT_DEF     *lit    ;

    int          map    ;
    int          vital  ;
    TARGET       target ;
    POWER_TARGET power  ;

    void        *rain   ;
} Work ;

extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot ) ;       /* okajima/effect/lit_man.c   */
extern void OK_RemoveDynamicLight( FVECTOR *pos );                   /* okajima/effect/lit_man.c   */
extern void *NewSpotRain() ;




static void Die( Work *work )
{
    OK_RemoveDynamicLight( (FVECTOR*)&work->objs->world.m[3] )  ;
    if ( work->objs )
    {
	DG_DequeueObjs( work->objs ) ;
	DG_FreePreshade( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
}

static void ORG_LGT_OffSpotLightMessage( int name )
{
    GV_MSG msg ;
    int buffer = 0 ;/*静止スポットライトを消すメッセージID*/

    msg.address = name    ;
    msg.message = &buffer ;
    msg.message_len = 1   ;
    GV_SendMessage( &msg ) ;
}

#if 0
static void ORG_LGT_OnSpotLightMessage( int name )
{
    GV_MSG msg ;
    int buffer = 1 ;/*静止スポットライトを消すメッセージID*/

    msg.address = name    ;
    msg.message = &buffer ;
    msg.message_len = 1   ;
    GV_SendMessage( &msg ) ;
}
#endif

static void ORG_LGT_OffSpotRainMessage( int name )
{
    GV_MSG msg ;
    int buffer = 0 ;/*スポット雨を消すメッセージID*/

    msg.address = name    ;
    msg.message = &buffer ;
    msg.message_len = 1   ;
    GV_SendMessage( &msg ) ;
}


static void Act( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( !msg->message[0] )
            *((int **)msg->message[1]) = &work->flag ;
}

static void ORG_LGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work      *work = (Work*)ptr ;
    FVECTOR    p = { -19500, 14500, -15000, 0 } ;
    extern void *NewBreakLight( FVECTOR *pos, FVECTOR *dir, int where ) ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

#if DEBUG_MODE
	printf( "NewPutSearchLightObject : Center%f %f %f\n",
		work->target.center.vx,
		work->target.center.vy,
		work->target.center.vz ) ;
#endif

	/* 既に壊れているか 銃の弾以外は壊れない */
	if ( !(off->weapon_type & (WP_BULLET|WP_M92)) || work->vital <= 0 )
	    return ;
	BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	if ( --work->vital > 0 )
	{
	    GM_SetNoise( NOISE_S, &work->target.center, work->map ) ;
	    GM_SeSetMode( SD_A_LHIBI01, &work->target.center, GM_SEMODE_BOMB ) ;
	}
	else
	{
	    DG_OBJS *objs = work->objs ;

	    DG_DestroyLightSphere( &work->target.center, 4000.f ) ;

	    if ( (work->objs = DG_MakeObjs( work->off, DG_FLAG_ONEPIECE| DG_FLAG_PAINT, 0 )) )
	    {
		GM_SeSetMode( SD_A_LLIGHT01, &work->target.center, GM_SEMODE_BOMB ) ;
		GM_SetNoise( NOISE_MM, &work->target.center, work->map ) ;

		ORG_LGT_OffSpotLightMessage( work->lt_name ) ;
		ORG_LGT_OffSpotRainMessage( work->lt_name ) ;

		if ( work->pr_name )
		    GCL_ExecProc( work->pr_name, NULL ) ;

		work->objs->world =  objs->world ;
		DG_QueueObjs( work->objs ) ;
		DG_MakePreshade( work->objs, work->lit ) ;

		DG_DequeueObjs( objs ) ;
		DG_FreeObjs( objs ) ;

		NewBreakLight( &p, &p, work->map ) ;
		p.vz +=  50.0f ;
		p.vy += 100.0f ;
		NewBreakLight( &p, &p, work->map ) ;
		p.vz += 100.0f ;
		NewBreakLight( &p, &p, work->map ) ;
	    }
	}
    }
}

static inline int ORG_LGT_InitTarget( Work *work, int map ) 
{
    FVECTOR t_size, t_pos ;
    FVECTOR uv = { work->off->ux, work->off->uy, work->off->uz, 0 } ;
    FVECTOR lv = { work->off->lx, work->off->ly, work->off->lz, 0 } ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;

    _sceVu0ApplyMatrix( &uv, &work->objs->world, &uv ) ;
    _sceVu0ApplyMatrix( &lv, &work->objs->world, &lv ) ;
    t_size.vx = fpu_Abs(uv.vx - lv.vx)*0.5f ;
    t_size.vy = fpu_Abs(uv.vy - lv.vy)*0.5f ;
    t_size.vz = fpu_Abs(uv.vz - lv.vz)*0.5f ;
    _sceVu0AddVector  ( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    t_pos.vx += work->off->tx ;
    t_pos.vy += work->off->ty ;
    t_pos.vz += work->off->tz ;
    _sceVu0AddVector  ( &t_pos, &t_pos, (FVECTOR *)&work->objs->world.m[W] ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH, map,
		  BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, ORG_LGT_TargetCallBack, work ) ;
    GM_MoveTarget( t, &t_pos ) ;
    GM_PutTarget( t ) ;
    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    int     i, buf[3] ;
    SVECTOR rot = { 0, 0, 0, 0 } ;

    if ( (i=GCL_GetOptionValue( 'l', 0 )) )
	work->lit = GV_GetCache( GV_CacheID( i, 'l') ) ;
    else
	return -1 ;

    if ( GCL_GetOption( 'm' ) )
    {
	work->on  = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') ) ;
	work->off = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') ) ;

	work->objs = DG_MakeObjs( work->on, DG_FLAG_ONEPIECE| DG_FLAG_PAINT, 0 ) ;
	DG_QueueObjs( work->objs ) ;
	DG_MakePreshade( work->objs, work->lit ) ;
    }
    else
	return -1 ;

    if ( GCL_GetOption( 'n' ) )
    {
	work->bg_name = GCL_GetNextInt() ;
	work->lt_name = GCL_GetNextInt() ;
    }
    else
	return -1 ;
    work->pr_name = GCL_GetOptionValue( 'b', 0 ) ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&work->objs->world.m[3] ) ;
	work->objs->world.m[3][3] = 1.0f ;
    }

    OK_SetDynamicLight( (FVECTOR*)&work->objs->world.m[3], &rot ) ;

    ORG_LGT_InitTarget( work, where ) ;
    work->vital = ORGA_N_VITAL ;
    work->name  = name  ;
    work->map   = where ;
    work->flag  = 0     ; 
    //work->rain  = NewSpotRain() ;

    return 0 ;
}


void *NewPutSearchLightObject( int name, int where )
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
