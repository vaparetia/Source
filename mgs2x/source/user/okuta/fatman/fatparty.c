/*
    fatparty.c
    ファットマン構造体用関数群（C++でいうところのSet***,Get***関数に近いものが置かれている）
    2001/03/19 Masafumi Okuta
    $Id: fatparty.c,v 1.1.1.3 2002/11/19 11:48:00 Yoshizawa1 Exp $
*/
static void FAT_SetStep( Work* 	work,		// ファットマンワーク
			 int	nStep)		// ステップ
{
    work->nStep  = nStep;
    work->nStepCntr = 0;
}

// ファットマン関連ファイルで完結するのでstaticにしておく
// 思考レベル１変更
static void FAT_SetThink1( Work* work,		// ファットマンワーク
			   int	 nNewThink1,	// 新しいルーチン:優先レベル
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
#ifdef DEBUG_MODE
    work->nPrevThink1 = work->think1;
#endif    

    if ( work->think1 != TH1_DAMAGE && nNewThink1 == TH1_DAMAGE ){
	work->nBeforeDmgThink3 = work->think3;
    }

    work->think1 = nNewThink1;
    FAT_SetThink2( work, nNewThink2, nNewThink3);
}
// 思考レベル２変更
static void FAT_SetThink2( Work* work,		// ファットマンワーク
			   int	 nNewThink2,	// 新しいルーチン:フェーズレベル
			   int	 nNewThink3)	// 新しいルーチン:動作レベル
{
#ifdef DEBUG_MODE
    work->nPrevThink2 = work->think2;
#endif    
    if ( work->think2 < TH2_START_DAMAGE && nNewThink2 < TH2_START_DAMAGE ){
	FAT_SetNewPhase( work, nNewThink2);	// フェーズ設定
	work->nRollerBreak = 0;			// ローラブレード壊れフラグクリア
    }

    work->think2 = nNewThink2;
    FAT_SetThink3( work, nNewThink3);
#if 1
    // 次候補を無効にする
    work->nNextThink3   = -1;
    work->bNext3Valid 	= 0;
#endif
}
// 思考レベル３変更
static void FAT_SetTh3( Work* 	work,		// ファットマンワーク
			int	nNewThink3)	// 新しいルーチン
{
#ifdef DEBUG_MODE
    work->nPrevThink3 = work->think3;
    work->nPrevCntr3  = work->count3;
    work->nPrevNextThink3 = work->nNextThink3;
#endif    
    if ( GM_IsGameOver() && work->think2 < TH2_START_DAMAGE ){	    // ゲームオーバー
	work->think1 = TH1_EVENT;
	work->think2 = TH2_EVENT;
	work->think3 = TH3_STAND;
	work->count3 = 0;
	work->nStep  = 0;
	work->npc.action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
	return;
    }

    work->think3 = nNewThink3;
    work->count3 = 0;
    work->nStep  = 0;
    work->npc.action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
}
// 次のルーチンストック
static void FAT_SetNextThink3( Work* work,		// ファットマンワーク
			       int   nNextThink3)	// 次のルーチン
{
    work->nNextThink3 = nNextThink3;
    work->bNext3Valid = 1;
}
// 思考レベル３変更:次候補が有効な場合はそちらからルーチンを選ぶ
static void FAT_SetThink3FromNext( Work* work,		// ファットマンワーク
				   int   nNewThink3)	// 新しいルーチン
{
    if ( !work->bNext3Valid ){	// 次候補が有効でなかった
	FAT_SetThink3( work, nNewThink3 );
	return;
    }

    FAT_SetThink3( work, work->nNextThink3 );

    // 次候補を無効にする
    work->nNextThink3   = -1;
    work->bNext3Valid 	= 0;
}
#if 0
// 次の候補を選出
static int FAT_SetNextFromNext2( Work* work)
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( work->nNextZone2Valid && FAT_CheckZone(work->nNextZone2) >= 0 ){ // 次候補あり
	FAT_SetNextZone( work, work->nNextZone2);
	_sceVu0CopyVector( &work->vecNextPos, &work->vecNextPos2);
	work->nNextZone2Valid = 0;
	work->nNextZone2 = -1;	 
	_sceVu0CopyVector( &work->vecNextPos2, &npc->ctrl->mov);
    }

    return 0;
}
#endif
// 新たな目標を設定する
static int FAT_SetNAim( Work* work, int nNextZone, FVECTOR* pvec)
{
    NPCWORK*	npc;
    npc = &work->npc;

    nNextZone = HZX_Zone1(nNextZone);

    if ( nNextZone < 0 || nNextZone >= 255 ){
	return -1;
    }

    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nNextZone) ){
	return -1;
    }
    FAT_SetNextZone( work, nNextZone);		// 目標ゾーン設定
    if ( pvec != NULL ){
	_sceVu0CopyVector( &work->vecNextPos, pvec );
    }else{
	HZX_ZON* pzone;
	pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, nNextZone, nNextZone ));
	if ( pzone != NULL ){
	    work->vecNextPos.vx = pzone->x;
	    work->vecNextPos.vy = pzone->y;
	    work->vecNextPos.vz = pzone->z;
	    work->vecNextPos.vw = 1.f;
	}else{
	    return (-1);
	}
    }
    return 0;
}

// 次に向かうゾーン設定
static void FAT_SetNZone( Work* work,		// ファットマンワーク
			  int   nNextZone)	// 次のゾーン
{
    work->nNextZone = nNextZone;	
}
// 次の次に向かうゾーン設定
static void FAT_SetNZone2( Work* work,		// ファットマンワーク
			   int   nNext2Zone)		// 次のゾーン
{
    HZX_ZON* 	pzone;
    NPCWORK*	npc;
    npc = &work->npc;

    if ( nNext2Zone < 0 || nNext2Zone >= 255 ){
	return;
    }

    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nNext2Zone) ){
	return;
    }
    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, nNext2Zone, nNext2Zone));

    if ( pzone == NULL ) return;

    work->nNextZone2 = nNext2Zone;

    { // ゾーン中心
	FVECTOR vec;
	vec.vx = pzone->x;
	vec.vy = pzone->y;
	vec.vz = pzone->z;
	vec.vw = 1.f;
	_sceVu0CopyVector( &work->vecNextPos2, &vec);
    }

    work->nNextZone2Valid = 1;
}

// 次の次に向かうゾーンを優先した設定
static void FAT_SetNZoneFrom2( Work* work,		// ファットマンワーク
			       int   nNextZone)	// 次のゾーン
{
    if ( !work->nNextZone2Valid ){ // Next2が無効な時
	FAT_SetNextZone( work, nNextZone );
	return;
    }else{
	FAT_SetNextZone( work, work->nNextZone2 );
	work->nNextZone2 = -1;
	work->nNextZone2Valid = 0;
    }
}
#if 1
// 次に向かうゾーンを設定
static void FAT_SetNPoint( Work*      work, 		// ファットマンワーク
			   int	 nNextZone,	// 次に向かうゾーン
			   FVECTOR*   pvecNext)	// 次に向かう位置:NULLの時はゾーン中心
{
    FVECTOR  vec;
    HZX_ZON* pzone;
    NPCWORK*	npc;

    npc = &work->npc;

    FAT_SetNextZone( work, nNextZone);	
    
    if ( pvecNext != NULL ){ // 位置指定あり
	_sceVu0CopyVector( &work->vecNextPos, pvecNext);
	if ( !HZX_InsideZone( npc->ctrl->hzx_id, pvecNext, work->nNextZone) ){ // 一致しない
	    FAT_SetNextZone( work, HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, pvecNext, -1) )); 
	}
    }else{
	// ゾーンの中心にする
	pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone) );
	vec.vx = pzone->x;
	vec.vy = pzone->y;
	vec.vz = pzone->z;
	vec.vw = 1.f;
	_sceVu0CopyVector( &work->vecNextPos, &vec);
    }
}
#else
// 次に向かうゾーンを設定
static void FAT_SetNPoint( Work*      work, 		// ファットマンワーク
			   int	 nNextZone,	// 次に向かうゾーン
			   FVECTOR*   pvecNext)	// 次に向かう位置:NULLの時はゾーン中心
{
    FVECTOR  vec;
    HZX_ZON* pzone;
    NPCWORK*	npc;

    npc = &work->npc;

    FAT_SetNextZone( work, nNextZone);	
    
    if ( pvecNext != NULL ){ // 位置指定あり
	if ( !HZX_InsideZone( npc->ctrl->hzx_id, pvecNext, work->nNextZone) ){ // 一致しない
	    // ゾーンの中心にする
	    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone) );
	    vec.vx = pzone->x;
	    vec.vy = pzone->y;
	    vec.vz = pzone->z;
	    vec.vw = 1.f;
	    _sceVu0CopyVector( &work->vecNextPos, &vec);
	}else{
	    // 指定点
	    _sceVu0CopyVector( &work->vecNextPos, pvecNext);
	}
    }else{
	// ゾーンの中心にする
	pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone) );
	vec.vx = pzone->x;
	vec.vy = pzone->y;
	vec.vz = pzone->z;
	vec.vw = 1.f;
	_sceVu0CopyVector( &work->vecNextPos, &vec);
    }
}
#endif
// 中継点を設定して目標のゾーンに向かう
static void FAT_SetNZoneWithRelayPoint( Work*      work, 		// ファットマンワーク
					int	   nRelayZone,	// 中継ゾーン
					int	   nAimZone,	// 目標ゾーン
					FVECTOR*   pvecRelay,	// 中継点 : NULLの時はゾーン中心
					FVECTOR*   pvecAim)		// 目標点 : NULLの時はゾーン中心
{
    FVECTOR  vec;
    HZX_ZON* pzone;
    NPCWORK*	npc;

    npc = &work->npc;

    FAT_SetNextZone( work, nRelayZone);	
    FAT_SetNextZone( work, nAimZone);

    // 中継設定
    if ( pvecRelay != NULL ){
#ifdef DEBUG_MODE
printf("Relay Zone == %d ::: %d\n", work->nNextZone, HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, pvecRelay, -1) ));
#endif
	_sceVu0CopyVector( &work->vecNextPos, pvecRelay);
	if ( !HZX_InsideZone( npc->ctrl->hzx_id, pvecRelay, work->nNextZone) ){
	    FAT_SetNextZone( work, HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, pvecRelay, -1) )); // 終点
	}
    }else{
	pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone) );
	vec.vx = pzone->x;
	vec.vy = pzone->y;
	vec.vz = pzone->z;
	vec.vw = 1.f;
	_sceVu0CopyVector( &work->vecNextPos, &vec);
    }

    // 目標設定
    if ( pvecAim != NULL ){
#ifdef DEBUG_MODE
printf("Aim Zone == %d ::: %d\n", work->nNextZone2, HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, pvecAim, -1) ));
#endif
	_sceVu0CopyVector( &work->vecNextPos2, pvecAim);
	if ( !HZX_InsideZone( npc->ctrl->hzx_id, pvecAim, work->nNextZone2) ){
	    FAT_SetNextZone( work, HZX_Zone1( HZX_GetAddress( npc->ctrl->hzx_id, pvecAim, -1) )); // 終点
	}
    }else{
	pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone2, work->nNextZone2) );
	vec.vx = pzone->x;
	vec.vy = pzone->y;
	vec.vz = pzone->z;
	vec.vw = 1.f;
	_sceVu0CopyVector( &work->vecNextPos2, &vec);
    }
}

// 新しいフェーズ設定
static void FAT_SetNewPhase( Work* work, int nPhase)
{
    NPCWORK*	npc;

    npc = &work->npc;
    // フェーズ更新
    work->nPrevPhase = work->nPhase;
    work->nPhase = nPhase;	
    work->nPhaseCntr = 0;

    if ( nPhase == TH2_BOMB_PHASE ){
	work->nBombPhaseCntr++;
	if ( work->nBombPhaseCntr >= 100000){
	    work->nBombPhaseCntr = 10000;
	}
    }else if ( nPhase == TH2_SHOOT_PHASE ){
	work->nShootPhaseCntr++;
	if ( work->nShootPhaseCntr >= 100000){
	    work->nShootPhaseCntr = 10000;
	}
    }

    // フェーズ開始時の耐久値取得
    work->nPhaseStartLife  = npc->action.life;
    work->nPhaseStartFaint = work->m9_faint;

    work->nVitalCntr = work->nVitalMax;	// 持久力を元に戻す

    // 情報送信用プロシージャコール
    if ( work->procFatInfo != 0){
	GCL_ARGS  gcl_args;
	int       data;
	
	data = work->nPhase;
	gcl_args.argc = 1;
	gcl_args.argv = &data;
	GCL_ExecProc( work->procFatInfo, &gcl_args ); // フェーズ更新
    }
}

// 銃口をターゲットに向ける
static void FAT_AdjustAimPoint( Work* 	 work,		// ファットマンワーク
				FVECTOR* pvecTrg)	// 目標地点
{
    FVECTOR	vecPos, vecOffset;
    FMATRIX 	mat;
    SVECTOR 	vecRot;
    NPCWORK*	npc;
    NPCACT*	act;

    npc = &work->npc;
    act = &work->npc.action;

    // グロックの位置
    vecOffset.vx = 9.f;
    vecOffset.vy = 0.f;
    vecOffset.vz = 71.f;
    vecOffset.vw = 1.f;
    mat = BODYWORLD( npc->body, HUMAN21_MIGI_TE );
    MAO_AbsPos( &vecPos, &vecOffset, &mat);
    _sceVu0CopyVector( (FVECTOR *)&mat.m[3][0], &vecPos);
    
    // vecTrgに向くための回転角度を求める
    GM_TrgToNearRotLimit( &vecPos, work->vecKoshiRot.vy, pvecTrg, &work->svecAdjust ) ;

    // モーションをいじって体をターゲット方向に向ける
    vecRot.vx = work->svecAdjust.vx / 6;
    vecRot.vy = work->svecAdjust.vy / 6;
    vecRot.vz = work->svecAdjust.vz / 6;

//  GM_AdjustRotBody( &work->body, &work->svecAdjust, HUMAN21_ATAMA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_ONAKA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MUNE );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE1 );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE2 );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_TE );
    work->bAdjustFlag = 1; // アジャスト設定フラグを立てる
}
// アジャストクリア
static void FAT_ClearAdjust( Work* work)
{
    SVECTOR 	vecRot;
    vecRot = DG_ZeroSVector;
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_ATAMA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_ONAKA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MUNE );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE1 );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE2 );
    GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_TE );
}
#if 0
// 可視パーツ情報から狙撃位置を設定
static int FAT_GetAimParts( Work* work )
{
    int i;
    int nRes;

    nRes = -1;

    for ( i = 0; i < FAT_CHK_MAX; i++){
	if ( work->pl_parts & (1 << i)){
	    nRes = FatCheckParts[i]; // 関節番号を返す
	}
    }

    return (nRes);
}
#endif
// グロック発射
static void FAT_ShootGlock( NPCWORK* npc,	// NPCワーク
			    int	  nRandX,	// x回転ランダム幅
			    int	  nRandY,	// y回転ランダム幅
			    int	  nRandZ)	// z回転ランダム幅
{
    Work*	work;
    SVECTOR	vecRot;
    FVECTOR	vec;
    FVECTOR	vecOffset;
    FMATRIX	matOrg, mat;

    work = (Work *)npc->character;
    if ( work->nBullet == 0 ) return; // 弾なし

    DG_SetPos( &BODYWORLD( npc->body, HUMAN21_MIGI_TE ) ) ;
    if ( nRandX != 0 ) vecRot.vx = KR_RandS( nRandX );
    else	       vecRot.vx = 0;
    if ( nRandY != 0 ) vecRot.vy = KR_RandS( nRandY );
    else	       vecRot.vy = 0;
    if ( nRandZ != 0 ) vecRot.vz = KR_RandS( nRandZ );
    else	       vecRot.vz = 0;
    DG_RotatePos( &vecRot ) ;
    DG_GetPos( &matOrg ) ;
    work->nBullet--;

    // 弾
    vecOffset.vx = 9.f;
    vecOffset.vy = 170.f;//-270.f;
    vecOffset.vz = 71.f;//71.f;
    vecOffset.vw = 1.f;
    MAO_AbsPos( &vec, &vecOffset, &matOrg);
    mat = matOrg;
    _sceVu0CopyVector( (FVECTOR *)&mat.m[3][0], &vec);
#ifdef DEBUG_MODE
//AN_Test_Eye2( &vec, 1 );
//MAO_DbgDrawMatrix( &mat, 150.f);
//printf("power rate = %f\n", work->fPowerRate);
#endif
    NewBullet( &mat, (BUL_TYPE_VISIBLE | BUL_TYPE_SPARK | BUL_TYPE_SPARK_SMOKE | 
		      BUL_TYPE_SCAR | BUL_TYPE_RED | BUL_TYPE_NOISE), PLAYER_SIDE, 
	       10, (int)((float)5 * work->fPowerRate), GLK_RANGE, 500, WP_Usp ) ;

    // 薬莢
    AN_CartridgeGLK_E( &BODYWORLD( npc->body, HUMAN21_MIGI_TE ), &work->gun, npc->ctrl );

    // マズルとブローバック
//    mat = BODYWORLD(&work->body, HUMAN21_MIGI_TE);
    AN_MazzleGLK( &BODYWORLD(&work->body, HUMAN21_MIGI_TE), 1, 0);
    work->wctrl.flag = WPEF_FLG_START;
}
// 指定の位置からオブジェが見えるかどうか
static int CheckPointLookableObject( FVECTOR*	pfvec,	// 指定する位置
				     OBJECT* 	pobj,	// 対象オブジェ
				     int	hzx_id,	// ハザードID
				     int	dir,	// 方向	
				     int	ang)	// 角度	
{
    int 	i;
    int		nRes;
    FVECTOR  	fvecPos;

    fvecPos = *pfvec;

    nRes = 0;
    // 指定点とプレイヤーの特定部位をオンラインチェックして結果を格納していく
    for ( i = 0; i < FAT_CHK_MAX; i++){
	FVECTOR*  	pfvecPart;
	FVECTOR 	vec;

	// パーツの位置取得
	pfvecPart = (FVECTOR *)&BODYWORLD( pobj, FatCheckParts[i]).m[3][0];
	// 方向取得
	_sceVu0SubVector( &vec, pfvecPart, &fvecPos ) ;

	// 指定方向から一定角度内に存在して
	if ( GV_DiffDirAbs( dir, _FVecDir2(&vec)) < ang && 
	     !ENE_EyeOnlineCheck( hzx_id, &fvecPos, pfvecPart ) ){
	    nRes |= (1 << i);
	}
    }

    return (nRes);
}

// 解体フェーズからフェーズ変更
static void FAT_ChgPhase( Work* work )
{
    work->nPhaseNum++; // フェーズ数更新
    if (work->nPhaseNum >= 1000000){
	work->nPhaseNum = 100000;
    }

    if ( GM_GameLevel == GM_LEVEL_EXTREME || GM_GameLevel == GM_LEVEL_E_EXTREME ){	// エクストリームは爆弾のみ
	FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	return;
    }

    switch( work->nShootPhaseCntr ){
    case FAT_PHASELEVEL_1:
	if ( work->nBombPhaseCntr < 2 ){ 	// 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{				// 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
	break;
    case FAT_PHASELEVEL_2:
	if ( work->nBombPhaseCntr < 4 ){ 	// 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{				// 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
	break;
    case FAT_PHASELEVEL_3:
	if ( work->nBombPhaseCntr < 5 ){ 	// 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{					// 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
	break;
    case FAT_PHASELEVEL_4:
	if ( work->nBombPhaseCntr < 6 ){ 	// 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{					// 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
	break;
    case FAT_PHASELEVEL_5:
	if ( work->nBombPhaseCntr < 5 ){ 	// 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{					// 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
	break;
    default:
#ifdef DEBUG_MODE
	printf("Phase Over!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif	
	if ( GM_GameLevel == GM_LEVEL_HARD || GM_GameLevel == GM_LEVEL_EXTREME || GM_GameLevel == GM_LEVEL_E_EXTREME) { // 爆弾フェーズ
	    FAT_SetThink2( work, TH2_BOMB_PHASE, TH3_PHASE_START);
	}else{									   // 銃撃フェーズ
	    FAT_SetThink2( work, TH2_SHOOT_PHASE, TH3_PHASE_START);
	}
    }
}
#if 0
// 次のゾーンと次の次のゾーンの角度を求める
static void FAT_GetDirNext( Work* work )
{
    int dir1, dir2, dir_diff, addr;
    FVECTOR vec, vec1, vecSub;
    HZX_ZON* pzone;
    NPCWORK*	npc;

    npc = &work->npc;
    pzone = ENE_HZX_GetZone( npc->ctrl->addr );
    vec1.vx = pzone->x;
    vec1.vy = pzone->y;
    vec1.vz = pzone->z;

    _sceVu0SubVector( &vecSub, &vec1, &npc->navi->next_zonepos );
    _sceVu0Normalize( &vec, &vecSub );
    dir1 = _FVecDir2( &vec ) ;	// プレイヤーへの方向 
    
    pzone = ENE_HZX_GetZone( npc->ctrl->addr );
    vec1.vx = pzone->x;
    vec1.vy = pzone->y;
    vec1.vz = pzone->z;

    addr = HZX_Zone1( HZX_NextZoneCrossGroup( HZX_Zone1(npc->navi->next_addr), HZX_Zone1(npc->navi->going_addr) ));
    pzone = ENE_HZX_GetZone( addr );
    vec1.vx = pzone->x;
    vec1.vy = pzone->y;
    vec1.vz = pzone->z;

    _sceVu0SubVector( &vecSub, &npc->navi->next_zonepos, &vec1);
    _sceVu0Normalize( &vec, &vecSub );
    dir2 = _FVecDir2( &vec ) ;	// プレイヤーへの方向 

    dir_diff = GV_DiffDirAbs( dir1, dir2 );
}

#endif

// ゾーンに入る位置を計算
static void FAT_GetZoneInPoint(FVECTOR* pvecRes, FVECTOR* pvec, int addr)
{
    float fD, fD2;
    FVECTOR vec, vecSub, vecZone;
    HZX_ZON* pzone;

    pzone = ENE_HZX_GetZone( HZX_Zone1( addr ) );
    vecZone.vx = pzone->x;
    vecZone.vy = pzone->y;
    vecZone.vz = pzone->z;

    _sceVu0SubVector( &vecSub, &vecZone, pvec);
    _sceVu0Normalize( &vec, &vecSub );

    // 傾き計算
    if (pzone->w != 0 && vec.vx != 0.f){
	fD  = (float)pzone->h / (float)pzone->w;
	fD2 = vec.vz / vec.vx;
    }else{
	pvecRes->vx = pzone->x;
	pvecRes->vy = (float)(pzone->y + 200.f);
	if ( vec.vz >= 0.f )	pvecRes->vz = (float)(pzone->z - pzone->h);
	else			pvecRes->vz = (float)(pzone->z + pzone->h);
	return;
    }

    // 交点計算
    if ( FABS(fD) < FABS(fD2) ){ // 
	pvecRes->vx = pzone->x - vec.vx * (float)pzone->h;
	pvecRes->vy = (float)(pzone->y + 200.f);
	if ( vec.vz >= 0.f )	pvecRes->vz = (float)(pzone->z - pzone->h);
	else			pvecRes->vz = (float)(pzone->z + pzone->h);
    }else{
	if ( vec.vx >= 0.f )	pvecRes->vx = (float)(pzone->x - pzone->w);
	else			pvecRes->vx = (float)(pzone->x + pzone->w);
	pvecRes->vy = (float)(pzone->y + 200.f);
	pvecRes->vz = pzone->z - vec.vz * (float)pzone->w;
    }
}
#if 0
// 再帰呼出型
static void FAT_GetSpeed(float* pfSpd, 		// 速度:結果
			 float* pfVelRate, 	// 減衰値参照用
			 NPCWORK* npc, 		// 
			 FVECTOR mov, 		// 
			 int addr, 
			 int dir, 
			 float fCurDist, 
			 int nCount)
{
    float	fDist;
    int		nDirDiff;
    int		nDiffLevel;
    int		newdir;
    FVECTOR 	vec, vecSub;

    if ( HZX_Zone1(addr) == -1 )
	return;

    FAT_GetZoneInPoint( &vec, &mov, addr);

    _sceVu0SubVector( &vecSub, &vec, &mov);
    fDist    = (float)_FVecLen3( &vecSub );
    newdir = _FVecDir2( &vecSub);
    nDirDiff = GV_DiffDirAbs( dir, newdir );
    if ( nDirDiff > 1024 ) nDiffLevel = nDirDiff - 1024;
    else		   nDiffLevel = nDirDiff;

    fCurDist += fDist;

//printf("%d : %f %f %d\n", nCount, (*pfSpd), fDist, nDiffLevel);
    if (nCount-- > 0 && addr != npc->navi->going_addr){
	HZX_ZONE_ADD hzaAddr;
	hzaAddr = HZX_NextZoneCrossGroup( addr, npc->navi->going_addr);
	addr = HZX_Zone1(hzaAddr);
	(*pfVelRate) *= 0.8f;
	(*pfSpd) -= (((float)nDiffLevel / 1024.f) * (*pfVelRate) );
	FAT_GetSpeed( pfSpd, pfVelRate, npc, vec, addr, newdir, fCurDist, nCount); // 再帰
    }
}
#endif
// 指定ゾーンの隣接ゾーンに入ったか判定
static int FAT_CheckNearZoneIn( int nCharaAddr,	// 指定キャラのゾーン番号
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

/* ターゲットコールバック関数 */
static	void	ChildTargCallBack( off, def, ptr )
TARGET	*off, *def ;
void	*ptr ;
{
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    def->power->force = off->power->force ;
//	    _sceVu0Normalize( &def->power->force, &def->power->force);

	}
    }
}
// モーションで移動中かどうか判別
static int FAT_CheckMotionMove( Work* work)
{
    NPCWORK* npc;
    NPCACT*  act;

    npc = &work->npc;
    act = &npc->action;

    switch( act->current_mot ){
    case FAT_MOT_RUN_START:
    case FAT_MOT_RUN:
    case FAT_MOT_BACK_RUN:
    case FAT_MOT_REVERSE:
    case FAT_MOT_RUN_TURN:
    case FAT_MOT_TURN_L:
    case FAT_MOT_TURN_R:
    case FAT_MOT_RUN_FIRE:
    case FAT_MOT_BACK_FIRE:
    case FAT_MOT_BODYATTACK:
    case FAT_MOT_RUN_RELOAD:
    case FAT_MOT_BOMB_RUNSET:
    case FAT_MOT_BOMB_RUNSET_L:
    case FAT_MOT_BOMB_RUNSET_R:
    case FAT_MOT_DANCE_1:
	return 1;
	break;
    }
#if 0
    if  ( (act->current_mot >= FAT_MOT_CHK_RUN_START && act->current_mot <= FAT_MOT_CHK_RUN_START) ||
	   act->current_mot == FAT_MOT_RUN_FIRE || act->current_mot == FAT_MOT_RUN_RELOAD ||
	   act->current_mot == FAT_MOT_BODYATTACK || act->current_mot == FAT_MOT_BOMB_RUNSET ||
	   act->current_mot == FAT_MOT_BOMB_RUNSET_L || act->current_mot == FAT_MOT_BOMB_RUNSET_R){
	return 1;
    }
#endif	
    return 0;
}
#if 0
static int FAT_CheckTurnLR( Work* work )
{
    NPCWORK*	npc;

    npc = &work->npc ;
    switch ( work->nRunMode ){
    case FAT_RUNMODE_TURN_L:
	if ( work->fBezierOne != 0.f)	work->nTurnMotRate = ( 70.f / (1.f / work->fBezierOne));
	else				work->nTurnMotRate = 1.f;
	NPC_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_TURN_L, MOTION_MASK_FULL, 
			    FAT_INTERP_DEF, (float)TIME_BASE * (work->nTurnMotRate * 3.f));
	return 1;
	break;
    case FAT_RUNMODE_TURN_R:	
	if ( work->fBezierOne != 0.f)	work->nTurnMotRate = ( 70.f / (1.f / work->fBezierOne));
	else				work->nTurnMotRate = 1.f;
	NPC_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_TURN_R, MOTION_MASK_FULL, 
			    FAT_INTERP_DEF, (float)TIME_BASE * (work->nTurnMotRate * 3.f));
	return 1;
	break;
    default:
	work->nTurnMotRate = 1.f;
    }

    return 0;
}
#endif
// ファットマンエルード落とし
static void FAT_EludeFall( Work* work )
{
    int 	nFall;
    FVECTOR	vecDir;
    FMATRIX 	mat;
    NPCWORK*	npc;
    extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );

    npc = &work->npc ;
    nFall = 0;

    // 走行時だけ
    if ( !(npc->action.status & NPC_ACT_STATUS_MOVE) ) return;

    // エルード中でプレイヤー生存,デモでない時
    if ( ((work->nFatInfo & FAT_INFO_ELUDE_NEAR) || (work->nFatInfo & FAT_INFO_ELUDE_FAR)) && 
	 !(work->nFatInfo & FAT_INFO_PL_DEAD) && !GM_CheckGameStatus( STATE_DEMO ) ){
	_sceVu0CopyMatrix( &mat, &BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE));
	if ( _MAO_FVec2Len2( &npc->ctrl->mov, (FVECTOR*)&mat.m[3][0] ) < 1000.f ){
	    MAO_GetDiffVec3( &vecDir, NULL, NULL, &npc->ctrl->mov,  (FVECTOR *)&mat.m[3][0]);
	    MAO_GetDiffVec3( &vecDir, NULL, NULL, &npc->ctrl->mov,  (FVECTOR *)&mat.m[3][0]);
	    _sceVu0ScaleVector( &vecDir, &vecDir, 3000.f);
	    GV_SetActorChild( work, NewBlood( &BODYWORLD( GM_PlayerBody, HUMAN21_MIGI_TE), 
					      (FVECTOR *)&mat.m[3][0], &vecDir, 0, 0 ) ) ;
	    nFall = 1;
	}
	_sceVu0CopyMatrix( &mat, &BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE));
	if ( _MAO_FVec2Len2( &npc->ctrl->mov, (FVECTOR *)&mat.m[3][0] ) < 1000.f ){
	    MAO_GetDiffVec3( &vecDir, NULL, NULL, &npc->ctrl->mov,  (FVECTOR *)&mat.m[3][0]);
	    _sceVu0ScaleVector( &vecDir, &vecDir, 3000.f);
	    GV_SetActorChild( work, NewBlood( &BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE), 
					      (FVECTOR *)&mat.m[3][0], &vecDir, 0, 0 ) ) ;
	    nFall = 1;
	}

	if ( nFall && GM_PlayerTarget != NULL ){ 
	    FVECTOR vecForce, vecSize;
	    TARGET*		offence;	// 攻撃用ターゲット
	    POWER_TARGET*	power;

	    GM_SeSetMode( FAT_SE[SE_ELATACK3], (FVECTOR *)&BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_TE).m[3][0], GM_SEMODE_BOMB); 

	    offence = &( work->atcElude );
	    power   = &( work->powElude );

	    vecSize.vx = 500.f;
	    vecSize.vy = 500.f;
	    vecSize.vz = 500.f;
	    vecSize.vx = 1.f;
	    
	    GM_SetTarget( offence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); 
	    GM_SetTargetWeaponType( offence, WP_PUNCHR ); 	// 武器タイプを設定
	    GM_MoveTarget3( offence, &mat );	// 体に追従
	    _sceVu0ScaleVector( &vecForce, (FVECTOR*)&mat.m[2][0], 300.f);
	    GM_SetPowerTarget( offence, power, POWER_ONCE, 255, 0, 1000, &vecForce );
	    GM_PutTarget( offence );				   // 攻撃ターゲット配置
	    GM_TargetSetDirectAttack( offence, GM_PlayerTarget ); // 直接攻撃ターゲットをたたき込む

	}
    }
}

#if 0
// 逆走撃ち判定
static int FAT_BackShoot( Work* work )
{
    int nRes, nDir;
    NPCWORK*	npc;
    npc = &work->npc;

    if ( GV_DiffDirAbs( work->pl_dir, npc->ctrl->rot.vy) > 1024 ){
	nRes = FAT_GetBackShootZoneLinePlaFront( work, &nDir );
	if ( nDir < 256 ){
	    return nRes;
	}
    }
    return -1;
}
#endif

// 反転チェック
static int FAT_CheckReverse( Work* work, int nNextZone )
{
    int nDir;
    NPCWORK*	npc;
    npc = &work->npc;

    nDir = GV_DiffDirAbs( MAO_GetDirZoneToZone( HZX_Zone1(npc->ctrl->addr), nNextZone), work->npc.ctrl->turn.vy);

    if ( nDir >= 1024 && nDir <= 3072) return 1;

    return 0;
}

// 次に移動する位置とナビターゲット設定
static void FAT_SetNPointWithNavigate( Work* work, int nZone, FVECTOR* pvec)
{
    NPCWORK*	npc;
    npc = &work->npc;

    FAT_SetNextPoint( work, nZone, pvec);

    work->navitrg.addr = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone);
    _sceVu0CopyVector( &work->navitrg.pos, &work->vecNextPos);
}

// ゾーン設定チェック
static int FAT_CheckNextZone( Work* work)
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( work->nNextZone == HZX_Zone1( npc->ctrl->addr ) ){
	FAT_SetThink3( work, TH3_STAND); // 立ち
	return -1;
    }	
    if (work->nNextZone == -1){	// 次候補がおかしい
        FAT_SetThink3( work, TH3_READY);
	return -1;
    }

    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), work->nNextZone) ){
        FAT_SetThink3( work, TH3_READY);
	return -1;
    }
    return 0;
}

// ゾーンチェック
static int FAT_CheckZone( Work* work, int nZone)
{
    NPCWORK*	npc;

    npc = &work->npc;

    if ( nZone == -1 ){	// 次候補がおかしい
	return -1;
    }

    if ( FAT_IntrptZoneCheck( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), nZone) ){
	return -1;
    }
    return 0;
}

#if 0
// 難易度取得
static int FAT_GetDifficulty( Work* work)
{
    return (GM_GameLevel);	
}
#endif
// ゾーン番号チェック
// 不正な時に１を返す
static int FAT_ChkZoneWrong( NPCWORK* npc, int nZone)
{
    int nTmp;

    if ( nZone < 0 || nZone >= 255 ){
	return 1;
    }

    nTmp = HZX_Address( npc->ctrl->hzx_id, nZone, nZone);
    if ( (GM_GetRIntrptZ2Z( npc->ctrl->addr, nTmp ) != NULL) ||
	 !GM_GetZIntrptZ2Z( npc->ctrl->addr, nTmp ) ){

    }
    return 0;
}
// めりこみ回避
static void FAT_EvadeIntoWall( NPCWORK* npc)
{
    int nWallDir1, nWallDir2, nDir;

    // 壁に接触していない
    if ( npc->ctrl->n_touches == 0) return;
    
    // 壁の方向を取得
    nWallDir1 = GV_VecDir2( &npc->ctrl->vecs[ 0 ] );
    nWallDir2 = GV_VecDir2( &npc->ctrl->vecs[ 1 ] );

    // 壁の方向とファットマンの方向の差が小さい方を取得
    if ( GV_DiffDirAbs( nWallDir1, npc->ctrl->rot.vy ) < GV_DiffDirAbs( nWallDir2, npc->ctrl->rot.vy )){
	nDir = nWallDir1;
    }else{
	nDir = nWallDir2;
    }

    // 壁接触したら頭がめりこまないように壁の方向に垂直な方向に向ける
    if ( GV_DiffDirS( nDir, npc->ctrl->rot.vy ) >= 0) {
	npc->ctrl->turn.vy = (nDir + 1024) & 4095;
    }else{
	npc->ctrl->turn.vy = (nDir - 1024) & 4095;
    }
}
// プレイヤーを押し戻す
#define BASE_EVADEPUSH_SPEED (80.f)
#define BASE_EVADEPUSH_TIME  (COUNT_VMODE(16))
static void FAT_CalcNockBack( Work* work)
{
    int nDist;

    if ( work->control.mov.vx == GM_PlayerPosition.vx && 
	 work->control.mov.vy == GM_PlayerPosition.vy && 
	 work->control.mov.vz == GM_PlayerPosition.vz){
	work->vecPush.vx = 0.f;
	work->vecPush.vy = 0.f;
	work->vecPush.vz = 1.f;
	work->vecPush.vw = 1.f;
	nDist = 0;
    }else{
	MAO_GetDiffVec3( &work->vecPush, &nDist, NULL, &work->control.mov, &GM_PlayerPosition);   // 距離と方向を取得
    }	

    if ( nDist < 1000 ){
	_sceVu0ScaleVector( &work->vecPush, &work->vecPush, BASE_EVADEPUSH_SPEED); 	       // めりこみ分を弾き返す
	work->vecPush.vy = 0.f;
	if ( work->nEvadePush == 0 && !GM_CheckPlayerStatus( PLAYER_WATCH) ){
	    FAT_CallSe( work, SE_REBFAT01, &work->npc.ctrl->mov, GM_SEMODE_NORMAL); // ぼいーん音
	}
	work->nEvadePush = BASE_EVADEPUSH_TIME;
    }
}

// 押し戻し
static void FAT_CalcPushPlayer( Work* work)
{
    GM_ResetPlayerStatus( PLAYER_PAD_OFF);
    if ( work->nEvadePush > 0){
	// 保険
	if ( work->nEvadePush > BASE_EVADEPUSH_TIME){
	    work->nEvadePush = BASE_EVADEPUSH_TIME;
	}
	if ( work->pl_dis < 1000 ){
	    _sceVu0ScaleVector( &work->vecPush, &work->vecPush, 0.9f);
	    // プレイヤーのステップに加算
	    if ( !GM_CheckPlayerStatus( PLAYER_DOWNED ) && !GM_CheckPlayerStatus( PLAYER_DAMAGED ) && 
		 !GM_CheckPlayerStatus( PLAYER_WATCH ) && !GM_CheckPlayerStatus( PLAYER_SQUAT ) && 
		 !GM_CheckPlayerStatus( PLAYER_GROUND )  && !GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) ){
		_sceVu0AddVector( &GM_PlayerControl->step, &GM_PlayerControl->step, &work->vecPush);      
		GM_SetPlayerStatus( PLAYER_PAD_OFF); // パッドオフ
	    }
	    work->nEvadeConse++;
	    if ( PL_CurrentWeapon() != WP_ColdSpray && work->nEvadeConse >= COUNT_VMODE(180) ){
		BodyEvade( &work->npc); // 緊急回避
                work->nEvadeConse = 0;
	    }
	}else{
	    work->nEvadeConse = 0;
	}

	work->nEvadePush--;
    }else{
	if ( work->pl_dis > 3000 ){ // 離れたら
	    work->nEvadeConse = 0;
	}
    }
#ifdef DEBUG_MODE
#if 0
    MENU_Locate( 100, 320, 0 );
    MENU_SetColor( 0x00, 0x00, 0x80 );
    MENU_Printf("cons - %d : push - %d :: %d", work->nEvadeConse, work->nEvadePush, work->pl_dis < 1000);
#endif
#endif

}

// 遮断していたら１が返る
static int FAT_IntrptZoneCheck( HZX_GROUP_ID hzx_id, int nFromZone, int nToZone)
{
    HZX_ZON* 	pzone;
    int 	nFromAddr, nToAddr;
     
    nFromAddr = HZX_Address( hzx_id, nFromZone, nFromZone );
    nToAddr   = HZX_Address( hzx_id, nToZone, nToZone );
    pzone = ENE_HZX_GetZone( nToAddr );

    if ( (GM_GetRIntrptZ2Z( nFromAddr, nToAddr ) != NULL) ||
	 !GM_GetZIntrptZ2Z( nFromAddr, nToAddr ) || (pzone->flag & 0x01)) {
	return 1;
    }

    return 0;
}
