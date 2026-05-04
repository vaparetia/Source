/*
    fatdemine.c
    ファットマン戦:解体フェーズ思考処理
    2001/04/12 Masafumi Okuta
    $Id: fatdemine.c,v 1.1.1.3 2002/11/19 11:47:58 Yoshizawa1 Exp $
*/
#define COUNT_STAND_WAIT (100)	// 
#define DEMINE_NEAR_DIST (2500)

// 余興ゾーンチェック
static int FAT_ChkAttractZoneIn( Work* work )
{
    int 	i, nFatZone;
    NPCWORK*	npc;

    npc = &work->npc;
    nFatZone = HZX_Zone1(npc->ctrl->addr);

    for ( i = 0; i < 2; i++ ){
	if ( nFatZone == work->nAttractZone[i] ){
	    return 1;
	}
    }
    return 0;
}

// 解体フェーズ用余興ポジションデータ
static void FAT_InitAttractPos2Zone( Work* work )
{
    int i;
    NPCWORK* npc;

    npc = &work->npc;

    for ( i = 0; i < 2; i++ ){
        work->nAttractZone[i] = HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, &FAT_ATTRACTION[i], -1) );
    }
}

// 解体フェーズでの余興ゾーン取得
static int FAT_GetAttractZone( Work* work )
{
    NPCWORK* npc;

    npc = &work->npc;

    // 余興ゾーンにいる
    if ( HZX_Zone1(npc->ctrl->addr) == work->nAttractZone[0] ){
	return work->nAttractZone[1];
    }else if ( HZX_Zone1(npc->ctrl->addr) == work->nAttractZone[1] ){
	return work->nAttractZone[0];
    }

    // 対のエリアの場所を取得する
    switch (FAT_GetContArea(work->nPlaArea)){
    case FATC4_AREA_HERI_LEFT:	 // ヘリポート左エリア
    case FATC4_AREA_HERI_CENTER: // ヘリポート中央エリア
    case FATC4_AREA_HERI_RIGHT:  // ヘリポート右エリア	
	return (work->nAttractZone[1]);
	break;
    case FATC4_AREA_CONT_LEFT:   // コンテナエリア左半分
    case FATC4_AREA_CONT_RIGHT:	 // コンテナエリア右半分
    case FATC4_AREA_BANQ_LEFT:	 // 棚エリア左半分
    case FATC4_AREA_BANQ_RIGHT:	 // 棚エリア右半分
	return (work->nAttractZone[0]);
	break;
    default:
	return (work->nAttractZone[0]);
    }
}

// 爆弾起動
static void Think3_CountDownStart( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){	
	if ( work->nBombCountNum > 0 ){ // 既に発動していたら
	    FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
	    FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
	    FAT_SetThink3( work, TH3_MOVE);	// 移動
	    return;
	}
	work->nBullet = GLK_BUL_NUM;	
	work->bBombStartFlag = 0; // クリア

	if ( work->nBombNum == 0 ){
	    npc->action.pad = PAD_COUNTDOWN_START_FAILED;   // スイッチを押す::失敗
	}else{
	    if ( work->nPutNum >= work->nBombMax ){
		npc->action.pad = PAD_COUNTDOWN_START;   // スイッチを押す
	    }else{
		npc->action.pad = PAD_COUNTDOWN_STARTDMG;// 撃ちながらスイッチを押す
		npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
	    }
	}
	work->bDemineTrue = 0;			// 本気モードクリア
    }   

    if ( work->npc.action.act_end ) {    
	FAT_SetThink3( work, TH3_READY);	// 移動
	return;
    }
    work->count3++;
}
// 爆弾起動
static void Think3_CountDownFirst( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){	
	if ( work->nBombCountNum > 0 ){ // 既に発動していたら
	    FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
	    FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
	    FAT_SetThink3( work, TH3_MOVE);	// 移動
	    return;
	}
	npc->action.pad = PAD_COUNTDOWN_START;	// スイッチを押す

	work->bBombStartFlag = 0; 		// フラグクリア
	work->bDemineTrue = 0;			// 本気モードクリア
	work->nBombPhaseCntr++;
    }   

    if ( work->count3 == COUNT_VMODE(30) ){	
	work->bBombStartFlag = 1; // フラグオン
	// 爆弾起動
	FAT_StartBombCount( work );
    }

    if ( work->npc.action.act_end ) {    
	FAT_SetThink3( work, TH3_READY);	// 移動
	return;
    }
    work->count3++;
}

// キャンセル以外の行動分岐はここで行なう
static void Think3_DemineReady( Work* work )
{
    NPCWORK*	npc;
    NPCACT*	act;

    float	fLifeRate;

    npc = &work->npc;
    act = &work->npc.action;

    // フェーズチェック
    if ( !work->bBombActive ){
	if ( work->nBombNum > 0){ // 起動していない
	    FAT_SetThink3( work, TH3_PHASE_START);
	    return;
	}else{
	    if ( work->bBombStartFlag ){ // フラグオン
		FAT_ChgPhase( work ); // フェーズ変更
	    }else{
		FAT_SetThink3( work, TH3_PHASE_START);
	    }
	    return;
	}
    }

    // 最優先チェック : 接近回避発動
    if ( work->pl_dis < ESC_NEAR_DIST ){
	// 逃げる
	FAT_SetNewAim( work, FAT_GetAreaDataZone( work ), NULL); // 対エリアに逃げる
	FAT_SetThink3( work, TH3_MOVE);
	return;
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
    }else if ( (work->nFatInfo & FAT_INFO_HEAD_CAUTION) ||
	       (work->nFatInfo & FAT_INFO_BODY_CAUTION) ){ // ダメージ直後
	// 残弾０
	if ( work->nBullet == 0){
	    // 弾が無いので対エリアに逃げがてら装填する
	    FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	    FAT_SetThink3( work, TH3_RUN_RELOAD);
	    return;
	}

	// イントルードチェック
	if ( GM_PlayerStatus & PLAYER_INTRUDE ){    // イントルード
	    FAT_SetNewAim( work, FAT_GetIntrudeEscZone( work, FAT_GetIntrudeArea()), NULL); // 退避ゾーン
	    FAT_SetThink3( work, TH3_MOVE);
	    return;
	}

	// プレイヤーを牽制
	if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	    // 背面撃ち
	    int nBackZone = FAT_CheckBackShoot( work);
	    if ( nBackZone > 0 && nBackZone < 255){
		FAT_SetNextZone( work, nBackZone);
		FAT_SetThink3( work, TH3_BACK_SHOOT);
		return;
	    }
	}

	// 逃げる
	FAT_SetNewAim( work, FAT_GetAreaDataZone( work ), NULL);
	FAT_SetThink3( work, TH3_MOVE);
	return;
    }else if (GM_PlayerStatus & PLAYER_INTRUDE){    	// イントルード
	FAT_SetNewAim( work, FAT_GetIntrudeEscZone( work, FAT_GetIntrudeArea()), NULL); // 退避ゾーン
	FAT_SetThink3( work, TH3_MOVE);	
	return;
    }
#if 1
    if ( GM_GameLevel == GM_LEVEL_EXTREME || GM_GameLevel == GM_LEVEL_E_EXTREME ){	// エクストリームは爆弾のみしかもまちぶせ
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }
    if ( act->life < work->m9_faint) fLifeRate = (float)act->life / (float)work->nVitality;
    else		     	     fLifeRate = (float)work->m9_faint / (float)work->nVitality;

    if ( fLifeRate > 0.75f ){ 	
	FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
	FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
	FAT_SetThink3( work, TH3_MOVE);			// 移動
	return;
    }else if ( fLifeRate > 0.33f ){	// 軽傷:背後ナビ
	int nBackZone;
	nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	if ( nBackZone >= 0){
	    FAT_SetNewAim( work, nBackZone, NULL);
	    FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
    }else if ( fLifeRate > 0.15f ){	// 重傷:爆弾守備
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }else{						// しにかけ:爆弾守備命中度UP
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }
#else
    if ( GM_GameLevel == GM_LEVEL_EXTREME || GM_GameLevel == GM_LEVEL_E_EXTREME ){	// エクストリームは爆弾のみしかもまちぶせ
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }
    if ( work->nShootPhaseCntr < 2){ 	
	FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
	FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
	FAT_SetThink3( work, TH3_MOVE);			// 移動
	return;
    }else if ( work->nShootPhaseCntr < 3 ){	// 軽傷:背後ナビ
	int nBackZone;
	nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	if ( nBackZone >= 0){
	    FAT_SetNewAim( work, nBackZone, NULL);
	    FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
    }else if ( work->nShootPhaseCntr < 4 ){	// 重傷:爆弾守備
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }else{						// しにかけ:爆弾守備命中度UP
	work->pdatGuard = FAT_SearchKaitaiC4FarPlayer(work);
	if ( work->pdatGuard != NULL && 
	     (HZX_Zone1(work->pdatGuard->pputData->zoneAddr) != HZX_Zone1(npc->ctrl->addr)) ){
	    FAT_SetNewAim( work, HZX_Zone1(work->pdatGuard->pputData->zoneAddr), &work->pdatGuard->pputData->vecPut);
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);	// 余興:爆弾守備
	    FAT_SetThink3( work, TH3_MOVE);			// 移動
	    return;
	}
	{
	    int nBackZone;
	    nBackZone = FAT_GetNearZoneNotPassDirPrio( work, HZX_Zone1(GM_PlayerAddress), GM_PlayerControl->rot.vy + 2048);
	    if ( nBackZone >= 0){
		FAT_SetNewAim( work, nBackZone, NULL);
		FAT_SetNextThink3( work, TH3_COUNT_BACKNAVI);	// 余興:背後ナビ
		FAT_SetThink3( work, TH3_MOVE);			// 移動
		return;
	    }
	}
    }
#endif
    // 条件がなかった
    FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
    FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
    FAT_SetThink3( work, TH3_MOVE);			// 移動
    return;
}

// 解体中にファットマンが行なう行動::余興
static void Think3_DisposalAttract( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // ゾーンチェック
    if ( !FAT_ChkAttractZoneIn(work) ){
	FAT_SetNewAim( work, FAT_GetAttractZone( work ), NULL);
	FAT_SetNextThink3( work, TH3_COUNT_ATTRACT);	// 余興
	FAT_SetThink3( work, TH3_MOVE);	// 移動
	return;
    }

    switch (FAT_GetContArea(work->nPlaArea)){
    case FATC4_AREA_HERI_LEFT:	// ヘリポート左エリア
    case FATC4_AREA_HERI_CENTER: // ヘリポート中央エリア
    case FATC4_AREA_HERI_RIGHT: // ヘリポート右エリア		
	if ( BP_PS2_rand() % 2 == 0){	FAT_SetThink3( work, TH3_COUNT_KAMOME);}
	else{			FAT_SetThink3( work, TH3_COUNT_DANCE);}
	break;
    case FATC4_AREA_CONT_LEFT: // コンテナエリア左半分
    case FATC4_AREA_CONT_RIGHT:	// コンテナエリア右半分
    case FATC4_AREA_BANQ_LEFT:	// 棚エリア左半分
    case FATC4_AREA_BANQ_RIGHT:	// 棚エリア右半分
	if ( BP_PS2_rand() % 2 == 0){	FAT_SetThink3( work, TH3_COUNT_KAMOME2); }
	else{			FAT_SetThink3( work, TH3_COUNT_DANCE); }
	break;
    }
}
#if 0
// 解体中にファットマンが行なう余興:座る
static void Think3_DisposalSquat( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	npc->action.pad = PAD_SQUAT;	// 踊り
	
    }
    // 反応
    if ( work->nFatInfo & FAT_INFO_SEE_PL && work->pl_dis < 3000 ){	// 視界内にプレイヤーがいる
	FAT_SetNewAim( work, FAT_GetAreaDataZone( work ), NULL);
	FAT_SetThink3( work, TH3_MOVE);	// プレイヤー追撃
	return;
    }
    // モーション終了
    if ( work->npc.action.act_end ){
	FAT_SetThink3( work, TH3_READY);	// 思考分岐準備
	return;
    }

    work->count3++;
}
#endif

// 解体時の移動中判断
static int FAT_ChkThkDemineMove( Work* work)
{ 
    return 0; 
}


// 解体時のファットマン台詞
static void FAT_NavigateC4( Work* work)
{
    int			nSeed, nTalk;
    FAT_C4_DATA 	bombData;

    if ( work->bGameClear) return; // ファットマン瀕死では呼ばない

    nSeed = MAO_Rand(10);

    nTalk = FAT_STRM_BOMB_PROV_11;

    if ( FAT_SearchKaitaiC4( work, &bombData) ){ // 解体中C4を取得
	if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_07; // そうそこだ
	else		nTalk = FAT_STRM_BOMB_PROV_08; // 早く解体してみろ
    }else{
	int nDistBomb = FAT_SearchKaitaiC4DistNearest( work);
	if ( FAT_VisibleC4Check( work) ){ // Ｃ４が画面内にある
	    if ( nDistBomb < 2500 ){ // スプレー届く範囲
		if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_07; // そうそこだ
		else		nTalk = FAT_STRM_BOMB_PROV_08; // 早く解体してみろ
	    }else{
		if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_03; // どこをさがしている
		else		nTalk = FAT_STRM_BOMB_PROV;    // ばかめ
	    }
	}else{
	    if ( work->nPlDemineC4Time >= FAT_DEMINE_PROV_TIME ){ // なかなか解体できないでいる
		if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_10; // うろうろするだけか
		else		nTalk = FAT_STRM_BOMB_PROV_02; // のろまめ
	    }else{
		if ( nDistBomb < 2500 ){ // スプレー届く範囲
		    if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_03; // どこをさがしている
		    else		nTalk = FAT_STRM_BOMB_PROV;    // ばかめ
		}else{
		    if (nSeed < 5)	nTalk = FAT_STRM_BOMB_PROV_10; // うろうろするだけか
		    else		nTalk = FAT_STRM_BOMB_PROV_04; // そんなところで
		}
	    }
	}
    }

    // 音声をコール
    FAT_StreamRequestJustTime( work, nTalk);
    work->nStepCntr = work->nStrmLen[ nTalk ] + COUNT_VMODE(60);

    // ブランク更新
    if ( work->nPlDemineTalkBlank > 0 ) work->nPlDemineTalkBlank--;
}


// 解体中にファットマンが行なう余興:ダンス
static void Think3_DisposalDance( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	if ( BP_PS2_rand() % 2 == 0 )	npc->action.pad = PAD_CIRCLE_DANCE;	// 踊り1
	else			npc->action.pad = PAD_CIRCLE_DANCE2;	// 踊り2
	FAT_StreamRequestJustTime( work, FAT_STRM_PROV_01 + BP_PS2_rand() % 3 ); // わらう
    }

    // 反応
    if ( (work->nFatInfo & FAT_INFO_SEE_PL) ){	// 視界内にプレイヤーがいる
	if ( work->pl_dis < 3000 ){		// 最接近
	    int nBDZone;
	    if ( (nBDZone = FAT_GetBodyAttackZone( work )) != -1 ){	// 体当りエリアチェック
		FAT_SetNZoneWithRelayPoint( work, nBDZone, FAT_GetAreaDataZone( work ), NULL, NULL); // 対エリアに逃げる
		FAT_SetNextThink3( work, TH3_MOVE);
		FAT_SetThink3( work, TH3_BODYATTACK);
		return;
	    }else{
		FAT_SetNewAim( work, FAT_GetAreaDataZone( work ), NULL);
		FAT_SetThink3( work, TH3_MOVE);	// 逃げる
		return;
	    }
	}else if ( work->pl_dis < GLK_DIST ){	// 射程内
	    if ( work->nFatInfo & FAT_INFO_LOCKED && work->fat_parts != 0 ){	// ロックオンされた＆プレイヤー捕捉

	    }
	}
    }
#if 0
    if ( npc->action.current_mot != FAT_MOT_RUN_START ){ // 移動計算
	FVECTOR vecDistance;
	FMATRIX mat;

	DG_SetPos2( &DG_ZeroVector, &npc->ctrl->rot);
	DG_GetPos( &mat);

	vecDistance.vx = 0.f;
	vecDistance.vy = 0.f;
	vecDistance.vz = 100.f;
	vecDistance.vw = 1.f;

	_sceVu0ApplyMatrix( &npc->ctrl->step, &mat, &vecDistance );
    }
#endif    
    if ( work->npc.action.act_end ) {    
	if ( work->npc.action.current_mot == FAT_MOT_DANCE_1 ){
	    FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_ROLLER_CRASH, npc->action.pad ) ;
	    FAT_SetThink1( work, TH1_DAMAGE, TH2_START_DAMAGE, TH3_WAIT);
//	    FAT_StreamRequestJustTime( work, FAT_STRM_OTTOTO_01); // ぬおーっと
	    FAT_Say( work, SE_FATDMG02 + (BP_PS2_rand() % 2), GM_SEMODE_BOMB);	// 転倒ダメージ音
//	    FAT_SetThink3( work, TH3_STOP);	
	    return;
	}else{
	    FAT_SetThink3( work, TH3_STAND);	
	    return;
	}
    }

    // 方向:目標方向を向く
    //    npc->ctrl->turn.vy += 32;	//work->aim_dir;

    work->count3++;
}

// 解体中にファットマンが行なう余興:カモメ戯れ
static void Think3_DisposalKamome( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
    }
    
    work->bKamomeRest = 1;

    npc->action.pad = PAD_KAMOME;	// カモメ戯れ

    if ( work->count3 == COUNT_VMODE(60) ){
	FAT_Say( work, SE_YUBIBUE, GM_SEMODE_BOMB);	//ファットマン指笛
	FAT_CallKamome( work, HUMAN21_HIDARI_TE, 0, -40, 95, MAO_GetRandom(-512,512));
	FAT_CallKamome( work, HUMAN21_HIDARI_UDE2, 0,  0, 95, MAO_GetRandom(-512,512));
	FAT_CallKamome( work, HUMAN21_MIGI_UDE1, -40,  85, 0, MAO_GetRandom(-512,512));
    }

    if ( work->count3 == COUNT_VMODE(360) ){
	if ( GetConcKamomeNum() > 0 ){ // カモメが止まっていたら
	    if ( BP_PS2_rand() % 10 < 8)	FAT_StreamRequestJustTime( work, FAT_STRM_IMMOTAL_02);	// すーつをよごすなよ
	    else			FAT_Say( work, SE_FATWAH02, GM_SEMODE_BOMB);		// ぬふふ
	}else{
	    NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE ); 	     // ？ 
	}
    }

    // さみしいので
    if ( work->count3 > COUNT_VMODE(360) && work->count3 % COUNT_VMODE(360) == 0){
	FAT_NavigateC4( work);	// ナビゲート台詞
    }

    if ( !work->bBombActive || work->count3 > COUNT_VMODE(1200)){
	FAT_SetThink3( work, TH3_READY);	
	return;
    }

    // 方向:目標方向を向く
    npc->ctrl->turn.vy = 0;

    work->count3++;
}

// 解体中にファットマンが行なう余興:襟カモメ
static void Think3_DisposalKamomeNeck( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

   // 初期設定
    if ( work->count3 == 0 ){
    }

    work->bKamomeRest = 1;
    
    npc->action.pad = PAD_KAMOME;	// カモメ戯れ

    if ( work->count3 == COUNT_VMODE(60) ){
	FAT_Say( work, SE_YUBIBUE, GM_SEMODE_BOMB);	//ファットマン指笛
	FAT_CallKamome( work, HUMAN21_ATAMA, 0, 120, -200, MAO_GetRandom(-12,12));
    }

    if ( GetConcKamomeNum() > 0 ){ // カモメが止まっていたら
	// さみしいので
	if ( work->count3 == COUNT_VMODE(240) ){
	    FAT_Say( work, SE_FATWAH01 + MAO_Rand(2), GM_SEMODE_BOMB);	//ファットマン嘲笑
	}
    }
    // さみしいので
    if ( work->count3 == COUNT_VMODE(480) ){
	FAT_StreamRequestJustTime( work, FAT_STRM_WHERE_01); // 音声再生::どこにいった
	NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE ); 	     // ？ 
    }

    if ( !work->bBombActive || work->count3 > COUNT_VMODE(1200)){
	FAT_SetThink3( work, TH3_READY);	
	return;
    }

    // 方向:目標方向を向く
    npc->ctrl->turn.vy = 0;

    work->count3++;
}

// 背後に立つ
static void Think3_DisposalBackNavi( Work* work )
{
    
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	if ( FAT_CheckNearZoneIn( HZX_Zone1(npc->ctrl->addr), HZX_Zone1(GM_PlayerAddress) ) ){
	    FAT_StreamRequestJustTime( work, FAT_STRM_PROV_01 + (BP_PS2_rand() % 3)); // 音声再生
	}else{
	    FAT_SetThink3( work, TH3_READY );	// 準備
	    return;
	}

	work->nBullet = GLK_BUL_NUM;	
	work->nShootLimit = 20;
	work->nShootCntr  = 20;

	work->nLooseShootMin = 500;
	work->nLooseShootMax = 2000;	
	work->nLooseDecay    = 32;

	work->nStep     = 0;
	work->nStepCntr = work->nStrmCntr;
	_sceVu0CopyVector( &work->vecAimPos, &GM_PlayerPosition); // 目標をプレイヤーに
    }
    npc->action.pad = PAD_BACKNAVI; // 銃構え

    switch( work->nStep){
    case 0:
	if ( work->nStepCntr-- <= 0 ){
	    FAT_NavigateC4( work);	// ナビゲート台詞
	    work->nStep = 1;
	}
	break;
    case 1:
	if ( work->nStepCntr-- <= 0 ){
	    work->nStepCntr = COUNT_VMODE(120);
	    work->nStep = 2;
	}
	break;
    case 2: // 発射
	// グロック発射
	if ( work->count3 % 3 == 0 ){
	    FAT_ShootGlock( npc, 0, 10, 10); 
	    work->nShootCntr--;
	}
	// 外す距離を狭める
	if ( work->nLooseShootMax > work->nLooseShootMin ){
	    work->nLooseShootMax -= work->nLooseDecay;
	}
	if ( work->nStepCntr-- <= 0 ){
	    FAT_SetThink3FromNext( work, TH3_READY );	// 思考
	    return;
	}
	break;
    }
    {
	FAT_C4_DATA 	bombPLDemineData;			// プレイヤーが解体している爆弾
	FVECTOR 	vecAdjPos, vecAim;
	
	static FVECTOR vecY = { 0.f, 1.f, 0.f, 1.f};

	if ( FAT_SearchKaitaiC4( work, &bombPLDemineData) ){ // 解体中C4を取得
	    int 	nDir;
	    float 	fRad;
	    FVECTOR	vecRel;

	    nDir = MAO_Rand( 4096 );
	    fRad = (float)MAO_GetRandom( 400, 600);

	    MAO_RelPos( &vecRel, &npc->ctrl->mov, &bombPLDemineData.matPos);	// ファットマンのC４爆弾からの相対位置を出す

	    vecAdjPos.vx = _TS_Sin( nDir + 1024) * fRad;
	    // 相対位置からファットマン方向のX方向を取得
	    if ( vecRel.vx >= 0.f ) 	vecAdjPos.vx =  fabs( vecAdjPos.vx );
	    else			vecAdjPos.vx = -fabs( vecAdjPos.vx );
	    vecAdjPos.vy = _TS_Sin( nDir) * fRad;
	    vecAdjPos.vz = 0.f;
	    vecAdjPos.vw = 1.f;
	    // C4爆弾のマトリクスに絶対化
	    _sceVu0ApplyMatrix( &vecAim, &bombPLDemineData.matPos, &vecAdjPos);
	    
	    // 補間
	    MAO_InterpVec( &work->vecShootTrg, &work->vecShootTrg, &vecAim, 0.1f);
	}else{
	    float   fCos;
	    FMATRIX mat, matPlayer;

	    // プレイヤーのマトリクスを作成
	    DG_SetPos2( &work->vecAimPos, &GM_PlayerControl->rot);
	    DG_GetPos( &matPlayer);
	    fCos = _sceVu0InnerProduct( (FVECTOR*)&matPlayer.m[2][0], &work->vecPlaDiff); // 内積
	    if (GM_PlayerStatus & PLAYER_WATCH){ // 主観？
		if ( (GM_PlayerStatus & PLAYER_HOLD) && fCos < -0.9f  ){ // 狙われている
		    MAO_InterpVec( &work->vecShootTrg, &work->vecShootTrg, &work->vecAimPos, 0.1f);
		}else{
		    // プレイヤーの方向を向くマトリクスを生成
		    MAO_GetAimMatrix( &mat, &npc->ctrl->mov, &work->vecAimPos, &vecY); 
		    _sceVu0CopyVector( (FVECTOR*)&mat.m[3][0], &work->vecAimPos); // 位置はプレイヤー

		    if ( work->count3 >= COUNT_VMODE(60) ){
			if ( _sceVu0InnerProduct( (FVECTOR*)&matPlayer.m[2][0], (FVECTOR*)&mat.m[0][0]) >= 0.f){
			    vecAdjPos.vx = (float)work->nLooseShootMax;
			}else{
			    vecAdjPos.vx = (float)-work->nLooseShootMax;
			}
			vecAdjPos.vy = 0.f;
			vecAdjPos.vz = 0.f;
			vecAdjPos.vw = 1.f;
		    }else{
			_sceVu0CopyVector( &vecAdjPos, &DG_ZeroVector);
		    }
		    _sceVu0ApplyMatrix( &vecAim, &mat, &vecAdjPos);
		    // 補間
		    MAO_InterpVec( &work->vecShootTrg, &work->vecShootTrg, &vecAim, 0.1f);
		}
	    }else{
		{
		    // プレイヤーの方向を向くマトリクスを生成
		    MAO_GetAimMatrix( &mat, &npc->ctrl->mov, &work->vecAimPos, &vecY); 
		    _sceVu0CopyVector( (FVECTOR*)&mat.m[3][0], &work->vecAimPos); // 位置はプレイヤー

		    if ( work->count3 >= COUNT_VMODE(60) ){
			if ( _sceVu0InnerProduct( (FVECTOR*)&matPlayer.m[2][0], (FVECTOR*)&mat.m[0][0]) >= 0.f){
			    vecAdjPos.vx = (float)work->nLooseShootMax;
			}else{
			    vecAdjPos.vx = (float)-work->nLooseShootMax;
			}
			vecAdjPos.vy = 0.f;
			vecAdjPos.vz = 0.f;
			vecAdjPos.vw = 1.f;
		    }else{
			_sceVu0CopyVector( &vecAdjPos, &DG_ZeroVector);
		    }
		    _sceVu0ApplyMatrix( &vecAim, &mat, &vecAdjPos);
		    
		    // 補間
		    MAO_InterpVec( &work->vecShootTrg, &work->vecShootTrg, &vecAim, 0.5f);
		}
		MAO_InterpVec( &work->vecAimPos, &work->vecAimPos, &GM_PlayerPosition, 0.4f); // 目標をプレイヤーに
	    }
	}
	FAT_AdjustAimPoint( work, &work->vecShootTrg ); // 追従
    }

    // 方向:目標方向を向く
    npc->action.dir = work->pl_dir;

    work->count3++;
}

// 爆弾守備まちぶせ
static void Think3_CountDownBombGuard( Work* work )
{
    FVECTOR vec;
    FMATRIX mat;
    
    static FVECTOR vecY = { 0.f, 1.f, 0.f, 1.f};
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){	
	work->nStep     = 0;
	work->nStepCntr = 0;

	work->nBullet = GLK_BUL_NUM;	
	work->nShootLimit = 20;
	work->nShootCntr  = 20;

	work->nLooseShootMin = 500;
	work->nLooseShootMax = 2000;	
	work->nLooseDecay    = 32;
    }   
    npc->action.pad = PAD_SHOT_READY;

    switch( work->nStep ){
    case 0: // 索敵
	if ( work->nStepCntr == COUNT_VMODE(300)){
	    int nSeed;
	    nSeed = BP_PS2_rand() % 2;
	    if ( nSeed == 0) 	  FAT_StreamRequestJustTime( work, FAT_STRM_WHERE_01); 
	    else 		  FAT_StreamRequestJustTime( work, FAT_STRM_WHERE_02);
	}

	if ( work->count3 >= COUNT_VMODE(1800) ) {    // タイムアウト
	    FAT_SetThink3( work, TH3_READY);	// 思考分岐へ
	    return;
	}
	if ( work->nFatInfo & FAT_INFO_SEE_PL ){ // 見えている
	    int nSeed;
	    work->nStep     = 1;
	    work->nStepCntr = 0;
	    
	    nSeed = BP_PS2_rand() % 10;
	    if ( nSeed < 2) 	  FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_PROV_08); 
	    else if ( nSeed < 4)  FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_PROV_11); 	
	    else if ( nSeed < 6)  FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_SET1); 	
	    else if ( nSeed < 8)  FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_PROV_02); 	
	    else 		  FAT_StreamRequestJustTime( work, FAT_STRM_PROV_02); 	
	}
	break;
    case 1: // 攻撃
	vec = DG_ZeroVector;
	vec.vy = (float)work->pl_dis - 6500.f;
	if ( vec.vy > 1000.f) vec.vy = 1000.f;
	else if ( vec.vy < 0.f)  vec.vy = 0.f;
	// プレイヤーの方向を向くマトリクスを生成
	MAO_GetAimMatrix( &mat, &npc->ctrl->mov, &GM_PlayerPosition, &vecY); 
	_sceVu0CopyVector( (FVECTOR*)&mat.m[3][0], &GM_PlayerPosition); // 位置はプレイヤー

	_sceVu0ApplyMatrix( &vec, &mat, &vec);
	
	MAO_InterpVec( &work->vecShootTrg, &work->vecShootTrg, &vec, 0.2f); // 目標をプレイヤーに
	FAT_AdjustAimPoint( work, &work->vecShootTrg ); // 追従
	// グロック発射
	if ( work->nStepCntr >= 0 && work->nStepCntr % 3 == 0 ){
	    FAT_ShootGlock( npc, 0, 10, 30); 
	    work->nShootCntr--;
	}

	// 弾切れ
	if ( work->nBullet == 0){
	    FAT_SetNextThink3( work, TH3_COUNT_BOMBGUARD);
	    FAT_SetThink3( work, TH3_RELOAD);
	    return;
	}

	break;
    }

    if ( work->pdatGuard == NULL || work->pdatGuard->nID == -1 || work->pdatGuard->nStatus < 0){
	FAT_SetThink3( work, TH3_READY);	// 思考分岐へ
	return;
    }

    npc->action.dir = work->pl_dir;

    work->nStepCntr++;
    work->count3++;
}
