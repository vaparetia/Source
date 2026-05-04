/*
    paract.c
    オウム動作処理
    2001/05/16 Masafumi Okuta
    $Id: paract.c,v 1.1.1.3 2002/11/19 11:48:05 Yoshizawa1 Exp $
*/
static int TBL_IDLE_PAR[PAR_MOT_MAX]   = { 40, 10, 10, 10, 10, 10, 10 };
static int TBL_ATTENT_PAR[PAR_MOT_MAX] = { 40, 10, 10, 20, 20, 0, 0 };
//static int TBL_CHGDIR_PAR[PAR_MOT_MAX] = { 40, 10, 10, 20, 20, 0, 0 };

// ターゲットに向ける
static void PAR_AdjustAimPoint( PARROT*		work,	
				FVECTOR*	pvecTrg)	// 目標地点
{
    SVECTOR 	vecRot;
    NPCWORK*	npc;
    NPCACT*	act;

    npc = &work->npc;
    act = &work->npc.action;
    
    // vecTrgに向くための回転角度を求める
    GM_TrgToNearRotLimit( &work->npc.ctrl->mov, work->npc.ctrl->rot.vy, pvecTrg, &work->svecAdjust ) ;

    // モーションをいじって体をターゲット方向に向ける
    vecRot.vx = work->svecAdjust.vx / 3;
    vecRot.vy = work->svecAdjust.vy / 3;
    vecRot.vz = work->svecAdjust.vz / 3;

    //    GM_AdjustRotBody( &work->body, &vecRot, 2 );
    GM_AdjustRotBody( &work->body, &vecRot, 11 );
    GM_AdjustRotBody( &work->body, &vecRot, 12 );
    GM_AdjustRotBody( &work->body, &vecRot, 21 );
}
// アジャストクリア
static void PAR_ClearAdjust( PARROT* work)
{
    SVECTOR 	vecRot;
    vecRot = DG_ZeroSVector;
    //    GM_AdjustRotBody( &work->body, &vecRot, 2 );
    GM_AdjustRotBody( &work->body, &vecRot, 11 );
    GM_AdjustRotBody( &work->body, &vecRot, 12 );
    GM_AdjustRotBody( &work->body, &vecRot, 21 );
}

// モーションランダム再生
static int PAR_GetNextMot( int* pnPar) // 確率
{
    int i;
    int nSeed = MAO_Rand( 100 );
    int nMax = 0;

    for ( i = 0; i < PAR_MOT_MAX; i++){
	nMax += pnPar[i];	// 最大値更新
	if ( nSeed < nMax ){
	    return (i);
	}
    }

    return 0;
}

// オウムターゲットコールバック : フォーチュンバリア
static void PAR_TargetCallback( TARGET *off, TARGET *def, void *ptr )
{
    PARROT *work = (PARROT *)ptr ;
    extern void *NewFortBulletCalled( int id, float rot_vy, float speed,
				      FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid, int life ) ;
    extern void *NewFortSplineBulletCalled( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *avoid ) ;
    float d ;

    if ( def->damaged & TARGET_POWER ){
	FVECTOR vecSub;

	GM_ClearTargetDamage( def ) ;

	/* 弾避けが呼ばれる */
        if ( off->weapon_type & (WP_BULLET|WP_M92|WP_PSG1|WP_PSG1T) ) {
	    /* 表示する弾の位置を計算 */
	    _sceVu0SubVector( &vecSub,
			      &def->hit,
			      &GM_PlayerControl->mov ) ;
	    d = _sceVu0InnerProduct( &vecSub, &vecSub ) ;
	    d = sceVu0Sqrt( d ) ;
	    d = d>3000.0f ? 3000.0f : d ;

	    _sceVu0Normalize( &vecSub, &off->power->force ) ;
	    _sceVu0ScaleVector( &vecSub, &vecSub, d ) ;
	    _sceVu0SubVector( &vecSub, &def->hit, &vecSub ) ;

	    NewFortSplineBulletCalled( &vecSub, &off->power->force,
				       (FVECTOR*)&BODYWORLD( &work->body, HUMAN21_KOSHI ).m[3][0] ) ;

	    GM_SeSetMode( SD_A_RICFOR01, &work->control.mov, GM_SEMODE_BOMB ) ;


	    // 攻撃受けたカウンタ更新
	    work->nAttackNum++;
	    if ( work->nAttackNum >= PAR_CALL_ENE){
		PAR_SetThink3( work, TH3_ENEMY_CALL);	// 敵兵を呼ぶ
		work->nAttackNum = 0;
	    }
	    work->nAttackCntr = PAR_CALL_INTV;
	}else if( off->weapon_type & WP_COLDSPRAY ){	// スプレー
	    PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_FLIPFLOP );
	}else if ( off->weapon_type & WP_BLAST ){	// 爆発系
	    PAR_SetThink1( work, TH1_NORMAL, TH2_ATARAXIA, TH3_ENEMY_CALL );	// 敵兵を呼ぶ
	}
    }
}


/*-------------------------------------------------------------*/
// 待機
static void ActIdle( NPCWORK *npc, int time )
{
    CONTROL*	ctrl;
    NPCACT*	act;

    act = &npc->action;
    ctrl = npc->ctrl;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 初期設定
    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );
	NPC_ChangeTargetSize( npc, PAR_TARGET_SIZE_STAND);
    }

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) { 
	return ;
    }

    // アジャスト
    {
	PARROT*	work;
	work = (PARROT *)npc->character;
	PAR_AdjustAimPoint( work, &work->vecAimPos);
    }

    // モーションチェック
    if ( act->current_mot == PAR_MOT_STAND ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    NPC_SetActMotion( npc, npc->base_mar, PAR_GetNextMot( &TBL_IDLE_PAR[0] ) );
	}
    }else{
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    switch ( act->current_mot ){
	    case PAR_MOT_FACE_L:	// 顔振り左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_L );
		break;
	    case PAR_MOT_FACE_R:	// 顔振り右
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_L:	// 首かしげ左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_R:	// 首かしげ右
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    default:			// その他
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    }
	    
	}
    }

    // 終了チェック
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }
}
// ばたつく
static void ActFlipFlop( NPCWORK *npc, int time )
{
    CONTROL*	ctrl;
    NPCACT*	act;
    PARROT*	work;

    act = &npc->action ;
    ctrl = npc->ctrl ;
    work = (PARROT *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 初期設定
    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );
	NPC_ChangeTargetSize( npc, PAR_TARGET_SIZE_STAND);

	PAR_ShakeCage( work, MAO_GetRandom(64, 96) );

	// 羽根
	{
	    FVECTOR vec;
	    extern void* NewParrotFeather( FVECTOR*, FVECTOR*, int, int, float, float, 
					   float, float, float, float, int);
	
	    DG_SetPos2( &DG_ZeroVector, &npc->ctrl->rot);
	    DG_RotVector( &vec, &vec, 1);
	    NewParrotFeather( &npc->ctrl->mov, &vec, 4, 1, 10.f, 20.f, 10.f, 20.f, 
			      -9.8f, 5.f, BP_PS2_rand() % 432140 + 432153);
	}
    }

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_TRG_OFF  ) ;

    // ダメージチェック
    if ( npc->CheckDamage( npc ) ) { 
	return;
    }

    // 終了チェック
    if ( npc->CheckPad( npc ) ){
	return;
    }

    // 再生終了
    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	act->act_end = 1;
	NPC_SetActMode( npc, NPC_ActStandStill ) ;
	return ;
    }
}
#if 0
// 方向転換
static void ActChengeDir( NPCWORK *npc, int time )
{
    CONTROL*	ctrl;
    NPCACT*	act;
    PARROT*	work;

    act = &npc->action ;
    ctrl = npc->ctrl ;
    work = (PARROT *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );
	NPC_ChangeTargetSize( npc, PAR_TARGET_SIZE_STAND);
    }
    
    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return ;
    }

    // モーションチェック
    if ( act->current_mot == PAR_MOT_STAND ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    NPC_SetActMotion( npc, npc->base_mar, PAR_GetNextMot( &TBL_CHGDIR_PAR[0] ) );
	}
    }else{
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    switch ( act->current_mot ){
	    case PAR_MOT_FACE_L:	// 顔振り左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_L );
		break;
	    case PAR_MOT_FACE_R:	// 顔振り右
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_L:	// 首かしげ左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_R:	// 首かしげ右
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    default:			// その他
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    }
	    
	}
    }

    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }

    // 進行方向 
    ctrl->turn.vy = act->dir ;
}

#endif

// 注目する
static	void ActAttention( NPCWORK *npc, int time )
{
    CONTROL*	ctrl;
    NPCACT*	act;
    PARROT*	work;

    act = &npc->action ;
    ctrl = npc->ctrl ;
    work = (PARROT *)npc->character;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    if ( time == 0 ) {
	NPC_SetActMotion( npc, npc->base_mar, act->set_mot );
	NPC_ChangeTargetSize( npc, PAR_TARGET_SIZE_STAND);
    }

    // アジャスト
    {
	PARROT*	work;
	work = (PARROT *)npc->character;
	PAR_AdjustAimPoint( work, &work->vecAimPos);
    }
    
    if ( npc->CheckDamage( npc ) ) { // ダメージチェック
	return ;
    }

    // モーションチェック
    if ( act->current_mot == PAR_MOT_STAND ){
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    NPC_SetActMotion( npc, npc->base_mar, PAR_GetNextMot( &TBL_ATTENT_PAR[0] ) );
	}
    }else{
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) { // 終了した
	    switch ( act->current_mot ){
	    case PAR_MOT_FACE_L:	// 顔振り左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_L );
		break;
	    case PAR_MOT_FACE_R:	// 顔振り右
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_L:	// 首かしげ左
		if ( MAO_Rand(10) < 5 )	NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		else			NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_NECK_R );
		break;
	    case PAR_MOT_NECK_R:	// 首かしげ右
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    default:			// その他
		NPC_SetActMotion( npc, npc->base_mar, PAR_MOT_STAND );
		break;
	    }
	    
	}
    }

    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }

    // 進行方向 
    ctrl->turn.vy = act->dir ;
}

// しゃべる
static void ActTalk( NPCWORK *npc, int time )
{
    NPCACT*	act;

    act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_STAND );

    // 初期設定
    if ( time == 0 ) {
	NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	NPC_ChangeTargetSize( npc, PAR_TARGET_SIZE_STAND);
    }

    // チェックダメージ
    if ( npc->CheckDamage( npc ) ) return ;

    // 終了チェック
    if ( act->pad != act->set_pad ) {
	if ( !npc->CheckPad( npc ) ) {
	    NPC_SetActMode( npc, NPC_ActStandStill ) ;
	}
	return ;
    }
    
    // 向き
    if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

/*-------------------------------------------------------------*/
static int	SampleCheckPad( NPCWORK *npc )
{
    NPCACT*	act;

    act = &npc->action;

    if ( act->pad == PAD_NONE ) return 0 ;

    switch ( act->pad ) {
    case PAD_IDLE : 	// 待機
	NPC_SetModeFromPad( npc, ActIdle, npc->base_mar, PAR_MOT_STAND, act->pad ) ;
	break ;
    case PAD_ATTENTION: // 注目
	NPC_SetModeFromPad( npc, ActAttention, npc->base_mar, PAR_MOT_FACE_L, act->pad ) ;
	break;
    case PAD_FLIPFLOP:	// ばたつく
	NPC_SetModeFromPad( npc, ActFlipFlop, npc->base_mar, PAR_MOT_FLIP, act->pad ) ;
	break ;
    case PAD_TALK:	// しゃべる
	NPC_SetModeFromPad( npc, ActTalk, npc->base_mar, PAR_MOT_TALK, act->pad ) ;
	break ;
    }
    return 1 ;
}

/*----------------------------------------------------------------*/
static	int	SmpCheckDamage( NPCWORK	*npc )
{	
    TARGET	*def ;
    NPCACT	*act ;
    NPCTARGET	*trg ;
    CAPTURE_TARGET	*cap ;
    long64	weapon ;
    PARROT*	work;

    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    weapon = 0 ;
    work = (PARROT *)npc->character;

    // 親ターゲット
    if ( TARGET_POWER & def->damaged ) {
	printf("smp damage weapon type [%lx]  \n",weapon);

	weapon = def->weapon_type ;
	NPC_DamageCaptureFlagClear( npc ) ;
	
	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	return 1 ;
    }

    // つかみ＆投げ
    if ( TARGET_CAPTURE & def->damaged ) {
	NPC_DamageFlagClear( npc ) ;	/* ダメージクリア */
	cap = trg->capture ;

	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	// 無効化
	cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	cap->capture = NULL ;	
	cap->flag = 0 ;
	return 1 ;
    }

    return 0 ;
}
/*----------------------------------------------------------------*/
static void Action( PARROT* work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

//	NPC_ActInit( npc ) ;
	NPC_Action( npc ) ;
	NPC_ActStatusCheck( npc ) ;

//	NPC_Gravitation( npc ) ;
}

