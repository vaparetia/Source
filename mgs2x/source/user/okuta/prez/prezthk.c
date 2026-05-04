/*
    prezthk.c
    大統領思考処理
    2001/08/23 Masafumi Okuta
    $Id: prezthk.c,v 1.1.1.3 2002/11/19 11:48:11 Yoshizawa1 Exp $
*/

/*----- Think3 --------------------------------------------------------*/
// 通常ダメージ
static void Think3_NormDamageWait( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
	PREZ_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
    }

    /* 途中でシステムダメージ処理に変化 */
    if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
	PREZ_SetThink2( work, TH2_SYSTEM_DAMAGE, TH3_WAIT );
    }
}

// NPCシステムダメージ
static void Think3_SysDamageWait( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( npc->action.status & NPC_ACT_STATUS_FAINT_END ) {
	PREZ_SetThink3( work, TH3_WAKEUP );
	return;
    }

    if ( !(npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM) ) {
	PREZ_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
	return ;
    }
}

// 起きる
static void Think3_WakeUp( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_WAKEUP ;
	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
    }

    if ( npc->action.act_end ) {
	PREZ_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
	return ;
    }

    work->count3++;
}


/*----- Think2 --------------------------------------------------------*/
// 平静
static void Think2_Ataraxia( Work *work )
{
    switch( work->think3 ) {
    case  TH3_STAND :		// 立ち
	Think3_Stand( work );
	break ;
    case  TH3_MOVE :		// 移動
	Think3_Move( work );
	break ;
    case TH3_ROUTE_STAND:	// ルート停止
	Think3_RouteStand( work );
	break;
    case TH3_ROUTE_MOVE:	// ルート移動	
	Think3_RouteMove( work );
	break;
    case TH3_LISTEN :		// 聞き耳
	Think3_Listen( work );
	break ;
    case TH3_LISTEN_WAIT :	// 聞き耳移動までのブランク
	Think3_ListenWait( work );
	break ;
    case TH3_NOTICE :		// 注目
	Think3_Notice( work );
	break ;
    case TH3_SLEEP :		// ねている
	Think3_Sleep( work );
	break ;
    case TH3_AWAKE :		// 目がさめる
	Think3_Awake( work );
	break ;
    case TH3_OVERLOOK :		// きょろきょろ
	Think3_Overlook( work );
	break ;
    }
}

// パニック
static void Think2_Panic( Work *work )
{
    switch( work->think3 ) {
    case  TH3_READY :
	Think3_PanicReady( work ) ;
	break ;
    case  TH3_STAND :
	Think3_PanicStand( work ) ;
	break ;
    case  TH3_MOVE :
	Think3_PanicMove( work ) ;
	break ;
    case TH3_FIND :
	Think3_PanicFind( work ) ;
	break ;
    case TH3_SCARED :
	Think3_PanicScared( work ) ;
	break ;
    case TH3_TUMBLE :
	Think3_PanicTumble( work ) ;
	break ;
    case TH3_FOOTUP:
	Think3_PanicFootUp( work ) ;
	break ;
    case TH3_BEHIND_B:		// 背中張りつき
	Think3_PanicEvadeBack( work ) ;
	break ;
    case TH3_BEHIND_L:		// 左壁張りつき
	Think3_PanicEvadeLeft( work ) ;
	break ;
    case TH3_BEHIND_R:		// 右
	Think3_PanicEvadeRight( work ) ;
	break ;
    case TH3_SIDESTEP_L:	// 横飛び:左
	Think3_PanicSideStepLeft( work ) ;
	break ;
    case TH3_SIDESTEP_R:	// 横飛び:右
	Think3_PanicSideStepRight( work ) ;
	break ;
    case TH3_SLIDE:		// スライティング
	Think3_PanicSlide( work ) ;
	break ;
    case TH3_SUMMER:		// サマーソルト
	Think3_PanicSummer( work ) ;
	break ;
    }
}

// 通常ダメージ
static void Think2_NormalDamage( Work *work )
{
    switch( work->think3 ) {
    case  TH3_WAIT :
	Think3_NormDamageWait( work ) ;
	break ;
    case  TH3_WAKEUP :
	Think3_WakeUp( work ) ;
	break ;
    }
}

// NPCシステムダメージ
static void Think2_SystemDamage( Work *work )
{
    switch( work->think3 ) {
    case  TH3_WAIT :
	Think3_SysDamageWait( work ) ;
	break ;
    case  TH3_WAKEUP :
	Think3_WakeUp( work ) ;
	break ;
    }
}

/*----- Think1 --------------------------------------------------------*/
// ダメージチェック関数
static int DamageCheck( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc ;

    // 通常
    if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
	work->think1 = TH1_DAMAGE ;
	work->think2 = TH2_NORMAL_DAMAGE;
	work->think3 = TH3_WAIT ;
	work->count3 = 0 ;
	return 1 ;
    }

    // NPCシステムレベル
    if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
	work->think1 = TH1_DAMAGE ;
	work->think2 = TH2_SYSTEM_DAMAGE ;
	work->think3 = TH3_WAIT ;
	work->count3 = 0 ;
	return 1 ;
    }

    return 0 ;
}

// 通常
static void Think1_Normal( Work *work )
{
    if ( DamageCheck( work ) ) return ;

    switch( work->think2 ) {
    case TH2_ATARAXIA:
	NPC_SetHeartBeat( &work->npc, NPC_HEART_BEAT_NORMAL);
	Think2_Ataraxia( work ) ;
	break;
    case TH2_PANIC:
	NPC_SetHeartBeat( &work->npc, NPC_HEART_BEAT_NORMAL*2);
	Think2_Panic( work ) ;
	break;
    }
}

// ダメージ
static void Think1_Damage( Work *work )
{
    switch( work->think2 ) {
    case  TH2_NORMAL_DAMAGE :
	NPC_SetHeartBeat( &work->npc, NPC_HEART_BEAT_NORMAL*2);
	Think2_NormalDamage( work ) ;
	break ;
    case  TH2_SYSTEM_DAMAGE :
	NPC_SetHeartBeat( &work->npc, NPC_HEART_BEAT_NORMAL*2);
	Think2_SystemDamage( work ) ;
	break ;
    }

}

/*----- 思考処理メイン --------------------------------------------------------*/
static void Think( Work *work )
{
    if (work->bDead){
	NPC_SetHeartBeat( &work->npc, 100000); // 心臓停止
	return;
    }

    switch( work->think1 ) {
    case  TH1_NORMAL :
	Think1_Normal( work ) ;
	break ;
    case  TH1_DAMAGE :
	Think1_Damage( work ) ;
	break ;
    }
}

// 思考関数
static void StartThink( Work *work )
{
    NPCWORK* npc;
    ROUTENAVI* rnavi;

    npc = &work->npc;
    rnavi = &work->rnavi;

    NPC_SetCheckPad( &work->npc, PREZ_CheckPad );
    NPC_SetCheckDamage( &work->npc, NormCheckDamage );

    PREZ_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_ROUTE_MOVE);
    work->nNextThink3 		= 0;
    work->bNextThink3Valid 	= 0;

    // ルート関連
    work->nRouteWaitTime 	= 0;
    PREZ_SetTrgpNextZone( work );
}
