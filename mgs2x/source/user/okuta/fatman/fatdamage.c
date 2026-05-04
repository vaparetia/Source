/*
    fatdamage.c
    ファットマンダメージ思考処理
    2001/03/23 Masafumi Okuta
    $Id: fatdamage.c,v 1.1.1.3 2002/11/19 11:47:57 Yoshizawa1 Exp $
*/

// ダメージ量
#define STANDHEAD_DMG_POINT	(40)	// 立ち状態での頭部ダメージ
#define RUNHEAD_DMG_POINT	(40)	// 走行状態での頭部ダメージ
#define FALLHEAD_DMG_POINT 	(40)	// 倒れ状態での頭部ダメージ
#define ONLYHEAD_DMG_POINT	(40)	// 頭部のみの判定での頭部ダメージ
#define CLAYMORE_DMG_POINT	(9)	// クレイモアダメージ
#define STUN_DMG_POINT		(5)	// スタングレネードダメージ
#define BODY_DMG_POINT		(1)	// ブラストスーツ部分のダメージ
#define PUNCH_DMG_POINT		(2)	// パンチダメージ
#define KICK_DMG_POINT		(3)	// キックダメージ
#define ROLLER_DMG_POINT	(6)	// ローラーダメージ
#define FALL_DMG_POINT		(1)	// 耐久できず倒れダメージ

#define JPN_EASY_BODY_DMG_POINT	(2)	// ブラストスーツ部分のダメージ
#define JPN_EASY_FALL_DMG_POINT	(2)	// 耐久できず倒れダメージ

enum{ // ダメージの種類
STANDHEAD_DAMAGE,	// 立ち状態での頭部ダメージ
RUNHEAD_DAMAGE,		// 走行状態での頭部ダメージ
FALLHEAD_DAMAGE, 	// 倒れ状態での頭部ダメージ
ONLYHEAD_DAMAGE,	// 頭部のみの判定での頭部ダメージ
CLAYMORE_DAMAGE,	// クレイモアダメージ
STUN_DAMAGE,		// スタングレネードダメージ
BODY_DAMAGE,		// ブラストスーツ部分のダメージ
PUNCH_DAMAGE,		// パンチダメージ
KICK_DAMAGE,		// キックダメージ
ROLLER_DAMAGE,		// ローラーダメージ
FALL_DAMAGE,		// 耐久できず倒れダメージ
};

// ダメージレート
#define DEFAULT_DMG_RATE 	(1.f)   // デフォルトダメージ
#define JPN_EASY_DMG_RATE 	(7.f)   // 日本版EASYダメージ
#define JPN_EASY_HEAD_DMG_RATE 	(1.5f)  // 日本版EASY頭部ダメージ

// ダメージ計算
static int FAT_GetDamageValue( int nDmgType )
{
    int nDamage    = 0; // ダメージ量
    float fDmgRate = DEFAULT_DMG_RATE; // ダメージレート

//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	fDmgRate = JPN_EASY_DMG_RATE; // 主観なしで勝てるように変更
	break;
    }
printf("にほんばん------------%d - %d - %f\n", GM_GameLevel, GM_LEVEL_VERYEASY , fDmgRate);
#endif

    switch( nDmgType ){
    case STANDHEAD_DAMAGE:	// 立ち状態での頭部ダメージ
	nDamage = (int)(STANDHEAD_DMG_POINT);
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nDamage = (int)(JPN_EASY_HEAD_DMG_RATE * STANDHEAD_DMG_POINT);
	break;
    }
#endif
	break;
    case RUNHEAD_DAMAGE:	// 走行状態での頭部ダメージ
	nDamage = (int)(RUNHEAD_DMG_POINT);
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nDamage = (int)(JPN_EASY_HEAD_DMG_RATE * RUNHEAD_DMG_POINT);
	break;
    }
#endif
	break;
    case FALLHEAD_DAMAGE: 	// 倒れ状態での頭部ダメージ
	nDamage = (int)(FALLHEAD_DMG_POINT);
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nDamage = (int)(JPN_EASY_HEAD_DMG_RATE * FALLHEAD_DMG_POINT);
	break;
    }
#endif
	break;
    case ONLYHEAD_DAMAGE:	// 頭部のみの判定での頭部ダメージ
	nDamage = (int)(ONLYHEAD_DMG_POINT);
	break;
    case CLAYMORE_DAMAGE:	// クレイモアダメージ
	nDamage = (int)(fDmgRate * CLAYMORE_DMG_POINT);
	break;
    case STUN_DAMAGE:		// スタングレネードダメージ
	nDamage = (int)(fDmgRate * STUN_DMG_POINT);
	break;
    case BODY_DAMAGE:		// ブラストスーツ部分のダメージ
	nDamage = (int)(fDmgRate * BODY_DMG_POINT);
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nDamage = (int)(fDmgRate * JPN_EASY_BODY_DMG_POINT);
	break;
    }
#endif
	break;
    case PUNCH_DAMAGE:		// パンチダメージ
	nDamage = (int)(fDmgRate * PUNCH_DMG_POINT);
	break;
    case KICK_DAMAGE:		// キックダメージ
	nDamage = (int)(fDmgRate * KICK_DMG_POINT);
	break;
    case ROLLER_DAMAGE:		// ローラーダメージ
	nDamage = (int)(fDmgRate * ROLLER_DMG_POINT);
	break;
    case FALL_DAMAGE:		// 耐久できず倒れダメージ
	nDamage = (int)(fDmgRate * FALL_DMG_POINT);
//#ifdef JAPANESE_BP_IGNORE()
#if 1
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	nDamage = (int)(fDmgRate * JPN_EASY_FALL_DMG_POINT);
	break;
    }
#endif
	break;
    default:
	nDamage = 0;
	break;
    }

    return (nDamage);
}

// ゲームクリア処理
static void FAT_CheckGameClear( Work* work, int nKillFlag)
{
    if ( nKillFlag ){
	// 殺傷人数を増やす
	if ( GM_KillCount < 30000){
	    GM_KillCount++;
#ifdef DEBUG_MODE
printf("FATMAN DEAD---------------KILL COUNT + 1\n");
#endif
	}
	GCL_ExecProc( work->procKillClear, NULL );	// クリア:ファットマン死亡
    }else{
	GCL_ExecProc( work->procSleepClear, NULL );	// クリア:ファットマン眠る
    }
}

// 特殊武器ダメージによるワークの設定
static void FAT_SetWepDmgWork( Work* work, long64 weapon )
{	
    if ( weapon & WP_STUNGRENADE ) {	// スタングレネード
	work->nStunTime = 0;		// スタン時間クリア
    }else if ( weapon & WP_M92 ){	// M9
	work->nNarcTime = 0;		// 麻酔時間クリア
    }
#if 0
    // 死亡イベントフラグ立つ
    if ( !((weapon & WP_M92) || (weapon & WP_PUNCHR) || 
	   (weapon & WP_PUNCHL) || (weapon & WP_KICK)) || 
	   (weapon & WP_COLDSPRAY || (weapon & WP_STUNGRENADE)) ){ 
	work->nKillFlag = 1;
    }
#endif
}
// 部位によるワーク設定
static void FAT_SetChildDmgWork( Work* work, int dam_child_num)
{
    // ダメージ経過時間用ワーク更新
    if ( dam_child_num == FAT_TRG_CHILD_HEAD ){	
	work->nHeadDmgTime = 0;	// 頭部のダメージ経過時間クリア
	work->nHeadDmgFlag = 1; // 頭部のダメージフラグオン
   
   // BP WARNING - The below IF never gets hit doing to anding the same type of test
   //              ... will not fix
   
    }else if ( dam_child_num == FAT_TRG_CHILD_ROLL_R && dam_child_num == FAT_TRG_CHILD_ROLL_L ){ 
	work->nRollDmgTime = 0;	// ローラーブレード部分のダメージ経過時間クリア
    }else{
	work->nBodyDmgTime = 0;	// ブラストスーツ部分のダメージ経過時間クリア
    }
}


// ダメージ硬直開始
static void Think3_StartDamage( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc;
    if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {	// ダメージ終了
	FAT_SetThink1( work, TH1_NORMAL, work->nPhase, TH3_READY );
	return;
    }

    if ( npc->action.status & FAT_ACT_STATUS_DOWN_END ) {
	FAT_SetThink3( work, TH3_WAKEUP);	// ダウン終了->起き上がる
	return ;
    }

    // 途中でシステムダメージ処理に変化 
    if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
	FAT_SetThink2( work, TH2_SYSTEM_DAMAGE, TH3_WAIT );
	return;
    }
}
// ダメージ硬直:システム
static void Think3_SysDamageWait( Work *work )
{
    NPCWORK*	npc;

    npc = &work->npc ;
    if ( npc->action.status & NPC_ACT_STATUS_FAINT_END ) {
	FAT_SetThink3( work, TH3_WAKEUP);	// 硬直終了->起き上がる
	return ;
    }

    if ( npc->action.status & FAT_ACT_STATUS_DOWN_END ) {
	FAT_SetThink3( work, TH3_WAKEUP);	// ダウン終了->起き上がる
	return ;
    }

    if ( !(npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM) ) { // NPCシステムダメージ処理終了
	FAT_SetThink1( work, TH1_NORMAL, work->nPhase, TH3_READY );
	return;
    }
}
// 起き上がり
static void Think3_WakeUp( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_WAKEUP;
    }

    if ( npc->action.act_end ) {
	FAT_SetThink1( work, TH1_NORMAL, work->nPhase, TH3_READY);
	return;
    }

    work->count3++;
}

// 停止時のダメージチェック
static int CheckDamageStand( NPCWORK	*npc )
{
    Work*		work;
    TARGET*		def;
    NPCACT*		act;
    NPCTARGET		*trg;
    CAPTURE_TARGET	*cap;
    long64		weapon;
    int			dam_child_num;
    int 		bNoDmg;
    
    work = (Work *)npc->character;
    act = &npc->action;
    trg = &npc->target;
    def = npc->target.deftrg;
    weapon = 0;

    // 子ターゲット:主観時
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
	bNoDmg = 0; // ダメージ計算フラグクリア
	trg->dam_trg = trg->def_child + dam_child_num;	
	weapon = trg->dam_trg->weapon_type;		// 喰らった武器
	FAT_SetWepDmgWork( work, weapon );		// 特殊武器によるダメージ時のワーク設定
	FAT_SetChildDmgWork( work, dam_child_num );	// 子ターゲットダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc );		// 掴みダメージフラグ消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ); // このフレームではダメージを受けない

	// 頭部ダメージ
	if ( dam_child_num == FAT_TRG_CHILD_HEAD ){
	    work->nInviCntr = INVINCIBLE_HEAD;	// 無敵時間セット
	}

	// 武器による分岐
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) ) { // パンチ
	    GM_SeSetMode( SD_P_PUNCH02, &trg->dam_trg->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
	    }else{
		NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
	    }
	    if (work->nBodyDurable >= work->nBodyDurableMax){ // のけぞる
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		work->m9_faint -= FAT_GetDamageValue( FALL_DAMAGE);
		FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
	    }else{
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, 
				    FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad ) ;
		work->m9_faint -= FAT_GetDamageValue( PUNCH_DAMAGE);
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
	    }
	} else if ( weapon & WP_KICK ) { // キック
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad );
	    GM_SeSetMode( SD_P_KICK02, &trg->dam_trg->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
	    else			NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    work->m9_faint -= FAT_GetDamageValue( KICK_DAMAGE);
	} else if ( (weapon & WP_M92) ){ // M9
	    if ( dam_child_num == FAT_TRG_CHILD_ROLL_R || 
		 dam_child_num == FAT_TRG_CHILD_ROLL_L ) { 	 // ローラーブレードに当たった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_ROLLER_CRASH, act->pad ) ;
		work->nRollerBreak = 1; // ローラーブレード破損！！
		work->m9_faint -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    } else if ( dam_child_num == FAT_TRG_CHILD_HEAD ){ 	 // 頭に当たった
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN2, act->pad ) ;
		work->m9_faint -= FAT_GetDamageValue( STANDHEAD_DAMAGE);	
	    } else{						 // ブラストスーツ部分に当たった
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
		FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
		bNoDmg = 1;
	    }
	} else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) { // USP & ソーコム
	    if ( dam_child_num == FAT_TRG_CHILD_ROLL_R || 
		 dam_child_num == FAT_TRG_CHILD_ROLL_L ) { 	 // ローラーブレードに当たった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_ROLLER_CRASH, act->pad ) ;
		work->nRollerBreak = 1; // ローラーブレード破損！！
		act->life -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    } else if ( dam_child_num == FAT_TRG_CHILD_HEAD ){ 	 // 頭に当たった
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN2, act->pad ) ;
		FAT_SpillBlood( work, &trg->dam_trg->hit, &trg->dam_trg->power->force); // 血を流す
		act->life -= FAT_GetDamageValue( STANDHEAD_DAMAGE);	// ダメージ計算
	    } else {	// ブラストスーツに当たった
		// ブラストスーツ部分の防弾エフェクト
		FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );

		if (work->nBodyDurable >= work->nBodyDurableMax){  // 耐久できなかった
		    FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
		    work->nBodyDurable = 0;	 
		    work->nBodyDurTime = 0;
		    work->nRollerBreak = 1; // ローラーブレード破損！！
		    work->m9_faint -= FAT_GetDamageValue( ROLLER_DAMAGE);
		}else{
		    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad ) ;
		    if ( !work->bGameClear ){ // クリア音声チェック
			if ( work->m9_faint > 0 && npc->action.life > 0 ){
			    if ( MAO_Rand( 2 ) == 0 ) FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB); // 音声:うっ
			    else		      FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB); // 音声:いて
			}else{
			    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
			}
		    }else{
			FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		    }
		    work->nBodyDurable++;    // 耐久値を減らす
		    work->nBodyDurTime = 0;
		    act->life -= FAT_GetDamageValue( BODY_DAMAGE);
		    return 0;
		}
	    }
	} else if ( weapon & WP_CLAYMORE ) {	// 爆発(クレイモア)
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
	    act->life -= FAT_GetDamageValue( CLAYMORE_DAMAGE);	// ダメージ
	} else if ( weapon & WP_COLDSPRAY ) {	// 爆弾凍結スプレー
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    //	    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad ) ;
	    return 0;
	} else if ( weapon & WP_STUNGRENADE ) {	// スタングレネード
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_STUN, act->pad ) ;
	    work->m9_faint -= FAT_GetDamageValue( STUN_DAMAGE);	
	} else if ( (weapon & WP_WEAPONCORE) || (weapon & WP_STUNFAR) || (weapon & WP_STAMP) ){   // ノーダメージ系
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    return 0;
	}  

	// ダメージ音声
	FAT_DamageVoice( work, weapon, dam_child_num, bNoDmg);

	if ( bNoDmg ){
	    FAT_UnSetActStatus( act, NPC_ACT_STATUS_DAMAGE); // ダメージフラグを消す
   	    return 0;
	}else{
	    return 1;
	}
    }

    // 親ターゲット:俯瞰用
    if ( TARGET_POWER & def->damaged ) {
	bNoDmg = 0; // ダメージ計算フラグクリア
	weapon = def->weapon_type ;
	FAT_SetWepDmgWork( work, weapon );	// 特殊武器によるダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc ) ;	// 掴みダメージフラグを消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ); //このフレームではダメージを受けない 

	// 武器による分岐 
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) ) { 		// パンチ
	    GM_SeSetMode( SD_P_PUNCH02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
	    }else{
		NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
	    }
	    if (work->nBodyDurable >= work->nBodyDurableMax){ // のけぞる
		FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		work->m9_faint -= FAT_GetDamageValue( FALL_DAMAGE);
	    }else{
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
		work->m9_faint -= FAT_GetDamageValue( PUNCH_DAMAGE);
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
	    }
	} else if ( weapon & WP_KICK ) {	// キック
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad );
	    GM_SeSetMode( SD_P_KICK02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
	    else			NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    work->m9_faint -= FAT_GetDamageValue( KICK_DAMAGE);
	} else if ( (weapon & WP_M92) ){
	    bNoDmg = 1;
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	    // 麻酔弾を刺す
	    NPC_SetNeedl( npc->body, HUMAN21_MUNE, &def->hit, MASUIDAN_MODEL);
  	} else if ((weapon & WP_USP) || (weapon & WP_SOCOM) ) {	// USP & ソーコム
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	    if (work->nBodyDurable >= work->nBodyDurableMax){  // 耐久できなかった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		act->life -= FAT_GetDamageValue( FALL_DAMAGE);
	    }else{
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad ) ;
		if ( !work->bGameClear ){ // クリア音声チェック
		    if ( work->m9_faint > 0 && npc->action.life > 0 ){
			if ( MAO_Rand( 2 ) == 0 ) FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB); // 音声:うっ
			else		      	  FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB); // 音声:いて
		    }else{
			FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		    }
		}else{
		    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		}
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
		act->life -= FAT_GetDamageValue( BODY_DAMAGE);
		return 0;
	    }
	} else if ( weapon & WP_CLAYMORE ) {	// 爆発(クレイモア)
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad );
	    act->life -= FAT_GetDamageValue( CLAYMORE_DAMAGE);	// ダメージ
	} else if ( weapon & WP_COLDSPRAY ) {	// 爆弾凍結スプレー
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    //	    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    return 0;
	} else if ( weapon & WP_STUNGRENADE ) {	// スタングレネード
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_STUN, act->pad );
	    work->m9_faint -= FAT_GetDamageValue( STUN_DAMAGE);	
	} else if ( (weapon & WP_WEAPONCORE) || (weapon & WP_STUNFAR) || (weapon & WP_STAMP) ){   // ノーダメージ系
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    return 0;
	} else {				// その他
	    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	}

	// ダメージ音声
	FAT_DamageVoice( work, weapon, FAT_TRG_CHILD_HEART, bNoDmg);
	if ( bNoDmg ){
	    FAT_UnSetActStatus( act, NPC_ACT_STATUS_DAMAGE); // ダメージフラグを消す
   	    return 0;
	}else{
	    return 1;
	}
    }

    // 掴み系
    if ( TARGET_CAPTURE & def->damaged ) {
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	cap = trg->capture ;
	
	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	if ( cap->flag & CAPTURE_HANG ) {		// 首絞め 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE);
	    cap->capture = NULL;	
	    cap->flag = 0;
	    FAT_SetModeFromPad( npc, ActHang, npc->base_mar, FAT_MOT_HANG, act->pad ); 	       // ヒップアタック
	} else if ( cap->flag & CAPTURE_THROW ) {	// 投げ 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE);
	    cap->capture = NULL;	
	    cap->flag = 0;
	    FAT_SetModeFromPad( npc, ActHang, npc->base_mar, FAT_MOT_ROLLER_CRASH, act->pad ); // 押し潰し
	}
	return 1 ;
    }

    // ノックバックターゲット
    if ( work->pushtrg.damaged & TARGET_TOUCH ) {
	work->pushtrg.damaged 	  = 0;
	work->pushtrg.weapon_type = 0;
	if ( work->nEvadePush == 0 ){
	    FAT_CalcNockBack( work);	// プレイヤーを弾く 
	}	
	return 0;
    }

    return 0 ;
}

// 走行時のダメージチェック
static int CheckDamageRun( NPCWORK	*npc )
{
    Work*		work;
    TARGET*		def;
    NPCACT*		act;
    NPCTARGET		*trg;
    CAPTURE_TARGET	*cap;
    long64		weapon;
    int			dam_child_num;
    int			bNoDmg;
    int			bHeadDmg;
    
    work = (Work *)npc->character;
    act = &npc->action;
    trg = &npc->target;
    def = npc->target.deftrg;
    weapon = 0;
//printf("CheckDamageRun !!!!!!!! %d\n", GV_Time);
    // 子ターゲットに対するダメージ
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
	bNoDmg = 0;
	bHeadDmg = 0;	// 頭部ダメージフラグクリア
	trg->dam_trg = trg->def_child + dam_child_num;	
	weapon = trg->dam_trg->weapon_type;	// 喰らった武器種類を取得
//printf("DMG %d : W = %d\n", __LINE__, weapon);
	NPC_DamageCaptureFlagClear( npc );	// 掴みフラグを消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;	// このフレームではダメージを受けない

	FAT_SetWepDmgWork( work, weapon );		// 特殊武器によるダメージ時のワーク設定
	FAT_SetChildDmgWork( work, dam_child_num );	// 子ターゲットダメージ時のワーク設定
	
	// 頭部ダメージ
	if ( dam_child_num == FAT_TRG_CHILD_HEAD ){
	    work->nInviCntr = INVINCIBLE_HEAD;	// 無敵時間セット
	}

	// 武器による分岐
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) ) { // 右パンチ
	    GM_SeSetMode( SD_P_PUNCH02, &trg->dam_trg->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
	    }else{
		NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
	    }
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	    if (work->nBodyDurable >= work->nBodyDurableMax){  // 耐久できなかった
		FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		work->m9_faint -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    }else{
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad ) ;
		if ( !work->bGameClear ){ // クリア音声チェック
		    if ( work->m9_faint > 0 && npc->action.life > 0 ){
			if ( MAO_Rand( 2 ) == 0 ) FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB); // 音声:うっ
			else		      	  FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB); // 音声:いて
		    }else{
			FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		    }
		}else{
		    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		}
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
		work->m9_faint -= FAT_GetDamageValue( BODY_DAMAGE);
		return 0;
	    }
	} else if ( weapon & WP_KICK ) { // キック
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
	    GM_SeSetMode( SD_P_KICK02, &trg->dam_trg->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
	    else			NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    work->m9_faint -= FAT_GetDamageValue( KICK_DAMAGE);
	} else if ( weapon & WP_M92 ) {	// M9
	    if ( dam_child_num == FAT_TRG_CHILD_ROLL_R ||
		 dam_child_num == FAT_TRG_CHILD_ROLL_L ) { 	// ローラーブレードに当たった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_RUN_ROLLER_CRASH, act->pad ) ;
		work->nRollerBreak = 1; // ローラーブレード破損！！
		work->m9_faint -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    } else if ( dam_child_num == FAT_TRG_CHILD_HEAD ){ 	// 頭に当たった
		FAT_SpillBlood( work, &trg->dam_trg->hit, &trg->dam_trg->power->force); // 血を流す
		if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		    FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
		}else{
		    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
		}
		work->m9_faint -= FAT_GetDamageValue( RUNHEAD_DAMAGE);	// ダメージ計算
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
	    } else { // ブラストスーツ
		// ブラストスーツ部分の防弾エフェクト
		FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
		bNoDmg = 1;
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
	    }
	} else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) {	// USP & ソーコム
	    if ( dam_child_num == FAT_TRG_CHILD_ROLL_R ||
		 dam_child_num == FAT_TRG_CHILD_ROLL_L ) { 	 // ローラーブレードに当たった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_RUN_ROLLER_CRASH, act->pad ) ;
		work->nRollerBreak = 1; // ローラーブレード破損！！
		act->life -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    } else if ( dam_child_num == FAT_TRG_CHILD_HEAD ){ 	 // 頭に当たった
		if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		    FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
		}else{
		    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
		}
		act->life -= FAT_GetDamageValue( STANDHEAD_DAMAGE);	// ダメージ計算
	    } else {
		// ブラストスーツ部分の防弾エフェクト
		FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
		if (work->nBodyDurable >= work->nBodyDurableMax){ // 倒れる
		    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
		    work->nBodyDurable = 0;	//
		    work->nBodyDurTime = 0;
		    act->life -= FAT_GetDamageValue( FALL_DAMAGE);
		}else{
		    act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
		    if ( !work->bGameClear ){ // クリア音声チェック
			if ( work->m9_faint > 0 && npc->action.life > 0 ){
			    if ( MAO_Rand( 2 ) == 0 )	FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB); // 音声:うっ
			    else		  	FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB); // 音声:いて
			}else{
			    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
			}
		    }else{
			FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		    }
		    act->life -= FAT_GetDamageValue( BODY_DAMAGE);
		    work->nBodyDurable++;    // 耐久値を減らす
		    work->nBodyDurTime = 0;
		    return 0;
		}
	    }
	} else if ( weapon & WP_CLAYMORE ) {	// 爆発(クレイモア)
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
	    act->life -= FAT_GetDamageValue( CLAYMORE_DAMAGE);	// ダメージ
	} else if ( weapon & WP_COLDSPRAY ) {	// 爆弾凍結スプレー
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
//	    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad ) ;
	    return 0;
	} else if ( weapon & WP_STUNGRENADE ) {	// スタングレネード
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_RUN_STUN, act->pad );
	    work->m9_faint -= FAT_GetDamageValue( STUN_DAMAGE);	
	} else if ( (weapon & WP_WEAPONCORE) || (weapon & WP_STUNFAR) || (weapon & WP_STAMP) ){   // ノーダメージ系
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    return 0;
	} else {				// その他
	    if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    }else{
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    }
	}
	// ダメージ音声
	FAT_DamageVoice( work, weapon, dam_child_num, bNoDmg);
	if ( bNoDmg ){
	    FAT_UnSetActStatus( act, NPC_ACT_STATUS_DAMAGE); // ダメージフラグを消す
   	    return 0;
	}else{
	    return 1;
	}
    }

    // 俯瞰時の計算
    // 爆発系以外はダメージは与えない
    if ( TARGET_POWER & def->damaged ) {
	bNoDmg = 0;
	weapon = def->weapon_type ;
	FAT_SetWepDmgWork( work, weapon );	// 特殊武器によるダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc ) ;
//printf("npc damage weapon type [0x%lx]  \n",weapon);

	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF );

	// 武器による分岐 
	if ( (weapon & WP_PUNCHR) || (weapon & WP_PUNCHL) ) { // パンチ
	    GM_SeSetMode( SD_P_PUNCH02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( GM_WeaponTypes[ PL_GetPlayerWeapon() ] & WP_TYPE_PUNCH_L ){    // 振動コール
		NewPadVibration2( GV_StrCode("rai_punch_02"), 0); // 強
	    }else{
		NewPadVibration2( GV_StrCode("rai_punch_01"), 0); // 弱
	    }
	    if (work->nBodyDurable >= work->nBodyDurableMax){ // 倒れる
		FAT_SetModeFromPad( npc, ActRunFall, npc->damage_mar, FAT_MOT_BLAST_DAM_3, act->pad ) ;
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		work->m9_faint -= FAT_GetDamageValue( FALL_DAMAGE);
	    }else{ // のけぞる
		if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		    FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, 
					FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad );
		}else{
		    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, 
					FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad );
		}
		work->m9_faint -= FAT_GetDamageValue( PUNCH_DAMAGE);
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
	    }
	} else if ( weapon & WP_KICK ) { // キック
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
	    GM_SeSetMode( SD_P_KICK02, &def->hit, GM_SEMODE_BOMB);	// 効果音コール
	    if ( weapon & WP_KICK1 ) 	NewPadVibration2( GV_StrCode("rai_kick_01"), 0); // キック１回目
	    else			NewPadVibration2( GV_StrCode("rai_kick_02"), 0); // キック２回目
	    work->m9_faint -= FAT_GetDamageValue( KICK_DAMAGE);
	} else if ( weapon & WP_M92 ) {	// M9
	    act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
	    bNoDmg = 1;
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	    // 麻酔弾を刺す
	    NPC_SetNeedl( npc->body, HUMAN21_MUNE, &def->hit, MASUIDAN_MODEL);
	} else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) {	// USP & ソーコム
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	    if (work->nBodyDurable >= work->nBodyDurableMax){  // 耐久できなかった
		FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
		work->nBodyDurable = 0;	 
		work->nBodyDurTime = 0;
		act->life -= FAT_GetDamageValue( ROLLER_DAMAGE);
	    }else{
		if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		    FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, 
					FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad );
		}else{
		    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, 
					FAT_MOT_BLAST_DAM_1 + ( work->nBodyDurable % 2 ), act->pad );
		}

		if ( !work->bGameClear ){ // クリア音声チェック
		    if ( work->m9_faint > 0 && npc->action.life > 0 ){
			if ( MAO_Rand( 2 ) == 0 ) FAT_Say( work, SE_FATDMG01, GM_SEMODE_BOMB); // 音声:うっ
			else		      	  FAT_Say( work, SE_FATDMG02, GM_SEMODE_BOMB); // 音声:いて
		    }else{
			FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		    }
		}else{
		    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], GM_SEMODE_BOMB);
		}
		work->nBodyDurable++;    // 耐久値を減らす
		work->nBodyDurTime = 0;
		act->life -= FAT_GetDamageValue( BODY_DAMAGE);
		return 0;
	    }
	} else if ( weapon & WP_CLAYMORE ) {	// 爆発(クレイモア)
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad );
	    act->life -= FAT_GetDamageValue( CLAYMORE_DAMAGE);	// ダメージ
	} else if ( weapon & WP_COLDSPRAY ) {	// 爆弾凍結スプレー
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
//	    FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    return 0;
	} else if ( weapon & WP_STUNGRENADE ) {	// スタングレネード
	    FAT_SetModeFromPad( npc, ActFall, npc->damage_mar, FAT_MOT_DAM_RUN_STUN, act->pad );
	    work->m9_faint -= FAT_GetDamageValue( STUN_DAMAGE);	
	} else if ( (weapon & WP_WEAPONCORE) || (weapon & WP_STUNFAR) || (weapon & WP_STAMP) ){   // ノーダメージ系
	    UNSET_FLAG( act->status, NPC_ACT_STATUS_DAMAGE);
	    return 0;
	} else {				// その他
	    if ( GM_GameLevel >= GM_LEVEL_HARD || work->nConsTime > 0){
		FAT_SetModeFromPad( npc, ActRunDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    }else{	
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad );
	    }
	}

	// ダメージ音声
	FAT_DamageVoice( work, weapon, FAT_TRG_CHILD_HEART, bNoDmg);
	if ( bNoDmg ){
	    FAT_UnSetActStatus( act, NPC_ACT_STATUS_DAMAGE); // ダメージフラグを消す
   	    return 0;
	}else{
	    return 1;
	}
    }

    // 掴み系
    if ( TARGET_CAPTURE & def->damaged ) {
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	cap = trg->capture ;
	
	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	if ( cap->flag & CAPTURE_HANG ) {		// 首絞め 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	} else if ( cap->flag & CAPTURE_THROW ) {	// 投げ 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	}
	return 1 ;
    }

    // ノックバックターゲット
    if ( work->pushtrg.damaged & TARGET_TOUCH ) {
	work->pushtrg.damaged 	  = 0;
	work->pushtrg.weapon_type = 0;
	if ( work->nEvadePush == 0 ){
	    FAT_CalcNockBack( work);	// プレイヤーを弾く 
	}
	return 0;
    }

    return 0 ;
}

// 転倒時のダメージチェック:ヘッドショットを狙う
static int CheckDamageDown( NPCWORK	*npc )
{
    Work*		work;
    TARGET*		def;
    NPCACT*		act;
    NPCTARGET		*trg;
    long64		weapon;
    int			dam_child_num;

    work = (Work *)npc->character;
    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    weapon = 0 ;
//printf("CheckDamageDown !!!!!!!! %d\n", GV_Time);

    // 頭ターゲットのみ判定
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
	trg->dam_trg = trg->def_child + dam_child_num ;
	weapon = trg->dam_trg->weapon_type ;
//printf("DMG %d : W = %d\n", __LINE__, weapon);
	FAT_SetWepDmgWork( work, weapon );	// 特殊武器によるダメージ時のワーク設定
	FAT_SetChildDmgWork( work, dam_child_num );	// 子ターゲットダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ); // このフレームではダメージを受けない
	
	if ( dam_child_num == FAT_TRG_CHILD_HEAD ){
	    // 武器による分岐
	    if ( weapon & WP_M92 ) {		// M9
		act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
//		FAT_SetModeFromPad( npc, ActDownDamage, npc->damage_mar, FAT_MOT_DOWN, act->pad ) ;
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
		// ダメージ
		work->m9_faint -= FAT_GetDamageValue( FALLHEAD_DAMAGE);
	    } else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) {	// USP & ソーコム
		FAT_SpillBlood( work, &trg->dam_trg->hit, &trg->dam_trg->power->force); // 血を流す
		FAT_SetModeFromPad( npc, ActDownDamage, npc->damage_mar, act->current_mot, act->pad );
		act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
		// ダメージ
		act->life -= FAT_GetDamageValue( FALLHEAD_DAMAGE);
	    }

	    // ダメージ音声
	    FAT_DamageVoice( work, weapon, FAT_TRG_CHILD_HEAD, 0);
	}else{
	    if ( dam_child_num != FAT_TRG_CHILD_ROLL_L &&
		 dam_child_num != FAT_TRG_CHILD_ROLL_R ){
		// ブラストスーツ部分の防弾エフェクト
		FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
	    }

	    act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
	    return 0;
	}
	return 1 ;
    }
    // 親ターゲット:俯瞰用
    if ( TARGET_POWER & def->damaged ) {
	NPC_DamageCaptureFlagClear( npc ) ;	// 掴みダメージフラグを消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ); //このフレームではダメージを受けない 
	return 0;
    }

    // 掴み系
    if ( TARGET_CAPTURE & def->damaged ) {
	NPC_DamageCaptureFlagClear( npc ) ;	// 掴みダメージフラグを消す
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	return 0;
    }

    // ノックバックターゲット
    if ( work->pushtrg.damaged & TARGET_TOUCH ) {
	work->pushtrg.damaged 	  = 0;
	work->pushtrg.weapon_type = 0;
	if ( work->nEvadePush == 0 ){
	    FAT_CalcNockBack( work);	// プレイヤーを弾く 
	}
	return 0;
    }

    return 0;
}

// 頭部のみのダメージチェック
static int CheckDamageHeadOnly( NPCWORK*	npc )
{
    Work*		work;
    TARGET*		def;
    NPCACT*		act;
    NPCTARGET		*trg;
    long64		weapon;
    int			dam_child_num;

    work = (Work *)npc->character;
    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    weapon = 0 ;

    // 頭ターゲットのみ判定
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0){
	trg->dam_trg = trg->def_child + dam_child_num ;
	weapon = trg->dam_trg->weapon_type ;
	FAT_SetWepDmgWork( work, weapon );		// 特殊武器によるダメージ時のワーク設定
	FAT_SetChildDmgWork( work, dam_child_num );	// 子ターゲットダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ); // このフレームではダメージを受けない

	if ( dam_child_num == FAT_TRG_CHILD_HEAD ) {
	    // 武器による分岐
	    if ( weapon & WP_M92 ) { // M9
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad ) ;
		work->m9_faint -= FAT_GetDamageValue( FALLHEAD_DAMAGE);	// 耐久値減らす
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
	    } else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) { // USP & ソーコム
		FAT_SpillBlood( work, &trg->dam_trg->hit, &trg->dam_trg->power->force); // 血を流す
		FAT_SetModeFromPad( npc, ActDamage, npc->damage_mar, FAT_MOT_DAM_GUN, act->pad ) ;
		act->life -= FAT_GetDamageValue( FALLHEAD_DAMAGE);		// ダメージ	
	    }

	    // ダメージ音声
	    FAT_DamageVoice( work, weapon, FAT_TRG_CHILD_HEAD, 0);
	}else{
	    if ( weapon & WP_BULLET ) { // 銃弾系
		if ( dam_child_num != FAT_TRG_CHILD_ROLL_L &&
		     dam_child_num != FAT_TRG_CHILD_ROLL_R ){
		    if ( weapon & WP_M92 ) {
			// 麻酔弾を刺す
			if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
			    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
		    }
		    // ブラストスーツ部分の防弾エフェクト
		    FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
		}
	    }
	    act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
	    return 0;
	}
	return 1 ;
    }
    // 親ターゲット:俯瞰用
    if ( TARGET_POWER & def->damaged ) {
	NPC_DamageCaptureFlagClear( npc ) ;	// 掴みダメージフラグを消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ); //このフレームではダメージを受けない 
	if ( weapon & WP_BULLET ) { // 銃弾系
	    if ( weapon & WP_M92 ) {
		// 麻酔弾を刺す
		NPC_SetNeedl( npc->body, HUMAN21_MUNE, &def->hit, MASUIDAN_MODEL);
	    }
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	}
	return 0;
    }
    // 掴み系
    if ( TARGET_CAPTURE & def->damaged ) {
	CAPTURE_TARGET	*cap;
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	cap = trg->capture ;
	
	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	if ( cap->flag & CAPTURE_HANG ) {		// 首絞め 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	}else{
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	}
	return 0;
    }

    // ノックバックターゲット
    if ( work->pushtrg.damaged & TARGET_TOUCH ) {
	work->pushtrg.damaged 	  = 0;
	work->pushtrg.weapon_type = 0;
	if ( work->nEvadePush == 0 ){
	    FAT_CalcNockBack( work);	// プレイヤーを弾く 
	}
	return 0;
    }

    return 0;
}


// 瀕死状態でのダメージチェック
static int CheckDamageNearDead( NPCWORK*	npc )
{
    Work*		work;
    TARGET*		def;
    NPCACT*		act;
    NPCTARGET		*trg;
    long64		weapon;
    int			dam_child_num;

    work = (Work *)npc->character;
    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    weapon = 0 ;

    // 頭ターゲットのみ判定
    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0){
	trg->dam_trg = trg->def_child + dam_child_num ;
	weapon = trg->dam_trg->weapon_type ;
	FAT_SetWepDmgWork( work, weapon );		// 特殊武器によるダメージ時のワーク設定
	FAT_SetChildDmgWork( work, dam_child_num );	// 子ターゲットダメージ時のワーク設定
	NPC_DamageCaptureFlagClear( npc ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ); // このフレームではダメージを受けない

	if ( dam_child_num == FAT_TRG_CHILD_HEAD ) {
	    // 武器による分岐
	    if ( weapon & WP_M92 ) { // M9
		FAT_SetModeFromPad( npc, ActDeadFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
		// 麻酔弾を刺す
		if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
		    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
	    } else if ( (weapon & WP_USP) || (weapon & WP_SOCOM) ) { // USP & ソーコム
		FAT_SpillBlood( work, &trg->dam_trg->hit, &trg->dam_trg->power->force); // 血を流す
		FAT_SetModeFromPad( npc, ActDeadFall, npc->damage_mar, FAT_MOT_DAM_BOMB, act->pad ) ;
	    }

	    FAT_CallSe( work, SE_FATOUT01, (FVECTOR *)&BODYWORLD(work->npc.body, HUMAN21_ATAMA ).m[3][0], 
			GM_SEMODE_BOMB);
	}else{
	    if ( weapon & WP_BULLET ) { // 銃弾系
		if ( dam_child_num != FAT_TRG_CHILD_ROLL_L &&
		     dam_child_num != FAT_TRG_CHILD_ROLL_R ){
		    if ( weapon & WP_M92 ) {
			// 麻酔弾を刺す
			if ( dam_child_num > FAT_TRG_CHILD_BLASTNECK_R  ) 
			    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL);
		    }
		    // ブラストスーツ部分の防弾エフェクト
		    FAT_BlastSuitEvadeShootEffect( work, &trg->dam_trg->hit, &trg->dam_trg->power->force );
		}
	    }
	    act->adj_piku_time = ADJ_PIKU_TIME;	// 体ピク
	    return 0;
	}
	return 1 ;
    }
    // 親ターゲット:俯瞰用
    if ( TARGET_POWER & def->damaged ) {
	NPC_DamageCaptureFlagClear( npc ) ;	// 掴みダメージフラグを消す
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ); //このフレームではダメージを受けない 
	if ( weapon & WP_BULLET ) { // 銃弾系
	    if ( weapon & WP_M92 ) {
		// 麻酔弾を刺す
		NPC_SetNeedl( npc->body, HUMAN21_MUNE, &def->hit, MASUIDAN_MODEL);
	    }
	    // ブラストスーツ部分の防弾エフェクト
	    FAT_BlastSuitEvadeShootEffect( work, &def->hit, &def->power->force );
	}
	return 0;
    }
    // 掴み系
    if ( TARGET_CAPTURE & def->damaged ) {
	CAPTURE_TARGET	*cap;
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
	cap = trg->capture ;
	
	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
	
	if ( cap->flag & CAPTURE_HANG ) {		// 首絞め 
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	}else{
	    cap->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
	    cap->capture = NULL ;	
	    cap->flag = 0 ;
	}
	return 0;
    }

    // ノックバックターゲット
    if ( work->pushtrg.damaged & TARGET_TOUCH ) {
	work->pushtrg.damaged 	  = 0;
	work->pushtrg.weapon_type = 0;
	if ( work->nEvadePush == 0 ){
	    FAT_CalcNockBack( work);	// プレイヤーを弾く 
	}
	return 0;
    }

    return 0;
}




