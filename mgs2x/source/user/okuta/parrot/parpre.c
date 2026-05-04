/*
    parpre.c
    オウム思考前処理

    2001/04/26 Masafumi Okuta
    $Id: parpre.c,v 1.1.1.3 2002/11/19 11:48:06 Yoshizawa1 Exp $
*/
// オウムの近くの爆弾を無効化
static inline void PAR_AvoidNearBomb( PARROT* work )
{
    GM_BOMB*	bomb;
    FVECTOR  	vec;
    int 	nDist;

    nDist = 4000.f * 4000.f;

    for ( bomb = GM_BombList.next; bomb != NULL ; bomb = bomb->next ){
	_sceVu0SubVector( &vec, bomb->mov, &work->npc.ctrl->mov);
	if ( _sceVu0InnerProduct( &vec, &vec ) < nDist){
	    bomb->flag |= GM_BMB_FLAG_NOBLAST;
	}
    }
}

static	void	CheckMessage( PARROT* work )
{
    GV_MSG	*msg ;
    int n_msg, code ;
    NPCWORK*	npc;
    CONTROL	*ctrl ;

    npc = &work->npc;
    ctrl = &work->control ;

    n_msg = ctrl->n_msg ;
    msg = ctrl->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case PAR_MESSAGE_TALK_ON :	// 喋り許可
	    work->nTalkFlag = 1;
	    break;
	case PAR_MESSAGE_TALK_OFF :	// 喋り不許可
	    work->nTalkFlag = 0;
	    break;
	case PAR_MESSAGE_ERO_LISTEN :	// エロ本聞き耳
	    work->nAttentionTime  = msg->message[ 1 ];	// 注目時間
	    PAR_SetThink3( work, TH3_ERO_LISTEN);	
	    break;
	case PAR_MESSAGE_EAMES_LISTEN :	// エイムズ聞き耳
	    work->vecAimPos.vx = (float)msg->message[ 1 ];		
	    work->vecAimPos.vy = (float)msg->message[ 2 ];		
	    work->vecAimPos.vz = (float)msg->message[ 3 ];		
	    work->vecAimPos.vw = (float)1.f;		
	    work->nAttentionTime  = msg->message[ 4 ];	// 注目時間	
	    PAR_SetThink3( work, TH3_EAMES_LISTEN);	
	    break;
	case PAR_MESSAGE_IDLE :		// アイドリング
	    PAR_SetThink3( work, TH3_STAND);	
	    break;
	case PAR_MESSAGE_ERO_ALLOW :	// エロ本喋り許可
	    work->nEroLearn = 1;
	    break;
	case PAR_MESSAGE_EAMES_ALLOW :	// エイムズ喋り許可
	    work->nEamesLearn = 1;
	    break;
	case PAR_MESSAGE_NOISE_RATE :	// 物音発生確率
	    work->nNoiseRate  = msg->message[ 1 ];
	    break;
	case PAR_MESSAGE_SUBJECT_RATE :	// 喋り内容の割合
	    work->nSubjectRate[PAR_SUBJECT_NORM] = msg->message[ 1 ];
	    work->nSubjectRate[PAR_SUBJECT_ERO]  = msg->message[ 2 ];
	    work->nSubjectRate[PAR_SUBJECT_EIMS] = msg->message[ 3 ];
	    break;
	default :
	    break;
	}
	msg++ ;
    }
}

static	void	InfoCheck( PARROT* work )
{
    FVECTOR	vec ;
    NPCWORK	*npc ;

    npc = &work->npc ;

    _sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
    work->pl_dis = _FVecLen3( &vec ) ;	/* プレイヤーとの距離 */
    work->pl_dir = _FVecDir2( &vec ) ;	/* プレイヤーへの方向 */

    // 頭の位置取得
    _sceVu0CopyVector( &work->vecFacePos, &BODYWORLD( &work->body, HUMAN21_ATAMA).m[3][0]);

}
/*----------------------------------------------------------------*/
static void PreProcess( PARROT* work )
{
    CheckMessage( work ) ;

    InfoCheck( work ) ;		/* 周りの情報チェック */
}


static void AfterProcess( PARROT* work )
{
    NPCWORK*	npc ;

    npc = &work->npc ;
    /* 顔の向き */
    npc->action.face_dir = MatToYRot( (FMATRIX*)&BODYWORLD( &work->body, HUMAN21_ATAMA) );  // npc->ctrl->rot.vy ;

    // 狙われカウンタ更新
    if ( work->nAttackCntr == 0 ){
	if ( work->nAttackNum > 0 ){
	    work->nAttackNum--;
	    work->nAttackCntr = PAR_CALL_INTV;
	}
    }else{
	work->nAttackCntr--;
    }

    // 敵兵呼び
    if ( work->nCallEnemyFlag ){
	if ( work->nCallEnemyCntr == 0){
	    GM_SetNoise( NOISE_M , &npc->ctrl->mov, npc->ctrl->map );	  // 音で敵を呼ぶ
	    work->nCallEnemyFlag = 0;
	}else{
	    work->nCallEnemyCntr--;
	}
    }
#ifdef DEBUG_MODE
#if 0
    DEBUG_Locate( 340, 100, 0 );
    DEBUG_Printf( "NUM   = %d \n", work->nAttackNum);
    DEBUG_Printf( "CNTR  = %d \n", work->nAttackCntr);
    DEBUG_Printf( "ERO   = %d \n", work->nEroLearn);
    DEBUG_Printf( "EAMES = %d \n", work->nEamesLearn);
    DEBUG_Printf( "%f\n", _MAO_FVec2Len3( &GM_PlayerPosition, &npc->ctrl->mov));
#endif
PAR_DbgSoundCheck(work);
PAR_DbgStrmMode(work);
#endif

    // サウンド関連管理更新
    PAR_SeManager( work );
    PAR_StreamUpdate( work );

    // 爆弾無効化
    PAR_AvoidNearBomb( work );

    work->homing.status |= HOMING_SKIP;
}


