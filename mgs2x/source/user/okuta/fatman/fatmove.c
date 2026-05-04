/*
    ファットマン移動ルーチン
    2001/03/15 Masafumi Okuta
    $Id: fatmove.c,v 1.1.1.3 2002/11/19 11:47:59 Yoshizawa1 Exp $
*/
extern HZX_ZON *ENE_HZX_GetZone(int addr);
#if 0
// ファットマン逃げ場所検索ルーチン
static int FAT_GetEscapeZone( Work* work )
{
    int i;
    int		nRes;
    u_char 	u8Dist;
    NPCWORK*	npc;
    HZX_ZON *pzone;
    npc = &work->npc;
    pzone = ENE_HZX_GetZone( GM_PlayerControl->addr );

    // 安全地帯を探す
    u8Dist = 0;
    nRes = HZX_NO_ZONE;
    for ( i = 0; i < HZX_MAX_SAFEZONE_NUM; i++){
	if (HZX_Zone1(pzone->safes[i]) != HZX_NO_ZONE && 
	    pzone->safes[i] != HZX_Zone1(npc->ctrl->addr) &&
	    pzone->safe_dists[i] > u8Dist){ // より遠い
	    u8Dist = pzone->safe_dists[i];
	    nRes = pzone->safes[i];
	}
    }
    if ( nRes == HZX_NO_ZONE ){    // なかった
printf(":::: cannot find safe zone\n");
	nRes = GetRandZone( work );
    }

    return nRes;
}
#endif
#if 0
/*プレイヤマップを設定*/
static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	return addr;
}
// 指定されたゾーンから逃げるためのゾーンを取得
static int GetEscapeZone(Work* work,	// ファットマンワーク
			 int   trgaddr,	// 指定ゾーン(ここから遠ざかる)
			 int   route)	// 回避するゾーン数
{
    int i;
    int nRoute = 0;
    int nTmpAddr;
    int nCurrAddr;

    trgaddr = HZX_Zone1(trgaddr);

    // イレギュラー
    if ( trgaddr == HZX_Zone1(work->npc.ctrl->addr) ||
	 trgaddr == HZX_NO_ZONE ||
	 HZX_Zone1(work->npc.ctrl->addr) == HZX_NO_ZONE ||
	 GM_GetRIntrptOne( trgaddr ) != NULL ){
	return work->npc.ctrl->addr;
    }
    nCurrAddr = nTmpAddr = HZX_Zone1(work->npc.ctrl->addr);

    for ( i = 0; i < 8; i++){
	nTmpAddr = HZX_FarZoneNavigate( work->npc.ctrl->hzx_id,	HZX_Zone1(nCurrAddr),HZX_Zone1(trgaddr), &nRoute);
	if (GM_GetRIntrptOne( trgaddr ) != NULL) break;
	if (nTmpAddr == trgaddr) break;
	if (nTmpAddr == nCurrAddr) break;
	nCurrAddr = nTmpAddr;

	if(route <= ENE_GetRouteDis( NULL, NULL, work->npc.ctrl->addr, AddrSet(nTmpAddr), route) )
	    break;
    }
    ASSERT(nTmpAddr != HZX_NO_ZONE);
    return AddrSet(nTmpAddr);
	
}
#endif

#if 0
// ポイント指定位置移動
// ある程度ゾーン移動して接近したらポイント指定移動にする
// ループ
static void Think3_PointMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	GM_ReSetNavi( npc->navi);
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
        work->navitrg.pos  = work->vecNextPos;  // 目標位置セット
    }

    npc->action.pad = PAD_POINT_MOVE;	// ポイント移動
#if 0
    // 壁にぶつかった
    if ( npc->ctrl->n_touches > 0 ){
	FAT_SetThink3FromNext( work, TH3_STAND);	// 立つ
	return;
    }
#endif
    
    if ( FAT_CheckNearZoneIn( npc->ctrl->addr, work->navitrg.addr) ){
	// 距離追跡
	if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	    FAT_SetThink3FromNext( work, TH3_STAND);	// 立つ
	    return;
	}
    }else{
	// ゾーン追跡
	GM_ZoneNavi( npc->navi, &work->navitrg);
    }


    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
#endif

// ファットマンのモーション別壁めりこみ軽減
static void FAT_SetRSphereFromMot(Work*	work)
{
    NPCWORK*	npc;
    NPCACT*	act;

    // local param init
    npc		= &work->npc;
    act      	= &npc->action;

    // モーション別にめりこみ軽減値を設定
    if ( act->current_mot >= FAT_MOT_RUN_START && act->current_mot <= FAT_MOT_TURN_R){ 
	npc->ctrl->r_sphere = FAT_R_SPHERE_MOV;	// 移動時（少し大きめ）
    }else{
	npc->ctrl->r_sphere = FAT_R_SPHERE_DEF;	// デフォルト値
    }
}




//------ ベジェ補間移動

// 傾き計算用の係数を求める
static void FAT_CalcBezierIncMod( Work* work, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3)
{
    FVECTOR vecTmp1, vecTmp2, vecTmp3;

    // tの係数の方
    _sceVu0ScaleVector( &vecTmp1, pvec1, 2.f);
    _sceVu0ScaleVector( &vecTmp2, pvec2, -4.f);
    _sceVu0ScaleVector( &vecTmp3, pvec3, 2.f);
    _sceVu0AddVector( &work->vecIncMod1, &vecTmp1, &vecTmp2);
    _sceVu0AddVector( &work->vecIncMod1, &work->vecIncMod1, &vecTmp3);
    
    _sceVu0ScaleVector( &vecTmp1, pvec1, -2.f);
    _sceVu0ScaleVector( &vecTmp2, pvec2, 2.f);
    _sceVu0AddVector( &work->vecIncMod2, &vecTmp1, &vecTmp2);
}

// 傾き取得
static void FAT_BezierInc( FVECTOR* pvec, FVECTOR* pvecIncMod1, FVECTOR* pvecIncMod2, float fRate) 
{
    FVECTOR vec;

    _sceVu0ScaleVector( &vec, pvecIncMod1, fRate);
    _sceVu0AddVector( pvec, &vec, pvecIncMod2);
    _sceVu0Normalize( pvec, pvec);
}
#if 0
// 曲線の長さを取得
static float FAT_BezierDist( FMATRIX* pmat, int nDiv)
{
    int 	i;
    float 	fDist;
    FVECTOR	vecPre, vecPos, vecSub;

    MAO_Bezier3InterpQuick( &vecPre, pmat, 0.f );
    fDist = 0.f;
    for ( i = 1; i < nDiv; i++){
	MAO_Bezier3InterpQuick( &vecPos, pmat, (float)i / (float)nDiv );
	_sceVu0SubVector( &vecSub, &vecPos, &vecPre);
	fDist += DG_SQRT( vecSub.vx * vecSub.vx + vecSub.vy * vecSub.vy + vecSub.vz * vecSub.vz );
	_sceVu0CopyVector( &vecPre, &vecPos);
    }
    
    return (fDist);
}
#endif
// 補間割合を取得
static float FAT_GetBezierRate( FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3, float fSpeed )
{
    float 	fDist;
    FVECTOR 	vec;

    _sceVu0SubVector( &vec, pvec2, pvec1 );		// 差分ベクトルを求める
    fDist = DG_SQRT( vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz );	// １と２の距離を取得
    _sceVu0SubVector( &vec, pvec3, pvec2 );		// 差分ベクトルを求める
    fDist += DG_SQRT( vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz );	// ２と３の距離を取得して加算

    if ( fDist == 0.f ) return 0.f;

    return (fSpeed / fDist);
}

// 移動準備
static void FAT_SetupMove( Work* work, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3)
{
    NPCWORK*	npc;

    npc = &work->npc;

    // ベジェ補間用のマトリクスを計算しておく
    MAO_MakeBezierMatrix( &work->matBezier, pvec1, pvec2, pvec3);
    // 補間割合クリア
    work->fBezierRate = 0.f;
    // １フレームあたりの補間割合を設定
    if ( work->nRollerBreak ){
	work->fBezierOne  = FAT_GetBezierRate( pvec1, pvec2, pvec3, FAT_SPEED * FAT_BREAK_SPEED);
    }else{
	work->fBezierOne  = FAT_GetBezierRate( pvec1, pvec2, pvec3, FAT_SPEED);
    }

    // 傾きを計算
    FAT_CalcBezierIncMod( work, pvec1, pvec2, pvec3);

    _sceVu0CopyVector( &work->vecBezStart, pvec1);
    _sceVu0CopyVector( &work->vecBezCtrl, pvec2);
    _sceVu0CopyVector( &work->vecBezEnd, pvec3);

}
// 初期設定
static void FAT_InitBezierMove( Work* work)
{
    // ワーク設定
    work->nBezIndex = 0;
    // ルート選択
    work->nBezAimZone = 161;
    FAT_SelectRoute( work, work->nBezAimZone);
    FAT_SetupMove( work, &FAT_BEZ_MOVE_POS[work->nBezIndex][0], &FAT_BEZ_MOVE_POS[work->nBezIndex][1], &FAT_BEZ_MOVE_POS[work->nBezIndex][2]);

    FAT_MakeZoneDataFromRoute( work);
}

// ルート移動
static void FAT_SetupRouteMove( Work* work, int nRouteIndex, int nReverse )
{
    NPCWORK*	npc;
    HZX_ZONE_ADD trgzone;

    npc = &work->npc;

    work->body.flag |= OBJECT_MOTIONSTEP_THROUGH; // モーション移動量無視

    // ワーク設定
    work->nBezIndex = nRouteIndex; // インデックス設定
    if ( nReverse ){ // 逆
	FAT_SetupMove( work, &FAT_BEZ_MOVE_POS[work->nBezIndex][2], &FAT_BEZ_MOVE_POS[work->nBezIndex][1], 
		             &FAT_BEZ_MOVE_POS[work->nBezIndex][0]);
    }else{	     // 正
	FAT_SetupMove( work, &FAT_BEZ_MOVE_POS[work->nBezIndex][0], &FAT_BEZ_MOVE_POS[work->nBezIndex][1], 
		             &FAT_BEZ_MOVE_POS[work->nBezIndex][2]);
    }
    // ゾーンアドレスからナビターゲットの情報を生成
    GM_ReSetNavi( npc->navi); // リセット
    trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
    GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
    
    work->nMoveForm = FAT_MOVEFORM_ROUTE;

    { // モーション分岐
	int nDir;
	FVECTOR vec1, vec2;

	// 始点と終点の傾きチェック
	FAT_BezierInc( &vec1, &work->vecIncMod1, &work->vecIncMod2, 0.f );
	FAT_BezierInc( &vec2, &work->vecIncMod1, &work->vecIncMod2, 1.f );
	
	nDir = GV_DiffDirAbs( _FVecDir2( &vec1), _FVecDir2( &vec2) );
//printf("nDIR------------------%d\n", nDir);

	if ( nDir >=  512 && nDir < 1536 ){
	    work->nRunMode = FAT_RUNMODE_TURN_R;
	}else if ( nDir > 2560 && nDir <= 4096 - 512 ){
	    work->nRunMode = FAT_RUNMODE_TURN_L;
	}else if ( nDir >= 1536 && nDir <= 2560){
	    FAT_SetThink3( work, TH3_REVERSE); 		// 反転
	    return;
	}else{
	    work->nRunMode = FAT_RUNMODE_NORMAL;
	}
    }

    work->nBezInterp = FAT_BEZIER_INTERP; // 補間時間
}
// ゾーン移動
static void FAT_SetupNavigateMove( Work* work, int nZoneTo )
{
    NPCWORK*	npc;
    HZX_ZONE_ADD trgzone;

    npc = &work->npc;
    work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH; // モーション移動量有効化
    work->vecShift = DG_ZeroVector;

    // リセット
    GM_ReSetNavi( npc->navi);

    // ゾーンアドレスからナビターゲットの情報を生成
    GM_ReSetNavi( npc->navi); // リセット
    trgzone = HZX_Address( npc->ctrl->hzx_id, nZoneTo, nZoneTo );
    GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );

    if ( HZX_InsideZone( npc->ctrl->hzx_id, &work->vecNextPos, work->nNextZone) ){
		work->navitrg.pos  = work->vecNextPos;  // 目標位置セット
    }

    work->nMoveForm = FAT_MOVEFORM_ZONE;
}
// ゾーン移動からルート移動へ
static void FAT_SetupToRouteMove( Work* work, int nIndex, int nReverse )
{
    NPCWORK*	npc;
    HZX_ZONE_ADD trgzone;

    npc = &work->npc;
    work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH; // モーション移動量有効化

    // ゾーンアドレスからナビターゲットの情報を生成
    if ( nReverse ){
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nRouteZone[nIndex][0], work->nRouteZone[nIndex][0] );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	work->navitrg.pos  = FAT_BEZ_MOVE_POS[nIndex][0];    // ポイント指定
    }else{
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nRouteZone[nIndex][1], work->nRouteZone[nIndex][1] );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	work->navitrg.pos  = FAT_BEZ_MOVE_POS[nIndex][2];    // ポイント指定
    }

    work->nMoveForm = FAT_MOVEFORM_TO_ROUTE;
}

#if 0
// 移動
static void FAT_Move( Work* work )
{
    FVECTOR vecPos, vecSub;

    // ベジェ補間
    MAO_Bezier3InterpQuick( &vecPos, &work->matBezier, work->fBezierRate );

    work->body.flag |= OBJECT_MOTIONSTEP_THROUGH; // モーション移動量無視
    _sceVu0SubVector( &work->control.step, &vecPos, &work->control.mov);
    work->control.step.vy = 0.f;

    // 傾きから方向取得
    FAT_BezierInc( &vecSub, &work->vecIncMod1, &work->vecIncMod2, work->fBezierRate );
    work->npc.action.dir = _FVecDir2( &vecSub);
    
    work->npc.action.pad = PAD_RUN_ACT;	// 走行開始

    work->fBezierRate += work->fBezierOne;	// 補間割合を足す 
    if ( work->fBezierRate >= 1.f){
		work->fBezierRate = 0.f;
		if ( FAT_SelectRoute( work, work->nBezAimZone) ){	// ルート選択
			// ワーク設定
			FAT_SetupMove( work, &FAT_BEZ_MOVE_POS[work->nBezIndex][0], &FAT_BEZ_MOVE_POS[work->nBezIndex][1], 
						   &FAT_BEZ_MOVE_POS[work->nBezIndex][2]);
		}else if ( FAT_SelectRouteRev( work, work->nBezAimZone) ){	// 逆ルート選択
			// ワーク設定
			FAT_SetupMove( work, &FAT_BEZ_MOVE_POS[work->nBezIndex][2], &FAT_BEZ_MOVE_POS[work->nBezIndex][1], 
						   &FAT_BEZ_MOVE_POS[work->nBezIndex][0]);
		}
    }
    FAT_ChgAimZone( work);
}
#endif

// ルートデータからゾーンを取得
static void FAT_MakeZoneDataFromRoute( Work* work)
{
    int i;

    for ( i = 0; i < FAT_ROUTEDATA_NUM; i++){
		work->nRouteZone[i][0] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_BEZ_MOVE_POS[i][0], -1) ); // 始点
		work->nRouteZone[i][1] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_BEZ_MOVE_POS[i][2], -1) ); // 終点
    }
}

// ルート選択
static int FAT_SelectRoute( Work* work, int nAimZone)
{
    int 	i,zone, nFatZone;
    NPCWORK*	npc;
    HZX_ZONE_ADD trgzone;

    npc = &work->npc;

    trgzone = HZX_Address( npc->ctrl->hzx_id, nAimZone, nAimZone);
    nFatZone = HZX_Zone1( npc->ctrl->addr); // ファットマンのゾーン取得

    GM_ReSetNavi( npc->navi); // リセット
    GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );

    for ( i = 0; i < FAT_ROUTEDATA_NUM; i++){
	HZX_ZONE_ADD hzaAddr;

	if ( nFatZone != work->nRouteZone[i][0] )
	    continue;

	// 目的地が終点と同じゾーン
	if ( nAimZone == work->nRouteZone[i][1]){
	    work->nBezIndex = i;	// インデックス更新
            return 1;
	}

	// 今いるゾーンから目標ゾーンまでに終点があるかチェック
	zone = nFatZone;
	while ( zone != HZX_NO_ZONE ){
	    hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, nAimZone);
	    zone = HZX_Zone1(hzaAddr);

	    if ( zone == work->nRouteZone[i][1]){
		work->nBezIndex = i;	// インデックス更新
		return 1;
	    }
	    if ( zone == nAimZone ){
		break;
	    }
	}

	// 始点から終点の間に目標地点があるか
	zone = work->nRouteZone[i][0];
	while ( zone != HZX_NO_ZONE ){
	    hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, work->nRouteZone[i][1]);
	    zone = HZX_Zone1(hzaAddr);

	    if ( zone == nAimZone){
		work->nBezIndex = i;	// インデックス更新
		return 1;
	    }
	    if ( zone == work->nRouteZone[i][1] ){
		break;
	    }
	}
    }
    return 0;
}
#if 0
// 逆ルート選択
static int FAT_SelectRouteRev( Work* work, int nAimZone)
{
    int 	i,zone, nFatZone;
    NPCWORK*	npc;
    HZX_ZONE_ADD trgzone;

    npc = &work->npc;

    trgzone = HZX_Address( npc->ctrl->hzx_id, nAimZone, nAimZone);
    nFatZone = HZX_Zone1( npc->ctrl->addr); // ファットマンのゾーン取得

    GM_ReSetNavi( npc->navi); // リセット
    GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );

    for ( i = 0; i < FAT_ROUTEDATA_NUM; i++){
	HZX_ZONE_ADD hzaAddr;

	if ( nFatZone != work->nRouteZone[i][1] )
	    continue;

	zone = nFatZone;

	if ( nAimZone == work->nRouteZone[i][0]){
	    work->nBezIndex = i;	// インデックス更新
            return 1;
	}
	while ( zone != HZX_NO_ZONE ){
	    hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, nAimZone);
	    zone = HZX_Zone1(hzaAddr);
	    if ( zone == work->nRouteZone[i][0]){
		work->nBezIndex = i;	// インデックス更新
		return 1;
	    }
	    if ( zone == nAimZone ){
		break;
	    }
	}
	// 始点から終点の間に目標地点があるか
	zone = work->nRouteZone[i][1];
	while ( zone != HZX_NO_ZONE ){
	    hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, work->nRouteZone[i][0]);
	    zone = HZX_Zone1(hzaAddr);

	    if ( zone == nAimZone){
		work->nBezIndex = i;	// インデックス更新
		return 1;
	    }
	    if ( zone == work->nRouteZone[i][0] ){
		break;
	    }
	}
    }
    return 0;
}
#endif
// 指定の正ルートデータが目標に向かうことのできるルートかどうかチェックする
// １でできる ０でできない
static int FAT_ChkRegularRoute( Work* work, int nIndex, int nFromZone, int nToZone )
{
    int 	zone;
    NPCWORK*	npc;
    HZX_ZONE_ADD hzaAddr;

    npc = &work->npc;

    // 目標ゾーンとルートの終端のゾーンが一致
    if ( nToZone == work->nRouteZone[nIndex][1]){
	return 1;
    }

    // 目標ゾーンまでの道順の中にルートの終点が存在するかチェック
    zone = nFromZone;
    while ( zone != HZX_NO_ZONE ){
	hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, nToZone);
	zone = HZX_Zone1(hzaAddr);
	if ( zone == work->nRouteZone[nIndex][1]){ // 発見
	    return 1;
	}
	if ( zone == nToZone ){ // 見つからなかった
	    break;
	}
    }
	
    // ゾーンとルートが接触するかチェック
    if ( FAT_CheckBezierInTetra( work, nToZone, nIndex) ){
	return 1;
    }

    return 0;
}
// 指定の逆ルートデータが目標に向かうことのできるルートかどうかチェックする
// １でできる ０でできない
static int FAT_ChkCounterRoute( Work* work, int nIndex, int nFromZone, int nToZone )
{
    int 	zone;
    NPCWORK*	npc;
    HZX_ZONE_ADD hzaAddr;

    npc = &work->npc;

    // 目標ゾーンとルートの終端のゾーンが一致
    if ( nToZone == work->nRouteZone[nIndex][0]){
	return 1;
    }

    // 目標ゾーンまでの道順の中にルートの終点が存在するかチェック
    zone = nFromZone;
    while ( zone != HZX_NO_ZONE ){
	hzaAddr = HZX_NextZone( npc->ctrl->hzx_id, zone, nToZone);
	zone = HZX_Zone1(hzaAddr);
	if ( zone == work->nRouteZone[nIndex][0]){ // 発見
	    return 1;
	}
	if ( zone == nToZone ){ // 見つからなかった
	    break;
	}
    }
	
    // ゾーンとルートが接触するかチェック
    if ( FAT_CheckBezierInTetra( work, nToZone, nIndex) ){
	return 1;
    }

    return 0;
}

// 自分が次に向かうゾーンの中で最終目標に向かうことのできるルートが存在するかしらべる
static int FAT_CheckRouteInNextZone( Work* work, int* pnIndex, int* pnReverse )
{
    int 	j, nIndex, nReverse, nRev, nDist, nDistTmp, nFromZone;
    NPCWORK*	npc;

    npc = &work->npc;

    nIndex = -1;
    nReverse = 0;
    nRev = 0;
    nDist = HZX_ZoneDistance( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), work->nNextZone); // 目標までのゾーン距離
    nFromZone = HZX_Zone1(work->navigate.next_addr); // 次に向かうゾーン
	
    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++ ){
	// 指定ゾーンが始点になるルートかどうかチェック
	if ( work->nRouteZone[j][0] == nFromZone ){
	    nRev = 0; // 正ルート
	}else if (work->nRouteZone[j][1] == nFromZone ){
	    nRev = 1; // 逆ルート
	}else{
	    continue;
	}

	// ゾーンとルートが接触するかチェック
	if ( FAT_CheckBezierInTetra( work, work->nNextZone, j) ){
	    nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
	    if ( nDistTmp < nDist){
		nIndex = j;
		nReverse = nRev;
		break;
	    }
	}

	if ( nRev ){
	    // 逆ルート
	    if ( FAT_ChkCounterRoute( work, j, nFromZone, work->nNextZone ) ){
		nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
		if (nDistTmp < nDist){
		    nDist = nDistTmp;
		    nIndex = j;
		    nReverse = 1;
		}
	    }
	}else{
	    // 正ルート
	    if ( FAT_ChkRegularRoute( work, j, nFromZone, work->nNextZone ) ){
		nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][1], work->nNextZone);
		if (nDistTmp < nDist){
		    nDist = nDistTmp;
		    nIndex = j;
		    nReverse = 0; 
		}
	    }
	}
    }
	
    if ( nIndex >= 0 ){ // 有効なルートが見つかった
//printf("----- %d %d +++++++++++++++++++++++++++++++++++++++++++++\n", nIndex, nReverse);
	(*pnIndex) = nIndex;
	(*pnReverse) = nReverse;
	return 1;
    }

    return 0;
}

// ゾーン更新時のチェック : ナビゲート移動時のゾーン切替えが行なわれる時, ルート終端時
static void FAT_CheckRouteZone( Work* work, int nZoneFrom, int nZoneTo)
{
    int 	j, nIndex, nReverse, nRev, nDist, nDistTmp;
    NPCWORK*	npc;

    npc = &work->npc;
    
    nDist = HZX_ZoneDistance( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), work->nNextZone); // 目標までのゾーン距離
    nIndex = -1;
    nReverse = 0;
    nRev = 0;

    if ( nZoneFrom == nZoneTo ){ // 目標が一致
	// ゾーン移動
	FAT_SetupNavigateMove( work, nZoneTo);
    }else{
	for ( j = 0; j < FAT_ROUTEDATA_NUM; j++ ){
	    // ファットマンのいるゾーンが始点になるルートかどうかチェック
	    if ( work->nRouteZone[j][0] == HZX_Zone1( npc->ctrl->addr) && 
		 _MAO_FVec2Len2( &npc->ctrl->mov, &FAT_BEZ_MOVE_POS[j][0]) < 2400.f ){
		nRev = 0; // 正ルート
	    }else if (work->nRouteZone[j][1] == HZX_Zone1( npc->ctrl->addr) && 
		 _MAO_FVec2Len2( &npc->ctrl->mov, &FAT_BEZ_MOVE_POS[j][2]) < 2400.f ){
		nRev = 1; // 逆ルート
	    }else{
		continue;
	    }

	    // ゾーンとルートが接触するかチェック
	    if ( FAT_CheckBezierInTetra( work, work->nNextZone, j) ){
		nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
		if ( nDistTmp < nDist){
		    nIndex = j;
		    nReverse = nRev;
		    break;
		}
	    }

	    if ( nRev ){
		// 逆ルート
		if ( FAT_ChkCounterRoute( work, j, HZX_Zone1( npc->ctrl->addr), work->nNextZone ) ){
		    nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
		    if (nDistTmp < nDist){
			nDist = nDistTmp;
			nIndex = j;
			nReverse = 1; 
		    }
		}
	    }else{
		// 正ルート
		if ( FAT_ChkRegularRoute( work, j, HZX_Zone1( npc->ctrl->addr), work->nNextZone ) ){
		    nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][1], work->nNextZone);
		    if (nDistTmp < nDist){
			nDist = nDistTmp;
			nIndex = j;
			nReverse = 0; 
		    }
		}
	    }
	}
	
	if ( nIndex >= 0 ){ // 有効なルートが見つかった
	    FAT_SetupRouteMove( work, nIndex, nReverse); // ルート設定
	}else{
	    // ゾーン移動
	    FAT_SetupNavigateMove( work, nZoneTo);
	}
    }
}
// ゾーン移動	
static int FAT_ZoneMove( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

#ifdef SPEED_CHG_1002ADD // ８の字対策速度変更ルーチン
    { // 速度変更
	float	fDist;
	FVECTOR vecDist;
	_sceVu0SubVector( &vecDist, &work->navitrg.pos, &npc->ctrl->mov ); 
	fDist = DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );
	if ( fDist < 2000.f )	work->fSpeed -= 0.05f;
	else			work->fSpeed += 0.05f;	

	if ( work->fSpeed < 0.75f ) 	work->fSpeed = 0.75f;
	else if ( work->fSpeed > 1.f )	work->fSpeed = 1.f;
	_sceVu0ScaleVector( &npc->ctrl->step, &npc->ctrl->step, work->fSpeed);
    }
#endif
    // nvtrgに向ってゾーン移動 
    if ( GM_NaviNear( npc->navi, &work->navitrg, FAT_NAVI_DIST ) ) {	
	return 1;
    }
    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    return 0;
}

// ルート移動
static int FAT_RouteMove( Work* work )
{
    float	fPrevRate;
    FVECTOR 	vec, vecPos, vecSub, vecPre, vecPreSub;
    NPCWORK*	npc;
    npc = &work->npc;

    fPrevRate = work->fBezierRate;
    work->fBezierRate += work->fBezierOne;	// 補間割合を足す 
    if ( work->fBezierRate > 1.f )	work->fBezierRate = 1.f;  // リミットチェック

    // エルミート補間
    {
		float fDist;
		FVECTOR vecStartZ, vecEndZ, vecDist;
		FMATRIX mat;
	
		DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot);
		DG_GetPos( &mat);

		// 速度取得
		_sceVu0SubVector( &vecDist, &work->vecBezCtrl, &work->vecBezStart );	// 差分ベクトルを求める
		fDist = DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );	// １と２の距離を取得
		_sceVu0SubVector( &vecDist, &work->vecBezEnd, &work->vecBezCtrl );	// 差分ベクトルを求める
		fDist += DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );// ２と３の距離を取得して加算
		// 始点の速度ベクトル設定
		FAT_BezierInc( &vecStartZ, &work->vecIncMod1, &work->vecIncMod2, 0.f ); // ベジェ曲線の微分から方向を取得
		_sceVu0ScaleVector( &vecStartZ, &vecStartZ, fDist);
		// 終点の速度ベクトル設定
		FAT_BezierInc( &vecEndZ, &work->vecIncMod1, &work->vecIncMod2, 1.f );	// ベジェ曲線の微分から方向を取得
		_sceVu0ScaleVector( &vecEndZ, &vecEndZ, fDist);
		// エルミート補間
		MAO_HermiteLerpVec( &vecPos, &vecSub, &work->vecBezStart, &work->vecBezEnd, 
							&vecStartZ, &vecEndZ, work->fBezierRate);
		MAO_HermiteLerpVec( &vecPre, &vecPreSub, &work->vecBezStart, &work->vecBezEnd, 
							&vecStartZ, &vecEndZ, fPrevRate);
		// 移動設定
		work->body.flag |= OBJECT_MOTIONSTEP_THROUGH; // モーション移動量無視

		{ // 横だけ反映
			FMATRIX mat;

			DG_SetPos2( &DG_ZeroVector, &work->npc.ctrl->rot);
			DG_GetPos( &mat );

			work->vecShift.vx += work->body.m_ctrl->mt3_ctrl[0].move->step.vx;
			_sceVu0ApplyMatrix( &vec, &mat, &work->vecShift );
			vecPos.vx += vec.vx;
		}

		if ( work->nBezInterp > 0 ){ // 補間
			FVECTOR vecDistance, vecBase, vecAim;

			_sceVu0SubVector( &vecDistance, &vecPos, &vecPre);
			_sceVu0AddVector( &vecBase, &work->control.mov, &vecDistance);
			_sceVu0CopyVector( &vecBase, &work->control.mov);
			_sceVu0CopyVector( &vecAim, &vecPos);
			MAO_InterpVec( &vecPos, &vecBase, &vecAim, 1.f - ( (float)work->nBezInterp / (float)FAT_BEZIER_INTERP) );
			work->nBezInterp--;
		}

		// 移動量（ベース）
		_sceVu0SubVector( &work->control.step, &vecPos, &work->control.mov);
		work->control.step.vy = 0.f;
// MAO_DbgDumpVector( &work->vecShift );
// MAO_DbgDumpVector( &vec );
		// 方向取得
		_sceVu0Normalize( &vecSub, &vecSub);
		work->npc.action.dir = _FVecDir2( &vecSub);
    }

    // 終点到達
    if ( work->fBezierRate >= 1.f){
	// 目標到達チェック
	if ( HZX_Zone1(npc->ctrl->addr) ==  work->nNextZone ){ 
	    return 1;
	}
	work->fBezierRate = 0.f;
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);
    }

    return 0;
}

// ゾーンとルートが接触するかチェック
static int FAT_CheckBezierInTetra( Work* work, int nZone, int nRoute )
{

    if ( ((work->pbBezierPass[ nRoute * 32 + (int)(nZone / 8) ] >> (nZone % 8)) & 0x01) == 0x01 ){
		return 1;
    }

    return 0;
}

// ファットマンのルート＆ゾーン共存移動
static int FAT_RouteZoneMove( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;
//printf("Move-Form == %d :: %d %d\n", work->nMoveForm, work->think2, work->think3 );
    switch ( work->nMoveForm ){
    case FAT_MOVEFORM_ZONE:	// ゾーン
	work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH; // モーション移動量を使用
	if ( FAT_ZoneMove( work ) ){ // ゾーン移動
	    // 到着した
	    if ( HZX_Zone1(npc->ctrl->addr) == work->nNextZone){ // 最終目的地到着
		if ( work->nNextZone2Valid && FAT_CheckZone(work, work->nNextZone2) >= 0 ){ // 次の指定があるなら
		    if ( FAT_CheckReverse( work, work->nNextZone2) ){ // 反転チェック
			FAT_SetNextPointWithNavigate( work, work->nNextZone2, &work->vecNextPos2);
			if ( work->bNext3Valid ){
			    FAT_SetNextThink3( work, work->nNextThink3);    // 次候補をキープ
			}else{
			    FAT_SetNextThink3( work, TH3_MOVE); 	    // 移動
			}
			FAT_SetThink3( work, TH3_REVERSE); 		// 反転
			work->nNextZone2Valid = 0;
			return 1;
		    }else{
			FAT_SetNextPointWithNavigate( work, work->nNextZone2, &work->vecNextPos2);
			if ( work->bNext3Valid ){ // 次の思考がある時だけ
			    FAT_SetThink3FromNext( work, TH3_MOVE);
			}
			work->nNextZone2Valid = 0;
			return 1;
		    }
		}else{
		    // 思考を入れる
		    FAT_SetThink3( work, TH3_STOP); // 停止
		    return 1;
		}
	    }else{ // 中継点到着
		FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone); // ルートチェック
	    }
	    return 1;
	}else{
//	    if (  npc->ctrl->addr != work->nPrevZone ){  // ゾーンが変わった
		int nIndex, nReverse;
		// 目標に隣接した
		if ( FAT_CheckNearZoneIn( HZX_Zone1(npc->ctrl->addr), work->nNextZone) ) {
		    // ゾーンアドレスからナビターゲットの情報を生成
		    FAT_SetupNavigateMove( work, work->nNextZone);
		}else{
		    if ( FAT_CheckRouteInNextZone( work, &nIndex, &nReverse) ){ // 隣接区域にルートがある
			FAT_SetupToRouteMove( work, nIndex, nReverse); // ルート始点へ移動
		    }
		}
//	    }
	}
	break;
    case FAT_MOVEFORM_ROUTE:	// ルート移動
	work->body.flag |= OBJECT_MOTIONSTEP_THROUGH; // モーションに依る移動をスキップ
	if ( FAT_RouteMove( work ) ){ // ルート移動
	    // ゾーンアドレスからナビターゲットの情報を生成
	    if ( HZX_Zone1( npc->ctrl->addr) == work->nNextZone ){ // 最終目的地に到着
		if ( work->nNextZone2Valid && FAT_CheckZone(work, work->nNextZone2) >= 0 ){ // 次の指定があるなら
		    if ( FAT_CheckReverse( work, work->nNextZone2) ){ // 反転チェック
			FAT_SetNextPointWithNavigate( work, work->nNextZone2, &work->vecNextPos2);
			if ( work->bNext3Valid ){
			    FAT_SetNextThink3( work, work->nNextThink3);    // 次候補をキープ
			}else{
			    FAT_SetNextThink3( work, TH3_MOVE); 	    // 移動
			}
			FAT_SetThink3( work, TH3_REVERSE); 	   // 反転
			work->nNextZone2Valid = 0;
			return 1;
		    }else{
			FAT_SetNextPointWithNavigate( work, work->nNextZone2, &work->vecNextPos2);
			work->nNextZone2Valid = 0;
			if ( work->bNext3Valid ){ // 次の思考がある時だけ
			    FAT_SetThink3FromNext( work, TH3_MOVE);
			}else{
			    FAT_SetThink3( work, TH3_STOP); 	// 停止
			}
			work->nNextZone2Valid = 0;
			return 1;
		    }
		}else{
		    FAT_SetThink3FromNext( work, TH3_STOP); 	// 停止
		    return 1;
		}
	    }else{
		FAT_CheckRouteZone( work, HZX_Zone1( npc->ctrl->addr), work->nNextZone);
	    }
	    return 1;
	}else{
	    // 隣接した
	    if ( HZX_Zone1(npc->ctrl->addr) == work->nNextZone ){
		FAT_SetupNavigateMove( work, work->nNextZone);
	    }
	}
	break;
    case FAT_MOVEFORM_TO_ROUTE:	// ゾーン->ルート
	work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;

	if ( FAT_ZoneMove( work ) ){
	    FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);
	}
	// 目標に隣接した
	if ( FAT_CheckNearZoneIn( HZX_Zone1(npc->ctrl->addr), work->nNextZone) ){
	    // ゾーンアドレスからナビターゲットの情報を生成
	    FAT_SetupNavigateMove( work, work->nNextZone);
	}
	break;
    }

    return 0;
}
