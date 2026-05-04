/*
  act_damage.h
  エマ ダメージアクト処理

  2001/02/21 Y.Korekado
  2001/02/23 T.Morita Revised
  $Id: act_damage.h,v 1.1.1.3 2002/11/19 11:46:01 Yoshizawa1 Exp $
*/

void EMA_ActDamage( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;

    if ( time == 0 )
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( !npc->CheckDamage( npc ) )
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	{
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
}


static void ActDropFromBridge( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_DEATH|NPC_ACT_STATUS_CAPTURE_OFF ) ;

    npc->ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK|CTRL_SKIP_FLR_CHECK ;
    if ( time == 0 )
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( !npc->CheckDamage( npc ) )
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	{
	    act->act_end = 1 ;
	    NPC_ActStatus( act, NPC_ACT_STATUS_DEATH ) ;
	    npc->body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}
}

static void ActSleptDropFromBridge( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_CAPTURE_OFF ) ;

    npc->ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ;
    if ( time == 0 )
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( !npc->CheckDamage( npc ) )
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	{
	    act->act_end = 1 ;
	    NPC_ActStatus( act, NPC_ACT_STATUS_DEATH ) ;
	    npc->body->m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	}
}

static void ActDown( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    Work   *work = npc->character ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_EYE_CLOSE|NPC_ACT_STATUS_CAPTURE_OFF ) ;

    /* 傾いた床に対応する */
    NPC_SetSlopeRotX( npc->ctrl ) ;
    work->shadow_flg = 0 ;

    if ( time == 0 )
	NPC_SetActMotion( npc, npc->base_mar, DAMG_DOWN ) ;

    if ( time > 60*5/TIME_BASE )
	NPC_SetActMode( npc, ActWakeup ) ;
}

static void ActFall( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    Work   *work = npc->character ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_CAPTURE_OFF ) ;

    /* 傾いた床に対応する */
    NPC_SetSlopeRotX( npc->ctrl ) ;
    work->shadow_flg = 0 ;

    if ( time == 0 )
	EMA_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( !npc->CheckDamage( npc ) )
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	{
	    if ( act->faint > 0 )
		NPC_SetActMode( npc, ActDown ) ;
	    else
	    {
		NPC_SetFaintCount( npc, EMA_FAINT_COUNT, NPC_FAINT_PIYO ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_PIYO ) ;
		NPC_SetActMode( npc, NPC_ActFaint ) ;
	    }
	}
}


static void ActSleepDown( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    Work   *work = npc->character ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_EYE_CLOSE|NPC_ACT_STATUS_CAPTURE_OFF ) ;

    /* 傾いた床に対応する */
    NPC_SetSlopeRotX( npc->ctrl ) ;
    work->shadow_flg = 0 ;

    if ( time == 0 )
	EMA_SetActMotion( npc, npc->base_mar, act->set_mot ) ;
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
    {
	act->act_end = 1 ;
	NPC_SetFaintCount( npc, EMA_SLEEP_COUNT, NPC_FAINT_ZZZ ) ;
	NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
	if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	{	
	    if ( !EMA_DamageGameOver( work, SNIPE_DOWN_FALL ) )
		NPC_SetActMode( npc, ActSleptDropFromBridge ) ;
	}
	else
	    NPC_SetActMode( npc, NPC_ActFaint ) ;
    }
}

static void ActDamageWakeUp( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;
    Work   *work = npc->character ;

    npc->action.status |= NPC_ACT_STATUS_CAPTURE_OFF ;
    if ( time == 0 )
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    NPC_ReSetSlopeRotX( npc->ctrl ) ;
    work->shadow_flg = 0 ;

    if ( time < 80*5/TIME_BASE )
    {
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;
	//NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN ) ;
    }
    if ( !npc->CheckDamage( npc ) )
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	{
	    NPC_ClearNeedl( npc->body ) ;
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
}
