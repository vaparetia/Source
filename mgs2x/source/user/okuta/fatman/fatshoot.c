/*
    fatshoot.c
    ファットマン戦銃撃フェーズの思考処理
    2001/03/29 Masafumi Okuta
    $Id: fatshoot.c,v 1.1.1.3 2002/11/19 11:48:01 Yoshizawa1 Exp $
*/
// 立ち
#define SHOOT_STAND_WAIT (90)

#define BH_LONG_LENGE (10000)

// 銃撃フェーズ開始 : PRE-FIX
static void Think3_ShootStart( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	if ( work->nPrevPhase != TH2_SHOOT_PHASE){
	    FAT_StreamRequest( work, FAT_STRM_SHOOT_START + (BP_PS2_rand() % 3)); // 音声再生
	}
	npc->action.pad = PAD_SHOOTPHASE_START;
    }

    // 終了
    if ( work->npc.action.act_end ) {
	FAT_SetThink3( work, TH3_READY);	// 準備ルーチンに移る
	return;
    }

    // 方向指定 
    npc->action.dir = work->pl_dir;	// プレイヤー方向に向く

    work->count3++;
}

#if 0
// 攻撃後の判断
static void FAT_AfterAttackJudge( Work* work )
{
    if ( GM_PlayerStatus & PLAYER_DOWNED ){ // 倒れ,匍匐
	FAT_SetNextZone( work, FAT_GetSafetyZoneNear( work )); // 一番近い安全地帯へ移動
	FAT_SetThink3( work, TH3_MOVE);
	return;
    }else if ( GM_PlayerStatus & PLAYER_HOLD ){ // 武器構え
	FAT_SetNextZone( work, FAT_GetSafetyZoneNear( work )); // 一番近い安全地帯へ移動
	FAT_SetThink3( work, TH3_MOVE);
	return;
    }
}
#endif

// 牽制攻撃
#define FAT_FEINT_START 	(15)
#define FAT_FEINT_LOST_CONTINUE (30)
static void Think3_ShootFeint( Work* work )
{
    FVECTOR	vec;
    NPCWORK*	npc;

    npc = &work->npc;

    if ( work->count3 == 0){
	work->nAimIndex = FAT_GLK_AIM_INDEX[ KR_RandU(AIM_INDEX_MAX) ]; // 狙撃関節を選ぶ
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    npc->action.pad = PAD_SHOT_READY; // 銃構え

    // 装填チェック
    if ( work->nBullet == 0 ){
	FAT_SetThink3( work, TH3_READY);	
	return ;
    }

    // グロック発射
    if ( work->count3 == COUNT_VMODE(FAT_FEINT_START) ){ 
	int nSeed;
	nSeed = MAO_Rand( 10 );
	if  ( nSeed < 2){
	    FAT_Say( work, SE_FATKIA02, GM_SEMODE_BOMB); // 音声:うりゃ
	}else if  ( nSeed < 4){
	    FAT_Say( work, SE_FATKIA03, GM_SEMODE_BOMB); // 音声:くらえ
	}else if  ( nSeed < 6){
	    FAT_Say( work, SE_FATKIA04, GM_SEMODE_BOMB); // 音声:しね
	}else if  ( nSeed < 8){
	    FAT_Say( work, SE_FATWAH01, GM_SEMODE_BOMB); // 音声:ぬはは
	}else{
	    FAT_Say( work, SE_FATWAH02, GM_SEMODE_BOMB); // 音声:ぬふふ
	}
    }
    if ( work->count3 > COUNT_VMODE(FAT_FEINT_START) ){ 
	if ( (work->nFatInfo & FAT_INFO_SEE_PL) ){ // 視界内にいる
	    if ( work->pl_dis < (GLK_DIST + 1000) ){	
		if ( work->count3 % GLK_INTERVAL == 0){	
		    npc->action.pad = PAD_SHOT_ACT;	// 発射
		}
	    }else{
		if ( work->count3 % GLK_INTERVAL == 0){	
		    npc->action.pad = PAD_SHOT_ACT;	// 発射
		}
	    }
	}else{ // 視界外
	    if ( work->nLostTrgCntr < COUNT_VMODE( FAT_FEINT_LOST_CONTINUE ) ){ // プレイヤー捕捉の続き
		if ( work->count3 % GLK_INTERVAL == 0){	
		    npc->action.pad = PAD_SHOT_ACT;	// 発射
		}
	    }else{
		if ( work->pl_dis < (GLK_DIST + 1000) ){	
		    if ( work->count3 % GLK_INTERVAL == 0){	
			npc->action.pad = PAD_SHOT_ACT;	// 発射
		    }
		}
	    }
	}
    }

    // 終了
    if ( work->count3 > COUNT_VMODE(75) ) {
	FAT_SetThink3FromNext( work, TH3_READY );	// 思考
	return;
    }

    // 向き変更

    if (work->nFatInfo & FAT_INFO_SEE_PL ){ 
	_sceVu0CopyVector( &vec, (FVECTOR *)&GM_PlayerBody->objs->objs[ work->nAimIndex ].world.m[3][0]);
    }else{
	vec = work->vecAimPos;
    }

    FAT_AdjustAimPoint( work, &vec );

    npc->ctrl->turn.vy = work->aim_dir; // ターゲット方向に向く

    work->count3++;
}
#if 0
// ポイント指定位置移動
// ある程度ゾーン移動して接近したらポイント指定移動にする
// ループ
static void Think3_ShootPointMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	if ( FAT_CheckNextZone( work) < 0 ) return; // 設定ミス
	// ルート移動設定
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);
    }

    npc->action.pad = PAD_RUN_ACT;	// 走行開始 : ループ再生

    if ( FAT_RouteZoneMove( work) ){ // ルート＆ゾーン共存移動
	return;
    }

    work->count3++;
}
#endif
// キャンセル以外の行動分岐はここで行なう
#define SHOOT_NEAR_DIST (2500)
static void Think3_ShootReady( Work* work )
{
    int 	nRes;
    NPCWORK*	npc;

    npc = &work->npc;

    // フェーズ終了チェック
    if ( work->nPhaseCntr > FAT_SHOOT_PHASE_TIME || 				   // 時間チェック
	 abs( npc->action.life - work->nPhaseStartLife) >= FAT_SHOOT_PHASE_RATE || // ダメージ
	 abs( work->m9_faint - work->nPhaseStartFaint) >= FAT_BOMB_PHASE_RATE   || // 気絶値
	 work->nIntrudeCntr >= FAT_INTRUDE_TIME ){				   // イントルード

	// 難度対応
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    if ( work->nShootPhaseCntr < FAT_PHASELEVEL_2){
		FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);  // 爆弾フェーズへ
		return;
	    }
	    break;
	case GM_LEVEL_EASY:
	    if ( work->nShootPhaseCntr < FAT_PHASELEVEL_3){
		FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);  // 爆弾フェーズへ
		return;
	    }
	    break;
	case GM_LEVEL_NORMAL:
	    if ( work->nShootPhaseCntr < FAT_PHASELEVEL_4){
		FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);  // 爆弾フェーズへ
		return;
	    }
	    break;
	case GM_LEVEL_HARD:
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);  	// 爆弾フェーズへ
	    return;
	    break;
	case GM_LEVEL_EXTREME:
	case GM_LEVEL_E_EXTREME:
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);  	// 爆弾フェーズへ
	    return;
	    break;
	}
    }

#if 1
//#ifdef JAPANESE_BP_IGNORE()
    // ばてばて:隙をつくる
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
    case GM_LEVEL_EASY:
	if ( work->nVitalCntr <= 60 ){
	    FAT_SetThink3( work, TH3_BLEATHLESS); // ひと息つく
	    return;
	}
	break;
    }
#endif

    // 最優先チェック : 接近回避発動
    if ( work->pl_dis < SHOOT_NEAR_DIST ){
	int nBDZone;
	if ( (nBDZone = FAT_GetBodyAttackZone( work )) != -1 ){	// 体当りエリアチェック
	    // 体当り->逃げ
	    FAT_SetNextZone( work, nBDZone);
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
    }

    // 優先パターン
    if ( work->nFatInfo & FAT_INFO_ELUDE_NEAR ){ // エルード近距離
	FAT_SetThink3( work, TH3_STANDBY); // 間を置いてあげる
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetNextThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}	
    }else if ( work->nFatInfo & FAT_INFO_ELUDE_FAR ){ // エルード遠距離
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}
    }


    // 優先パターン
    if ( work->nFatInfo & FAT_INFO_ELUDE_NEAR ){ // エルード近距離
	FAT_SetThink3( work, TH3_STANDBY); // 間を置いてあげる
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetNextThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}	
    }else if ( work->nFatInfo & FAT_INFO_ELUDE_FAR ){ // エルード遠距離
	if ( FAT_CheckElude( work ) ){ // エルード落し用ポイントチェック
	    FAT_SetThink3( work, TH3_ELUDE_FALL); // エルード落とし移動
	    return;
	}
    }else if ( work->nFatInfo & FAT_INFO_HEAD_CAUTION ){ // 頭部ダメージ直後
	int nBDZone;
	if ( work->nBullet == 0){
	    // 弾が無いので対エリアに逃げがてら装填する
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	    FAT_SetThink3( work, TH3_RUN_RELOAD);
printf("RUN-RELOAD!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	    return;
	}
        if ( (nBDZone = FAT_GetBodyAttackZone( work )) != -1 ){	// 体当りエリアチェック
	    // 体当り->逃げ
	    FAT_SetNextZone( work, nBDZone);
	    FAT_SetNextZone2( work, FAT_GetAreaDataZone( work )); // 対エリアに逃げる
	    FAT_SetNextThink3( work, TH3_MOVE);
	    FAT_SetThink3( work, TH3_BODYATTACK);
	    return;
	}

	if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	    if ( work->pl_dis < GLK_DIST){ // 射程外
		FAT_SetThink3( work, TH3_FEINT_SHOOT); // 牽制攻撃
		return;
	    }
	}

	// 逃げる
	FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	FAT_SetThink3( work, TH3_MOVE);
	return;
    }else if ( work->nIntrudeCntr > 0 ){    // イントルード対策
	int nIntrContZone;
	nIntrContZone = FAT_GetIntrudeEscZone( work, FAT_GetIntrudeArea());
	if ( nIntrContZone > 0 && nIntrContZone < 255 && 	
	     HZX_Zone1(npc->ctrl->addr) != FAT_GetIntrudeEscZone( work, FAT_GetIntrudeArea())){
	    if ( BP_PS2_rand() % 10 < 2){ // どこにいった!!
		FAT_StreamRequest( work, FAT_STRM_WHERE_01 + (BP_PS2_rand() % 2)); // 音声再生
	    }
	    FAT_SetNextZone( work, nIntrContZone); // 退避ゾーンに移動
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}else{
	    // 対エリアに逃げる
	    nIntrContZone = FAT_GetAreaDataZoneEx( work, FAT_GetContArea( work->nFatArea ));
	    FAT_SetNextZone( work, nIntrContZone); // 対エリアのゾーンに移動
	    FAT_SetNextThink3( work, TH3_STAND);
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}
	return;
    }
    // 残弾数チェック
    if ( work->nBullet == 0 ){
	int nBackZone = FAT_CheckBackShoot( work);
	if ( nBackZone != -1 ){
	    FAT_SetNZoneWithRelayPoint( work, nBackZone, FAT_GetAreaDataZone( work ), NULL, NULL);
	    FAT_SetNextThink3( work, TH3_RUN_RELOAD);
	    FAT_SetThink3( work, TH3_MOVE);
	}else{
	    FAT_SetNZoneWithRelayPoint( work, FAT_GetSafetyZoneNear( work), FAT_GetAreaDataZone( work ), NULL, NULL);
	    FAT_SetNextThink3( work, TH3_RUN_RELOAD);
	    FAT_SetThink3( work, TH3_MOVE);
	}
	return;
    }
    
    { // 後ろ撃ち判定
	// 背後に回られている
	if ( work->pl_dis < 5000.f && GV_DiffDirAbs( npc->ctrl->rot.vy, work->pl_dir ) > 512 &&
	     FAT_CheckNearZoneIn( HZX_Zone1(npc->ctrl->addr), HZX_Zone1(GM_PlayerAddress) ) ){ // プレイヤーと隣接した
	    int nBackZone = FAT_ChkFarZoneFromCrossDir( work, work->pl_dir, 5000.f);
	    if ( !FAT_ChkZoneWrong( npc, nBackZone) ){
		FAT_SetNextZone( work, nBackZone);
		FAT_SetThink3( work, TH3_BACK_SHOOT);
printf("BACK SHOOT TO PL DIR = %d \n", __LINE__);
		return;
	    }

	    nBackZone = FAT_CheckBackShoot( work);
	    if ( nBackZone != -1){
		FAT_SetNextZone( work, nBackZone);
		FAT_SetThink3( work, TH3_BACK_SHOOT);
printf("BACK SHOOT = %d \n", __LINE__);
		return;
	    }
	}
    }

    // プレイヤーに接近
    nRes = FAT_GetZoneFromInnerMin( &work->npc, GM_PlayerAddress, &work->vecPlaDiff, NULL);
    if ( nRes != HZX_Zone1(npc->ctrl->addr) && nRes != -1){ 
	FAT_SetNZoneWithRelayPoint( work, nRes, FAT_GetAreaDataZone( work ), NULL, NULL);
	FAT_SetNextThink3( work, TH3_MOVE); 
	FAT_SetThink3( work, TH3_RUN_SHOOT); 
printf("Go to Player near = %d \n", __LINE__);
        return;
    }else{
	FAT_SetThink3( work, TH3_FEINT_SHOOT); 
printf("FEINT NEAR PL = %d \n", __LINE__);
        return;
    }
}







// -------------------------------- 思考判断関数
// Think3_Move :: 思考分岐用
static int FAT_ChkThkShootMove( Work* work)
{ 
    NPCWORK*	npc;
    npc = &work->npc;

    // 弾切れなら装填
    if ( work->nBullet == 0 ){
	FAT_SetNextThink3( work, TH3_MOVE);
	FAT_SetThink3( work, TH3_RUN_RELOAD);
	return 1;
    }

    // プレイヤーと隣接した時可能ならば後ろ走り撃ちを行なう
    if ( FAT_CheckNearZoneIn( HZX_Zone1(npc->ctrl->addr), HZX_Zone1(GM_PlayerAddress) ) ){ // プレイヤーと隣接した
	if ( !(GM_PlayerStatus & PLAYER_DAMAGED) ){ // プレイヤーがダメージ中でなかったら
	    int nBackZone = FAT_CheckBackShoot( work);
	    if ( !FAT_ChkZoneWrong( npc, nBackZone) ){ // ゾーン番号チェック
		FAT_SetNextZone( work, nBackZone);
		FAT_SetThink3( work, TH3_BACK_SHOOT);
		return 1;
	    }
	}
    }


    return 0; 
}
