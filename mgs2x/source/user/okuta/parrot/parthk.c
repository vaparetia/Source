/*
    parthk.c
    オウム思考処理
    2001/05/12 Masafumi Okuta
    $Id: parthk.c,v 1.1.1.3 2002/11/19 11:48:07 Yoshizawa1 Exp $
*/
//static int nTmpTalk = 1;

extern int WP_InMic( FVECTOR *pos );	// 指向性マイク判定 : skoba/weapon/spray.c 
/*----- Think3 --------------------------------------------------------*/
// アイドリング
static void Think3_Stand( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	work->nIdleTime = COUNT_VMODE( MAO_GetRandom( work->nTalkBlank1, work->nTalkBlank2) );

	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青

    }
    _sceVu0CopyVector( &work->vecAimPos, &GM_PlayerPosition); // プレイヤー注目

    // 位置強制修正
    MAO_InterpVec( &npc->ctrl->mov, &npc->ctrl->mov, &work->vecInitPos, 0.1f);

    npc->action.pad = PAD_IDLE;	// 待機

    // ほったらかし
    if ( work->count3 >= work->nIdleTime){ // アイドリング終了
	int nSeed, nBase;

	nSeed = BP_PS2_rand() % 100;
	nBase = 0;

	// エロ本
	if ( work->nEroLearn){
	    nBase += work->nSubjectRate[PAR_SUBJECT_ERO];
	}
	if ( nSeed < nBase ){
	    PAR_SetThink3( work, TH3_ERO_SPEAK);	
	    return;
	}

	// エイムズ
	if ( work->nEamesLearn){
	    nBase += work->nSubjectRate[PAR_SUBJECT_EIMS];
	}
	if ( nSeed < nBase ){
	    PAR_SetThink3( work, TH3_EAMES_SPEAK);	
	    return;
	}
	
	PAR_SetThink3( work, TH3_SAY);	
	return;
    }

    work->count3++;
}
// 音声再生
static void Think3_Say( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	if ( work->nTalkFlag ){
	    int nStrm, nSeed;
	    nSeed = BP_PS2_rand() % 100;
	    if ( nSeed < 80 ) 	    nStrm = PAR_STRM_TALK01 + (BP_PS2_rand() % 4);
	    else if ( nSeed < 95 )  nStrm = PAR_STRM_RARE;
	    else		    nStrm = PAR_STRM_OTACON;
	
	    PAR_StreamRequest( work, nStrm); // 音声再生
	    work->nIdleTime = work->nStrmLen[ nStrm ]; // 音声長さ取得
	    if ( BP_PS2_rand() % 100 < work->nNoiseRate ){
		GM_SetNoise( NOISE_SS , &npc->ctrl->mov, npc->ctrl->map );	  // 物音
	    }
	}
	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青
    }


    // 位置強制修正
    MAO_InterpVec( &npc->ctrl->mov, &npc->ctrl->mov, &work->vecInitPos, 0.1f);

    npc->action.pad = PAD_TALK;	// 待機

    if ( work->count3 >= work->nIdleTime){ // アイドリング終了
	PAR_SetThink3( work, TH3_STAND);   
	return;
    }
    work->count3++;
}
// エロ本ありか聞き耳
static void Think3_EroBookListen( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
    }


    // 位置強制修正
    MAO_InterpVec( &npc->ctrl->mov, &npc->ctrl->mov, &work->vecInitPos, 0.1f);

    npc->action.pad = PAD_ATTENTION;	// 注目

    if ( work->count3 >= COUNT_VMODE(work->nAttentionTime) ){ // 注目終了 ストリームの長さで
	PAR_SetThink3( work, TH3_ERO_SPEAK);   // 待機
	return;
    }
    work->count3++;
}
// エロ本ありか喋る
static void Think3_EroBookSpeak( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	if ( work->nTalkFlag ){
	    int nStrm;
	    nStrm = PAR_STRM_ERO01 + BP_PS2_rand() % 3;
	    PAR_StreamRequest( work, nStrm); // 音声再生
	    work->nIdleTime = work->nStrmLen[ nStrm ]; // 音声長さ取得

	    npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青色
	}
    }

    // 位置強制修正
    MAO_InterpVec( &npc->ctrl->mov, &npc->ctrl->mov, &work->vecInitPos, 0.1f);

    npc->action.pad = PAD_TALK;	// 注目

    if ( work->count3 >= work->nIdleTime ){ // 注目終了 ストリームの長さで
	PAR_SetThink3( work, TH3_STAND);   // 待機
	return;
    }
    work->count3++;

}
// エイムズ問いかけ聞く
static void Think3_EamesListen( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色
    }
    _sceVu0CopyVector( &work->vecAimPos, &GM_PlayerPosition); // プレイヤー注目

    npc->action.pad = PAD_ATTENTION;	// 注目

    if ( work->count3 >= COUNT_VMODE(work->nAttentionTime) ){ // 注目終了 ストリームの長さで
	PAR_SetThink3( work, TH3_STAND);   // 待機
	return;
    }
    work->count3++;
}
// エイムズ返し
static void Think3_EamesSpeak( PARROT* work )
{
    NPCWORK*	npc;
    
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	if ( work->nTalkFlag ){
	    int nStrm = PAR_STRM_EAMES01 + (BP_PS2_rand() % 2); // 一回
	    PAR_StreamRequest( work, nStrm); 	   // 音声再生
	    work->nIdleTime = work->nStrmLen[ nStrm ]; // 音声長さ取得
	}
	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー黄色
    }

    npc->action.pad = PAD_TALK;	// 喋り

    if ( work->count3 >= work->nIdleTime ){ // 注目終了 ストリームの長さで
	PAR_SetThink3( work, TH3_STAND);   // 待機
	return;
    }
    work->count3++;
}

// 敵兵呼出
#define ENECALL_DELAY (COUNT_VMODE(30))
static void Think3_EnemyCall( PARROT* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	if ( work->nTalkFlag ){
	    int nStrm;
	    nStrm = PAR_STRM_ENEMY01 + (BP_PS2_rand() % 2);
	    PAR_StreamRequest( work, nStrm);  // 音声再生
	    work->nIdleTime = work->nStrmLen[ nStrm ]; // 音声長さ取得
	}else{
	    work->nIdleTime = COUNT_VMODE(120); // 音声長さ取得
	}
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤色

	npc->action.pad = PAD_FLIPFLOP;	// ばたばた
    }	
//    npc->action.pad = PAD_TALK;	// 喋り
//    npc->action.pad = PAD_IDLE;	// 待機
#if 0
    if ( work->count3 == work->nIdleTime){ // アイドリング終了
	PAR_SetThink3( work, TH3_STAND);   // 待機
	return;
    }
#else
    if ( npc->action.act_end ) {
	PAR_SetThink3( work, TH3_STAND);   // 待機
	return;
    }
#endif
    work->count3++;

}

// ばたばた
static void Think3_Flipflop( PARROT* work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	if ( work->nTalkFlag ){
	    int nStrm, nSeed;
	    nSeed = BP_PS2_rand() % 100;
	    if ( nSeed < 90 ){
		nStrm = PAR_STRM_HELP01;
	    }else {
		nStrm = PAR_STRM_RARE;
	    }
	
	    PAR_StreamRequest( work, nStrm); // 音声再生
	}
	npc->action.radar_color = RADAR_COLOR_YELOW;	// レーダー黄色

	npc->action.pad = PAD_FLIPFLOP;	// ばたばた

	// 位置強制修正
	MAO_InterpVec( &npc->ctrl->mov, &npc->ctrl->mov, &work->vecInitPos, 0.1f);
    }


    if ( work->nTalkFlag ){
	if ( work->count3 == COUNT_VMODE(30) ){
	    GM_SetNoise( NOISE_S, &npc->ctrl->mov, npc->ctrl->map);	  // 物音(ばさばさ音)
	}
    }

    if ( npc->action.act_end ) {
	PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
	return ;
    }

    work->count3++;
}


static void Think3_SmpDamageWait( PARROT* work )
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
	PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
    }

    /* 途中でシステムダメージ処理に変化 */
    if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
	PAR_SetThink2( work, TH2_SYSTEM_DAMAGE, TH3_WAIT );
    }
}

static void Think3_SysDamageWait( PARROT* work )
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( npc->action.status & NPC_ACT_STATUS_FAINT_END ) {
	PAR_SetThink3( work, TH3_WAKEUP );
	return;
    }

    if ( !(npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM) ) {
	PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
	return ;
    }
}

static void Think3_WakeUp( PARROT* work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( CheckThinkInit( work) ){ // 初期化チェック
	npc->action.pad = PAD_WAKEUP ;
	npc->action.radar_color = RADAR_COLOR_BLUE;	// レーダー青色
    }

    if ( npc->action.act_end ) { // 終了
	PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND );
	return ;
    }

    work->count3++;
}


/*----- Think2 --------------------------------------------------------*/
// 平静
static void Think2_Ataraxia( PARROT* work )
{
    switch( work->think3 ) {
    case  TH3_STAND :		// 待機
	Think3_Stand( work ) ;
	break ;
    case  TH3_SAY :		// 適当に喋る
	Think3_Say( work ) ;
	break;
    case TH3_ERO_LISTEN :	// エロ本聞き耳
	Think3_EroBookListen( work ) ;
	break ;
    case TH3_ERO_SPEAK :	// エロ本ありかを語る
	Think3_EroBookSpeak( work ) ;
	break ;
    case TH3_EAMES_LISTEN :	// あんたがエイムズ 聞く
	Think3_EamesListen( work ) ;
	break ;
    case TH3_EAMES_SPEAK :	// あんたがエイムズ ものまね
	Think3_EamesSpeak( work ) ;
	break ;
    case TH3_ENEMY_CALL :	// 敵兵を呼ぶ
	Think3_EnemyCall( work ) ;
	break ;
    case TH3_FLIPFLOP :		// ばたばた
	Think3_Flipflop( work ) ;
	break;
    }
}
static void Think2_SampleDamage( PARROT* work )
{
    switch( work->think3 ) {
    case  TH3_WAIT :
	Think3_SmpDamageWait( work ) ;
	break ;
    case  TH3_WAKEUP :
	Think3_WakeUp( work ) ;
	break ;
    }
}

static void Think2_SystemDamage( PARROT* work )
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
static int DamageCheck( PARROT* work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_SAMPLE_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}
	if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_SYSTEM_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}
	return 0 ;
}

static void Think1_Normal( PARROT* work )
{
    if ( DamageCheck( work ) ) return ;

    switch( work->think2 ) {
    case TH2_ATARAXIA:
	Think2_Ataraxia( work ) ;
	break;
    }
}


static void Think1_Damage( PARROT* work )
{
    switch( work->think2 ) {
    case  TH2_SAMPLE_DAMAGE :
	Think2_SampleDamage( work ) ;
	break ;
    case  TH2_SYSTEM_DAMAGE :
	Think2_SystemDamage( work ) ;
	break ;
    }

}

/*----- 思考処理メイン --------------------------------------------------------*/
static void Think( PARROT* work )
{

    switch( work->think1 ) {
    case  TH1_NORMAL :
	Think1_Normal( work ) ;
	break ;
    case  TH1_DAMAGE :
	Think1_Damage( work ) ;
	break ;
    }
}
/*----- --------------------------------------------------------*/
static void StartThink( PARROT* work )
{
    NPCWORK* npc;

    npc = &work->npc;

    NPC_SetCheckPad( &work->npc, SampleCheckPad ) ;
    NPC_SetCheckDamage( &work->npc, SmpCheckDamage ) ;

    PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_STAND);
    work->nNextThink3 		= 0;
    work->bNextThink3Valid 	= 0;
}
