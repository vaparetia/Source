//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp.c
  ヴァンプ（狙撃イベント）

  2001/03/23 T.Morita
  $Id: vamp.c,v 1.1.1.3 2002/11/19 11:46:34 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#define __MAIN_FILE__
#include	"include/vamp.h"

static void Act( Work *work )
{
    VMPS_Message( work ) ;

    if ( !VMPS_Flag( VMPS_F_NON_ACTION ) )
    {
        NPCWORK *npc = &work->npc ;
	
        /* コントロール処理 */
        VMPS_ActControl( work ) ;
        /* ＮＰＣ前処理 */
        NPC_PreProcess( npc ) ;

        VMPS_Recognize( work ) ; /* 認知処理 */
        VMPS_Think( work ) ;     /* 思考処理 */
        VMPS_Action( work ) ;    /* 行動処理 */
        VMPS_Display( work ) ;   /* 表示処理 */

        /* ＮＰＣ後処理 */
        NPC_AfterProcess( npc ) ;
    }
}

static void Die( Work *work )
{
    /* NPCを解放 */
    NPC_FreeResources( &work->npc ) ;
    /* ゲージを解放 */
    GM_RemoveGageSet( &work->gage ) ;
    /* エマ専用床をはずす */
    HZX_RemoveDynamicFloor( work->d_floor ) ;

    VMPS_Work = NULL ;
}

static int GetResources( Work *work, int name, int where )
{
    VMPS_Work = work ;

    VMPS_InitNPC( work, name, where ) ;
    VMPS_InitControl( work, name, where ) ;
    VMPS_InitTarget( work, name, where ) ;
    VMPS_InitPosition( work, name, where ) ;
    VMPS_InitParams( work, name, where ) ;
    VMPS_InitInfoDisp( work, name, where ) ;
    VMPS_InitFaceAnime( work, name, where ) ;
    VMPS_InitDynamicFloor( work, name, where ) ;
    VMPS_InitHomingTarget( work, name, where ) ;

    return 0 ;
}


void *NewSniperVamp( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x08 ) ;
    //work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) 
    {
	GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, where ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
