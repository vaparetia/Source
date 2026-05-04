/*
    kmginfo.c
    かもめマネージャ情報関連処理
    2001/05/15 Masafumi Okuta
    $Id: kmginfo.c,v 1.1.1.3 2002/11/19 11:48:03 Yoshizawa1 Exp $
*/

static void KMM_CheckNoiseTakeOff( KAMOME_MNG*);
static void KMM_SetTakeOffOperation( KAMOME_MNG* kmmng, int code, FVECTOR* pvecPos, int nRadius);
static void KMM_SetLandCBoxOperation( KAMOME_MNG* kmmng);

static void KMMNG_UpdateInfo( KAMOME_MNG* kmmng)
{
    KAMOME* pKamome;
    
    kmmng->nPrevInfo = kmmng->nInfo;
    kmmng->nInfo = 0;	// クリア
    kmmng->nCboxNum = 0;

    // メッセージ関連
    {	
	int i;
	
	for ( i = 0 ; i < KMNG_MESS_MAX; i++){
	    kmmng->ucMsgCntr[i] = 0x00; // メッセージカウンタクリア
	    kmmng->ucMsgMax[i]  = 0x00; // メッセージ最大数クリア
	}
    }


    // ノイズチェック
    switch (GM_NoisePower){
    case NOISE_DEC:
    case NOISE_SS:
    case NOISE_S:
    case NOISE_CHOUDAN:
    case NOISE_SCREAM:
	kmmng->nInfo |= KMM_INFO_NOISE_S;		// 音が発生
	kmmng->vecNoise = GM_NoisePosition;
	break;
    case NOISE_MM:
    case NOISE_M:
	kmmng->nInfo |= KMM_INFO_NOISE_M;		// 音が発生
	kmmng->vecNoise = GM_NoisePosition;
	break;
    case NOISE_L:
    case NOISE_LL:
	kmmng->nInfo |= KMM_INFO_NOISE_L;		// 音が発生
	kmmng->vecNoise = GM_NoisePosition;
	break;
    }

    // チャフチェック
    if ( !( kmmng->nPrevGameStatus & STATE_CHAFF) && (GM_GameStatus & STATE_CHAFF) ){
	kmmng->nInfo |= KMM_INFO_NOISE_L;		// 音で対応
	kmmng->vecNoise = GM_PlayerPosition;
    }

    // かもめ達の状態取得
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	if ( pKamome->think.nThink1 == TH1_DAMAGE ){
	    kmmng->nInfo |= KMM_INFO_CAUTION;		// 仲間が撃たれた
	    kmmng->vecCaution = pKamome->mov;
	}
	if ( pKamome->think.nThink3 == TH3_ATCMOVE ){
	    kmmng->nInfo |= KMM_INFO_ATTACK_MOVE;	// 攻撃移動中のかもめがいる
	}
	if ( pKamome->think.nThink3 == TH3_ATTACK ){
	    kmmng->nInfo |= KMM_INFO_ATTACK;		// 攻撃かもめがいる
	}
	if ( pKamome->think.nThink3 >= TH3_CHECK_CBOX_START && pKamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	    kmmng->nInfo |= KMM_INFO_CBOX;		// 段ボールかもめがいる
	    kmmng->nCboxNum++;
	}
    }

    // だんぼーる
    if (  GM_PlayerBoxBody != NULL && (GM_PlayerStatus & PLAYER_CB_BOX) && 
	  !(GM_PlayerStatus & PLAYER_CBBOX_RUN) && !(GM_PlayerStatus & PLAYER_MOVE) && 
	  !(GM_PlayerStatus & PLAYER_CB_BOX_STAND) && !(GM_PlayerStatus & PLAYER_CB_BOX_CANCELED)){
	if ( kmmng->nCboxCntr < KMM_CBOX_TIME ){
	    kmmng->nCboxCntr++;
	}
//	kmmng->nInfo |= KMM_INFO_PL_CBOX;	// 段ボール着地可能
    }else{
	kmmng->nCboxCntr = 0;
    }

    // ZOE段ボール ＆ ステルスチェック
    if ( PL_GetPlayerItem() == IT_CBBoxE || (GM_PlayerStatus & PLAYER_STEALTH) ){
	kmmng->nInfo |= KMM_INFO_PL_EYE_CBOX;
    }

    // 攻撃チェック
    if ( kmmng->nAngryLimit >= KMM_ANGRY_NUM){
	KMMNG_SendMessageNumAttackPlayer( kmmng );
	kmmng->nAngryLimit = 0;
    }


    // エルードチェック
    if ( GM_PlayerBody != NULL && (GM_PlayerStatus & PLAYER_BEYOND)){
	FVECTOR vecPos;
	// 捕まり位置の生成
	_sceVu0AddVector( &vecPos, (FVECTOR*)&GM_PlayerBody->objs->objs[HUMAN21_MIGI_TE].world.m[3][0],
			  (FVECTOR*)&GM_PlayerBody->objs->objs[HUMAN21_HIDARI_TE].world.m[3][0]);
	_sceVu0ScaleVector( &kmmng->vecElude, &vecPos, 0.5f);

    }

    // 死体またぎ
    if ( !(GM_PlayerStatus & PLAYER_ON_CORPSE) && (GM_PlayerStatus & PLAYER_BEHIND) ){
	FVECTOR vecBound1, vecBound2;
	vecBound1.vx = GM_PlayerPosition.vx - KMM_CORP_CHK_W;
	vecBound1.vy = GM_PlayerPosition.vy - KMM_CORP_CHK_H;
	vecBound1.vz = GM_PlayerPosition.vz - KMM_CORP_CHK_W;
	vecBound2.vx = GM_PlayerPosition.vx + KMM_CORP_CHK_W;
	vecBound2.vy = GM_PlayerPosition.vy + KMM_CORP_CHK_H;
	vecBound2.vz = GM_PlayerPosition.vz + KMM_CORP_CHK_W;
	for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	    if ( pKamome->think.nThink3 < TH3_CHECK_SLEEP_START ) continue;	// 通常時はみない

	    if ( MAO_BoundCheck( &pKamome->mov, &vecBound1, &vecBound2 ) ){
		GM_PlayerStatus |= PLAYER_ON_CORPSE;	
		return;
	    }
	}
    }
    // 一フレーム前のゲームステータス取得
    kmmng->nPrevGameStatus = STATE_CHAFF;
}
// 基本:計算量少ないver
static void KMMNG_BaseLight( KAMOME_MNG* kmmng )
{
    if ( kmmng->nInfo & KMM_INFO_CAUTION ){ // 仲間が攻撃された
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecCaution, 2000.f);
	return;
    }

    // 音反応チェック
    KMM_CheckNoiseTakeOff( kmmng);

}
// 基本:ずうずうしい
static void KMMNG_BaseBardy( KAMOME_MNG* kmmng )
{
    if ( kmmng->nInfo & KMM_INFO_NOISE_M ){ 	  // 銃声レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 2000.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_L ){ // 爆発音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 5000.f);
	return;
    }

    // 音反応チェック
    KMM_CheckNoiseTakeOff( kmmng);

    if ( kmmng->nInfo & KMM_INFO_CAUTION ){ // 仲間が攻撃された
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecCaution, 5000.f);
	return;
    }

}
// 基本:敏感
static void KMMNG_BaseEscape( KAMOME_MNG* kmmng )
{
    // 最接近すると飛び立つ
    KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &GM_PlayerPosition, 3000.f);

    // 近距離でプレイヤーが走る,パンチ,キックを出すと飛び立つ

    if ( kmmng->nInfo & KMM_INFO_NOISE_S ){ 	  // 物音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 2000.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_M ){ // 銃声レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 3000.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_L ){ // 爆発音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 5000.f);
	return;
    }

    // 音反応チェック
    KMM_CheckNoiseTakeOff( kmmng);

    if ( kmmng->nInfo & KMM_INFO_CAUTION ){ // 仲間が攻撃されてた
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecCaution, 5000.f);
	return;
    }
}
// 基本:怒り
static void KMMNG_BaseAngry( KAMOME_MNG* kmmng )
{
    // 最接近すると飛び立つ

    // 近距離でプレイヤーが走る,パンチ,キックを出すと飛び立つ

    if ( kmmng->nInfo & KMM_INFO_NOISE_M ){ // 銃声レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 3000.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_L ){ // 爆発音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 5000.f);
	return;
    }

    if ( kmmng->nInfo & KMM_INFO_CAUTION ){ // 仲間が攻撃されてた
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecCaution, 5000.f);
	return;
    }


    // 音反応チェック
    KMM_CheckNoiseTakeOff( kmmng);

}
// 音反応飛び立ち判定
static void KMM_CheckNoiseTakeOff( KAMOME_MNG* kmmng)
{
    if ( kmmng->nInfo & KMM_INFO_NOISE_S ){ 	  // 物音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 2500.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_M ){ // 銃声レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 5000.f);
	return;
    }else if ( kmmng->nInfo & KMM_INFO_NOISE_L ){ // 爆発音レベル
	KMM_SetTakeOffOperation( kmmng, KMNG_MESS_ALL_TAKEOFF, &kmmng->vecNoise, 10000.f);
	return;
    }
}
// カモメマネージャ基本判断
static void KMMNG_JudgeBase( KAMOME_MNG* kmmng )
{
    switch ( kmmng->nBaseRoutine ){
    case KMNG_BASE_LIGHT:	// 基本状態:計算量少ないver
	KMMNG_BaseLight( kmmng);	
	break;
    case KMNG_BASE_BARDY: 	// 基本状態:ずうずうしい
	KMMNG_BaseBardy( kmmng);	
	break;
    case KMNG_BASE_ESCAPE: 	// 基本状態:敏感
	KMMNG_BaseEscape( kmmng);	
	break;
    case KMNG_BASE_ANGRY: 	// 基本状態:怒り
	KMMNG_BaseAngry( kmmng);	
	break;
    case KMNG_BASE_NORMAL: 	// 基本状態:特徴なし
    default:			// 基本状態:その他
	// 音反応チェック
	KMM_CheckNoiseTakeOff( kmmng);
	break;
    }
    // 段ボール
    if ( kmmng->nCboxCntr >= KMM_CBOX_TIME ) KMM_SetLandCBoxOperation( kmmng);
	
}

// カモメがダメージ受けた時に呼ばれるプロシージャ
static void KMM_CallDmgProc( KAMOME_MNG* kmmng, int nType)
{
    // プロシージャコール
    if ( kmmng->procDamage != 0){
	GCL_ARGS  gcl_args;
	int       data;
	
	data = nType; // 種類を渡す
	gcl_args.argc = 1;
	gcl_args.argv = &data;
	GCL_ExecProc( kmmng->procDamage, &gcl_args ); 
    }
}
