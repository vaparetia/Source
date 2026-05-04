/*
	fatpre.c
	ファットマン思考前処理
	2000/04/03 Masafumi Okuta
	$Id: fatpre.c,v 1.1.1.3 2002/11/19 11:48:00 Yoshizawa1 Exp $
*/
enum {
FAT_MESSAGE_MODE_FIRST_PUT,	// 初回設置
FAT_MESSAGE_PICK_ATTACK,	// カモメのくちばし攻撃
};

// 死体またぎ
#define FAT_CORP_CHK_W		(600.f)		// 死体またぎ発動幅
#define FAT_CORP_CHK_H		(1250.f)	// 死体またぎ発動高さ	

// 耐久値回復時間
static inline int FAT_GetDurRecoverTime( void )
{
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	return (COUNT_VMODE(180));	// ダウン時間
	break;
    case GM_LEVEL_EASY:
	return (COUNT_VMODE(120));	// ダウン時間
	break;
    case GM_LEVEL_NORMAL:
	return (COUNT_VMODE(90));	// ダウン時間
	break;
    case GM_LEVEL_HARD:
	return (COUNT_VMODE(90));	// ダウン時間
	break;
    default:
	return (COUNT_VMODE(60));	// ダウン時間
	break;
    }
}

// メッセージチェック
static	void	CheckMessage( Work *work )
{
    GV_MSG*	msg;
    int 	i, n_msg, code;
    CONTROL*	ctrl;
    NPCWORK*	npc;

    npc = &work->npc;
    ctrl = &work->control ;

    n_msg = ctrl->n_msg ;
    msg = ctrl->msg ;

    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];
	switch( code ) {
	case FAT_MESSAGE_MODE_FIRST_PUT: // 初回C4設置
	    // 爆弾設置
	    work->nBombTimer = COUNT_VMODE(msg->message[ 1 ]);
	    for ( i = 2; i < msg->message_len; i++ ){
		SetNewFatmanC4( work, &work->datPoint[ msg->message[i] ], npc->ctrl->map, work->nBombTimer);
	    }
	    FAT_SetThink2( work, TH2_DISPOSAL_PHASE, TH3_COUNT_FIRST);	// 解体フェーズへ
	    break;
	case FAT_MESSAGE_PICK_ATTACK: 	  // カモメのくちばし攻撃
	    {
		FVECTOR vecHit, vecPick, vecBlood;
		
		vecPick.vx = (float)msg->message[ 1 ];
		vecPick.vy = (float)msg->message[ 2 ];
		vecPick.vz = (float)msg->message[ 3 ];
		vecPick.vw = 1.f;
		
		// 当たり位置を計算
		_sceVu0SubVector( &vecHit, &vecPick, (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_ATAMA).m[3][0]);
                _sceVu0Normalize( &vecHit, &vecHit);
		_sceVu0ScaleVector( &vecHit, &vecHit, 0.5f);

		// 流血
		_sceVu0CopyVector( &vecBlood, (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_ATAMA).m[3][0]);
		FAT_SpillBlood( work, &vecBlood, &vecHit);

		// ダメージモードへ
		FAT_SetThink1( work, TH1_DAMAGE, TH2_START_DAMAGE, TH3_WAIT);
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN2, npc->action.pad );
		FAT_Say( work, SE_FATDMG04, GM_SEMODE_BOMB); // ダメージ音声
		npc->action.life -= 1;
	    }
	    break;
	default :
	    break ;
	}
	msg++ ;
    }
}
// 周辺情報のチェック
static	void	InfoCheck( Work *work )
{
    FVECTOR	vec;
    NPCWORK*	npc;

    npc = &work->npc ;

    // 目標とファットマンの位置差分単位ベクトル
    _sceVu0SubVector(  &vec, &work->vecAimPos, &npc->ctrl->mov ) ;
    _sceVu0Normalize( &work->vecAimDiff, &vec );
    work->aim_dis = _FVecLen3( &vec ) ;	// プレイヤーとの距離 
    work->aim_dir = _FVecDir2( &vec ) ;	// プレイヤーへの方向 

    // プレイヤーとファットマンの位置差分単位ベクトル
    _sceVu0SubVector(  &vec, &GM_PlayerPosition, &npc->ctrl->mov ) ;
    _sceVu0Normalize( &work->vecPlaDiff, &vec );
    work->pl_dis = _FVecLen3( &vec ) ;	// プレイヤーとの距離 
    work->pl_dir = _FVecDir2( &vec ) ;	// プレイヤーへの方向 

    // エリア更新
    work->nFatArea = FAT_GetArea( &work->npc.ctrl->mov ); // ファットマンのエリア
    work->nPlaArea = FAT_GetArea( &GM_PlayerPosition ); // プレイヤーのエリア
    work->nAimArea = FAT_GetArea( &work->vecAimPos ); // 目標(プレイヤー)のエリア

    work->nAimZone  = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &work->vecAimPos, -1) );

    // ファットマン関係
    _sceVu0CopyVector( &work->vecFatDir, (FVECTOR *)&BODYWORLD( &work->body, HUMAN21_ONAKA).m[2][0]); 

    // 無敵時間更新
    if (work->nInviCntr >= INVINCIBLE_TIME) 	work->nInviCntr = INVINCIBLE_TIME;
    if (work->nInviCntr > 0) 			work->nInviCntr--;		

    CheckFatmanInfo( work );				// ファットマンの情報を更新

    FAT_EludeFall(work);				// エルード落しチェック

    // イントルードチェック
    if (GM_PlayerStatus & PLAYER_INTRUDE){
	if (work->nIntrudeCntr < FAT_INTRUDE_TIME)  work->nIntrudeCntr++;
    }else{
	// タイマー減衰
	if (work->nIntrudeCntr > FAT_INTRUDE_DEC)   work->nIntrudeCntr -= FAT_INTRUDE_DEC; 
	else					    work->nIntrudeCntr = 0;
    }

    // ビハインドチェック
    if (GM_PlayerStatus & PLAYER_CAUTION){
	if (work->nBehindCntr < FAT_BEHIND_TIME)    work->nBehindCntr++;
    }else{
	// タイマー減衰
	if (work->nBehindCntr > FAT_BEHIND_DEC)     work->nBehindCntr -= FAT_BEHIND_DEC; 
	else					    work->nBehindCntr = 0;
    }
    // 目標位置更新
    if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	if ( !(GM_PlayerStatus & PLAYER_INTRUDE) ){
//	    work->vecAimPos = GM_PlayerPosition;
	    work->nLostTrgCntr = 0;
	}
    }else{
	if ( work->nLostTrgCntr < FAT_LOST_TRG_LIMIT) { // 見失う
	    work->nLostTrgCntr++;
	}
	if ( work->nLostTrgCntr < COUNT_VMODE(30) ){	// すぐに見失うことはない
//	    work->vecAimPos = GM_PlayerPosition;
	}
	if ( work->nBodyDmgTime < COUNT_VMODE(15) || 	
	     work->nHeadDmgTime < COUNT_VMODE(15) || 
	     work->nRollDmgTime < COUNT_VMODE(15)){	// ダメージで検知
//	    work->vecAimPos = GM_PlayerPosition;
	    work->nLostTrgCntr = 0;
	}
	FAT_CheckNoise( work );		// 物音チェック
    }

    // ダメージチェック関数更新
    if ( npc->action.status & FAT_ACT_STATUS_DMG_DEAD ){	// 瀕死
	NPC_SetCheckDamage( npc, CheckDamageNearDead );  	
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = Cheild_Target_Size[FAT_TRG_CHILD_HEAD];
	// 襟のターゲットをスキップ解除
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);

	GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_MUNE )), 
			 &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
    }else if ( npc->action.status & FAT_ACT_STATUS_DMG_STAND ){	// 立ち  
	NPC_SetCheckDamage( npc, CheckDamageStand );  		// 仮に立ち状態
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = Cheild_Target_Size[FAT_TRG_CHILD_HEAD];
	// 襟のターゲットをスキップ解除
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);

	GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_MUNE )), 
			 &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
    }else if ( npc->action.status & FAT_ACT_STATUS_DMG_RUN ){	// 走行時
	NPC_SetCheckDamage( npc, CheckDamageRun );    
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = Cheild_Target_Size[FAT_TRG_CHILD_HEAD];

	// 襟のターゲットをスキップ解除
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);

	GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_MUNE )), 
			 &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
    }else if ( npc->action.status & FAT_ACT_STATUS_DMG_DOWN ){	// ダウン時
	NPC_SetCheckDamage( npc, CheckDamageDown );   
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = Cheild_Target_Size[FAT_TRG_CHILD_HEAD];
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	case GM_LEVEL_EASY:
	    // 襟のターゲットをスキップ
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);
	case GM_LEVEL_NORMAL:
	    GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_ATAMA )), 
			     &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	    GM_SetHomingTrgTarget( &work->homing, &work->def_child[FAT_TRG_CHILD_HEAD] );
	    break;
	default:
	    GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_MUNE )), 
			     &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	    GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
	}
    }else if ( npc->action.status & FAT_ACT_STATUS_DMG_HEAD ){	// 頭部のみ
	NPC_SetCheckDamage( npc, CheckDamageHeadOnly );   
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = FAT_ScaleHead;
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	case GM_LEVEL_EASY:
	    // 襟のターゲットをスキップ
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	    GM_TargetSetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);
	case GM_LEVEL_NORMAL:
	    GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_ATAMA )), 
			     &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	    GM_SetHomingTrgTarget( &work->homing, &work->def_child[FAT_TRG_CHILD_HEAD] );
	    break;
	default:
	    GM_SetHomingTrg( &work->homing, &(BODYWORLD( &work->body, HUMAN21_MUNE )), 
			     &work->body, &(npc->ctrl->hzx_id), npc->ctrl, 0 ) ;
	    GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
	}
    }else{							// 設定ミス
	NPC_SetCheckDamage( npc, CheckDamageStand );  		// 仮に立ち状態
	npc->target.def_child[FAT_TRG_CHILD_HEAD].size = Cheild_Target_Size[FAT_TRG_CHILD_HEAD];
	// 襟のターゲットをスキップ解除
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_F]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_L]);
	GM_TargetResetSkip( &work->def_child[FAT_TRG_CHILD_BLASTNECK_R]);
	GM_SetHomingTrgTarget( &work->homing, &work->deftrg );
    }

    // 耐久値チェック
    if ( work->nFatInfo & FAT_INFO_DOWN ){
	work->nBodyDurable = 0;	// 耐久値復活
	work->nBodyDurTime = 0;
    }else{
	if ( work->nBodyDurable > 0 ){ // 耐久値が減っていたら
	    if ( work->nBodyDurTime++ >= work->nBodyDurableRecover ){
		work->nBodyDurable--;	// 耐久値回復
		work->nBodyDurTime = 0;
//		if ( work->nConsTime == 0 ) work->nConsTime = COUNT_VMODE(360);
	    }
	}
    }

    // 連続ダメージ監視
    if ( work->nConsTime > 0 ) work->nConsTime--;

    // 毎フレームクリアするフラグ
    work->bRollerStop = 0;
    work->bReloadFlag = 0;
    if ( work->nRollerBack > 0) // バック移動カウンタ
	work->nRollerBack--;

    // モーション再生レート更新
    if ( work->body.m_ctrl->mt3_ctrl->motion_total_time != 0.f ){
	work->fFatPlayRate = (float)work->body.m_ctrl->mt3_ctrl->play_time / 
	    (float)work->body.m_ctrl->mt3_ctrl->motion_total_time;
    }else{
	work->fFatPlayRate = 0.f;
    }

    // プレイヤー回避半径
    work->nEvadeRad = (work->pl_dis < 3000)? work->pl_dis : 3000;

    // 持久力更新
    if ( FAT_CheckMotionMove( work ) ){ // 移動中かどうかモーションから判断
	work->nVitalCntr -= FAT_LOST_VITAL;
	if ( work->nVitalCntr < 0 ) work->nVitalCntr = 0;
    }else{
#if 0
	work->nVitalCntr += FAT_RECOVER_VITAL;
	if ( work->nVitalCntr > work->nVitalMax ) work->nVitalCntr = work->nVitalMax;
#endif
    }

    // カモメ休憩終了チェック
    {	
	if ( work->bPrevKmRest && !work->bKamomeRest ){
	    FAT_TakeOffKamome( work, COUNT_VMODE(0)); // 飛び立て
	}
	work->bPrevKmRest = work->bKamomeRest;
	work->bKamomeRest = 0;
    }   

    // 走行モードクリア
    work->nRunMode = 0;

    // エフェクトフラグ
    work->bSpecialEffect = 0;

    // 腰の向き取得
    TS_VecToRot( &work->vecKoshiRot, (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_KOSHI).m[2][0] );
}

// 爆弾に関する情報更新
static void BombCheck( Work* work )
{
    UpdateC4Work( work );
}

// 前処理:思考と行動で使う情報などを更新
static void PreProcess( Work *work )
{
    CheckMessage( work );		// メッセージチェック

    InfoCheck( work );			// 周辺情報チェック 

    BombCheck( work );			// 爆弾情報チェック 
}

// 後処理:現在の情報を保存,デバック埋め込み
static void AfterProcess( Work *work )
{
    NPCWORK*	npc;
    NPCACT*		act;

    npc = &work->npc;
    act = &npc->action;
	
    work->nPrevZone = npc->ctrl->addr; // ファットマンの位置取得:履歴用

    // めりこみ軽減値更新
    FAT_SetRSphereFromMot( work );

    // アジャストチェック
    if ( !work->bAdjustFlag )	FAT_ClearAdjust( work );
    else			work->bAdjustFlag = 0;
    
    // ゲームクリアチェック
    if ( !GM_IsGameOver() ){ // ゲームオーバーチェック
	if ( work->bGameClear ){
	    // クリアプロシージャコール
	    if ( !work->bBombActive && !work->bGameOver && 
		 work->bCallClearProc ){ // 解体中,ゲームオーバーは除く
		work->bCallClearProc = 0;
		GM_SetGameStatus( STATE_PRG_DEMO); // デモフラグを立ててプレイヤーが死なないようにする
#ifdef DEBUG_MODE
printf("Prog Demo Flag = ON :: ClearProc Call\n");
#endif
		FAT_CheckGameClear( work, work->bKillFlag );
		return;
	    }
	}else{
	    // M9気絶によるゲームクリアチェック
	    if ( work->m9_faint <= 0 ){	
		work->m9_faint  = 0;
		// 情報送信用プロシージャコール
		if ( work->procFatInfo != 0){
		    GCL_ARGS  gcl_args;
		    int       data;
	
		    // やられ声 : ぬおー
		    if ( work->nStrmHandle != -1){
			GM_StreamStop( work->nStrmHandle );
			work->nStrmHandle = -1;
		    }
		    GM_SeSetMode( FAT_SE[SE_FATOUT01], (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0],
				  GM_SEMODE_BOMB); 

		    data = FAT_GCLINFO_GAME_CLEAR;
		    gcl_args.argc = 1;
		    gcl_args.argv = &data;
		    GCL_ExecProc( work->procFatInfo, &gcl_args ); // ゲームクリア条件を満たした

		    FAT_SetModeFromPad( npc, ActDeadFall, npc->base_mar, FAT_MOT_BLAST_DAM_3, act->pad );
		}
		work->bKillFlag  = 0;
		work->bGameClear = 1;
	    }else if ( npc->action.life <= 0 ){	    // 体力０によるゲームクリアチェック
		npc->action.life = 0;
		// 情報送信用プロシージャコール
		if ( work->procFatInfo != 0){
		    GCL_ARGS  gcl_args;
		    int       data;

		    // やられ声 : ぬおー
		    if ( work->nStrmHandle != -1){
			GM_StreamStop( work->nStrmHandle );
			work->nStrmHandle = -1;
		    }
		    GM_SeSetMode( FAT_SE[SE_FATOUT01], (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0],
				  GM_SEMODE_BOMB); 
	
		    data = FAT_GCLINFO_GAME_CLEAR;
		    gcl_args.argc = 1;
		    gcl_args.argv = &data;
		    GCL_ExecProc( work->procFatInfo, &gcl_args ); // ゲームクリア条件を満たした

		    FAT_SetModeFromPad( npc, ActDeadFall, npc->base_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
		}
		work->bKillFlag = 1;
		work->bGameClear = 1;
	    }
	}
    }

    // ライフゲージ更新 
    if ( work->m9_faint <= 0 )	    work->m9_faint  = 0;
    if ( npc->action.life <= 0 )    npc->action.life = 0;
    if ( work->gage.value != npc->action.life ) work->gage.value = npc->action.life ;
    if ( work->gage.m9_value != work->m9_faint) work->gage.m9_value = work->m9_faint;

    // 顔の向き 
    npc->action.face_dir = MatToYRot( (FMATRIX*)&BODYWORLD( &work->body, HUMAN21_ONAKA) ); 

    // 武器エフェクト更新
    WeaponEfAct( &work->wctrl);

    // ファットマン地震（画面揺れ+振動）チェック
    FAT_QuakeUpdate( work);

    // サウンド関連管理更新
    FAT_StreamUpdate( work );		// ストリーミング
    if ( !GM_IsGameOver() ){
	FAT_SeManager( work );	      	// 内蔵SE
	FAT_ResidentRollerSe( work ); 	// ローラブレードの音
    }

    // フェーズ継続時間更新
    if ( work->nPhaseCntr < 3000000){
	work->nPhaseCntr++;
    }

    // 生体センサー
    GM_VibrateSensor( npc->ctrl, &work->nVibCensorTime);

    // 死体またぎ
    if ( npc->action.status & FAT_ACT_STATUS_DMG_DOWN && !(GM_PlayerStatus & PLAYER_ON_CORPSE) ){
	FVECTOR vecBound1, vecBound2;
	vecBound1.vx = GM_PlayerPosition.vx - FAT_CORP_CHK_W;
	vecBound1.vy = GM_PlayerPosition.vy - FAT_CORP_CHK_H;
	vecBound1.vz = GM_PlayerPosition.vz - FAT_CORP_CHK_W;
	vecBound2.vx = GM_PlayerPosition.vx + FAT_CORP_CHK_W;
	vecBound2.vy = GM_PlayerPosition.vy + FAT_CORP_CHK_H;
	vecBound2.vz = GM_PlayerPosition.vz + FAT_CORP_CHK_W;
	if ( MAO_BoundCheck( &npc->ctrl->mov, &vecBound1, &vecBound2 ) ){
	    GM_PlayerStatus |= PLAYER_ON_CORPSE;	
	}
    }

    // 目パチ
    if ( npc->action.status & NPC_ACT_STATUS_EYE_CLOSE ){
	FAT_SendFaceMess( work, 0); // 目パチ
    }else if ( npc->action.status & FAT_ACT_STATUS_EYE_CLPOSE ){
	FAT_SendFaceMess( work, 1); // 目閉じ
    }

    // 押し戻し
    FAT_CalcPushPlayer( work );

#ifdef DEBUG_MODE    
    FAT_Debug( work );	// デバック
#endif

#ifdef DEBUG_MODE    
#if 0
    MENU_Locate( 100, 320, 0 );
    MENU_SetColor( 0x00, 0x00, 0x80 );
    MENU_Printf("%d : %d", work->nVitalCntr, work->nVitalMax);
#endif
    {
	int nTmp;
	HZX_ZON*	zone;
	extern HZX_ZON *ENE_HZX_GetZone(int addr);

	nTmp = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone);
	zone = ENE_HZX_GetZone( nTmp );
	if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1( npc->ctrl->addr), work->nNextZone ) ){
printf("------------------- !!Zone Intrpt!! --------------------\n");
printf( "TH1=%2d : TH2=%2d : TH3=%2d : NT3=%2d\n", work->think1, work->think2, work->think3, work->nNextThink3);
printf( "PRE-TH1=%2d : PRE-TH2=%2d : PRE-TH3=%2d : PRE-NT3=%2d\n", 
	   work->nPrevThink1, work->nPrevThink2, work->nPrevThink3, work->nPrevNextThink3);
printf("fatpre.c ::: Error Zone --- %d :\n", work->nNextZone);
MAO_DbgDumpVector( &work->vecNextPos ); 
printf("zone flag = %d\n", zone->flag);
printf("---------------------------------------\n");
	}
    }
#endif
}


