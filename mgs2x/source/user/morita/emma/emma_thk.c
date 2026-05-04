//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_thk.c
  エマ思考処理

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_thk.c,v 1.1.1.3 2002/11/19 11:46:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include	"include/emma.h"

#include "think/thk_comein.h"
#include "think/thk_rai_idle.h"
#include "think/thk_rai_move.h"
#include "think/thk_afraid.h"
#include "think/thk_slater.h"

#include "think/thk_sniper.h"
#include "think/thk_player.h"


static inline void EMA_ThinkNormal( Work *work )
{
    if ( EMA_Flag( EMA_F_EVENT_SNIPE ) )
	EMA_ThinkSniperEvent( work ) ;  /* think/thk_snipe.h  */
    else if ( work->ext_trg )
	EMA_ThinkPlayerTrigger( work ) ;/* think/thk_player.h */

    /* パッドが変わった時点でも time をクリアする */
    if ( work->npc.action.pad != work->npc.action.set_pad )
	work->time = 0 ;
}

static inline void EMA_ThinkWakeup( Work *work )
{
    switch ( work->npc.action.set_pad )
    {
    case DAMG_DOWN :
    case DAMG_FALL :
    case DAMG_SLEEP :
    case DAMG_TUMBLE:
	if ( work->npc.action.faint <= 0 )
	    work->npc.action.faint = EMA_FAINT ;
	work->npc.action.pad = DAMG_WAKE ;
    }
}

static inline void EMA_ThinkGameover( Work *work )
{
    if ( work->flag & EMA_F_GAMEOVER )
    {
	if ( (EMA_Flag( EMA_F_EVENT_SNIPE ) &&
	      EMA_Flag( EMA_F_HURT_BY_VMP )) ||
	     GM_CheckObject_IsEnd( &work->body, 0 ) )/* モーション終了持呼ぶ */
	{
	    work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;

	    GM_GameOverProcEnd( work ) ;
	}
    }
    else if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
	work->npc.action.pad = PAL_IDLE ;

    EMA_ResetSetFlag( EMA_F_ENB_ARM_IK, EMA_F_NON_FACING ) ;
}


/*----- 思考処理メイン-----------------------------------------------------*/
void EMA_Think( Work *work )
{
    work->npc.action.pad = work->npc.action.set_pad ;

    /* ゲームオーバー時の処理 */
    if ( GM_IsGameOver() )
	EMA_ThinkGameover( work ) ;

    else if ( EMA_GameOverCheck( work, 0 ) )
    {
	/* ノーマル状態 */
	if ( !(work->npc.action.status & (NPC_ACT_STATUS_DAMAGE|
					  NPC_ACT_STATUS_NPCSYS_DAM)) )
	    EMA_ThinkNormal( work ) ;    

	/* 起きたよ */
	else if ( work->npc.action.status & NPC_ACT_STATUS_FAINT_END )
	    EMA_ThinkWakeup( work ) ;

	/* 何もしない時は,IKをOFF */
	else
	    EMA_ResetSetFlag( EMA_F_ENB_ARM_IK, EMA_F_NON_FACING ) ;
    }
}
