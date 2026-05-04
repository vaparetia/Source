/*
    parinit.c
    オウム初期化関連
    2001/04/26 Masafumi Okuta
    $Id: parinit.c,v 1.1.1.3 2002/11/19 11:48:06 Yoshizawa1 Exp $
*/

// サウンド関連ワーク
static void PAR_InitSoundWork( PARROT* work )
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
    work->nStrmMode	 = GM_SEMODE_NORMAL;
    work->nSeCntr	 = 0;
    work->nStrmHandle 	 = -1;
    work->nStrmIndex	 = -1;
    work->nStrmIndexNext = -1;
    work->bStrmProcFlag  = 0;
    work->nStrmProc 	 = 0;
    work->nStrmCntr   	 = 0;
    work->nSoundID 	 = GCL_GetOptionValue( 's', 32 ); // オウムのサウンドキャラ判別ID

    work->nCallEnemyFlag = 0;
    work->nCallEnemyCntr = 0;

    work->nTalkBlank1	 = COUNT_VMODE(GCL_GetOptionValue( 'b', 240 )); // オウムのサウンドキャラ判別ID
    work->nTalkBlank2	 = COUNT_VMODE(GCL_GetOptionValue( 'c', 500 )); // オウムのサウンドキャラ判別ID

    work->nSubjectRate[PAR_SUBJECT_NORM] = 60;
    work->nSubjectRate[PAR_SUBJECT_ERO]  = 15;
    work->nSubjectRate[PAR_SUBJECT_EIMS] = 15;

    NPC_SetSeID( &work->npc, work->nSoundID);	// サウンド用ID設定
printf("SOUND ----- ID ------- %d\n", work->nSoundID);								 

    // ストリーム音声番号の登録 
    if ( !GCL_GetOption( 'v' ) ){
	printf( "president GCL -v option not found \n" );
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){
	    work->nStrmNo[i] = 0;
	}
    }else{
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){	
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
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){
	    work->nStrmFlag[i] = 0;
	}
    }else{
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){
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
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){
	    work->nStrmLen[i] = 0;
	}
    }else{
	for( i = 0 ; i < PAR_STRM_MAX ; i++ ){
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
    printf("TALK Blank %d～%d\n", work->nTalkBlank1, work->nTalkBlank2);
    printf("GCL sound data end--------------------\n");

#endif

    // データ数取得
    work->nStrmDataNum = nNoNum;
}

// ワークの初期化
static void PAR_WorkInit(PARROT* work)
{
    work->nAttackNum  	   = 0;
    work->nAttackCntr 	   = 0;
    work->nPlayerAttention = 0;
    work->nEroLearn	   = 0;
    work->nEamesLearn	   = 0;
    work->nNoiseRate	   = 10;

    work->nTalkFlag 	   = 0;

    { // 注視点
	_sceVu0CopyVector ( &work->vecAimPos, &work->npc.ctrl->mov);
	work->vecAimPos.vz += 500.f;
    }
    PAR_InitSoundWork( work );
}




