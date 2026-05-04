/*
    fatact.c
    ファットマン動作
    2001/03/22 Masafumi Okuta
    $Id: fatact.c,v 1.1.1.3 2002/11/19 11:47:56 Yoshizawa1 Exp $
*/

// ダウン時間取得
static inline int FAT_GetDownTime( void)
{
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	return (COUNT_VMODE(120));	// ダウン時間
	break;
    case GM_LEVEL_EASY:
	return (COUNT_VMODE(90));	// ダウン時間
	break;
    case GM_LEVEL_NORMAL:
	return (COUNT_VMODE(60));	// ダウン時間
	break;
    case GM_LEVEL_HARD:
	return (COUNT_VMODE(60));	// ダウン時間
	break;
    default:
	return (COUNT_VMODE(30));	// ダウン時間
	break;
    }
}

// 走る
static	void ActRun( NPCWORK *npc, int time )
{
    int		nDir;
    Work*	work;
    CONTROL*	ctrl;
    NPCACT*	act;

    // local param init
    work = (Work*)npc->character;
    ctrl = npc->ctrl;
    act  = &npc->action;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ) {
	if ( act->current_mot != FAT_MOT_RUN &&
	     act->current_mot != FAT_MOT_TURN_L &&
	     act->current_mot != FAT_MOT_TURN_R &&
	     act->current_mot != FAT_MOT_REVERSE &&
	     act->current_mot != FAT_MOT_TURN_TO_FRONT &&
	     act->current_mot != FAT_MOT_STOP &&
	     act->current_mot != FAT_MOT_RUN_FIRE &&
	     act->current_mot != FAT_MOT_BOMB_RUNSET &&
	     act->current_mot != FAT_MOT_BOMB_RUNSET_L &&
	     act->current_mot != FAT_MOT_BOMB_RUNSET_R ){
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN_START);	// 走り始めから
	}else{
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN);		// 走りから
	}

	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // 目パチ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) {
	ctrl->turn.vz = 0 ;	// 体の傾きを止める	
	return ;
    }

    // 思考側からの終了チェック
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    ctrl->turn.vz = 0 ;	// 体の傾きを止める	
	    NPC_SetActMode( npc, NPC_ActStandStill );
	}
	return;
    }

    if ( work->nRollerBreak ){ // ローラーブレードが壊れていたら
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // モーション毎の処理
    if ( act->current_mot == FAT_MOT_RUN_START ){   // 走り始め
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// モーション終了
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_RUN, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }else if ( act->current_mot == FAT_MOT_RUN ){   // 走行中
//	if ( FAT_CheckTurnLR(work) ){ }
    }else if ( act->current_mot == FAT_MOT_TURN_L ){   // 左ターン
	// 終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// モーション終了
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_RUN, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE);
	}
    }else if ( act->current_mot == FAT_MOT_TURN_R ){   // 右ターン
	// 終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	// モーション終了
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_RUN, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }

    //----- 攻撃判定発生
//    if ( act->current_mot != FAT_MOT_RUN_START || MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 10 )){
    {
	// 衝突判定
	Clash( npc );        
	// ローラーブレード攻撃判定
	RollerBladeAttack( npc );
    }

    // 方向転換時の体の傾き 
    FAT_Incline( ctrl );

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}
// 旋回(割り込み可能)
static void ActTurnSide( NPCWORK *npc, int time )
{
    NPCACT*	act;
    Work*	work;
    CONTROL*	ctrl;

    act = &npc->action;
    work = (Work*)npc->character;
    ctrl = npc->ctrl;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// 飛び出しモーション
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);	// ターゲットサイズ変更
    }

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( work->nRollerBreak ){ // ローラーブレード壊れチェック
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;

    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ){
	ctrl->turn.vz = 0; // 体の傾きをなくす
	return;
    }
    // 思考チェンジ
    if ( npc->CheckPad( npc ) ){
	ctrl->turn.vz = 0; // 体の傾きをなくす
	return;
    }

    // 衝突判定
    Clash( npc );        
    // ローラーブレード攻撃判定
    RollerBladeAttack( npc );

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }

    // 方向転換時の体の傾き 
    FAT_Incline( ctrl );
    // 方向
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}

// 180度旋回
static void ActTurn180( NPCWORK *npc, int time )
{
    NPCACT*	act;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action;
    ctrl = npc->ctrl;
    work     = (Work*)npc->character ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// モーション
	ctrl->turn.vz = 0;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // 目パチ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	
    }

    // 特殊エフェクト 
    if ( work->fFatPlayRate >= 0.32f && work->fFatPlayRate < 0.7462f ){
	work->bSpecialEffect = 1;
    }
    if ( work->nRollerBreak ){ // ローラーブレード壊れ
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return;

    //----- 攻撃判定発生
    // 衝突判定
    Clash( npc );        
    // ローラーブレード攻撃判定
    RollerBladeAttack( npc );

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}

// 停止
static void ActStop( NPCWORK *npc, int time )
{
    NPCACT*	act;
    Work*	work;

    act = &npc->action;
    work     = (Work*)npc->character ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// モーション再生
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // 目パチ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	
    }

    if ( work->nRollerBreak ){ // ローラーブレード破損
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return;

    //------- 攻撃判定
    if ( work->fFatPlayRate < 0.5f){ // 停止するまで
	// 衝突判定
	Clash( npc );        
	// ローラーブレード攻撃判定
	RollerBladeAttack( npc );
    }

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// ポイント移動
static void ActPointMove( NPCWORK *npc, int time)	
{
    int		nDir;
    NPCACT*	act;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action;
    ctrl = npc->ctrl;
    work     = (Work*)npc->character ;
    
    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, FAT_MOT_RUN ) ;
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);	// ターゲットサイズ変更
    }

    // 目パチ
    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	
    }

    if ( work->nRollerBreak ){ // ローラーブレード破壊チェック
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    // 新動作が発動
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }

    //----- 攻撃判定
    // 衝突判定
    Clash( npc );        
    // ローラーブレード攻撃判定
    RollerBladeAttack( npc );

    // 方向転換時の体の傾き 
    FAT_Incline( ctrl ) ;

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}

// 走り中のリロード
static	void ActRunReload( NPCWORK *npc, int time )
{
    int		nDir;
    Work*	work;
    CONTROL*	ctrl;
    NPCACT*	act;
		
    // local param init
    work     = (Work*)npc->character ;
    ctrl     = npc->ctrl ;
    act      = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN_RELOAD);	//
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nCartStep = CART_STEP_RELEASE;
    }

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }
    // カートリッジ
    switch ( work->nCartStep ){
    case CART_STEP_RELEASE: 	// 外す
	if ( work->fFatPlayRate >= GLK_RUN_CART_OUT ) {		
	    work->nCartCtrl = 4; 
	    work->nCartStep = CART_STEP_HOLD;
	}
	break;
    case CART_STEP_HOLD: 	// 手に持つ
	if ( work->fFatPlayRate >= GLK_RUN_CART_APPEAR ){
	    work->nCartCtrl = 2; // 
	    work->nCartStep = CART_STEP_RELOAD;
	}
	break;
    case CART_STEP_RELOAD: 	// 装填
	work->nCartCtrl = 2; // 
	if ( work->fFatPlayRate >= GLK_RUN_RELOAD_TIME){
	    work->wctrl.flag = WPEF_FLG_START;
	    work->nBullet = GLK_BUL_NUM;	
	    work->nCartCtrl = 1; // 装着
	    work->nCartStep = CART_STEP_END;
	}
	break;
    case CART_STEP_END: 	// 終了
	break;
    }
//printf("%d ::: %d\n", work->nCartStep, work->nCartCtrl);
    if ( work->fFatPlayRate < GLK_RELOAD_TIME ) {
	work->wctrl.flag = WPEF_FLG_START; 
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) {
	ctrl->turn.vz = 0 ;			// 体の傾きを止める	
	return ;
    }
    // パッドチェック
    if ( npc->CheckPad( npc ) ){
	ctrl->turn.vz = 0 ;			// 体の傾きを止める	
	return ;
    }

    // モーション再生終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	ctrl->turn.vz = 0 ;			// 体の傾きを止める	
	act->act_end = 1 ;
	FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad ); // 移動へ
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN);			 // 走りから
	return;
    }

    //----- 攻撃判定発生
    // 衝突判定
    Clash( npc );        
    // ローラーブレード攻撃判定
    RollerBladeAttack( npc );

    // 方向転換時の体の傾き 
    FAT_Incline( ctrl );

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}

// 銃構え
static void ActGunStand( NPCWORK *npc, int time )
{
    int		nDir;
    NPCACT*	act;
    CONTROL*	ctrl;
    Work*	work;

    work     = (Work*)npc->character ;
    ctrl     = npc->ctrl ;
    act = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ); 		      // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) { 
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_FIRE_P ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;

    // 新動作が発動
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}

// 射撃
static void ActGunShoot( NPCWORK *npc, int time )
{
    NPCACT*	act;

    act = &npc->action ;

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ); 		      // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );		      // FATMAN拡張分

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_FIRE );
	FAT_ShootGlock( npc, 0, 10, 10); 	// グロック発射
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( npc->CheckDamage( npc ) ) return;	// ダメージチェック
    if ( npc->CheckPad( npc ) ) return;		// パッドチェック

    if ( time >= 4 ) {	// タイムアウト
	NPC_SetActMode( npc, ActGunStand );
	return ;
    }
}

// 銃構え
static void ActRunShotReady( NPCWORK *npc, int time )
{
    int		nDir;
    Work*	work;
    CONTROL*	ctrl;
    NPCACT*	act;
		
    // local param init
    work     = (Work*)npc->character ;
    ctrl     = npc->ctrl ;
    act      = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) { 
	FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_RUN_FIRE, MOTION_MASK_FULL, 
			    FAT_INTERP_DEF, (float)TIME_BASE);
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
	work->nShootCntr = work->nShootLimit;	// 発射数設定
    }

    FAT_AdjustAimPoint( work, &GM_PlayerPosition ); // プレイヤー追従

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;
#if 0
    // 新動作が発動
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }
#endif
    // 武器発射
    if ( time >= 40 && time % 3 == 0 && work->nShootCntr > 0) {
	FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
	work->nShootCntr--;
    }
    
    // 衝突判定
    Clash( npc );

    // 終了
    if ( (work->nShootCntr == 0 || work->nBullet == 0) && GM_CheckObject_IsEnd( npc->body, 0 ) ) {	
	act->act_end = 1;
	FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad );
	return;
    }

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}
// 走り撃ち
static void ActRunShotAct( NPCWORK *npc, int time )
{
    int 	nDir;
    NPCACT*	act;
    Work* 	work;
    CONTROL*	ctrl;

    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    act = &npc->action ;
    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_ShootGlock( npc, 0, 10, 10); 	// グロック発射
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // 衝突判定
    Clash( npc );

    if ( npc->CheckDamage( npc ) ) return;	// ダメージチェック
    if ( npc->CheckPad( npc ) ) return;		// パッドチェック

    if ( time == 4 ) {	// タイムアウト
	NPC_SetActMode( npc, ActRunShotReady );
	return ;
    }
    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}
// 逆走構え
static void ActBackShotReady( NPCWORK *npc, int time )
{
    int		nDir;
    SVECTOR 	vecRot;
    Work*	work;
    CONTROL*	ctrl;
    NPCACT*	act;
		
    // local param init
    work     = (Work*)npc->character ;
    ctrl     = npc->ctrl ;
    act      = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) { 
	if ( act->current_mot != FAT_MOT_RUN_START &&
	     act->current_mot != FAT_MOT_TURN_BACK &&
	     act->current_mot != FAT_MOT_BACK_FIRE ){
	    if ( act->current_mot == FAT_MOT_RUN ) FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN_TURN );  // Front->Back
	    else				   FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN_START ); // 走り始め
	    
	}
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    if ( act->current_mot == FAT_MOT_RUN_START ){ // 走り始め
	if ( work->fFatPlayRate >= ((float)30/(float)69) ) {	
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN_TURN );  // Front -> Back
	    return;
	}
    }else if ( act->current_mot == FAT_MOT_RUN_TURN){ // Front -> Back
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BACK_FIRE );  // 逆走銃構え	
	    return;
	}
    }else if ( act->current_mot == FAT_MOT_BACK_FIRE ){ // 逆走銃構え
	if ( time % GLK_INTERVAL == 0){
	    if ( work->nBullet != 0){
		FAT_ShootGlock( npc, 0, 10, 10); 	// グロック発射
	    }
	}
	// 横一閃(少ししたに下げて)
	vecRot.vx = -32;
	vecRot.vy = (work->nBullet * 512 / GLK_BUL_NUM) - 256;
	vecRot.vz = 0;
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
	work->bAdjustFlag = 1; // アジャスト設定フラグを立てる
	if ( work->nBullet == 0){
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_TURN_TO_FRONT);  // 逆走銃構え	
	    return;
	}
    }else if ( act->current_mot == FAT_MOT_TURN_TO_FRONT ){ // 逆走銃構え
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	
	    act->act_end = 1 ;
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN );  // 逆走銃構え	
	    return;
	}
    }
    
    // 衝突判定
    Clash( npc );

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ){
	vecRot.vx = 0;
	vecRot.vy = 0;
	vecRot.vz = 0;
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
	return ;
    }
    // パッドチェック
    if ( npc->CheckPad( npc ) ){
	vecRot.vx = 0;
	vecRot.vy = 0;
	vecRot.vz = 0;
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
	return ;
    }

    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}
// 逆走撃ち
static void ActBackShotAct( NPCWORK *npc, int time )
{
    int 	nDir;
    NPCACT*	act;
    Work* 	work;
    CONTROL*	ctrl;

    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    act = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF ); // NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );		      // FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_ShootGlock( npc, 0, 10, 10); 	// グロック発射
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    // 衝突判定
    Clash( npc );

    if ( npc->CheckDamage( npc ) ) return;	// ダメージチェック
    if ( npc->CheckPad( npc ) ) return;		// パッドチェック

    if ( time == 4 ) {	// タイムアウト
	NPC_SetActMode( npc, ActBackShotReady );
	return ;
    }
    // 進行方向
    if ( FAT_GetEvadeVector( work, &nDir) ){
	if ( GV_DiffDirAbs( work->pl_dir, npc->navi->next_dir ) < 512){
	    ctrl->turn.vy = nDir;
	}else{
	    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
	}
    }else{
	if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
    }
}
// 装填
static void ActReload( NPCWORK *npc, int time)
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action ;
    work = (Work*)npc->character;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ); 	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RELOAD);	// 装填モーション
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nCartStep = CART_STEP_RELEASE;	// カートリッジコントロール用ステップ
    }

    // カートリッジ
    switch ( work->nCartStep ){
    case CART_STEP_RELEASE: 	// 外す
	if ( work->fFatPlayRate >= GLK_CART_OUT ) {		
	    work->nCartCtrl = 4; 
	    work->nCartStep = CART_STEP_HOLD;
	}
	break;
    case CART_STEP_HOLD: 	// 手に持つ
	if ( work->fFatPlayRate >= GLK_CART_APPEAR ){
	    work->nCartCtrl = 2; // 
	    work->nCartStep = CART_STEP_RELOAD;
	}
	break;
    case CART_STEP_RELOAD: 	// 装填
	work->nCartCtrl = 2; 
	if ( work->fFatPlayRate >= GLK_RELOAD_TIME){
	    work->wctrl.flag = WPEF_FLG_START;
	    work->nBullet = GLK_BUL_NUM;	
	    work->nCartCtrl = 1; // 装着
	    work->nCartStep = CART_STEP_END;
	}
	break;
    case CART_STEP_END: 	// 終了
	break;
    }
    if ( work->fFatPlayRate < GLK_RELOAD_TIME ) {
	work->wctrl.flag = WPEF_FLG_START; 
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ){
	work->nCartCtrl = 0; // 装着
	return;
    }
    // パッドチェック
    if ( npc->CheckPad( npc ) ){
	work->nCartCtrl = 0; // 装着
	return;
    }

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	act->act_end = 1 ;
	NPC_SetActMode( npc, ActGunStand );	// 構え状態
	return ;
    }
    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;
}
// くらくら起き上がり
#define FAT_WAKEUP_SAY 		(30)
static void ActWakeup( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work*)npc->character;
    act = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_DAMAGE ); 	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_HEAD );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->damage_mar, act->set_mot ); // 起き上がり
	work->fFatMotSpeed = 0.5f; // 再生スピード設定
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * work->fFatMotSpeed) );
	if ( act->current_mot == FAT_MOT_RISE_1 ){ // うつぶせ->頭振り振り
	    NPC_CallHeadMark( npc, HMK2_TYPE_PIYO);
	}
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_SQUAT);
    }

    // ダメージ : 頭部のみ
    if ( npc->CheckDamage( npc ) ){
	work->nInviCntr = INVINCIBLE_HEAD;	// 無敵時間セット
	NPC_CallHeadMark( npc, HMK2_TYPE_KILL);
	return;
    }

    if ( act->current_mot == FAT_MOT_RISE_1 ){ // うつぶせ->頭振り振り
	NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    FAT_SetActMotion( npc, npc->damage_mar, FAT_MOT_RISE_2 ); // くらくら
	    work->fFatMotSpeed = 0.5f; // 再生スピード設定
	    MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * work->fFatMotSpeed) );

	    FAT_ClearNeedl( npc->body ); // 麻酔弾を抜く
	}
    }else if ( act->current_mot == FAT_MOT_RISE_2 ){ // 頭振り振り
	if ( time % COUNT_VMODE(10) == 0 ){
	    NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
	}
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    FAT_SetActMotion( npc, npc->damage_mar, FAT_MOT_RISE_3 ); // 体を起こす
	    NPC_CallHeadMark( npc, HMK2_TYPE_KILL);
	    if ( !work->nHeadDmgFlag){ // 頭部にダメージを受けていなかったら
		FAT_StreamRequest( work, FAT_STRM_OTTOTO_02); // 危ない危ない
	    }
	}
    }else if ( act->current_mot == FAT_MOT_RISE_3 ){ // 頭振り振り->復帰
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1;
	    work->nInviCntr = INVINCIBLE_TIME;	// 無敵時間セット
	    NPC_SetActMode( npc, NPC_ActStandStill );
	    return;
	}
    }else if ( act->current_mot == FAT_MOT_BACK_RISE ){ // あおむけ->復帰
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1;
	    work->nInviCntr = INVINCIBLE_TIME;	// 無敵時間セット
	    NPC_SetActMode( npc, NPC_ActStandStill );
	    FAT_ClearNeedl( npc->body); // 麻酔弾を抜く
	    if ( !work->nHeadDmgFlag){ // 頭部にダメージを受けていなかったら
		FAT_StreamRequest( work, FAT_STRM_OTTOTO_02); // 危ない危ない
	    }
	    return;
	}

	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 40 ) ){
	    // 怒り音声
	    if ( work->nFatInfo & FAT_INFO_BAD ||
		 work->nFatInfo & FAT_INFO_DYING ){
		if ( MAO_Rand( 2 ) == 0) FAT_Say( work, SE_FATASE01, GM_SEMODE_BOMB); // 音声:畜生
		else		     	 FAT_Say( work, SE_FATHAA01, GM_SEMODE_BOMB); // 音声:はぁはぁ
	    }else if (work->nFatInfo & FAT_INFO_WOUND) {
		if ( MAO_Rand( 10 ) < 4)
		    FAT_Say( work, SE_FATASE01, GM_SEMODE_BOMB); // 音声:畜生
	    }else{
		if ( MAO_Rand( 10 ) < 3)
		    FAT_Say( work, SE_FATASE01, GM_SEMODE_BOMB); // 音声:やるな
	    }
	}
    }
}

// 爆弾設置:停止状態
static void ActBombSetStop( NPCWORK *npc, int time )
{
    NPCACT*	act ;
    Work*	work;

    act = &npc->action;
    work     = (Work*)npc->character ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_CAPTURE_OFF ); 	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期化
    if ( time == 0 ) {
        Work*	work;
	FAT_PUTPOINT_DATA* pputData;

	work 	 = (Work *)npc->character ;
	pputData = FAT_GetCurrPutPosData(work);
//printf("おきかた = %x\n", pputData->nType);
	switch ( pputData->nType & FAT_TYPE_CHECK){
	case FAT_TYPE_UNDER: // 下置き
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_DOWN_SET ) ;
	    // ターゲットサイズ変更
	    NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_SQUAT);
	    break;
	case FAT_TYPE_STAND: // 立ち置き
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_STAND_SET ) ;
	    // ターゲットサイズ変更
	    NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
	    break;
	case FAT_TYPE_UPPER: // 伸び置き
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_STRECH_SET ) ;
	    // ターゲットサイズ変更
	    NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
	    break;
	default:
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_STAND_SET ) ;
	    // ターゲットサイズ変更
	    NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
	}
	work->bPutIndexUsed = 0;
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;

    // 爆弾設置
    if ( !work->bPutIndexUsed && 
	 ((act->current_mot == FAT_MOT_BOMB_DOWN_SET && work->fFatPlayRate >= 0.585f) ||
	 (act->current_mot == FAT_MOT_BOMB_STAND_SET && work->fFatPlayRate >= 0.704f ) ||
	 (act->current_mot == FAT_MOT_BOMB_STRECH_SET && work->fFatPlayRate >= 0.711f )) ){
	int 	nRes;
	FMATRIX mat;
	Work*	work;
	FAT_PUTPOINT_DATA* pputData;

	work 	 = (Work *)npc->character ;
	pputData = FAT_GetCurrPutPosData(work);

	mat = DG_UnitMatrix;

	work->bPutIndexUsed = 1; // 設置フラグ

	if ( pputData == NULL ){
	    mat.m[3][0] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][0] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][0]) * 0.5f;
	    mat.m[3][1] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][1] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][1]) * 0.5f;
	    mat.m[3][2] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][2] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][2]) * 0.5f;
	}else{
	    GV_VecToMat( &pputData->vecPos, &mat );
	}

	// 音声再生
	if ( work->nPutNum < 5 ){
	    FAT_Say( work, SE_FATDOKN5 + (5 - (work->nPutNum + 1)), GM_SEMODE_BOMB); 
	}else{
	    FAT_StreamRequest( work, FAT_STRM_BOMB_SET2); // 爆弾セット
	}
        nRes = SetNewFatmanC4( work, FAT_GetCurrPutPosData(work), npc->ctrl->map, work->nBombTimer); 	// 爆弾設置
    }

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// 爆弾設置:走り状態
static void ActBombSetRun( NPCWORK *npc, int time )
{
    NPCACT*	act ;
    Work*	work;

    act = &npc->action;
    work     = (Work*)npc->character ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF );	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );			// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( npc->CheckDamage( npc ) ) return ;

    if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 60) ){ // 
        int 	nRes;
	FMATRIX mat;
        Work*	work;
	FAT_PUTPOINT_DATA* pputData;

	work = (Work *)npc->character ;	
	pputData = FAT_GetCurrPutPosData(work);

	mat = DG_UnitMatrix;
	if ( pputData == NULL ){
	    mat.m[3][0] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][0] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][0]) * 0.5f;
	    mat.m[3][1] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][1] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][1]) * 0.5f;
	    mat.m[3][2] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][2] +
			   work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][2]) * 0.5f;
	}else{
	    GV_VecToMat( &pputData->vecPos, &mat );
	}
	// 音声再生
	if ( work->nPutNum < 5 ){
	    FAT_Say( work, SE_FATDOKN5 + (5 - (work->nPutNum + 1)), GM_SEMODE_BOMB); 
	}else{
	    FAT_StreamRequest( work, FAT_STRM_BOMB_SET2); // 爆弾セット
	}
        nRes = SetNewFatmanC4( work, FAT_GetCurrPutPosData(work), npc->ctrl->map, work->nBombTimer); 	// 爆弾設置
    }

    if ( work->nRollerBreak ){ // ローラーブレード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }
    
    // 衝突判定
    Clash( npc );

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad );
	return ;
    }
}
// 爆弾設置:走り状態
static void ActBombSetTurn( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act ;
    CONTROL*	ctrl;

    work     = (Work*)npc->character ;
    act = &npc->action ;
    ctrl = npc->ctrl;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF );	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );			// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->bPutIndexUsed = 0;
    }

    if ( work->nRollerBreak ){ // ローラーブレード破損チェック
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * FAT_BREAK_SPEED) ) ;
    }

    if ( act->current_mot == FAT_MOT_BOMB_RUNSET_L ||
	 act->current_mot == FAT_MOT_BOMB_RUNSET_R ){ // 設置モーション
	if ( !work->bPutIndexUsed && work->fFatPlayRate >= 0.45f ){ // 設置
	    int 		nRes;
	    FMATRIX 		mat;
	    FAT_PUTPOINT_DATA* 	pputData;

	    work->bPutIndexUsed = 1; // 設置したフラグ
	    pputData = FAT_GetCurrPutPosData(work); // 設置座標を取得

	    mat = DG_UnitMatrix;
	    if ( pputData == NULL ){
		mat.m[3][0] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][0] +
			       work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][0]) * 0.5f;
		mat.m[3][1] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][1] +
			       work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][1]) * 0.5f;
		mat.m[3][2] = (work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3][2] +
			       work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3][2]) * 0.5f;
	    }else{
		GV_VecToMat( &pputData->vecPos, &mat );
	    }

	    work->bPutFlag = 1;	// 設置フラグ
	    // 音声再生
	    if ( work->nPutNum < 5 ){ 
		FAT_Say( work, SE_FATDOKN5 + (5 - (work->nPutNum + 1)), GM_SEMODE_BOMB); // カウント
	    }else{
		FAT_StreamRequest( work, FAT_STRM_BOMB_SET2); // 爆弾セット
	    }
	    nRes = SetNewFatmanC4( work, FAT_GetCurrPutPosData(work), npc->ctrl->map, work->nBombTimer); // 爆弾設置
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BACK_RUN);
	}
    }else if (act->current_mot == FAT_MOT_BACK_RUN){ // 後ろ走行
	if ( work->fFatPlayRate > 0.3f ) {
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_TURN_TO_FRONT);
	}
    }else if (act->current_mot == FAT_MOT_TURN_TO_FRONT){ // 前へ
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad );
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_RUN); // 走行へ
	    return ;
	}
    }
    
    
    // 衝突判定
    Clash( npc );


    if ( npc->CheckDamage( npc ) ) return ;

    // 進行方向
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;

}
// 爆弾起動
static void ActCountDownStart( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action;
    work = (Work *)npc->character;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }
#if 0
    if ( work->fFatPlayRate <= 0.72380f ){ // ボタン押すまで無敵
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF );	// 無敵
    }
#endif
    // 起動
    if ( !work->bBombActive && work->fFatPlayRate >= 0.72380f ) { // スイッチを押す瞬間
	Work* work;
	work = (Work*)npc->character;
	work->bBombStartFlag = 1; // フラグオン
        GM_SeSetMode( SD_E_C4SW11, &npc->ctrl->mov, GM_SEMODE_BOMB); // スイッチ音
	FAT_StartBombCount( work );
    }

    if ( npc->CheckDamage( npc ) ){
	return ;
    }


    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// ダメージによる爆弾起動
static void ActCountDownStartFromDmg( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action;
    work = (Work *)npc->character;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START) ;
	FAT_SetActMotionEX( npc, LAYER_OVER1, npc->base_mar, FAT_MOT_FIRE_P, MOTION_MASK_ARM_R, 
			    FAT_INTERP_DEF, (float)TIME_BASE ); // 右手は撃ちモーション
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nShootCntr = MAO_GetRandom(10, 20);
//printf("てすとーーーーーーーーーーーー\n");
    }
    FAT_AdjustAimPoint( work, &GM_PlayerPosition);

    // 武器発射
    if ( time >= COUNT_VMODE(40) && time % 3 == 0 && work->nShootCntr > 0) {
	FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
	work->nShootCntr--;
    }

#if 0
    if ( work->fFatPlayRate <= 0.72380f ){ // ボタン押すまで無敵
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF );	// 無敵
    }
#endif

    // 起動
    if ( !work->bBombActive && work->fFatPlayRate >= 0.72380f ) { // スイッチを押す瞬間
	Work* work;
	work = (Work*)npc->character;
	work->bBombStartFlag = 1; // フラグオン
        GM_SeSetMode( SD_E_C4SW11, &npc->ctrl->mov, GM_SEMODE_BOMB); // スイッチ音
	FAT_StartBombCount( work );
    }

    if ( npc->CheckDamage( npc ) ){
	return ;
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// 爆弾起動失敗
static void ActCountDownStartFailed( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action;
    work = (Work *)npc->character;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START_S);
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if (act->current_mot == FAT_MOT_BOMB_COUNT_START_S){ 
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START_P);
	    work->bBombStartFlag = 1; // フラグオン
	}
    }else if (act->current_mot == FAT_MOT_BOMB_COUNT_START_P){ 
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 40) ){
	    NPC_CallHeadMark( npc, HMK2_TYPE_WHT_QE ); // ？
	}
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {	
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START_E);
	}
    }else if (act->current_mot == FAT_MOT_BOMB_COUNT_START_E){ 
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return ;
	}
    }

    if ( npc->CheckDamage( npc ) ){
	return ;
    }

}
// 牽制撃ち
static void ActFeintShoot( NPCWORK *npc, int time )
{
    NPCACT*	act;
    act = &npc->action;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }
    // 武器発射
    if ( time >= 40 && time < 100 && time % 3 == 0) {
	FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) )	return;
    // 終了
    if ( npc->CheckPad( npc ) )		return;

    // 終了
    if ( time >= 120 ) {	
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return;
    }
}
// 制限撃ち
static void ActLimitShoot( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action;
    work = (Work *)npc->character;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nShootCntr = work->nShootLimit;	// 発射数設定
    }

    // 武器発射
    if ( time >= 40 && time % 3 == 0 && work->nShootCntr > 0) {
	FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
	work->nShootCntr--;
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) )	return;
    // パッドチェック
    if ( npc->CheckPad( npc ) )		return;

    // 終了
    if ( work->nShootCntr == 0 || work->nBullet == 0 ) {	
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return;
    }
}
// 飛び出し撃ち
static void ActTobidasi( NPCWORK *npc, int time )
{
    NPCACT*	act;
    act = &npc->action;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );		// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// 飛び出しモーション
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// スライド撃ち
static void ActSlideShoot( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    act = &npc->action;
    work = (Work *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージチェック    
    if ( npc->CheckDamage( npc ) ){	
	return;
    }
#if 0
    if ( act->current_mot == FAT_MOT_PEEK_START_L){
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_PEEK_FIRE_L, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }
    if ( act->current_mot == FAT_MOT_PEEK_FIRE_L){
	// 発射
	if ( time >= 45 && time < 105 && time % 3 == 0) {
	    FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
	}

	if ( time >= 105 || work->nBullet == 0){
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_PEEK_END_L, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }
    if ( act->current_mot == FAT_MOT_PEEK_END_L){
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return;
	}
    }

    if ( act->current_mot == FAT_MOT_PEEK_START_R){
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_PEEK_FIRE_R, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }
    if ( act->current_mot == FAT_MOT_PEEK_FIRE_R){
	// 発射
	if ( time >= 45 && time < 105 && time % 3 == 0) {
	    FAT_ShootGlock( npc, 0, 10, 10); // グロック発射
	}

	if ( time >= 105 || work->nBullet == 0){
	    FAT_SetActMotionEX( npc, LAYER_BASE, npc->base_mar, FAT_MOT_PEEK_END_R, MOTION_MASK_FULL, 
				FAT_INTERP_DEF, (float)TIME_BASE );
	}
    }
    if ( act->current_mot == FAT_MOT_PEEK_END_R){
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return;
	}
    }
#else
	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return;
	}
#endif
}

// 体当り
static void ActBodyAttack( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work*)npc->character;
    act  = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF );
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// モーション再生スピード
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * 1.2f) ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }
    
    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    if ( act->current_mot == FAT_MOT_RUN_START){
	// 攻撃判定
	if ( time >= 5 ){
	    Attack( npc );
	}
	// 停止
	if ( time >= 45){
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_STOP );	
	}
    }else if ( act->current_mot == FAT_MOT_STOP){
	// 攻撃判定 : 再生レートで判定
	if ( work->fFatPlayRate < 0.5f ){
	    Attack( npc );
	}

	// モーション再生終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1;
	    NPC_SetActMode( npc, NPC_ActStandStill );
	    return ;
	}
    }

    // 進行方向更新
    if ( act->dir > 0 ) npc->ctrl->turn.vy = act->dir;
}
// 見渡す
static void ActOverLook( NPCWORK *npc, int time )
{
    NPCACT*	act;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return;
    // 終了
    if ( npc->CheckPad( npc ) )	   return;
    
    // 終了
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill );
	}
	return;
    }

}
// 息切れ
#define BLEATHLESS_JP_VE_TIME  	(COUNT_VMODE(900)) // 息切れでばてている時間
#define BLEATHLESS_JP_EA_TIME  	(COUNT_VMODE(600)) // 息切れでばてている時間
#define BLEATHLESS_TIME  	(COUNT_VMODE(360)) // 息切れでばてている時間
#define BLEATHLESS_SAY_INTV  	(COUNT_VMODE(90))  // 息切れ音
static void ActBleathless( NPCWORK *npc, int time )
{
    int		nBleathTime;
    float 	fSpeed;
    Work*	work;		// ファットマンワーク
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action ;

//#ifdef JAPANESE_BP_IGNORE()
#if 1
    // ばてばて:隙をつくる
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nBleathTime = BLEATHLESS_JP_VE_TIME;
	break;
    case GM_LEVEL_EASY:
	nBleathTime = BLEATHLESS_JP_EA_TIME;
	break;
    default:
	nBleathTime = BLEATHLESS_TIME;
    }
#else
    nBleathTime = BLEATHLESS_TIME;
#endif
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ); //
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_STAND );
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    fSpeed = 1.f + (float)(1.f - time / nBleathTime) * 3.f;
    MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE * fSpeed ); // 遅くしてみる
    if ( time % BLEATHLESS_SAY_INTV == 0 ) {
	FAT_Say( work, SE_FATHAA01, GM_SEMODE_BOMB); // 息切れ
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    if ( time >= nBleathTime){
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill );
	return ;
    }
}

// 踊る
static void ActOdori( NPCWORK *npc, int time )
{
    NPCACT*	act;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_CAPTURE_OFF ); //
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );
	MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.5f ); // 遅くしてみる
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    // 終了
    if ( npc->CheckPad( npc ) ) return;

    // モーション再生終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill );
	return ;
    }
}
// 座る
static void ActSquat( NPCWORK *npc, int time )
{
    NPCACT*	act;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ); //
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );
	MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*0.5f ); // 遅くしてみる
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_SQUAT);
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;

    // モーション再生終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill );
	return ;
    }
}
// エルード指きり
static void ActEludeKill( NPCWORK *npc, int time )
{
    NPCACT*	act;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_CAPTURE_OFF );
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// モーション再生
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * 1.2f) ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージ
    if ( npc->CheckDamage( npc ) ) return;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return;

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}

// ホールドアップ
static void ActHoldUp( NPCWORK *npc, int time )
{
    NPCACT*	act;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND|NPC_ACT_STATUS_TRG_OFF ); //
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_STAND );
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // ダメージ
//    if ( npc->CheckDamage( npc ) ) return;

}
// 解体フェーズ::ダンス
static void ActCircleDance( NPCWORK *npc, int time )
{
    NPCACT	*act ;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	// 円ダンス
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // 再生速度アップ
    if ( act->current_mot == FAT_MOT_DANCE_1 ){
	MT_SetMotionSpeed( npc->body->m_ctrl, (( float )TIME_BASE * ( 1.5f * work->fFatPlayRate + 0.5f )) );
    }

    // チェックダメージ
    if ( npc->CheckDamage( npc ) ){
	return;
    }
    // パッドチェック
    if ( npc->CheckPad( npc ) ){
	return ;
    }

    // 衝突判定
    Clash( npc );        

    // 方向転換時の体の傾き 
    FAT_Incline( ctrl );

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}

// 解体フェーズ::カモメ
static void ActKamome( NPCWORK *npc, int time )
{
    NPCACT*	act;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ){
	FAT_SetActMotion( npc, npc->base_mar, act->set_mot );	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    // 終了
    if ( act->current_mot == FAT_MOT_BOMB_COUNT_START_S ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    FAT_SetActMotion( npc, npc->base_mar, FAT_MOT_BOMB_COUNT_START_P);
	}
    }else{

    }

    // チェックダメージ
    if ( npc->CheckDamage( npc ) ) return ;

    // 新動作が発動
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }

    // 方向
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}
// ローラーブレード修理
static void ActRepair( NPCWORK *npc, int time )
{
    NPCACT	*act ;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }
    
    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return ;

    // 終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }

    // 方向更新
    //    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}
// 銃撃フェーズスタート
static void ActShootPhaseStart( NPCWORK *npc, int time )
{
    NPCACT	*act ;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;

	work->nFireCurrent = 0;
	work->nFireIntv    = COUNT_VMODE(40);
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nBullet = GLK_BUL_NUM;	// 誤魔化しで装填しておく
	work->nCartStep = CART_STEP_RELEASE;
    }
    
    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return ;

    // 装填
    if ( work->fFatPlayRate >= GLK_START_RELOAD){
	work->wctrl.flag = WPEF_FLG_START;
    }

    if ( act->current_mot == FAT_MOT_GUN_PHASE_START){ // フェーズ開始合図モーション
	// 終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    FAT_SetActMotion( npc, act->set_mar, FAT_MOT_FIRE_P ); 
	}
    }else{ // 銃構えモーション
	SVECTOR vecRot;
	vecRot.vx = 32;
	vecRot.vy = ((work->nFireCurrent * 256 / 8) - 128) / 4;
	vecRot.vz = 0;
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_KATA );
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE1 );
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_UDE2 );
	GM_AdjustRotBody( &work->body, &vecRot, HUMAN21_MIGI_TE );
	work->bAdjustFlag = 1; // アジャスト設定フラグを立てる
	
	if ( work->nFireCurrent < 8 && work->nFireIntv-- == 0){
//	    FAT_SetActMotion( npc, act->set_mar, FAT_MOT_FIRE_END ); 
  	    work->nFireIntv = 3;
	    FAT_ShootGlock( npc, 0, 0, 0); // 発射
	    work->nFireCurrent++;
	}

	// 終了
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return ;
	}
    }
    // 方向更新
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}

// 爆弾フェーズスタート
static void ActBombPhaseStart( NPCWORK *npc, int time )
{
    NPCACT	*act ;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	work->nRollerBreak = 0; // ローラーブレード復活
    }
    
    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;
    // パッドチェック
    if ( npc->CheckPad( npc ) ) return ;

    // 装填
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
    // 方向更新
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}

// 背後に立つ
static void ActBackNavi( NPCWORK *npc, int time )
{
    NPCACT	*act ;
    CONTROL*	ctrl;
    Work*	work;

    act = &npc->action ;
    ctrl     = npc->ctrl ;
    work     = (Work*)npc->character ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分

    if ( time % COUNT_VMODE(180) == COUNT_VMODE(60) ){
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE);	// 目パチ
    }

    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }
    
    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;

    // 思考側からの終了チェック
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    ctrl->turn.vz = 0 ;	// 体の傾きを止める	
	    NPC_SetActMode( npc, NPC_ActStandStill );
	}
	return;
    }

    // 方向更新
    if ( act->dir > 0 ) ctrl->turn.vy = act->dir;
}


/*-------------------------------------------------------------*/
static int	SampleCheckPad( NPCWORK *npc )
{
    NPCACT*	act ;

    act = &npc->action ;
    if ( act->pad == PAD_NONE ) return 0 ;

    switch ( act->pad ) {
    case PAD_RUN_ACT:	 	// ローラーブレード走行
	if ( act->current_mot != FAT_MOT_RUN ) 
	    FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN_START, act->pad );
	else				       
	    FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad );
	break ;
    case PAD_TURN_LEFT:		// 旋回(左)
	FAT_SetModeFromPad( npc, ActTurnSide, npc->base_mar, FAT_MOT_TURN_L, act->pad );
	break;
    case PAD_TURN_RIGHT:	// 旋回(右)
	FAT_SetModeFromPad( npc, ActTurnSide, npc->base_mar, FAT_MOT_TURN_R, act->pad );
	break;
    case PAD_REVERSE:		// 急ターン
	FAT_SetModeFromPad( npc, ActTurn180, npc->base_mar, FAT_MOT_REVERSE, act->pad );
	break;
    case PAD_TURN_BACK:		// 旋回(後ろへ)
	FAT_SetModeFromPad( npc, ActTurn180, npc->base_mar, FAT_MOT_RUN_TURN, act->pad );
	break;
    case PAD_TURN_FRONT:	// 旋回(前へ)
	FAT_SetModeFromPad( npc, ActTurn180, npc->base_mar, FAT_MOT_TURN_TO_FRONT, act->pad );
	break;
    case PAD_STOP:		// 停止
	FAT_SetModeFromPad( npc, ActStop, npc->base_mar, FAT_MOT_STOP, act->pad );
	break;
    case PAD_POINT_MOVE:	// 指定点移動
	FAT_SetModeFromPad( npc, ActPointMove, npc->base_mar, act->current_mot, act->pad );
	break;
    case PAD_SHOT_READY:	// 銃構え:静止
	FAT_SetModeFromPad( npc, ActGunStand, npc->base_mar, FAT_MOT_FIRE_P, act->pad ) ;
	break ;
    case PAD_SHOT_ACT:		// 銃発射:静止
	FAT_SetModeFromPad( npc, ActGunShoot, npc->base_mar, FAT_MOT_FIRE, act->pad ) ;
	break ;
    case PAD_RUNSHOT_READY:	// 銃構え:走り
	FAT_SetModeFromPad( npc, ActRunShotReady, npc->base_mar, FAT_MOT_RUN_FIRE, act->pad ) ;
	break ;
    case PAD_RUNSHOT_ACT:	// 銃発射:走り
	FAT_SetModeFromPad( npc, ActRunShotAct, npc->base_mar, FAT_MOT_RUN_FIRE, act->pad ) ;
	break ;
    case PAD_BACKSHOT_READY:
	FAT_SetModeFromPad( npc, ActBackShotReady, npc->base_mar, FAT_MOT_RUN, act->pad ) ;
	break ;
    case PAD_BACKSHOT_ACT:	// 銃発射:逆走
	FAT_SetModeFromPad( npc, ActBackShotAct, npc->base_mar, FAT_MOT_RUN, act->pad ) ;
	break ;
    case PAD_RELOAD:		// 装填
	FAT_SetModeFromPad( npc, ActReload, npc->base_mar, FAT_MOT_RELOAD, act->pad ) ;
	break;
    case PAD_RUN_RELOAD:	// 走行中に装填
	FAT_SetModeFromPad( npc, ActRunReload, npc->base_mar, FAT_MOT_RUN_FIRE, act->pad ) ;
	break;
    case PAD_SET_BOMB_STOP :	// 停止置き
	FAT_SetModeFromPad( npc, ActBombSetStop, npc->base_mar, FAT_MOT_BOMB_SET, act->pad ) ;
	break;
    case PAD_SET_BOMB_RUN :	// 走り置き（旧）
	FAT_SetModeFromPad( npc, ActBombSetRun, npc->base_mar, FAT_MOT_BOMB_RUNSET, act->pad ) ;
	break;
    case PAD_SET_BOMB_TURN_L :	// 回転置き:左
	FAT_SetModeFromPad( npc, ActBombSetTurn, npc->base_mar, FAT_MOT_BOMB_RUNSET_L, act->pad ) ;
	break;
    case PAD_SET_BOMB_TURN_R :	// 回転置き:右
	FAT_SetModeFromPad( npc, ActBombSetTurn, npc->base_mar, FAT_MOT_BOMB_RUNSET_R, act->pad ) ;
	break;
    case PAD_COUNTDOWN_START:	// 爆弾起動
	FAT_SetModeFromPad( npc, ActCountDownStart, npc->base_mar, FAT_MOT_BOMB_COUNT_START, act->pad ) ;
	break;
    case PAD_COUNTDOWN_STARTDMG:// ダメージによる爆弾起動
	FAT_SetModeFromPad( npc, ActCountDownStartFromDmg, npc->base_mar, FAT_MOT_BOMB_COUNT_START, act->pad ) ;
	break;
    case PAD_COUNTDOWN_START_FAILED:	// 爆弾起動失敗
	FAT_SetModeFromPad( npc, ActCountDownStartFailed, npc->base_mar, FAT_MOT_BOMB_COUNT_START_S, act->pad ) ;
	break;
    case PAD_WAKEUP:		// 起き上がり
	FAT_SetModeFromPad( npc, ActWakeup, npc->damage_mar, FAT_MOT_RISE_1, act->pad ) ;
	break ;
    case PAD_FEINT_SHOOT:	// 牽制撃ち
	FAT_SetModeFromPad( npc, ActFeintShoot, npc->base_mar, FAT_MOT_FIRE, act->pad);
	break;
    case PAD_LIMIT_SHOOT:	// 制限撃ち
	FAT_SetModeFromPad( npc, ActLimitShoot, npc->base_mar, FAT_MOT_FIRE, act->pad);
	break;
    case PAD_TOBIDASI_L:
	FAT_SetModeFromPad( npc, ActTobidasi, npc->damage_mar, FAT_MOT_FIRE, act->pad ) ;
	break;
    case PAD_TOBIDASI_R:
	FAT_SetModeFromPad( npc, ActTobidasi, npc->damage_mar, FAT_MOT_FIRE, act->pad ) ;
	break;
    case PAD_SLIDE_SHOOT_L:
	FAT_SetModeFromPad( npc, ActSlideShoot, npc->base_mar, FAT_MOT_FIRE, act->pad);
	break;
    case PAD_SLIDE_SHOOT_R:
	FAT_SetModeFromPad( npc, ActSlideShoot, npc->base_mar, FAT_MOT_FIRE, act->pad);
	break;
    case PAD_BODYATTACK:	// 体当り
	FAT_SetModeFromPad( npc, ActBodyAttack, npc->base_mar, FAT_MOT_RUN_START, act->pad);
	break;
    case PAD_OVERLOOK:		// 見渡す
	FAT_SetModeFromPad( npc, ActOverLook, npc->base_mar, FAT_MOT_FIRE_P, act->pad);
	break;
    case PAD_BLEATHLESS:	// 息切れ
	FAT_SetModeFromPad( npc, ActBleathless, npc->base_mar, FAT_MOT_STAND, act->pad);
	break;
    case PAD_ODORI:
	FAT_SetModeFromPad( npc, ActOdori, npc->base_mar, FAT_MOT_FIRE, act->pad ) ;
	break;
    case PAD_SQUAT:
	FAT_SetModeFromPad( npc, ActSquat, npc->base_mar, FAT_MOT_FIRE, act->pad ) ;
	break;
    case PAD_HOLDUP:
	FAT_SetModeFromPad( npc, ActHoldUp, npc->base_mar, FAT_MOT_FIRE, act->pad ) ;
	break;
    case PAD_REPAIR:
	FAT_SetModeFromPad( npc, ActRepair, npc->base_mar, FAT_MOT_ROLLER_REPAIR, act->pad ) ;
	break;
    case PAD_CIRCLE_DANCE:
	FAT_SetModeFromPad( npc, ActCircleDance, npc->base_mar, FAT_MOT_DANCE_1, act->pad ) ;
	break;
    case PAD_CIRCLE_DANCE2:
	FAT_SetModeFromPad( npc, ActCircleDance, npc->base_mar, FAT_MOT_DANCE_2, act->pad ) ;
	break;
    case PAD_KAMOME:
	FAT_SetModeFromPad( npc, ActKamome, npc->base_mar, FAT_MOT_BOMB_COUNT_START_S, act->pad ) ;
	break;
    case PAD_BACKNAVI:
	FAT_SetModeFromPad( npc, ActBackNavi, npc->base_mar, FAT_MOT_FIRE_P, act->pad ) ;
	break;
    case PAD_ELUDE_KILL:
	FAT_SetModeFromPad( npc, ActEludeKill, npc->base_mar, FAT_MOT_REVERSE, act->pad ) ;
	break;
    case PAD_SHOOTPHASE_START:	// 銃撃フェーズ開始
	FAT_SetModeFromPad( npc, ActShootPhaseStart, npc->base_mar, FAT_MOT_GUN_PHASE_START, act->pad ) ;
	break;
    case PAD_BOMBPHASE_START:	// 爆弾フェーズ開始
	FAT_SetModeFromPad( npc, ActBombPhaseStart, npc->base_mar, FAT_MOT_BOMB_PHASE_START, act->pad ) ;
	break;
    }
    return 1 ;
}

/*----- ダメージアクション -----------------------------------------------------*/
static void ActDamage( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action;

    if ( work->nInviCntr > 0 || time > 30){
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | NPC_ACT_STATUS_CAPTURE_OFF ) ; // 無敵
    }else{
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE); 
    }
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    if ( npc->CheckDamage( npc ) ) return;

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
// 走行ダメージ
static void ActRunDamage( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action;

    if ( work->nInviCntr > 0 || time > 30){
	NPC_ActStatus( act, NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | NPC_ACT_STATUS_CAPTURE_OFF ) ; // 無敵
    }else{
	NPC_ActStatus( act, NPC_ACT_STATUS_MOVE | NPC_ACT_STATUS_DAMAGE); 
    }
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_RUN );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    if ( time == 0 ) {
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);
    }

    { // 移動計算
#if 1
	FVECTOR vecDistance;
	FMATRIX mat;

	DG_SetPos2( &DG_ZeroVector, &npc->ctrl->rot);
	DG_GetPos( &mat);

	vecDistance.vx = 0.f;
	vecDistance.vy = 0.f;
	vecDistance.vz = work->nMoveDist * 0.9f;
	vecDistance.vw = 1.f;
	_sceVu0ApplyMatrix( &npc->ctrl->step, &mat, &vecDistance );
#else
	_sceVu0CopyVector( &npc->ctrl->step, &work->vecPreDmgStep);
#endif
    }

    // 衝突判定 : 日本語版は切るかも
#if 0
    if ( GV_DiffDirAbs( work->pl_dir, npc->ctrl->rot.vy ) < 512){
	Clash( npc );        
    }
#else
    Clash( npc );        
#endif

    if ( npc->CheckDamage( npc ) ) return;

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1 ;
	FAT_SetModeFromPad( npc, ActRun, npc->base_mar, FAT_MOT_RUN, act->pad ) ;
	return ;
    }
}
// 倒れる
static void ActFall( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | 
		   NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_IK_DOWN) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    if ( time == 0 ) {
	// モーション再生
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;

	// ふっとばし方向
	if ( act->current_mot == FAT_MOT_DAM_BOMB ){
	    npc->ctrl->turn.vy = npc->ctrl->rot.vy = work->pl_dir;
	}	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);

	work->nHeadDmgFlag = 0; // 頭部のダメージフラグクリア
    }

    // めりこみ回避
    FAT_EvadeIntoWall( npc);

    // チェックダメージ
    if ( npc->CheckDamage( npc ) ) return ;

    // 振動
    if ( act->current_mot == FAT_MOT_BLAST_DAM_3 ){ // ブラストスーツ耐久限界
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 71) ){
	    FAT_Quake( work, 10, 128, 12);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_STUN ){ // スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 63) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_RUN_STUN ){ // 走行時スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 54) ){
	    FAT_Quake( work, 10, 256, 20);
	}
    }else if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 17) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){ // 走行時ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 7) ){
	    FAT_Quake( work, 10, 256, 18);
	}else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 31) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_BOMB ){ // 爆弾ふっ飛び
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 15) ){
	    FAT_Quake( work, 10, 256, 15);
	}else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 39) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){
	    FAT_SetModeFromPad( npc, ActDown, npc->base_mar, FAT_MOT_DOWN, act->pad ) ;
	}else{
	    FAT_SetModeFromPad( npc, ActDown, npc->base_mar, FAT_MOT_DOWN_FRONT, act->pad ) ;
	}
	return ;
    }
}
// 走行時に倒れるもので移動をつけたいもの
static void ActRunFall( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | 
		   NPC_ACT_STATUS_CAPTURE_OFF) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    if ( time == 0 ) {
	// モーション再生
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;

	// ふっとばし方向
	if ( act->current_mot == FAT_MOT_DAM_BOMB ){
	    npc->ctrl->rot.vy = work->pl_dir;
	    npc->ctrl->turn.vy = npc->ctrl->rot.vy;
	}	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);
    }

    // めりこみ回避
    FAT_EvadeIntoWall( npc);

    // チェックダメージ
    if ( npc->CheckDamage( npc ) ) return ;

    // 振動
    if ( act->current_mot == FAT_MOT_BLAST_DAM_3 ){ // ブラストスーツ耐久限界
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 71) ){
	    FAT_Quake( work, 10, 128, 12);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_STUN ){ // スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 63) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_RUN_STUN ){ // 走行時スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 54) ){
	    FAT_Quake( work, 10, 256, 20);
	}
    }else if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 17) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){ // 走行時ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 7) ){
	    FAT_Quake( work, 10, 256, 18);
	}else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 31) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_BOMB ){ // 爆弾ふっ飛び
	if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 34) ){
	    FAT_Quake( work, 10, 256, 15);
	}else if ( MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 5 * 52) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }
#if 0
    if ( act->current_mot == FAT_MOT_BLAST_DAM_3 ){ // 移動計算
	FVECTOR vecDistance;
	FMATRIX mat;

	DG_SetPos2( &DG_ZeroVector, &npc->ctrl->rot);
	DG_GetPos( &mat);

	vecDistance.vx = 0.f;
	vecDistance.vy = 0.f;
	if ( abs( npc->ctrl->rot.vy - work->vecKoshiRot.vy) < 1024 ) vecDistance.vz =  work->nMoveDist * 0.9f;
	else							     vecDistance.vz = -work->nMoveDist * 0.9f;
	vecDistance.vw = 1.f;
	_sceVu0ApplyMatrix( &npc->ctrl->step, &mat, &vecDistance );
    }
#endif

    if ( act->current_mot == FAT_MOT_ROLLER_CRASH){ // 移動計算
	FVECTOR vecDistance;
	FMATRIX mat;

	DG_SetPos2( &DG_ZeroVector, &npc->ctrl->rot);
	DG_GetPos( &mat);

	vecDistance.vx = 0.f;
	vecDistance.vy = 0.f;
	vecDistance.vz = -((float)work->nMoveDist * 0.5f);
	vecDistance.vw = 1.f;
	_sceVu0ApplyMatrix( &npc->ctrl->step, &mat, &vecDistance );
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){
	    FAT_SetModeFromPad( npc, ActDown, npc->base_mar, FAT_MOT_DOWN, act->pad ) ;
	}else{
	    FAT_SetModeFromPad( npc, ActDown, npc->base_mar, FAT_MOT_DOWN_FRONT, act->pad ) ;
	}
	return ;
    }
}
// ダウン:::当たり判定なし
static void ActDownDamage( NPCWORK *npc, int time )
{
    NPCACT	*act ;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | 
		   NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_IK_DOWN ); // 無敵
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN);	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    // 初期化
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->damage_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;

    // 終了チェック
    if ( time >= FAT_GetDownTime() ) {
	NPC_ActStatus( act, FAT_ACT_STATUS_DOWN_END );	// ダウン終了フラグ
	if ( act->current_mot == FAT_MOT_DOWN ){
	    FAT_SetModeFromPad( npc, ActWakeup, npc->base_mar, FAT_MOT_BACK_RISE, act->pad ) ;
	}else{
	    FAT_SetModeFromPad( npc, ActWakeup, npc->base_mar, FAT_MOT_RISE_1, act->pad ) ;
	}
	return ;
    }
}
// ダウン :: Pre-FIX
static void ActDown( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT	*act ;

    work = (Work *)npc->character;
    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_IK_DOWN); 
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    // 初期化
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->damage_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);
	{
	    GCL_ARGS  gcl_args;
	    int       data;
	
	    data = FAT_GCLINFO_FALL;
	    gcl_args.argc = 1;
	    gcl_args.argv = &data;
	    GCL_ExecProc( work->procFatInfo, &gcl_args ); // 倒れた
	}
    }
    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;

    // 終了チェック
    if ( time >= FAT_GetDownTime() ) {
	NPC_ActStatus( act, FAT_ACT_STATUS_DOWN_END );	// ダウン終了フラグ
	// 仰向け,うつぶせで分岐
	if ( act->current_mot == FAT_MOT_DOWN ){ 
	    FAT_SetModeFromPad( npc, ActWakeup, npc->base_mar, FAT_MOT_BACK_RISE, act->pad ) ;
	}else{
	    FAT_SetModeFromPad( npc, ActWakeup, npc->base_mar, FAT_MOT_RISE_1, act->pad ) ;
	}
	return ;
    }
}
// 掴みカウンター :: Pre-FIX
static void ActHang( NPCWORK *npc, int time )
{
    NPCACT*	act;
    Work*	work;
    work = (Work *)npc->character;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF | NPC_ACT_STATUS_CAPTURE_OFF ) ; // 無敵(バグが怖いので)
    if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // 押し潰し
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE );
	NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分
    }else{
	NPC_ActStatus( act, FAT_ACT_STATUS_DMG_STAND );	// FATMAN拡張分
    }

    if ( time == 0 ) {
	int nSeed;

	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_STAND);

	if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // 押し潰し
	    MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * 0.5f) ) ;
	    // 音声再生
	    nSeed = MAO_Rand( 10 );
	    if ( nSeed < 5 )		FAT_Say( work, SE_FATDMG03, GM_SEMODE_BOMB);
	    else 			FAT_Say( work, SE_FATKIA02, GM_SEMODE_BOMB);
	}else{
	    // 音声再生
	    nSeed = MAO_Rand( 10 );
	    if ( nSeed < 4 )		FAT_Say( work, SE_FATDMB01, GM_SEMODE_BOMB);
	    else if ( nSeed < 8 )	FAT_Say( work, SE_FATKIA01, GM_SEMODE_BOMB);
	    else		       	FAT_Say( work, SE_FATNOD01, GM_SEMODE_BOMB);
	}
    }

    // ふっとばし
    if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // 押し潰し
	if ( time >= COUNT_VMODE(20) && time < COUNT_VMODE(40)) {
	    CaptureCounter( npc );
	}
    }else{ // ヒップアタック
	if ( time >= COUNT_VMODE(16) && time < COUNT_VMODE(35)) {
	    CaptureCounter( npc );
	}
    }
    
    if ( npc->CheckDamage( npc ) ) return;


    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // 押し潰し
	    NPC_ActStatus( act, FAT_ACT_STATUS_DOWN_END );	// ダウン終了フラグ
	    FAT_SetModeFromPad( npc, ActWakeup, npc->base_mar, FAT_MOT_RISE_1, act->pad ) ;
	}else{
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	    return ;
	}
    }
}
// 瀕死
static void ActNearDeath( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work*)npc->character;
    act = &npc->action ;

    // 動作ステータス設定
    NPC_ActStatus( act, NPC_ACT_STATUS_CAPTURE_OFF ); 	// NPCシステム設定
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DEAD );	// FATMAN拡張分
    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ


    // 初期設定
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->damage_mar, act->set_mot ); // 起き上がり
	work->fFatMotSpeed = 0.5f; // 再生スピード設定
	MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * work->fFatMotSpeed) );

	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_SQUAT);
    }

    // ダメージ : 頭部のみ
    if ( npc->CheckDamage( npc ) ){
	work->nInviCntr = INVINCIBLE_HEAD;	// 無敵時間セット
	return;
    }

    if ( act->current_mot == FAT_MOT_RISE_1 ){ // うつぶせ->頭振り振り
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 再生終了
	    FAT_SetActMotion( npc, npc->damage_mar, FAT_MOT_RISE_DEAD ); // 四つん這い
	    FAT_Say( work, SE_FATHAA01, GM_SEMODE_BOMB); // 音声:はぁはぁ
	    work->fFatMotSpeed = 1.0f; // 再生スピード設定
	    MT_SetMotionSpeed( npc->body->m_ctrl, (float)(( float )TIME_BASE * work->fFatMotSpeed) );
	}
    }else if ( act->current_mot == FAT_MOT_RISE_DEAD ){ // 頭振り振り

    }
}
// 死亡時用ダウン:::当たり判定なし
static void ActDead( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | 
		   NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_IK_DOWN ); // 無敵
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分

    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ

    // 初期化
    if ( time == 0 ) {
	FAT_SetActMotion( npc, npc->damage_mar, act->set_mot ) ;
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);
    }

    // 終了チェック
    if ( work->bBombActive && time >= FAT_GetDownTime() ) { // 爆弾がカウントダウンしている時
	NPC_ActStatus( act, FAT_ACT_STATUS_DOWN_END );	// ダウン終了フラグ
	if ( act->current_mot == FAT_MOT_DOWN_FRONT ){ // 仰向け時だけ
	    FAT_SetModeFromPad( npc, ActNearDeath, npc->base_mar, FAT_MOT_RISE_1, act->pad ) ;
	}
	return ;
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) return ;
}

// 死亡時用倒れる
static void ActDeadFall( NPCWORK *npc, int time )
{
    Work*	work;
    NPCACT*	act;

    work = (Work *)npc->character;
    act = &npc->action;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE | NPC_ACT_STATUS_TRG_OFF | 
		   NPC_ACT_STATUS_CAPTURE_OFF | NPC_ACT_STATUS_IK_DOWN) ;
    NPC_ActStatus( act, FAT_ACT_STATUS_DMG_DOWN );	// FATMAN拡張分

    NPC_ActStatus( act, FAT_ACT_STATUS_EYE_CLPOSE);	// 目閉じ

    if ( time == 0 ) {
	// モーション再生
	FAT_SetActMotion( npc, act->set_mar, act->set_mot ) ;

	// ふっとばし方向
	if ( act->current_mot == FAT_MOT_DAM_BOMB ){
	    npc->ctrl->rot.vy = work->pl_dir;
	    npc->ctrl->turn.vy = npc->ctrl->rot.vy;
	}	
	// ターゲットサイズ変更
	NPC_ChangeTargetSize( npc, FAT_TARGET_SIZE_DOWN);
    }

    if ( !work->bBombActive && !work->bGameOver){
	GM_SetGameStatus( STATE_PRG_DEMO); // 無敵にする
    }


    // チェックダメージ
    if ( npc->CheckDamage( npc ) ) return ;

    // 振動
    if ( act->current_mot == FAT_MOT_BLAST_DAM_3 ){ // ブラストスーツ耐久限界
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 71) ){
	    FAT_Quake( work, 10, 128, 12);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_STUN ){ // スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 63) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_RUN_STUN ){ // 走行時スタングレネード転倒
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 54) ){
	    FAT_Quake( work, 10, 256, 20);
	}
    }else if ( act->current_mot == FAT_MOT_ROLLER_CRASH ){ // ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 17) ){
	    FAT_Quake( work, 10, 164, 14);
	}
    }else if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){ // 走行時ローラーブレード破損
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 7) ){
	    FAT_Quake( work, 10, 256, 18);
	}else if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 31) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }else if ( act->current_mot == FAT_MOT_DAM_BOMB ){ // 爆弾ふっ飛び
	if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 34) ){
	    FAT_Quake( work, 10, 256, 15);
	}else if ( MT_CHECK_MOTION_TIME( npc->body->m_ctrl, 0, 5 * 52) ){
	    FAT_Quake( work, 5, 64, 10);
	}
    }

    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	work->bCallClearProc = 1; // クリアプロシージャコール可能

	if ( act->current_mot == FAT_MOT_RUN_ROLLER_CRASH ){
	    FAT_SetModeFromPad( npc, ActDead, npc->base_mar, FAT_MOT_DOWN, act->pad ) ;
	}else{
	    FAT_SetModeFromPad( npc, ActDead, npc->base_mar, FAT_MOT_DOWN_FRONT, act->pad ) ;
	}
	return ;
    }
}

/*----------------------------------------------------------------*/
static void Action( Work *work )
{
    NPCWORK	*npc;

    npc = &work->npc;

//  NPC_ActInit( npc ) ;
    // 思考で利用するフラグ	
    work->bPutFlag    = 0;	// 設置フラグクリア

    NPC_Action( npc );

    // 無敵チェック
    if (work->nInviCntr > 0){
	NPC_ActStatus( &work->npc.action, NPC_ACT_STATUS_TRG_OFF ); // 無敵にする
    }

    // ステータスチェック
    NPC_ActStatusCheck( npc );

    // 重力チェック
    NPC_Gravitation( npc );
}

