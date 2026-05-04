/*
    kmmove.c
    カモメ移動処理
    2001/05/02 Masafumi Okuta
    $Id: kmmove.c,v 1.1.1.3 2002/11/19 11:48:04 Yoshizawa1 Exp $
*/
#define KMM_SCR_RATE (1.25f) // 速度ベクトル水増し
const int KMM_DIST_CHECK2 = (600 * 600);

#define KMM_CHK_HZX  	(HZX_CHK_RECOIL_TYPE | HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR) 		// チェックする属性
#define KMM_SKIP_FLOOR  (HZX_FLOOR_PITFALL | HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE)	// スキップする属性

// エルミート補間移動
static int KMM_UpdateHermiteWork( KAMOME* kamome )
{
    int     nRes = 0;
    int	    nDist;
    float   fScale;
    FVECTOR vec1, vec2, vecNext, vecNext2;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    vecNext  = kamome->vecRoute[kamome->nRouteIndex];
    vecNext2 = kamome->vecRoute[(kamome->nRouteIndex+1)%KMM_ROUTE_MAX];

    // 始点と終点を設定
    _sceVu0CopyVector( &kamome->vecStartPos, &kamome->mov);
    _sceVu0CopyVector( &kamome->vecAimPos, &vecNext);

    // 始点->終点, 終点->次の終点のそれぞれの差分ベクトルを生成
    _sceVu0SubVector( &vec1, &vecNext, &kamome->mov);
    nDist = _FVecLen3( &vec1);
    _sceVu0SubVector( &vec2, &vecNext2, &vecNext);

    // 始点と終点の速度ベクトル生成
    fScale = kamome->fSpeed * kmmng->fBaseSpeed;
    _sceVu0CopyVector( &kamome->vecStartZ, &kamome->vecAimZ);
    _sceVu0AddVector( &kamome->vecAimZ, &kamome->mov, &vecNext2);
    _sceVu0Normalize( &kamome->vecAimZ, &kamome->vecAimZ);
    _sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecAimZ, fScale);

    if (nDist < KMM_DIV_DIST){ // 既に着地している
	KMM_SetThink3( kamome, TH3_FLYING);
	return 1;
    }

    kamome->nRouteCntr = 0;
    kamome->nRouteTime = (int)(nDist / KMM_DIV_DIST);

    return (nRes);
}


// 移動ワーク初期化
static void KMM_InitMoveWork( KAMOME* kamome )
{
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    kamome->nRouteIndex = 0;
    kamome->nRouteCntr  = 0;
    kamome->nRouteTime  = 0;

    kamome->nRestTime   = -1; // 無効にしておく

    {
	KMM_MakeFlyingArea( kamome, &kamome->vecMoveArea1, &kamome->vecMoveArea2);
#if 0
	for ( i = 0; i < KMM_ROUTE_MAX; i++){
	    kamome->vecRoute[i].vx = KMM_GetDemoRandom( kmmng, (int)kamome->vecMoveArea1.vx, (int)kamome->vecMoveArea2.vx);
	    kamome->vecRoute[i].vy = KMM_GetDemoRandom( kmmng, (int)kamome->vecMoveArea1.vy, (int)kamome->vecMoveArea2.vy);
	    kamome->vecRoute[i].vz = KMM_GetDemoRandom( kmmng, (int)kamome->vecMoveArea1.vz, (int)kamome->vecMoveArea2.vz);
	    kamome->vecRoute[i].vw = 1.f;
	}	
#endif
    }

#if 1
    {
	int 	nDist;
	float 	fScale;
	FVECTOR vec1, vec2;
	
	// 始点と終点を設定
	_sceVu0CopyVector( &kamome->vecStartPos, &kamome->mov);
	_sceVu0CopyVector( &kamome->vecAimPos, &kamome->vecRoute[0]);

	// 始点->終点, 終点->次の終点のそれぞれの差分ベクトルを生成
	_sceVu0SubVector( &vec1, &kamome->vecRoute[0], &kamome->mov);
	nDist = _FVecLen3( &vec1);
	_sceVu0SubVector( &vec2, &kamome->vecRoute[1], &kamome->vecRoute[0]);

	// 始点と終点の速度ベクトル生成
	fScale = kamome->fSpeed * kmmng->fBaseSpeed;
	_sceVu0ScaleVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0], fScale);

	kamome->vecStartZ = kamome->vecAimZ;
	_sceVu0AddVector( &kamome->vecAimZ, &kamome->mov, &kamome->vecRoute[1]);
	_sceVu0Normalize( &kamome->vecAimZ, &kamome->vecAimZ);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecAimZ, fScale);

	if (nDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
	kamome->nRouteCntr = 0;
	kamome->nRouteTime = (int)(nDist / KMM_DIV_DIST);
    }
#else
    KMM_UpdateHermiteWork( kamome);
#endif
    kamome->nRouteTime 	  = 0;
    kamome->nLandDist     = 0;
    kamome->vecMove	  = kamome->mov;
    kamome->vecLand 	  = kamome->mov;
    kamome->vecLandZ.vx   = 0.f;
    kamome->vecLandZ.vy   = 0.f;
    kamome->vecLandZ.vz   = 1.f;
    kamome->vecLandZ.vw   = 0.f;
}
#if 0
// ルート間をエルミート補間移動
static int UpdateInterpWork( KAMOME* kamome)
{
    int nRes;
    FVECTOR vecZ;


    nRes = 0;

    // ルート更新
    kamome->nRouteCntr++;
    if ( kamome->nRouteCntr >= kamome->nRouteTime){
	kamome->nRouteCntr = 0;

	// 位置更新
	kamome->vecMove = kamome->vecAimPos;
	vecZ		= kamome->vecAimZ;

	// インデックス更新
	kamome->nRouteIndex++;

	if ( kamome->nRouteIndex >= KMM_ROUTE_MAX){
	    kamome->nRouteIndex = 0;
	    nRes = 1; // ルート終端合図
	}

	// 補間ワーク更新
	KMM_UpdateHermiteWork( kamome );
    }else{
	// 補間
	{
	    ASSERT( kamome->nRouteTime != 0 );    
	    MAO_HermiteLerpVecScale( &kamome->vecMove, &vecZ, 
				     &kamome->vecStartPos, &kamome->vecAimPos,
				     &kamome->vecStartZ,  &kamome->vecAimZ, 
				     ((float)kamome->nRouteCntr / (float)kamome->nRouteTime), 
				     (float)kamome->nRouteTime);
	}
    }

    // 方向更新
    _sceVu0Normalize( &vecZ, &vecZ);
    TS_VecToRot( &kamome->turn, &vecZ );

    return (nRes);
}
#endif
// 滑空移動
#define HOVER_SLIDE_Z 	   (COUNT_VMODE(360))
#define HOVER_SLIDE_Z_HALF (COUNT_VMODE(180))
#define HOVER_ROT_Z 	   (COUNT_VMODE(360))
#define HOVER_ROT_Z_HALF (COUNT_VMODE(180))
static int UpdateHoverWork( KAMOME* kamome)
{
    SVECTOR vecRot;
    FVECTOR vec;
    KMMTHINK* think;

    think = &kamome->think;

    vec.vx = _TS_Sin( ((think->nCount3 * 8) % 4096) ) * 10.f;
    vec.vy = _TS_Sin( ((think->nCount3 * 8 + 1024) % 4096) ) * 2.f;
    vec.vz = (vec.vx) * 0.5f;
    vec.vw = 1.f;

    kamome->turn.vx = 0;
    kamome->turn.vz = (int)(-vec.vx * 16.f); // 傾き

    vecRot.vx = kamome->rot.vx;
    vecRot.vy = kamome->rot.vy;
    vecRot.vz = 0;

    MAO_AbsPos( &kamome->vecMove, &vec, &kamome->matOrg);
    
    return 0;
}

// 着地点に向かう
// 最終目標点から一定距離内に入ると1が返る
static int KMM_LandMove( KAMOME* kamome)
{
    FVECTOR vecZ;

    // 補間
    MAO_HermiteLerpVec( &kamome->vecMove, &vecZ, 
			&kamome->vecStartPos, &kamome->vecAimPos,
			&kamome->vecStartZ,  &kamome->vecAimZ, 
			((float)kamome->nRouteCntr / (float)kamome->nRouteTime));

    if ( kamome->nRouteCntr++ >= kamome->nRouteTime){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov;
	kamome->vecStartZ = kamome->vecAimZ;	
	return 1;
    }

    // 方向取得
    _sceVu0Normalize( &vecZ, &vecZ);
    TS_VecToRot( &kamome->turn, &vecZ );

    return 0;
}

// 着地
static int KMM_LandingMove( KAMOME* kamome )
{
    float	   fPlayRate;

    // モーション再生割合
    fPlayRate = KMM_GetMotPlayRate( kamome );

    if ( fPlayRate >= 0.4f){ // 強制着地
	MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &kamome->vecLand, 0.1f);
    }else{
	float	fRate;
	FVECTOR vecWind;
	static FVECTOR vecY = { 0.f, 2.f, 0.f, 1.f };

	fRate = (1.f - fPlayRate * fPlayRate) * 0.3f;
	fRate = (float)G_wind_intense * fRate;
	if (fRate > 2.f) fRate = 2.f;

	_sceVu0Normalize( &vecWind, &kamome->vecAimZ);
	_sceVu0ScaleVector( &vecWind, &vecWind, -fRate);
	_sceVu0AddVector( &vecWind, &vecWind, &vecY);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecWind);

	MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &kamome->vecLand, 0.05f * fPlayRate * fPlayRate);
    }

    return 0;
}

// ばたつき移動
static int KMM_FlipflopMove( KAMOME* kamome )
{
    float	   fPlayRate;
    FVECTOR	   vecZ;
    KMMACT* act;

    act = &kamome->action;

    // モーション再生割合
    fPlayRate = KMM_GetMotPlayRate( kamome );

    if ( act->current_mot == KMM_MOT_HOVER ){ // 滑空
	float	fRate;
	FVECTOR vecWind, vecCounter;
	static FVECTOR vecY = { 0.f, 2.f, 0.f, 1.f };

	fRate = (1.f - fPlayRate * fPlayRate) * 0.3f;
	fRate = (float)G_wind_intense * fRate;
	if (fRate > 8.f) fRate = 8.f;

	_sceVu0Normalize( &vecWind, (FVECTOR *)&G_wind_matrix.m[2][0]);
	_sceVu0ScaleVector( &vecWind, &vecWind, fRate);
	// 風に影響
	_sceVu0AddVector( &vecWind, &vecWind, &vecY);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecWind);
	// 逆方向に流す
	_sceVu0ScaleVector( &vecCounter, (FVECTOR *)&kamome->matOrg.m[2][0], -16.f);
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vecCounter);
	// 横方向に流す
	kamome->vecAppend.vx = _TS_Sin( (kamome->think.nCount3 * 8) % 4096);
	_sceVu0ScaleVector( &vecCounter, (FVECTOR *)&kamome->matOrg.m[0][0], kamome->vecAppend.vx * 0.5f);
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vecCounter);
	MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &kamome->vecLand, 0.05f);

	// 方向取得
	_sceVu0Normalize( &vecZ, &kamome->vecAimZ);
	TS_VecToRot( &kamome->turn, &vecZ );
	kamome->turn.vx -= 256;
	kamome->turn.vz =  (short)(kamome->vecAppend.vx * 10.f);
    }else{
	float	fRate;
	FVECTOR vecWind;
	static FVECTOR vecY = { 0.f, 1.5f, 0.f, 1.f };

	fRate = (1.f - fPlayRate * fPlayRate) * 0.3f;
	fRate = (float)G_wind_intense * fRate;
	if (fRate > 6.f) fRate = 6.f;

	_sceVu0Normalize( &vecWind, (FVECTOR *)&G_wind_matrix.m[2][0]);
	_sceVu0ScaleVector( &vecWind, &vecWind, fRate);
	// 移動量を設定->座標更新
	_sceVu0AddVector( &vecWind, &vecWind, &vecY);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecWind);

	MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &kamome->vecLand, 0.08f * fPlayRate * fPlayRate);
	
	// 方向取得
	_sceVu0Normalize( &vecZ, &kamome->vecAimZ);
	TS_VecToRot( &kamome->turn, &vecZ );
	kamome->turn.vx -= 256;
    }


    return 0;
}

// 攻撃移動
static int KMM_AttackMove( KAMOME* kamome )
{
    int		nDist;
    FVECTOR 	vecZ, vecAim, vecPos, vecStep, vecFrom;
    CONTROL*	pctrl;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    pctrl = kamome->pctrlAim; // NULLチェックは入る前にしている

    _sceVu0CopyVector( &vecFrom, &kamome->mov);

    // 相対取得
    MAO_GetDiffVec3( &vecAim, &nDist, NULL, &kamome->mov, &pctrl->mov );

    MAO_InterpVec( &vecZ, (FVECTOR*)&kamome->matOrg.m[2][0], &vecAim, 0.1f);

    // ハザードチェック
    _sceVu0ScaleVector( &vecStep, &vecZ, kamome->fSpeed * kmmng->fBaseSpeed + 100.f);
    _sceVu0AddVector( &vecPos, &vecStep, &kamome->mov);
    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &vecFrom, &vecPos, KMM_CHK_HZX, 0, KMM_SKIP_FLOOR ) != 0 ){ // 障害物あり
	vecStep = DG_ZeroVector;    // 移動量０
	TS_VecToRot( &kamome->turn, &vecAim );	// 向き
	kamome->turn.vx -= 512;
	return -1;
    }else{
	_sceVu0ScaleVector( &vecStep, &vecZ, kamome->fSpeed * kmmng->fBaseSpeed + 10.f); // 移動量取得
	TS_VecToRot( &kamome->turn, &vecAim );  // 向き
    }
    _sceVu0AddVector( &kamome->vecMove, &vecStep, &kamome->mov); // 移動

    if (nDist < 2000){
	kamome->vecLand = pctrl->mov;
	return 1;
    }

    return 0;
}
// 攻撃
static int KMM_Attack( KAMOME* kamome )
{
    int		nDist;
    FVECTOR 	vec, vecZ, vecAim;
    KMMACT* 		act;
    CONTROL*	pctrl;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    act = &kamome->action;
    pctrl = kamome->pctrlAim; // NULLチェックは入る前にしている

    // 相対取得
    MAO_GetDiffVec3( &vecAim, &nDist, NULL, &kamome->mov, &pctrl->mov);
    
    if (nDist >= 400){
	FVECTOR vecStep;

	MAO_InterpVec( &vecZ, (FVECTOR*)&kamome->matOrg.m[2][0], &vecAim, 0.1f);

	_sceVu0ScaleVector( &vecStep, &vecZ, 100.f);
	_sceVu0AddVector( &vec, &vecStep, &kamome->mov);

	// 進行方向にハザードあり
	if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &vec, KMM_CHK_HZX, 0, KMM_SKIP_FLOOR )  ){
	    TS_VecToRot( &kamome->turn, &vecAim );
	    kamome->turn.vx -= 128;
	}else{
	    TS_VecToRot( &kamome->turn, &vecAim );
	    _sceVu0ScaleVector( &vecStep, &vecZ, kamome->fSpeed * kmmng->fBaseSpeed + 10.f);
	    _sceVu0AddVector( &kamome->vecMove, &vecStep, &kamome->mov);
	}
    }else{
	// 直接攻撃
	KMM_SetFlag( kamome, KMM_FLAG_DIRECT_ATC); 
	// 攻撃音チェック
	KMM_AddAtcSeList( kamome->pvManager, kamome);
	kamome->turn.vx = -128;
    }
    

    return 0;
}

// キャラ付随
static int KMM_ConcomitantMove( KAMOME* kamome )
{

    SVECTOR vecRot;
    FVECTOR vec;
    KMMTHINK* think;
    CONTROL*	pctrl;

    pctrl = kamome->pctrlAim; // NULLチェックは入る前にしている

    think = &kamome->think;

    vec.vx = _TS_Sin( ((think->nCount3 * 8) % 4096) ) * 10.f;
    vec.vy = _TS_Sin( ((think->nCount3 * 8 + 1024) % 4096) ) * 2.f;
    vec.vz = (vec.vx) * 0.5f;
    vec.vw = 1.f;

    kamome->turn.vx = 0;
    kamome->turn.vy = pctrl->rot.vy;
    kamome->turn.vz = (int)(-vec.vx * 16.f); // 傾き

    vecRot.vx = kamome->rot.vx;
    vecRot.vy = kamome->rot.vy;
    vecRot.vz = 0;

    MAO_AbsPos( &kamome->vecMove, &vec, &kamome->matOrg);

    vec.vx = pctrl->mov.vx;
    vec.vy = pctrl->mov.vy + 5000.f; // kamome->vecMove.vy;
    vec.vz = pctrl->mov.vz;

    MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &vec, 0.1f);

    return 0;
}

// カメラ方向付随
static int KMM_CameraZMove( KAMOME* kamome )
{

    SVECTOR vecRot;
    FVECTOR vec;
    KMMTHINK* think;

    think = &kamome->think;

    vec.vx = _TS_Sin( ((think->nCount3 * 8) % 4096) ) * 10.f;
    vec.vy = _TS_Sin( ((think->nCount3 * 8 + 1024) % 4096) ) * 2.f;
    vec.vz = (vec.vx) * 0.5f;
    vec.vw = 1.f;

    kamome->turn.vx = 0;
    kamome->turn.vz = (int)(-vec.vx * 16.f); // 傾き

    vecRot.vx = kamome->rot.vx;
    vecRot.vy = kamome->rot.vy;
    vecRot.vz = 0;

    MAO_AbsPos( &kamome->vecMove, &vec, &kamome->matOrg);

    { // カメラ方向に
	FVECTOR vecZ;
	_sceVu0ScaleVector( &vecZ, (FVECTOR *)&DG_Chanls[0].eye.m[2][0], 5000.f);
	_sceVu0AddVector( &vec, &GM_PlayerPosition, &vecZ);
    }

    MAO_InterpVec( &kamome->vecMove, &kamome->vecMove, &vec, 0.1f);

    return 0;
}

// 段ボール乗り
static int KMM_BoxRide( KAMOME* kamome )
{
    FVECTOR vecZ;
    FMATRIX matBox;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    _sceVu0CopyMatrix( &matBox, &GM_PlayerBoxBody->objs->objs[0].world);
    MAO_AbsVec(&kamome->vecAimPos, &kamome->vecLand, &matBox);
    _sceVu0AddVector(&kamome->vecAimPos, &kamome->vecAimPos, (FVECTOR*)&matBox.m[3][0]);
    kamome->vecAimPos.vy += KAMOME_Y_OFFSET;

    // 補間
    MAO_HermiteLerpVec( &kamome->vecMove, &vecZ, 
			&kamome->vecStartPos, &kamome->vecAimPos,
			&kamome->vecStartZ,  &kamome->vecAimZ, 
			((float)kamome->nRouteCntr / (float)kamome->nRouteTime));

    if ( kamome->nRouteCntr++ >= kamome->nRouteTime){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov;
	kamome->vecStartZ = kamome->vecAimZ;	
	return 1;
    }

    // 方向取得
    _sceVu0Normalize( &vecZ, &vecZ);
    TS_VecToRot( &kamome->turn, &vecZ );

    return 0;
}

// 落下攻撃
static int KMM_AddFallAtcList( KAMOME_MNG* kmmng, KAMOME* kamome)
{
    int i;

    if ( kmmng == NULL ) return 0;

    for ( i = 0; i < KMM_FALL_ATC_NUM;	i++){
	if ( kmmng->pkmmFallAtc[i] == NULL ){ // 空なら
	    kmmng->pkmmFallAtc[i] = kamome;
	    return 1;
	}
    }
    return 0;
}

// 落下
static int KMM_FallMove( KAMOME* kamome )
{
    FVECTOR vec;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 落下リストに追加
    KMM_AddFallAtcList( kmmng, kamome);

    _sceVu0CopyVector( &kamome->vecMove, &kamome->mov);

    // もとの進行速度をのこしつつ
    if ( kamome->fSpeed > 0.f ){
	vec.vx = kamome->vecFallSpd.vx * kamome->fSpeed;
	vec.vy = 0.f;
	vec.vz = kamome->vecFallSpd.vz * kamome->fSpeed;
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vec);
	kamome->fSpeed -= 0.01f;
	if ( kamome->fSpeed < 0.f ) kamome->fSpeed = 0.f;
    }
    // 落下
    kamome->vecMove.vy -= kamome->vecFallSpd.vy;
    if ( kamome->vecFallSpd.vy < kmmng->nFallSpeed) kamome->vecFallSpd.vy += KMM_FALL_ADD;

    switch ( kamome->nNumber % 4 ){
    case 0:
	kamome->turn.vy += 32;
	break;
    case 1:
	kamome->turn.vy -= 32;
	break;
    case 2:
	kamome->turn.vy -= 16;
	break;
    case 3:
	kamome->turn.vy += 16;
	break;
    }    
    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &kamome->vecMove, 
				KMM_CHK_HZX, 0, KMM_SKIP_FLOOR ) ){
	HZX_GetOnlinePoint( &kamome->vecMove);
	kamome->vecMove.vy += KMM_FLOOR_OFFSET;

	kamome->turn.vx = 0;
	kamome->turn.vy = Get_EvadeWallDir( kamome);
	kamome->turn.vz = 0;
	return 1;
    }

    return 0;
}



// 指定目標に対してゾーン追跡で移動（隣接エリアまで）
// -1 : 発見できない
//  0 : 追跡中
static int KMM_GetTargetNearZonePos( KAMOME* kamome, FVECTOR* pvecPos, int addr, FVECTOR* pvecAim, int* pnAddr )
{
    NAVIGATE 	navi;
    NAVITARGET	n_trg;

    if ( kamome->pctrlAim == NULL ) return -1; // 対象が見つからない

    navi.ctrl       = kamome->pctrlAim;
    navi.going_addr = -1;
    
    // ナビターゲット設定
    n_trg.pos  = (*pvecPos);
    n_trg.addr = addr;

    // ゾーン検索
    GM_ZoneNavi( &navi, &n_trg);

    pvecAim->vx = navi.next_zonepos.vx;
    pvecAim->vy = kamome->mov.vy;
    pvecAim->vz = navi.next_zonepos.vz;

    (*pnAddr) = navi.next_addr;

    return 0;
}
// 隣接ゾーン
static int KMM_CheckNearZone( int nCharaAddr, int nAimAddr )
{
    int i;
    int nCharaZone, nAimZone;
    HZX_ZON* 	pzone;		// ゾーンデータ
    extern HZX_ZON *ENE_HZX_GetZone(int addr);

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

static u_char TBL_AREA_INDEX[4][KMM_ROUTE_MAX] = {
    {  0, 2, 15, 13,  4, 1,  7, 14,  8, 10, 6, 12, 9,  3, 5, 11 },
    { 11, 5,  3,  9, 12, 6, 10,  8, 14,  7, 1,  4, 13, 15, 2, 0  },
    {  3, 13, 11, 1, 4, 14, 2,  0, 6,  15, 9,  12, 5, 7, 10, 8  },
    {  8, 10,  7, 5, 12, 9, 15, 6, 0, 2, 14, 4, 1, 11, 13, 3 },
};

// X % 4
// Y / 8
// Z / 4 % 2

static void KMM_MakeFlyingArea( KAMOME* kamome, FVECTOR* pvecMin, FVECTOR* pvecMax)
{
    int	i, nX, nY, nZ, nSeed;
    int	nIndexX, nIndexY, nIndexZ;
    int nOneX, nOneY, nOneZ;

    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    nOneX = (int)(pvecMax->vx - pvecMin->vx);
    nOneY = (int)(pvecMax->vy - pvecMin->vy);
    nOneZ = (int)(pvecMax->vz - pvecMin->vz);

    nX = (int)(nOneX / 4);
    nY = (int)(nOneY / 2);
    nZ = (int)(nOneZ / 2);

    nSeed = KMM_DemoRand( kmmng, 4 );
    
    for ( i = 0; i < KMM_ROUTE_MAX; i++){
	nIndexX = (int)(TBL_AREA_INDEX[nSeed][i] % 4);
	nIndexY = (int)(TBL_AREA_INDEX[nSeed][i] / 8);
	nIndexZ = (int)((int)(TBL_AREA_INDEX[nSeed][i] / 4) % 2);

	kamome->vecRoute[i].vx = (KMM_DemoRand( kmmng, nX ) + nX * nIndexX) + pvecMin->vx; 
	kamome->vecRoute[i].vy = (KMM_DemoRand( kmmng, nY ) + nY * nIndexY) + pvecMin->vy; 
	kamome->vecRoute[i].vz = (KMM_DemoRand( kmmng, nZ ) + nZ * nIndexZ) + pvecMin->vz; 
	kamome->vecRoute[i].vw = 1.f;
    }
}
#if 0
// 補間回転移動
static int KMM_QuatMove( KAMOME* kamome )
{
    int 		nRes = 0;
    FVECTOR 		vecSub, vecSpd;
    FMATRIX 		matTo, matRes;
    static FVECTOR 	vecY = { 0.f, 1.f, 0.f, 1.f };

    //    kamome->vecAimPos = GM_PlayerPosition;

    // かもめのマトリクス生成
    MAO_GetAimMatrix( &matTo, &kamome->vecAimPos, &kamome->mov, &vecY);

    // マトリクス補間
    MAO_InterpMatrix( &matRes, &kamome->matOrg, &matTo, 0.1f);

    kamome->fSpeed = 0.8f;

    // 座標更新
    _sceVu0ScaleVector( &vecSpd, (FVECTOR*)&matRes.m[2][0], kamome->fSpeed);
    _sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecSpd );

    // 向き更新
    TS_VecToRot( &kamome->turn, (FVECTOR*)&matRes.m[2][0] );

    // ルート更新
    {
	int nDist2;

	_sceVu0SubVector( &vecSub, &kamome->vecAimPos, &kamome->mov);
	nDist2 = _sceVu0InnerProduct( &vecSub, &vecSub); // 距離の２乗
	
	if ( nDist2 < KMM_DIST_CHECK2){
	    if ( kamome->bChgRouteIndex ){
		kamome->vecAimPos = kamome->vecRoute[kamome->nRouteIndex];
		// インデックス更新
		kamome->nRouteIndex++;
		kamome->bChgRouteIndex = 0;
		if ( kamome->nRouteIndex >= KMM_ROUTE_MAX){
		    kamome->nRouteIndex = 0;
		    nRes = 1; // ルート終端合図
		}
	    }
	}else{
	    if ( !kamome->bChgRouteIndex ){
		kamome->bChgRouteIndex = 1;
	    }
	}
    }
    return (nRes);
}
#endif
// オイラー角を使って移動
static int KMM_EulerMove( KAMOME* kamome )
{
    int nRes = 0;
    float   fSpdRate;
    SVECTOR vecAim;
    FVECTOR vecDiff;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 方向取得
    _sceVu0SubVector( &vecDiff, &kamome->vecAimPos, &kamome->mov);
    _sceVu0Normalize( &vecDiff, &vecDiff);
    TS_VecToRot( &vecAim, &vecDiff );
    {
	int		nDiff;
        int		nDiffLimit;
	int		incline;
	float		fSpd;
	float		fFloat;
	FVECTOR		vecSpd;

	incline = GV_DiffDirS( kamome->rot.vy, vecAim.vy );
	if ( incline > KMM_INCLINE_ANG ) incline = KMM_INCLINE_ANG;
	else if ( incline < -KMM_INCLINE_ANG ) incline = -KMM_INCLINE_ANG;

	fSpdRate = (1.f - KMM_SPDDECAY_RATE) + ( 1.f - (float)abs(incline) / (float)KMM_INCLINE_ANG ) * 
	    KMM_SPDDECAY_RATE;

	fFloat = 0.f;
	if ( kamome->action.current_mot == KMM_MOT_FLYING ||
	     kamome->action.current_mot == KMM_MOT_FLYING02 ||
	     kamome->action.current_mot == KMM_MOT_FLYING03 ){ // 飛行
	    float fRate = KMM_GetMotPlayRate( kamome);
	    float fAimRate;
	    if ( fRate < 0.25f ){
		fAimRate = fRate * 4.f;
		kamome->fSpeed += 0.05f * fAimRate;
	    }else if ( fRate < 0.5f ){
		fAimRate = 1.f;
		kamome->fSpeed += 0.05f;
	    }else if ( fRate < 0.75f ){
		fAimRate = (fRate - 0.5f) * 4.f;
		kamome->fSpeed += 0.05f * (1.f - fAimRate);
	    }else {
		fAimRate = (fRate - 0.75f) * 4.f;
		kamome->fSpeed -= 0.020f * fAimRate;
	    }

	    if ( fRate < 0.66666666667f ){
		fFloat = 16.f * (fRate / 0.66666666667f);
	    }else{
		fFloat = 16.f * ((1.f - fRate) / (1.f - 0.66666666667f));
	    }

	    if ( kamome->fSpeed > 1.f ) kamome->fSpeed = 1.f;
	    else if ( kamome->fSpeed < 0.f ) kamome->fSpeed = 0.f;
	    //	    kamome->fSpeed = fAimRate; // kamome->fSpeed * 0.9f + fAimRate * 0.1f;
	}else if ( kamome->action.current_mot == KMM_MOT_HOVER){
	    kamome->fSpeed -= 0.01f;
	    if ( kamome->fSpeed > 1.f ) kamome->fSpeed = 1.f;
	    else if ( kamome->fSpeed < 0.8f ) kamome->fSpeed = 0.8f;

	    fFloat = 0.f;
	}
	fSpd = (kamome->fSpeed * fSpdRate) * kmmng->fBaseSpeed;
	

	// 移動量を設定->座標更新
	_sceVu0ScaleVector( &vecSpd, (FVECTOR*)&kamome->matOrg.m[2][0], fSpd);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecSpd );
	kamome->vecMove.vy += fFloat;

	// 旋回
	nDiffLimit = KMM_ROTATE_ANG + (int)(KMM_ROTADD_ANG * fSpdRate);

	nDiff = GV_DiffDirS( kamome->rot.vx, vecAim.vx );
	if ( nDiff > nDiffLimit ) nDiff = nDiffLimit;
	else if ( nDiff < -nDiffLimit ) nDiff = -nDiffLimit;
	kamome->rot.vx += nDiff;

	nDiff = GV_DiffDirS( kamome->rot.vy, vecAim.vy );
	if ( nDiff > nDiffLimit ) nDiff = nDiffLimit;
	else if ( nDiff < -nDiffLimit ) nDiff = -nDiffLimit;
	kamome->rot.vy += nDiff;

	// 補間先角度設定
	kamome->turn.vx = kamome->rot.vx;
	kamome->turn.vy = kamome->rot.vy;

	incline = -incline;

   GV_NearExp16PV( &kamome->rot.vz, &incline, 1 );

   kamome->turn.vz = kamome->rot.vz;
    }


    // ルート更新
    {
	int nDist2;
	FVECTOR vecSub;

	_sceVu0SubVector( &vecSub, &kamome->vecAimPos, &kamome->mov);
	nDist2 = (int)_sceVu0InnerProduct( &vecSub, &vecSub); // 距離の２乗
	
	if ( nDist2 < KMM_DIST_CHECK2){
	    if ( kamome->bChgRouteIndex ){
		// インデックス更新
		kamome->nRouteIndex++;
		kamome->bChgRouteIndex = 0;
		if ( kamome->nRouteIndex >= KMM_ROUTE_MAX){
		    kamome->nRouteIndex = 0;
		    nRes = 1; // ルート終端合図
		}
		kamome->vecAimPos = kamome->vecRoute[kamome->nRouteIndex];
	    }
	}else{
	    if ( !kamome->bChgRouteIndex ){
		kamome->bChgRouteIndex = 1;
	    }
	}
    }
    return (nRes);
}


// オイラー角を使って着地
static int KMM_EulerLand( KAMOME* kamome )
{
    float   fSpdRate;
    SVECTOR vecAim;
    FVECTOR vecDiff;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 方向取得
//    _sceVu0SubVector( &vecDiff, &kamome->vecAimPos, &kamome->mov);
    _sceVu0SubVector( &vecDiff, &kamome->vecLand, &kamome->mov);
    _sceVu0Normalize( &vecDiff, &vecDiff);
    TS_VecToRot( &vecAim, &vecDiff );
    {
	int		nDiff;
        int		nDiffLimit;
	int		incline;
	float		fSpd;
	FVECTOR		vecSpd;

	incline = GV_DiffDirS( kamome->rot.vy, vecAim.vy );
	if ( incline > KMM_INCLINE_ANG ) incline = KMM_INCLINE_ANG;
	else if ( incline < -KMM_INCLINE_ANG ) incline = -KMM_INCLINE_ANG;

	fSpdRate = (1.f - KMM_SPDDECAY_RATE) + ( 1.f - (float)abs(incline) / (float)KMM_INCLINE_ANG ) * KMM_SPDDECAY_RATE;
	fSpd = kamome->fSpeed * kmmng->fBaseSpeed * fSpdRate;

	// 移動量を設定->座標更新
	_sceVu0ScaleVector( &vecSpd, (FVECTOR*)&kamome->matOrg.m[2][0], fSpd);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vecSpd );

	// 旋回
	nDiffLimit = KMM_ROTATE_ANG + (int)(KMM_ROTADD_ANG * fSpdRate);

	nDiff = GV_DiffDirS( kamome->rot.vx, vecAim.vx );
	if ( nDiff > nDiffLimit ) nDiff = nDiffLimit;
	else if ( nDiff < -nDiffLimit ) nDiff = -nDiffLimit;
	kamome->rot.vx += nDiff;

	nDiff = GV_DiffDirS( kamome->rot.vy, vecAim.vy );
	if ( nDiff > nDiffLimit ) nDiff = nDiffLimit;
	else if ( nDiff < -nDiffLimit ) nDiff = -nDiffLimit;
	kamome->rot.vy += nDiff;

	// 補間先角度設定
	kamome->turn.vx = kamome->rot.vx;
	kamome->turn.vy = kamome->rot.vy;

	incline = -incline;

   GV_NearExp16PV( &kamome->rot.vz, &incline, 1 );
	kamome->turn.vz = kamome->rot.vz;
    }

    // ルート更新
    {
	int nDist2;
	FVECTOR vecSub;

	_sceVu0SubVector( &vecSub, &kamome->vecLand, &kamome->mov);
	nDist2 = (int)_sceVu0InnerProduct( &vecSub, &vecSub); // 距離の２乗
	
	if ( nDist2 < KMM_DIST_CHECK2){
	    return (1);
	}
    }
    return (0);
}

// ダウン中の地面チェック
static int KMM_ChkDownFloorCheck( KAMOME* kamome)
{
    FVECTOR vecTo;

    _sceVu0CopyVector( &vecTo, &kamome->mov);
    vecTo.vy -= 200.f;

    // 床がなくなった
    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &vecTo, KMM_CHK_HZX, 0, KMM_SKIP_FLOOR ) == 0){ 
	return 1;
    }

    return 0;
}
