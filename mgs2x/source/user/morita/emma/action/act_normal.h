/*
  act_normal.h
  エマ ノーマルのアクション処理

  2001/02/21 Y.Korekado
  2001/02/23 T.Morita Revised
  $Id: act_normal.h,v 1.1.1.3 2002/11/19 11:46:01 Yoshizawa1 Exp $
*/


static void ActSitDown( NPCWORK *npc, int time )
{
    EMA_ActOneTimeMotion( npc, time ) ;
    if ( time > 20 )
	npc->action.status |= NPC_ACT_STATUS_CAPTURE_OFF ;
}

static void ActSquat( NPCWORK *npc, int time )
{
    EMA_ActLoopMotion( npc, time ) ;
    npc->action.status |= NPC_ACT_STATUS_CAPTURE_OFF ;
}


static void ActDownTumble( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    Work   *work = npc->character ;

    NPC_ActStatus( act, NPC_ACT_STATUS_CAPTURE_OFF ) ;
    NPC_SetSlopeRotX( npc->ctrl ) ;

    if ( time == 0 )
	work->capture.flag &= ~CAPTURE_C4EXIST ;/* 転ぶとC4が取れる */

    EMA_ActOneTimeMotion( npc, time ) ;
    if ( time > 20 )
	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN  ) ;
}





static void ActWalkCorpse( NPCWORK *npc, int time )
{
    if ( time == 0 )
    {
	//GM_ConfigObjectAction( npc->body, 1, AFRAID_WALK, 5*4, BODY_MASK|ARMS_MASK, 120/5*TIME_BASE ) ;
    }
#if 0
    /* 33フレームで止める */
    if ( npc->body->m_ctrl->mt3_ctrl[0].play_time >= 33*5 )
	{
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, EMA_ActStandStill ) ;
	    GM_ConfigObjectAction( npc->body, 1, -1, 0, 0xffffffff, 120*5/TIME_BASE ) ;
	}
#endif
    EMA_ActLoopMotion( npc, time ) ;
}

static void ActWakeup( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    int     mot ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_CAPTURE_OFF ) ;
    NPC_ReSetSlopeRotX( npc->ctrl ) ;

    if ( time == 0 )
    {
	if ( act->faint <= 0 )
	    act->faint = EMA_FAINT ;
	mot = DAMG_WAKE ;
	EMA_SetActMotion( npc, npc->base_mar, mot ) ;
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
    {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
