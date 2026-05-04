/*
    prezparty.c
    ニキータ大統領構造体用関数群（C++でいうところのSet,Getメンバ関数など）
    2001/04/24 Masafumi Okuta
    $Id: prezparty.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/

// 思考レベル１変更
static void PREZ_SetThink1( Work* work,		// ファットマンワーク
			   int	 nNewThink1,	// 新しいルーチン:優先レベル
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
    if ( nNewThink2 == TH2_ATARAXIA){
	work->bFindNikita = 0;
    }
    work->think1 = nNewThink1;
    work->think2 = nNewThink2;
    work->think3 = nNewThink3;
    work->count3 = 0;
}
// 思考レベル２変更
static void PREZ_SetThink2( Work* work,		// ファットマンワーク
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
    if ( nNewThink2 == TH2_ATARAXIA){
	work->bFindNikita = 0;
    }

    work->think2 = nNewThink2;
    work->think3 = nNewThink3;
    work->count3 = 0;
}
// 思考レベル３変更
static void PREZ_SetThink3( Work* work,		// ファットマンワーク
			   int	 nNewThink3)	// 新しいルーチン
{
    work->think3 = nNewThink3;
    work->count3 = 0;
}

// 思考レベル３変更
static void PREZ_SetNextThink3( Work* 	work,		// ファットマンワーク
				int	nNewThink3)	// 新しいルーチン
{
    work->nNextThink3 = nNewThink3;
    work->bNextThink3Valid = 1;
}

// 思考レベル３変更
static void PREZ_SetThink3FromNext( Work*  work,		// ファットマンワーク
				    int	   nNewThink3)	// 新しいルーチン
{
    if ( work->bNextThink3Valid ){
	PREZ_SetThink3( work, work->nNextThink3);
	work->bNextThink3Valid = 0;
    }else{
	PREZ_SetThink3( work, nNewThink3);
    }
}

// 指定ゾーンの隣接ゾーンに入ったか判定
static int PREZ_CheckNearZoneIn( int nCharaAddr,	// 指定キャラのゾーン番号
				 int nAimAddr) 	// 指定ゾーン番号
{
    int i;
    int nCharaZone, nAimZone;
    HZX_ZON* 	pzone;		// ゾーンデータ

    // ゾーンアドレスからゾーン番号取得
    nCharaZone = HZX_Zone1(nCharaAddr);
    nAimZone   = HZX_Zone1(nAimAddr);

    // 目標ゾーン
    if (nAimZone == -1) return 0;

    // 指定ゾーン内かどうか
    if ( nCharaZone == nAimZone ) return 1;

    pzone = ENE_HZX_GetZone( nAimAddr );	// ゾーンアドレスからデータへのポインタを取得
    // 隣接ゾーンから探す
    for ( i = 0; i < 6; i++){
	if ( pzone->nears[i] != HZX_NO_ZONE && pzone->nears[i] == nCharaZone){
	    return 1; // 隣接ゾーンにいる
	}
    }

    return 0;
}
#if 0
// 隣接ゾーンのなかから与えられたベクトルに対して最も内積値の小さいゾーンを返す
static int PREZ_GetZoneFromInnerMin( NPCWORK* npc,
				    int      nZoneAddr,	// 対象ゾーンアドレス
				    FVECTOR* pfvec,	// 方向ベクトル
				    float*   pfRes)	// 内積結果が入る
{
    int 	i;
    int 	nTmp;
    int 	nZoneRes;	// 結果
    float 	fInner;		// 内積値
    float 	fTmp;		// 内積値:一時置き
    FVECTOR     fvecHzx;	// ゾーン位置
    FVECTOR     fvecTmp;	// 一時置き
    HZX_ZON* 	pzone;		// ゾーンデータ
    HZX_ZON* 	pzoneTmp;	// ゾーンデータ

    nZoneRes = -1;			// 初期化
    pzone = ENE_HZX_GetZone( nZoneAddr );	// ゾーンアドレスからデータへのポインタを取得
    fInner = 1.f;				
    
    for ( i = 0; i < 6; i++){
	if ( pzone->nears[i] != HZX_NO_ZONE ){ // 隣接ゾーンから探す
	    nTmp = HZX_Address( npc->ctrl->hzx_id, pzone->nears[i], pzone->nears[i]);
	    if ( (GM_GetRIntrptZ2Z( npc->ctrl->addr, nTmp ) != NULL) ||
		 !GM_GetZIntrptZ2Z( npc->ctrl->addr, nTmp ) ) {
		continue;
	    }
	    pzoneTmp = ENE_HZX_GetZone( pzone->nears[i] );
	    fvecHzx.vx = pzoneTmp->x;
	    fvecHzx.vy = pzoneTmp->y;
	    fvecHzx.vz = pzoneTmp->z;
	    _sceVu0SubVector( &fvecTmp, &fvecHzx, &GM_PlayerPosition); 
	    _sceVu0Normalize( &fvecTmp, &fvecTmp);
	    fTmp = _sceVu0InnerProduct( pfvec, &fvecTmp); // 内積計算
	    if ( fTmp < fInner ){	// より背後に近いゾーンがみつかった
		nZoneRes = pzone->nears[i]; // 
		fInner = fTmp;	// 内積値更新
	    }
	}
    }

    // 内積結果取得
    if ( pfRes != NULL)
	(*pfRes) = fInner;

    return nZoneRes;
}
// 隣接ゾーンのなかから与えられたベクトルに対して最も内積値の大きいゾーンを返す
static int PREZ_GetZoneFromInnerMax( NPCWORK* npc,
				    int      nZoneAddr,	// 対象ゾーンアドレス
				    FVECTOR* pfvec,	// 方向ベクトル
				    float*   pfRes)	// 内積結果が入る
{
    int 	i;
    int		nTmp;
    int 	nZoneRes;	// 結果
    float 	fInner;		// 内積値
    float 	fTmp;		// 内積値:一時置き
    FVECTOR     fvecHzx;	// ゾーン位置
    FVECTOR     fvecTmp;	// 一時置き
    HZX_ZON* 	pzone;		// ゾーンデータ
    HZX_ZON* 	pzoneTmp;	// ゾーンデータ

    nZoneRes = -1;			// 初期化
    pzone = ENE_HZX_GetZone( nZoneAddr );	// ゾーンアドレスからデータへのポインタを取得
    fInner = -1.f;				
    
    for ( i = 0; i < 6; i++){
	if ( pzone->nears[i] != HZX_NO_ZONE ){ // 隣接ゾーンから探す
	    nTmp = HZX_Address( npc->ctrl->hzx_id, pzone->nears[i], pzone->nears[i]);
	    if ( (GM_GetRIntrptZ2Z( npc->ctrl->addr, nTmp ) != NULL) ||
		 !GM_GetZIntrptZ2Z( npc->ctrl->addr, nTmp ) ) {
		continue;
	    }
	    pzoneTmp = ENE_HZX_GetZone( pzone->nears[i] );
	    fvecHzx.vx = pzoneTmp->x;
	    fvecHzx.vy = pzoneTmp->y;
	    fvecHzx.vz = pzoneTmp->z;
	    _sceVu0SubVector( &fvecTmp, &fvecHzx, &GM_PlayerPosition); 
	    _sceVu0Normalize( &fvecTmp, &fvecTmp);
	    fTmp = _sceVu0InnerProduct( pfvec, &fvecTmp); // 内積計算
	    if ( fTmp > fInner ){	// より正面に近いゾーンがみつかった
		nZoneRes = pzone->nears[i]; // 
		fInner = fTmp;	// 内積値更新
	    }
	}
    }
    // 内積結果取得
    if ( pfRes != NULL)
	(*pfRes) = fInner;

    return nZoneRes;
}
#endif
// ニキータイベント大統領の難易度取得
int NewGclGetPrezLevel( void )
{
    int nLevel;
    GCL_VAR_REF ref; 

    if ( _pworkPrez != NULL ) 	nLevel = _pworkPrez->nDifficult;
    else			nLevel = GM_GameLevel;

    if ( GCL_NextStr() == NULL ) return -1 ;
    GCL_GetNextVarRef( &ref ); // 参照データの取得
    GCL_SetVarRef( &ref, 0, nLevel );

    return 1;
}

// ニキータイベント大統領のライフ取得
int NewGclGetPrezLife( void )
{
    int nLife;
    GCL_VAR_REF ref; 

    if ( _pworkPrez != NULL ) 	nLife = _pworkPrez->npc.action.life;
    else			nLife = PREZ_LIFE;

    if ( GCL_NextStr() == NULL ) return -1 ;
    GCL_GetNextVarRef( &ref ); // 参照データの取得
    GCL_SetVarRef( &ref, 0, nLife );

    return 1;
}

// 最も近い壁検出
// 負の値は検出できなかった
int PREZ_GetWallDir( Work* work)
{
    FVECTOR vecWallDir;
    CONTROL* ctrl;

    ctrl = &work->control;

    if ( ctrl->n_touches == 0 ){ // 壁検出できなかった
	return (-1);
    }

    // 壁の方向を検出
    _sceVu0CopyVector( &vecWallDir, &ctrl->vecs[0]);
    _sceVu0Normalize( &vecWallDir, &vecWallDir);

    return (GV_VecDir2( &vecWallDir));
}

// 顔アニメフラグをセット
static void PREZ_SetFaceFlag( Work* work, int nMess)
{
    work->nFaceMess = nMess;
}

// 顔アニメキャラにモーション再生メッセージ
static void PREZ_SendFaceMess( Work* work, int nType)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { // メッセージ設定
	GV_MSG msg ;
	u_int buffer[  ] = { 0, nType };
	
	msg.address = work->nFaceChara;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int);
	GV_SendMessage( &msg ) ;
    }
}

#if 0
// 顔アニメキャラに視線制御メッセージ
static void PREZ_SendEyeMess( Work* work, int nInterpTime, int nType, FVECTOR* pvec)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { // メッセージ設定
	GV_MSG msg ;
	u_int buffer[  ] = { 1, nType, nInterpTime, (int)pvec->vx, (int)pvec->vy, (int)pvec->vz };

	msg.address = work->nFaceChara;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }

}
#endif
