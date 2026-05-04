/*
    prezpre.c
    ニキータイベント用大統領思考前処理

    2001/04/26 Masafumi Okuta
    $Id: prezpre.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/

// メッセージ受信
static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code ;
    NPCWORK*	npc;
    CONTROL	*ctrl ;
    extern int	ENE_ReadNodes( ROUTENAVI*, int );

    npc = &work->npc;
    ctrl = &work->control ;

    n_msg = ctrl->n_msg ;
    msg = ctrl->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case PREZ_MESSAGE_SLEEP :
	    npc->ctrl->mov.vx = msg->message[ 1 ];
	    npc->ctrl->mov.vy = msg->message[ 2 ];
	    npc->ctrl->mov.vz = msg->message[ 3 ];
	    npc->ctrl->rot.vy = msg->message[ 4 ];
	    npc->ctrl->turn.vy = msg->message[ 4 ];
	    PREZ_SetThink3( work, TH3_SLEEP );
	    break ;
	case PREZ_MESSAGE_CHG_ROUTE:
	    ENE_ReadNodes( npc->rnavi, msg->message[ 1 ] ); // ルート変更	
	    work->vecNextPos = npc->rnavi->nodes[(short)npc->rnavi->next_node];
	    work->nNextZone  = HZX_GetAddress( npc->ctrl->hzx_id, &work->vecNextPos, -1);
	    break;
	case PREZ_MESSAGE_ROUTE_QUICK:
	    ENE_ReadNodes( npc->rnavi, msg->message[ 1 ] ); // ルート変更	
	    work->vecNextPos = npc->rnavi->nodes[(short)npc->rnavi->next_node];
	    work->nNextZone  = HZX_GetAddress( npc->ctrl->hzx_id, &work->vecNextPos, -1);
	    {
		HZX_ZONE_ADD trgzone;
		// ゾーンアドレスからナビターゲットの情報を生成
		trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
		GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
		// 目標位置セット
		work->navitrg.pos  = work->vecNextPos;  
	    }
	    break;
	default :
	    break ;
	}
	msg++ ;
    }
}

// 周辺情報更新
static	void	InfoCheck( Work *work )
{
    FVECTOR	vec ;
    NPCWORK	*npc ;

    npc = &work->npc ;

    _sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
    work->pl_dis = _FVecLen3( &vec ) ;	/* プレイヤーとの距離 */
    work->pl_dir = _FVecDir2( &vec ) ;	/* プレイヤーへの方向 */

    // 頭の位置取得
    _sceVu0CopyVector( &work->vecFacePos, &BODYWORLD( &work->body, HUMAN21_ATAMA).m[3][0]);

    // 顔アニメ
    work->nFaceMess = -1;

    UpdateNikitaInfo( work );	// ニキータ情報更新

    // 雑音関連
    if ( work->nNoiseCntr > 0 ){
	work->nNoiseCntr--;
    }else if ( work->nNoiseCntr == 0 ){
	if ( work->nNoiseLevel > 0){
	    work->nNoiseLevel--;
	}
    }

#ifdef DEBUG_MODE
    PREZ_DbgSetMuteki( work );
#endif
}
// 前処理
static void PreProcess( Work *work )
{
    CheckMessage( work ) ;	// メッセージ受信

    InfoCheck( work ) ;		// 周辺情報更新
}

// 後処理
static void AfterProcess( Work *work )
{
    NPCWORK*	npc ;

    npc = &work->npc ;

    // 顔の向き 
    npc->action.face_dir = npc->ctrl->rot.vy ;

    // 死亡チェック
    if ( !work->bDead && npc->action.life <= 0){
	npc->action.life = 0;
#if 1
	// 殺傷人数を増やす
	if ( GM_KillCount < 30000){
	    GM_KillCount++;
	}
#endif
	work->bDead = 1;
    }
    
    // 視界
    if ( npc->action.status & NPC_ACT_STATUS_EYE_CLOSE){
	npc->action.eye_sight = 0;
    }else{
	npc->action.eye_sight = 4000;
    }


    // 顔更新
    if ( work->nFaceMess >= 0) PREZ_SendFaceMess( work, work->nFaceMess);
		
    // ライフゲージ更新 
    if ( work->gage.value != npc->action.life ) {
	work->gage.value = npc->action.life ;
    }

    // サウンド関連管理更新
    PREZ_SeManager( work );
    PREZ_StreamUpdate( work );

#ifdef DEBUG_MODE
    // デバッグ
    MAO_DbgPlayerPosDump();
    PREZ_DbgSetTrg( work );
    PREZ_DbgDrawInfo( work );
#endif
}




