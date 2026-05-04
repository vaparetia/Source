/*
    prezact.c
    大統領動作処理
    2001/05/12 Masafumi Okuta
    $Id: prezact.c,v 1.1.1.3 2002/11/19 11:48:08 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/
static void ActStandStill( NPCWORK* npc, int time );
static void ActMove( NPCWORK* npc, int time );
static void ActListen( NPCWORK* npc, int time );
static void ActNotice( NPCWORK* npc, int time );
static void ActFind( NPCWORK* npc, int time );
static void ActScared( NPCWORK* npc, int time );
static void ActTumble( NPCWORK* npc, int time );
static void ActMutekiOneTime( NPCWORK* npc, int time );
static void ActFootUp( NPCWORK* npc, int time );
static void ActBehind( NPCWORK* npc, int time );
static void ActSleep( NPCWORK* npc, int time );
static void ActAwake( NPCWORK* npc, int time );
static void ActFall( NPCWORK* npc, int time );
static void ActDown( NPCWORK* npc, int time );
static void ActWakeup( NPCWORK* npc, int time );
/*----------------------------------------------------------------*/

// 直立待ち
static void ActStandStill( NPCWORK* npc, int time )
{
    NPCACT *act ;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    if ( npc->CheckDamage( npc ) ) return ;
    if ( npc->CheckPad( npc ) ) return ;

    if ( act->dir >= 0 ) {
	npc->ctrl->turn.vy = act->dir ;
	NPC_SetActMode( npc, NPC_ActMove ) ;
	return ;
    }
}

// 移動
static	void ActMove( NPCWORK* npc, int time )
{
    CONTROL*	ctrl;
    NPCACT*	act;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;
    ctrl = npc->ctrl ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE  ) ;

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }
    
    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return ;
    }

    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, ActStandStill ) ;
	}
	return ;
    }
	
    if ( act->dir < 0 ) {
	NPC_SetActMode( npc, ActStandStill );
	return ;
    }

    // 進行方向 
    ctrl->turn.vy = act->dir ;
}

// 聞き耳を立てる
static void ActListen( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    // 初期設定
    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    if ( act->current_mot == PREZ_MOT_LISTEN_START ){ // 聞き耳開始
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// 再生終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_LISTEN );
	}
    }else if ( act->current_mot == PREZ_MOT_LISTEN ){ // 聞き耳ポーズ
	Work* work;
	work = (void *)npc->character;
	if ( work->bMotEnd ){ // 終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_LISTEN_END );
	}
    }else if ( act->current_mot == PREZ_MOT_LISTEN_END ){  // 聞き耳終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    act->act_end = 1;
	    NPC_SetActMode( npc, ActStandStill );
	    return;
	}
    }
    // ダメージ
    if ( npc->CheckDamage( npc ) ) return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return ;
    // 方向更新
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

// 注目する
static void ActNotice( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_THINK);

    // 初期設定
    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    // のぞきこみ
    if ( act->current_mot == PREZ_MOT_PEEP_START ){ // のぞき込み開始
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// 再生終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_PEEP );
	}
    }else if ( act->current_mot == PREZ_MOT_PEEP ){ // のぞき込みポーズ
	Work* work;
	work = (void *)npc->character;
	if ( work->bMotEnd ){ // 終了
	    act->act_end = 1;
	    NPC_SetActMode( npc, ActStandStill );
	    return;
	}
    }

    // うで組
    if ( act->current_mot == PREZ_MOT_HOLDARM_START ){ // うで組開始
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// 再生終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_HOLDARM );
	}
    }else if ( act->current_mot == PREZ_MOT_HOLDARM ){ // うで組ポーズ
	Work* work;
	work = (void *)npc->character;
	if ( work->bMotEnd ){ // 終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_HOLDARM_END );
	}
    }else if ( act->current_mot == PREZ_MOT_HOLDARM_END ){  // うで組終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    act->act_end = 1;
	    NPC_SetActMode( npc, ActStandStill );
	    return;
	}
    }


    // ダメージ
    if ( npc->CheckDamage( npc ) ) return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return ;
    // 方向更新
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

// 発見びっくり
static void ActFind( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work*  work = (Work *)npc->character;
    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);

	// 顔アニメ
	PREZ_SetFaceFlag( work, PREZ_FACE_EYE_SURPRISE);
    }
    
    if ( npc->CheckDamage( npc ) ) return ;

    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, ActStandStill ) ;
	}
	return ;
    }
    
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

// おびえる
static void ActScared( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work* 	work;

    act = &npc->action ;
    work = (void *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_SQUAT);

	work->vecAdjust.vx = 0;
	work->vecAdjust.vz = 0;
	work->nHeadAdj = 0;
	work->nNoticeCntr = 0;
	// 顔アニメ
	PREZ_SetFaceFlag( work, PREZ_FACE_EYE_SCARE);
    }

    if ( npc->CheckDamage( npc ) ) { 	// ダメージチェック
	work->vecAdjust = DG_ZeroSVector;
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
	return ;
    }

    if ( npc->CheckPad( npc ) ){	// パッドチェック
	work->vecAdjust = DG_ZeroSVector;
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
 	return ;
    }

    // しゃがみ
    if ( act->current_mot == PREZ_MOT_SQUAT_START ){ // しゃがみ開始
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// 再生終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_SQUAT );
	}
    }else if ( act->current_mot == PREZ_MOT_SQUAT ){ // しゃがみポーズ
	Work* work;
	work = (void *)npc->character;
	if ( work->bMotEnd ){ // 終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_SQUAT_END );
	}
    }else if ( act->current_mot == PREZ_MOT_SQUAT_END ){  // しゃがみ終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_STAND );
	}
    }else if ( act->current_mot == PREZ_MOT_STAND ){  // 立ち
	if ( work->bMotEnd ) { // 終了
	    work->vecAdjust = DG_ZeroSVector;
	    GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	    GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
	    act->act_end = 1;
	    NPC_SetActMode( npc, ActStandStill );
	    return;
	}
    }
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}
// 転ぶ->起き上がる
static void ActTumble( NPCWORK* npc, int time )
{
    Work* 	work;
    NPCACT*	act;

    act = &npc->action ;
    work = (Work*)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 初期化
    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_DOWN);
    }

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_CLOSE);

    if ( act->current_mot == PREZ_MOT_TUMBLE){	// 転倒
	// 再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    work->nDownCntr = 0;
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_DOWN_IDLE );
	}
    }else if ( act->current_mot == PREZ_MOT_DOWN_IDLE ){	// 倒れ持続
	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN );
	// 終了
	if ( work->nDownCntr++ >= PREZ_TUMBLE_TIME ) {
	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_WAKEUP );
	}
    }else if ( act->current_mot == PREZ_MOT_WAKEUP ){	// 起き上がり
	// 再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, ActStandStill ) ;
	    return ;
	}
    }

    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return;
    }

    if ( npc->CheckPad( npc ) ){    // パッドチェック
 	return;
    }

    
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

// 無敵で１回再生
static void ActMutekiOneTime( NPCWORK* npc, int time )
{
    NPCACT *act ;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
    }

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    if ( act->current_mot == PREZ_MOT_FOOTUP ){
	MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.5f ) ;
    }

    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return ;
    }

    if ( npc->CheckPad( npc ) ) 	return ;


    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, ActStandStill ) ;
	return ;
    }
}

// 足あげ
static void ActFootUp( NPCWORK* npc, int time )
{
    NPCACT *act ;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_FOOTUP);
    }

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    if ( act->current_mot == PREZ_MOT_FOOTUP ){
	MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.5f ) ;
    }

    if ( npc->CheckDamage( npc ) )  return; // ダメージチェック
    if ( npc->CheckPad( npc ) )     return; // パッドチェック

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, ActStandStill ) ;
	return ;
    }
}

// はりつき
static void ActBehind( NPCWORK* npc, int time )
{
    NPCACT *act ;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
    }

    // 顔アニメ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ) PREZ_SetFaceFlag( work, PREZ_FACE_EYE_BLINK);

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) 	return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) 	return ;

    if ( act->current_mot == PREZ_MOT_BEHIND_F_START_L ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_F_POSE_L );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_F_START_R ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_F_POSE_R );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_B_START ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_B_POSE );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_F_POSE_L ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_F_END_L );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_F_POSE_R ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_F_END_R );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_B_POSE ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    NPC_SetActMotion( npc, npc->base_mar, PREZ_MOT_BEHIND_B_END );	
	}
    }else if ( act->current_mot == PREZ_MOT_BEHIND_F_END_L ||
	       act->current_mot == PREZ_MOT_BEHIND_F_END_R ||
	       act->current_mot == PREZ_MOT_BEHIND_B_END ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, ActStandStill ) ;
	    return ;
	}
    }
    //    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

// ねている
static void ActSleep( NPCWORK* npc, int time )
{
    Work* 	work;
    NPCACT*	act;

    act = &npc->action ;
    work = (Work*)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_EYE_CLOSE) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_CLOSE);

    if ( time % COUNT_VMODE(90) == 0 ){
	NPC_CallHeadMark( npc, HMK2_TYPE_ZZZ ); // ZZZ
	if ( MAO_Rand(2) == 0 ) PREZ_Say( work, SE_ZZZ01, GM_SEMODE_BOMB); // いびき１
	else			PREZ_Say( work, SE_ZZZ02, GM_SEMODE_BOMB); // いびき２
    }

    if ( npc->CheckDamage( npc ) ){
	NPC_CallHeadMark( npc, HMK2_TYPE_KILL ); // ZZZを消す
	return ;
    }
    if ( act->pad != act->set_pad ) {
	NPC_CallHeadMark( npc, HMK2_TYPE_KILL ); // ZZZを消す
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, ActStandStill ) ;
	}
	return ;
    }
}

// 目がさめた
static void ActAwake( NPCWORK* npc, int time )
{
    Work* 	work;
    NPCACT*	act;

    act = &npc->action ;
    work = (Work*)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_CallHeadMark( npc, HMK2_TYPE_RED_AT ); // !
	PREZ_StreamRequestJustTime( work, PREZ_STRM_NOISE01);
//	PREZ_CallSe(work, SE_BIKKURI, &npc->ctrl->mov, GM_SEMODE_BOMB); // びっくり
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    if ( npc->CheckDamage( npc ) ) return ;

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, ActStandStill ) ;
	return ;
    }
}

// 見渡す
static void ActOverlook( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work* 	work;
    
    act = &npc->action;
    work = (void *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 初期設定
    if ( time == 0 ){
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );	
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    // アジャスト
    GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
    GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ){
	work->vecAdjust = DG_ZeroSVector;
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
	return;
    }

    // パッドチェック
    if ( npc->CheckPad( npc ) ){
	work->vecAdjust = DG_ZeroSVector;
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
 	return;
    }

    // 終了
    if ( work->bMotEnd ) { 
	act->act_end = 1;
	NPC_SetActMode( npc, ActStandStill );
	work->vecAdjust = DG_ZeroSVector;
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
	return;
    }
}

// 起き上がる
static void ActWakeup( NPCWORK* npc, int time )
{
    NPCACT	*act ;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->damage_mar, PREZ_MOT_BOMBDMG_END ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_STAND);
    }

    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return ;
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, ActStandStill ) ;
	return ;
    }
}

// パッドチェック関数
static int PREZ_CheckPad( NPCWORK* npc )
{
    NPCACT*	act;

    act = &npc->action;

    if ( act->pad == PAD_NONE ) return 0 ;

    switch ( act->pad ) {
    case PAD_MOVE :		// 移動
	NPC_SetModeFromPad( npc, ActMove, npc->base_mar, PREZ_MOT_WALK, act->pad ) ;
	break ;
    case PAD_RUN :		// 走る
	NPC_SetModeFromPad( npc, ActMove, npc->base_mar, PREZ_MOT_RUN, act->pad ) ;
	break ;
    case PAD_LISTEN: 		// 聞き耳
	NPC_SetModeFromPad( npc, ActListen, npc->base_mar, PREZ_MOT_LISTEN_START, act->pad ) ;
	break;
    case PAD_NOTICE:    	// 注目
	if ( MAO_Rand(2) == 0)  NPC_SetModeFromPad( npc, ActNotice, npc->base_mar, PREZ_MOT_PEEP_START, act->pad );
        else			NPC_SetModeFromPad( npc, ActNotice, npc->base_mar, PREZ_MOT_HOLDARM_START, act->pad );
	break;
    case PAD_FIND:		// 発見
	NPC_SetModeFromPad( npc, ActFind, npc->base_mar, PREZ_MOT_FIND, act->pad ) ;
	break;
    case PAD_SCARED:		// 怖がりしゃがむ
	NPC_SetModeFromPad( npc, ActScared, npc->base_mar, PREZ_MOT_SQUAT_START, act->pad ) ;
	break;
    case PAD_TUMBLE:		// 転ぶ
	NPC_SetModeFromPad( npc, ActTumble, npc->base_mar, PREZ_MOT_TUMBLE, act->pad ) ;
	break;
    case PAD_SLIDE:		// スライディング
	NPC_SetModeFromPad( npc, ActMutekiOneTime, npc->base_mar, PREZ_MOT_EVADE_U, act->pad ) ;
	break;
    case PAD_SUMMER:		// サマーソルト
	NPC_SetModeFromPad( npc, ActMutekiOneTime, npc->base_mar, PREZ_MOT_EVADE_D, act->pad ) ;
	break;
    case PAD_FOOTUP:		// 足あげ
	NPC_SetModeFromPad( npc, ActFootUp, npc->base_mar, PREZ_MOT_FOOTUP, act->pad ) ;
	break;
    case PAD_BEHIND_L:		// 左張りつき
	NPC_SetModeFromPad( npc, ActBehind, npc->base_mar, PREZ_MOT_BEHIND_F_START_L, act->pad ) ;
	break;
    case PAD_BEHIND_R:		// 右張りつき
	NPC_SetModeFromPad( npc, ActBehind, npc->base_mar, PREZ_MOT_BEHIND_F_START_R, act->pad ) ;
	break;
    case PAD_BEHIND_B:		// 後ろ張りつき
	NPC_SetModeFromPad( npc, ActBehind, npc->base_mar, PREZ_MOT_BEHIND_B_START, act->pad ) ;
	break;
    case PAD_SIDESTEP_L:	// 横飛び左
	NPC_SetModeFromPad( npc, ActMutekiOneTime, npc->base_mar, PREZ_MOT_SIDESTEP_L, act->pad ) ;
	break;
    case PAD_SIDESTEP_R:	// 横飛び右
	NPC_SetModeFromPad( npc, ActMutekiOneTime, npc->base_mar, PREZ_MOT_SIDESTEP_R, act->pad ) ;
	break;
    case PAD_SLEEP:		// 睡眠
	NPC_SetModeFromPad( npc, ActSleep, npc->base_mar, PREZ_MOT_SLEEP2_POSE, act->pad ) ;
	break;
    case PAD_AWAKE:		// 目覚め
	NPC_SetModeFromPad( npc, ActAwake, npc->base_mar, PREZ_MOT_AWAKE, act->pad ) ;
	break;
    case PAD_WAKEUP :		// 起きる
	NPC_SetModeFromPad( npc, ActWakeup, npc->damage_mar, PREZ_MOT_BOMBDMG_END, act->pad ) ;
	break ;
    case PAD_OVERLOOK :		// きょろきょろ
	NPC_SetModeFromPad( npc, ActOverlook, npc->base_mar, PREZ_MOT_STAND, act->pad ) ;
	break ;
    }

    return 1;
}

// 倒れる
static void ActFall( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF);

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_DOWN);

	// ダメージ音声
	if ( work->nStrmHandle != -1 ){
	    GM_StreamStop( work->nStrmHandle ); // ストリームキャンセル
	    work->nStrmHandle = -1;
	}
	work->nStrmIndexNext = -1;
	GM_SeSetMode( PREZ_SE[(SE_PREDMG01 + MAO_Rand(3))], 
		      (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0],
		      GM_SEMODE_BOMB); // ダメージ
    }

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_CLOSE);

    if ( npc->CheckDamage( npc ) ) return ;

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( act->faint > 0 ) {
	    NPC_SetActMode( npc, ActDown ) ;
	} else {
	    NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
	    NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
	    NPC_SetActMode( npc, NPC_ActFaint ) ;
	}
	return ;
    }
}
// 死亡
static void ActDead( NPCWORK* npc, int time )
{
    NPCACT*	act;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF );

    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_BOMBDMG_START ) ;
	/* モーション遅いので再生スピードUP */
	MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.75f ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_DOWN);

	// プロシージャコール
	GCL_ExecProc( work->procOver, NULL );

	// アウト音声
	if ( work->nStrmHandle != -1 ){
	    GM_StreamStop( work->nStrmHandle ); // ストリームキャンセル
	    work->nStrmHandle = -1;
	}
	work->nStrmIndexNext = -1;
	GM_SeSetMode( PREZ_SE[SE_PREOUT01], (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0],
		      GM_SEMODE_BOMB); 
    }

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_CLOSE);

    if ( act->current_mot == PREZ_MOT_BOMBDMG_START ){ // ふっ飛び
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    // ゲームオーバー処理を終了
	    GM_GameOverProcEnd( work );	

	    NPC_SetActMotion( npc, act->set_mar, PREZ_MOT_BOMBDMG_POSE );
	}
    }else if ( act->current_mot == PREZ_MOT_BOMBDMG_POSE ){ // 死亡

    }	
}

// 倒れている状態
static void ActDown( NPCWORK* npc, int time )
{
    NPCACT	*act ;
    Work*  work = (Work *)npc->character;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF ) ;

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->damage_mar, PREZ_MOT_BOMBDMG_POSE ) ;
	NPC_ChangeTargetSize( npc, PREZ_TARGET_SIZE_DOWN);
    }

    // 顔アニメ
    PREZ_SetFaceFlag( work, PREZ_FACE_EYE_CLOSE);

    if ( npc->CheckDamage( npc ) ) return ;

    if ( time > COUNT_VMODE(60) ) {
	NPC_SetActMode( npc, ActWakeup ); // 起きる
	return ;
    }
}

/*----------------------------------------------------------------*/
// 通常チェックダメージ
static	int	NormCheckDamage( NPCWORK* npc )
{	
    Work*	work;
    TARGET	*def ;
    NPCACT	*act ;
    NPCTARGET	*trg ;
    CAPTURE_TARGET	*cap ;
    long64	weapon ;
    int		dam_child_num ;

    work = (Work*)npc->character;
    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    weapon = 0 ;

    // 子ターゲットにダメージ
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) { 
	trg->dam_trg = trg->def_child + dam_child_num ;
	weapon = trg->dam_trg->weapon_type ;
	NPC_DamageCaptureFlagClear( npc ) ;

	// このフレームではダメージを受けない 
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF );

	if ( weapon & WP_NIKITA ){
#ifdef DEBUG_MODE
	    if ( !nDbgPrezMutekiDraw ) act->life -= PREZ_BOMB_DMG; // 無敵チェック
#else
	    act->life -= PREZ_BOMB_DMG;
#endif	
	    if ( act->life <= 0){
		// ゲームオーバー処理を開始 
		GM_GameOverProcStart( work );
		// 死亡
		NPC_SetModeFromPad( npc, ActDead, npc->damage_mar, PREZ_MOT_BOMBDMG_START, act->pad ) ;
	    }else{
		// 転倒
		NPC_SetModeFromPad( npc, ActFall, npc->damage_mar, PREZ_MOT_BOMBDMG_START, act->pad ) ;
	    }
	    return 1;
	}else{
	    return 0;
	}
    }

    // 親ターゲットにダメージ
    if ( TARGET_POWER & def->damaged ) {
	weapon = def->weapon_type ;
	NPC_DamageCaptureFlagClear( npc ) ;
	
	// このフレームではダメージを受けない
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  );

	if ( weapon & WP_NIKITA ){
#ifdef DEBUG_MODE
	    if ( !nDbgPrezMutekiDraw ) act->life -= PREZ_BOMB_DMG; // 無敵チェック
#else
	    act->life -= PREZ_BOMB_DMG;
#endif
	    if ( act->life <= 0){
		// ゲームオーバー処理を開始 
		GM_GameOverProcStart( work );
		// 死亡
		NPC_SetModeFromPad( npc, ActDead, npc->damage_mar, PREZ_MOT_BOMBDMG_START, act->pad ) ;
	    }else{
		// 転倒
		NPC_SetModeFromPad( npc, ActFall, npc->damage_mar, PREZ_MOT_BOMBDMG_START, act->pad ) ;
	    }
	    return 1;
	}else{
	    return 0;
	}
    }

    // キャプチャーは意味なし
    if ( TARGET_CAPTURE & def->damaged ) {
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	cap = trg->capture ;

	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE);
	cap->capture = NULL;	
	cap->flag = 0;
    }

    return 0;
}
/*----------------------------------------------------------------*/
static void Action( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc;

//  NPC_ActInit( npc ) ;
    NPC_Action( npc );
    NPC_ActStatusCheck( npc );

    NPC_Gravitation( npc );
}

