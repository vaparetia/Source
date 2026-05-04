/*
    kmthink.c
    カモメ思考処理
    2001/05/02 Masafumi Okuta
    $Id: kmthink.c,v 1.1.1.3 2002/11/19 11:48:05 Yoshizawa1 Exp $
*/

//static int UpdateInterpWork( KAMOME* kamome);

#include "BP_TrophySystem.h"

static int DamageCheck( KAMOME* kamome )
{
    KAMOME_MNG* kmmng;
    KMMACT* act;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;

    if ( act->status & KMM_STATUS_DEAD ) { // 死亡
	KMM_SetThink1( kamome, TH1_DAMAGE, TH2_DAMAGE, TH3_FALL); // 落下
	return 1;
    }

    if ( act->status & KMM_STATUS_SLEEP ) { // 睡眠
        if ( kamome->think.nThink3 == TH3_GROUNDSLEEP || kamome->think.nThink3 == TH3_STANDSLEEP){
	    // アジャストいれるかも
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, kamome->think.nThink3);
	    kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間を再設定
	}else if ( kamome->think.nThink3 == TH3_REST ||
		   kamome->think.nThink3 == TH3_RESTGROUND ){ // 立ったまま睡眠
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_STANDSLEEP);
	    kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間を再設定
	}else{ 					 // 地面に落ちて睡眠
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_SLEEPFALL);
	    kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間を再設定
	}
	return 1;
    }

    if ( act->status & KMM_STATUS_FEINT ) { // 気絶
	if ( kamome->think.nThink3 == TH3_GROUNDSLEEP || kamome->think.nThink3 == TH3_STANDSLEEP){
	    // アジャストいれるかも
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, kamome->think.nThink3);
	    kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_FEINT_MIN, KMM_FEINT_MAX); // 気絶時間を再設定
	}else {
	    KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FEINTFALL);
	    kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_FEINT_MIN, KMM_FEINT_MAX); // 気絶時間を再設定
	}
	return 1;
    }

    if ( act->status & KMM_STATUS_DAMAGE ) { // ダメージ
	KMM_SetThink1( kamome, TH1_DAMAGE, TH2_DAMAGE, TH3_WAIT);
	return 1;
    }

    return 0;
}

// 羽休め
static void Think3_Rest( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // ダンボール羽休めモードのとき
    if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	// プレイヤーがダンボールモードでなくなったらもしくはZOEダンボールorステルス迷彩を装備したら止める
	if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) || (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	    KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}
    }
    // コントロールキャラ用
    if (  kamome->think.nThink3 == TH3_CTRL_REST ){
	FVECTOR 	vecPos;
	CONTROL* 	pctrl;
	OBJECT*		pbody;
	if ( (pctrl = GM_SearchWhere(kamome->nAimName)) == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}

	pbody = pctrl->object;
	if ( pbody == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}
	_sceVu0ApplyMatrix( &vecPos, &BODYWORLD( pbody, kamome->nNode), &kamome->vecOffset );

	kamome->vecLand.vx = vecPos.vx;
	kamome->vecLand.vy = vecPos.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = vecPos.vz;
	kamome->vecLand.vw = 1.f;	

	_sceVu0CopyVector( &kamome->vecAimPos, &kamome->vecLand);	// 着地点
	_sceVu0CopyVector( &kamome->vecMove, &kamome->vecLand);		// 位置更新

	// w20cファットマン戦用
	if ( kamome->nNode == HUMAN21_ATAMA){
	    if ( kamome->action.current_mot == KMM_MOT_PICK_A ||
		 kamome->action.current_mot == KMM_MOT_PICK_B){
		if ( MEMMOT_CheckMotionTime( &kamome->mmt_ctrl, 19) ){
		    KMM_PickAttackMessage( kamome);
		}
	    }
	}
#if 0
	{
	    if ( kamome->action.current_mot == KMM_MOT_PICK_B ){
		if ( MEMMOT_CheckMotionTime( &kamome->mmt_ctrl, 19) ||
		     MEMMOT_CheckMotionTime( &kamome->mmt_ctrl, 33) ||
		     MEMMOT_CheckMotionTime( &kamome->mmt_ctrl, 45) ){
		    KMM_PickAttackMessage( kamome);
		}
	    }else if ( kamome->action.current_mot == KMM_MOT_PICK_A ){
		if ( MEMMOT_CheckMotionTime( &kamome->mmt_ctrl, 79) ){
		    KMM_PickAttackMessage( kamome);
		}
	    }
	}
#endif
    }


    // 初期設定
    if ( think->nCount3 == 0 ){

    }
    kamome->action.pad = PAD_IDLE; // 待機

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    if ( kamome->nRestTime > 0 && think->nCount3 >= kamome->nRestTime){ // 時間指定あり
	KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	kamome->nRestTime = -1; // 無効にする
	return;
    }
    
    think->nCount3++;
}
// 地面で羽休め
static void Think3_RestGround( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // ダンボール羽休めモードのとき
    if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	// プレイヤーがダンボールモードでなくなったらもしくはZOEダンボールorステルス迷彩を装備したら止める
	if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) || (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	    KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}
    }

    // 床チェック
    if ( ( kamome->nNumber % 8 == GV_Time % 8 ) && KMM_ChkDownFloorCheck( kamome) ){
	KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	kamome->nRestTime = -1; // 無効にする
	return;
    }

    // 初期設定
    if ( think->nCount3 == 0 ){
	_sceVu0CopyVector( &kamome->vecLand, &kamome->mov); // 着地点
    }
    kamome->action.pad = PAD_IDLEGROUND; // 待機

						
    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

//    _sceVu0CopyVector( &kamome->vecMove, &kamome->vecLand);

    if ( kamome->nRestTime > 0 && think->nCount3 >= kamome->nRestTime){ // 時間指定あり
	KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	kamome->nRestTime = -1; // 無効にする
	return;
    }
    
    think->nCount3++;
}
// 飛行
static void Think3_Flying( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->bChgRouteIndex = 0;
	// 補間ワークを設定しなおす
	KMM_UpdateHermiteWork( kamome );
    }
    kamome->action.pad = PAD_FLYING;

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    {
	if ( (kamome->action.current_mot == KMM_MOT_IDLE) ||
	     (kamome->action.current_mot >= KMM_MOT_WALK_START && kamome->action.current_mot <= KMM_MOT_SURPRISE_BACK) || 
	     (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30) ) ||
	     (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69) ) ){
	}else{
	    KMM_EulerMove( kamome );	// オイラー角補間移動（最速）
	}
    }

    think->nCount3++;
}
// 滑空
static void Think3_Hovering( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;

    kamome->action.pad = PAD_HOVERING;

    // 滑空移動
    UpdateHoverWork( kamome );

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    think->nCount3++;
}
// 着地移動
static void Think3_LandingMove( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov; 	// 始点
//	kamome->vecAimPos   = kamome->vecLand;	// 終点

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecLand);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_LANDING);
	    return;
	}
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
//	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
//	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }


    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    
//    if ( KMM_LandMove( kamome )){	// エルミート補間で移動
    if ( KMM_EulerLand( kamome )){	// オイラー角を使って着地
					       
	KMM_SetThink3( kamome, TH3_LANDING);
	return;
    }

    if ( think->nCount3 > COUNT_VMODE(1800) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    think->nCount3++;
}
// 着地
static void Think3_Landing( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // ダンボール着地モードのとき
    if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	// プレイヤーがダンボールモードでなくなったらもしくはZOEダンボールorステルス迷彩を装備したら止める
	if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) || (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
	_sceVu0CopyVector( &kamome->vecAimZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	kamome->turn.vx = 0.f;
	kamome->turn.vz = 0.f;
    }

    // コントロールキャラ用
    if (  kamome->think.nThink3 == TH3_CTRL_LANDING ){
	FVECTOR 	vecPos;
	CONTROL* 	pctrl;
	OBJECT*	pbody;
	if ( (pctrl = GM_SearchWhere(kamome->nAimName)) == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}

	pbody = pctrl->object;
	if ( pbody == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
	_sceVu0ApplyMatrix( &vecPos, &BODYWORLD( pbody, kamome->nNode), &kamome->vecOffset );

	kamome->vecLand.vx = vecPos.vx;
	kamome->vecLand.vy = vecPos.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = vecPos.vz;
	kamome->vecLand.vw = 1.f;	

	_sceVu0CopyVector( &kamome->vecAimPos, &kamome->vecLand);	// 着地点
    }


    if ( think->nCount3 == 0 ){
	kamome->action.pad = PAD_LANDING;

	kamome->nHeight = kamome->vecLand.vy;
	
	_sceVu0CopyVector( &kamome->vecAimZ, &kamome->vecLandZ);
	kamome->turn.vx = 0;
	kamome->turn.vy = kamome->nLandYRot;
	kamome->turn.vz = 0;
    }

    // 移動
    KMM_LandingMove( kamome );

    if ( kamome->action.act_end && (_MAO_FVec2Len2( &kamome->mov, &kamome->vecLand) < 10.f) ){
	_sceVu0CopyVector( &kamome->vecMove, &kamome->vecLand); // 着地点に強制的に移動
	if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	    KMM_SetThink3( kamome, TH3_CBOX_REST); // 段ボールの上で羽休め
	}else if  ( kamome->think.nThink3 == TH3_ELUDELAND ){
	    KMM_SetThink3( kamome, TH3_ELUDING);   // エルード妨害中
	}else if  ( kamome->think.nThink3 == TH3_CTRL_LANDING ){
	    KMM_SetThink3( kamome, TH3_CTRL_REST);   // キャラ付随
	}else{
	    KMM_SetThink3( kamome, TH3_REST);	   // 羽休め
	}
	return;
    }

    think->nCount3++;
}
#if 0
// コントロールキャラに着地
static void Think3_CtrlLanding( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // ダンボール着地モードのとき
    if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	// プレイヤーがダンボールモードでなくなったらもしくはZOEダンボールorステルス迷彩を装備したら止める
	if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) || (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
	_sceVu0CopyVector( &kamome->vecAimZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	kamome->turn.vx = 0.f;
	kamome->turn.vz = 0.f;
    }

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->action.pad = PAD_LANDING;
	// ダンボール着地モードのとき
	if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	    kamome->vecLand = kamome->vecAimPos ;	// 終点を設定
	}
	kamome->nHeight = kamome->vecLand.vy;
	
        _sceVu0CopyVector( &kamome->vecAimZ, &kamome->vecLandZ);
	kamome->turn.vx = 0;
	kamome->turn.vy = kamome->nLandYRot;
	kamome->turn.vz = 0;
    }

    // 移動
    KMM_LandingMove( kamome );

    if ( kamome->action.act_end && (_MAO_FVec2Len2( &kamome->mov, &kamome->vecLand) < 10.f) ){
	if ( kamome->think.nThink3 >= TH3_CHECK_CBOX_START && kamome->think.nThink3 <= TH3_CHECK_CBOX_END ){
	    KMM_SetThink3( kamome, TH3_CBOX_REST); // 段ボールの上で羽休め
	}else if  ( kamome->think.nThink3 == TH3_ELUDELAND ){
	    KMM_SetThink3( kamome, TH3_ELUDING);   // エルード妨害中
	}else{
	    KMM_SetThink3( kamome, TH3_REST);	   // 羽休め
	}
	return;
    }

    think->nCount3++;
}
#endif
// 集合移動
static void Think3_FlipMove( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov; 	// 始点
	kamome->vecAimPos   = kamome->vecLand;	// 終点

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_FLIPFLOP);
	    return;
	}
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 着地点の向き
	MAO_GetDiffVec3( &kamome->vecLandZ, NULL, NULL, &kamome->vecAimPos, &kamome->vecAggAim);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // エルミート補間で移動
    if (KMM_LandMove( kamome )){
	KMM_SetThink3( kamome, TH3_FLIPFLOP);
	return;
    }

    think->nCount3++;
}
// ばたつく
static void Think3_Flipflop( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->vecAppend = DG_ZeroVector;
    }

    kamome->action.pad = PAD_FLIPFLOP;	// ばたつき動作

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 
    KMM_FlipflopMove( kamome );

    think->nCount3++;
}
static void Think3_Backward( KAMOME* kamome )
{
    KMMTHINK* think;

    think = &kamome->think;

}
// 攻撃移動
static void Think3_AttackMove( KAMOME* kamome )
{
    int g, addr;
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    extern void  HZX_Pos2Zone( FVECTOR* pos, int* g, int* z );

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;

    if ( !KMM_SearchAim( kamome, kamome->nAimName) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 攻撃対象がいない
    if ( kamome->pctrlAim == NULL ||
	 kamome->ptrgAim  == NULL ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // ZOEダンボールorステルス迷彩だ！！
    if ( (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // カモメのゾーン
    HZX_Pos2Zone( &kamome->mov, &g, &addr);

    // 初期設定
    if ( think->nCount3 == 0 ){
	int nRes, nAddr;
	HZX_ZON* z;
	FVECTOR vecAim;

	z = HZX_GetZoneFromAdd( addr);

	nRes = KMM_GetTargetNearZonePos( kamome, &kamome->mov, addr, &vecAim, &nAddr);
	if ( nRes == -1 ){ // 見つからず
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}else if ( nRes == 1){ // 到達している
	    KMM_SetThink3( kamome, TH3_ATTACK);
	    return;
	}
	if ( nAddr == HZX_NO_ZONE || addr == HZX_NO_ZONE || z == NULL){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}

	{
	    kamome->nRouteCntr = 0;
	    kamome->vecStartPos = kamome->mov; 	// 始点
	    kamome->vecAimPos   = vecAim;	// 終点

	    // 距離取得
	    MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);

	    if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
		KMM_SetThink3( kamome, TH3_ATTACK);
		return;
	    }

	    kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	    // 向き
	    _sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	    _sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	    _sceVu0AddVector( &kamome->vecAimZ, &kamome->mov, &kamome->pctrlAim->mov); 
	    _sceVu0Normalize( &kamome->vecAimZ, &kamome->vecAimZ);
	    _sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecAimZ, kamome->fSpeed * kmmng->fBaseSpeed);
	}
//KMM_DbgDumpMoveWork(kamome);

	kamome->nNear = 0;
    }

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 移動
    if ( kamome->nNear ){ // 隣接ゾーンチェック
	int nRes = KMM_AttackMove( kamome ); // 攻撃移動
	if ( nRes == 1){ 	 // 攻撃
	    KMM_SetThink3( kamome, TH3_ATTACK);
	    return;
	}else if ( nRes == -1 ){ // 障害物ですすめなくなった
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
    }else{ 
	// 中継点に移動
        // KMM_LandMove( kamome)
	if ( KMM_EulerLand( kamome) || KMM_CheckNearZone( addr, HZX_Zone1(kamome->pctrlAim->addr) ) ){
	    kamome->nNear = 1;
	}
    }

    think->nCount3++;

}

// 攻撃移動
static void Think3_Attack( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( think->nCount3 == 0 ){

    }

    // 付随対象がいない
    if ( kamome->pctrlAim == NULL ||
	 kamome->ptrgAim  == NULL ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // ZOEダンボールorステルス迷彩だ！！
    if ( (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 動作設定
    kamome->action.pad = PAD_POKE;	// つつき

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 攻撃
    KMM_Attack( kamome );

    // 終了
    if ( think->nCount3 >= COUNT_VMODE(60) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    think->nCount3++;
}
// 飛び立ち
static void Think3_Takeoff(KAMOME* kamome )
{
    FVECTOR   vec;
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	// 動作設定
	kamome->action.pad = PAD_TAKEOFF;	// 緊急飛び立ち

	kamome->fSpeed = 60.f / kmmng->fBaseSpeed;
    }

    if ( (kamome->action.current_mot == KMM_MOT_IDLE) ||
	 (kamome->action.current_mot >= KMM_MOT_WALK_START && kamome->action.current_mot <= KMM_MOT_SURPRISE_BACK) || 
	 (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time < COUNT_VMODE(30)) ||
	 (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time < COUNT_VMODE(69)) ){
	// モーション移動にまかせる
//printf("モーション移動 %d ::: %f\n", kamome->action.current_mot, kamome->mmt_ctrl.m_time);
    }else{
//printf("計算移動       %d ::: %f\n", kamome->action.current_mot, kamome->mmt_ctrl.m_time);
	// 上昇方向
	if ( (kamome->action.current_mot == KMM_MOT_FLYAWAY_QUICK && kamome->mmt_ctrl.m_time == COUNT_VMODE(30)) ||
	     (kamome->action.current_mot == KMM_MOT_FLYAWAY && kamome->mmt_ctrl.m_time == COUNT_VMODE(69)) ){
	    if ( kmmng->bPigeon ){
		kamome->turn.vy += KMM_GetDemoRandom( kmmng, -96, 96);
		kamome->turn.vx -= KMM_GetDemoRandom( kmmng, 100, 412);
	    }else{
		kamome->turn.vy += KMM_GetDemoRandom( kmmng, -96, 96);
		kamome->turn.vx -= KMM_GetDemoRandom( kmmng, 400, 712);
	    }
	}
	// 移動量を設定->座標更新
	_sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vec);
    }

    // 終了処理
    if ( kamome->action.act_end ){
	KMM_SetThink3( kamome, TH3_FLYING);
	// 飛行モーション
	kamome->action.pad = PAD_FLYING;

	return;
    }

    think->nCount3++;
}
// 緊急回避
static void Think3_Evade(KAMOME* kamome )
{
    FVECTOR   vec;
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	// 動作設定
	kamome->action.pad = PAD_TAKEOFF;	// 緊急飛び立ち

        kamome->turn.vy += KMM_GetDemoRandom( kmmng, -96, 96);
        kamome->turn.vx -= KMM_GetDemoRandom( kmmng, 400, 512);

	kamome->fSpeed = 65.f / kmmng->fBaseSpeed;
    }
   
    // 移動量を設定->座標更新
    _sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], kamome->fSpeed * kmmng->fBaseSpeed);
    _sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vec);

    // 終了処理
    if ( kamome->action.act_end ){
	KMM_SetThink3( kamome, TH3_FLYING);
	// 飛行モーション
	kamome->action.pad = PAD_FLYING;

	return;
    }

    think->nCount3++;
}

// キャラに付随するまでの移動
static void Think3_ConcMove( KAMOME* kamome )
{
    KMMTHINK* think;
    CONTROL*	pctrl;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;
    pctrl = kamome->pctrlAim; // NULLチェックは入る前にしている

    if ( pctrl == NULL ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov; 	// 始点
	kamome->vecAimPos.vx = pctrl->mov.vx;	// 終点
	kamome->vecAimPos.vy = pctrl->mov.vy + 5000.f;	
	kamome->vecAimPos.vz = pctrl->mov.vz;	

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_CONCOMITANT);
	    return;
	}
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // エルミート補間で移動
    if (KMM_LandMove( kamome )){
	KMM_SetThink3( kamome, TH3_CONCOMITANT);
	return;
    }

    think->nCount3++;
}

// キャラに付随する
static void Think3_Concomitant( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;


    // 初期設定
    if ( think->nCount3 == 0 ){

    }

    // 攻撃対象がいない
    if ( kamome->pctrlAim == NULL ||
	 kamome->ptrgAim  == NULL ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 飛行モーション
    kamome->action.pad = PAD_HOVERING;	// 滑空

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 付随移動
    if ( KMM_ConcomitantMove( kamome ) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    think->nCount3++;

}

// カメラ方向で固定ホバリング
static void Think3_CamZHover( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;


    // 初期設定
    if ( think->nCount3 == 0 ){

    }

    // 飛行モーション
    kamome->action.pad = PAD_HOVERING;	// 滑空

    // プレイヤーが主観でなくなったら止める
    if ( !(GM_PlayerStatus & PLAYER_WATCH) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 付随移動
    if ( KMM_CameraZMove( kamome ) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    think->nCount3++;

}

// 段ボール乗り
static void Think3_CBoxRide( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // プレイヤーがダンボールモードでなくなったらもしくはZOEダンボールorステルス迷彩を装備したら止める
    if ( !(kmmng->nInfo & KMM_INFO_PL_CBOX) || (kmmng->nInfo & KMM_INFO_PL_EYE_CBOX) ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->vecStartPos = kamome->mov; 	// 始点

	_sceVu0CopyMatrix( &kamome->matOrg, &GM_PlayerBoxBody->objs->objs[0].world);
	MAO_AbsVec( &kamome->vecAimPos, &kamome->vecLand, &kamome->matOrg);
	_sceVu0AddVector(&kamome->vecAimPos, &kamome->vecAimPos, (FVECTOR*)&kamome->matOrg.m[3][0]);
	kamome->vecAimPos.vy += KAMOME_Y_OFFSET;

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_CBOX_LANDING);
	    return;
	}
	kamome->nRouteCntr = 0;
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }

    // ハザードチェック
    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &kamome->vecMove, HZX_CHK_RECOIL_TYPE_ONLY, 0, 0 ) != 0 ){
	FVECTOR* vec;
	HZX_GetOnlinePoint( &kamome->vecMove );
	_sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], -10.f ); // 後方に引っ張る
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vec);
	KMM_SetThink3( kamome, TH3_BACKOFF );
	return;
    }

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;	// 滑空

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 付随移動
    if ( KMM_BoxRide( kamome ) ){
	KMM_SetThink3( kamome, TH3_CBOX_LANDING); // 着地動作へ
	return;
    }

    think->nCount3++;

}
// エルード妨害移動中
static void Think3_EludeMove( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nRouteCntr = 0;
	kamome->vecStartPos = kamome->mov; 	// 始点
	kamome->vecAimPos   = kamome->vecLand;	// 終点

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_ELUDELAND);
	    return;
	}
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    { // 接近しすぎて飛び立つ
	FVECTOR vecFrom, vecTo;
	vecFrom.vx = kamome->mov.vx - kmmng->nEludeDist;
	vecFrom.vy = kamome->mov.vy - kmmng->nEludeDist;
	vecFrom.vz = kamome->mov.vz - kmmng->nEludeDist;
	vecTo.vx   = kamome->mov.vx + kmmng->nEludeDist;
	vecTo.vy   = kamome->mov.vy + kmmng->nEludeDist;
	vecTo.vz   = kamome->mov.vz + kmmng->nEludeDist;

	if ( MAO_BoundCheck( &kmmng->vecElude, &vecFrom, &vecTo ) ){
	    KMM_SetThink3( kamome, TH3_TAKEOFF); // 緊急飛び立ち
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}
    }
    // エルミート補間で移動
    if (KMM_LandMove( kamome )){
	KMM_SetThink3( kamome, TH3_ELUDELAND);
	return;
    }

    think->nCount3++;
}
// エルード妨害中
static void Think3_Eluding( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( think->nCount3 == 0 ){
	
    }

    kamome->action.pad = PAD_IDLE; // 待機

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    { // 接近しすぎて飛び立つ
	FVECTOR vecFrom, vecTo;
	vecFrom.vx = kamome->mov.vx - kmmng->nEludeDist;
	vecFrom.vy = kamome->mov.vy - kmmng->nEludeDist;
	vecFrom.vz = kamome->mov.vz - kmmng->nEludeDist;
	vecTo.vx   = kamome->mov.vx + kmmng->nEludeDist;
	vecTo.vy   = kamome->mov.vy + kmmng->nEludeDist;
	vecTo.vz   = kamome->mov.vz + kmmng->nEludeDist;

	if ( MAO_BoundCheck( &kmmng->vecElude, &vecFrom, &vecTo ) ){
	    KMM_SetThink3( kamome, TH3_TAKEOFF); // 緊急飛び立ち
	    kamome->nRestTime = -1; // 無効にする
	    return;
	}
    }
    if ( kamome->nRestTime > 0 && think->nCount3 >= kamome->nRestTime){ // 時間指定あり
	KMM_SetThink3( kamome, TH3_FLYING); // 飛行
	kamome->nRestTime = -1; // 無効にする
	return;
    }
    
    think->nCount3++;
}
// コントロールキャラに付随
static void Think3_CtrlMove( KAMOME* kamome )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    // 対象キャラが見つからない時
    if ( GM_SearchWhere(kamome->nAimName) == NULL ){
	KMM_SetThink3( kamome, TH3_FLYING);
	return;
    }

    // 初期設定
    if ( think->nCount3 == 0 ){
	_sceVu0CopyVector( &kamome->vecStartPos, &kamome->mov); 	// 始点
	_sceVu0CopyVector( &kamome->vecAimPos, &kamome->vecLand);	// 着地点

	// 距離取得
	MAO_GetDiffVec3( NULL, &kamome->nLandDist, NULL, &kamome->vecStartPos, &kamome->vecAimPos);
	if (kamome->nLandDist < KMM_DIV_DIST){ // 既に着地している
	    KMM_SetThink3( kamome, TH3_CTRL_LANDING);
	    return;
	}
	kamome->nRouteCntr = 0;
	kamome->nRouteTime = (int)(kamome->nLandDist / KMM_DIV_DIST);

	// 向き
	_sceVu0CopyVector( &kamome->vecStartZ, (FVECTOR*)&kamome->matOrg.m[2][0]);
	_sceVu0ScaleVector( &kamome->vecStartZ, &kamome->vecStartZ, kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0ScaleVector( &kamome->vecAimZ, &kamome->vecLandZ, kamome->fSpeed * kmmng->fBaseSpeed);
    }

    {
	FVECTOR 	vecPos;
	CONTROL* 	pctrl;
	OBJECT*	pbody;
	pctrl = GM_SearchWhere(kamome->nAimName);
	if ( pctrl == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}

	pbody = pctrl->object;
	if ( pbody == NULL ){
	    KMM_SetThink3( kamome, TH3_FLYING);
	    return;
	}
	_sceVu0ApplyMatrix( &vecPos, &BODYWORLD( pbody, kamome->nNode), &kamome->vecOffset );

	kamome->vecLand.vx = vecPos.vx;
	kamome->vecLand.vy = vecPos.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = vecPos.vz;
	kamome->vecLand.vw = 1.f;	

	_sceVu0CopyVector( &kamome->vecAimPos, &kamome->vecLand);	// 着地点
    }

    // ハザードチェック
    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &kamome->vecMove, HZX_CHK_RECOIL_TYPE_ONLY, 0, 0 ) != 0 ){
	FVECTOR* vec;
	HZX_GetOnlinePoint( &kamome->vecMove );
	_sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], -10.f ); // 後方に引っ張る
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vec);
	KMM_SetThink3( kamome, TH3_BACKOFF );
	return;
    }

    // 飛行モーション
    kamome->action.pad = PAD_FLYING;	// 滑空

    // 遅延思考切替え
    if ( KMM_SetThink3FromDelay( kamome )){
	return;
    }

    // 付随移動
    if ( KMM_EulerLand( kamome ) ){
	KMM_SetThink3( kamome, TH3_CTRL_LANDING); // 着地動作へ
	return;
    }

    think->nCount3++;

}

// ダメージ硬直
static void Think3_DamageWait( KAMOME* kamome)
{
    KMMTHINK* think;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->action.pad = PAD_DAMAGE;	// 落下動作
    }

    if ( kamome->action.act_end ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLYING);
	return;
    }

    think->nCount3++;
}

// 後ろ移動
static void Think3_Backoff( KAMOME* kamome)
{
    FVECTOR vec;
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    think = &kamome->think;
    kmmng = (KAMOME_MNG*)kamome->pvManager;

    // 初期設定
    if ( think->nCount3 == 0 ){
	think->nThinkTime = KMM_GetDemoRandom( kmmng, 60, 90);

	kamome->turn.vx -= 256;
	_sceVu0CopyVector( &kamome->vecAimZ, (FVECTOR*)&kamome->matOrg.m[1][0]);
	kamome->fSpeed = 50.f / kmmng->fBaseSpeed;
    }

    kamome->action.pad = PAD_CAUTION;	// ばたばた

    if ( kamome->fSpeed > 0.05f ) kamome->fSpeed -= 0.005f;

    // 移動量を設定->座標更新
    _sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->vecAimZ, kamome->fSpeed * kmmng->fBaseSpeed);
    _sceVu0AddVector( &kamome->vecMove, &kamome->mov, &vec);
    _sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], -kamome->fSpeed * 0.5f * kmmng->fBaseSpeed); // 後方
    _sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vec);

    if ( HZX_OnlineHazardCheck( kamome->hzx_id, &kamome->mov, &kamome->vecMove, HZX_CHK_RECOIL_TYPE_ONLY, 0, 0 ) != 0 ){
	HZX_GetOnlinePoint( &kamome->vecMove );
	_sceVu0ScaleVector( &vec, (FVECTOR*)&kamome->matOrg.m[2][0], -10.f ); // 後方に引っ張る
	_sceVu0AddVector( &kamome->vecMove, &kamome->vecMove, &vec);
    }

    if ( kamome->action.act_end ){ // 再生終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLYING);
        kamome->action.pad = PAD_FLYING;	// 飛行
	kamome->fSpeed = 80.f / kmmng->fBaseSpeed;
	return;
    }

    think->nCount3++;
}

// 落下
static void Think3_DamageFall( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	FVECTOR vecSpd;

	_sceVu0ScaleVector( &vecSpd, (FVECTOR*)&kamome->matOrg.m[2][0], kamome->fSpeed * kmmng->fBaseSpeed);
	_sceVu0CopyVector( &kamome->vecFallSpd, &vecSpd); // 移動量を記憶
	kamome->fSpeed = 1.f;
    }

    kamome->action.pad = PAD_FALL;	// 落下動作

    if ( KMM_FallMove( kamome ) ){ // 落下処理
	KMM_SetThink3( kamome, TH3_DEAD); // 死亡
	kamome->vecLand.vx = kamome->mov.vx;
	kamome->vecLand.vy = kamome->mov.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = kamome->mov.vz;
	kamome->vecLand.vw = 1.f;	
	return;
    }

    think->nCount3++;
}
// 睡眠落下
static void Think3_DamageSleepFall( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	_sceVu0CopyVector( &kamome->vecFallSpd, &kamome->step); // 移動量を記憶
    }

    kamome->action.pad = PAD_FALL;	// 落下動作

    if ( KMM_FallMove( kamome ) ){ // 落下処理
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_GROUNDSLEEP); // 地面で睡眠
	kamome->vecLand.vx = kamome->mov.vx;
	kamome->vecLand.vy = kamome->mov.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = kamome->mov.vz;
	kamome->vecLand.vw = 1.f;	
	return;
    }

    think->nCount3++;
}
// 地面で睡眠
static void Think3_DamageGroundSleep( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間
	KMM_CallHeadMark( kamome, KMM_HEADMARK_ZZZ);
    }

    kamome->action.pad = PAD_GROUNDSLEEP;	// 地面で睡眠

    if ( think->nCount3 % COUNT_VMODE(10) == 0 && KMM_ChkDownFloorCheck( kamome) ){ // 床チェック
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_SLEEPFALL); // 睡眠落下
	KMM_EraseHeadMark( kamome);
	return;
    }

    if ( think->nCount3 >= kamome->nSleep ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_WAKEUP); // 起きる
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}
// 立ったまま睡眠
static void Think3_DamageStandSleep( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間
	KMM_CallHeadMark( kamome, KMM_HEADMARK_ZZZ);
    }

    kamome->action.pad = PAD_STANDSLEEP;	// たったままで睡眠

    if ( think->nCount3 % COUNT_VMODE(10) == 0 && KMM_ChkDownFloorCheck( kamome) ){ // 床チェック
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_SLEEPFALL); // 睡眠落下
	KMM_EraseHeadMark( kamome);
	return;
    }
    if ( think->nCount3 >= kamome->nSleep ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_STANDWAKEUP); // 起きる
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}

// 起きる
static void Think3_Wakeup( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->action.pad = PAD_WAKEUP;	// たったままで睡眠
    }


    if ( kamome->action.act_end ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLYING); // 起きる
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}
// 起きる
static void Think3_StandWakeup( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_SLEEP_MIN, KMM_SLEEP_MAX); // 睡眠時間
	KMM_CallHeadMark( kamome, KMM_HEADMARK_ZZZ);
    }

    kamome->action.pad = PAD_STANDSLEEP;	// たったままで睡眠

    if ( think->nCount3 >= kamome->nSleep ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_TAKEOFF); // 飛び立つ
//	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLYING); // 起きる
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}
// 気絶落下
static void Think3_DamageFeintFall( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	_sceVu0CopyVector( &kamome->vecFallSpd, &kamome->step); // 移動量を記憶
    }

    kamome->action.pad = PAD_FALL;	// 落下動作

    if ( KMM_FallMove( kamome ) ){ // 落下処理
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FEINT); // 気絶
	kamome->vecLand.vx = kamome->mov.vx;
	kamome->vecLand.vy = kamome->mov.vy + KAMOME_Y_OFFSET;
	kamome->vecLand.vz = kamome->mov.vz;
	kamome->vecLand.vw = 1.f;	
	return;
    }

    think->nCount3++;
}
// 地面で気絶
static void Think3_DamageFeint( KAMOME* kamome)
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->nSleep = KMM_GetDemoRandom( kmmng, KMM_FEINT_MIN, KMM_FEINT_MAX); // 睡眠時間
	KMM_CallHeadMark( kamome, KMM_HEADMARK_PIYO);
    }

    kamome->action.pad = PAD_FEINT;	// 気絶

    if ( think->nCount3 % COUNT_VMODE(10) == 0 && KMM_ChkDownFloorCheck( kamome) ){ // 床チェック
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FEINTFALL); // 気絶落下
	KMM_EraseHeadMark( kamome);
	return;
    }

    if ( think->nCount3 >= kamome->nSleep ){ // 終了
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_WAKEUP); // 起きる
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}

// 死亡
static void Think3_DamageDead( KAMOME* kamome)
{
    KMMTHINK* think;
    think = &kamome->think;

    // 初期設定
    if ( think->nCount3 == 0 ){
	kamome->turn.vx = kamome->rot.vx = 0;
    }

    kamome->action.pad = PAD_DEAD;	// 死亡

     //AndyO: Removed in MGS_2_and_3_Trophy_Info_v2_110527_rev.xls
    //BP_TrophySystem_UnlockTrophy( kTRP_KillSeagull );

    if ( think->nCount3 % COUNT_VMODE(10) == 0 && KMM_ChkDownFloorCheck( kamome) ){ // 床チェック
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FALL); // 落下
	KMM_EraseHeadMark( kamome);
	return;
    }

    think->nCount3++;
}

// 思考
static void Think2_Normal( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;

    switch ( think->nThink3 ){
    case TH3_REST:		// 羽休め
	Think3_Rest( kamome);
	break;
    case TH3_RESTGROUND:	// 地面で羽休め
	Think3_RestGround( kamome);
	break;
    case TH3_FLYING:		// 飛行
	Think3_Flying( kamome);
	break;
    case TH3_HOVERING:		// 滑空
	Think3_Hovering( kamome);
	break;
    case TH3_LANDMOVE:		// 着地移動
	Think3_LandingMove( kamome );
	break;
    case TH3_LANDING:		// 着地
	Think3_Landing( kamome);
	break;
    case TH3_FLIPMOVE:		// 集合移動
	Think3_FlipMove( kamome);
	break;
    case TH3_FLIPFLOP:		// ばたつき
	Think3_Flipflop( kamome);
	break;
    case TH3_BACKWARD:		// 後ろ移動
	Think3_Backward( kamome);
	break;
    case TH3_ATCMOVE:		// 攻撃移動
	Think3_AttackMove( kamome);
	break;
    case TH3_ATTACK:		// 攻撃
	Think3_Attack( kamome);
	break;
    case TH3_TAKEOFF:		// 緊急飛び立ち
	Think3_Takeoff( kamome);
	break;
    case TH3_EVADE:		// 回避
	Think3_Evade( kamome);
	break;
    case TH3_BACKOFF:		// 後ろへ移動
	Think3_Backoff( kamome);
	break;
    case TH3_CONCMOVE:		// キャラに付随して移動するまでの移動
	Think3_ConcMove( kamome);
	break;
    case TH3_CONCOMITANT:	// キャラに付随して移動
	Think3_Concomitant( kamome);
	break;
    case TH3_CAMZ_FIX:		// カメラ方向で固定ホバリング
	Think3_CamZHover( kamome);
	break;
    case TH3_CBOX_LAND:		// 段ボール着地移動
	Think3_CBoxRide( kamome);
	break;
    case TH3_CBOX_LANDING:	// 段ボール着地
	Think3_Landing( kamome);
	break;
    case TH3_CBOX_REST:		// 段ボール羽休め
	Think3_Rest( kamome);
	break;
    case TH3_ELUDEMOVE:		// エルード妨害移動中
	Think3_EludeMove( kamome);
	break;
    case TH3_ELUDELAND:		// エルード妨害着地中
	Think3_Landing( kamome);
	break;
    case TH3_ELUDING:		// エルード中
	Think3_Eluding( kamome);
	break;
    case TH3_CTRL_LAND:		// コントロール着地移動
	Think3_CtrlMove( kamome);
	break;
    case TH3_CTRL_LANDING:	// コントロール着地
	Think3_Landing( kamome);
	break;
    case TH3_CTRL_REST:		// コントロール羽休め
	Think3_Rest( kamome);
	break;
    case TH3_WAKEUP:		// 起きる
	Think3_Wakeup( kamome);
	break;
    case TH3_STANDWAKEUP:	// 立ち状態で起きる
	Think3_StandWakeup( kamome);
	break;
    case TH3_FALL:		// 落下 : デモ用
	Think3_DamageFall( kamome);
	break;	
    case TH3_SLEEPFALL:		// 睡眠落下
	Think3_DamageSleepFall( kamome);
	break;	
    case TH3_GROUNDSLEEP:	// 地面で睡眠
	Think3_DamageGroundSleep( kamome);
	break;	
    case TH3_STANDSLEEP:	// たったまま睡眠
	Think3_DamageStandSleep( kamome);
	break;	
    case TH3_FEINTFALL:		// 気絶落下
	Think3_DamageFeintFall( kamome);
	break;	
    case TH3_FEINT:		// 気絶
	Think3_DamageFeint( kamome);
	break;	
    case TH3_DEAD:		// 死亡 : デモ用
	Think3_DamageDead( kamome);
	break;	
    }
}

// ダメージ思考
static void Think2_Damage( KAMOME* kamome )
{
    KMMTHINK* think;
    think = &kamome->think;

    switch ( think->nThink3 ){
    case TH3_WAIT:		// 待機
	Think3_DamageWait( kamome);
	break;
    case TH3_FALL:		// 落下
	Think3_DamageFall( kamome);
	break;	
    case TH3_SLEEPFALL:		// 睡眠落下
	Think3_DamageSleepFall( kamome);
	break;	
    case TH3_GROUNDSLEEP:	// 地面で睡眠
	Think3_DamageGroundSleep( kamome);
	break;	
    case TH3_STANDSLEEP:	// たったまま睡眠
	Think3_DamageStandSleep( kamome);
	break;	
    case TH3_FEINTFALL:		// 気絶落下
	Think3_DamageFeintFall( kamome);
	break;	
    case TH3_FEINT:		// 気絶
	Think3_DamageFeint( kamome);
	break;	
    case TH3_DEAD:		// 死亡
	Think3_DamageDead( kamome);
	break;	
    }
}
// 思考
static void Think1_Normal( KAMOME* kamome )
{
    if ( kamome->think.nThink3 < TH3_CHECK_DAMAGE_START && DamageCheck( kamome ) ) return ;

    Think2_Normal( kamome );

}
// ダメージ思考
static void Think1_Damage( KAMOME* kamome )
{
    Think2_Damage( kamome );

}
// 思考
static void Think( KAMOME* kamome )
{
    KMMTHINK* think;

    think = &kamome->think;

    kamome->nFlag = 0;

    switch ( think->nThink1 ){
    case TH1_NORMAL:		// 通常
	Think1_Normal( kamome );
	break;
    case TH1_DAMAGE:		// ダメージ
	Think1_Damage( kamome );
	break;
    }

}

// 思考初期化
static void KMM_InitThink( KAMOME* kamome, int nInitThink )
{
    KMMTHINK* think;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    think = &kamome->think;

    kamome->fSpeed = KMM_SPEED_DEF / kmmng->fBaseSpeed;

    switch ( nInitThink ){
    case KMM_INITTHK_REST:		// 羽休め
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_REST);
	break;
    case KMM_INITTHK_FLYAWAY:		// 飛び立ち
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_TAKEOFF);
	break;
    case KMM_INITTHK_FLYING:		// 飛行
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLYING);
	break;
    case KMM_INITTHK_FLIPFLOP:		// ばたばた
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_FLIPFLOP);
	break;
    case KMM_INITTHK_GROUNDREST:	// 地面で羽休め
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_RESTGROUND);
	break;
    default: // 止まりからスタート
	KMM_SetThink1( kamome, TH1_NORMAL, TH2_NORMAL, TH3_REST);
    }

}
