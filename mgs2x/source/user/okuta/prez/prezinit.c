/*
    prezinit.c
    ニキータイベント用大統領初期化関連
    2001/04/26 Masafumi Okuta
    $Id: prezinit.c,v 1.1.1.3 2002/11/19 11:48:09 Yoshizawa1 Exp $
*/

// 顔アニメ初期化
static void PREZ_InitFaceAnime( Work* work )
{
    extern void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id );
    work->nFaceChara = FACEANIME_NAME;
    NewFaceAnimation( work->nFaceChara, work->body.evmobj, PREZ_FAR_NAME);
}

// ニキータ情報ワークの初期化
static void PREZ_InitNikitaInfoWork(Work* work)
{
    NPCWORK* npc;
    char* opt;
    int i,j;

    npc = &work->npc;

    work->nDifficult = GM_GameLevel;

    work->bDead = 0;

    printf("PRESIDENT-GAME LEVEL == %d\n", work->nDifficult);

    work->nDownTime = 0;
    work->nDownCntr = COUNT_VMODE(120);

    work->nComplaintCntr = 0;
    work->nComplaintMax  = COUNT_VMODE(1200);

    work->procOver 	 = GCL_GetOptionValue( 'o', 0 );
    work->procFirstKnock = GCL_GetOptionValue( 's', 0 );
    work->nReactNikDist  = GCL_GetOptionValue( 'd', 0 );
    work->nReactBombDist = GCL_GetOptionValue( 'b', 0 );
    work->nNoiseTime 	 = GCL_GetOptionValue( 'e', 0 );
    work->nNoticeTime 	 = GCL_GetOptionValue( 'k', 0 );
    work->nPanicTime	 = GCL_GetOptionValue( 't', 0 );
    if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){	
	if ( ENE_GCL_GetFV( opt, &work->vecRelay ) == 0 ){
	    work->nRelayZone = HZX_Zone1(HZX_GetAddress( npc->ctrl->hzx_id, &work->vecRelay, -1));
	}else{
	    work->nRelayZone = -1;
	    work->vecRelay = DG_ZeroVector;
	}
    }
    // 聞き耳ポイント
    work->nNoisePosNum = 0;
    if ( ( opt = GCL_GetOption( 'n' ) ) != NULL ){	
	for ( i = 0; i < PREZ_NOISE_POS_MAX; i++){
	    if ( ENE_GCL_GetFV( opt, &work->vecNoisePos[i] ) == 0 ){
		work->nNoiseZone[i] = HZX_Zone1(HZX_GetAddress( npc->ctrl->hzx_id, &work->vecNoisePos[i], -1));
		work->nNoisePosNum++;
	    }else{
		work->vecNoisePos[i] = DG_ZeroVector;
		work->nNoiseZone[i] = -1;
	    }
	}
    }else{
	for ( i = 0; i < PREZ_NOISE_POS_MAX; i++){
	    work->vecNoisePos[i] = DG_ZeroVector;
	    work->nNoiseZone[i] = -1;
	}
    }
    // 聞き耳方向
    if ( ( opt = GCL_GetOption( 'w' ) ) != NULL ){	
	for ( i = 0; i < PREZ_NOISE_POS_MAX; i++){
	    if ( GCL_NextStr() != NULL ){
		work->nNoiseDir[i] = GCL_GetNextInt();
	    }else{
		work->nNoiseDir[i] = 0;
	    }
	}
    }else{
	for ( i = 0; i < PREZ_NOISE_POS_MAX; i++){
	    work->nNoiseDir[i] = 0;
	}
    }

    // パニックポイント
    work->nPanicPosNum = 0;
    if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){	
	for ( i = 0; i < PREZ_PANIC_POS_MAX; i++){
	    if ( ENE_GCL_GetFV( opt, &work->vecPanicPos[i] ) == 0 ){
		work->nPanicZone[i] = HZX_Zone1(HZX_GetAddress( npc->ctrl->hzx_id, &work->vecPanicPos[i], -1));
		work->nPanicPosNum++;
printf("Panic Pos %d : %d\n", i, PREZ_GetArea( &work->vecPanicPos[i]));
	    }else{
		work->vecPanicPos[i] = DG_ZeroVector;
		work->nPanicZone[i] = -1;
	    }
	}
    }else{
	for ( i = 0; i < PREZ_PANIC_POS_MAX; i++){
	    work->vecPanicPos[i] = DG_ZeroVector;
	    work->nPanicZone[i] = -1;
	}
    }

    // 逃げエリア
    if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){	
	for ( i = 0; i < PREZ_AREA_MAX; i++){
	    for ( j = 0; j < PREZ_AREA_MAX; j++){
		if ( GCL_NextStr() != NULL ){
		    work->nEscArea[i][j] = GCL_GetNextInt();
		}else{
		    work->nEscArea[i][j] = -1;
		}
	    }
	}
    }else{
	for ( i = 0; i < PREZ_AREA_MAX; i++){
	    for ( j = 0; j < PREZ_AREA_MAX; j++){
		work->nEscArea[i][j] = -1;
	    }
	}
    }

    // 反応
    if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){	
	for ( i = 0; i < PREZ_AREA_MAX; i++){
	    for ( j = 0; j < PREZ_AREA_MAX; j++){
		if ( GCL_NextStr() != NULL ){
		    work->nReact[i][j] = GCL_GetNextInt();
		}else{
		    work->nReact[i][j] = -1;
		}
	    }
	}
    }else{
	for ( i = 0; i < PREZ_AREA_MAX; i++){
	    for ( j = 0; j < PREZ_AREA_MAX; j++){
		work->nReact[i][j] = -1;
	    }
	}
    }

    // ノイズ検知トラップ
    if ( ( opt = GCL_GetOption( 'g' ) ) != NULL ){	
	int nNoiseTrap;
	if ( GCL_NextStr() != NULL ){
	    HZX_BLOCK* 	blk;
	    HZX_TRP* 	trp;

	    nNoiseTrap = GCL_GetNextInt();
	    HZX_FindTrap(GM_GetHzxGroupID( work->npc.ctrl->map ), nNoiseTrap, &blk, &trp);
	    
	    if ( blk == NULL || trp == NULL ){
		_sceVu0CopyVector( &work->vecNoiseTrap[0], &DG_ZeroVector);
		_sceVu0CopyVector( &work->vecNoiseTrap[1], &DG_ZeroVector);
		printf("Noise trap not found!!\n");
	    }

	    work->vecNoiseTrap[0].vx = (float)(blk->tx + trp->b1.vx);
	    work->vecNoiseTrap[0].vy = (float)(blk->ty + trp->b1.vy);
	    work->vecNoiseTrap[0].vz = (float)(blk->tz + trp->b1.vz);
	    work->vecNoiseTrap[0].vw = 1.f;
	    work->vecNoiseTrap[1].vx = (float)(blk->tx + trp->b2.vx);
	    work->vecNoiseTrap[1].vy = (float)(blk->ty + trp->b2.vy);
	    work->vecNoiseTrap[1].vz = (float)(blk->tz + trp->b2.vz);
	    work->vecNoiseTrap[1].vw = 1.f;
	}
    }else{
	_sceVu0CopyVector( &work->vecNoiseTrap[0], &DG_ZeroVector);
	_sceVu0CopyVector( &work->vecNoiseTrap[1], &DG_ZeroVector);
#ifdef DEBUG_MODE
	printf("Noise trap GCL Data not found!!\n");
#endif
    }
    
#ifdef DEBUG_MODE
    printf("PREZ::GCL-DATA ---------------- start\n");
    printf("MaxLife    = %d\n", work->nMaxLife);
    printf("GameOver   = %d\n", work->procOver );
    printf("NikitaDist = %d\n", work->nReactNikDist);
    printf("BombDist   = %d\n", work->nReactBombDist);
    printf("NoiseTime  = %d\n", work->nNoiseTime);
    printf("PanicTime  = %d\n", work->nPanicTime);
    printf("NoiseNum   = %d\n", work->nNoisePosNum);
    printf("PanicNum   = %d\n", work->nPanicPosNum);
    printf("PREZ::GCL-DATA ---------------- end\n");
#endif

}

// サウンド関連ワーク
static void PREZ_InitSoundWork( Work* work )
{
    int i;
    int nNoNum;
    int nFlagNum;
    int nLenNum;
//    extern	int	DG_TickCount;			/* 起動からの総VSyncカウント数 */

    // local param init
    nNoNum = 0;
    nFlagNum  = 0;
    nLenNum   = 0;

    // sound work init
    work->nSeCntr	 = 0;
    work->nStrmHandle 	 = -1;
    work->nStrmIndex	 = -1;
    work->nStrmIndexNext = -1;
    work->bStrmProcFlag  = 0;
    work->nStrmProc 	 = 0;
    work->nStrmCntr   	 = 0;
    work->nSoundID 	 = GCL_GetOptionValue( 's', 32 ); // ファットマンのサウンドキャラ判別ID

    NPC_SetSeID( &work->npc, work->nSoundID);	// サウンド用ID設定
printf("SOUND ----- ID ------- %d\n", work->nSoundID);								 

    // ストリーム音声番号の登録 
    if ( !GCL_GetOption( 'v' ) ){
	printf( "president GCL -v option not found \n" );
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){
	    work->nStrmNo[i] = 0;
	}
    }else{
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){	
	    if ( GCL_NextStr() != NULL ){
		work->nStrmNo[i] = GCL_GetNextInt();
		nNoNum++;
	    }else{
		work->nStrmNo[i] = 0;
	    }
	}
    }
    // ストリーム音声フラグの登録 
    if ( !GCL_GetOption( 'f' ) ){
	printf( "president GCL -f option not found \n" );
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){
	    work->nStrmFlag[i] = 0;
	}
    }else{
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){
	    if ( GCL_NextStr() != NULL ){
		work->nStrmFlag[i] = GCL_GetNextInt();
		nFlagNum++;
	    }else{
		work->nStrmFlag[i] = 0;
	    }
	}
    }
    // ストリーム音声長さの登録 
    if ( !GCL_GetOption( 'l' ) ){
	printf( "president GCL -l option not found \n" );
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){
	    work->nStrmLen[i] = 0;
	}
    }else{
	for( i = 0 ; i < PREZ_STRM_MAX ; i++ ){
	    if ( GCL_NextStr() != NULL ){
		work->nStrmLen[i] = (GCL_GetNextInt() / TIME_BASE) + 1;
		nLenNum++;
	    }else{
		work->nStrmLen[i] = 0;
	    }
	}
    }

#ifdef DEBUG_MODE
    printf("GCL sound data start------------------\n");
    printf("No   Num = %d\n", nNoNum);
    printf("Flag Num = %d\n", nFlagNum);
    printf("Len  Num = %d\n", nLenNum);
    printf("GCL sound data end--------------------\n");
#endif

    // データ数取得
    work->nStrmDataNum = nNoNum;
}

// ワークの初期化
static void PREZ_WorkInit(Work* work)
{
    PREZ_InitFaceAnime( work);
    PREZ_InitNikitaInfoWork( work );
    PREZ_InitSoundWork( work );

    work->nNoiseCntr 		= 0;
    work->nNoiseLevel 		= 0;
    work->bFirstKnockEnd 	= 0;

    work->control.s_sphere = 1000; // 壁検出距離

#ifdef DEBUG_MODE
    PREZ_CallDbgLevel();	// 難易度
    PREZ_CallDbgDump(); 	// デバック
    PREZ_CallDbgTrg();		// ターゲット
    PREZ_CallDbgMuteki();	// 無敵フラグ

    PREZ_DbgDrawTrgBox( work ); // ターゲット表示
#endif

}




