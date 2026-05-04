/*
    fatattack.c
    ファットマン戦 : 攻撃関連
    2001/03/23 Masafumi Okuta
    $Id: fatattack.c,v 1.1.1.3 2002/11/19 11:47:56 Yoshizawa1 Exp $
*/

//----------------------------------- 攻撃関連 ---------------------------------------
// 体当りコールバック
static void	HitCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work *work;	
    NPCWORK* npc;
    static int nBodySeTime = 0;

    work = (Work* )ptr;

    npc = &work->npc;

    if (abs(nBodySeTime - GV_Time) < 120){
	return;
    }
    GM_SeSetMode( SD_E_ATARU02, &npc->ctrl->mov, GM_SEMODE_BOMB);
    nBodySeTime = GV_Time;

}
// 衝突コールバック
static void	CrashCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work *work;	
    NPCWORK* npc;
    static int nBodySeTime = 0;

    work = (Work* )ptr;

    npc = &work->npc;

    if (abs(nBodySeTime - DG_TickCount) < COUNT_VMODE(120) ){
	return;
    }
    GM_SeSetMode( SD_V_GBSDWN01, &npc->ctrl->mov, GM_SEMODE_BOMB);
    nBodySeTime = DG_TickCount;

}

// 体当り判定生成
static void Attack( NPCWORK *npc )
{
    Work*		work;		// ファットマンワーク
    TARGET*		trgOffence;	// 攻撃用ターゲット
    POWER_TARGET*	pow;		// 
    FVECTOR		vecForce;	// 力積

    static FVECTOR vecSize = { 400.f,  400.f, 400.f };	// ターゲットサイズ
    static FVECTOR vecRot  = {   0.f, -100.f, 100.f };	// 攻撃方向（回転角度）

    work = (Work *)npc->character;
    trgOffence = &( work->attack );
    pow = &( work->pow );

    GM_SetTarget( trgOffence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); // ターゲット初期化
    GM_SetTargetWeaponType( trgOffence, WP_KICK ); 				// 武器タイプを設定
    GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_MUNE ) ) );	// 体に追従

    // 攻撃ターゲットの位置設定
    DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot ) ;
    DG_RotVector( &vecRot, &vecForce, 1 );

    GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
			  (int)((float)BODYATTACK_POW * work->fPowerRate), &vecForce );	// 攻撃用ターゲットの設定
    GM_SetTargetCallBack( trgOffence, HitCallBack, ( void * )work ) ;    
    GM_PutTarget( trgOffence );							// 攻撃ターゲット配置
}
// 衝突:転ばせはしない
static void Clash( NPCWORK *npc )
{
    Work*		work;		// ファットマンワーク
    TARGET*		trgOffence;	// 攻撃用ターゲット
    POWER_TARGET*	pow;		// 
    FVECTOR		vecForce;	// 力積

    static FVECTOR vecSize = { 200.f,  200.f, 200.f };	// ターゲットサイズ
    static FVECTOR vecRot  = {   0.f, -100.f, 100.f };	// 攻撃方向（回転角度）

    work = (Work *)npc->character;
    trgOffence = &( work->attack );
    pow = &( work->pow );

    // ターゲット初期化
    GM_SetTarget( trgOffence, TARGET_OFFENSE | TARGET_TOUCH, 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); 
    GM_SetTargetWeaponType( trgOffence, WP_BODY ); 				// 武器タイプを設定
    GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_MUNE ) ) );	// 体に追従

    // 攻撃ターゲットの位置設定
    DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot ) ;
    DG_RotVector( &vecRot, &vecForce, 1 );
    GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
			  (int)((float)BODYCLASH_POW * work->fPowerRate), &vecForce );	// 攻撃用ターゲットの設定
    GM_SetTargetCallBack( trgOffence, CrashCallBack, ( void * )work ) ;    
    GM_PutTarget( trgOffence );							// 攻撃ターゲット配置
}
// キャプチャーふっとばし判定生成
static void CaptureCounter( NPCWORK *npc )
{
    Work*		work;		// ファットマンワーク
    TARGET*		trgOffence;	// 攻撃用ターゲット
    POWER_TARGET*	pow;		// 
    FVECTOR		vecForce;	// 力積

    static FVECTOR vecSize = { 1500.f,  1500.f, 1500.f };	// ターゲットサイズ

    work = (Work *)npc->character;
    trgOffence = &( work->atcBlow );
    pow = &( work->powBlow );

    GM_SetTarget( trgOffence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); // ターゲット初期化
    GM_SetTargetWeaponType( trgOffence, WP_SOFTBLOW ); 				// 武器タイプを設定
    GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_MUNE ) ) );	// 体に追従

    // 力積計算
    MAO_GetDiffVec3( &vecForce, NULL, NULL, &npc->ctrl->mov, &GM_PlayerPosition);
    _sceVu0ScaleVector( &vecForce, &vecForce, 180.f);

    // 攻撃ターゲットの位置設定
    GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
		       (int)((float)CAPTUREEVADE_POW * work->fPowerRate), &vecForce );	// 攻撃用ターゲットの設定
    GM_PutTarget( trgOffence );							// 攻撃ターゲット配置
}

// ローラーブレード
static void RollerBladeAttack( NPCWORK *npc )
{
    Work*		work;		// ファットマンワーク
    TARGET*		trgOffence;	// 攻撃用ターゲット
    POWER_TARGET*	pow;		// 
    FVECTOR		vecForce;	// 力積

    static FVECTOR vecSize = { 20.f,  50.f, 20.f };	// ターゲットサイズ

    { // 左足
	work = (Work *)npc->character;
	trgOffence = &( work->atcRollerL );
	pow = &( work->powRollerL );

	GM_SetTarget( trgOffence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); // ターゲット初期化
	GM_SetTargetWeaponType( trgOffence, WP_BULLET ); 			  	   // 武器タイプを設定
	GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_HIDARI_KAKATO ) ) );	   // 体に追従

	// 力積計算
	MAO_GetDiffVec3( &vecForce, NULL, NULL, &npc->ctrl->mov, &GM_PlayerPosition);
	_sceVu0ScaleVector( &vecForce, &vecForce, 10.f);

	// 攻撃ターゲットの位置設定
	GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
			   (int)((float)ROLLERBLADE_POW * work->fPowerRate), &vecForce );
	GM_PutTarget( trgOffence );						       
    }

    { // 右足
	work = (Work *)npc->character;
	trgOffence = &( work->atcRollerR );
	pow = &( work->powRollerR );

	GM_SetTarget( trgOffence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); // ターゲット初期化
	GM_SetTargetWeaponType( trgOffence, WP_BULLET ); 			  	   // 武器タイプを設定
	GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_MIGI_KAKATO ) ) );	   // 体に追従

	// 力積計算
	MAO_GetDiffVec3( &vecForce, NULL, NULL, &npc->ctrl->mov, &GM_PlayerPosition);
	_sceVu0ScaleVector( &vecForce, &vecForce, 10.f);

	// 攻撃ターゲットの位置設定
	GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
			   (int)((float)ROLLERBLADE_POW * work->fPowerRate), &vecForce );
	GM_PutTarget( trgOffence );						       
    }
}
// 体弾き
static void BodyEvade( NPCWORK *npc )
{
    Work*		work;		// ファットマンワーク
    TARGET*		trgOffence;	// 攻撃用ターゲット
    POWER_TARGET*	pow;		// 
    FVECTOR		vecForce;	// 力積

    static FVECTOR vecSize = { 400.f,  400.f, 400.f };	// ターゲットサイズ
    static FVECTOR vecRot  = {   0.f, -100.f, 100.f };	// 攻撃方向（回転角度）

    work = (Work *)npc->character;
    trgOffence = &( work->atcEvade );
    pow = &( work->powEvade );

    GM_SetTarget( trgOffence, TARGET_OFFENSE , 0, PLAYER_SIDE, &vecSize, &DG_ZeroVector ); // ターゲット初期化
    GM_SetTargetWeaponType( trgOffence, WP_KICK ); 				// 武器タイプを設定
    GM_MoveTarget3( trgOffence, &( BODYWORLD(npc->body, HUMAN21_MUNE ) ) );	// 体に追従

    // 攻撃ターゲットの位置設定
    DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot ) ;
    DG_RotVector( &vecRot, &vecForce, 1 );

    GM_SetPowerTarget( trgOffence, pow, POWER_ONCE, 255, 0, 
			  (int)((float)BODYEVADE_POW * work->fPowerRate), &vecForce );	// 攻撃用ターゲットの設定
    GM_SetTargetCallBack( trgOffence, HitCallBack, ( void * )work ) ;    
    GM_PutTarget( trgOffence );							// 攻撃ターゲット配置
}

// グロック装填
static void Think3_Reload( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_RELOAD; 		// 装填動作
    }

    // 終了チェック
    if ( work->npc.action.act_end ){
	FAT_SetThink3FromNext( work, TH3_READY);	// 思考準備へ
	return;
    }

    work->count3++;
}

// 走りながらリロード
static void Think3_RunReload( Work* work )
{
    int nRes;
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	
printf("----- RUN RELOAD START ----- \n");
	if ( FAT_CheckNextZone( work) < 0 ){
MAO_PRINTF("RunReload >>>> Next Zone ------------- %d\n", work->nNextZone);
	    return; // 設定ミス
	}

	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone); // ルートチェック

	npc->action.pad = PAD_RUN_RELOAD;	// 走行開始 : ループ再生
    }

    if ( FAT_RouteZoneMove( work) ){ 	// ルート＆ゾーン共存移動
MAO_PRINTF("RunReload >>>> Route Zone End------------- %d\n", work->nNextZone);
	nRes = FAT_GetAreaDataZoneEx( work, FAT_GetContArea( work->nFatArea ) );
        FAT_SetNewAim( work, nRes, NULL);  // 新目標設定
printf("----- RUN RELOAD NEW AIM ----- %d\n", nRes);
    }

    // 終了
    if ( work->npc.action.act_end ) {	
	FAT_SetThink3FromNext( work, TH3_MOVE); // 準備
	if ( work->think3 == TH3_MOVE)	npc->action.pad = PAD_RUN_ACT;	
MAO_PRINTF("RunReload >>>> Action End------------- %d\n", work->think3);
	return;
    }
    
    work->count3++;
}

// 体当り適応エリアチェックを行ない満たしたらゾーンを返す
// 失敗したら : -1
#define FAT_BODYATTACK_DIST 	(5000)	// 体当り発動距離
#define FAT_BODYATTACK_DIRDIFF  (256)	// 体当り方向
#define FAT_BODYATTACK_INNER    (0.f)	// 体当りゾーン判定用内積値
static int FAT_GetBodyAttackZone( Work* work )
{
    if ( work->pl_dis <  FAT_BODYATTACK_DIST &&
	 GV_DiffDirAbs( work->pl_dir, work->npc.ctrl->rot.vy) < FAT_BODYATTACK_DIRDIFF ){ // 距離,方向判定
	return ( HZX_Zone1( GM_PlayerAddress ) );	// 成功
    }
    return (-1);	// 失敗
}

// 体当たり
static void Think3_BodyAttack( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {
	// ゾーンアドレスからナビターゲットの情報を生成
	HZX_ZONE_ADD trgzone;
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );

	npc->action.pad = PAD_BODYATTACK;	// 体当り

	if ( MAO_Rand( 2 ) == 0) FAT_Say( work, SE_FATKIA01, GM_SEMODE_BOMB); // 音声:ふん
	else		         FAT_Say( work, SE_FATKIA02, GM_SEMODE_BOMB); // 音声:うりゃ
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    // 終了チェック
    if ( work->npc.action.act_end ){
	if ( work->nNextZone2Valid && FAT_CheckZone(work, work->nNextZone2) >= 0 ){ // 次の指定があるなら
	    FAT_SetNextZoneFrom2( work, GetRandZone( work ) );  // 先行設定orランダムで次の点指定
	    FAT_SetThink3FromNext( work, TH3_MOVE); 		// 先行設定or移動へ
	    return;
	}else{
	    FAT_SetThink3FromNext( work, TH3_READY);	// 先行設定or思考準備へ
	}
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}

// 逆走
static void Think3_BackShoot( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {
	// ゾーンアドレスからナビターゲットの情報を生成
	HZX_ZONE_ADD trgzone;
	GM_ReSetNavi( npc->navi); // リセット
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	// 音声
	if ( MAO_Rand( 2 ) == 0) FAT_StreamRequestJustTime( work, FAT_STRM_ELUDE_01); // 音声:しねー
	else		         FAT_StreamRequestJustTime( work, FAT_STRM_ELUDE_02); // 音声:くらえー	 

	npc->action.pad = PAD_BACKSHOT_READY;	// 逆走
	npc->action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

#if 0
    // 残弾数チェック
    if ( work->nBullet == 0 ){
	FAT_SetNextZone( work, FAT_GetAreaDataZone( work ));
	FAT_SetNextThink3( work, TH3_MOVE);
	FAT_SetThink3( work, TH3_RUN_RELOAD);
	return;
    }
#endif
    // 終了チェック
    if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	if ( work->nNextZone2Valid && FAT_CheckZone(work, work->nNextZone2) >= 0 ){ // 次の指定があるなら
	    FAT_SetNextZoneFrom2( work, GetRandZone( work ) );  // 先行設定orランダムで次の点指定
	    FAT_SetThink3FromNext( work, TH3_MOVE); 		// 先行設定or移動へ
	    return;
	}else{
	    FAT_SetThink3FromNext( work, TH3_READY);	// 先行設定or思考準備へ
	}
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
