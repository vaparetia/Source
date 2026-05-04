//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_control.c
   コントロール君

   2000/12/22 T. Morita
   $Id: demo_control.c,v 1.1.1.3 2002/11/19 11:45:56 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "../../mode/demo/eft_con.h"
#include "../include/util.h"

typedef struct work_t
{
    GV_ACT_EX   actor;
	EFTCONTROL	eft_con;
    //CONTROL     control ;
} Work ;

static void Die( Work *work )
{
    //GM_FreeControl( &work->control ) ;
	DM_EftControlDelList( &work->eft_con );
}

#define DBG_CTRL 0

#if DBG_CTRL
static void Act( Work *work )
{
    FMATRIX world ;
    FVECTOR rot ;

    printf( "CONTROL[%x] pos(%.0f %.0f %.0f) rot(%.1f,%.1f,%.1f)\n",
	    work->eft_con.name, 
	    work->eft_con.mov.vx,
	    work->eft_con.mov.vy,
	    work->eft_con.mov.vz,
	    (work->eft_con.rot.vx * 180.0f)/2048.0f,
	    (work->eft_con.rot.vy * 180.0f)/2048.0f,
	    (work->eft_con.rot.vz * 180.0f)/2048.0f ) ;

    rot.vx = (short)((work->eft_con.rot.vx * M_PI)/2048.0f) ;
    rot.vy = (short)((work->eft_con.rot.vy * M_PI)/2048.0f) ;
    rot.vz = (short)((work->eft_con.rot.vz * M_PI)/2048.0f) ;

    _sceVu0RotMatrixX( &world, &DG_UnitMatrix,rot.vx ) ;
    _sceVu0RotMatrixY( &world, &world,        rot.vy ) ;
    _sceVu0RotMatrixZ( &world, &world,        rot.vz ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)world.m[3], &work->eft_con.mov ) ;

    HZX_ViewMatrix( &world, 100.0f ) ;
}
#endif

static inline void GetResources( Work *work, int name, int where )
{
    //GM_InitControlEx( &work->control, name, where, 0 ) ;

printf( "NewControlBoy%x %x\n", name, &work->eft_con ) ;

	DM_EftControlAddList( name, &work->eft_con );
	work->eft_con.mov.vw = 1.0f;

printf( "NewControlBoy End\n" ) ;
}

void *NewControlBoy( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
#if DBG_CTRL
        GV_SetActor( &work->actor, Act, Die ) ;
#else
        GV_SetActor( &work->actor, NULL, Die ) ;
#endif
	GV_ActorEX( &work->actor ) ;
        GetResources( work, name, where ) ;
    }
    return work ;
}
