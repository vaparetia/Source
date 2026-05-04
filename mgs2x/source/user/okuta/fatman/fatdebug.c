/*
    fatdebug.c
    ファットマン戦 : デバック用
    2001/04/04 Masafumi Okuta
    $Id: fatdebug.c,v 1.1.1.3 2002/11/19 11:47:58 Yoshizawa1 Exp $
*/
#ifdef DEBUG_MODE
#define FAT_BEZIER_RATE (8)

enum{
DBG_DUMP_LEVEL_MENU_LV,
DBG_DUMP_LEVEL_MENU_PT,
};
static void FAT_DbgPutLevelView(DebugView *DbgWork )
{
    int i, j, n;
    FVECTOR vec1, vec2;
    FAT_PUTPOINT_DATA*	pputData;
    Work*     work;
    static int _nMenu = 0;

    work = (Work*)DbgWork->pvFatWork;

    // デバック思考以外は現在選択している番号が出る
    if ( (*dbgthink_menu.target) == FATTHKDBG_OFF){
	work->nDbgLevel   = work->nC4Level;
	work->nDbgPattern = work->nC4Pattern;
    }else{
	// デバックメニュー操作
	if ((GV_PadData[ 0 ].status & PAD_L1)){
	    switch ( _nMenu ){
	    case DBG_DUMP_LEVEL_MENU_LV:
		if ((GV_PadData[ 0 ].press & PAD_L)){
		    work->nDbgLevel--;
		    if ( work->nDbgLevel < 0 ) work->nDbgLevel = FAT_C4_LEVEL - 1;
		}
		if ((GV_PadData[ 0 ].press & PAD_R)){
		    work->nDbgLevel++;
		    if ( work->nDbgLevel >= FAT_C4_LEVEL ) work->nDbgLevel = 0;
		}

		if ( (GV_PadData[ 0 ].press & PAD_U) ||
		     (GV_PadData[ 0 ].press & PAD_D) ){
		    _nMenu = DBG_DUMP_LEVEL_MENU_PT;
		}
		break;
	    case DBG_DUMP_LEVEL_MENU_PT:
		if ((GV_PadData[ 0 ].press & PAD_L)){
		    work->nDbgPattern--;
		    if ( work->nDbgPattern < 0 ) work->nDbgPattern = FAT_C4_PATTERN - 1;
		}
		if ((GV_PadData[ 0 ].press & PAD_R)){
		    work->nDbgPattern++;
		    if ( work->nDbgPattern >= FAT_C4_PATTERN ) work->nDbgPattern = 0;
		}
		if ( (GV_PadData[ 0 ].press & PAD_U) ||
		     (GV_PadData[ 0 ].press & PAD_D) ){
		    _nMenu = DBG_DUMP_LEVEL_MENU_LV;
		}
		break;
	    }
	}
    }

    // 箱表示
    {
	n = work->nDbgLevel;
	j = work->nDbgPattern;

	{
	    for ( i = 0; i < FAT_C4_POSNUM; i++){
		pputData = FAT_GetPosData( work, n, j, i);
		vec1.vx = pputData->vecPos.vx - 300.f;
		vec1.vy = pputData->vecPos.vy - 300.f;
		vec1.vz = pputData->vecPos.vz - 300.f;
		vec2.vx = pputData->vecPos.vx + 300.f;
		vec2.vy = pputData->vecPos.vy + 300.f;
		vec2.vz = pputData->vecPos.vz + 300.f;
		// 通し番号
		SIG_NumPrint(&pputData->vecPos, i);

		// 箱表示
		if ( pputData->nStatus != 0 ){
		    NewBoundingBoxView( &vec1, &vec2, 255, 0, 0 );
		}else{
		    switch ( FAT_PointType(pputData) & FAT_TYPE_CHECK){
		    case FAT_TYPE_UNDER:
			NewBoundingBoxView( &vec1, &vec2, 255, 255, 0);
			break;
		    case FAT_TYPE_STAND:
			NewBoundingBoxView( &vec1, &vec2, 0, 255, 0 );
			break;
		    case FAT_TYPE_UPPER:
			NewBoundingBoxView( &vec1, &vec2, 255, 0, 255 );
			break;
		    default:
			NewBoundingBoxView( &vec1, &vec2, 0, 255, 255 );
			break;
		    }
		}
	    }
	}
    }
    DEBUG_Locate( 330, 220, 0 );
    DEBUG_Printf( "LEVEL   = %2d \n", work->nDbgLevel + 1 );
    DEBUG_Printf( "PATTERN = %2d \n", work->nDbgPattern + 1 );
    if ((*dbgthink_menu.target) == FATTHKDBG_OFF)    DEBUG_Printf( "\n");
    else 					     DEBUG_Printf( "INDEX = %2d\n", work->nDbgIndex);	
    DEBUG_Printf( "\n");
    DEBUG_Printf( "ALREADY PUT = RED\n");
    DEBUG_Printf( "UNDER       = YELLOW\n");
    DEBUG_Printf( "STAND       = GREEN\n");
    DEBUG_Printf( "UPPER       = PURPLE\n");
    DEBUG_Printf( "RUN         = SKYBLUE\n");
    DEBUG_Printf( "OTHER       = BLUE\n");

    DEBUG_Locate( 320, 220 + 16 * _nMenu, 0 );
    DEBUG_Printf( "*\n");
}
static void FAT_DbgPutTypeView(DebugView *DbgWork ){}
static void FAT_DbgPutContView(DebugView *DbgWork ){}

static void DbgViewDie(void){}
static void DbgViewAct(DebugView *DbgWork )
{
    Work*     work;

    work = (Work*)DbgWork->pvFatWork;

    switch (*dbgview_menu.target){ // デバック
    case FATDBG_PUTLV:
	FAT_DbgPutLevelView( DbgWork );
	break;
    case FATDBG_PUTTYPE:
	FAT_DbgPutTypeView( DbgWork );
	break;
    case FATDBG_CONT:
	FAT_DbgPutContView( DbgWork );
	break;
    case FATDBG_BEHIND_POS:
//	FAT_DbgViewBehindPos( &work->unitBehind );
	break;
    case FATDBG_BEHIND_AREA:
//	FAT_DbgViewBehindArea( &work->unitBehind );
	break;
    }
}
// ファットマンC４びゅわー
void *NewFatC4PointView( work, name , where )
Work*	work;
int	name ;
int	where ;
{
    DebugView *DbgWork;

    DbgWork = (DebugView *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( DebugView ), 0 ) ;
    if ( DbgWork != NULL) {
	GV_SetActor( &DbgWork->actor, DbgViewAct, DbgViewDie) ;
	GV_ActorEX( &DbgWork->actor);
	DbgWork->count = 0;
	DbgWork->where = where;
	DbgWork->pvFatWork = work;
    }
    return (void *)DbgWork ;
}


static void Think3_DbgStand( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 向き
    if ( work->count3 == 0 ) {	// ゾーンアドレスからナビターゲットの情報を生成
	npc->action.pad = PAD_OVERLOOK;	// 
    }
	   
    npc->ctrl->turn.vy = work->aim_dir;

    work->count3++;
}

// 爆弾フェーズ : 移動
// ループモーション
static void Think3_DbgMove( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {	// ゾーンアドレスからナビターゲットの情報を生成
	if ( work->nNextZone == -1){	// 次候補がおかしい
	    FAT_SetThink3( work, TH3_STAND);
	    return;
	}

	// 設定
	FAT_CheckRouteZone( work, HZX_Zone1(npc->ctrl->addr), work->nNextZone);
    }

    npc->action.pad = PAD_RUN_ACT;	// 走行開始


//MAO_DbgDrawBox( &work->navitrg.pos, 300.f, 300.f, 300.f, 0xff, 0x00, 0x00);
//MAO_DbgDumpVector( &work->navitrg.pos);
//printf("%d %d * %d >> %d:::%f --- %d\n", HZX_Zone1(work->navitrg.addr), HZX_Zone1(work->navigate.going_addr), 
//       npc->action.dir, work->nMoveForm, work->fBezierRate, (work->body.flag & OBJECT_MOTIONSTEP_THROUGH) );

    if ( FAT_RouteZoneMove( work) ){
	return;
    } 

    work->count3++;
}
// 牽制攻撃
static void Think3_DbgShoot( Work* work )
{
    NPCWORK*	npc;

    npc = &work->npc;

    npc->action.pad = PAD_SHOT_READY; // 銃構え

    // グロック発射
    if ( work->count3 > COUNT_VMODE(15) ){
	if ( work->count3 % COUNT_VMODE( 4 ) == 0){
	    npc->action.pad = PAD_SHOT_ACT;	
	}
    }

    // 装填チェック
    if ( work->nBullet == 0 ){
	FAT_SetThink3( work, TH3_RELOAD);	
	return ;
    }

    // 終了
    if ( work->count3 > COUNT_VMODE(105) ) {
	FAT_SetThink3( work, TH3_STAND );	// 思考
	return;
    }

    // 向き変更
    FAT_AdjustAimPoint( work, &work->vecAimPos );

    work->count3++;
}
// グロック装填
static void Think3_DbgReload( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_RELOAD; 	// 装填動作にする
    }

    // 終了
    if ( work->npc.action.act_end ){
	FAT_SetThink3( work, TH3_STAND);	// 次の動作へ 
	return;
    }

    work->count3++;
}
// ポイント指定位置移動
static void Think3_DbgPointMove( Work *work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ){
	HZX_ZONE_ADD trgzone;
	// ゾーンアドレスからナビターゲットの情報を生成
	FAT_SetNextZone( work, HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &work->vecNextPos, -1) ));
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
	if ( HZX_InsideZone( npc->ctrl->hzx_id, &work->vecNextPos, work->nNextZone) ){
	    work->navitrg.pos  = work->vecNextPos;  // 目標位置セット
	}
    }

    npc->action.pad = PAD_POINT_MOVE;	// ポイント移動

    if ( work->nRunningPut && GM_NaviNear( npc->navi, npc->nvtrg, FAT_BOMB_DIST ) ) {	
	if ( work->bNext3Valid ){ // 次の指定があるなら
	    FAT_SetThink3FromNext( work, TH3_READY); 	// 準備動作へ
	    return;
	}
    }
    // 同一ゾーン内で移動量以上はなれているか
    if ( GM_NaviNear( npc->navi, &work->navitrg, POINT_MOVE_DIST ) ){
	FAT_SetThink3( work, TH3_STOP_PUT);		// 設置
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
// 走り置き
#define ZONE_POINT_DIST (1500)
static void Think3_RunningDbgPut( Work* work )
{
    FVECTOR		vec;
    NPCWORK*		npc;
    FAT_PUTPOINT_DATA*	pput;

    npc = &work->npc;
    pput = FAT_GetCurrPutPosData( work );

    // 初期設定
    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_SET_BOMB_RUN;	// 移動状態で爆弾設置
    }

    // 終了条件
    if ( work->npc.action.act_end ) {
	FAT_SetThink3( work, TH3_READY);	// 思考準備へ
	return;
    }

    // 向き	
    _sceVu0SubVector( &vec, &pput->vecPos, &npc->ctrl->mov);
    npc->ctrl->turn.vy = _FVecDir2(&vec);

    work->count3++;
}
// 回転置き
static void Think3_TurnDbgPut( Work* work )
{
    HZX_ZONE_ADD trgzone;
    NPCWORK*	npc;
    npc = &work->npc;

    // 初期設定
    if ( work->count3 == 0 ) {
	// 現在選択中の設置位置情報を取得
	FAT_SetNextZone( work, FAT_GetCurrentC4Zone( work));
	FAT_GetCurrentC4Put( work, &work->vecNextPos);

	// ゾーンアドレスからナビターゲットの情報を生成
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
        work->navitrg.pos  = work->vecNextPos;  // 目標位置セット

	work->bPutFlag = 0; // 設置フラグ初期化
	
	// 方向取得
	if ( FAT_GetTurnPutDir(work) < 0)	npc->action.pad = PAD_SET_BOMB_TURN_R;
	else					npc->action.pad = PAD_SET_BOMB_TURN_L;
    }

    if ( work->bPutFlag ){ // 設置したら次の目標を設定
	// ゾーンアドレスからナビターゲットの情報を生成
	FAT_SetNextZone( work, FAT_GetTurnPutRefZone( work )); // 回転置き用参照ゾーンデータから取得
	trgzone = HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone );
	GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone );
    }

    // 追跡
    GM_NaviNear( npc->navi, npc->nvtrg, FAT_BOMB_DIST );

    // 終了条件
    if ( work->npc.action.act_end ) {	
	if ( work->nPutNum < work->nBombMax ){
	    if ( FAT_SetNewC4Point( work ) ){		// 次のターン設置へ
		return;
	    }
	}

	FAT_SetThink3( work, TH3_READY);	// 思考準備へ
	return;
    }

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}
// 停止置き
static void Think3_StopDbgPut( Work* work )
{
    NPCWORK*	npc;
    FAT_PUTPOINT_DATA*	pput;
    FVECTOR	vecDir;
    int		nDir;

    npc = &work->npc;
    pput = FAT_GetCurrPutPosData( work );
    _sceVu0SubVector( &vecDir, &pput->vecPos, &npc->ctrl->mov);
    nDir = _FVecDir2(&vecDir);

    if ( work->count3 == 0 ) {
	if ( GV_DiffDirAbs( npc->ctrl->rot.vy, nDir) >= 128 ){ // 向くまでcount3は0
	    // 向き
	    npc->ctrl->turn.vy = nDir;
	}else{
	    // 設置方式でモーション分岐
	    npc->action.pad = PAD_SET_BOMB_STOP;	

	    work->count3 = 1;
	}
    }else{
	// 終了
	if ( work->npc.action.act_end ) {
	    FAT_SetThink3( work, TH3_READY);	// 爆弾設置点に移動
	    return;
	}
	// 向き
	npc->ctrl->turn.vy = nDir;
	// カウンタ更新
	work->count3++;
    }
}

static void FAT_DbgThink(Work* work)
{
Think3_DbgShoot( work );
return;

    //    FAT_Move( work );
    //    return;

    if ((GV_PadData[ 0 ].status & PAD_L1)){ // 爆弾置きデバック
	if ((GV_PadData[ 0 ].press & PAD_AL)){
	    work->nDbgIndex--;
	    if ( work->nDbgIndex < 0 ) work->nDbgIndex = FAT_C4_POSNUM - 1;
	    FAT_UpdatePutPos( work );
	}else if ((GV_PadData[ 0 ].press & PAD_AR)){
	    work->nDbgIndex++;
	    if ( work->nDbgIndex >= FAT_C4_POSNUM ) work->nDbgIndex = 0;
	    FAT_UpdatePutPos( work );
	}
    }
    if ((GV_PadData[ 0 ].status & PAD_AL)){ // 爆弾置きデバック
	FAT_SetNextZone( work, GetRandZone( work ));
	FAT_SetThink3( work, TH3_BACK_SHOOT);
    }	
    if ((GV_PadData[ 0 ].status & PAD_AR)){ // 爆弾置きデバック
	FAT_SetThink3( work, TH3_REVERSE);
    }	
    switch( work->think3 ){
    case TH3_READY:		// 準備
	Think3_DbgStand( work );
	break;
    case TH3_STAND:		// 待機
	Think3_DbgStand( work );
	break;
    case TH3_MOVE:		// 移動
	Think3_DbgMove( work );
	break;
    case TH3_REVERSE:		// 反転
	Think3_Turn180( work );
	break;
    case TH3_BACK_SHOOT:	// 射撃
	Think3_BackShoot( work );
	break;
    case TH3_FEINT_SHOOT:	// 射撃
	Think3_DbgShoot( work );
	break;
    case TH3_RELOAD:		// リロード
	Think3_DbgReload( work );
	break;
    case TH3_RUN_RELOAD:
	Think3_RunReload( work );
	break;
    case TH3_POINT_MOVE:	// 指定点に移動
	Think3_DbgPointMove( work );
	break;
    case TH3_RUN_PUT:		// 移動中に爆弾セット
	Think3_RunningDbgPut( work );
	break;
    case TH3_STOP_PUT:		// 停止して爆弾セット
	Think3_StopDbgPut( work );
	break;	
    case TH3_TURN_PUT:		// 回転置き
	Think3_TurnDbgPut( work );
	break;
    case TH3_OVERLOOK:
	Think3_OverLook( work );
	break;	
    }
}
// デバックメニュー対応のステータスビュワー
static void FAT_DbgViewStatus( Work* work )
{
    if ( *debug_menu.target ){ // デバック

	MENU_Locate( 100, 320, 0 );
	MENU_SetColor( 0x00, 0x00, 0x80 );
	MENU_Printf("%d %d", work->nShootPhaseCntr, work->nBombPhaseCntr);

    printf( "TH1=%2d : TH2=%2d : TH3=%2d : NT3=%2d :: %d -> 0x%x\n", work->think1, work->think2, work->think3, work->nNextThink3, work->npc.action.pad, work->npc.actmode_call);
	DEBUG_Locate( 40, 120, 0 );
	DEBUG_Printf( "KILL-CNT=%d\n", GM_KillCount );
	DEBUG_Printf("GAME-LEVEL:: %d\n", GM_GameLevel);
	DEBUG_Printf( "TH1=%2d : TH2=%2d : TH3=%2d : NT3=%2d\n", work->think1, work->think2, work->think3, work->nNextThink3);
	DEBUG_Printf( "CT3=%2d : PC=%10d\n", work->count3, work->nPhaseCntr);
	DEBUG_Printf( "ZONE = N=%4d : I=%3d : F=%1d\n", work->nNextZone, work->nC4Index, work->bPutIndexUsed);
	DEBUG_Printf( "DIST = %5d %5d, DIR  = %4d %4d\n", work->pl_dis, work->aim_dis, work->pl_dir, 
		      GV_DiffDirAbs( work->pl_dir, work->npc.ctrl->rot.vy));
	DEBUG_Printf( "PAD  = %2d : %2d :: 0x%x\n", work->npc.action.pad, work->npc.action.set_pad, 
		       work->npc.actmode_call );
	DEBUG_Printf( "C4   = B=%2d : C=%2d : P=%2d : X=%2d : F=%2d : C=%5d\n", 
		      work->nBombNum, work->nBombCountNum, work->nPutNum, work->nBombMax, 
		      work->nFreezeNum, work->nBombCntr);
	DEBUG_Printf( "C4PUT= Lv=%2d : Pt=%2d : In=%2d\n", work->nC4Level, work->nC4Pattern, work->nC4Index);
	DEBUG_Printf( "VISIBLE = 0x%08x : 0x%08x\n", work->pl_parts, work->fat_parts);
	DEBUG_Printf( "Bullet = %d\n", work->nBullet);
	DEBUG_Printf( "Sound = I=%2d : H=%4d : C=%5d : S=%3d\n\n", work->nStrmIndexNext, 
		      work->nStrmHandle, work->nStrmCntr, 
		      (work->nStrmHandle != -1) ? GM_StreamStatus( work->nStrmHandle ) : 0);
	
	if ((GV_PadData[ 0 ].press & PAD_AR)){
	    printf( "TH1=%2d : TH2=%2d : TH3=%2d : NT3=%2d\n", work->think1, work->think2, 
			  work->think3, work->nNextThink3);
	    printf( "CT3=%2d : PC=%10d\n", work->count3, work->nPhaseCntr);
	    printf( "ZONE = N=%4d : I=%3d : F=%1d\n", work->nNextZone, work->nC4Index, work->bPutIndexUsed);
	    printf( "DIST = %5d %5d, DIR  = %4d %4d\n", work->pl_dis, work->aim_dis, work->pl_dir, 
			  GV_DiffDirAbs( work->pl_dir, work->npc.ctrl->rot.vy));
	    printf( "PAD  = %2d : %2d :: Faint= %4d\n", work->npc.action.pad, work->npc.action.set_pad, 
			  work->npc.action.faint );
	    printf( "C4   = B=%2d : C=%2d : P=%2d : X=%2d : F=%2d : C=%5d\n", 
			  work->nBombNum, work->nBombCountNum, work->nPutNum, work->nBombMax, 
			  work->nFreezeNum, work->nBombCntr);
	    printf( "C4PUT= Lv=%2d : Pt=%2d : In=%2d\n", work->nC4Level, work->nC4Pattern, work->nC4Index);
	    printf( "VISIBLE = 0x%08x : 0x%08x\n", work->pl_parts, work->fat_parts);
	    printf( "Bullet = %d\n", work->nBullet);
	    printf( "Sound = I=%2d : H=%4d : C=%5d : S=%3d\n\n", work->nStrmIndexNext, 
			  work->nStrmHandle, work->nStrmCntr, 
			  (work->nStrmHandle != -1) ? GM_StreamStatus( work->nStrmHandle ) : 0);
	}
    }
}
#if 0
// 強制２１関節以降を修正
static void FAT_TestFace( Work* work )
{
    NPCWORK*	npc;
    npc = &work->npc;

    NPC_SetActMotionEX( npc, 2,
			npc->base_mar, FAT_MOT_FACEFIX, 0xffffffffffe00000, ACT_INTERP_DEF, (float)TIME_BASE ) ;
    SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 2 ].flag, MT3_ACTIVE ) ;
printf("=============== face fix mode on ================\n");
}
#endif

// -------------------ファットマンステータス デバック
// ファットマン捕捉位置表示
static void FAT_DbgDrawEyePos( Work* work )
{
    MAO_DbgDrawBox( &work->vecAimPos, 300.f, 300.f, 300.f, 0xff, 0x00, 0x00);
}
// ファットマン進行ルート表示
static void FAT_DbgDrawRoute( Work* work )
{
    int 	i, addr;
    FVECTOR 	vec;
    NPCWORK* 	npc;

    npc = &work->npc;
    addr = npc->navi->next_addr;
    vec = npc->ctrl->mov;

    for (i = 0; i < 7; i++){
	HZX_ZONE_ADD hzaAddr;

	if ( addr == -1 || addr == npc->navi->going_addr)
	    return;

	FAT_GetZoneInPoint( &vec, &vec, addr);
	MAO_DbgDrawBox( &vec, 100, 100, 100, 0x00, 0x00, 0xff ); // ターゲット位置

	hzaAddr = HZX_NextZoneCrossGroup( addr, npc->navi->going_addr);
	addr = HZX_Zone1(hzaAddr);
    }

}
// サウンドデバッグ
static void FAT_DbgSound(Work* work)
{
    static int nStrm = 0;
    static int nSe = 0;
    static int nSeMode = GM_SEMODE_NORMAL;

    // ストリームデバック
    if ((GV_PadData[ 0 ].press & PAD_L)){
	nStrm--;
	if (nStrm < 0)
	    nStrm = work->nStrmDataNum - 1;
	FAT_StreamRequest( work, nStrm);
    }
    if ((GV_PadData[ 0 ].press & PAD_R)){
	nStrm++;
	if (nStrm >= work->nStrmDataNum)
	    nStrm = 0;
	FAT_StreamRequest( work, nStrm);
    }
    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_D)){
	nSe--;
	if (nSe < 0)
	    nSe = SE_MAX - 1;

#ifdef ENGLISH
	switch( nSe ){
	case SE_FATSTN01: 
	case SE_FATSTN02: 
	case SE_FATDMB01: 
	case SE_FATDMB02: 
	case SE_FATDOKN0: 
	case SE_FATWHE01: 
	case SE_FATWHE02: 
	case SE_FATNOD01: 
	case SE_FATNOD02: 
	case SE_FATCUT01: 
	case SE_FATCUT02:
	    GM_JimakuSeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	    break;
	default:
	    GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	}
#else
	GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
#endif
    }
    if ((GV_PadData[ 0 ].press & PAD_U)){
	nSe++;
	if (nSe >= SE_MAX)
	    nSe = 0;
#ifdef ENGLISH
	switch( nSe ){
	case SE_FATSTN01: 
	case SE_FATSTN02: 
	case SE_FATDMB01: 
	case SE_FATDMB02: 
	case SE_FATDOKN0: 
	case SE_FATWHE01: 
	case SE_FATWHE02: 
	case SE_FATNOD01: 
	case SE_FATNOD02: 
	case SE_FATCUT01: 
	case SE_FATCUT02:
	    GM_JimakuSeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	    break;
	default:
	    GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	}
#else
	GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
#endif
    }

    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_L1)){
#ifdef ENGLISH
	switch( nSe ){
	case SE_FATSTN01: 
	case SE_FATSTN02: 
	case SE_FATDMB01: 
	case SE_FATDMB02: 
	case SE_FATDOKN0: 
	case SE_FATWHE01: 
	case SE_FATWHE02: 
	case SE_FATNOD01: 
	case SE_FATNOD02: 
	case SE_FATCUT01: 
	case SE_FATCUT02:
	    GM_JimakuSeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	    break;
	default:
	    GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
	}
#else
	GM_SeSetMode( FAT_SE[nSe], &GM_PlayerPosition, GM_SEMODE_BOMB);
#endif
    }
    DEBUG_Locate( 340, 300, 0 );
    DEBUG_Printf( "STREAM = %2d \n", nStrm);
    DEBUG_Printf( "SE     = %2d \n", nSe);
    DEBUG_Printf( "SE     = %s \n", (nSeMode == GM_SEMODE_NORMAL)? "NORMAL" : "BOMB");
}
// ターゲット箱描画
static void FAT_DbgDrawTrgBox( Work* work )
{
    int i;

    if ( *dbgstatus_menu.target == FATSTDBG_TRG ){
	NewTargetView( &work->deftrg, 32, 255, 32 ) ;
	for (i=0;i<CHILD_TARGET_NUM;i++){
	    NewTargetView( &work->def_child[i], 200, 34, 184 ) ;
	}
    }
}

static void FAT_DbgStatusUpdate( Work* work )
{
    NPCWORK* 	npc;

    npc = &work->npc;

    switch ( *dbgstatus_menu.target ){ // デバック
    case FATSTDBG_MUTEKI:	// 無敵
	{
	    FVECTOR vec1, vec2;
	    FMATRIX mat;

	    DG_SetPos2( &npc->ctrl->mov, &npc->ctrl->rot );
	    DG_GetPos( &mat );
	    MAO_SETFVEC( vec1, -300.f, -300.f, -300.f );
	    _sceVu0AddVector( &vec1, &vec1, &npc->ctrl->mov);
	    MAO_SETFVEC( vec2, 300.f, 300.f, 300.f );
	    _sceVu0AddVector( &vec2, &vec2, &npc->ctrl->mov);
	    if ( work->npc.action.status & NPC_ACT_STATUS_TRG_OFF)
		NewBoundingBoxView( &vec1, &vec2, 0, 0, 255);
	    else
		NewBoundingBoxView( &vec1, &vec2, 255, 0, 0);	    
	}
	break;
    case FATSTDBG_EYEPOS:	// 捕捉
	FAT_DbgDrawEyePos( work );
	break;
    case FATSTDBG_ROUTE:	// ルート
	FAT_DbgDrawRoute( work );
	break;
    case FATSTDBG_SOUND:	// サウンド 
	FAT_DbgSound( work );
	break;
    }
}

#ifdef DEBUG_MODE

static void FAT_CallDbgDump(void){
    GM_AddDebugMenu( &debug_menu );	// ダンプデバック登録
}
static void FAT_CallDbgViewer(void){
    GM_AddDebugMenu( &dbgview_menu );	// データデバック登録
}
static void FAT_CallDbgStatus(void){
    GM_AddDebugMenu( &dbgstatus_menu );	// ステータスデバック登録
}
static void FAT_CallDbgThink(void){
    GM_AddDebugMenu( &dbgthink_menu );	// 思考デバック登録
}
static void FAT_CallDbgMotion(void){
    GM_AddDebugMenu( &dbgmot_menu );	// モーションデバック登録
}
static void FAT_CallDbgBomb(void){
    GM_AddDebugMenu( &dbgbomb_menu );	// データデバック登録
}

#else
// デバック解除
static void FAT_CallDbgDump(void){}
static void FAT_CallDbgViewer(void){}
static void FAT_CallDbgStatus(void){}
static void FAT_CallDbgThink(void){}
static void FAT_CallDbgMotion(void){}
#endif

// デバック用爆弾設置更新
static int FAT_UpdatePutPos(Work* work)
{
    FAT_PUTPOINT_DATA*  pput;

    // 強制書き換え
    work->nC4Level   = work->nDbgLevel;
    work->nC4Pattern = work->nDbgPattern;
    work->nC4Index   = work->nDbgIndex;

    pput = &work->datPoint[ work->nC4IndexData[work->nDbgLevel][work->nDbgPattern][work->nDbgIndex] ];
    if ( pput->nType == FAT_TYPE_RUN ){ // 走り置きタイプ
	int	nZone;
	nZone = HZX_Zone1( pput->zoneAddr ); 	// 設置ゾーン設定
	work->vecNextPos = pput->vecPut;	// ポイント設定
	FAT_SetNextZone( work, nZone);		// 目標ゾーン設定
	FAT_SetNextThink3( work, TH3_TURN_PUT);	// 回転置き
	FAT_SetThink3( work, TH3_POINT_MOVE);	// 思考準備へ
	work->nRunningPut = 1;			// 走り置きフラグON
	return 1;
    }else{
	FAT_SetNextZone( work, pput->zoneAddr);	// 目標ゾーン設定
	work->vecNextPos = pput->vecPut;		// ポイント設定
	FAT_SetThink3( work, TH3_POINT_MOVE);		// 爆弾設置移動	
	work->nRunningPut = 0;
	return 1;
    }
}
// モーションデバック
static void FAT_DbgMotionUpdate( Work* work)
{
    NPCWORK*	npc;
	
    static int _CurrMot[2] = { 0, 0 }; 
    static int	nInterval  = 0;
    static int	nLayer     = 0;
    static int	nLoop      = 0;

    npc = &work->npc;

    if (nInterval > 0) nInterval--;

    if ( *dbgmot_menu.target ){
	work->body.flag |= OBJECT_MOTIONSTEP_THROUGH;

	if ((GV_PadData[ 0 ].right_dy > 223)){ // 
	    if ( nInterval == 0){
		nInterval = 20;
		if ( _CurrMot[nLayer]++ >= FAT_MOT_MAX) _CurrMot[nLayer] = 0;

		if ( nLayer == 0){
		    NPC_SetActMotionEX( npc, nLayer, npc->base_mar, _CurrMot[nLayer], 
					0xffffffffffe00000 | MOTION_MASK_UPPER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
		}else{
		    NPC_SetActMotionEX( npc, nLayer, npc->base_mar, _CurrMot[nLayer], 
					MOTION_MASK_LOWER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
		}
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ nLayer ].flag, MT3_ACTIVE ) ;
	    }
	}
	if ((GV_PadData[ 0 ].right_dy < 32)){
	    if ( nInterval == 0){
		nInterval = 20;
		if ( _CurrMot[nLayer]-- <= 0) _CurrMot[nLayer] = FAT_MOT_MAX - 1;

		if ( nLayer == 0){
		    NPC_SetActMotionEX( npc, nLayer, npc->base_mar, _CurrMot[nLayer], 
					0xffffffffffe00000 | MOTION_MASK_UPPER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
		}else{
		    NPC_SetActMotionEX( npc, nLayer, npc->base_mar, _CurrMot[nLayer], 
					MOTION_MASK_LOWER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
		}
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ nLayer ].flag, MT3_ACTIVE ) ;
	    }
	}
	if ((GV_PadData[ 0 ].press & PAD_L1)){ // 再生リセット
	    NPC_SetActMotionEX( npc, 0, npc->base_mar, _CurrMot[0], 
				0xffffffffffe00000 | MOTION_MASK_UPPER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
	    NPC_SetActMotionEX( npc, 1, npc->base_mar, _CurrMot[1], 
				MOTION_MASK_LOWER, ACT_INTERP_DEF, (float)TIME_BASE ) ;
	    SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 0 ].flag, MT3_ACTIVE ) ;
	    SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_ACTIVE ) ;
	}
	if ((GV_PadData[ 0 ].press & PAD_AL)){ // ループ選択
            nLoop++;
	    nLoop %= 2;
	}
	if ((GV_PadData[ 0 ].press & PAD_AR)){ // レイヤー選択
            nLayer++;
	    nLayer %= 2;
	}

	if ( !nLoop ){ // ループチェック
	    if ( GM_CheckObject_IsEnd( npc->body, 0 ) ){
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 0 ].flag, MT3_SLEEP);
	    }
	    if ( GM_CheckObject_IsEnd( npc->body, 1 ) ){
		SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ 1 ].flag, MT3_SLEEP);
	    }
	}

	MENU_Locate( 300, 320, 0 );
	MENU_SetColor( 0x00, 0x00, 0x80 );
	MENU_Printf( "UPPER-MOT = %d %.2f\n", _CurrMot[0], (float)work->body.m_ctrl->mt3_ctrl[0].play_time / 
		     (float)work->body.m_ctrl->mt3_ctrl[0].motion_total_time);
	MENU_Printf( "LOWER-MOT = %d %.2f\n", _CurrMot[1], (float)work->body.m_ctrl->mt3_ctrl[1].play_time / 
		     (float)work->body.m_ctrl->mt3_ctrl[1].motion_total_time);
	MENU_Printf( "\n");
	MENU_Printf( "LOOP      = %d \n", nLoop );
	if ( nLayer == 0) MENU_Locate( 280, 320, 0 );
	else		  MENU_Locate( 280, 336, 0 );
	MENU_Printf( "*\n");
    }else{
	work->body.flag &= ~OBJECT_MOTIONSTEP_THROUGH;
    }
}
#if 0
// ルート描画
static void FAT_CheckRoute( Work* work)
{
    int i,j, nIndex, nReverse, nRev, nDist, nDistTmp, nMode;
    FVECTOR vecPos, vecPre;
    FVECTOR vec[FAT_BEZIER_RATE*2 + 2];
    NPCWORK*	npc;

    npc = &work->npc;

    nDist = HZX_ZoneDistance( npc->ctrl->hzx_id, HZX_Zone1(npc->ctrl->addr), work->nNextZone); // 目標までのゾーン距離
    nIndex = -1;
    nReverse = 0;
    nRev = 0;
    
    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++ ){
	nMode = 0;
	// ファットマンのいるゾーンが始点になるルートかどうかチェック
	if ( work->nRouteZone[j][0] == HZX_Zone1( npc->ctrl->addr) && 
	     _MAO_FVec2Len2( &npc->ctrl->mov, &FAT_BEZ_MOVE_POS[j][0]) < 2400.f ){
	    nRev = 0; // 正ルート
	}else if (work->nRouteZone[j][1] == HZX_Zone1( npc->ctrl->addr) && 
		  _MAO_FVec2Len2( &npc->ctrl->mov, &FAT_BEZ_MOVE_POS[j][2]) < 2400.f ){
	    nRev = 1; // 逆ルート
	}else{
	    continue;
	}
	// ゾーンとルートが接触するかチェック
	if ( FAT_CheckBezierInTetra( work, work->nNextZone, j) ){
	    nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
	    if ( nDistTmp < nDist){
		nIndex = j;
		nReverse = nRev;
		nMode = 1;
	    }
	}

	if ( nRev ){
	    // 逆ルート
	    if ( FAT_ChkCounterRoute( work, j, HZX_Zone1( npc->ctrl->addr), work->nNextZone ) ){
		nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][0], work->nNextZone);
		if (nDistTmp < nDist){
		    nDist = nDistTmp;
		    nIndex = j;
		    nReverse = 1; 
		    nMode = 3;
		}
	    }
	}else{
	    // 正ルート
	    if ( FAT_ChkRegularRoute( work, j, HZX_Zone1( npc->ctrl->addr), work->nNextZone ) ){
		nDistTmp = HZX_ZoneDistance( npc->ctrl->hzx_id, work->nRouteZone[j][1], work->nNextZone);
		if (nDistTmp < nDist){
		    nDist = nDistTmp;
		    nIndex = j;
		    nReverse = 0; 
		    nMode = 2;
		}
	    }
	}
#if 0
	// ベジェ補間用のマトリクスを計算しておく
	MAO_MakeBezierMatrix( &mat, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);
	vecPre = FAT_BEZ_MOVE_POS[j][0];
	for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
	    MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
	    vec[i*2+0] = vecPre;
	    vec[i*2+1] = vecPos;
	    vecPre = vecPos;
	}
	vec[FAT_BEZIER_RATE*2]   = vecPre;
	vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][2];
	NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0xff, 0x00, 0x00);
#else
	{ // エルミート補間
	    vecPre = FAT_BEZ_MOVE_POS[j][0];
	    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
		float fDist;
		FVECTOR vecStartZ, vecEndZ, vecSub, vecIncMod1, vecIncMod2, vecDist;
		FMATRIX mat;
	
		DG_SetPos2( &work->npc.ctrl->mov, &work->npc.ctrl->rot);
		DG_GetPos( &mat);

		FAT_DbgCalcBezierIncMod( &vecIncMod1, &vecIncMod2, &FAT_BEZ_MOVE_POS[j][0], 
					 &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);

		_sceVu0SubVector( &vecDist, &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][0] );		// 差分ベクトルを求める
		fDist = DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );	// １と２の距離を取得
		_sceVu0SubVector( &vecDist, &FAT_BEZ_MOVE_POS[j][2], &FAT_BEZ_MOVE_POS[j][1] );		// 差分ベクトルを求める
		fDist += DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );// ２と３の距離を取得して加算

		FAT_BezierInc( &vecStartZ, &vecIncMod1, &vecIncMod2, 0.f );
		_sceVu0ScaleVector( &vecStartZ, &vecStartZ, fDist);
		FAT_BezierInc( &vecEndZ, &vecIncMod1, &vecIncMod2, 1.f );
		_sceVu0ScaleVector( &vecEndZ, &vecEndZ, fDist);
		MAO_HermiteLerpVec( &vecPos, &vecSub, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][2], 	
				    &vecStartZ, &vecEndZ, (float)i / (float)FAT_BEZIER_RATE);
		vec[i*2+0] = vecPre;
		vec[i*2+1] = vecPos;
		vecPre = vecPos;
	    }
	    vec[FAT_BEZIER_RATE*2+0] = vecPre;
	    vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][2];
	    switch ( nMode ){
	    case 0:
		NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0x00, 0x00);
		break;
	    case 1:
		NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0xff, 0x00, 0x00);
		break;
	    case 2:
		NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0xff, 0x00);
		break;
	    case 3:
		NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0x00, 0xff);
		break;
	    }
	}
#endif
    }
}
// 始点描画
static void FAT_CheckFirstPos( Work* work)
{
    int j;
    
    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++ ){
	AN_Test_Eye2( &FAT_BEZ_MOVE_POS[j][0], 1 ); // 始点位置
    }
}
// ルートチェック
static void FAT_DbgCheckRoute( Work* work)
{
    int i, j, nCnt, nCntR;
    int time;
    FVECTOR vecPos, vecPre;
    FVECTOR vec[2*FAT_ROUTEDATA_NUM+2];
    FMATRIX mat;

GV_SET_PRFC_CLOCK();
    
    nCnt = 0;
    nCntR = 0;
    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++ ){
	// 正ルート
	if ( work->nRouteZone[j][0] == HZX_Zone1( work->npc.ctrl->addr ) &&
	     ( FAT_CheckBezierInTetra( work, work->nNextZone, j) || 
	     FAT_ChkRegularRoute( work, j, HZX_Zone1( work->npc.ctrl->addr ), work->nNextZone )) ){
	    MAO_MakeBezierMatrix( &mat, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);
	    vecPre = FAT_BEZ_MOVE_POS[j][0];
	    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
		// 補間
		MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
		vec[i*2+0] = vecPre;
		vec[i*2+1] = vecPos;
		vecPre = vecPos;
	    }
	    vec[FAT_BEZIER_RATE*2+0] = vecPre;
	    vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][2];
	    NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0xff, 0x00, 0x00);

	    SIG_NumPrint(&FAT_BEZ_MOVE_POS[j][0], j);
	    nCnt++;
	}
	// 逆ルート
	if ( work->nRouteZone[j][1] == HZX_Zone1( work->npc.ctrl->addr ) &&
	     ( FAT_CheckBezierInTetra( work, work->nNextZone, j) || 
	     FAT_ChkCounterRoute( work, j, HZX_Zone1( work->npc.ctrl->addr ), work->nNextZone ) )){

	    MAO_MakeBezierMatrix( &mat, &FAT_BEZ_MOVE_POS[j][2], &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][0]);
	    vecPre = FAT_BEZ_MOVE_POS[j][2];
	    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
		// 補間
		MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
		vec[i*2+0] = vecPre;
		vec[i*2+1] = vecPos;
		vecPre = vecPos;
	    }
	    vec[FAT_BEZIER_RATE*2+0] = vecPre;
	    vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][0];
	    NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0x00, 0xff);

	    SIG_NumPrint(&FAT_BEZ_MOVE_POS[j][0], j);
	    nCntR++;
	}
    }
time = GV_GET_PRFC_CLOCK();
    MENU_Locate( 40, 310, 0 );
    MENU_SetColor( 0x20, 0x20, 0x20 );
    MENU_Printf("REGULAR = %d\n", nCnt);
    MENU_Printf("COUNTER = %d\n", nCntR);
    MENU_Printf("Next    = %d\n", work->nNextZone);
    MENU_Printf("EXECTIME = %.3f\n", (float)time / 60.f / 262.f );
}

// 傾き計算用の係数を求める
static void FAT_DbgCalcBezierIncMod( FVECTOR* pvecIncMod1, FVECTOR* pvecIncMod2, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3)
{
    FVECTOR vecTmp1, vecTmp2, vecTmp3;

    // tの係数の方
    _sceVu0ScaleVector( &vecTmp1, pvec1, 2.f);
    _sceVu0ScaleVector( &vecTmp2, pvec2, -4.f);
    _sceVu0ScaleVector( &vecTmp3, pvec3, 2.f);
    _sceVu0AddVector( pvecIncMod1, &vecTmp1, &vecTmp2);
    _sceVu0AddVector( pvecIncMod1, pvecIncMod1, &vecTmp3);
    
    _sceVu0ScaleVector( &vecTmp1, pvec1, -2.f);
    _sceVu0ScaleVector( &vecTmp2, pvec2, 2.f);
    _sceVu0AddVector( pvecIncMod2, &vecTmp1, &vecTmp2);
}
#endif
#if 0
// ルートが指定ゾーンを通過するかチェック
static void FAT_CheckPassZone( Work* work, int nZone)
{
    int i, j;
    FVECTOR vecPos, vecPre;
    FVECTOR vec[2*FAT_ROUTEDATA_NUM+2];
    FMATRIX mat;

    for ( j = 0; j < FAT_ROUTEDATA_NUM; j++){
	if ( FAT_CheckBezierInTetra( work, nZone, j) ){
	    MAO_MakeBezierMatrix( &mat, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);
	    vecPre = FAT_BEZ_MOVE_POS[j][0];
	    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
		// 補間
		MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
		vec[i*2+0] = vecPre;
		vec[i*2+1] = vecPos;
		vecPre = vecPos;
	    }
	    vec[FAT_BEZIER_RATE*2+0] = vecPre;
	    vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][2];
	    NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0xff, 0x00);

	    vecPre = FAT_BEZ_MOVE_POS[j][0];
	    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
		float fDist;
		FVECTOR vecStartZ, vecEndZ, vecSub, vecIncMod1, vecIncMod2, vecDist;
		FMATRIX mat;
	
		DG_SetPos2( &work->npc.ctrl->mov, &work->npc.ctrl->rot);
		DG_GetPos( &mat);

		FAT_DbgCalcBezierIncMod( &vecIncMod1, &vecIncMod2, &FAT_BEZ_MOVE_POS[j][0], 
					 &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][2]);

		_sceVu0SubVector( &vecDist, &FAT_BEZ_MOVE_POS[j][1], &FAT_BEZ_MOVE_POS[j][0] );		// 差分ベクトルを求める
		fDist = DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );	// １と２の距離を取得
		_sceVu0SubVector( &vecDist, &FAT_BEZ_MOVE_POS[j][2], &FAT_BEZ_MOVE_POS[j][1] );		// 差分ベクトルを求める
		fDist += DG_SQRT( vecDist.vx * vecDist.vx + vecDist.vy * vecDist.vy + vecDist.vz * vecDist.vz );// ２と３の距離を取得して加算

		FAT_BezierInc( &vecStartZ, &vecIncMod1, &vecIncMod2, 0.f );
		_sceVu0ScaleVector( &vecStartZ, &vecStartZ, fDist);
		FAT_BezierInc( &vecEndZ, &vecIncMod1, &vecIncMod2, 1.f );
		_sceVu0ScaleVector( &vecEndZ, &vecEndZ, fDist);
		MAO_HermiteLerpVec( &vecPos, &vecSub, &FAT_BEZ_MOVE_POS[j][0], &FAT_BEZ_MOVE_POS[j][2], 	
				    &vecStartZ, &vecEndZ, (float)i / (float)FAT_BEZIER_RATE);
		vec[i*2+0] = vecPre;
		vec[i*2+1] = vecPos;
		vecPre = vecPos;
	    }
	    vec[FAT_BEZIER_RATE*2+0] = vecPre;
	    vec[FAT_BEZIER_RATE*2+1] = FAT_BEZ_MOVE_POS[j][2];
	    NewLineView( &vec[0], (FAT_BEZIER_RATE+1), 0x00, 0x00, 0xff);

	    SIG_NumPrint(&FAT_BEZ_MOVE_POS[j][0], j);
#if 0
	    {
		int nDist;
		MAO_GetDiffVec3( NULL, &nDist, NULL, GM_PlayerPosition, FAT_BEZ_MOVE_POS[j][0]);
		if ( nDist < 800){
		    printf(" %d ", j);
		}
		MAO_GetDiffVec3( NULL, &nDist, NULL, GM_PlayerPosition, FAT_BEZ_MOVE_POS[j][2]);
		if ( nDist < 800){
		    printf(" @%d ", j);
		}
	    }
#endif
	}
    }
    //    printf("\n", j);
}
static void FAT_DbgBezier( Work* work)
{
    int i;
    int time;

    static int nStep = 0;
    static int nInterval  = 0;
    static int bRouteCheck = 0;

    FMATRIX mat;
    FVECTOR vecPos, vecPre;
    FVECTOR vec[FAT_BEZIER_RATE*2];

    static FVECTOR VEC_TEST[3] = {
	{  52121.9f,  12565.6f, -90402.7f }, // 
	{  56136.0f,  12551.1f, -86597.0f }, // 
	{  50875.8f,  12547.9f, -85171.8f }, // 
    };

    if (nInterval > 0) nInterval--;

    if ((GV_PadData[ 0 ].right_dy < 32)){ // 
	if ( nInterval == 0){
	    nInterval = 20;
	    if ( nStep++ >= 3) nStep = 0;
	}
    }else if ((GV_PadData[ 0 ].right_dy > 223)){
	if ( nInterval == 0){
	    nInterval = 20;
	    if ( nStep-- < 0) nStep = 2;
	}
    }

    switch ( nStep){
    case 0:
	_sceVu0CopyVector(&VEC_TEST[0], &GM_PlayerPosition);
	break;
    case 1:
	_sceVu0CopyVector(&VEC_TEST[2], &GM_PlayerPosition);
	break;
    case 2:
	_sceVu0CopyVector(&VEC_TEST[1], &GM_PlayerPosition);
	break;
    }

GV_SET_PRFC_CLOCK();
    vecPre = VEC_TEST[0];
    MAO_MakeBezierMatrix( &mat, &VEC_TEST[0], &VEC_TEST[1], &VEC_TEST[2]);
    for ( i = 0; i < FAT_BEZIER_RATE; i++ ){
	// 補間
//	MAO_Bezier3Interp( &vecPos, &VEC_TEST[0], &VEC_TEST[1], &VEC_TEST[2], (float)i / (float)FAT_BEZIER_RATE);
	MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
	vec[i*2+0] = vecPre;
	vec[i*2+1] = vecPos;
	vecPre = vecPos;
    }
time = GV_GET_PRFC_CLOCK();

    NewLineView( &vec[0], FAT_BEZIER_RATE, 0x00, 0xff, 0x00);
    for ( i = 0; i < 3; i++){
	AN_Test_Eye2( &VEC_TEST[i], 1 ); // ターゲット位置
    }

    MENU_Locate( 240, 10, 0 );
    MENU_SetColor( 0x20, 0x20, 0x20 );
    MENU_Printf("BEZIER-EXECTIME = %.3f\n", (float)time / 60.f / 262.f );
    switch( nStep ){
    case 0:
	MENU_Printf("START-POS\n");
	break;
    case 1:
	MENU_Printf("END-POS\n");
	break;
    case 2:
	MENU_Printf("CONTROL-POS\n");
	break;
    }

    // ダンプ
    if ( (GV_PadData[ 0 ].press & PAD_L1) ){
	printf("{ // \n");
	for ( i = 0; i < 3; i++){
	    printf("  { %8.1ff, %8.1ff, %8.1ff },\n", VEC_TEST[i].vx, VEC_TEST[i].vy, VEC_TEST[i].vz);
	}
	printf("}, \n");
    }
    // 全点を終点だった位置に集める
    if ( (GV_PadData[ 0 ].press & PAD_AR) ){
	_sceVu0CopyVector( &VEC_TEST[0], &VEC_TEST[2]);
	_sceVu0CopyVector( &VEC_TEST[1], &VEC_TEST[2]);
	nStep = 1;
    }
    // 全点を終点だった位置に集める
    if ( (GV_PadData[ 0 ].press & PAD_AL) ){
	bRouteCheck++;
	if ( bRouteCheck > 3 ) bRouteCheck = 0;
    }

    // ルートチェック
    switch ( bRouteCheck ){
    case 1:
	FAT_CheckRoute(work);
	break;
    case 2:
//	FAT_CheckFirstPos(work);
        FAT_CheckPassZone( work, HZX_Zone1(GM_PlayerAddress) );
	break;
    case 3:
	FAT_DbgCheckRoute(work);
	break;
    }
MAO_DbgDrawBox( &work->vecBezStart, 300.f, 300.f, 300.f, 0xff, 0xff, 0x00);
MAO_DbgDrawBox( &work->vecBezEnd, 300.f, 300.f, 300.f, 0xff, 0xff, 0xff);

#if 0
    { // 傾き
	FVECTOR vecBz, vecOld;

	FAT_CalcBezierIncMod( work, &VEC_TEST[0], &VEC_TEST[1], &VEC_TEST[2]);
    
	vecOld = VEC_TEST[0];
	for ( i = 0; i < FAT_BEZIER_RATE; i++){
	    FAT_BezierInc( &vecBz, &work->vecIncMod1, &work->vecIncMod2, (float)i / (float)FAT_BEZIER_RATE);
	    _sceVu0ScaleVector( &vecBz, &vecBz, 1200.f);
	    MAO_Bezier3InterpQuick( &vecPos, &mat, (float)i / (float)FAT_BEZIER_RATE );
	    _sceVu0AddVector( &vecBz, &vecPos, &vecBz);
	    vec[0] = vecPos;
	    vec[1] = vecBz;
	    NewLineView( &vec[0], 1, 0xff, 0x00, 0x00);
	}
	MENU_Printf("%.3f\n", FAT_BezierDist( &mat, 32) ); // 距離
	MENU_Printf("%.3f\n", FAT_BezierDist( &mat, 64) ); // 距離
	MENU_Printf("%.3f\n", FAT_BezierDist( &mat, 256) ); // 距離

    }
#endif
}
#endif

// 爆弾設置デバック
#if 0
static void FAT_DbgBombPut( Work* work)
{
    static int nDbgBombIndex = 0;
}
#endif

static void FAT_DbgBombThink(Work* work)
{
    int		i;
    static int  nInterval = 0;
    FAT_C4_DATA* 	pbombData;	// 爆弾情報構造体へのポインタ

    // 爆弾置きデバック
    if ((GV_PadData[ 0 ].right_dy > 223)){
	if ( nInterval == 0){
	    nInterval = 20;
	    work->nDbgIndex--;
	    if ( work->nDbgIndex < 0 ) work->nDbgIndex = work->nPutPosNum - 1;
	    // 新しい目標を設定
	    work->pCurPosData =  &work->datPoint[work->nDbgIndex]; // 設置場所取得
	    FAT_SetNewAim( work, HZX_Zone1( work->pCurPosData->zoneAddr), &work->pCurPosData->vecPut);
	    FAT_SetThink3( work, TH3_POINT_MOVE); 	// ピンポイント移動
	}
    }else if ((GV_PadData[ 0 ].right_dy < 32)){
	if ( nInterval == 0){
	    nInterval = 20;
	    work->nDbgIndex++;
	    if ( work->nDbgIndex >= work->nPutPosNum ) work->nDbgIndex = 0;
	    // 新しい目標を設定
	    work->pCurPosData =  &work->datPoint[work->nDbgIndex]; // 設置場所取得
	    FAT_SetNewAim( work, HZX_Zone1( work->pCurPosData->zoneAddr), &work->pCurPosData->vecPut);
	    FAT_SetThink3( work, TH3_POINT_MOVE); 	// ピンポイント移動
	}
    }
    
    // レーダーもやけし
    if ( (GV_PadData[ 0 ].press & PAD_AR) ){
	pbombData = work->bombData;
	for ( i = 0; i < MAX_BOMB_NUM; i++){
	    if ( ( pbombData + i )->pvRadar != NULL ){ // レーダー表示
		BOMB_Erase( ( pbombData + i )->pvRadar, BOMB_RADAR_FEED_OUT);
		( pbombData + i )->pvRadar = NULL;
	    }
	}
    }

    if (nInterval > 0) nInterval--;

    switch( work->think3 ){
    case TH3_READY:		// 準備
	Think3_DbgStand( work );
	break;
    case TH3_STAND:		// 待機
	Think3_DbgStand( work );
	break;
    case TH3_MOVE:		// 移動
	Think3_Move( work );
	break;
    case TH3_REVERSE:		// 反転
	Think3_Turn180( work );
	break;
    case TH3_POINT_MOVE:	// 指定点に移動
	Think3_DbgPointMove( work );
	break;
    case TH3_RUN_PUT:		// 移動中に爆弾セット
	Think3_RunningDbgPut( work );
	break;
    case TH3_STOP_PUT:		// 停止して爆弾セット
	Think3_StopDbgPut( work );
	break;	
    case TH3_TURN_PUT:		// 回転置き
	Think3_TurnDbgPut( work );
	break;
    }

    {
	FVECTOR vec;
	DEBUG_Locate( 330, 220, 0 );
	DEBUG_Printf( "INDEX = %3d \n", work->nDbgIndex );
	FAT_GetCurrentC4Put( work, &vec);
	MAO_DbgDrawBox( &vec, 300.f, 300.f, 300.f, 0xff, 0x00, 0x00);
	FAT_GetCurrentC4Pos( work, &vec);
	MAO_DbgDrawBox( &vec, 300.f, 300.f, 300.f, 0x00, 0x00, 0xff);

	MAO_DbgDrawTrap( work->pCurPosData->nTrapName, 0x00, 0xff, 0x00); // もやトラップ
    }
}


#ifdef DEBUG_MODE
static void FAT_Debug( Work* work)
{
    // デバックステータスダンプ
    FAT_DbgViewStatus( work ); 
    // ステータスデバック更新
    FAT_DbgStatusUpdate( work );
    // モーションデバック更新
    FAT_DbgMotionUpdate( work );
}
#else
static void FAT_Debug( Work* work){}
#endif
#endif
