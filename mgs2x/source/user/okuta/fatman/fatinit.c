/*
    fatinit.c
    ファットマン初期化関連
    2001/04/09 Masafumi Okuta
    $Id: fatinit.c,v 1.1.1.3 2002/11/19 11:47:59 Yoshizawa1 Exp $
*/

// ステータスワーク初期化
static void FAT_InitStatusWork( Work* work )
{
    FMATRIX 	mat;

    // その他システム
    work->nRollerBreak	 = 0;
    work->bKillFlag 	 = 0;
    work->bGameClear	 = 0;
    work->bGameOver 	 = 0;
    work->bCallClearProc = 0;
    _sceVu0CopyVector( &work->vecPrePos, &work->npc.ctrl->mov);
    _sceVu0CopyVector( &work->vecStep, &work->npc.ctrl->step);
    _sceVu0CopyVector( &work->vecPreDmgStep, &work->npc.ctrl->step);
    work->nMoveDist 	= 0.f;
    TS_VecToRot( &work->vecKoshiRot, (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_KOSHI).m[2][0] );

    // ステータス
    work->nAimIndex     = 0;
    work->fFatMotSpeed  = 1.f;
    work->nBullet 	= GLK_BUL_NUM;
    work->bAdjustFlag 	= 0;
    work->svecAdjust 	= DG_ZeroSVector;
    work->bRollerStop 	= 0;
    work->nRollerBack   = 0;
    work->nPhaseCntr  	= 0;
    work->vecNextPos	= work->npc.ctrl->mov;
    DG_SetPos2( &work->npc.ctrl->mov, &work->npc.ctrl->rot );
    DG_GetPos( &mat );
    _sceVu0CopyVector( &work->vecFatDir, (FVECTOR *)&mat.m[2][0] );
    work->nInviCntr 	= 0;
    work->nNoChkDmgFlag = 0;
    work->nFatPrevInfo  = 0x00000000;
    work->nFatInfo  	= 0x00000000;

    work->nNarcTime 	= NARC_CAUT_TIME;
    work->nStunTime 	= STUN_CAUT_TIME;
    work->nAttackTime 	= 0;
    work->nBehindCntr	= 0;
    work->nIntrudeCntr  = 0;
    work->nBodyDurable 	= 0;
    work->nBodyDurTime 	= 0;
    work->nBodyDmgTime 	= BODY_CAUT_TIME;
    work->nHeadDmgTime 	= HEAD_CAUT_TIME;
    work->nRollDmgTime  = ROLL_CAUT_TIME;
    work->nPrevZone 	= 0;

    work->nEvadeRad 	= 750;
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	work->nVitalMax	= FAT_VITAL_JP_VEASY_MAX;
	break;
    case GM_LEVEL_EASY:
	work->nVitalMax	= FAT_VITAL_JP_EASY_MAX;
	break;
    default:
	work->nVitalMax	= FAT_VITAL_MAX;
    }
#else
    work->nVitalMax	= FAT_VITAL_MAX;
#endif
    work->nVitalCntr 	= work->nVitalMax;
    work->nFireCurrent  = 0;
    work->nFireIntv 	= COUNT_VMODE(3);
    work->nHeadDmgFlag 	= 0;

    work->nVibCensorTime = 0;
    
    work->nRunMode 	= FAT_RUNMODE_NORMAL;
    work->nTurnMotRate  = 1.f;
    work->nMoveForm 	= FAT_MOVEFORM_ZONE;

    work->nQuakeCntr 	= 0;

    work->pCurPosData 	= &work->datPoint[0]; 
    work->pdatGuard 	= NULL;

    // フェーズ関連
    work->nBombPhaseCntr  = 0;
    work->nShootPhaseCntr = 0;
    work->bBombStartFlag  = 0;
    work->nConsTime 	  = 0;

    // かもめ
    work->codeKmmngName = GCL_GetOptionValue( 'm', GV_StrCode("かもめ管理") );
    work->bKamomeRest = 0;
    work->bPrevKmRest = 0;

    // 思考分岐チェック
    FAT_SetCheckThink( work, FAT_ChkThkTemporary );

    work->bPutFlag	= 0;
}
// 周辺情報ワーク
static void FAT_InitInfoWork( Work* work )
{
    FVECTOR vec;

    // 視界情報チェック
    FAT_InitEyeWork( work );	

    // 目標（プレイヤー）との位置関係
//  work->vecCautionPos = GM_PlayerPosition;
    work->vecAimPos = GM_PlayerPosition;
    _sceVu0SubVector(  &vec, &work->vecAimPos, &work->npc.ctrl->mov ) ;
    _sceVu0Normalize( &work->vecAimDiff, &vec );
    work->aim_dis = _FVecLen3( &vec ) ;	
    work->aim_dir = _FVecDir2( &vec ) ;	
    // プレイヤーとの位置関係
    _sceVu0SubVector(  &vec, &GM_PlayerPosition, &work->npc.ctrl->mov ) ;
    _sceVu0Normalize( &work->vecPlaDiff, &vec );
    work->pl_dis = _FVecLen3( &vec ) ;	// プレイヤーとの距離 
    work->pl_dir = _FVecDir2( &vec ) ;	// プレイヤーへの方向 

    // エリア設定
    work->nFatArea  = FAT_GetArea( &work->npc.ctrl->mov);
    work->nPlaArea  = FAT_GetArea( &GM_PlayerPosition);
    work->nAimArea  = FAT_GetArea( &work->vecAimPos);

    // 飛び出しエリア設定
//    FAT_SetTobidasiArea( work );

    // 可視判定関連設定
    work->pl_parts   = 0x00000000;
    work->fat_parts  = 0x00000000;
}
// 爆弾情報ワーク
static void FAT_InitBombWork( Work* work )
{
    InitC4Work(work);
}

// サウンド関連ワーク
static void FAT_InitSoundWork( Work* work )
{
    int i;
    int nNoNum;
    int nFlagNum;
    int nLenNum;

    // local param init
    nNoNum = 0;
    nFlagNum  = 0;
    nLenNum   = 0;

    // sound work init
    work->nSoundID 	 = MT_SETABLE_FAT; // ファットマンのサウンドキャラ判別ID
    NPC_SetSeID( &work->npc, work->nSoundID);	// サウンド用ID設定
    work->nSeCntr	 = 0;
    work->nStrmOnce	 = 0;
    work->nStrmHandle 	 = -1;
    work->nStrmIndex	 = -1;
    work->nStrmIndexNext = -1;
    work->bStrmProcFlag  = 0;
    work->nStrmProc 	 = 0;
    work->nStrmCntr   	 = 0;

    // ストリーム音声番号の登録 
    if ( !GCL_GetOption( 'v' ) ){
#ifdef DEBUG_MODE
	printf( "fatman GCL -v option not found \n" );
#endif
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){
	    work->nStrmNo[i] = 0;
	}
    }else{
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){	
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
#ifdef DEBUG_MODE
	printf( "fatman GCL -f option not found \n" );
#endif
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){
	    work->nStrmFlag[i] = 0;
	}
    }else{
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){
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
#ifdef DEBUG_MODE
	printf( "fatman GCL -l option not found \n" );
#endif
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){
	    work->nStrmLen[i] = 0;
	}
    }else{
	for( i = 0 ; i < FAT_STRM_MAX ; i++ ){
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

// エルード落とし
static void FAT_InitEludeWork( Work* work )
{
    int i;

    for ( i = 0; i < FAT_ELUDE_FALL_NUM; i++){
	work->nEludeZone[i] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_ELUDE_POS[i], -1) );
    }
}

// データワーク
static void FAT_InitDataWork( Work* work )
{
    NPCWORK*	npc;		// NPCワークへのポインタ
    npc = &work->npc;
    // 位置データワークの初期化
    FAT_InitAreaPos2Zone( work ); 		// エリア対応データ
    FAT_InitEscPos2Zone( work );		// 逃げゾーンデータ
    FAT_InitAttractPos2Zone( work );		// 余興ポジションデータ
    FAT_InitEludeWork( work);			// エルード落とし用ゾーンデータ
    FAT_InitTurnPutRef( work );			// 回転設置参照ゾーンデータ
    FAT_InitBezierMove( work);			// ベジェ移動用データ
    // GCLから爆弾設置場所データ生成
    FAT_GetGclBombData( work );
}

// イベントワーク
static void FAT_InitEventWork( Work* work )
{
    // GCLから各種プロシージャを取得
    work->procShootC4Over   	= GCL_GetOptionValue( 'u', 0 );
    work->procGameOver   	= GCL_GetOptionValue( 'o', 0 );
    work->procKillClear  	= GCL_GetOptionValue( 'k', 0 );
    work->procSleepClear 	= GCL_GetOptionValue( 'e', 0 );
    work->procFatInfo    	= GCL_GetOptionValue( 'g', 0 );
#ifdef DEBUG_MODE
    printf("GCL proc data start------------------\n");
    printf("GameOver    = %d\n", work->procGameOver);
    printf("Kill Clear  = %d\n", work->procKillClear);
    printf("Sleep Clear = %d\n", work->procSleepClear);
    printf("Sleep Clear = %d\n", work->procFatInfo);
    printf("GCL proc data end--------------------\n");
#endif
}

// デバックワーク
static void FAT_InitDebugWork( Work* work )
{
#ifdef DEBUG_MODE
    work->nFatFixFace = 0;
    work->nDbgLevel 	= 0;
    work->nDbgPattern 	= 0;
    work->nDbgIndex	= 0;
#endif
#ifdef DEBUG_MODE
    // デバックメニュー登録
    FAT_CallDbgDump();	
    FAT_CallDbgThink();
    FAT_CallDbgMotion();
    FAT_CallDbgStatus();
    FAT_CallDbgViewer();
    FAT_CallDbgBomb();

    FAT_DbgDrawTrgBox(work);
#endif
}

// 顔アニメ初期化
static void FAT_InitFaceAnime( Work* work )
{
    extern void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id );
    work->nFaceChara = FACEANIME_NAME;
    NewFaceAnimation( work->nFaceChara, work->body.evmobj, FAT_FAR_NAME);
}

// ノックバック用ターゲット
static void FAT_InitPushTarget( Work* work)
{
    TARGET*	ptrg;
    static FVECTOR size = { 400.f, 1000.f, 400.f, 1.f };

    ptrg = &work->pushtrg ;

    work->nEvadePush = 0;
    work->nEvadeConse = 0;
    work->vecPush    = DG_ZeroVector;

    work->fSpeed     = 1.f;

    GM_SetTarget( ptrg, TARGET_TOUCH|TARGET_DEFENSE|TARGET_THROUGH, 1, ENEMY_SIDE,  &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( ptrg, 0 ) ;
    GM_PutTarget( ptrg ) ;
}

