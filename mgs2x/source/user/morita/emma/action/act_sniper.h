/*
  act_snipe.h
  エマ 狙撃イベント専用アクション

  2001/02/21 Y.Korekado
  2001/02/23 T.Morita Revised
  $Id: act_sniper.h,v 1.1.1.3 2002/11/19 11:46:01 Yoshizawa1 Exp $
*/

static void ActGoDown( NPCWORK *npc, int time )
{
    CONTROL *ctrl = npc->ctrl ;
    OBJECT  *body = npc->body ;
    Work    *work = (Work *)npc->character ;

    if ( !time )
	ctrl->skip_flag |=  CTRL_SKIP_SEG_CHECK ;
    if ( GM_CheckObject_IsEnd( body, 0 ) )
	EMA_SetFlag( EMA_F_RESET_CONTROL ) ;

    work->height =
	BODYPOS( body, HUMAN21_KOSHI            )->vy -
	BODYPOS( body, HUMAN21_HIDARI_TSUMASAKI )->vy -
	body->height ;

    EMA_ActOneTimeMotion( npc, time ) ;
}

static void ActGoUp( NPCWORK *npc, int time )
{
    CONTROL *ctrl = npc->ctrl ;
    OBJECT  *body = npc->body ;
    Work    *work = (Work *)npc->character ;

    if ( time < 23*5/TIME_BASE )
	ctrl->skip_flag |=  (CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK) ;
    if ( GM_CheckObject_IsEnd( body, 0 ) )
	EMA_SetFlag( EMA_F_RESET_CONTROL ) ;
    EMA_ActOneTimeMotion( npc, time ) ;
}
