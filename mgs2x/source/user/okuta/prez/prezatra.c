/*
    prezatra.c
    大統領ニキータイベント平静時
    2001/04/25 Masafumi Okuta
    $Id: prezatra.c,v 1.1.1.3 2002/11/19 11:48:09 Yoshizawa1 Exp $
*/

#include "BP_Misc.h"

static void Think3_Stand( Work *work )
{
    // ニキータ発見
    if ( FindNikita( work) ){	
	return; 
    }
    // 音聞き
    if ( PREZ_CheckNoise(work) ){
	return;
    } 


    if ( work->count3 > COUNT_VMODE(300) ){
	PREZ_SetTrgpNextZone( work );
	return;
    }
    work->count3++;
}
#define POINT_MOVE_DIST (500)
static void Think3_Move( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	// 目標位置セット
        work->navitrg.pos  = work->vecNextPos;  

	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
    }

    // ニキータ発見
    if ( FindNikita( work) ){	
	return; 
    }

    npc->action.pad = PAD_MOVE;	// ポイント移動
    
    if ( PREZ_CheckNearZoneIn( npc->ctrl->addr, work->navitrg.addr) ){
	// 距離追跡
	if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	    PREZ_SetThink3FromNext( work, TH3_STAND);	// 立つ
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
// ルート移動待機
static void Think3_RouteStand( Work *work )
{

    if ( work->count3 == 0){
	work->npc.action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
    }
    
    // ニキータ発見
    if ( FindNikita( work) ){	
	return; 
    }
    // 音聞き
    if ( PREZ_CheckNoise(work) ){
	return;
    } 


    // 待機終了
    if ( work->count3 > work->nRouteWaitTime ){
	PREZ_SetTrgpNextZone( work );
//	PREZ_SetThink3( work, TH3_ROUTE_MOVE);
	return;
    }

    work->count3++;
}
// ルート移動
static void Think3_RouteMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	// 目標位置セット
        work->navitrg.pos  = work->vecNextPos;  

	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
    }

    npc->action.pad = PAD_MOVE;	// ポイント移動

    // ニキータ発見
    if ( FindNikita( work) ){	
	return; 
    }
    // 音反応
    if ( PREZ_CheckNoise(work) ){
	return;
    } 

    if ( PREZ_CheckNearZoneIn( npc->ctrl->addr, work->navitrg.addr) ){
	// 距離追跡
	if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	    PREZ_SetRouteWait(work);	// ルート待機設定
	    return;
	}
    }else{
	// ゾーン追跡
	GM_ZoneNavi( npc->navi, &work->navitrg);
    }

    if ( PL_CurrentWeapon() == WP_Mic && work->nComplaintCntr++ >= work->nComplaintMax ){
	PREZ_StreamRequest( work, PREZ_STRM_COMP01 + BP_PS2_rand() % 7); // 音声 : 愚痴
	work->nComplaintCntr = 0;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
// 聞き耳
static void Think3_Listen( Work *work )
{
    NPCWORK	*npc;

    npc = &work->npc;

    if ( work->count3 == 0 ) {
	FVECTOR vecPos;
	NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE ); // ？
	work->bMotEnd = 0;
	npc->action.pad = PAD_LISTEN;	// 聞き耳

	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
	vecPos.vx = 0.f;
	vecPos.vy = 0.f;
	vecPos.vz = 800.f;
	vecPos.vw = 1.f;
	DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot );
	DG_PutVector( &vecPos, &vecPos, 1);
	if ( HZX_OnlineHazardCheck( npc->ctrl->hzx_id, &npc->ctrl->mov, &vecPos, HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE, 0, 0 ) == 0 ){ // 障害物ない 
	    PREZ_SetThink3( work, TH3_NOTICE);	// 注目へ移行
	    return;
	}
    }

    if ( work->count3 == COUNT_VMODE(15) ) {
//	PREZ_Say( work, SE_PREKNOK1 + MAO_Rand(2), GM_SEMODE_BOMB); // 気合い
//	PREZ_StreamRequest( work, 0);
    }

    if ( FindNikita( work) ){	// ニキータ発見
	return; 
    }

    if ( PREZ_CheckNoiseInListen(work) ){    // 音反応
	return;
    } 

    if ( work->count3 >= COUNT_VMODE(work->nNoiseTime) ) {
	work->bMotEnd = 1; // 終了フラグ
    }

    if ( npc->action.act_end ) { // モーション側から終了許可が降りた
	PREZ_SetThink3( work, TH3_NOTICE);	// 注目へ移行
	return ;
    }

    work->count3++;
}

// 聞き耳を立てる
static void Think3_ListenWait( Work *work )
{
    NPCWORK	*npc;

    npc = &work->npc;

    if ( work->count3 == 0 ) {
	NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE ); // ？
	work->bMotEnd = 0;
	npc->action.pad = PAD_OVERLOOK;	// 見渡す
	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
	{ // 音のした方向を向く
	    int nDir; 
	    nDir = MAO_GetDirZoneToZone( HZX_Zone1( npc->ctrl->addr), work->nNextZone);
	    npc->ctrl->turn.vy = nDir;
	}
    }

    if ( work->count3 == COUNT_VMODE(15) ) {
//	PREZ_Say( work, SE_PREKNOK1 + MAO_Rand(2), GM_SEMODE_BOMB); // 気合い
//	PREZ_StreamRequest( work, 0);
    }

    if ( FindNikita( work) ){	// ニキータ発見
	return; 
    }

    if ( PREZ_CheckNoiseInListen(work) ){    // 音反応
	return;
    } 

    if ( work->count3 >= COUNT_VMODE(60) ) { // 終了
	PREZ_SetNextThink3( work, TH3_LISTEN);
	PREZ_SetThink3( work, TH3_MOVE);
	return ;
    }

    work->count3++;
}
// 注目
static void Think3_Notice( Work *work )
{
    NPCWORK	*npc;

    npc = &work->npc;

    if ( work->count3 == 0 ) {
	work->bMotEnd = 0;
	npc->action.pad = PAD_NOTICE;	// 聞き耳

	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
    }

    if ( FindNikita( work) ){	// ニキータ発見
	return; 
    }
    
    if ( work->count3 >= COUNT_VMODE(work->nNoticeTime) ) { // 時間チェック
	work->bMotEnd = 1; // 終了フラグ
    }

    if ( npc->action.act_end ) { // モーション側から終了許可が降りた
	PREZ_SetTrgpNextZone( work );	// ルートに戻る
	return ;
    }

    work->count3++;
}

// 居眠り
static void  Think3_Sleep( Work *work )
{
    NPCWORK	*npc;

    npc = &work->npc;

    if ( work->count3 == 0 ) {
	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
	npc->ctrl->r_sphere = 450;
	npc->ctrl->s_sphere = 500;
    }

    // 音反応:ねている時専用
    if ( PREZ_CheckNoiseInSleep(work) ){
	return;
    } 

    npc->action.pad = PAD_SLEEP;	// ねている

    work->count3++;
}

// 起きる
static void Think3_Awake( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
//	PREZ_Say( work, SE_HATENA, GM_SEMODE_BOMB); // 気合い
	npc->action.pad = PAD_AWAKE;	// 起きる

	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
    }

    if ( npc->action.act_end ) {
	if ( PREZ_GetArea( &work->vecLastBombPos) != -1 ){
	    work->vecNextPos = work->vecRelay;
	    work->nNextZone  = HZX_Zone1(HZX_GetAddress( npc->ctrl->hzx_id, &work->vecRelay, -1));
	    PREZ_SetNextThink3( work, TH3_OVERLOOK );
	    PREZ_SetThink3( work, TH3_MOVE );
	    return ;
	}else{
	    PREZ_SetThink3( work, TH3_STAND );
	    return ;
	}
    }

    work->count3++;
}

// きょろきょろ
#define OVERLOOK_VOIDED 	(30) 	// 見渡し開始までの隙
#define OVERLOOK_INTERVAL	(120) 	// 見渡し間隔
#define OVERLOOK_TIMEOUT	(230)	// 見渡し終了

#define OVERLOOL_ADJROT		(512)

static void Think3_Overlook( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0){
	work->bMotEnd = 0;
	npc->action.pad = PAD_OVERLOOK;	// 見渡す
	
	work->vecAdjust.vx = 0;
	work->vecAdjust.vz = 0;
	work->nHeadAdj = 0;
	work->nNoticeCntr = 0;

	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
    }

    // 振り向き
    if ( !work->bMotEnd && work->count3 > COUNT_VMODE(OVERLOOK_VOIDED)) {
	SVECTOR vecAim;
	vecAim.vx = 0;
	vecAim.vz = 0;
	switch ( work->nHeadAdj ){
	case 0:
	    if ( work->nHeadInterp < 8 ){
		vecAim.vy = -OVERLOOL_ADJROT;
      
      if ( BP_IsPAL()==TRUE )
   		GV_NearExp8PVPAL( &work->vecAdjust, &vecAim, 3 );
      else
		   GV_NearExp8PV( &work->vecAdjust, &vecAim, 3 );

      work->nHeadInterp++;
	    }else{
		if ( work->nNoticeCntr++ > COUNT_VMODE(60) ){
		    work->nHeadInterp = 0;
		    work->nHeadAdj    = 1;
		    work->nNoticeCntr = 0;
		}
	    }
	    break;
	case 1:
	    if ( work->nHeadInterp < 8 ){
		vecAim.vy = OVERLOOL_ADJROT;

      if ( BP_IsPAL()==TRUE )
   		GV_NearExp8PVPAL( &work->vecAdjust, &vecAim, 3 );
      else
         GV_NearExp8PV( &work->vecAdjust, &vecAim, 3 );

      work->nHeadInterp++;
	    }else{
		if ( work->nNoticeCntr++ > COUNT_VMODE(60) ){
		    work->nHeadInterp = 0;
		    work->nHeadAdj    = 2;
		    work->nNoticeCntr = 0;
		}
	    }
	    break;
	case 2:
	    if ( work->nHeadInterp < 8 ){
		vecAim.vy = 0;

      if ( BP_IsPAL()==TRUE )
   		GV_NearExp8PVPAL( &work->vecAdjust, &vecAim, 3 );
      else
		   GV_NearExp8PV( &work->vecAdjust, &vecAim, 3 );

      work->nHeadInterp++;
	    }else{
		work->vecAdjust.vy  = 0;
		work->bMotEnd = 1;
	    }
	    break;
	}
    }

    // ニキータ発見
    if ( FindNikita( work) ){	
	return; 
    }

    // 音聞き
    if ( PREZ_CheckNoise(work) ){
	return;
    } 
    
    if ( npc->action.act_end ) { // モーション側から終了許可が降りた
	PREZ_SetThink3FromNext( work, TH3_NOTICE); //
	return;
    }
    work->count3++;
}







