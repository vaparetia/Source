/*
    fatcommon.c
    ファットマン思考処理:共通部分
    2001/06/02 Masafumi Okuta
    $Id: fatcommon.c,v 1.1.1.3 2002/11/19 11:47:57 Yoshizawa1 Exp $
*/
// 立ち
#define STAND_WAIT_TIME (COUNT_VMODE(6))// (COUNT_VMODE(60))
static void Think3_Stand( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
    }

    // 反応
    if ( (work->nFatInfo & FAT_INFO_SEE_PL) && work->pl_dis < 3000 ){	// 視界内にプレイヤーがいる
	FAT_SetNewAim( work, FAT_GetAttractZone(work), NULL);
	FAT_SetThink3( work, TH3_MOVE);	// プレイヤー追撃
	return;
    }

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }

    // 終了チェック
    if ( work->count3 >= STAND_WAIT_TIME ) {	
	FAT_SetThink3( work, TH3_READY); // 準備
	return;
    }

    // 方向:目標方向を向く
    npc->ctrl->turn.vy = work->aim_dir;

    work->count3++;
}
// 移動
static void Think3_Move( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	if ( FAT_CheckNextZone( work) < 0 ){
MAO_PRINTF("Move <<<< Next Think ------------- %d\n", work->nNextThink3);
	    return; // 設定ミス
	}
	// ルート移動設定
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);

	// 思考分岐チェック
	if ( work->think2 == TH2_DISPOSAL_PHASE ){ 	// 解体フェーズ思考
	    FAT_SetCheckThink( work, FAT_ChkThkDemineMove );
	}else if ( work->think2 == TH2_SHOOT_PHASE ){	// 銃撃フェーズ思考
	    FAT_SetCheckThink( work, FAT_ChkThkShootMove );
	}else{
	    FAT_SetCheckThink( work, FAT_ChkThkTemporary );
	}
    }

    npc->action.pad = PAD_RUN_ACT;	// 走行開始 : ループ再生

    if ( FAT_RouteZoneMove( work) ){ 	// ルート＆ゾーン共存移動
MAO_PRINTF("Move <<<< Next Think ------------- %d\n", work->nNextThink3);
	return;
    }

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
MAO_PRINTF("Move <<<< Next Think ------------- %d\n", work->nNextThink3);
	return;
    }

    work->count3++;
}

// 停止
static void Think3_Stop( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	npc->action.pad = PAD_STOP;	// 停止

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
    }

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3FromNext( work, TH3_READY); // 次候補がなければ思考分岐へ
	return;
    }

    work->count3++;
}

// 180旋回
static void Think3_Turn180( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	if ( work->think3 == TH3_TURN_BACK ) 	  npc->action.pad = PAD_TURN_BACK;	// 旋回(後ろ)
	else if ( work->think3 == TH3_TURN_BACK ) npc->action.pad = PAD_TURN_FRONT;	// 旋回(前)	
	else					  npc->action.pad = PAD_REVERSE;	// 反転

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
    }

#if 1
    // nvtrgに向ってゾーン移動 
    if ( GM_NaviNear( npc->navi, npc->nvtrg, FAT_NAVI_DIST ) ) { // 目標到達チェック
	if ( work->nNextZone2Valid && FAT_CheckZone(work, work->nNextZone2) >= 0 ){ // 次の指定があるなら
	    FAT_SetNextZoneFrom2( work, GetRandZone( work ) );  // 次の点指定
	    FAT_SetThink3FromNext( work, TH3_MOVE); 		// 移動
	    return;
	}else{
	    FAT_SetThink3( work, TH3_STOP); 			// 停止 7/25 ループバグ
	    return;
	}
    }
#endif

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3( work, TH3_MOVE); // 移動へ
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
// ポイント指定位置移動
static void Think3_PointMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	work->nNextZone = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &work->vecNextPos, -1) );
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	if ( HZX_InsideZone( npc->ctrl->hzx_id, &work->vecNextPos, work->nNextZone) ){ // 目標位置が指定内にあったら	
	    work->navitrg.pos  = work->vecNextPos;  // 目標位置セット
	}

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
    }

    npc->action.pad = PAD_POINT_MOVE;	// ポイント移動


    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }

    // ナビゲート更新
    if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	FAT_SetThink3( work, TH3_READY); // 設置
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
// ばてる
static void Think3_Bleathless( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){	
	npc->action.pad = PAD_BLEATHLESS; // ばてばて

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
	work->nVitalCntr = work->nVitalMax;	// 持久力を元に戻す
    }

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3FromNext( work, TH3_READY); // 準備
	return;
    }

    work->count3++;
}
// 見渡す
#define OVERLOOK_VOIDED 	(COUNT_VMODE(15)) 	// 見渡し開始までの隙
#define OVERLOOK_INTERVAL	(COUNT_VMODE(60)) 	// 見渡し間隔
#define OVERLOOK_TIMEOUT	(COUNT_VMODE(100))	// 見渡し終了
static void Think3_OverLook( Work* work )
{
    int 	near;
    NPCWORK*	npc;

    npc = &work->npc;

    if ( work->nLostTrgCntr > FAT_LOST_TRG_THINK ){ // 見失い音声
	if ( work->count3 == OVERLOOK_INTERVAL + OVERLOOK_VOIDED ){
	    if ( MAO_Rand( 2 ) == 0)   FAT_Say( work, SE_FATWHE01, GM_SEMODE_BOMB);
	    else		       FAT_Say( work, SE_FATWHE02, GM_SEMODE_BOMB);
	}
	NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE); // ？
    }

    // ふりむき
    if ( !((work->count3+OVERLOOK_VOIDED) % COUNT_VMODE(OVERLOOK_INTERVAL)) ) { // (OVERLOOK_VOIDED)フレーム後から
	// ルートのつながっている方向の中で背後に一番近いゾーン方向を向く
	near = FAT_GetZoneFromInnerMin( &work->npc, npc->ctrl->addr, &work->vecFatDir, NULL);
	if ( near !=  -1 ) { 
	    npc->ctrl->turn.vy = ENE_ZoneDir( &npc->ctrl->mov, near, npc->ctrl->hzx_id ) ; 
	}

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );
    }
    
    npc->action.pad = PAD_OVERLOOK;	// 見渡す

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }


    // プレイヤー発見
    if ( work->nFatInfo & FAT_INFO_SEE_PL){
	NPC_CallHeadMark( npc, HMK2_TYPE_RED_AT); // ！
	if ( work->pl_dis < GLK_DIST) { // 射程内にいる
	    FAT_SetThink3( work, TH3_FEINT_SHOOT);	// 威嚇射撃へ
	    return;
	}else{
	    FAT_SetThink3( work, TH3_RUN_SHOOT);	// 走り撃ちへ
	    return;
	}
    }

    // 時間終了
    if ( work->count3 >= COUNT_VMODE(OVERLOOK_TIMEOUT) ){
	FAT_SetThink3FromNext( work, TH3_READY);	// 思考準備へ
	return;
    }
    work->count3++;
}
// 牽制攻撃
static void Think3_Feint( Work* work )
{
    NPCWORK*	npc;

    npc = &work->npc;

    if ( work->count3 ==  0){
	_sceVu0CopyVector( &work->vecAimPos, &GM_PlayerPosition); // 目標をプレイヤーに
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    npc->action.pad = PAD_SHOT_READY; // 銃構え

    // グロック発射
    if ( work->count3 > COUNT_VMODE(15) ){
	if ( work->nBehindCntr > 0){ // 視界内 or ビハインド警戒中
	    if ( work->count3 % COUNT_VMODE( 3 ) == 0){
		npc->action.pad = PAD_SHOT_ACT;	
	    }
	    npc->ctrl->turn.vy = work->aim_dir; // ターゲット方向に向く
	}else if ( (work->nFatInfo & FAT_INFO_SEE_PL) || work->pl_parts != 0 || 
		   work->nLostTrgCntr < COUNT_VMODE(75) ){
	    if ( work->pl_dis < (GLK_DIST + 1000) ){	
		if ( work->count3 % COUNT_VMODE( 3 ) == 0){
		    npc->action.pad = PAD_SHOT_ACT;	
		}
	    }else{
		if ( work->count3 < COUNT_VMODE(45) && work->count3 % COUNT_VMODE( 3 ) == 0){
		    npc->action.pad = PAD_SHOT_ACT;	
		}
	    }
	}else{
	    npc->ctrl->turn.vy = work->aim_dir; // ターゲット方向に向く
	}
    }

    // 装填チェック
    if ( work->nBullet == 0 ){
	FAT_SetNextThink3( work, TH3_READY);	
	FAT_SetThink3( work, TH3_RELOAD);	
	return ;
    }

    // 終了
    if ( work->count3 > COUNT_VMODE(75) ) {
	FAT_SetThink3FromNext( work, TH3_READY );	// 思考
	return;
    }

    // 補間
    if ( work->nFatInfo & FAT_INFO_SEE_PL ){
	MAO_InterpVector( &work->vecAimPos, &work->vecAimPos, &GM_PlayerPosition, 0.15f);
    }
    // 銃口を向ける
    FAT_AdjustAimPoint( work, &work->vecAimPos );

    work->count3++;
}
// 走り撃ち
static void Think3_RunShoot( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){

	if ( FAT_CheckNextZone( work) < 0 ) return; // 設定ミス

	// ルート移動設定
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);

	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );

	work->nShootLimit = 8;	// 発射数設定

	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤

	npc->action.pad = PAD_RUNSHOT_READY;	// 走り撃ち構え
    }

    if ( FAT_RouteZoneMove( work) ){ 	// ルート＆ゾーン共存移動
MAO_PRINTF("Move <<<< Next Think ------------- %d\n", work->nNextThink3);
	return;
    }

    if ( work->CheckThink( work) ){ 	// フェーズ毎の思考チェック
	return;
    }

    if ( work->npc.action.act_end ) {  // 終了
	FAT_SetThink3( work, TH3_MOVE);	// 移動へ	
	npc->action.pad = PAD_RUN_ACT;	// 走り状態維持
	return;
    }

    work->count3++;
}


// エルード落し用目標設定
// 設定完了すると1を返す。 それ以外は0
static int FAT_CheckElude( Work* work )
{
    int j, nDist1, nDist2, nDistTmp;
    int	nIndex[2];
    NPCWORK*	npc;

    npc = &work->npc;

    nDist1 = 65500;
    nDist2 = 65500;
    nIndex[0] = -1;
    nIndex[1] = -1;
    for ( j = 0; j < FAT_ELUDE_FALL_NUM; j++ ){
	// 中継点を算出
	nDistTmp = _MAO_FVec2Len2( &GM_PlayerPosition, &FAT_ELUDE_POS[j]); 
	if ( nDistTmp < nDist1 ){
	    nDist1 = nDistTmp;
	    nIndex[0] = j;
	}
    }
    for ( j = 0; j < FAT_ELUDE_FALL_NUM; j++ ){
	if ( j == nIndex[0] ) continue;
	// 目標点を算出
	nDistTmp = _MAO_FVec2Len2( &GM_PlayerPosition, &FAT_ELUDE_POS[j]); 
	if ( nDistTmp < nDist2 ){
	    nDist2 = nDistTmp;
	    nIndex[1] = j;
	}
    }

    if ( FAT_ChkZoneWrong( npc, HZX_Zone1(GM_PlayerAddress)) ){
	// ゾーン不正
	FAT_SetNZoneWithRelayPoint( work, work->nEludeZone[nIndex[0]], work->nEludeZone[nIndex[1]], 
				    &FAT_ELUDE_POS[nIndex[0]], &FAT_ELUDE_POS[nIndex[1]]);
    }else{
	// プレイヤーのいるゾーンを目指す
	FVECTOR vecPlayer;
	FAT_GetZoneInPoint( &vecPlayer, &GM_PlayerPosition, GM_PlayerAddress);
	FAT_SetNZoneWithRelayPoint( work, work->nEludeZone[nIndex[0]], HZX_Zone1(GM_PlayerAddress), 
				    &FAT_ELUDE_POS[nIndex[0]], &vecPlayer);
    }

    return 1;
}

// エルード落とし
static void Think3_EludeFall( Work* work)
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	if ( work->nNextZone == HZX_Zone1( npc->ctrl->addr ) ){
	    FVECTOR vecPlayer;

	    if ( HZX_Zone1( GM_PlayerAddress) < 0 ){
		FAT_SetThink3( work, TH3_FEINT_SHOOT); // 牽制撃ち
		return;
	    }
	    // プレイヤーエルード地点を目指す
	    FAT_GetZoneInPoint( &vecPlayer, &GM_PlayerPosition, GM_PlayerAddress);
	    GM_ReSetNavi( npc->navi); // リセット
	    _sceVu0CopyVector(&work->navitrg.pos, &vecPlayer);
	    GM_SetNaviTargetFromZoneAddr( npc->nvtrg, GM_PlayerAddress );

	    FAT_SetThink3( work, TH3_ELUDE_KILL); // 指きり
	    return;
	}
	if (work->nNextZone == -1){	// 次候補がおかしい
	    FAT_SetThink3( work, TH3_READY);
	    return;
	}
	// 設定
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);

	work->nStrmOnce = 0;	// 一回呼出ストリーム
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    npc->action.pad = PAD_RUN_ACT;	// 走行開始 : ループ再生

    {
	float fDist = _MAO_FVec2Len2( &npc->ctrl->mov, &GM_PlayerPosition);
	if ( fDist < 3000.f ){
	    FAT_SetThink3( work, TH3_ELUDE_KILL); // 指きり
	    return;
	}else if ( fDist < 5000.f && !work->nStrmOnce){
	    FAT_StreamRequest( work, FAT_STRM_ELUDE_01 + BP_PS2_rand() % 2); // エルードキルの台詞
	    work->nStrmOnce = 1;
	}
    }

    if ( FAT_RouteZoneMove(work) ){
	return;
    }

    work->count3++;
}

// 指きり
static void Think3_EludeKill( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	npc->action.pad = PAD_ELUDE_KILL;	// 停止
    }

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3( work, TH3_READY); // 次候補がなければ思考分岐へ
	return;
    }

    GM_NaviNear( npc->navi, npc->nvtrg, 500 );

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}

// エルード中のプレイヤーを狙撃
enum{
FAT_ELUDESHOOT_RELOAD,
FAT_ELUDESHOOT_ADJUST,
FAT_ELUDESHOOT_LEFT,
FAT_ELUDESHOOT_RIGHT,
};
static void Think3_EludeShoot( Work* work)
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	npc->action.pad = PAD_RELOAD;	// リロード
	FAT_SetStep( work, FAT_ELUDESHOOT_RELOAD);
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    switch ( work->nStep ){
    case FAT_ELUDESHOOT_RELOAD: // リロード
	// 終了
	if ( work->npc.action.act_end ) {
	    FAT_SetStep( work, FAT_ELUDESHOOT_ADJUST);
	}
	break;
    case FAT_ELUDESHOOT_ADJUST: // アジャスト
	npc->action.pad = PAD_SHOT_READY; // 銃構え

	FAT_AdjustAimPoint( work, (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE ).m[3][0] );
	
	// 終了
	if ( work->nStepCntr++ > COUNT_VMODE(60) ) {
	    work->nShootLimit = 5;		// 発射数
	    npc->action.pad = PAD_LIMIT_SHOOT;	// 左手狙撃	
	    FAT_SetStep( work, FAT_ELUDESHOOT_LEFT);
	}
	break;
    case FAT_ELUDESHOOT_LEFT:   // 左手狙撃

	FAT_AdjustAimPoint( work, (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE ).m[3][0] );

	// 終了
	if ( work->npc.action.act_end ) {
	    work->nShootLimit = 5;		// 発射数
	    npc->action.pad = PAD_LIMIT_SHOOT;	// 右手狙撃
	    FAT_SetStep( work, FAT_ELUDESHOOT_RIGHT);
	}
	break;
    case FAT_ELUDESHOOT_RIGHT:  // 右手狙撃
	FAT_AdjustAimPoint( work, (FVECTOR*)&BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_TE ).m[3][0] );
	// 終了
	if ( work->npc.action.act_end ) {
	    FAT_SetThink3( work, TH3_READY); // 次候補がなければ思考分岐へ
	    return;
	}
	break;
    }

    // 方向設定
    npc->action.dir = work->pl_dir;

    work->count3++;
}

// 間を置くアクション
static void Think3_Standby( Work* work)
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	npc->action.pad = PAD_REPAIR;	// ローラーブレード修理
    }

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3FromNext( work, TH3_READY); // 次候補がなければ思考分岐へ
	return;
    }

    // 方向設定
    npc->action.dir = work->pl_dir;

    work->count3++;
}

// イベント用
static void Think3_EventStand( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
	npc->action.pad = PAD_REPAIR;	// 
	// 思考分岐チェック
	FAT_SetCheckThink( work, FAT_ChkThkTemporary );

	FAT_StreamRequest( work, FAT_STRM_GAMEOVER); // ふぁっとへっど
    }

    // 方向:目標方向を向く
    npc->ctrl->turn.vy = work->aim_dir;

    work->count3++;
}
