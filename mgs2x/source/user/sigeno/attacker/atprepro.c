/*
	atprepro.c
	attacker act前処理

	1999/07/06 K.Sigeno
	$Id: atprepro.c,v 1.1.1.3 2002/11/19 11:49:00 Yoshizawa1 Exp $
*/

static void	AT_CheckMessage(ENETHINK	*entk )
{
    GV_MSG	*msg ;
	int n_msg, code ;
//	FVECTOR		pos;
	
    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;

#if 1	//10.11 by kore
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case AT_MSG_ROOTCHANGE :
				entk->rnavi->next_route = (short)msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
//					entk->rnavi->next_route += COM_GetRootOfset( ) ;
					entk->rnavi->next_route += GM_RouteOffset ; 
				}
				if ( entk->act->status & ACT_STATUS_UNREAL ) {
					ENE_RouteWarp( entk ) ;
				}
printf("attacker:change route [%d]->[%d] \n",entk->rnavi->c_route, entk->rnavi->next_route ) ;
			break ;
			case ENE_MSG_ACTION_END :
				SET_FLAG( entk->receive, ENE_ORDER_ACTION_END ) ;
			break ;
			case AT_MSG_SEARCHCHANGE :
				entk->search_route = (short)msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					entk->search_route += GM_RouteOffset ; 
				}
				if( msg->message_len >= 3 ) {
					entk->rnavi2->chang_node = (u_char)msg->message[ 2 ] ;
				}
			break ;
		}
		msg++ ;
	}
#endif

}


static void PreProcess( Work *work )
{
	ENETHINK	*entk ;
	AT_THK *at_thk ;



	entk = &work->enethink ;
	at_thk = (AT_THK *) entk->character ;
//	NoticeCheck( &(work->enethink) ) ;
	AT_CheckMessage( entk ) ;

	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

//	SIG_CheckShieldBroken(entk) ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		ENE_EyeInfoCheck( entk, &entk->npc_eyei ) ;	/* 視線情報チェック */
	}

	if(entk->status & ENE_STATUS_CONVERT2){
		ENE_EyeInfoCheck( entk, &entk->bd_eyei ) ;
	}

	ENE_NoticeCheck( entk ) ;
	/*ここで 視覚を危険値に反映*/
	ENE_SetAlertLevel( entk ) ;
	/*毎フレーム寝かすべきフラグはここに*/
	at_thk->at_status &= ~AT_ST_NO_SHOT ;

}
