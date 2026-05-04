/*
    preznikita.c
    大統領ニキータ関連処理
    2001/04/24 Masafumi Okuta
    $Id: preznikita.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/
// ニキータ情報更新
#define NIK_NEAR_DIST 		(4500)	// 絶対反応
#define NIK_NEAR_DIST_LISTEN 	(2500)	// 聞き耳時絶対反応
#define NIKITA_OUTAREA_X	(-2251.f)
#define NIKITA_OUTAREA_Z	(-241034.f)
static void UpdateNikitaInfo(Work* work)
{
    int 	nNearDist = NIK_NEAR_DIST_LISTEN;
    NPCWORK* 	npc;

    npc = &work->npc;

    // ゲームシステム側から情報取得
    work->bNikita 	= GM_NikitaAlive[0];
    work->vecNikita 	= GM_NikitaPosition[0];
    // 差分データを生成
    MAO_GetDiffVec3( &work->vecDiffNik, &work->nNikDist, &work->nNikDir, &npc->ctrl->mov, &work->vecNikita);

    // 絶対発見距離の設定
    if ( work->think3 == TH3_LISTEN || work->think3 == TH3_NOTICE ){ // 聞き耳時
	switch ( GM_GameLevel ) {
	case GM_LEVEL_VERYEASY:
	case GM_LEVEL_EASY:
	case GM_LEVEL_NORMAL:
	    nNearDist = NIK_NEAR_DIST_LISTEN;
	    break;
	case GM_LEVEL_HARD:
	case GM_LEVEL_EXTREME:
	case GM_LEVEL_E_EXTREME:
	    nNearDist = NIK_NEAR_DIST;
	    break;
	default:
	    nNearDist = NIK_NEAR_DIST;
	}
    }else{
	nNearDist = NIK_NEAR_DIST;
    }

    // ニキータ発見チェック
    if ( work->bNikita == 1 && ( work->vecNikita.vx > NIKITA_OUTAREA_X || work->vecNikita.vz < NIKITA_OUTAREA_Z ) &&
	 ( work->nNikDist < nNearDist || GV_DiffDirAbs( npc->action.face_dir, work->nNikDir) < PREZ_VIEW_DIR ) &&
	 !ENE_EyeOnlineCheck( npc->ctrl->hzx_id, &work->vecFacePos, &work->vecNikita ) ){
	work->bSeeNikita = 1; 
    }else{
	work->bSeeNikita = 0;
    }
    // ニキータの弾が消えたら発見フラグをオフにする
    if ( work->bNikita != 1){
	work->bFindNikita = 0;
    }
}

// ニキータ発見に対する反応
// 思考更新が行なわれたら 1
// 何もなかったら 0 
static int FindNikita(Work* work)
{
    NPCWORK	*npc;

    npc = &work->npc;

    // 既に見つけていたら弾く
    if ( work->bFindNikita )
	return 0;

    // ニキータを見つけた
    if ( work->bSeeNikita ){
	work->bFindNikita = 1;
	if ( work->nNikDist >= PREZ_NIKITA_EVADE){ 	 // 通常反応
	    PREZ_SetThink2( work, TH2_PANIC, TH3_READY);	// パニック状態に
	    return 1;
	}else if ( work->nNikDist >= PREZ_NIKITA_EVADE){ // 回避
	    PREZ_SetThink2( work, TH2_PANIC, TH3_READY);	// 回避
	    return 1;
	}else{						 // 緊急回避
	    PREZ_SetThink2( work, TH2_PANIC, TH3_READY);	// 緊急回避
	    return 1;
	}
    }
    return 0;
}

