/*
    fatzone.c
    ファットマン戦用ゾーン取得
    2001/03/29 Masafumi Okuta
    $Id: fatzone.c,v 1.1.1.3 2002/11/19 11:48:02 Yoshizawa1 Exp $
*/
extern HZX_ZON *ENE_HZX_GetZone(int addr);

// ランダムでゾーンを選択
static int GetRandZone( Work* work )
{
    int nResZone;
    int nTmp;
    NPCWORK*	npc;
    npc = &work->npc;

    nResZone = KR_RandU(194) + 42;
    nTmp = HZX_Address( npc->ctrl->hzx_id, nResZone, nResZone);

    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nResZone ) ){
#ifdef DEBUG_MODE
printf("++++++++ next zone intrpt ++++++++\n");	
#endif
        // エリア対応ゾーンデータから目標(プレイヤー)の対となるゾーン番号を取得
        nResZone = FAT_GetAreaDataZone(work);
    }

    return nResZone;
}
// 自分から最も近い回避ゾーン番号取得
// 失敗した時はエリア対応ゾーンから目標(プレイヤー)の対となるゾーン番号を取得
static int FAT_GetSafetyZoneNear( Work* work )
{
    int i;
    int nTmp;
    int nZone;
    int nResZoneNo;
    int nTmpDist;
    int nDist;
    HZX_ZON* pzonePlayer;
    HZX_ZON* pzoneFatman;
    NPCWORK*	npc;

    npc = &work->npc;
    // プレイヤー,ファットマンのゾーン取得
    pzonePlayer = ENE_HZX_GetZone( GM_PlayerControl->addr );
    pzoneFatman = ENE_HZX_GetZone( npc->ctrl->addr );
    
    nResZoneNo = -1;
    nDist = 100000;
    for ( i = 0; i < HZX_MAX_SAFEZONE_NUM; i++){
	HZX_ZON* pzoneTmp;
	FVECTOR  vec1, vecSub;

	nZone = pzonePlayer->safes[i];
	nTmp = HZX_Address( npc->ctrl->hzx_id, nZone, nZone);
	if (pzonePlayer->safes[i] == HZX_NO_ZONE ||
	    pzonePlayer->safes[i] == HZX_Zone1(npc->ctrl->addr) || 
	    FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1( npc->ctrl->addr), nZone ) ){
            continue;
	}
	// 最も近い安全ゾーンを探す
	pzoneTmp = ENE_HZX_GetZone( nTmp );
	vec1.vx = pzoneTmp->x;
	vec1.vy = pzoneTmp->y;
	vec1.vz = pzoneTmp->z;
	_sceVu0SubVector( &vecSub, &vec1, &npc->ctrl->mov);
	nTmpDist = _FVecLen3( &vecSub ) ;	// 距離 

	// 更新チェック
	if (nTmpDist < nDist){
	    nDist = nTmpDist;
	    nResZoneNo = pzonePlayer->safes[i];
	}
    }

    // 検索失敗
    if ( nResZoneNo == -1 ){
	// エリア対応ゾーンデータから目標(プレイヤー)の対となるゾーン番号を取得
	nResZoneNo = FAT_GetAreaDataZone(work);
printf("safe near error!!  ===== %d\n", nResZoneNo);
    }

    return nResZoneNo;
}
// 隣接ゾーンのなかから与えられたベクトルに対して最も内積値の小さいゾーンを返す
static int FAT_GetZoneFromInnerMin( NPCWORK* npc,
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
	    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), pzone->nears[i] ) ){
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
#if 0
// 隣接ゾーンのなかから与えられたベクトルに対して最も内積値の大きいゾーンを返す
static int FAT_GetZoneFromInnerMax( NPCWORK* npc,
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
	    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1( npc->ctrl->addr), pzone->nears[i] ) ) {
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
#if 0
// 指定方向にあって指定距離以上離れているゾーン取得
static int GetFarZoneDir(NPCWORK* 	npc,
			 int		nAddr,		// 基準となるアドレス
			 FVECTOR	vecDir,		// 指定方向
			 int 		nDistMax,	// 指定距離
		         int*		pnResDist)	// 結果距離
{
    int 	nZone;
    int 	nRes, nTmp;
    int 	nResDist;
    int 	nDist;
    float	fResInner;

    nRes = nZone = HZX_Zone1(nAddr);
    nResDist = nDist = 0;
    nDistMax = nDistMax >> 8; // ゾーン距離に変換
    fResInner = 1.f;

    // 一定距離はなれるまで検索
    while (nDist < nDistMax){
	nResDist = nDist;
	nRes = FAT_GetZoneFromInnerMax( npc, nZone, &vecDir, &fResInner); // 内積値の一番大きい隣接ゾーンを返す
	if (nRes == -1)     	break; // 空だった
	nTmp = HZX_Address( npc->ctrl->hzx_id, nRes, nRes);
	if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nRes ) ) { // 一応省略できる
	    break;
	}
	if (fResInner > 0.f){ // 前方にあり
	    // 距離計算
	    nDist += HZX_ZoneDistance( npc->ctrl->hzx_id, HZX_Zone1(nAddr), nRes);
	    // 目標ゾーン更新
	    nZone = nRes;
	}else{
	    break;
	}
    }

    if ( pnResDist != NULL ){ // ゾーン計算距離を変換して距離を返す
	(*pnResDist) = nResDist << 8;
    }

    return nZone;
}
#endif
#if 0
// プレイヤーの向いている方向にあって指定距離以上離れているゾーン取得
static int GetFarZonePlayerDir(NPCWORK*	npc,
			       int 	nDistMax,	// 指定距離
			       int*	pnResDist)	// 帰ってきた距離
{
    FMATRIX mat;
    FVECTOR* pvec;

    // 向き取得
    DG_SetPos2( &GM_PlayerControl->mov, &GM_PlayerControl->rot);
    DG_GetPos(&mat);
    pvec = (FVECTOR*)&mat.m[2][0];

    return GetFarZoneDir( npc, GM_PlayerAddress, (*pvec), nDistMax, pnResDist);
}
#endif
// プレイヤーを避ける移動方向の取得
#define PL_EVADE_RAD 	(500)
#define EVADE_DIR	(512)
static int FAT_GetEvadeVector(Work* 	work,
			      int*	pnDir) 	
{
    int nDir;
    NPCWORK* npc;

    npc = &work->npc;
// とりあえず封印
    return 0;

    if ( work->pl_dis <= work->nEvadeRad + PL_EVADE_RAD){
	if (GV_DiffDirS((int)npc->ctrl->rot.vy, work->pl_dir) < 0){
	    nDir = work->pl_dir + EVADE_DIR;
	    nDir = 4095 & nDir;
	}else{
	    nDir = work->pl_dir - EVADE_DIR;
	    if ( nDir < 0 ){
		nDir += 4096;
	    }
	}
	(*pnDir) = nDir;
	return 1;
    }
    return 0;
}

// エリア別候補データから指定エリアの対となるゾーン番号を取得
static int FAT_GetAreaDataZoneEx( Work* work, int nArea )
{
    NPCWORK* npc;

    npc = &work->npc;

    return ( work->nAreaZone[nArea][ KR_RandU(5)]);
}
// エリア別候補データからプレイヤーの対となるゾーン番号を取得
static int FAT_GetAreaDataZone( Work* work )
{
    NPCWORK* npc;

    npc = &work->npc;

    if ( FAT_GetContArea( work->nPlaArea ) != work->nFatArea ){
	return ( work->nAreaZone[FAT_GetContArea( work->nPlaArea )][ KR_RandU(5)]);
    }else{
	return ( FAT_GetAreaDataZoneEx( work, FAT_GetContArea( work->nFatArea ) ) );
    }
}
// 回転設置用参照ゾーンデータ作成
static void FAT_InitTurnPutRef( Work* work )
{
    int i;

    for ( i = 0; i < FAT_TURNPUT_NUM; i++){
	work->nTurnPutRefZone[i] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_TURNPUT_AIM[i], -1) );
    }
}

// 距離一定以上で方向差が一番小さい回転設置用参照ゾーンを取得
static int FAT_GetTurnPutRefZone( Work* work )
{
    int i, nRes;
    int nDist, nDir, nDirDiff, nDirDiffTmp;
    NPCWORK* npc;

    npc = &work->npc;

    nRes = -1;
    nDist = 0;
    nDir = 0;
    nDirDiff = 4096;
    for ( i = 0; i < FAT_TURNPUT_NUM; i++){
	MAO_GetDiffVec3( NULL, &nDist, &nDir, &npc->ctrl->mov, &FAT_TURNPUT_AIM[i] );

	if ( nDist < 8000) continue;

	nDirDiffTmp = GV_DiffDirAbs( npc->ctrl->rot.vy, nDir);

	if ( nDirDiffTmp < nDirDiff ){
	    nRes = work->nTurnPutRefZone[i];
	    nDirDiff = nDirDiffTmp;
	}
    }

    return (nRes);
}

// 指定方向で一番離れているゾーン番号と距離を返す
static int FAT_FarZoneInFan( Work* work, int* pnDist, int nFromAddr, int nDir, int nChkDir)
{
    int i, nZoneDir, nResZone, nFromZone, nDist, nDistTmp, nToAddr;
    NPCWORK*	npc;

    npc = &work->npc;
    nResZone = -1;
    nFromZone = HZX_Zone1(nFromAddr);
    nDist = 0;

    for ( i = 0; i < 255; i++ ){
	nToAddr = HZX_Address( npc->ctrl->hzx_id, i, i);
	// 可視判定
	if ( ENE_ReadOnlinInfo( nFromAddr, nToAddr ) ) continue;

	// 方向チェック
	nZoneDir = MAO_GetDirZoneToZone( nFromZone, i);
	if ( GV_DiffDirAbs( nDir, nZoneDir ) > nChkDir ) continue;

	// 遮断チェック
	if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, nFromZone, i ) ) {
	    continue;
	}

	// 距離判定
	nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, nFromZone, i);
	if ( nDistTmp >= nDist ){
	    nResZone = i;
	    nDist = nDistTmp;
	}
    }
    
    if ( pnDist != NULL ) (*pnDist) = nDist;

    return nResZone;
}

// 背後撃ち条件をチェック :: 
// 成立時は目標ゾーン,不成立時は-1が返る
static int FAT_CheckBackShoot( Work* work)
{
    int nDir, nDist, nDistTmp;
    int nTmpZone, nResZone, nTmp;
    NPCWORK*	npc;

    npc = &work->npc;

    if ( GV_DiffDirAbs( work->pl_dir, npc->ctrl->rot.vy ) < 1024 ){ // プレイヤーが背後に回った
	return -1;
    }

    if ( work->pl_dir >= 512 && work->pl_dir < 1536)	    nDir = 1024;
    else if ( work->pl_dir >= 1536 && work->pl_dir < 2560)  nDir = 2048;
    else if ( work->pl_dir >= 2560 && work->pl_dir < 3584)  nDir = 3064;
    else 	    					    nDir = 0;


    nDir = (nDir + 1024) & 4095;
    nResZone = FAT_FarZoneInFan( work, &nDist, npc->ctrl->addr, nDir, 128);
    if ( nResZone != -1 ){
	nDist = 0;
    }

    nDir = (nDir + 1024) & 4095;
    nTmpZone = FAT_FarZoneInFan( work, &nDistTmp, npc->ctrl->addr, nDir, 128);
    if ( nTmpZone != -1 && nDistTmp > nDist ){
	nResZone = nTmpZone;
	nDist = nDistTmp;
    }

    nDir = (nDir + 1024) & 4095;
    nTmpZone = FAT_FarZoneInFan( work, &nDistTmp, npc->ctrl->addr, nDir, 128);
    if ( nTmpZone != -1 && nDistTmp > nDist ){
	nResZone = nTmpZone;
	nDist = nDistTmp;
    }

    nTmp = HZX_Address( npc->ctrl->hzx_id, nResZone, nResZone);
    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nResZone) ){
	nResZone = -1;
    }    
    
    return (nResZone);
}

// 離れたゾーンがあるかを方向（1024毎で分類）,最低距離を条件としてチェック
// 成立時は目標ゾーン,不成立時は-1が返る
static int FAT_ChkFarZoneFromCrossDir( Work* work, int nDirCurrent, int nDistMin)
{
    int nDir, nDist;
    int nResZone;
    NPCWORK*	npc;

    npc = &work->npc;

    nResZone = -1;

    if ( nDirCurrent >= 512 && nDirCurrent < 1536)	    nDir = 1024;
    else if ( nDirCurrent >= 1536 && nDirCurrent < 2560)    nDir = 2048;
    else if ( nDirCurrent >= 2560 && nDirCurrent < 3584)    nDir = 3064;
    else 	    					    nDir = 0;


    nResZone = FAT_FarZoneInFan( work, &nDist, npc->ctrl->addr, nDir, 128);
    if ( nResZone != -1 && nDist < nDistMin ){
	nResZone = -1;
    }
    return (nResZone);
}
#if 0
// 指定されたゾーンの隣接ゾーンからそのゾーンを通過しないゾーンを取得
static int FAT_GetNearZoneNotPass( Work* work, int nZone)
{
    int i;
    HZX_ZON* 	pzone;
    NPCWORK*	npc;

    npc = &work->npc;

    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, nZone, nZone ));	
    // 隣接ゾーンから探す
    for ( i = 0; i < 6; i++){
	if ( pzone->nears[i] != HZX_NO_ZONE){
	    if ( !MAO_ChkZoneInZ2Z( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), pzone->nears[i], nZone) ){
		return (pzone->nears[i]); // 隣接ゾーンで元ゾーンを通過しないものを取得
	    }
	}
    }

    return (-1);
}
#endif
// 指定されたゾーンの隣接ゾーンからそのゾーンを通過しないゾーンを取得
// 方向計算をして一番方向差の少ないゾーンを取得
static int FAT_GetNearZoneNotPassDirPrio( Work* work, int nZone, int nAimDir)
{
    int 	i, nDir, nDirTmp, nDirDiff, nResZone;
    HZX_ZON* 	pzone;
    NPCWORK*	npc;

    npc = &work->npc;

    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, nZone, nZone ));	
    // 隣接ゾーンから探す
    nDir 	= 4096;
    nResZone 	= -1;
    for ( i = 0; i < 6; i++){
	if ( pzone->nears[i] != HZX_NO_ZONE){
	    // 隣接ゾーンで元ゾーンを通過しないものを取得
	    if ( !MAO_ChkZoneInZ2Z( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), pzone->nears[i], nZone) ){
		nDirTmp = MAO_GetDirZoneToZone( nZone, pzone->nears[i]);
		nDirDiff = GV_DiffDirAbs( nAimDir, nDirTmp);
		if ( nDirDiff < nDir ){
		    nDir = nDirDiff;
		    nResZone = i;
		}
	    }
	}
    }
    if ( nResZone >= 0 ) return (pzone->nears[nResZone]); 
    else		 return (-1);
}
