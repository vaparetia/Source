/*
    preznoise.c
    ニキータ大統領音反応関連
    2001/04/24 Masafumi Okuta
    $Id: preznoise.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/

// ノイズレベル更新
static void PREZ_AddNoiseLevel( Work* work, int nTime)
{
    if ( work->nNoiseLevel < PREZ_NOISE_LEVEL_MAX ){
	work->nNoiseLevel++;
	work->nNoiseCntr = nTime;
    }
}

// 聞き耳モード
#define LISTEN_RELAY_DIST (5000) // 中継地点を設ける距離
static void PREZ_SetKnockMode(Work* work)
{
    int i, nRes;
    int nDist, nDistTmp;
    FVECTOR vec;
    NPCWORK* npc;

    npc = &work->npc;

    nRes = -1;
    nDist = 100000;
    for ( i =0; i < work->nNoisePosNum; i++){
	MAO_GetDiffVec3( NULL, &nDistTmp, NULL, &GM_NoisePosition, &work->vecNoisePos[i]);
	if ( nDistTmp < nDist ){
	    nDist = nDistTmp;
	    nRes = i;
	    vec = work->vecNoisePos[i];
	}
    }

    if (nRes != -1){
	int nRelayDist;
	// 目的地までの距離をはかる
	MAO_GetDiffVec3( NULL, &nRelayDist, NULL, &work->control.mov, &work->vecNoisePos[nRes]);
	if ( nRelayDist > LISTEN_RELAY_DIST && nRelayDist < nDist ){ // 中継をはさむ
	    NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE); // ？
	    work->nNextZone = work->nRelayZone;
	    work->vecNextPos = work->vecRelay;
	    PREZ_SetNextThink3( work, TH3_OVERLOOK);
	    PREZ_SetThink3( work, TH3_MOVE);
	    return;
	}else{ // 聞き耳ポイントへ移動
	    if ( !work->bFirstKnockEnd ){ // 初回音反応デモへ
		GCL_ARGS  gcl_args;
		int       data[3]; // 大統領の位置を与える

		data[0] = (int)npc->ctrl->mov.vx;
		data[1] = (int)npc->ctrl->mov.vy;
		data[2] = (int)npc->ctrl->mov.vz;
		gcl_args.argc = 3;
		gcl_args.argv = &data[0];
		GCL_ExecProc( work->procFirstKnock, &gcl_args ); // 初回音反応デモ呼出
		work->bFirstKnockEnd = 1; // 初回音反応デモ終了フラグを立てる
	    }
//	    NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE); // ？
	    work->nNextZone = work->nNoiseZone[nRes];
	    work->vecNextPos = vec;
	    PREZ_SetThink3( work, TH3_LISTEN_WAIT); // 
	    return;
	}
    }

}

// 爆弾モード
#define PREZ_DIST_BOMB_FIND (5000) // 視界外でも爆発地点を理解する距離
static void PREZ_SetBombMode(Work* work)
{
    int i, nRes;
    int nDist, nDistTmp;
    FVECTOR vec;
    NPCWORK* npc;

    npc = &work->npc;

//printf("**************************** sound mode\n");	

    if ( work->think3 == TH3_SLEEP){
	NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE); // ？
	PREZ_SetThink3( work, TH3_AWAKE);
	return;
    }else{
	if ( MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){ // 部屋の中で爆発
	    int nDir;
	    MAO_GetDiffVec3( NULL, &nDist, &nDir, &work->vecFacePos, &GM_NoisePosition);
	    if ( nDist < PREZ_DIST_BOMB_FIND ||
		 !ENE_EyeOnlineCheck( npc->ctrl->hzx_id, &work->vecFacePos, &GM_NoisePosition ) ){ // 爆発地点が見える
		// びっくり->怖がる
		npc->ctrl->turn.vy = nDir; // 爆発地点を向く
		PREZ_SetNextThink3( work, TH3_SCARED);	
		PREZ_SetThink2( work, TH2_PANIC, TH3_FIND);
//printf("FIND->SCARED\n");	
	    }else{
		// あたりを見渡す
		NPC_CallHeadMark( npc, HMK2_TYPE_RED_AT); // !
		work->nNextZone = work->nRelayZone;
		work->vecNextPos = work->vecRelay;
		PREZ_SetNextThink3( work, TH3_OVERLOOK);
		PREZ_SetThink3( work, TH3_MOVE);
//printf("MOVE->OVERLOOK\n");	
	    }
	    return;
	}else{
	    // 聞き耳ポイントへ移動
	    nRes = -1;
	    nDist = 100000;
	    for ( i =0; i < work->nNoisePosNum; i++){
		MAO_GetDiffVec3( NULL, &nDistTmp, NULL, &GM_NoisePosition, &work->vecNoisePos[i]);
		if ( nDistTmp < nDist ){
		    nDist = nDistTmp;
		    nRes = work->nNoiseZone[i];
		    vec = work->vecNoisePos[i];
		}
	    }

	    if (nRes != -1){
		work->nNextZone = nRes;
		work->vecNextPos = vec;
		PREZ_SetThink3( work, TH3_LISTEN_WAIT); // 
//printf("cannot look ---- TO-POINT\n");	
	    }else{
		// あたりを見渡す
		work->nNextZone = work->nRelayZone;
		work->vecNextPos = work->vecRelay;
		PREZ_SetNextThink3( work, TH3_OVERLOOK);
		PREZ_SetThink3( work, TH3_MOVE);
//printf("cannot look ---- MOVE->OVERLOOK\n");	
	    }
	    return;
	}
    }

}

// 音チェック
static int PREZ_CheckNoise(Work* work)
{

    switch (GM_NoisePower){
    case NOISE_S:
    case NOISE_SS:
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}
	PREZ_SetKnockMode( work );
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	{
	    if ( work->nNoiseLevel ==  PREZ_NOISE_LEVEL_MAX ){
		PREZ_StreamRequestJustTime( work, PREZ_STRM_NOISE05); // 音声
	    }else{
		PREZ_StreamRequestJustTime( work, PREZ_STRM_NOISE01 + BP_PS2_rand() % 4); // 音声
	    }
	}	
	return 1;
	break;
#if 0
    case NOISE_MM:
    case NOISE_M:
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}
//printf("M --- noise %d\n", GM_NoisePower);
	PREZ_SetKnockMode( work );
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	PREZ_StreamRequest( work, PREZ_STRM_NOISE01 + BP_PS2_rand() % 4); // 音声
	return 1;
	break;
#endif
    case NOISE_L:
//printf("L --- noise %d\n", GM_NoisePower);
	PREZ_SetBombMode( work );
	work->vecLastBombPos = GM_NoisePosition;
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	PREZ_StreamRequest( work, PREZ_STRM_PANIC01); // 音声
	return 1;
	break;
    }

    
    return 0;
}

// 音チェック
static int PREZ_CheckNoiseInListen(Work* work)
{
    switch (GM_NoisePower){
    case NOISE_S:
    case NOISE_SS:
//printf("S +++ noise %d\n", GM_NoisePower);
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	if ( BP_PS2_rand() % 10 == 0 ){
	    if ( work->nNoiseLevel ==  PREZ_NOISE_LEVEL_MAX ){
		PREZ_StreamRequestJustTime( work, PREZ_STRM_NOISE05); // 音声
	    }else{
		PREZ_StreamRequestJustTime( work, PREZ_STRM_NOISE01 + BP_PS2_rand() % 4); // 音声
	    }
	}	
	return 1;
	break;
#if 0
    case NOISE_MM:
    case NOISE_M:
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}
//printf("M +++ noise %d\n", GM_NoisePower);
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	return 1;
	break;
#endif
    case NOISE_L:
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}
//printf("L +++ noise %d\n", GM_NoisePower);
	PREZ_SetBombMode( work );
	work->vecLastBombPos = GM_NoisePosition;
	PREZ_AddNoiseLevel( work, PREZ_NOISE_LEVEL_TIME);
	return 1;
	break;
    }

    return 0;
}

// ねている時の音チェック
static int PREZ_CheckNoiseInSleep(Work* work)
{
    switch (GM_NoisePower){
    case NOISE_L:
	if ( !MAO_BoundCheck( &GM_NoisePosition, &work->vecNoiseTrap[0], &work->vecNoiseTrap[1]) ){
	    return 0;
	}

	PREZ_SetBombMode( work );


	work->vecLastBombPos = GM_NoisePosition;
	return 1;
	break;
    }
    return 0;
}
