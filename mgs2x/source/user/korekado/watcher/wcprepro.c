/*
	wcprepro.c
	見張り兵、思考前処理

	1999/07/07 Y.Korekado
	$Id: wcprepro.c,v 1.1.1.3 2002/11/19 11:44:31 Yoshizawa1 Exp $
	
*/

/* メッセージ処理 */
static	void	WatcherCheckMessage( entk )
ENETHINK	*entk ;
{
    GV_MSG	*msg ;
	int n_msg, code ;

    n_msg = entk->ctrl->n_msg ;
	msg = entk->ctrl->msg ;

if  ( n_msg > 0 ) {
//	printf("message get id=[%d]\n",entk->id ) ;
}

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case ENE_MSG_ROOTCHANGE :
//printf("change route [%d]->[%d] deffende[%d][%d][%d]\n",
//		entk->rnavi->c_route, msg->message[ 1 ],msg->message[ 2 ],msg->message[ 3 ],msg->message[ 4 ] ) ;
				entk->rnavi->next_route = (short)msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					entk->rnavi->next_route += COM_GetRootOfset( ) ;
				}
				if ( msg->message_len >= 6 ) {
					entk->def_pos.vx = (float)msg->message[ 2 ] ;
					entk->def_pos.vy = (float)msg->message[ 3 ] ;
					entk->def_pos.vz = (float)msg->message[ 4 ] ;
					entk->def_mapbit = GM_GetHzxGroupID( ( GM_GetMapID( msg->message[ 5 ] ) ) ) ;
//0なら追跡攻撃					ASSERT( entk->def_mapbit != 0 ) ;
					if ( msg->message_len >= 7 ) {
printf("change node [%d] \n",msg->message[ 6 ] ) ;
						entk->rnavi->chang_node = (u_char)msg->message[ 6 ] ;
					} else {
						entk->rnavi->chang_node = -1 ;
					}
//					printf(" Message Group ID [%x] \n",entk->def_mapbit ) ;
				}
			break ;
			case ENE_MSG_ACTION_END :
				SET_FLAG( entk->receive, ENE_ORDER_ACTION_END ) ;
			break ;
			case ENE_MSG_HOUNYOU_HOM_ON :
				SET_FLAG( entk->sw.hounyou, 0x02 ) ;
			break ;
			case ENE_MSG_HOUNYOU_HOM_OFF :
				UNSET_FLAG( entk->sw.hounyou, 0x02 ) ;
			break ;
		}
		msg++ ;
	}
}

static void PreProcess( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;

	WatcherCheckMessage( entk ) ;

	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;

	ENE_EyeInfoCheck( entk, &entk->pl_eyei ) ;	/* 視線情報チェック */
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		ENE_EyeInfoCheck( entk, &entk->npc_eyei ) ;	/* 視線情報チェック */
	}
	ENE_NoticeCheck( entk ) ;					/* noticeチェック */
	ENE_SetAlertLevel( entk ) ;					/* 危険値計算 */
}
