/*
    prezdbg.c
    大統領ニキータイベントデバックメニュー
    2001/04/23 Masafumi Okuta
    $Id: prezdbg.c,v 1.1.1.3 2002/11/19 11:48:09 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE
// デバック情報ダンプ

#ifdef PSX2
static GM_DEBUG_MENU dbg_info = {
    class:	"PREZ",
    menu:	"INFO-DRAW",
    max:	2,
    items:( char *[] ){ "OFF", "STATUS" },
    values: ( int [] ){ PREZ_DRAW_OFF, PREZ_DRAW_TRG },
    target: 	&nDbgPrezInfoDraw,
    mask:	0x00000001,	// 必須
};
// デバック情報ダンプ
static GM_DEBUG_MENU dbg_level = {
    class:	"PREZ",
    menu:	"LEVEL",
    max:	5,
    items:( char *[] ){ "VERY-EASY", "EASY", "NORMAL", "HARD", "EXTREME" },
    values: ( int [] ){ PREZ_LEVEL_VERYEASY, PREZ_LEVEL_EASY, PREZ_LEVEL_NORMAL, PREZ_LEVEL_HARD, PREZ_LEVEL_EXTREME },
    target: 	&nDbgPrezLevelDraw,
    mask:	0x0000000f,	// 必須
};
static GM_DEBUG_MENU dbg_trg = {
    class:	"PREZ",
    menu:	"TRG",
    max:	2,
    items:( char *[] ){ "OFF", "TRG-OFFVIEW" },
    values: ( int [] ){ PREZ_DBGTRG_OFF, PREZ_DBGTRG_VIEW },
    target: 	&nDbgPrezTrgDraw,
    mask:	0x0000000f,	// 必須
};
static GM_DEBUG_MENU dbg_muteki = {
    class:	"PREZ",
    menu:	"MUTEKI",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){ PREZ_DBGMUTEKI_OFF, PREZ_DBGMUTEKI_ON },
    target: 	&nDbgPrezMutekiDraw,
    mask:	0x0000000f,	// 必須
};
#else

static char *debug_info_items[] = { "OFF", "STATUS" };
static int debug_info_values[] = { PREZ_DRAW_OFF, PREZ_DRAW_TRG };
static GM_DEBUG_MENU dbg_info = { 
	NULL,
	"PREZ",
	"INFO_DRAW",
	debug_info_items,
	debug_info_values,
	&nDbgPrezInfoDraw,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *debug_level_items[] = { "VERY-EASY", "EASY", "NORMAL", "HARD", "EXTREME" };
static int debug_level_values[] = { PREZ_LEVEL_VERYEASY, PREZ_LEVEL_EASY, PREZ_LEVEL_NORMAL, PREZ_LEVEL_HARD, PREZ_LEVEL_EXTREME };
static GM_DEBUG_MENU dbg_level = { 
	NULL,
	"PREZ",
	"LEVEL",
	debug_level_items,
	debug_level_values,
	&nDbgPrezLevelDraw,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	5,
	0,
	0
};

static char *debug_trg_items[] = { "OFF", "TRG-OFFVIEW" };
static int debug_trg_values[] = { PREZ_DBGTRG_OFF, PREZ_DBGTRG_VIEW };
static GM_DEBUG_MENU dbg_trg = { 
	NULL,
	"PREZ",
	"TRG",
	debug_trg_items,
	debug_trg_values,
	&nDbgPrezTrgDraw,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *debug_muteki_items[] = { "OFF", "ON" };
static int debug_muteki_values[] = { PREZ_DBGMUTEKI_OFF, PREZ_DBGMUTEKI_ON };
static GM_DEBUG_MENU dbg_muteki = { 
	NULL,
	"PREZ",
	"MUTEKI",
	debug_muteki_items,
	debug_muteki_values,
	&nDbgPrezMutekiDraw,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

#endif


//---------------------------------------------------------
static void PREZ_CallDbgDump(void){
    GM_AddDebugMenu( &dbg_info );	// ダンプデバック登録
}

// ターゲット描画
static void PREZ_DrawTarget(Work* work)
{
    NPCWORK* npc;

    npc = &work->npc;
    DEBUG_Locate( 40, 180, 0 );
    DEBUG_Printf( "KILL-CNT=%d\n", GM_KillCount );
    DEBUG_Printf( "TH1=%2d\n", work->think1 );
    DEBUG_Printf( "TH2=%2d\n", work->think2 );
    DEBUG_Printf( "TH3=%2d\n", work->think3 );
    DEBUG_Printf( "CT3=%2d\n", work->count3 );
    DEBUG_Printf( "FAINT%2d\n", work->npc.action.faint );
    DEBUG_Printf( "LIFE%2d\n", work->npc.action.life );
    DEBUG_Printf( "FIND=%2d\n", work->bFindNikita );
    DEBUG_Printf( "DIR-CAM:%5d\n", GV_DiffDirAbs( npc->ctrl->rot.vy, MatToYRot(&DG_Chanls[0].eye) ));
    if ( work->bNikita ){
	DEBUG_Printf( "NIK-POS : %5d %5d %5d\n", (int)work->vecNikita.vx, 
		      (int)work->vecNikita.vy, (int)work->vecNikita.vz);
	DEBUG_Printf( "NIK = DIR:%5d, DIST:%6d\n", work->nNikDir, work->nNikDist);
	DEBUG_Printf( "DIR-DIFF:%5d\n", GV_DiffDirAbs( npc->action.face_dir, work->nNikDir) );
    }
}

// 関数
static void PREZ_DbgDrawInfo(Work* work)
{
    NPCWORK* 	npc;

    npc = &work->npc;

    switch ( *dbg_info.target ){ // デバック
    case PREZ_DRAW_TRG:	// ターゲット描画
	PREZ_DrawTarget(work);
	break;
    }

}
//---------------------------------------------------------
static void PREZ_CallDbgLevel(void)
{
    GM_AddDebugMenu( &dbg_level );	// ダンプデバック登録
}
//---------------------------------------------------------
static void PREZ_CallDbgTrg(void)
{
    GM_AddDebugMenu( &dbg_trg );	// ダンプデバック登録
}
static void PREZ_DbgDrawTrgBox( Work* work )
{
    int i;
    NPCWORK* npc;

    npc = &work->npc;

    if ( *dbg_trg.target == PREZ_DBGTRG_VIEW ){
	NewTargetView( &work->deftrg, 32, 255, 32 ) ;
	for (i=0;i<CHILD_TARGET_NUM;i++){
	    NewTargetView( &work->def_child[i], 200, 34, 184 ) ;
	}
    }
}
static void PREZ_DbgSetTrg( Work* work )
{
    NPCWORK* npc;

    npc = &work->npc;
    switch ( *dbg_trg.target ) {
    case PREZ_DBGTRG_OFF:
	break;
    case PREZ_DBGTRG_VIEW:
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
    }
}
//------------------------------------------------------
// デバック
static void PREZ_CallDbgMuteki(void)
{
    GM_AddDebugMenu( &dbg_muteki );	// ダンプデバック登録
}

static void PREZ_DbgSetMuteki( Work* work )
{
    NPCWORK* npc;

    npc = &work->npc;
    switch ( *dbg_muteki.target ) {
    case PREZ_DBGMUTEKI_OFF:
	break;
    case PREZ_DBGMUTEKI_ON:
	work->npc.action.life = work->nMaxLife;
	break;
    }
}
#if 0
//---------------------------------------------------------
static void PREZ_DbgSound(Work* work)
{
    static int nStrm = 0;
    static int nSe = 0;
    static int nSeMode = GM_SEMODE_NORMAL;

    // ストリームデバック
    if ((GV_PadData[ 0 ].press & PAD_L)){
	nStrm--;
	if (nStrm < 0)
	    nStrm = work->nStrmDataNum - 1;
	PREZ_StreamRequest( work, nStrm);
    }
    if ((GV_PadData[ 0 ].press & PAD_R)){
	nStrm++;
	if (nStrm >= work->nStrmDataNum)
	    nStrm = 0;
	PREZ_StreamRequest( work, nStrm);
    }
    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_D)){
	nSe--;
	if (nSe < 0)
	    nSe = SE_MAX - 1;
	
	PREZ_Say( work, nSe, nSeMode);
    }
    if ((GV_PadData[ 0 ].press & PAD_U)){
	nSe++;
	if (nSe >= SE_MAX)
	    nSe = 0;
	PREZ_Say( work, nSe, nSeMode);
    }

    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_L1)){
	if ( nSeMode == GM_SEMODE_NORMAL ) nSeMode = GM_SEMODE_BOMB;
	else				   nSeMode = GM_SEMODE_NORMAL;
    }
    DEBUG_Locate( 340, 300, 0 );
    DEBUG_Printf( "STREAM = %2d \n", nStrm);
    DEBUG_Printf( "SE     = %2d \n", nSe);
    DEBUG_Printf( "SE     = %s \n", (nSeMode == GM_SEMODE_NORMAL)? "NORMAL" : "BOMB");
}

static void PREZ_DbgDrawAimZone( Work* work)
{
    FVECTOR 	vecCenter;
    HZX_ZON*	pzone;
    NPCWORK*	npc;

    npc = &work->npc;
    pzone = ENE_HZX_GetZone( HZX_Address( npc->ctrl->hzx_id, work->nNextZone, work->nNextZone ));

    if ( pzone == NULL ) return;

    vecCenter.vx = pzone->x;
    vecCenter.vy = pzone->y;
    vecCenter.vz = pzone->z;
    vecCenter.vw = 1.f;

    MAO_DbgDrawBox( &vecCenter, pzone->w, 500.f, pzone->h, 0xff, 0x00, 0x00);
}
#endif

#else
static void PREZ_DbgSetTrg( Work* work ){}
static void PREZ_DbgDrawInfo(Work* work){}
#endif

