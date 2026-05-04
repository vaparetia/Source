/*
    prezpanic.c
    大統領ニキータイベント混乱処理
    2001/04/24 Masafumi Okuta
    $Id: prezpanic.c,v 1.1.1.3 2002/11/19 11:48:10 Yoshizawa1 Exp $
*/
// 今のゾーンからパニックゾーンの間にニキータが存在しないパニック逃げゾーンデータのインデックスを取得
static inline int PREZ_GetPanicPos( Work *work )
{
    int nPrezZone;
    int nPrezArea, nEscArea;
    NPCWORK*	npc;
    
    npc = &work->npc;

    nPrezZone = HZX_Zone1(npc->ctrl->addr);
    nPrezArea = PREZ_GetArea( &npc->ctrl->mov );
    nEscArea  = PREZ_GetArea( &work->vecNikita );

    if ( nPrezArea < 0 || nPrezArea >= PREZ_AREA_MAX || 
	 nEscArea < 0 || nEscArea >= PREZ_AREA_MAX || 
	 work->nEscArea[nPrezArea][nEscArea] == -1){ // 対エリアがない
	return (-1);
    }

    // 思考タイプ
    work->nReactThink = work->nReact[nPrezArea][nEscArea];
    return work->nEscArea[nPrezArea][nEscArea];
}
// パニックゾーン設定
static int PREZ_SetPanicZone( Work *work )
{
    int nRes;

    if ( (nRes = PREZ_GetPanicPos(work)) != -1 ){
	work->vecNextPos = work->vecPanicPos[nRes];
	work->nNextZone  = work->nPanicZone[nRes];
	PREZ_SetNextThink3( work, TH3_MOVE);	// 逃げる
	PREZ_SetThink3( work, TH3_FIND);	// びっくり
	return 1;
    }

    return 0;
}

// 反応取得
static int Get_ReactThink( Work* work)
{
    int nDir;
    CONTROL* ctrl;

    ctrl = &work->control;
	    
    switch ( work->nReactThink ){
    case REACT_BASE: 		// しゃがむorあしあげ
	if ( work->vecNikita.vy >= PREZ_NIKITA_DIV_HEAIGHT)	return (TH3_SCARED);	// おびえしゃがみ
	else							return (TH3_FOOTUP);	// 足あげ
	break;
    case REACT_BEHIND:		// はりつき	
	nDir = PREZ_GetWallDir( work);
	if ( nDir >= 0 ){
	    int nDirDiff;

	    // 方向を補正
	    if ( ctrl->rot.vy >= 512 && ctrl->rot.vy < 1536)		ctrl->turn.vy = 1024;
	    else if ( ctrl->rot.vy >= 1536 && ctrl->rot.vy < 2560)	ctrl->turn.vy = 2048;
	    else if ( ctrl->rot.vy >= 2560 && ctrl->rot.vy < 3584)	ctrl->turn.vy = 2048;
	    else							ctrl->turn.vy = 0;

	    nDirDiff = GV_DiffDirS( ctrl->turn.vy, nDir); // 壁の方向取得
	    if ( abs(nDirDiff) < 512 ){		// 前
		ctrl->turn.vy += 2048;
		ctrl->turn.vy &= 4095;
		return (TH3_BEHIND_B);	
	    }else if ( nDirDiff >= 512 && nDirDiff < 1536 ){	// 左
		return (TH3_BEHIND_L);	
	    }else if ( nDirDiff >= -1536 && nDirDiff < -512 ){	// 右
		return (TH3_BEHIND_R);	
	    }else {						// 後
		return (TH3_BEHIND_B);	
	    }
	}else{
	    if ( work->vecNikita.vy >= PREZ_NIKITA_DIV_HEAIGHT)	return (TH3_SCARED);	// おびえしゃがみ
	    else						return (TH3_FOOTUP);	// 足あげ
	}
	break;
    case REACT_SIDESTEP:	// 横飛び
	{
	    if ( ctrl->n_touches == 0 ){
		FVECTOR vec;
		FMATRIX mat;

		DG_SetPos2( &ctrl->mov, &ctrl->rot);
		DG_GetPos( &mat);

		_sceVu0CopyVector( &vec, &ctrl->vecs[0]);
		MAO_RelVec( &vec, &vec, &mat);
		if ( vec.vx < 0)      return (TH3_SIDESTEP_R);	
		else		      return (TH3_SIDESTEP_L);	
	    }else{
		if ( BP_PS2_rand() % 2 == 0) return (TH3_SIDESTEP_L);	
		else		      return (TH3_SIDESTEP_R);	
	    }
	}
	break;
    case REACT_TUMBLE:		// 転倒
	return (TH3_TUMBLE);
	break;
    case REACT_SPECIAL:		// スライディングorサマーソルト
	if ( work->vecNikita.vy >= PREZ_NIKITA_DIV_HEAIGHT)	return (TH3_SLIDE);	// スライディング
	else						return (TH3_SUMMER);	// サマーソルト
	break;
    }

    return (TH3_SCARED);
}

static void Think3_PanicStand( Work *work )
{
    if ( work->count3 == 0){
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    // ニキータ発見
    if ( FindNikita( work) ){
	return;
    }

    // タイムアウト
    if ( work->count3 > COUNT_VMODE(300) ){
	PREZ_SetTrgpNextZone( work );
	return;
    }

    work->count3++;
}

//--------------- 逃げる
#define VEASY_EVADE_NEAR 	(1500)
#define VEASY_EVADE_DIST 	(3000)
#define VEASY_EVADE_DIRDIFF	(512)
static int PanicMoveReactVEasy( Work* work )
{	
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( ( work->nNikDist < VEASY_EVADE_NEAR || ( work->nNikDist < VEASY_EVADE_DIST && 
	 HZX_Zone1(npc->ctrl->addr) == HZX_Zone1(work->navitrg.addr)) ) && 
	 GV_DiffDirAbs( npc->ctrl->rot.vy, MatToYRot(&DG_Chanls[0].eye) ) > 2048 -  VEASY_EVADE_DIRDIFF ){
	PREZ_SetThink3( work, TH3_SUMMER );
	return 1;
    }

    return 0;
}
static int PanicMoveReactEasy( Work* work )
{	
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( ( work->nNikDist < VEASY_EVADE_NEAR || ( work->nNikDist < VEASY_EVADE_DIST && 
	 HZX_Zone1(npc->ctrl->addr) == HZX_Zone1(work->navitrg.addr)) ) && 
	 GV_DiffDirAbs( npc->ctrl->rot.vy, MatToYRot(&DG_Chanls[0].eye) ) > 2048 -  VEASY_EVADE_DIRDIFF ){
	PREZ_SetThink3( work, TH3_SUMMER );
	return 1;
    }

    return 0;
}
static int PanicMoveReactNormal( Work* work )
{	
    NPCWORK*	npc;
	
    npc = &work->npc;
#if 0
    if ( work->count3 >= COUNT_VMODE(40)){
	PREZ_SetThink3( work, TH3_TUMBLE);	// 転倒
	return 1;
    }
#endif
    return 0;
}
static int PanicMoveReactHard( Work* work )
{	
    NPCWORK*	npc;
	
    npc = &work->npc;
#if 0
    if ( work->count3 >= COUNT_VMODE(40)){
	PREZ_SetThink3( work, TH3_TUMBLE);	// 転倒
	return 1;
    }
#endif

    return 0;
}
static int PanicMoveReactExtreme( Work* work )
{	

    return 0;
}

static int PREZ_EvadeCheck( Work* work)
{
    int nRes;

    nRes = 0;

    switch ( work->nDifficult ){
    case GM_LEVEL_VERYEASY:
	nRes = PanicMoveReactVEasy( work );
	break;
    case GM_LEVEL_EASY:
	nRes = PanicMoveReactEasy( work );
	break;
    case GM_LEVEL_NORMAL:
	nRes = PanicMoveReactNormal( work );
	break;
    case GM_LEVEL_HARD:
	nRes = PanicMoveReactHard( work );
	break;
    case GM_LEVEL_EXTREME:
    case GM_LEVEL_E_EXTREME:
	nRes = PanicMoveReactExtreme( work );
	break;
    }
    return (nRes);
}

#define PANIC_MOVE_DIST (800)
static void Think3_PanicMove( Work *work )
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
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    npc->action.pad = PAD_RUN;	// ポイント移動
				       
    if ( PREZ_EvadeCheck(work) ){ // 回避チェック
	return;
    }

#if 0    
    if ( PREZ_CheckNearZoneIn( npc->ctrl->addr, work->navitrg.addr) ){
	// 距離追跡
	if ( GM_NaviNear( npc->navi, &work->navitrg, PANIC_MOVE_DIST ) ){
	    PREZ_SetThink3FromNext( work, TH3_SCARED);	// 怖がり
	    return;
	}
    }else{
	// ゾーン追跡
	GM_ZoneNavi( npc->navi, &work->navitrg);
    }
#else
    if ( GM_Navi( npc->navi, &work->navitrg, 400 ) ){
	PREZ_SetThink3( work, Get_ReactThink( work) ); // 反応
	return;
    }
#endif

    // 移動方向指定 
    npc->action.dir = npc->navi->next_dir;

    work->count3++;
}

// ニキータ発見
#define FIND_DELAY (COUNT_VMODE(10)) // 発見->認識までのディレイ
static void Think3_PanicFind( Work *work )
{
    NPCWORK	*npc ;

    npc = &work->npc ;

    if ( work->count3 == FIND_DELAY ) {
	NPC_CallHeadMark( npc, HMK2_TYPE_RED_AT ); // ！
	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC01 + MAO_Rand(2));	
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    npc->action.pad = PAD_FIND;	// 発見
    

    if ( work->count3 >= COUNT_VMODE(30) && GV_DiffDirAbs( npc->ctrl->rot.vy, work->nNikDir) < 128 ) {
	PREZ_SetThink3FromNext( work, TH3_SCARED);	// おびえる
	return ;
    }

    // 移動方向指定 
    npc->action.dir = work->nNikDir;

    work->count3++;
}
// 怖がり
static void Think3_PanicScared( Work *work )
{
    NPCWORK*	npc;
    NPCACT*	act;

    npc = &work->npc;
    act = &npc->action ;

    // 初期設定
    if ( work->count3 == 0 ) {
	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC03 + MAO_Rand(3));	
	npc->action.pad = PAD_SCARED; // 怖がる
	work->bMotEnd = 0;
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    // 時間
    if ( work->count3 > PREZ_SCARED_TIME ) {
	work->bMotEnd = 1;
    }

    if ( act->current_mot == PREZ_MOT_SQUAT_END || act->current_mot == PREZ_MOT_STAND ){  // 振り向き
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

	// アジャスト
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_MUNE );
	GM_AdjustRotBody( &work->body, &work->vecAdjust, HUMAN21_ATAMA );
    }

    // 終了
    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND);	// 立つ
	return ;
    }

    work->count3++;
}
// 転倒
static void Think3_PanicTumble( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	npc->action.pad = PAD_TUMBLE;
	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC04);	
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );
	return ;
    }

    work->count3++;
}
// スライディング
#define EVADE_Y_JUDGE	(1000.f)
static void Think3_PanicSlide( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	PREZ_Say( work, SE_PRESUP04, GM_SEMODE_BOMB); // 気合い
	npc->action.pad = PAD_SLIDE;	// スライディング
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	PREZ_SetThink3( work, TH3_SCARED );	// 怖がり
	return ;
    }

    work->count3++;
}
// サマーソルト
static void Think3_PanicSummer( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	PREZ_Say( work, SE_PRESUP04, GM_SEMODE_BOMB); // 気合い
	npc->action.pad = PAD_SUMMER;	// 回避下
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 足あげ
static void Think3_PanicFootUp( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	PREZ_Say( work, SE_PRESUP04, GM_SEMODE_BOMB); 	// 気合い
	npc->action.pad = PAD_FOOTUP; 			// 足あげ
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 背中はりつき回避
static void Think3_PanicEvadeBack( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	if ( BP_PS2_rand() % 2 == 0)	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC03);	
	else			PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC05);	
	npc->action.pad = PAD_BEHIND_B;	// 回避背中張りつき
	work->npc.ctrl->turn.vy = work->npc.ctrl->rot.vy + 2048;
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 左壁張りつき
static void Think3_PanicEvadeLeft( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	if ( BP_PS2_rand() % 2 == 0)	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC03);	
	else			PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC05);	
	npc->action.pad = PAD_BEHIND_L;	// 回避背中張りつき
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 右壁張りつき
static void Think3_PanicEvadeRight( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	if ( BP_PS2_rand() % 2 == 0)	PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC03);	
	else			PREZ_StreamRequestJustTime( work, PREZ_STRM_PANIC05);	
	npc->action.pad = PAD_BEHIND_R;	// 回避背中張りつき
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 横飛び左
static void Think3_PanicSideStepLeft( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	PREZ_Say( work, SE_PRESUP04, GM_SEMODE_BOMB); // 気合い
	npc->action.pad = PAD_SIDESTEP_L;	// 回避背中張りつき
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// 横飛び右
static void Think3_PanicSideStepRight( Work *work )
{
    NPCWORK*	npc;
	
    npc = &work->npc;

    if ( work->count3 == 0 ) {
	PREZ_Say( work, SE_PRESUP04, GM_SEMODE_BOMB); // 気合い
	npc->action.pad = PAD_SIDESTEP_R;	// 回避背中張りつき
	work->npc.action.radar_color = RADAR_COLOR_RED;	// レーダー赤
    }

    if ( npc->action.act_end ) {
	work->bFindNikita = 0;
	PREZ_SetThink3( work, TH3_STAND );	// 立ち
	return ;
    }

    work->count3++;
}
// Very Easy
// 避けまくり＆スペシャル回避
static void PanicReadyVEasy( Work* work )
{
    if ( PREZ_SetPanicZone(work) ){
	return;
    }else{
	PREZ_SetNextThink3( work, TH3_SUMMER);	// 回避
	PREZ_SetThink3( work, TH3_FIND);	// びっくり
	return;
    }
}
static void PanicReadyEasy( Work* work )
{
    if ( PREZ_SetPanicZone(work) ){
	return;
    }else{
	PREZ_SetNextThink3( work, TH3_SCARED);	// おびえる
	PREZ_SetThink3( work, TH3_FIND);	// 立つ    
	return ;
    }
}
static void PanicReadyNormal( Work* work )
{
    if ( PREZ_SetPanicZone(work) ){
	return;
    }else{
	PREZ_SetNextThink3( work, TH3_SCARED);	// おびえる
	PREZ_SetThink3( work, TH3_FIND);	// 立つ    
	return ;
    }
}
static void PanicReadyHard( Work* work )
{
    if ( PREZ_SetPanicZone(work) ){
	return;
    }else{
	PREZ_SetNextThink3( work, TH3_SCARED);	// おびえる
	PREZ_SetThink3( work, TH3_FIND);	// 立つ    
	return ;
    }
}
static void PanicReadyExtreme( Work* work )
{
    if ( PREZ_SetPanicZone(work) ){
	return;
    }else{
	PREZ_SetNextThink3( work, TH3_SCARED);	// おびえる
	PREZ_SetThink3( work, TH3_FIND);	// 立つ    
	return ;
    }
}

// パニック状態準備段階
static void Think3_PanicReady( Work *work )
{
    NPCWORK	*npc ;

    npc = &work->npc ;

    switch ( work->nDifficult ){
    case GM_LEVEL_VERYEASY:
	PanicReadyVEasy( work );
	break;
    case GM_LEVEL_EASY:
	PanicReadyEasy( work );
	break;
    case GM_LEVEL_NORMAL:
	PanicReadyNormal( work );
	break;
    case GM_LEVEL_HARD:
	PanicReadyHard( work );
	break;
    case GM_LEVEL_EXTREME:
    case GM_LEVEL_E_EXTREME:
	PanicReadyExtreme( work );
	break;
    default:
	PanicReadyExtreme( work );
	break;
    }
}



