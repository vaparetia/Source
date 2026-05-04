/*
    fateye.c
    ファトマン視界情報
    2001/03/14 Masafumi Okuta 
    $Id: fateye.c,v 1.1.1.3 2002/11/19 11:47:58 Yoshizawa1 Exp $
*/
#define DEF_EYE_DIST (25000)	// 視界距離
#define DEF_EYE_ANG  (0.6f) 	// 視野角度 : 内積値

static void FAT_InitEyeWork( Work* work)
{
    work->nDist 	= DEF_EYE_DIST;
    work->fEyeAng  	= DEF_EYE_ANG;
    work->nLostTrgCntr  = 0;
}

// プレイヤーが視界内にいるか判定
// 1 : いた
// 0 : いない
static int FAT_CheckEye( Work* work )
{
#if 1
    return ( ( work->pl_parts != 0) ? 1 : 0);
#else
    NPCWORK*	npc;
    npc = &work->npc;

    // 距離判定
    if ( work->pl_dis > work->nDist ){
	return 0;
    }

    // 方向判定
    if ( _sceVu0InnerProduct( &work->vecFatDir, &work->vecPlaDiff) < work->fEyeAng ){
	return 0;
    }

    // 高さは省略

    // 障害物検知
    if ( ENE_EyeOnlineCheck( npc->ctrl->hzx_id, npc->ctrl->mov, GM_PlayerPosition ) ){
	return 0;
    }

    return 1;	// 見つけた
#endif
}
// 目標地点がみえるかどうか
static int FAT_CheckAimVisible( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 距離判定
    if ( work->aim_dis > work->nDist ){
	return 0;
    }

    // 方向判定
    if ( _sceVu0InnerProduct( &work->vecFatDir, &work->vecAimDiff) < work->fEyeAng ){
	return 0;
    }

    // 高さは省略

    // 障害物検知
    if ( ENE_EyeOnlineCheck( npc->ctrl->hzx_id, &npc->ctrl->mov, &work->vecAimPos ) ){
	return 0;
    }

    return 1;	// 見つけた
}

// 物音に反応する判定
// 1 : 反能
// 0 : なにもない
static int FAT_CheckNoise( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // PL_GetPlayerWeapon()	// プレイヤーの装備している武器

    if (work->nFatInfo & FAT_INFO_SHOOT){	// 銃声
	work->vecAimPos = GM_PlayerPosition;
	work->nLostTrgCntr = 0;
	return 1;
    }
    
    return 0;
}

// ファットマン情報
static void CheckFatmanInfo( Work* work )
{
    float fLifeRate;
    FVECTOR	vec;
    NPCWORK*	npc;
    NPCACT*	act;

    npc = &work->npc;
    act = &work->npc.action;
    work->nFatPrevInfo  = work->nFatInfo;	// 前の情報を取得
    work->nFatInfo 	= 0x00000000; 		// 情報フラグ初期化
    _sceVu0CopyVector( &work->vecFatDir, (FVECTOR *)&BODYWORLD( &work->body, 0).m[2][0]); // ファットマンの向き

    // 特定パーツ毎の可視判定
    vec.vx = npc->ctrl->mov.vx;
    vec.vy = BODYWORLD( npc->body, HUMAN21_ATAMA).m[3][1];
    vec.vz = npc->ctrl->mov.vz;
    work->pl_parts = CheckPointLookableObject( &vec, GM_PlayerBody, npc->ctrl->hzx_id, 
					       npc->action.face_dir, VIEW_ANGLE); 
    vec.vx = GM_PlayerPosition.vx;
    vec.vy = BODYWORLD( GM_PlayerBody, HUMAN21_ATAMA).m[3][1];
    vec.vz = GM_PlayerPosition.vz;
    work->fat_parts = CheckPointLookableObject( &vec, npc->body, GM_PlayerControl->hzx_id, 
						GM_PlayerControl->rot.vy, VIEW_ANGLE); 

    // ロックオンされた
    if ( (GM_PlayerStatus & PLAYER_HOLD) && GV_DiffDirAbs( work->pl_dir, work->npc.ctrl->rot.vy) < 256){	
	work->nFatInfo |= FAT_INFO_LOCKED;	
    }

    // エルード (ぶらさがっているときだけ)
    if ( (GM_PlayerStatus & PLAYER_BEYOND) && !(GM_PlayerStatus & PLAYER_FORCE)){	
	if ( work->pl_dis < FAT_ELUDE_DIV_DIST)	work->nFatInfo |= FAT_INFO_ELUDE_NEAR;	
	else					work->nFatInfo |= FAT_INFO_ELUDE_FAR;	
    }

    // プレイヤー死亡
    if ( GM_PlayerStatus & PLAYER_DEAD ){
	work->nFatInfo |= FAT_INFO_PL_DEAD;
    }
    // ブラストスーツ狙撃直後
    if ( work->nBodyDmgTime < BODY_CAUT_TIME){
	work->nFatInfo |= FAT_INFO_BODY_CAUTION;	
	work->nBodyDmgTime++;
    }
    // 頭部狙撃直後
    if ( work->nHeadDmgTime < HEAD_CAUT_TIME){
	work->nFatInfo |= FAT_INFO_HEAD_CAUTION;	
	work->nHeadDmgTime++;
    }
    // ローラーブレード狙撃直後
    if ( work->nRollDmgTime < ROLL_CAUT_TIME){
	work->nFatInfo |= FAT_INFO_ROLL_CAUTION;	
	work->nRollDmgTime++;
    }

    // 物音
    if ( FAT_CheckNoise(work) )	work->nFatInfo |= FAT_INFO_NOISE;	

    // 体力割合
    if ( act->life < work->m9_faint) fLifeRate = (float)act->life / (float)work->nVitality;
    else			     fLifeRate = (float)work->m9_faint / (float)work->nVitality;
    if ( fLifeRate > 0.75f ) 	 	work->nFatInfo |= FAT_INFO_FINE;	// 正常
    else if ( fLifeRate > 0.5f ) 	work->nFatInfo |= FAT_INFO_WOUND;	// 軽傷
    else if ( fLifeRate > 0.25f )	work->nFatInfo |= FAT_INFO_BAD;		// 重傷
    else				work->nFatInfo |= FAT_INFO_DYING;	// 死にかけ

    // 麻酔
    if ( work->nNarcTime < NARC_CAUT_TIME ){
	work->nFatInfo |= FAT_INFO_NARC;	
	work->nNarcTime++;
    }

    // 痺れ
    if ( work->nStunTime < STUN_CAUT_TIME ){
	work->nFatInfo |= FAT_INFO_STUN;	
	work->nStunTime++;
    }

    // 攻撃した
    if ( work->nAttackTime < ATTACK_CAUT_TIME){
	work->nFatInfo |= FAT_INFO_ATTACK;	
	work->nAttackTime++;
    }

    // 発射チェック
    if ( GM_WeaponFire == WP_m92 || GM_WeaponFire == WP_Socom ){
	work->nFatInfo |= FAT_INFO_SHOOT;	
    }

    // ダウンチェック
    if ( act->current_mot == FAT_MOT_DOWN){
	work->nFatInfo |= FAT_INFO_DOWN;
    }

    // プレイヤー捕捉
    if ( FAT_CheckEye(work) ){	
	work->nFatInfo |= FAT_INFO_SEE_PL;	
    }

    // 目標地点捕捉
    if ( FAT_CheckAimVisible(work) ){	
	work->nFatInfo |= FAT_INFO_SEE_AIM;	
    }
    
    // 警戒地点捕捉
    if ( FAT_CheckAimVisible(work) ){	
	work->nFatInfo |= FAT_INFO_SEE_CAUTION;	
    }
}



