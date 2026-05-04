/*
    kmmess.c
    カモメメッセージ機構
    2001/05/07 Masafumi Okuta
    $Id: kmmess.c,v 1.1.1.3 2002/11/19 11:48:03 Yoshizawa1 Exp $
*/
#if 0
// 指定点着地メッセージを反映
static void KMM_MessageLandPos( KAMOME* 	kamome, 
				int*		nMsg)
{
    int nDelay;
    FVECTOR vecTmp;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecTmp.vx = (float)nMsg[ 1 ];
    vecTmp.vy = (float)nMsg[ 2 ] + KAMOME_Y_OFFSET;
    vecTmp.vz = (float)nMsg[ 3 ];
    vecTmp.vw = 1.f;
    nDelay    = nMsg[ 4 ];

    // 着地先にカモメがいるかチェック
    if ( KMMNG_CheckLandKamomeInBox( &vecTmp, 120.f, 120.f, 120.f) ){
	return;
    }
    _sceVu0CopyVector( &kamome->vecLand, &vecTmp);

    // 着地方向の設定
    kamome->nLandYRot = KMM_GetDemoRandom( kmmng, 0, 4096);

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng, nDelay));
}
#endif
// 指定線上着地メッセージを反映
static void KMM_MessageLandLine( KAMOME* 	kamome, 
				 int*		nMsg)
{
    float   fDist, fRate;
    int     nDist, nPar, nDelay;
    FVECTOR vecFrom, vecTo, vecTmp;
    FVECTOR vecDir, vecOffset;
    static FVECTOR vecY	       = { 0.f, 1.f, 0.f, 1.f };
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecFrom.vx = (float)nMsg[ 1 ];	// 始点
    vecFrom.vy = (float)nMsg[ 2 ] + KAMOME_Y_OFFSET;
    vecFrom.vz = (float)nMsg[ 3 ];
    vecTo.vx   = (float)nMsg[ 4 ];	// 終点
    vecTo.vy   = (float)nMsg[ 5 ] + KAMOME_Y_OFFSET;
    vecTo.vz   = (float)nMsg[ 6 ];
    nPar       = nMsg[ 7 ];		// 割合( 1/1000単位指定 : 負でランダム)
    nDelay     = nMsg[ 8 ];		// ディレイ

    // 相対取得
    MAO_GetDiffVec3( &vecDir, &nDist, NULL, &vecFrom, &vecTo);

    if ( nDist == 0 ){ // ２点の距離０
	kamome->vecLand = vecFrom;
	KMM_SetThink3( kamome, TH3_LANDMOVE);
	return;
    }

    // 二点間を結ぶベクトルの何％オフセット
    fDist = (float)nDist;
    if ( nPar >= 0 ){ // ％指定
	fRate = (float)nPar / 1000.f * fDist;
    }else{ // 通し番号生成
	float fAddRate;
	fAddRate = 0.02f * frnd();
	if ( kmmng->nInitKmmNum > 0) fRate = (float)kamome->nNumber / (float)kmmng->nInitKmmNum + fAddRate;
	else			     fRate = 0.5f;
    }
    _sceVu0ScaleVector( &vecOffset, &vecDir, fRate);
    _sceVu0AddVector( &vecTmp, &vecFrom, &vecOffset);
    // 着地先にカモメがいるかチェック
    if ( KMMNG_CheckLandKamomeInBox( &vecTmp, 120.f, 120.f, 120.f) ){
	return;
    }
    _sceVu0CopyVector( &kamome->vecLand, &vecTmp);
    kamome->vecLand.vw = 1.f;

    _sceVu0OuterProduct( &kamome->vecLandZ, &vecDir, &vecY); // 着地方向の生成

    // 着地方向の設定
    {	
	SVECTOR vecRot;
	TS_VecToRot( &vecRot, &kamome->vecLandZ );
	kamome->nLandYRot = vecRot.vy + KMM_GetDemoRandom( kmmng, -256, 256);
    }

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// 指定エリア着地メッセージを反映
static void KMM_MessageLandArea( KAMOME* 	kamome, 
				 int*		nMsg)
{
    int nDelay;
    int	    nX, nY, nZ;
    FVECTOR vecCenter, vecTmp;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecCenter.vx = (float)nMsg[ 1 ];	// 中心点
    vecCenter.vy = (float)nMsg[ 2 ] + KAMOME_Y_OFFSET;
    vecCenter.vz = (float)nMsg[ 3 ];
    nX   	 = 	  nMsg[ 4 ];	// X幅
    nY   	 = 	  nMsg[ 5 ];	// Y幅
    nZ   	 = 	  nMsg[ 6 ];	// Z幅
    nDelay 	 = 	  nMsg[ 7 ];	// 命令遅延

    vecTmp.vx = vecCenter.vx + ((nX == 0) ? 0.f : (float)KMM_GetDemoRandom(kmmng, -nX, nX));
    vecTmp.vy = vecCenter.vy + ((nY == 0) ? 0.f : (float)KMM_GetDemoRandom(kmmng, -nY, nY));
    vecTmp.vz = vecCenter.vz + ((nZ == 0) ? 0.f : (float)KMM_GetDemoRandom(kmmng, -nZ, nZ));
    vecTmp.vw = 1.f;

    // 着地先にカモメがいるかチェック
    if ( KMMNG_CheckLandKamomeInBox( &vecTmp, 120.f, 120.f, 120.f) ){
	return;
    }
    _sceVu0CopyVector( &kamome->vecLand, &vecTmp);
    kamome->vecLand.vw = 1.f;


    // 着地方向の設定
    kamome->nLandYRot = KMM_GetDemoRandom( kmmng, 0, 4096);
    
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// 指定点着地メッセージを反映
static void KMM_MessageFly( KAMOME* 	kamome, 
			    int*	nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 1 ];
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLYING );
    else    		KMM_SetThink3Delay( kamome, TH3_FLYING, KMM_DemoRand( kmmng,  nDelay));
}

static HOMING_TRG* MAO_SearchEneHomingTrg( int nName)
{
    HOMING_TRG* phtrg;

    for ( phtrg = GM_GetHoming(); phtrg != NULL; phtrg = phtrg->next ) {
	if ( !(phtrg->status & HOMING_ENEMY) || (phtrg->status & HOMING_UNREAL) ){
	   // BP_WARNING - The below if is wrong !a&b instead of !(a&b) 
      if ( !phtrg->status & HOMING_NPC)    continue;
	}

	if ( phtrg->ctrl != NULL && phtrg->trg != NULL ){
	    if ( phtrg->ctrl->name == nName ){
		return phtrg;
	    }
	}
    }

    return NULL;
}

static int KMM_SearchAim( KAMOME* kamome,
			  int     nName)
{
    HOMING_TRG* phtrg;


    if ( GM_PlayerControl != NULL && nName == GM_PlayerControl->name ){ // プレイヤー
	kamome->pctrlAim = GM_PlayerControl;
	kamome->ptrgAim  = GM_PlayerTarget;
	kamome->nAimName = nName;
	return 1;
    }else{ // 敵兵
	phtrg = MAO_SearchEneHomingTrg( nName ); 
	if ( phtrg != NULL){
	    kamome->pctrlAim = phtrg->ctrl; 
	    kamome->ptrgAim  = phtrg->trg;
	    kamome->nAimName = nName;
	    return 1;
	}else{
	    kamome->pctrlAim = NULL; 
	    kamome->ptrgAim  = NULL;
	}
    }
    return 0;
}

// 全カモメ攻撃メッセージを反映
static void KMM_MessageAttack( KAMOME* 	kamome, 
			    int*	nMsg)
{
    int nDelay, nName;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay  = nMsg[ 1 ]; // 遅延
    nName   = nMsg[ 2 ]; // 名前

    if ( KMM_SearchAim( kamome, nName) ){
	kamome->nAimName = nName;
	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_ATCMOVE );
	else    		KMM_SetThink3Delay( kamome, TH3_ATCMOVE, KMM_DemoRand( kmmng,  nDelay));
    }else{
#ifdef DEBUG_MODE
	printf("cannot find attack target :: %d\n", nName);
#endif
    }
}
// 指定数カモメ攻撃メッセージを反映
static void KMM_MessageAttackNum( KAMOME* 	kamome, 
				  int*		nMsg)
{
    int nDelay, nName;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nName   = nMsg[ 2 ]; // 名前
    nDelay  = nMsg[ 3 ]; // 遅延

    if ( KMM_SearchAim( kamome, nName) ){
	kamome->nAimName = nName;
	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_ATCMOVE );
	else    		KMM_SetThink3Delay( kamome, TH3_ATCMOVE, KMM_DemoRand( kmmng,  nDelay));
    }else{
#ifdef DEBUG_MODE
	printf("cannot find attack target :: %d\n", nName);
#endif
    }
}

// 指定キャラの頭上付随メッセージを反映
static void KMM_MessageConcomitant( KAMOME* 	kamome, 
				    int*	nMsg)
{
    int nDelay, nName;
    HOMING_TRG* phtrg;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay  = nMsg[ 1 ]; // 遅延
    nName   = nMsg[ 2 ]; // 名前

    if ( GM_PlayerControl != NULL && nName == GM_PlayerControl->name ){ // プレイヤー
	kamome->pctrlAim = GM_PlayerControl;
	kamome->ptrgAim  = GM_PlayerTarget;
    }else{ // 敵兵
	phtrg = MAO_SearchEneHomingTrg( nName ); 
	if ( phtrg != NULL ){
	    kamome->pctrlAim = phtrg->ctrl; 
	    kamome->ptrgAim  = phtrg->trg;
	}
    }

    if ( kamome->pctrlAim != NULL && kamome->ptrgAim != NULL ){
	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_CONCMOVE );
	else    		KMM_SetThink3Delay( kamome, TH3_CONCMOVE, KMM_DemoRand( kmmng,  nDelay));
    }else{
#ifdef DEBUG_MODE
	printf("cannot find cont target :: %d\n", nName);
#endif
    }
}
// 死亡メッセージを反映
static void KMM_MessageFall( KAMOME* 	kamome, 
			    int*	nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 1 ];

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FALL );
    else		KMM_SetThink3Delay( kamome, TH3_FALL, KMM_DemoRand( kmmng,  nDelay));
}
// 指定範囲内にいるカモメの飛行エリアを変更する
static void KMM_MessageChangeAreaSphere( KAMOME* 	kamome, 
					 int*		nMsg)
{
    int		nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 1～3は中心, 4は半径
    nTrap 		 = nMsg[ 5 ];
    nDelay 		 = nMsg[ 6 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLYING );
	else    		KMM_SetThink3Delay( kamome, TH3_FLYING, KMM_DemoRand( kmmng,  nDelay));
    }
}
// 指定トラップ内にいるカモメの飛行エリアを変更する
static void KMM_MessageChangeAreaTrap( KAMOME* 	kamome, 
					 int*		nMsg)
{
    int		nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 1は検索トラップ
    nTrap 		 = nMsg[ 2 ];
    nDelay 		 = nMsg[ 3 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLYING );
	else    		KMM_SetThink3Delay( kamome, TH3_FLYING, KMM_DemoRand( kmmng,  nDelay));
    }
}
// 指定トラップ内にいるカモメの飛行エリアを変更する
static void KMM_MessageChgAreaInBox( KAMOME* 	kamome, 
				     int*	nMsg)
{
    int		nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecMoveArea1.vx = (float)nMsg[ 7 ];
    kamome->vecMoveArea1.vy = (float)nMsg[ 8 ];
    kamome->vecMoveArea1.vz = (float)nMsg[ 9 ];
    kamome->vecMoveArea2.vx = (float)nMsg[ 10 ];
    kamome->vecMoveArea2.vy = (float)nMsg[ 11 ];
    kamome->vecMoveArea2.vz = (float)nMsg[ 12 ];
    nDelay 		    = nMsg[ 13 ];


    KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLYING );
    else    		KMM_SetThink3Delay( kamome, TH3_FLYING, KMM_DemoRand( kmmng,  nDelay));
}
// エルード妨害 : ライン指定
static void KMM_MessageEludeLine( KAMOME* 	kamome, 
				 int*		nMsg)
{
    int	    nDist, nDelay, nAdd, nOffset, nPar;
    float	fDist, fRate;
    FVECTOR vecFrom, vecTo, vecDir, vecOffset;
    KAMOME_MNG* kmmng;
    static FVECTOR vecY	       = { 0.f, 1.f, 0.f, 1.f };

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 1はトラップ名
    vecFrom.vx 		= (float)nMsg[ 2 ];	// 始点
    vecFrom.vy 		= (float)nMsg[ 3 ] + KAMOME_Y_OFFSET;
    vecFrom.vz 		= (float)nMsg[ 4 ];
    vecTo.vx   		= (float)nMsg[ 5 ];	// 終点
    vecTo.vy   		= (float)nMsg[ 6 ] + KAMOME_Y_OFFSET;
    vecTo.vz   		= (float)nMsg[ 7 ];
    nPar       		= nMsg[ 8 ];		// 割合( 1/1000単位指定 : 負でランダム)
    nAdd		= nMsg[ 9 ];		// 誤差割合( 1/1000単位指定)
    kamome->nRestTime	= KMM_GetDemoRandom(kmmng,  nMsg[ 10 ], nMsg[ 11 ]);	// アイドリング時間 
    nDelay		= nMsg[ 12 ];		// 命令遅延

    // 相対取得
    MAO_GetDiffVec3( &vecDir, &nDist, NULL, &vecFrom, &vecTo);

    if ( nDist == 0 ){ // ２点の距離０
	kamome->vecLand = vecFrom;
	KMM_SetThink3( kamome, TH3_LANDMOVE);
	return;
    }
    
    // 二点間を結ぶベクトルの何％オフセット
    fDist = (float)nDist;

    if ( nAdd != 0 ) nOffset = nPar + KMM_GetDemoRandom( kmmng, -nAdd, nAdd);
    else	     nOffset = nPar;

    if ( nOffset < 0) nOffset = 0;
    else if ( nOffset > 1000 ) nOffset = 1000;

    if ( nPar >= 0 ){ // ％指定
	fRate = (float)nOffset / 1000.f * fDist;
    }else{ // ％ランダム生成
	fRate = ((float)KMM_DemoRand( kmmng, nDist));
    }
    _sceVu0ScaleVector( &vecOffset, &vecDir, fRate);
    _sceVu0AddVector( &kamome->vecLand, &vecFrom, &vecOffset);
    kamome->vecLand.vw = 1.f;

    // 着地方向の生成
    _sceVu0OuterProduct( &kamome->vecLandZ, &vecDir, &vecY); 

    // 着地方向の設定
    {	
	SVECTOR vecRot;
	TS_VecToRot( &vecRot, &kamome->vecLandZ );
	kamome->nLandYRot = vecRot.vy + KMM_GetDemoRandom( kmmng, -256, 256);
    }

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_ELUDEMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_ELUDEMOVE, KMM_DemoRand( kmmng,  nDelay));
}

// ばたつき集合メッセージを反映
static void KMM_MessageFlipflop( KAMOME* 	kamome, 
				 int*		nMsg)
{
    int 	nDelay;
    int	    	nTrapName;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecAggAim.vx = (float)nMsg[ 1 ];	// 注目点
    kamome->vecAggAim.vy = (float)nMsg[ 2 ] + KAMOME_Y_OFFSET;
    kamome->vecAggAim.vz = (float)nMsg[ 3 ];
    kamome->vecAggAim.vw = 1.f;
    nTrapName	 = 	  nMsg[ 4 ];	// トラップ名
    nDelay 	 = 	  nMsg[ 5 ];	// 命令遅延

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!\n", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	kamome->vecMoveArea1.vx = blk->tx + trp->b1.vx;
	kamome->vecMoveArea1.vy = blk->ty + trp->b1.vy;
	kamome->vecMoveArea1.vz = blk->tz + trp->b1.vz;
	kamome->vecMoveArea2.vx = blk->tx + trp->b2.vx;
	kamome->vecMoveArea2.vy = blk->ty + trp->b2.vy;
	kamome->vecMoveArea2.vz = blk->tz + trp->b2.vz;

	kamome->vecLand.vx = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vx, (int)kamome->vecMoveArea2.vx);
	kamome->vecLand.vy = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vy, (int)kamome->vecMoveArea2.vy);
	kamome->vecLand.vz = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vz, (int)kamome->vecMoveArea2.vz);
    }

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLIPMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_FLIPMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// ばたつき集合メッセージ(エリア)を反映
static void KMM_MessageAggregate( KAMOME* 	kamome, 
				  int*		nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecAggAim.vx = (float)nMsg[ 1 ];	// 注目点
    kamome->vecAggAim.vy = (float)nMsg[ 2 ] + KAMOME_Y_OFFSET;
    kamome->vecAggAim.vz = (float)nMsg[ 3 ];
    kamome->vecAggAim.vw = 1.f;

    {
	kamome->vecMoveArea1.vx = (float)nMsg[ 4 ];
	kamome->vecMoveArea1.vy = (float)nMsg[ 5 ];
	kamome->vecMoveArea1.vz = (float)nMsg[ 6 ];
	kamome->vecMoveArea2.vx = (float)nMsg[ 7 ];
	kamome->vecMoveArea2.vy = (float)nMsg[ 8 ];
	kamome->vecMoveArea2.vz = (float)nMsg[ 9 ];

	kamome->vecLand.vx = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vx, (int)kamome->vecMoveArea2.vx);
	kamome->vecLand.vy = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vy, (int)kamome->vecMoveArea2.vy);
	kamome->vecLand.vz = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vz, (int)kamome->vecMoveArea2.vz);
    }

    nDelay 	 = 	  nMsg[ 10 ];	// 命令遅延


    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLIPMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_FLIPMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// ばたつき集合メッセージ(エリア)を反映
static void KMM_MessageAggregateFromTrap( KAMOME* 	kamome, 
					  int*		nMsg)
{
    int nDelay, nTrapName;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecAggAim.vx = (float)nMsg[ 3 ];	// 注目点
    kamome->vecAggAim.vy = (float)nMsg[ 4 ] + KAMOME_Y_OFFSET;
    kamome->vecAggAim.vz = (float)nMsg[ 5 ];
    kamome->vecAggAim.vw = 1.f;
    nTrapName		 = nMsg[ 6 ];
    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!\n", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	kamome->vecMoveArea1.vx = blk->tx + trp->b1.vx;
	kamome->vecMoveArea1.vy = blk->ty + trp->b1.vy;
	kamome->vecMoveArea1.vz = blk->tz + trp->b1.vz;
	kamome->vecMoveArea2.vx = blk->tx + trp->b2.vx;
	kamome->vecMoveArea2.vy = blk->ty + trp->b2.vy;
	kamome->vecMoveArea2.vz = blk->tz + trp->b2.vz;

	kamome->vecLand.vx = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vx, (int)kamome->vecMoveArea2.vx);
	kamome->vecLand.vy = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vy, (int)kamome->vecMoveArea2.vy);
	kamome->vecLand.vz = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vz, (int)kamome->vecMoveArea2.vz);
    }

    nDelay 	 = 	  nMsg[ 7 ];	// 命令遅延

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLIPMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_FLIPMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// ばたつき集合メッセージ(エリア)を反映
static void KMM_MessageAggregateFromArea( KAMOME* 	kamome, 
					  int*		nMsg)
{
    int nDelay, nTrapName;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecAggAim.vx = (float)nMsg[ 8 ];	// 注目点
    kamome->vecAggAim.vy = (float)nMsg[ 9 ] + KAMOME_Y_OFFSET;
    kamome->vecAggAim.vz = (float)nMsg[ 10 ];
    kamome->vecAggAim.vw = 1.f;

    nTrapName		 = nMsg[ 11 ];
    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrapName, &blk, &trp);

	if ( blk == NULL || trp == NULL){
#ifdef DEBUG_MODE
	    MAO_PRINTF("trap == %d not found!!\n", nTrapName);
	    ASSERT( 0 );
#endif
	    return;
	}

	kamome->vecMoveArea1.vx = blk->tx + trp->b1.vx;
	kamome->vecMoveArea1.vy = blk->ty + trp->b1.vy;
	kamome->vecMoveArea1.vz = blk->tz + trp->b1.vz;
	kamome->vecMoveArea2.vx = blk->tx + trp->b2.vx;
	kamome->vecMoveArea2.vy = blk->ty + trp->b2.vy;
	kamome->vecMoveArea2.vz = blk->tz + trp->b2.vz;

	kamome->vecLand.vx = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vx, (int)kamome->vecMoveArea2.vx);
	kamome->vecLand.vy = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vy, (int)kamome->vecMoveArea2.vy);
	kamome->vecLand.vz = KMM_GetDemoRandom(kmmng, (int)kamome->vecMoveArea1.vz, (int)kamome->vecMoveArea2.vz);
    }

    nDelay 	 = 	  nMsg[ 12 ];	// 命令遅延

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FLIPMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_FLIPMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// 緊急飛び立ちメッセージを反映
static void KMM_MessageTakeoff( KAMOME* 	kamome, 
				int*		nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 1 ];
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
}
// 指定球内緊急飛び立ちメッセージを反映
static void KMM_MessageTakeoffInSphere( KAMOME* 	kamome, 
					int*		nMsg)
{
    int nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nTrap  = nMsg[ 5 ];
    kamome->fSpeed = (float)nMsg[ 6 ] / kmmng->fBaseSpeed;
    nDelay = nMsg[ 7 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
	else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
    }
}
// 指定球内緊急散開メッセージを反映
static void KMM_MessageDeployInSphere( KAMOME* 	kamome, 
				       int*	nMsg)
{
    int nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nTrap  = nMsg[ 5 ];
    kamome->fSpeed = (float)nMsg[ 6 ] / kmmng->fBaseSpeed;
    nDelay = nMsg[ 7 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( kamome->think.nThink3 == TH3_FLIPFLOP || kamome->think.nThink3 == TH3_LANDING ){
	    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_BACKOFF );
	    else    		KMM_SetThink3Delay( kamome, TH3_BACKOFF, KMM_DemoRand( kmmng,  nDelay));
	}else{
	    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
	    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
	}
    }
}
// 指定トラップ内にいるカモメを離陸させる
static void KMM_MessageTakeoffInBox( KAMOME* 	kamome, 
				    int*	nMsg)
{
    int		nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecMoveArea1.vx = (float)nMsg[ 7 ];
    kamome->vecMoveArea1.vy = (float)nMsg[ 8 ];
    kamome->vecMoveArea1.vz = (float)nMsg[ 9 ];
    kamome->vecMoveArea2.vx = (float)nMsg[ 10 ];
    kamome->vecMoveArea2.vy = (float)nMsg[ 11 ];
    kamome->vecMoveArea2.vz = (float)nMsg[ 12 ];
    kamome->fSpeed 	    = (float)nMsg[ 13 ] / kmmng->fBaseSpeed;
    nDelay 		    = 	     nMsg[ 14 ];

    KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
}
// 指定トラップ内にいるカモメを離散させる
static void KMM_MessageDeployInBox( KAMOME* 	kamome, 
				    int*	nMsg)
{
    int		nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->vecMoveArea1.vx = (float)nMsg[ 7 ];
    kamome->vecMoveArea1.vy = (float)nMsg[ 8 ];
    kamome->vecMoveArea1.vz = (float)nMsg[ 9 ];
    kamome->vecMoveArea2.vx = (float)nMsg[ 10 ];
    kamome->vecMoveArea2.vy = (float)nMsg[ 11 ];
    kamome->vecMoveArea2.vz = (float)nMsg[ 12 ];
    kamome->fSpeed 	    = (float)nMsg[ 13 ] / kmmng->fBaseSpeed;
    nDelay 		    = 	     nMsg[ 14 ];

    KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

    if ( kamome->think.nThink3 == TH3_FLIPFLOP || kamome->think.nThink3 == TH3_LANDING ){
	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_BACKOFF );
	else    		KMM_SetThink3Delay( kamome, TH3_BACKOFF, KMM_DemoRand( kmmng,  nDelay));
    }else{
	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
	else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
    }
}
// 指定トラップ内緊急離陸メッセージを反映
static void KMM_MessageTakeoffInTrap( KAMOME* 	kamome, 
				     int*	nMsg)
{
    int nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nTrap  = nMsg[ 2 ];
    kamome->fSpeed = (float)nMsg[ 3 ] / kmmng->fBaseSpeed;
    nDelay = nMsg[ 4 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
	else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
    }
}
// 指定トラップ内緊急散開メッセージを反映
static void KMM_MessageDeployInTrap( KAMOME* 	kamome, 
				     int*	nMsg)
{
    int nDelay, nTrap;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nTrap  = nMsg[ 2 ];
    kamome->fSpeed = (float)nMsg[ 3 ] / kmmng->fBaseSpeed;
    nDelay = nMsg[ 4 ];

    {
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	kamome->vecMoveArea1.vx = (float)(blk->tx + trp->b1.vx);
	kamome->vecMoveArea1.vy = (float)(blk->ty + trp->b1.vy);
	kamome->vecMoveArea1.vz = (float)(blk->tz + trp->b1.vz);
	kamome->vecMoveArea2.vx = (float)(blk->tx + trp->b2.vx);
	kamome->vecMoveArea2.vy = (float)(blk->ty + trp->b2.vy);
	kamome->vecMoveArea2.vz = (float)(blk->tz + trp->b2.vz);

	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);

	if ( kamome->think.nThink3 == TH3_FLIPFLOP || kamome->think.nThink3 == TH3_LANDING ){
	    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_BACKOFF );
	    else    		KMM_SetThink3Delay( kamome, TH3_BACKOFF, KMM_DemoRand( kmmng,  nDelay));
	}else{
	    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
	    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));
	}
    }
}

// 指定球内のカモメの中から指定数を線着地させるメッセージを反映
static void KMM_MessageLandLineInSphere( KAMOME* 	kamome, 
					 int*		nMsg)
{
    float   fRate, fAddRate;
    int     nDist, nDelay, nLandTimeMin, nLandTimeMax;
    FVECTOR vecFrom, vecTo, vecTmp;
    FVECTOR vecDir, vecOffset;
    static FVECTOR vecY	       = { 0.f, 1.f, 0.f, 1.f };

    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecFrom.vx = (float)nMsg[ 6 ];	// 始点
    vecFrom.vy = (float)nMsg[ 7 ] + KAMOME_Y_OFFSET;
    vecFrom.vz = (float)nMsg[ 8 ];
    vecTo.vx   = (float)nMsg[ 9 ];	// 終点
    vecTo.vy   = (float)nMsg[ 10 ] + KAMOME_Y_OFFSET;
    vecTo.vz   = (float)nMsg[ 11 ];

    nLandTimeMin = nMsg[ 12 ];		// 着地時間
    nLandTimeMax = nMsg[ 13 ];
    nDelay 	 = nMsg[ 14 ];		// ディレイ時間

    // 線上のどの位置にとまるか始点からの割合で求める
    if ( kmmng->ucMsgMax[kamome->nMsgIndex] != 0){
	fAddRate = 0.05f * frnd();
	if ( kmmng->nInitKmmNum > 0) fRate = (float)kamome->nNumber / (float)kmmng->nInitKmmNum + fAddRate;
	else			     fRate = 0.5f;
    }else{
	fRate = 1.f; 
    }

    // 相対取得
    MAO_GetDiffVec3( &vecDir, &nDist, NULL, &vecFrom, &vecTo);

    // 着地点を設定
    _sceVu0ScaleVector( &vecOffset, &vecDir, fRate * (float)nDist);
    _sceVu0AddVector( &vecTmp, &vecFrom, &vecOffset);

    // 着地先にカモメがいるかチェック
    if ( KMMNG_CheckLandKamomeInBox( &vecTmp, 120.f, 120.f, 120.f) ){
	return;
    }

    _sceVu0CopyVector( &kamome->vecLand, &vecTmp);
    kamome->vecLand.vw = 1.f;

    _sceVu0OuterProduct( &kamome->vecLandZ, &vecDir, &vecY); // 着地方向の生成

    // 着地方向の設定
    {	
	SVECTOR vecRot;
	TS_VecToRot( &vecRot, &kamome->vecLandZ );
	kamome->nLandYRot = vecRot.vy + KMM_GetDemoRandom( kmmng, -256, 256);
    }

    kamome->nRestTime = KMM_GetDemoRandom(kmmng, nLandTimeMin, nLandTimeMax);
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng,  nDelay));
}
// 指定トラップ内のカモメの中から指定数を線着地させるメッセージを反映
static void KMM_MessageLandLineInTrap( KAMOME* 	kamome, 
				       int*	nMsg)
{
    float   fRate, fAddRate;
    int     nDist, nDelay, nLandTimeMin, nLandTimeMax;
    FVECTOR vecFrom, vecTo, vecTmp;
    FVECTOR vecDir, vecOffset;
    static FVECTOR vecY	       = { 0.f, 1.f, 0.f, 1.f };

    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecFrom.vx = (float)nMsg[ 3 ];	// 始点
    vecFrom.vy = (float)nMsg[ 4 ] + KAMOME_Y_OFFSET;
    vecFrom.vz = (float)nMsg[ 5 ];
    vecTo.vx   = (float)nMsg[ 6 ];	// 終点
    vecTo.vy   = (float)nMsg[ 7 ] + KAMOME_Y_OFFSET;
    vecTo.vz   = (float)nMsg[ 8 ];

    nLandTimeMin = nMsg[ 9 ];		// 着地時間
    nLandTimeMax = nMsg[ 10 ];
    nDelay 	 = nMsg[ 11 ];		// ディレイ時間

    // 線上のどの位置にとまるか始点からの割合で求める
    if ( kmmng->ucMsgMax[kamome->nMsgIndex] != 0){
	fAddRate = 0.05f * frnd();
	if ( kmmng->nInitKmmNum > 0) fRate = (float)kamome->nNumber / (float)kmmng->nInitKmmNum + fAddRate;
	else			     fRate = 0.5f;
    }else{
	fRate = 1.f; 
    }
    if (fRate > 1.f) fRate = 1.f;
    else if (fRate < 0.f) fRate = 0.f;

    // 相対取得
    MAO_GetDiffVec3( &vecDir, &nDist, NULL, &vecFrom, &vecTo);

    // 着地点を設定
    _sceVu0ScaleVector( &vecOffset, &vecDir, (fRate * (float)nDist));
    _sceVu0AddVector( &vecTmp, &vecFrom, &vecOffset);

    // 着地先にカモメがいるかチェック
    if ( KMMNG_CheckLandKamomeInBox( &vecTmp, 180.f, 120.f, 180.f) ){
	return;
    }

    _sceVu0CopyVector( &kamome->vecLand, &vecTmp);
    kamome->vecLand.vw = 1.f;

    _sceVu0OuterProduct( &kamome->vecLandZ, &vecDir, &vecY); // 着地方向の生成

    // 着地方向の設定
    {	
	SVECTOR vecRot;
	TS_VecToRot( &vecRot, &kamome->vecLandZ );
	kamome->nLandYRot = vecRot.vy + KMM_GetDemoRandom( kmmng, -256, 256);
    }

    kamome->nRestTime = KMM_GetDemoRandom(kmmng, nLandTimeMin, nLandTimeMax);
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng,  nDelay));
}

// 死亡メッセージを反映
static void KMM_MessageFallInArea( KAMOME* 	kamome, 
				   int*		nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 7 ];

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_FALL );
    else		KMM_SetThink3Delay( kamome, TH3_FALL, KMM_DemoRand( kmmng,  nDelay));
}

// 高さ加算メッセージを反映
static void KMM_MessageAddYInArea( KAMOME* 	kamome, 
				   int*		nMsg)
{
    int nAddMin, nAddMax;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nAddMin = nMsg[ 7 ];
    nAddMax = nMsg[ 8 ];

    kamome->mov.vy += KMM_GetDemoRandom(kmmng, nAddMin, nAddMax);
    kamome->vecMove.vy = kamome->mov.vy;
}


// 指定トラップに着地
static void KMM_MessageLandTrap( KAMOME* 	kamome, 
				 int*		nMsg)
{
    int	    	nTrap, nBaseY, nAddY, nDelay;
    float	fRateX, fRateZ, fAddRate;
    FVECTOR vecFrom, vecTo;

    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // １は匹数,２は検索トラップ
    nTrap  = nMsg[ 3 ];  // 着地トラップ
    nBaseY  = nMsg[ 4 ]; // 基準方向
    nAddY  = nMsg[ 5 ];  // ランダム範囲
    kamome->nRestTime = KMM_GetDemoRandom(kmmng,  nMsg[ 6 ], nMsg[ 7 ]);	// アイドリング時間 
    nDelay = nMsg[ 8 ];  // ディレイ幅

    { // 着地トラップのサイズ取得
	HZX_BLOCK* 	blk;
	HZX_TRP* 	trp;
	KAMOME_MNG* kmmng;
	kmmng = (KAMOME_MNG*)kamome->pvManager;

	// 着地用トラップのサイズ取得
	HZX_FindTrap(GM_GetHzxGroupID( kamome->map ), nTrap, &blk, &trp);
	ASSERT( blk != NULL);
	ASSERT( trp != NULL);
	vecFrom.vx = (float)(blk->tx + trp->b1.vx);
	vecFrom.vy = (float)(blk->ty + trp->b1.vy);
	vecFrom.vz = (float)(blk->tz + trp->b1.vz);
	vecTo.vx   = (float)(blk->tx + trp->b2.vx);
	vecTo.vy   = (float)(blk->ty + trp->b2.vy);
	vecTo.vz   = (float)(blk->tz + trp->b2.vz);
    }

    // X値分を設定
    if ( kmmng->ucMsgCntr[kamome->nMsgIndex] % 2 == 0){  // 偶数
	fRateX = (KMM_GetFRnd( kmmng) * 0.4f + 0.05f ); // 左半分
    }else{ 						// 奇数
	fRateX = (KMM_GetFRnd( kmmng) * 0.4f + 0.55f ); // 右半分
    }
    // Z値分を設定
    if ( kmmng->ucMsgMax[kamome->nMsgIndex] != 0){
	fAddRate = (0.8f / (float)kmmng->ucMsgMax[kamome->nMsgIndex]) * frnd();
	fRateZ = (float)kmmng->ucMsgCntr[kamome->nMsgIndex] / (float)kmmng->ucMsgMax[kamome->nMsgIndex] + fAddRate;
    }else{
	fRateZ = 1.f; 
    }
    // 着地点の設定
    kamome->vecLand.vx = vecFrom.vx * fRateX + vecTo.vx * ( 1.f - fRateX);
    kamome->vecLand.vy = vecFrom.vy + KAMOME_Y_OFFSET;
    kamome->vecLand.vz = vecFrom.vz * fRateZ + vecTo.vz * ( 1.f - fRateZ);
    kamome->vecLand.vw = 1.f;

    // 着地方向の設定
    kamome->nLandYRot = nBaseY;
    if ( nAddY != 0 ) kamome->nLandYRot += KMM_GetDemoRandom(kmmng, -nAddY, nAddY);

    // 着地命令
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_LANDMOVE );
    else    		KMM_SetThink3Delay( kamome, TH3_LANDMOVE, KMM_DemoRand( kmmng, nDelay));
}
// 指定キャラの関節着地メッセージを反映
static void KMM_MessageLandObjectNode( KAMOME* 	kamome, 
				       int*	nMsg)
{
    int 	nDelay;
    FVECTOR 	vecPos;
    CONTROL* 	pctrl;
    OBJECT*	pbody;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->nAimName		= nMsg[ 7 ];	// 名前
    kamome->nNode 		= nMsg[ 8 ];	// 関節
    kamome->nLandYRot 		= nMsg[ 9 ];	// 方向
    kamome->vecOffset.vx 	= (float)nMsg[ 10 ];
    kamome->vecOffset.vy 	= (float)nMsg[ 11 ];
    kamome->vecOffset.vz 	= (float)nMsg[ 12 ];
    kamome->vecOffset.vw 	= 1.f;
    nDelay 	        	= nMsg[ 13 ];

    pctrl = GM_SearchWhere(kamome->nAimName);

    if ( pctrl == NULL ) return ;

    pbody = pctrl->object;

    _sceVu0ApplyMatrix( &vecPos, &BODYWORLD( pbody, kamome->nNode), &kamome->vecOffset );

    kamome->vecLand.vx = vecPos.vx;
    kamome->vecLand.vy = vecPos.vy + KAMOME_Y_OFFSET;
    kamome->vecLand.vz = vecPos.vz;
    kamome->vecLand.vw = 1.f;
    
    // 着地方向の設定

    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_CTRL_LAND );
    else    		KMM_SetThink3Delay( kamome, TH3_CTRL_LAND, KMM_DemoRand( kmmng, nDelay));
}
// 元の飛行エリアに戻るためカモメを離陸
static void KMM_MessageTakeoffTurnBack( KAMOME* 	kamome, 
				       int*	nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 7 ];
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));

}
// 付随を止める
static void KMM_MessageConcomitantEnd( KAMOME* 	kamome, 
				       int*	nMsg)
{
    int nDelay;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nDelay = nMsg[ 2 ];
    if ( nDelay == 0) 	KMM_SetThink3( kamome, TH3_TAKEOFF );
    else    		KMM_SetThink3Delay( kamome, TH3_TAKEOFF, KMM_DemoRand( kmmng,  nDelay));

}

// メッセージ反映
static inline void KMM_SettingMessage( KAMOME* kamome, int code, int* pmsg)
{
    switch ( code ){
    case KMNG_MESS_ALL_LAND_LINE:	// みんな着地:線
	KMM_MessageLandLine( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_LAND_AREA:	// みんな着地:エリア
	KMM_MessageLandArea( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_FLY:		// みんな飛び立て
	KMM_MessageFly( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_ATTACK:		// みんな攻撃
	KMM_MessageAttack( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_CONCOMITANT:	// みんな頭上付随
	KMM_MessageConcomitant( kamome, pmsg );
	break;
    case KMNG_MESS_BLOCKELUDE_LINE:	// エルード妨害
	KMM_MessageEludeLine( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_FLIPFLOP:	// みんなばたばた集合:トラップ指定
	KMM_MessageFlipflop( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_AGGREGATE:	// みんなばたばた集合:エリア指定
	KMM_MessageAggregate( kamome, pmsg );
	break;
    case KMNG_MESS_CHANGE_AREA_SPHERE:	// 指定球内エリア変更
	KMM_MessageChangeAreaSphere( kamome, pmsg );
	break;
    case KMNG_MESS_CHANGE_AREA_TRAP:	// 指定球内エリア変更
	KMM_MessageChangeAreaTrap( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_TAKEOFF:	// みんな緊急飛び立ち
	KMM_MessageTakeoff( kamome, pmsg );
	break;
    case KMNG_MESS_ALL_DEAD:	// みんな死亡
	KMM_MessageFall( kamome, pmsg );
	break;
    case KMNG_MESS_TRAP_AGGREGATE:	// みんな集合:トラップ指定
	KMM_MessageAggregateFromTrap( kamome, pmsg );
	break;
    case KMNG_MESS_AREA_AGGREGATE:	// みんな集合:エリア指定
	KMM_MessageAggregateFromArea( kamome, pmsg );
	break;
    case KMNG_MESS_TAKEOFF_SPHERE:  // 指定球内緊急飛び立ち
	KMM_MessageTakeoffInSphere( kamome, pmsg );
	break;
    case KMNG_MESS_DEPLOY_SPHERE:  // 指定球内緊急散開
	KMM_MessageDeployInSphere( kamome, pmsg );
	break;
    case KMNG_MESS_TAKEOFF_TRAP:  // 指定トラップ内緊急飛び立ち
	KMM_MessageTakeoffInTrap( kamome, pmsg );
	break;
    case KMNG_MESS_DEPLOY_TRAP:  // 指定トラップ内緊急散開
	KMM_MessageDeployInTrap( kamome, pmsg );
	break;
    case KMNG_MESS_LAND_LINE_SPHERE:// 指定球内から搾取して線着地	 
	KMM_MessageLandLineInSphere( kamome, pmsg );
	break;
    case KMNG_MESS_LAND_LINE_TRAP:	// 指定トラップ内から搾取して線着地
	KMM_MessageLandLineInTrap( kamome, pmsg );
	break;
    case KMNG_MESS_LAND_AREA_SPHERE:	// 指定球内から搾取して空間着地
	break;
    case KMNG_MESS_LAND_AREA_TRAP:	// 指定トラップ内から搾取してトラップ着地
	KMM_MessageLandTrap( kamome, pmsg );
	break;
    case KMNG_MESS_AREA_DEAD:	// 指定範囲内死亡
	KMM_MessageFallInArea( kamome, pmsg );
	break;
    case KMNG_MESS_ADD_Y:		// 指定範囲内高さを加算
	KMM_MessageAddYInArea( kamome, pmsg );
	break;
    case KMNG_MESS_CHANGE_AREA_BOX:	// 指定範囲内の飛行エリア変更
	KMM_MessageChgAreaInBox( kamome, pmsg );
	break;
    case KMNG_MESS_TAKEOFF_BOX:		// 指定範囲内のカモメを離陸
	KMM_MessageTakeoffInBox( kamome, pmsg );
	break;
    case KMNG_MESS_DEPLOY_BOX:		// 指定範囲内のカモメを離散
	KMM_MessageDeployInBox( kamome, pmsg );
	break;
    case KMNG_MESS_NUM_ATTACK:		// 指定数で攻撃
	KMM_MessageAttackNum( kamome, pmsg );
	break;
    case KMNG_MESS_COMEON:		// よびよせ
	KMM_MessageLandObjectNode( kamome, pmsg );
	break;
    case KMNG_MESS_TAKEOFF_TURNBACK:	// 元の飛行エリアに戻るためカモメを離陸
	KMM_MessageTakeoffTurnBack( kamome, pmsg );
	break;
    case KMNG_MESS_COMEON_END:		// 付随をやめる
	KMM_MessageConcomitantEnd( kamome, pmsg );
	break;
    }
}

// メッセージ取得
static void KMM_RecieveMessage( KAMOME* kamome)
{
    KAMOME_MNG* kmmng;
    GV_MSG*	msg;
    int 	n_msg, code;

    // かもめマネージャからのメッセージ
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // name = 0ならなにもしない
    if ( kamome->name == 0 ){ return; }
    
    n_msg = GV_ReceiveMessage( kamome->name, &kamome->msg ); // メッセージ取得

    msg = kamome->msg;

    // 個人宛メッセージを反映
    while ( n_msg-- > 0 ){ // 自分に対して飛ばされるメッセージ
	code = msg->message[ 0 ] ;
	KMM_SettingMessage( kamome, code, &msg->message[ 0 ]);
	msg++;
    }

    // かもめマネージャ宛メッセージを反映
    if ( kamome->nMsgIndex >= 0 && kamome->nMsgIndex < KMNG_MESS_MAX ){ 
	msg = &kmmng->msg[ kamome->nMsgIndex ]; // マネージャ側で選定されたメッセージを処理
	if ( (kamome->action.status & KMM_STATUS_DEAD)  ||
	     (kamome->action.status & KMM_STATUS_SLEEP) ||
	     (kamome->action.status & KMM_STATUS_FEINT)) {

	}else{
	    KMM_SettingMessage( kamome, kamome->nMessage, &msg->message[ 0 ]); 
	    kmmng->ucMsgCntr[ kamome->nMsgIndex ]++; // メッセージ処理カウンタを更新
	}
    }
}


// くちばし攻撃
#define FAT_MESSAGE_PICK_ATTACK (1)
static void KMM_PickAttackMessage( KAMOME* kamome)
{
    { // 
	GV_MSG msg ;
	u_int buffer[  ] = { FAT_MESSAGE_PICK_ATTACK, (int)BODYWORLD( &kamome->body, 3).m[3][0], 
			     (int)BODYWORLD( &kamome->body, 3).m[3][1], (int)BODYWORLD( &kamome->body, 3).m[3][2] };
	
	msg.address = kamome->nAimName;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
}



