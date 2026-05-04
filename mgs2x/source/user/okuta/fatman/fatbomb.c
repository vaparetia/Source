/*
    fatbomb.c
    ファットマン戦 : 爆弾フェーズ思考ルーチン
    2001/03/14 Masafumi Okuta
    $Id: fatbomb.c,v 1.1.1.3 2002/11/19 11:47:57 Yoshizawa1 Exp $
*/
#define BOMB_STAND_WAIT (COUNT_VMODE(40))
#define FAT_BOMB_DIST	(1200)


/* ---------------------------------- think3 爆弾フェーズ ------------------ */
// 爆弾フェーズ : 開始宣言
static void Think3_BombStart( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	float fLifeRate;
	if ( work->nPrevPhase != TH2_DISPOSAL_PHASE){ // 前のフェーズが解体でなかったら
	    FAT_StreamRequest( work, FAT_STRM_BOMB_START_3); // 音声再生
	}else{
	    FAT_Say( work, SE_FATWAH01 + MAO_Rand(2), GM_SEMODE_BOMB);	//ファットマン嘲笑
	}								
	
	npc->action.pad = PAD_BOMBPHASE_START;

	// 爆弾設置数クリア
	work->nPutNum  = 0;			// 設置した数クリア
	work->nPutIntvCntr = 0;			// 設置間隔時間をクリア

	// 体力＆気絶ゲージによる分岐
	if ( npc->action.life < work->m9_faint) fLifeRate = (float)npc->action.life / (float)work->nVitality;
	else	        		 	fLifeRate = (float)work->m9_faint / (float)work->nVitality;
	if ( fLifeRate > work->fBombDifficlutRate[3])	     FAT_ChangeC4Difficult(work, 0);
	else if ( fLifeRate > work->fBombDifficlutRate[2])   FAT_ChangeC4Difficult(work, 1);
	else if ( fLifeRate > work->fBombDifficlutRate[1])   FAT_ChangeC4Difficult(work, 2);
	else						     FAT_ChangeC4Difficult(work, 3);

	// 設置する爆弾の情報を設定
	if ( work->nBombPutMin == work->nBombPutMax ) 
	    work->nBombMax = work->nBombPutMin;
	else				              
	    work->nBombMax = MAO_GetRandom( work->nBombPutMin, work->nBombPutMax);	

	// タイマー設定
	work->nBombTimer = work->nBombBaseTime + ( work->nBombMax - work->nBombPutMin ) * work->nBombAddTime;

	// 体力＆気絶値取得
	work->nLastBombLife  = work->nPhaseStartLife;
	work->nLastBombFaint = work->nPhaseStartFaint;
    }

    // 終了
    if ( npc->action.act_end ) {
	FAT_SetThink3( work, TH3_READY);	// 爆弾設置点に移動
	return;
    }

    work->count3++;
}
// ポイント指定位置移動
// ある程度ゾーン移動して接近したらポイント指定移動にする
// ループ
static void Think3_BombPointMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	GM_ReSetNavi( npc->navi); // リセット
#if 1
	FAT_SetNextZone( work, HZX_Zone1( work->pCurPosData->zoneAddr ));
	_sceVu0CopyVector( &work->vecNextPos, &work->pCurPosData->vecPut);
#else
	FAT_SetNextZone( work, HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &work->vecNextPos, -1) ));
#endif
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	if ( HZX_InsideZone( npc->ctrl->hzx_id, &work->vecNextPos, work->nNextZone) ){
	    work->navitrg.pos  = work->vecNextPos;  // 目標位置セット
	}else{

	}
    }

    npc->action.pad = PAD_POINT_MOVE;	// ポイント移動

    // 走り設置
    if ( work->nRunningPut && GM_NaviNear( npc->navi, npc->nvtrg, FAT_BOMB_DIST ) && 
	 FAT_CheckNearZoneIn( npc->ctrl->addr, HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone )) ){
	FAT_SetThink3( work, TH3_TURN_PUT);	// 回転置き
	return;
    }
    // 停止設置
    if ( GM_Navi( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	FAT_SetThink3( work, TH3_STOP_PUT);		// 設置
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}

// 走り置き
#define ZONE_POINT_DIST (1500)
static void Think3_RunningPut( Work* work )
{
    FVECTOR		vec;
    NPCWORK*		npc;
    FAT_PUTPOINT_DATA*	pput;

    npc = &work->npc;
    pput = FAT_GetCurrPutPosData( work );

    // 初期設定
    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_SET_BOMB_RUN;	// 移動状態で爆弾設置
    }

    // 終了条件
    if ( work->npc.action.act_end ) {
	FAT_SetThink3( work, TH3_READY);	// 思考準備へ
	return;
    }

    // 向き	
    _sceVu0SubVector( &vec, &pput->vecPut, &npc->ctrl->mov);
    npc->ctrl->turn.vy = _FVecDir2(&vec);

    work->count3++;
}
// 回転置き
static void Think3_TurnPut( Work* work )
{
    FVECTOR		vec;
    NPCWORK*		npc;
    FAT_PUTPOINT_DATA*	pput;
    HZX_ZONE_ADD 	trgzone;

    npc = &work->npc;
    pput = FAT_GetCurrPutPosData( work );

    // 初期設定
    if ( work->count3 == 0 ) {
	// 現在選択中の設置位置情報を取得
	FAT_SetNextZone( work, FAT_GetCurrentC4Zone( work));
	FAT_GetCurrentC4Put( work, &work->vecNextPos);

	// ゾーンアドレスからナビターゲットの情報を生成
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
        work->navitrg.pos  = work->vecNextPos;  // 目標位置セット

	work->bPutFlag = 0; // 設置フラグ初期化
	
	// 方向取得
	if ( FAT_GetTurnPutDir(work) < 0)	npc->action.pad = PAD_SET_BOMB_TURN_R;
	else					npc->action.pad = PAD_SET_BOMB_TURN_L;
    }

    if ( work->bPutFlag ){ // 設置したら次の目標を設定
	// ゾーンアドレスからナビターゲットの情報を生成
	GM_ReSetNavi( npc->navi); // リセット
	FAT_SetNextZone( work, FAT_GetTurnPutRefZone( work )); // 回転置き用参照ゾーンデータから取得
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
    }

    // 追跡
    if ( GM_NaviNear( npc->navi, npc->nvtrg, FAT_BOMB_DIST ) ) {
	// 向き	
	_sceVu0SubVector( &vec, &pput->vecPut, &npc->ctrl->mov);
	npc->action.dir = _FVecDir2(&vec);
    }else{
	// 移動方向指定 
	npc->action.dir = npc->navi->next_dir;
    }

    // 終了条件
    if ( work->npc.action.act_end ) {	
	if ( work->nPutNum < work->nBombMax ){
	    if ( FAT_SetNewC4Point( work ) ){		// 次のターン設置へ
		return;
	    }
	}

	FAT_SetThink3( work, TH3_READY);	// 思考準備へ
	return;
    }


    work->count3++;
}
// 停止置き
static void Think3_StopPut( Work* work )
{
    NPCWORK*	npc;
    FAT_PUTPOINT_DATA*	pput;
    FVECTOR	vecDir;
    int		nDir;

    npc = &work->npc;
    pput = FAT_GetCurrPutPosData( work );
    _sceVu0SubVector( &vecDir, &pput->vecPos, &npc->ctrl->mov);
    nDir = _FVecDir2(&vecDir);

    if ( work->count3 == 0 ) {
	if ( GV_DiffDirAbs( npc->ctrl->rot.vy, nDir) >= 128 ){ // 向くまでcount3は0
	    // 向き
	    npc->ctrl->turn.vy = nDir;
	}else{
	    // 設置方式でモーション分岐
	    npc->action.pad = PAD_SET_BOMB_STOP;	

	    work->count3 = 1;
	}
    }else{
	// 終了
	if ( work->npc.action.act_end ) {
	    FAT_SetThink3( work, TH3_READY);	// 爆弾設置点に移動
	    return;
	}
	// 向き
	npc->ctrl->turn.vy = nDir;
	// カウンタ更新
	work->count3++;
    }
}
// キャンセル以外の行動分岐はここで行なう
static void Think3_BombReady( Work* work )
{
    NPCWORK*	npc;

    npc = &work->npc;

    // フェーズ終了チェック
    if ( work->nPutNum >= work->nBombMax ){  // 設置数でチェック
	FAT_SetThink2( work, TH2_DISPOSAL_PHASE, TH3_PHASE_START);	// 解体フェーズへ
	// 開始合図音声
	FAT_StreamRequest( work, FAT_STRM_BOMB_START + BP_PS2_rand() % 2 ); // 爆弾開始
	return;
    }else if ( work->nPutNum > 0 && work->nBombNum > 0 ){ 
	if ( abs( npc->action.life - work->nLastBombLife) >= FAT_BOMB_PHASE_RATE || 
	     abs( work->m9_faint - work->nLastBombFaint) >= FAT_BOMB_PHASE_RATE ){ // 最後の爆弾設置からダメージ量が一定を越えたら
	    // ダメージによる起動
	    FAT_SetThink2( work, TH2_DISPOSAL_PHASE, TH3_PHASE_START);	// 解体フェーズへ
	    FAT_Say( work, SE_FATDMB01, GM_SEMODE_BOMB); // 音声:じゃまをするな

	    return;
	}
    }
    
    // 最優先チェック 
    if ( work->pl_dis < ESC_NEAR_DIST ){  // 接近回避発動
#if 0
	int nBDZone;
	if ( (nBDZone = FAT_GetBodyAttackZone( work )) != -1 ){	// 体当りエリアチェック
	    // 体当り->逃げ
	    FAT_SetNextZone( work, nBDZone); // 体当り判定結果
	    FAT_SetNextZone2( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	    FAT_SetNextThink3( work, TH3_MOVE);
	    FAT_SetThink3( work, TH3_BODYATTACK);
	    return;
	}else{
	    // 逃げる
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}
#endif
	// 逃げる
	FAT_SetNextZone( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	FAT_SetThink3( work, TH3_MOVE);
	return;
    }

    // 優先チェック
    if ( work->nFatInfo & FAT_INFO_ELUDE_NEAR ){ // エルード近距離
	FAT_SetThink3( work, TH3_STANDBY); // 間を置いてあげる
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetNextThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}	
#if 0
	if ( (work->nFatInfo & FAT_INFO_SEE_PL) && GM_GameLevel >= GM_LEVEL_NORMAL ){ // 視界内にいる
	    FAT_SetThink3( work, TH3_FEINT_SHOOT); // 牽制攻撃
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	    FAT_SetNextThink3( work, TH3_MOVE);
	    return;
	}else{
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	    FAT_SetThink3( work, TH3_MOVE);
//	    FAT_SetNextThink3( work, TH3_MOVE);
	    return;
	}
#endif
    }else if ( work->nFatInfo & FAT_INFO_ELUDE_FAR ){ // エルード遠距離
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}
    }else if ( work->nFatInfo & FAT_INFO_HEAD_CAUTION ){ // 頭部ダメージ直後
	if ( work->nBullet == 0){
	    // 弾が無いので対エリアに逃げてから装填する
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	    FAT_SetNextThink3( work, TH3_RELOAD);
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}
	if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	    if ( work->pl_dis < GLK_DIST && GM_GameLevel >= GM_LEVEL_HARD ){ // 射程内
		FAT_SetThink3( work, TH3_FEINT_SHOOT); // 牽制攻撃
		FAT_Say( work, SE_FATDMB01, GM_SEMODE_BOMB); // 音声:じゃまをするな
		return;
	    }
	}
	if ( FAT_SetNewC4Point( work ) ){ // 位置再設定
	    return;
	}
    }else if ( work->nFatInfo & FAT_INFO_BODY_CAUTION ){ // ブラストスーツ部分ダメージ直後
	if ( work->nBullet == 0){
	    // 弾が無いので対エリアに逃げてから装填する
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	    FAT_SetNextThink3( work, TH3_RELOAD);
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}
	if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	    if ( work->pl_dis < GLK_DIST){ // 射程外
		if ( GV_DiffDirAbs( work->pl_dir, npc->ctrl->rot.vy) < 1024  && GM_GameLevel >= GM_LEVEL_HARD ){ // 前方にいる
		    FAT_SetThink3( work, TH3_FEINT_SHOOT); // 牽制攻撃
		    FAT_Say( work, SE_FATWAH01 + MAO_Rand(2), GM_SEMODE_BOMB);	//ファットマン嘲笑
                    return;
		}
	    }
	}
	if ( FAT_SetNewC4Point( work ) ){ // 位置再設定
	    return;
	}
    }

    // 通常思考判断
    if ( FAT_SetNewC4Point(work) ){ // C4ポイントを探す
	return;
    }

    FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
    FAT_SetThink3( work, TH3_MOVE);
#ifdef DEBUG_MODE
printf("**************** Cannot Find New-C4Point ****************\n");
GV_ERROR( GV_ERROR_WORLD_BOTTOM ) ;
#endif
}

