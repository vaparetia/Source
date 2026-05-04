//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_dbg.c

  フォーチュンデバック
  2001/05/08 Masafumi Okuta
  $Id: emma_dbg.c,v 1.1.1.3 2002/11/19 11:45:59 Yoshizawa1 Exp $
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
#include "debugmenu.h"	

#include	"include/emma.h"

extern void	*NewTargetView2( TARGET			*targ, u_char r, u_char g, u_char b );

int EMA_DbgInvincible = 0 ;
int EMA_DbgStatus     = 0 ;
int EMA_DbgEvent      = 0 ;

static char *DbgInvisibleItems [] = { "0", "1", "2" } ;
static int   DbgInvisibleValues[] = {  0,   1,   2  } ;
static GM_DEBUG_MENU DbgInvisible = 
{
	NULL,
    "EMMA",
    "MUTEKI-MODE",
	DbgInvisibleItems ,
	DbgInvisibleValues,
    (int*)&EMA_DbgInvincible,
	0x00000003,	/* mask*/   
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	3, /*max*/
	0,0

};

static char *DbgStatusItems [] = { "OFF", "ON" } ;  /*  items*/
static int   DbgStatusValues[] = {  0,   1,  } ;   /* values*/
static GM_DEBUG_MENU DbgStatus = 
{
	NULL,
	"EMMA",        /* class*/  
	"STATUS",    /*menu*/   
	DbgStatusItems ,
	DbgStatusValues,
	(int*)&EMA_DbgStatus,  /*target*/ 
	0x00000001,	/* mask*/   
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	2, /*max*/
	0,0
};

static char *DbgEventItems [] = { "OFF", "TARGET" } ;
static int   DbgEventValues[] = {  0,   1,  } ;
static GM_DEBUG_MENU DbgEvent = 
{
	NULL,
    "EMMA",
    "EVENT",
	DbgEventItems ,
	DbgEventValues,
    (int*)&EMA_DbgEvent,
	0x00000001,	/* mask*/   
	NULL, /*func*/
	0, /*strid*/
	0, /*types */
	2, /*max*/
	0,0
};


int MatToEularType = 0 ;


void EMA_InitDbgConfiguration()
{
    GM_AddDebugMenu( &DbgInvisible ) ;
    GM_AddDebugMenu( &DbgStatus ) ;
    GM_AddDebugMenu( &DbgEvent ) ;
}

void EMA_DbgDispEvent( Work *work )
{
    int i ;

    switch( EMA_DbgEvent )
    {
    case 1:
	NewTargetView2( &work->deftrg, 200, 32, 151 ) ;
	for ( i=EMA_TARGET_CHILD_NUM ; --i>=0 ; )
	    NewTargetView2( &work->def_child[i], 200, 50, 32 ) ;
    }
	
}

void EMA_DbgDispStatus( Work *work )
{
    extern void AN_Test_Eye2( FVECTOR *pos, int size ) ;

    if ( EMA_DbgStatus )
    {
	DEBUG_Locate( 260, 12, 0 ) ;
	DEBUG_Printf( "LIFE %d FLG %08x\n", work->npc.action.life, work->flag ) ;

	DEBUG_Printf( "MOTION %d ID%d\n", work->npc.action.current_mot ) ;
	DEBUG_Printf( "NAVI %04x (%.0f %.0f)\n", work->navitrg.addr,
		      work->navitrg.pos.vx, work->navitrg.pos.vz ) ;

	AN_Test_Eye2( &work->navitrg.pos, 2 ) ;

	if ( work->route )
	    DEBUG_Printf( "ROUTE flag%d act%d time%d",
			  work->route->flag, work->route->act, work->route->time ) ;

    }
}
