/*
    prezsound.c
    大統領ニキータイベント:音関連
    2001/03/22 Masafumi Okuta
    $Id: prezsound.c,v 1.1.1.3 2002/11/19 11:48:11 Yoshizawa1 Exp $
*/
// 内蔵SE管理
static void PREZ_SeManager(Work*    work)
{
    // ブランクタイムを減らす
    if ( work->nSeCntr > 0 )
	work->nSeCntr--;
}

// 内蔵SE再生
// 成功:1
// 失敗:0
static int PREZ_CallSe(Work*    work,	// 大統領ワーク
		      int      nSeIndex,	// SE番号（インデックス）
		      FVECTOR* pvecPos,
		      int      nMode)
{
    if ( nSeIndex < 0 && nSeIndex >= SE_MAX ) return 0;

    // 現在のストリーム再生状況を取得
    if ( work->nStrmHandle != -1){
	if (GM_StreamStatus( work->nStrmHandle ) != GM_STREAM_STATE_END ){ // ストリーム再生中
	    return 0;
	}
# if 0
	if ( nSeIndex <= SE_FATSTN02 ){
	    GM_StreamStop( work->nStrmHandle ); // ダメージ発声なのでストリームキャンセル
	}else{
	    if (GM_StreamStatus( work->nStrmHandle ) != GM_STREAM_STATE_END ){ // ストリーム再生中
		return 0;
	    }
	}
#endif
    }

    {
	GM_SeSetMode( PREZ_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	return 1;
    }
#if 0    
    if ( nSeIndex > SE_FATSTN02 && nSeIndex <= SE_C4OUT08 ){ // 通常発声
	if (work->nSeCntr == 0 ){ // 再生ブランクチェック
	    GM_SeSetMode( PREZ_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	    work->nSeCntr = PREZ_SE_BLANK;	// ブランクを設定	
	    return 1;
	}
    }else{
	GM_SeSetMode( PREZ_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	if ( nSeIndex <= SE_FATSTN02 ){	// ダメージ発声ならブランクを設定	
	    work->nSeCntr = PREZ_SE_BLANK;	
	}
	return 1;
    }
#endif
    return 0;
}
// 大統領発声
static int PREZ_Say(Work*    work,
		   int      nSeIndex,	
		   int      nMode)
{
    return PREZ_CallSe( work, nSeIndex, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], nMode);
}

// ストリームリクエスト
static void PREZ_StreamRequest( Work* 	work,	// 大統領ワーク
			       int 	nIndex)	// ストリームテーブルインデックス
{
    // プロシージャ指定がある時は通さない？:暫定
    if ( work->bStrmProcFlag && work->nStrmIndexNext != -1 )
	return;

printf("PREZ-STRM ==== %d\n", nIndex);
    work->nStrmIndexNext = nIndex;
}
// ストリームリクエスト 再生できない時はあきらめる
static void PREZ_StreamRequestJustTime( Work* 	work,	// 大統領ワーク
					int 	nIndex)	// ストリームテーブルインデックス
{
    // プロシージャ指定がある時は通さない？:暫定
    if ( work->bStrmProcFlag && work->nStrmIndexNext != -1 && work->nStrmCntr > 0)
	return;
printf("PREZ-STRM ==== %d\n", nIndex);
    work->nStrmIndexNext = nIndex;
}
#if 0
// プロシージャ指定つきストリームリクエスト
static void PREZ_StreamRequestWithProc( Work* 	work,	// 大統領ワーク
				       int 	nIndex,	// ストリームテーブルインデックス
				       int	nProc)	// 再生終了時に呼びたいプロシージャ
{
    work->nStrmIndexNext = nIndex;
    work->bStrmProcFlag  = 1;
    work->nStrmProc 	 = nProc;
}
#endif
// ストリーム管理更新
static void PREZ_StreamUpdate( Work* work ) // 大統領ワーク
{
    if ( ( work->nStrmHandle == -1 || GM_StreamStatus( work->nStrmHandle ) == GM_STREAM_STATE_END) && 
	 work->nStrmCntr == 0 ){ // 再生が終っている
	if ( work->nStrmHandle != -1 && GM_StreamStatus( work->nStrmHandle ) ){
	    work->nStrmHandle = -1; // ハンドラの解放
	}
	if ( work->nStrmIndexNext != -1 ){ // リクエストあり
	    int	vol, pan ;

	    // 死亡処理中はアウト声のみ
	    if ( GM_IsGameOver() && work->nStrmIndexNext != PREZ_STRM_GAMEOVER ){
		return;
	    }

            work->nStrmIndex = work->nStrmIndexNext; // インデックス更新

	    if ( work->bStrmProcFlag ){ // 再生:プロシージャつき
//		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
//						  GM_STREAM_FLAG_PROC( work->nStrmProc ) ); 
		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
						  GM_STREAM_FLAG_PROC( work->nStrmProc ) | GM_STREAM_FLAG_3D ); 
		if ( work->nStrmHandle != -1){
		    // 位置情報からストリームとPANを設定
         float bp_angle;
		    GM_SeGetVolPan( &work->npc.ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
//		    GM_VoxStreamSetPan( work->nStrmHandle, vol, pan ) ;
		    GM_VoxStreamSetParam( work->nStrmHandle, &work->npc.ctrl->mov, work->npc.ctrl->addr, vol, pan, bp_angle ) ;
		    work->bStrmProcFlag = 0;
		    work->nStrmProc = 0;
		}
	    }else{ // 通常再生
//		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
//						  work->nStrmFlag[ work->nStrmIndex ]); 
		work->nStrmHandle = GM_VoxStream( work->nStrmNo[ work->nStrmIndex ], 
						  work->nStrmFlag[ work->nStrmIndex ] | GM_STREAM_FLAG_3D ); 
		if ( work->nStrmHandle != -1){
		    // 位置情報からストリームとPANを設定
         float bp_angle;
		    GM_SeGetVolPan( &work->npc.ctrl->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
		    if ( !MAO_BoundCheck( (FVECTOR*)&DG_Chanls[0].eye.m[3][0], 
					  &work->vecNoiseTrap[0], &work->vecNoiseTrap[1] ) ){
			vol /= 2; // 壁越しは落す
		    }
//		    GM_VoxStreamSetPan( work->nStrmHandle, vol, pan ) ;
		    GM_VoxStreamSetParam( work->nStrmHandle, &work->npc.ctrl->mov, work->npc.ctrl->addr, vol, pan, bp_angle ) ;
		}
	    }

	    work->nStrmIndexNext = -1; // 無効にしておく

	    if ( work->nStrmHandle != -1 ){
		work->nStrmCntr = work->nStrmLen[ work->nStrmIndex ] + COUNT_VMODE(120);	// 長さ取得
	    }
	}
    }
	
    if ( work->nStrmCntr > 0){
	work->nStrmCntr--; // ストリーム再生カウンタを更新
    }
}
