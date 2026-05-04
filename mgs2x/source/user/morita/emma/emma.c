//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma.c
  エマ

  2001/02/08 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma.c,v 1.1.1.3 2002/11/19 11:45:59 Yoshizawa1 Exp $
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
#include	"include/emma.h"

static void Act( Work *work )
{
    EMA_Message( work ) ;

    if ( !EMA_Flag( EMA_F_NON_ACTION ) )
    {
        NPCWORK *npc = &work->npc ;

        /* ＮＰＣシステム系コントロール処理 */
        EMA_ActControl( work ) ;
        /* ＮＰＣ前処理 */
        NPC_PreProcess( npc ) ;

        EMA_Recognize( work ) ; /* 認知処理 */
        EMA_Think( work ) ;     /* 思考処理 */
        EMA_Action( work ) ;    /* 行動処理 */

        /* ＮＰＣ後処理 */
        NPC_AfterProcess( npc ) ;
    }
    else
        EMA_Sleep( work ) ;     /* 行動停止中の処理 */
}

static void Die( Work *work )
{
    /* 映り込みモデルを解放 */
    EMA_FreeMirrorObj( work->hair_obj ) ;
    EMA_FreeMirrorObj( work->glass_obj ) ;

    /* IKを解放 */
    EMA_FreePuppetIK( work->arm_ik ) ;
    /* NPCを解放 */
    NPC_FreeResources( &work->npc ) ;
    /* ゲージを解放 */
    GM_RemoveGageSet( &work->gage ) ;
    /* ワーク初期化 */
    EMA_FreeCommandWork() ;

    /* 顔アニメのワークを解放 */
    EMA_FreeFaceAnimation( work->face_h ) ;
}

static int GetResources( Work *work, int name, int where )
{
    /* ワーク初期化 */
    EMA_InitCommandWork( work ) ;

    EMA_InitFlags( work, name, where ) ;
    EMA_InitNPC( work, name, where ) ;
    EMA_InitControl( work, name, where ) ;
    EMA_InitTarget( work, name, where ) ;
    EMA_InitPosition( work, name, where ) ;
    EMA_InitParams( work, name, where ) ;
    EMA_InitInfoDisp( work, name, where ) ;
    EMA_InitFaceAnime( work, name, where ) ;
    EMA_InitRoute( work, name, where ) ;
    EMA_InitVoice( work, name, where ) ;

    EMA_InitEquipment( work, name, where ) ;
    EMA_InitHoming( work, name, where ) ;

    EMA_EffectStart( work, where ) ;

#if DEBUG_MODE
    EMA_InitDbgConfiguration() ;/* emma_dbg.c */
#endif

    return 0 ;
}


void *NewEmma( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
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

void *NewEma( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) 
    {
	GV_SetActor( &work->actor, /*Act*/NULL, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, where ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
