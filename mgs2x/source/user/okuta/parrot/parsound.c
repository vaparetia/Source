/*
    parsound.c
    オウム:音関連
    2001/05/12 Masafumi Okuta
    $Id: parsound.c,v 1.1.1.3 2002/11/19 11:48:07 Yoshizawa1 Exp $
*/
#define SE_COS_10	(0.9848077530122f)
#define SE_COS_15	(0.9659258262891f)
#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)

// ----------------------------------- 室内用
static VOLUMECURVE PREZ_BaseVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 13000, -1, -1 },
{ 0x3F, 0x20, 0x08, 0xff, 0xff },
{ 7000, 12000, -1, -1 },
{ 0x3F, 0x20, 0x08, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_NormVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 13000, -1, -1 },
{ 0x3F, 0x20, 0x08, 0xff, 0xff },
{ 7000, 12000, -1, -1 },
{ 0x3F, 0x20, 0x08, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_OwnValCurve = {
1,
SE_COS_60,
SE_COS_90,
{ 12000, -1, -1, -1 },
{ 0x3F, 0x04, 0xff, 0xff, 0xff },
{ 11800, -1, -1, -1 },
{ 0x3F, 0x04, 0xff, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_DemoVolCurve = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 14000, -1, -1 },
{ 0x3F, 0x04, 0x1f, 0xff, 0xff },
{ 7000, 13000, -1, -1 },
{ 0x3F, 0x04, 0x1f, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_MicVolCurve = {
2,
SE_COS_20,
SE_COS_30,
{ 10000, 12000, -1, -1 },
{ 0x3F, 0x3F, 0x00, 0xff, 0xff },
{ 0, 5000,-1, -1 },
{ 0x08, 0x00, 0x00, 0xff, 0xff },
1.0f
} ;

static VOLCURVES PREZ_ExtraVolCurves = {
&PREZ_NormVolCurve,
&PREZ_OwnValCurve,
&PREZ_DemoVolCurve,
&PREZ_MicVolCurve
};

// ----------------------------------- 室外用
static VOLUMECURVE PREZ_BaseVolCurve2 = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 13000, -1, -1 },
{ 0x15, 0x06, 0x00, 0xff, 0xff },
{ 7000, 12000, -1, -1 },
{ 0x15, 0x06, 0x00, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_NormVolCurve2 = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 13000, -1, -1 },
{ 0x12, 0x1, 0x02, 0xff, 0xff },
{ 7000, 12000, -1, -1 },
{ 0x12, 0x1, 0x02, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_OwnValCurve2 = {
1,
SE_COS_60,
SE_COS_90,
{ 3000, -1, -1, -1 },
{ 0x10, 0x01, 0xff, 0xff, 0xff },
{ 3800, -1, -1, -1 },
{ 0x10, 0x01, 0xff, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_DemoVolCurve2 = {
2,
SE_COS_75,
SE_COS_90,
{ 8000, 14000, -1, -1 },
{ 0x1f, 0x04, 0x1f, 0xff, 0xff },
{ 7000, 13000, -1, -1 },
{ 0x1f, 0x04, 0x1f, 0xff, 0xff },
1.0f
} ;
static VOLUMECURVE PREZ_MicVolCurve2 = {
2,
SE_COS_20,
SE_COS_30,
{ 4000, 12000, -1, -1 },
{ 0x2f, 0x20, 0x00, 0xff, 0xff },
{ 0, 2000,-1, -1 },
{ 0x08, 0x00, 0x00, 0xff, 0xff },
1.0f
} ;

static VOLCURVES PREZ_ExtraVolCurves2 = {
&PREZ_NormVolCurve2,
&PREZ_OwnValCurve2,
&PREZ_DemoVolCurve2,
&PREZ_MicVolCurve2
};
// 内蔵SE管理
static void PAR_SeManager(PARROT*    work)
{
    // ブランクタイムを減らす
    if ( work->nSeCntr > 0 )
	work->nSeCntr--;
}

// 内蔵SE再生
// 成功:1
// 失敗:0
static int PAR_CallSe(PARROT*    work,	// オウムワーク
		      int      nSeIndex,	// SE番号（インデックス）
		      FVECTOR* pvecPos,
		      int      nMode)
{
    if ( nSeIndex < 0 || nSeIndex >= SE_MAX) return 0;

    // 現在のストリーム再生状況を取得
    if ( work->nStrmHandle != -1){
	if (GM_StreamStatus( work->nStrmHandle ) != GM_STREAM_STATE_END ){ // ストリーム再生中
	    return 0;
	}
    }

    {
	GM_SeSetMode( PAR_SE[nSeIndex], pvecPos, nMode);	// 効果音コール
	return 1;
    }

    return 0;
}
// オウム発声
static int PAR_Say(PARROT*    work,
		   int      nSeIndex,	
		   int      nMode)
{
    return PAR_CallSe( work, nSeIndex, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], nMode);
}

// ストリームリクエスト
static void PAR_StreamReq( PARROT* 	work,	// オウムワーク
			   int 		nIndex)	// ストリームテーブルインデックス
{
    // プロシージャ指定がある時は通さない？:暫定
    if ( work->nStrmIndexNext != -1 || work->bStrmProcFlag   )
	return;

    work->nStrmIndexNext = nIndex;
}
#if 0
// プロシージャ指定つきストリームリクエスト
static void PAR_StreamRequestWithProc( PARROT* 	work,	// オウムワーク
				       int 	nIndex,	// ストリームテーブルインデックス
				       int	nProc)	// 再生終了時に呼びたいプロシージャ
{
    work->nStrmIndexNext = nIndex;
    work->bStrmProcFlag  = 1;
    work->nStrmProc 	 = nProc;
}
#endif
// ストリーム管理更新
static void PAR_StreamUpdate( PARROT* work ) // オウムワーク
{

    if ( ( work->nStrmHandle == -1 || GM_StreamStatus( work->nStrmHandle ) == GM_STREAM_STATE_END) && 
		 work->nStrmCntr == 0 ){ // 再生が終っている
		if ( work->nStrmHandle != -1 && GM_StreamStatus( work->nStrmHandle ) ){
		    work->nStrmHandle = -1; // ハンドラの解放
	}

	if ( GM_VRStatus & GM_VR_CLEAR ) {
//	    work->nStrmHandle = -1; // ハンドラの解放
	    work->nStrmIndexNext = -1; // リクエスト無効
	}

	if ( work->nStrmIndexNext != -1 ){ // リクエストあり
            work->nStrmIndex = work->nStrmIndexNext; // インデックス更新

	    if ( work->bStrmProcFlag ){ // 再生:プロシージャつき
		// 音声再生
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
		int vol, pan;
      float bp_angle;
		if (work->nTalkFlag){
		    // 位置情報からストリームとPANを設定
		    GM_SetVolCurveInDisFromPos( &GM_PlayerPosition, PREZ_BaseVolCurve.in_dis, 
						PREZ_NormVolCurve.in_dis, 3 ) ;
		    GM_SeGetVolPanFromVolCurves( &work->npc.ctrl->mov, &vol, &pan, &PREZ_ExtraVolCurves, &bp_angle );
		}else{
		    GM_SetVolCurveInDisFromPos( &GM_PlayerPosition, PREZ_BaseVolCurve2.in_dis, 
						PREZ_NormVolCurve2.in_dis, 3 ) ;
		    GM_SeGetVolPanFromVolCurves( &work->npc.ctrl->mov, &vol, &pan, &PREZ_ExtraVolCurves2, &bp_angle );
		}
//		GM_SeGetVolPan( &work->npc.ctrl->mov, work->nStrmMode, &vol, &pan ) ;
//		GM_VoxStreamSetPan( work->nStrmHandle, vol, pan ) ;
		GM_VoxStreamSetParam( work->nStrmHandle, &work->npc.ctrl->mov, work->npc.ctrl->addr, vol, pan, bp_angle ) ;

		// 敵を呼ぶ
		if ( work->nStrmIndex >= PAR_STRM_ENEMY01 && 
		     work->nStrmIndex <= PAR_STRM_ENEMY03 ){
		    work->nCallEnemyFlag = 1;
		    work->nCallEnemyCntr = work->nStrmLen[ work->nStrmIndex ]; // これより短いとキャンセルを喰らう
		}
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
