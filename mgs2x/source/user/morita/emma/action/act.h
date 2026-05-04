/*
  act.h
  エマ 標準アクション処理

  2001/02/21 Y.Korekado
  2001/02/23 T.Morita Revised
  $Id: act.h,v 1.1.1.3 2002/11/19 11:46:01 Yoshizawa1 Exp $
*/



void EMA_SetActMotion( NPCWORK *npc, int mar, int mot_num )
{
    Work *work = (Work *)npc->character ;

    NPC_SetActMotionEX( npc,
			LAYER_BASE, mar, mot_num,
			MOTION_MASK_FULL, work->interp, (float)TIME_BASE ) ;
    work->interp = ACT_INTERP_DEF ;
}

void EMA_ActLoopMotion( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    if ( time == 0 )
	EMA_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( npc->CheckDamage( npc ) )
	return ;
    if ( act->pad != act->set_pad )
    {
	if ( !npc->CheckPad( npc ) )
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
	act->act_end = 1 ;
    if ( act->dir >= 0 )
	npc->ctrl->turn.vy = act->dir ;

}

/* １回こっきりモーション */
void EMA_ActOneTimeMotion( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    if ( time == 0 )
	EMA_SetActMotion( npc, act->set_mar, act->set_mot ) ;

    if ( npc->CheckDamage( npc ) )
	return ;
    if ( act->pad != act->set_pad )
    {
	if ( !npc->CheckPad( npc ) )
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) )
    {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
    if ( act->dir >= 0 )
	npc->ctrl->turn.vy = act->dir ;
}



void EMA_ActLayerMotion( Work *work, int mot, int start, int flag )
{
    GM_ConfigObjectAction( work->npc.body, 1, mot, start*5,
			   flag, 120/5*TIME_BASE ) ;
}


