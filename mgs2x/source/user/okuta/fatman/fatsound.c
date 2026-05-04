/*
    fatsound.c
    ファットマン:音関連
    2001/03/22 Masafumi Okuta
    $Id: fatsound.c,v 1.1.1.3 2002/11/19 11:48:01 Yoshizawa1 Exp $
*/
// 内蔵SE管理
static void FAT_SeManager(Work*    work)
{
    // ブランクタイムを減らす
    if ( work->nSeCntr > 0 )
	work->nSeCntr--;
}

// 内蔵SE再生
// 成功:1
// 失敗:0
static int FAT_CallSe(Work*    work,	// ファットマンワーク
		      int      nSeIndex,	// SE番号（インデックス）
		      FVECTOR* pvecPos,
		      int      nMode)
{
    if ( nSeIndex < 0 || nSeIndex >= SE_MAX ) return 0;

    // 現在のストリーム再生状況を取得
    if ( work->nStrmHandle != -1){
	if ( nSeIndex <= SE_FATOUT01 ){ // ダメージ声に関しては優先で鳴らす
	    if ( !work->bStrmProcFlag ){ // プロシージャ指定がなければ
		GM_StreamStop( work->nStrmHandle ); // ダメージ発声なのでストリームキャンセル
		work->nStrmIndexNext = -1; 
	    }
	}else{
	    if (GM_StreamStatus( work->nStrmHandle ) != GM_STREAM_STATE_END ){ // ストリーム再生中
		return 0;
	    }
	}
    }
    
    if ( nSeIndex > SE_FATSTN02 && nSeIndex <= SE_C4OUT08 ){ // 通常発声
	if (work->nSeCntr == 0 ){ // 再生ブランクチェック
#ifdef ENGLISH
	    switch( nSeIndex ){
	    case SE_FATSTN01:
	    case SE_FATSTN02: 
	    case SE_FATDMB01: 
	    case SE_FATDMB02: 
	    case SE_FATDOKN0: 
	    case SE_FATWHE01: 
	    case SE_FATWHE02: 
	    case SE_FATNOD01: 
	    case SE_FATNOD02: 
	    case SE_FATCUT01: 
	    case SE_FATCUT02:
		GM_JimakuSeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
		break;
	    default:
		GM_SeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	    }
#else
	    GM_SeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
#endif
	    work->nSeCntr = FAT_SE_BLANK;	// ブランクを設定	
	    return 1;
	}
    }else{
#ifdef ENGLISH
	switch( nSeIndex ){
	case SE_FATSTN01: 
	case SE_FATSTN02: 
	case SE_FATDMB01: 
	case SE_FATDMB02: 
	case SE_FATDOKN0: 
	case SE_FATWHE01: 
	case SE_FATWHE02: 
	case SE_FATNOD01: 
	case SE_FATNOD02: 
	case SE_FATCUT01: 
	case SE_FATCUT02:
	    GM_JimakuSeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	    break;
	default:
	    GM_SeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	}
#else
	GM_SeSetMode( FAT_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
#endif
	if ( nSeIndex <= SE_FATSTN02 ){	// ダメージ発声ならブランクを設定	
	    work->nSeCntr = FAT_SE_BLANK;	
	}
	return 1;
    }

    return 0;
}
// ファットマン発声
static int FAT_Say(Work*    work,
		   int      nSeIndex,	
		   int      nMode)
{
    if ( work->bGameClear ) return 0;

    return FAT_CallSe( work, nSeIndex, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], nMode);
}

// ストリームリクエスト::遅延がでてもよいから喋らせる
static void FAT_StreamRequest( Work* 	work,	// ファットマンワーク
			       int 	nIndex)	// ストリームテーブルインデックス
{
    // プロシージャ指定がある時は通さない？:暫定
    if ( work->bStrmProcFlag && work->nStrmIndexNext != -1 )
	return;
//    if ( work->bGameClear ) return ;

    work->nStrmIndexNext = nIndex;
}
// ストリームリクエスト::その時再生できないならあきらめる
static void FAT_StreamRequestJustTime( Work* 	work,	// ファットマンワーク
				       int 	nIndex)	// ストリームテーブルインデックス
{
    // プロシージャ指定がある時は通さない？:暫定
    if ( work->bStrmProcFlag && work->nStrmIndexNext != -1 && work->nStrmCntr > 0)
	return;
//    if ( work->bGameClear ) return ;

    work->nStrmIndexNext = nIndex;
}
#if 0
// プロシージャ指定つきストリームリクエスト
static void FAT_StreamRequestWithProc( Work* 	work,	// ファットマンワーク
				       int 	nIndex,	// ストリームテーブルインデックス
				       int	nProc)	// 再生終了時に呼びたいプロシージャ
{
    GM_StreamStop( work->nStrmHandle ); // 最優先なのでストリームキャンセル
    work->nStrmIndexNext = nIndex;
    work->bStrmProcFlag  = 1;
    work->nStrmProc 	 = nProc;
}
#endif
// ストリーム管理更新
static void FAT_StreamUpdate( Work* work ) // ファットマンワーク
{
    // ゲームオーバーチェック（ゲームオーバー音声のみ通過）
    if ( GM_IsGameOver() && work->nStrmIndexNext != FAT_STRM_GAMEOVER ) return;

    // デモ中ははじく
    if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) return; 

    if ( ( work->nStrmHandle == -1 || GM_StreamStatus( work->nStrmHandle ) == GM_STREAM_STATE_END) && 
	 work->nStrmCntr == 0 ){ // 再生が終っている

	// ハンドラが生きていたら
	if ( work->nStrmHandle != -1 && GM_StreamStatus( work->nStrmHandle ) ){
	    work->nStrmHandle = -1; // ハンドラの解放
	}

	if ( work->nStrmIndexNext != -1 ){ // リクエストあり
	    int	vol, pan;
            work->nStrmIndex = work->nStrmIndexNext; // インデックス更新
	    if ( work->bStrmProcFlag ){ // 再生:プロシージャつき
//		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
//						  GM_STREAM_FLAG_PROC( work->nStrmProc ) ); 
		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
						  GM_STREAM_FLAG_PROC( work->nStrmProc ) | GM_STREAM_FLAG_3D ); 
		work->bStrmProcFlag = 0;
		work->nStrmProc = 0;
	    }else{ // 通常再生
//		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
//						  work->nStrmFlag[ work->nStrmIndex ]); 
		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
						  work->nStrmFlag[ work->nStrmIndex ] | GM_STREAM_FLAG_3D ); 
	    }
	    
	    if ( work->nStrmHandle != -1 ){
		// 位置情報からストリームとPANを設定
          float bp_angle;
	        GM_SeGetVolPan( &work->npc.ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
//		GM_VoxStreamSetPan( work->nStrmHandle, vol, pan ) ;
		GM_VoxStreamSetParam( work->nStrmHandle, &work->npc.ctrl->mov, work->npc.ctrl->addr, vol, pan, bp_angle ) ;
	    }
	    work->nStrmIndexNext = -1; // 無効にしておく

	    if ( work->nStrmHandle != -1 ){
		work->nStrmCntr = work->nStrmLen[ work->nStrmIndex ];	// 長さ取得
	    }
	}
    }else{
	if ( work->nStrmCntr > 0) work->nStrmCntr--; // ストリーム再生カウンタを更新
    }
}

// ダメージを受けた時の内蔵SE音声呼びわけ
static void FAT_DamageVoice( Work* work, long64 weapon, int part, int bNoDmg)
{
    NPCWORK*	npc;
    npc = &work->npc;

    // ゲームオーバーチェック
    if ( GM_IsGameOver() ) return;

    if ( work->m9_faint <= 0 || npc->action.life <= 0 ){
	return;
    }


    // ダメージ音
    if ( part == FAT_TRG_CHILD_HEAD ){ // 頭部ダメージなら
	if ( MAO_Rand( 2 ) == 0)    FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB);
	else			    FAT_Say( work, SE_FATDMG03, GM_SEMODE_BOMB);
	return;
    }else if ( part == FAT_TRG_CHILD_ROLL_R || part == FAT_TRG_CHILD_ROLL_L ){ // ローラーダメージなら
	if ( MAO_Rand( 2 ) == 0)    FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB);
	else			    FAT_Say( work, SE_FATDMG03, GM_SEMODE_BOMB);
	return;
    }else{
	if ( bNoDmg){
	    int nSeed = MAO_Rand( 10 );
	    if ( nSeed < 1) 	 FAT_Say( work, SE_FATNOD01, GM_SEMODE_BOMB);
	    else if ( nSeed < 2) FAT_Say( work, SE_FATNOD02, GM_SEMODE_BOMB);
	    else if ( nSeed < 4) FAT_StreamRequestJustTime( work, FAT_STRM_IMMOTAL_01); // なにかしたか
	    else if ( nSeed < 6) FAT_StreamRequestJustTime( work, FAT_STRM_IMMOTAL_02); // すーつ
	    else if ( nSeed < 8) FAT_StreamRequestJustTime( work, FAT_STRM_IMMOTAL_03); // どこをねらって
	    else 		 FAT_StreamRequestJustTime( work, FAT_STRM_IMMOTAL_04); // なんのつもりだ
	    return;
	}

	if ( weapon & WP_STUNGRENADE ){	// スタングレネード
	    if ( MAO_Rand( 2 ) == 0){	
		FAT_Say( work, SE_FATSTN01, GM_SEMODE_BOMB);
	    }else{
		FAT_Say( work, SE_FATSTN02, GM_SEMODE_BOMB);
	    }	
	    return;
	}else if ( weapon & WP_CLAYMORE ){ // クレイモア
	    if ( MAO_Rand( 2 ) == 0){	
		FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB);
	    }else{
		FAT_Say( work, SE_FATDMG03, GM_SEMODE_BOMB);
	    }	
	    return;
	}else{
	    switch ( MAO_Rand( 3 ) ){
	    case 0:
		FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB);
		break;
	    case 1:
		FAT_Say( work, SE_FATDMG03, GM_SEMODE_BOMB);
		break;
	    case 2:
		FAT_Say( work, SE_FATDMG04, GM_SEMODE_BOMB);
		break;
	    }	
	    return;
	}
    }
}

// 常駐ローラー音
#define INTV_RESIDENT_ROLLER_SE (COUNT_VMODE(24)) // 24fps毎にコール
static void FAT_ResidentRollerSe( Work* work )
{
    int		nSeType;
    NPCWORK* npc;

    npc = &work->npc;
    if (GV_Time % INTV_RESIDENT_ROLLER_SE != 0) // 
	return;

    if ( FAT_CheckMotionMove( work ) ){ // 移動中かどうかモーションから判断	
	nSeType = ((npc->ctrl->flr_atrs[0] & 0xf0000000) >> 28);
	if ( nSeType == 0){
	    GM_SeSetMode( SD_E_LOOPCK02, &npc->ctrl->mov, GM_SEMODE_BOMB);
	}else if ( nSeType == 2){
	    GM_SeSetMode( SD_E_LOOPMT01, &npc->ctrl->mov, GM_SEMODE_BOMB);
	}else if ( nSeType == 3){
	    GM_SeSetMode( SD_E_LOOPMT02, &npc->ctrl->mov, GM_SEMODE_BOMB);
	}
    }
}



