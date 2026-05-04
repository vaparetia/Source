/*
    fatthink.c
    ファットマン思考処理
    2001/03/23 Masafumi Okuta
    $Id: fatthink.c,v 1.1.1.3 2002/11/19 11:48:01 Yoshizawa1 Exp $
*/
#include "fatcommon.c"

/*----- Think2 --------------------------------------------------------*/
// 銃撃戦ルーチン
static void Think2_ShootPhase( Work *work )
{
    switch ( work->think3 ){
    case TH3_PHASE_START:	// フェーズ開始
	Think3_ShootStart( work );
	break;	
    case TH3_READY:		// 準備動作
	Think3_ShootReady( work );
	break;
    case TH3_STAND:		// 待機
	Think3_Stand( work );
	break;
    case TH3_MOVE:		// 移動
	Think3_Move( work );
	break;
    case TH3_TURN_BACK:		// 後ろを向く
    case TH3_TURN_FRONT:	// 前を向く
    case TH3_REVERSE:		// 移動
	Think3_Turn180( work);
	break;
    case TH3_STOP:		// 停止
	Think3_Stop( work );
	break;	
    case TH3_POINT_MOVE:	// 指定点に移動
	Think3_PointMove( work );
	break;
    case TH3_RELOAD:		// 装填
	Think3_Reload( work );
	break;
    case TH3_RUN_RELOAD:
	Think3_RunReload( work );
	break;
    case TH3_RUN_SHOOT:		// 走り撃ち
	Think3_RunShoot( work );
	break;
    case TH3_BACK_SHOOT:	// 逆走撃ち
	Think3_BackShoot( work );
	break;
    case TH3_FEINT_SHOOT:	// 牽制撃ち
	Think3_ShootFeint( work );
	break;
    case TH3_OVERLOOK:
	Think3_OverLook( work );
	break;	
    case TH3_BLEATHLESS:
	Think3_Bleathless( work );
	break;	
    case TH3_BODYATTACK:		// 体当り
	Think3_BodyAttack( work );			       
	break;
    case TH3_ELUDE_FALL:	// エルード落とし
	Think3_EludeFall( work );			       
	break;
    case TH3_ELUDE_KILL:	// エルード指きり
	Think3_EludeKill( work );			       
	break;
    case TH3_ELUDE_SHOOT:	// エルード狙撃
	Think3_EludeShoot( work );			       
	break;
    case TH3_STANDBY:		// 待機
	Think3_Standby( work );			       
	break;
    default:
	FAT_SetThink3( work, TH3_STAND );
#ifdef DEBUG_MODE
printf("think1 = %d : %d\n", work->nPrevThink1, work->think1);
printf("think2 = %d : %d\n", work->nPrevThink2, work->think2);
printf("think3 = %d : %d\n", work->nPrevThink3, work->think3);
printf("count3 = %d : %d\n", work->nPrevCntr3, work->count3);
printf("next3  = %d : %d\n", work->nPrevNextThink3, work->nNextThink3);
printf("!!!!!!!!!!!!!!Shoot phase : think3 error\n");
#endif	
    }

}
// 爆弾設置ルーチン
static void Think2_BombPhase( Work *work )
{
    switch ( work->think3 ){
    case TH3_PHASE_START:	// フェーズ開始
	Think3_BombStart( work );
	break;
    case TH3_READY:		// 準備
	Think3_BombReady( work );
	break;
    case TH3_STAND:		// 待機
	Think3_Stand( work );
	break;
    case TH3_MOVE:		// 移動
	Think3_Move( work );
	break;
    case TH3_TURN_BACK:		// 後ろを向く
    case TH3_TURN_FRONT:	// 前を向く
    case TH3_REVERSE:		// 移動
	Think3_Turn180( work);
	break;
    case TH3_STOP:		// 停止
	Think3_Stop( work );
	break;	
    case TH3_POINT_MOVE:	// 指定点に移動
	Think3_BombPointMove( work );
	break;
    case TH3_RELOAD:		// 装填
	Think3_Reload( work );
	break;
    case TH3_RUN_RELOAD:
	Think3_RunReload( work );
	break;
    case TH3_RUN_SHOOT:		// 走り撃ち
	Think3_RunShoot( work );
	break;
    case TH3_BACK_SHOOT:	// 逆走撃ち
	Think3_BackShoot( work );
	break;
    case TH3_FEINT_SHOOT:	// 牽制撃ち
	Think3_Feint( work );
	break;
    case TH3_BODYATTACK:		// 体当り
	Think3_BodyAttack( work );			       
	break;
    case TH3_RUN_PUT:		// 移動中に爆弾セット
	Think3_RunningPut( work );
	break;
    case TH3_STOP_PUT:		// 停止して爆弾セット
	Think3_StopPut( work );
	break;	
    case TH3_TURN_PUT:		// 回転置き
	Think3_TurnPut( work );
	break;
    case TH3_OVERLOOK:
	Think3_OverLook( work );
	break;	
    case TH3_BLEATHLESS:
	Think3_Bleathless( work );
	break;	
    case TH3_ELUDE_FALL:	// エルード落とし
	Think3_EludeFall( work );			       
	break;
    case TH3_ELUDE_KILL:	// エルード指きり
	Think3_EludeKill( work );			       
	break;
    case TH3_ELUDE_SHOOT:	// エルード狙撃
	Think3_EludeShoot( work );			       
	break;
    case TH3_STANDBY:		// 待機
	Think3_Standby( work );			       
	break;
    default:
	FAT_SetThink3( work, TH3_STAND );
#ifdef DEBUG_MODE
printf("think1 = %d : %d\n", work->nPrevThink1, work->think1);
printf("think2 = %d : %d\n", work->nPrevThink2, work->think2);
printf("think3 = %d : %d\n", work->nPrevThink3, work->think3);
printf("count3 = %d : %d\n", work->nPrevCntr3, work->count3);
printf("next3  = %d : %d\n", work->nPrevNextThink3, work->nNextThink3);
printf("!!!!!!!!!!!!!!Bomb phase : think3 error\n");
#endif	
    }

    // 設置点設定からの時間
    if ( work->bPutIndexUsed ){
	work->nPutIntvCntr = 0;
    }else{
	if ( work->nPutIntvCntr < 10000){
	    work->nPutIntvCntr++;
	}
    }
	
}
// 爆弾起動後(カウントダウン中)ルーチン
static void Think2_DisposalPhase( Work *work )
{
    switch ( work->think3 ){
    case TH3_PHASE_START:	// フェーズ開始
	Think3_CountDownStart( work );
	break;
    case TH3_READY:		// 警戒
	Think3_DemineReady( work );
	break;
    case TH3_STAND:		// 立ち
	Think3_Stand( work );
	break;
    case TH3_MOVE:		// 移動
	Think3_Move( work );
	break;
    case TH3_TURN_BACK:		// 後ろを向く
    case TH3_TURN_FRONT:	// 前を向く
    case TH3_REVERSE:		// 移動
	Think3_Turn180( work);
	break;
    case TH3_STOP:		// 停止
	Think3_Stop( work );
	break;	
    case TH3_POINT_MOVE:	// 指定点に移動
	Think3_PointMove( work );
	break;
    case TH3_RELOAD:		// 装填
	Think3_Reload( work );
	break;
    case TH3_RUN_RELOAD:
	Think3_RunReload( work );
	break;
    case TH3_RUN_SHOOT:		// 走り撃ち
	Think3_RunShoot( work );
	break;
    case TH3_BACK_SHOOT:	// 逆走撃ち
	Think3_BackShoot( work );
	break;
    case TH3_FEINT_SHOOT:	// 牽制撃ち
	Think3_ShootFeint( work );
	break;
    case TH3_BODYATTACK:	// 体当り
	Think3_BodyAttack( work );			       
	break;
    case TH3_OVERLOOK:
	Think3_OverLook( work );
	break;	
    case TH3_BLEATHLESS:
	Think3_Bleathless( work );
	break;	
    case TH3_COUNT_ATTRACT:	// 余興
	Think3_DisposalAttract( work );
	break;
    case TH3_COUNT_DANCE:	// 踊り
	Think3_DisposalDance( work );
	break;
    case TH3_COUNT_KAMOME:	// かもめ戯れ
	Think3_DisposalKamome( work );
	break;
    case TH3_COUNT_KAMOME2:	// かもめ戯れ２
	Think3_DisposalKamomeNeck( work );
	break;
    case TH3_COUNT_BACKNAVI:	// 背後ナビ
	Think3_DisposalBackNavi( work );
	break;
    case TH3_COUNT_BOMBGUARD:	// 爆弾守備
	Think3_CountDownBombGuard( work );			       
	break;
    case TH3_COUNT_MOVE:	// 移動
	Think3_Move( work );
	break;
    case TH3_COUNT_FIRST:	// 初回
	Think3_CountDownFirst( work );
	break;
    case TH3_ELUDE_FALL:	// エルード落とし
	Think3_EludeFall( work );			       
	break;
    case TH3_ELUDE_KILL:	// エルード指きり
	Think3_EludeKill( work );			       
	break;
    case TH3_ELUDE_SHOOT:	// エルード狙撃
	Think3_EludeShoot( work );			       
	break;
    case TH3_STANDBY:		// 待機
	Think3_Standby( work );			       
	break;
    default:
	FAT_SetThink3( work, TH3_STAND );
#ifdef DEBUG_MODE
printf("think1 = %d : %d\n", work->nPrevThink1, work->think1);
printf("think2 = %d : %d\n", work->nPrevThink2, work->think2);
printf("think3 = %d : %d\n", work->nPrevThink3, work->think3);

printf("count3 = %d : %d\n", work->nPrevCntr3, work->count3);
printf("next3  = %d : %d\n", work->nPrevNextThink3, work->nNextThink3);
printf("!!!!!!!!!!!!!!Disposal phase : think3 error\n");
#endif	
    }
    
    // Ｃ４処理ブランク
    if ( work->nPlDemineC4Time < COUNT_VMODE(100000) )work->nPlDemineC4Time++;
}

// ダメージフェーズ開始
static void Think2_StartDamage( Work *work )
{
    switch( work->think3 ) {
    case TH3_WAIT:
	Think3_StartDamage( work );
	break;
    case TH3_WAKEUP:
	Think3_WakeUp( work );
	break;
    default: ;
#ifdef DEBUG_MODE
printf("think1 = %d : %d\n", work->nPrevThink1, work->think1);
printf("think2 = %d : %d\n", work->nPrevThink2, work->think2);
printf("think3 = %d : %d\n", work->nPrevThink3, work->think3);
printf("count3 = %d : %d\n", work->nPrevCntr3, work->count3);
printf("next3  = %d : %d\n", work->nPrevNextThink3, work->nNextThink3);
printf("!!!!!!!!!!!!!!start damage phase : think3 error\n");
#endif	
    }
}
// NPCシステムのダメージ処理
static void Think2_SystemDamage( Work *work )
{
    switch( work->think3 ) {
    case TH3_WAIT:
	Think3_SysDamageWait( work ) ;
	break;
    case TH3_WAKEUP:
	Think3_WakeUp( work ) ;
	break;
    default: ;
#ifdef DEBUG_MODE
printf("think1 = %d : %d\n", work->nPrevThink1, work->think1);
printf("think2 = %d : %d\n", work->nPrevThink2, work->think2);
printf("think3 = %d : %d\n", work->nPrevThink3, work->think3);
printf("count3 = %d : %d\n", work->nPrevCntr3, work->count3);
printf("next3  = %d : %d\n", work->nPrevNextThink3, work->nNextThink3);
printf("!!!!!!!!!!!!!!system damage phase : think3 error\n");
#endif	
    }
}
// イベント
static void Think2_Event( Work *work )
{
    Think3_EventStand( work ) ;
}

/*----- Think1 --------------------------------------------------------*/
static int DamageCheck( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
	FAT_SetThink1( work, TH1_DAMAGE, TH2_START_DAMAGE, TH3_WAIT);
	return 1;
    }
    if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
	FAT_SetThink1( work, TH1_DAMAGE, TH2_SYSTEM_DAMAGE, TH3_WAIT);
	return 1;
    }
    return 0;
}

static void Think1_Normal( Work *work )
{
    if ( DamageCheck( work ) ) return ;

//Think2_Test( work);
//return;

    switch( work->think2 ) {
    case TH2_SHOOT_PHASE:		// 銃撃フェーズ
	Think2_ShootPhase( work );
	break;
    case TH2_BOMB_PHASE:		// 爆弾フェーズ
	Think2_BombPhase( work );
	break;
    case TH2_DISPOSAL_PHASE:		// 解体フェーズ
	Think2_DisposalPhase( work );
	break;
    }
}

static void Think1_Damage( Work *work )
{
    switch( work->think2 ) {
    case  TH2_START_DAMAGE :
	Think2_StartDamage( work ) ;
	break ;
    case  TH2_SYSTEM_DAMAGE :
	Think2_SystemDamage( work ) ;
	break ;
    }

}
// イベント
static void Think1_Event( Work *work )
{
    Think2_Event( work ) ;
}

// 思考処理メイン 
static void Think( Work *work )
{
#ifdef DEBUG_MODE

    if ( *dbgbomb_menu.target ){
	FAT_DbgBombThink(work);
	return;
    }
    if ( (*dbgthink_menu.target) == FATTHKDBG_ON){
	FAT_DbgThink(work); // デバック思考
	return;
    }

    if ( *dbgmot_menu.target ){
	return;
    }

#endif

    // ゲームクリア
    if ( work->bGameClear )
	return;

    switch( work->think1 ) { 
    case  TH1_NORMAL:	// 通常
	Think1_Normal( work );
	break;
    case  TH1_DAMAGE:	// ダメージ
	Think1_Damage( work );
	break;
    case  TH1_EVENT:	// イベント
	Think1_Event( work );
	break;
    }
}
/*------------------------------ Init -------------------------------*/
// 思考開始
static void StartThink( Work *work )
{
    NPC_SetCheckPad( &work->npc, SampleCheckPad );	  // 動作チェック関数
    NPC_SetCheckDamage( &work->npc, CheckDamageStand );   // ダメージチェック関数

    work->nPhase    		= TH2_BOMB_PHASE;
    work->nPrevPhase    	= TH2_BOMB_PHASE;
    work->nPhaseNum 		= 0;
    work->nPhaseStartLife 	= work->npc.action.life;
    work->nPhaseStartFaint 	= work->m9_faint;
    work->nLastBombLife		= work->npc.action.life;
    work->nLastBombFaint	= work->m9_faint;
    work->nNextZone 		= 0;
    work->nNextZone2Valid 	= 0;
    work->nNextZone2 		= 0;

    // ルーチン初期値
    work->nBeforeDmgThink3 = TH3_PHASE_START;
    FAT_SetNewC4Point(work);

    FAT_SetThink1( work, TH1_NORMAL, TH2_DISPOSAL_PHASE, TH3_STAND); // 解体フェーズ
    FAT_SetCheckThink( work, FAT_ChkThkTemporary );

    work->nNextThink3   	= -1;
    work->bNext3Valid 		= 0;

    // フェーズ別で使用するワーク初期化
    work->bDemineTrue	= 0;
}



