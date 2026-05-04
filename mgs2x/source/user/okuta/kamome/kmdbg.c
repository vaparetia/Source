/*
    kmdebug.c
    カモメデバック
    2001/05/08 Masafumi Okuta
    $Id: kmdbg.c,v 1.1.1.3 2002/11/19 11:48:02 Yoshizawa1 Exp $
*/
#ifdef DEBUG_MODE
static KAMOME* 	pKamomeDraw = NULL;

//////////////////////////////////////////////////////////////////////////////////////////
//  				かもめマネージャ関連					//
//////////////////////////////////////////////////////////////////////////////////////////
static int nDbgNum   = 0;
static int nDbgRoute = 0;
static int nDbgSound = 0;
static int nDbgArea  = 0;



#ifdef PSX2
static GM_DEBUG_MENU dbg_number = {
    class:	"KAMOME",
    menu:	"NUMBER-DRAW",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgNum,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU dbg_route = {
    class:	"KAMOME",
    menu:	"ROUTE-DRAW",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgRoute,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU dbg_sound = {
    class:	"KAMOME",
    menu:	"SOUND",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgSound,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU dbg_area = {
    class:	"KAMOME",
    menu:	"AREA",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgArea,
    mask:	0x00000001,	// 必須
};

#else


static char *dbg_number_items[] = { "ON", "OFF" };
static int dbg_number_values[] = { 1, 0 };
static GM_DEBUG_MENU dbg_number = {
	NULL,
	"KAMOME",
	"NUMBER-DRAW",
	dbg_number_items,
	dbg_number_values,
	&nDbgNum,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *dbg_route_items[] = { "ON", "OFF" };
static int dbg_route_values[] = { 1, 0 };
static GM_DEBUG_MENU dbg_route = {
	NULL,
	"KAMOME",
	"ROUTE-DRAW",
	dbg_route_items,
	dbg_route_values,
	&nDbgRoute,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *dbg_sound_items[] = { "ON", "OFF" };
static int dbg_sound_values[] = { 1, 0 };
static GM_DEBUG_MENU dbg_sound = {
	NULL,
	"KAMOME",
	"SOUND",
	dbg_sound_items,
	dbg_sound_values,
	&nDbgSound,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *dbg_area_items[] = { "ON", "OFF" };
static int dbg_area_values[] = { 1, 0 };
static GM_DEBUG_MENU dbg_area = {
	NULL,
	"KAMOME",
	"AREA",
	dbg_area_items,
	dbg_area_values,
	&nDbgArea,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

#endif

static void KMM_CallDbgNumber(void){
    GM_AddDebugMenu( &dbg_number );	// 番号表示デバック登録
}
static void KMM_CallDbgRoute(void){
    GM_AddDebugMenu( &dbg_route );	// ルート表示デバック登録
}
static void KMM_CallDbgSound(void){
    GM_AddDebugMenu( &dbg_sound );	// サウンド表示デバック登録
}
static void KMM_CallDbgArea(void){
    GM_AddDebugMenu( &dbg_area );	// エリア表示デバック登録
}


// SEチェック
static void KMM_DbgCheckSe(KAMOME_MNG* kmmng)
{
    static int	nDbgSe 		= 0;
    static int	nDbgSeMode	= 0;

    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_D)){
	nDbgSe--;
	if (nDbgSe < 0)
	    nDbgSe = KMM_SE_MAX - 1;
	
	GM_SeSetMode( KAMOME_SE_TBL[nDbgSe], &GM_PlayerPosition, nDbgSeMode);	// 効果音コール 
    }
    if ((GV_PadData[ 0 ].press & PAD_U)){
	nDbgSe++;
	if (nDbgSe >= KMM_SE_MAX)
	    nDbgSe = 0;
	GM_SeSetMode( KAMOME_SE_TBL[nDbgSe], &GM_PlayerPosition, nDbgSeMode);	// 効果音コール 
    }

    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_L1)){
	if ( nDbgSeMode == GM_SEMODE_NORMAL ) nDbgSeMode = GM_SEMODE_BOMB;
	else				      nDbgSeMode = GM_SEMODE_NORMAL;
    }

    DEBUG_Locate( 340, 300, 0 );
    DEBUG_Printf( "SE      = %2d \n", nDbgSe);
    DEBUG_Printf( "SE-MODE = %s \n", (nDbgSeMode == GM_SEMODE_NORMAL)? "NORMAL" : "BOMB");

}


// 通し番号表示
static void KMM_DbgDrawNumber( KAMOME_MNG* kmmng )
{
    KAMOME* pKamome;
    int i = 0;
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	SIG_NumPrint( &pKamome->mov, i);
	i++;
    }

}
static void DbgDrawKamomeRoute( KAMOME* kamome )
{
}
// ルート表示
static void KMM_DbgDrawRoute( KAMOME_MNG* kmmng )
{
    extern void AN_Test_Eye2( FVECTOR*, int);
    DbgDrawKamomeRoute( pKamomeDraw );

    MAO_DbgDrawBox( &pKamomeDraw->vecRoute[pKamomeDraw->nRouteIndex], 300.f, 300.f, 300.f, 0, 255, 0 );
    MAO_DbgDrawBox( &pKamomeDraw->vecAimPos, 400.f, 400.f, 200.f, 255, 255, 255 );
    AN_Test_Eye2( &pKamomeDraw->vecLand, 1);

    if ( GV_PadData[0].press & PAD_AR){
	printf("pKamomeDraw === 0x%x\n", pKamomeDraw);
    }
}

// エリア表示
static void KMM_DbgDrawArea( KAMOME_MNG* kmmng)
{
    NewBoundingBoxView( &pKamomeDraw->vecMoveArea1, &pKamomeDraw->vecMoveArea2, 255, 0, 0 );
    MAO_DbgDrawBox( &pKamomeDraw->mov, 300.f, 300.f, 300.f, 0, 0, 255 );
}

// かもめデバック処理
static void KMM_DbgProcess(KAMOME_MNG* kmmng)
{
    int			nCurr, nEnd;
    int			bFind	    = 0;
    static int		nInterval   = 0;
    KAMOME* 	 	pKamome;

    if ( kmmng->pkmmTop == NULL ) return; // 空っぽ
    if ( *dbg_number.target ) KMM_DbgDrawNumber( kmmng); // 通し番号
    if ( *dbg_sound.target )  KMM_DbgCheckSe( kmmng);

    if ( !(*dbg_route.target) && !(*dbg_area.target) ) return;

    nCurr = 0;
    for ( pKamome = kmmng->pkmmTop; pKamome != NULL; pKamome = pKamome->pkmmNext ){
	nCurr++;
	if ( pKamome == pKamomeDraw ){ // 検索
	    bFind = 1;
	    break;
	}
    }
    nEnd = nCurr;

    if ( !bFind ){ // 先頭
	pKamomeDraw = kmmng->pkmmTop;
    }

    if (nInterval > 0) nInterval--;

    if ((GV_PadData[ 0 ].right_dy > 223)){
	if ( nInterval == 0){
	    nInterval = 20;
	    if ( pKamomeDraw->pkmmNext != NULL ){
		pKamomeDraw = pKamomeDraw->pkmmNext;
		nCurr++;
	    }else{
		pKamomeDraw = kmmng->pkmmTop;
		nCurr = 0;
	    }
	}else{

	}
    }
    if ((GV_PadData[ 0 ].right_dy < 32)){
	if ( nInterval == 0){
	    nInterval = 20;
	    if ( pKamomeDraw->pkmmPrev != NULL ){
		pKamomeDraw = pKamomeDraw->pkmmPrev;
		nCurr--;
	    }else{
		pKamomeDraw = kmmng->pkmmEnd;
		nCurr = nEnd;
	    }
	}
    }

    if ( pKamomeDraw == NULL ){
	return ;
    }
	
    if ( *dbg_route.target )  KMM_DbgDrawRoute( kmmng);  // SEチェック
    if ( *dbg_area.target )   KMM_DbgDrawArea( kmmng);
    DEBUG_Locate( 340, 240, 0 );
    DEBUG_Printf( "KAMOME-NUM = %d \n", nCurr);
    DEBUG_Printf( "0x%x \n", pKamomeDraw);


    {
	DEBUG_Locate( 340, 300, 0 );
	DEBUG_Printf( "TH1     = %d \n", pKamomeDraw->think.nThink1);
	DEBUG_Printf( "TH2     = %d \n", pKamomeDraw->think.nThink2);
	DEBUG_Printf( "TH3     = %d \n", pKamomeDraw->think.nThink3);
	DEBUG_Printf( "PREV-TH3= %d \n", pKamomeDraw->think.nLastThink3);
	DEBUG_Printf( "LAST-TH3= %d \n", pKamomeDraw->think.nPrevThink3);
	DEBUG_Printf( "CNTR3   = %d \n", pKamomeDraw->think.nCount3);
	DEBUG_Printf( "CBOX-CNTR = %d \n", kmmng->nCboxCntr);
	DEBUG_Printf( "CBOX-NUM  = %d \n", kmmng->nCboxNum);
    }
//    MAO_DbgDrawBox( &kmmng->vecNoise, 300.f, 300.f, 300.f, 0x00, 0xff, 0x00);
}

// かもめデバックワーク初期化
static void KMM_InitDebugWork(KAMOME_MNG* kmmng)
{
    KMM_CallDbgNumber();
    KMM_CallDbgSound();
    KMM_CallDbgRoute();
    KMM_CallDbgArea();
}
// かもめ処理時間ダンプ
static void KMM_DbgDumpExecTime(KAMOME_MNG* kmmng, int nTime)
{
    if ( nDbgNum != 0 || nDbgRoute != 0 || nDbgSound != 0 || nDbgArea != 0 ){
	DEBUG_Locate( 240, 10, 0 );
	DEBUG_Printf("KAMOME-EXECTIME = %.3f\n", (float)nTime / 60.f / 262.f );
	
	if ( pKamomeDraw != NULL){
	    DEBUG_Printf( "LOD-LEVEL  = %d \n", pKamomeDraw->nLod);
	    DEBUG_Printf( "KMM-ANGRY  = %d \n", kmmng->nAngryLimit);
	}
    }
}


// コントロールを切替え
static int KMM_DbgActControl( KAMOME_MNG* kmmng )
{
    CONTROL*	ctrl;
    KAMOME* 	kamome;
    ctrl = &kmmng->control ;
    if ( pKamomeDraw != NULL){ // デバック注目かもめに付随
	MEMMOT_DATA*	pmmtData;

	kamome = pKamomeDraw;//kmmng->pkmmTop;
	pmmtData = kamome->mmt_ctrl.pmmtData;
	ctrl->mov = kamome->mov;
	ctrl->rot = kamome->rot;

	return 1;
    }

    return 0;
}

typedef struct _TEST_LINE {
    GV_ACT_EX		actor;		// 動作用
    FVECTOR vec[2];
    u_char r,g,b;
}TestLine;

// 動作関数
static void TestLineAct( TestLine* work )
{
    NewLineView( &work->vec[0], 1, work->r, work->g, work->b);
}
static void TestLineDie( TestLine* work )
{
}

static int TestLineGetResources( TestLine*	work,
				 FVECTOR* 	pvec1, 
				 FVECTOR* 	pvec2, 
				 u_char 	r,
				 u_char 	g,
				 u_char 	b)
{
    _sceVu0CopyVector( &work->vec[0], pvec1);
    _sceVu0CopyVector( &work->vec[1], pvec2);
    work->r = r;
    work->g = g;
    work->b = b;

    return 0;
}

void* NewMaoTestLine( FVECTOR* 	pvec1, 
		      FVECTOR* 	pvec2, 
		      u_char 	r,
		      u_char 	g,
		      u_char 	b)
{
    TestLine*	work;
    OPERATOR();

    work = (TestLine*)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( TestLine ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), TestLineAct, TestLineDie ) ;
	GV_ActorEX( &work->actor );
	if ( TestLineGetResources( work, pvec1, pvec2, r, g, b ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }	
    return (void*)work ;
}
#endif
