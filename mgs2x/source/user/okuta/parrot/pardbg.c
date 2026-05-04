/*
    pardbg.c
    オウム関連デバック
    2001/05/12 Masafumi Okuta
    $Id: pardbg.c,v 1.1.1.3 2002/11/19 11:48:06 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE
enum{
PAR_SOUND_OFF,
PAR_SOUND_SE,
PAR_SOUND_STREAM,
};
// デバック情報ダンプ
static int nDbgParSound = 0;

#ifdef PSX2
static GM_DEBUG_MENU dbg_sound = {
    class:	"PARROT",
    menu:	"SOUND-CHECK",
    max:	3,
    items:( char *[] ){ "OFF", "SE", "STREAM" },
    values: ( int [] ){ PAR_SOUND_OFF, PAR_SOUND_SE, PAR_SOUND_STREAM },
    target: 	&nDbgParSound,
    mask:	0x00000003,	// 必須
};
#else
static char *debug_sound_items[] = { "OFF", "SE", "STREAM" };
static int debug_sound_values[] = { PAR_SOUND_OFF, PAR_SOUND_SE, PAR_SOUND_STREAM };
static GM_DEBUG_MENU dbg_sound = { 
	NULL,
	"PARROT",
	"SOUND-CHECK",
	debug_sound_items,
	debug_sound_values,
	&nDbgParSound,
	0x00000003,	 /* 必須 */
	NULL,
	0,
	0,
	3,
	0,
	0
};

#endif

static void PAR_CallDbgSound(void){
    GM_AddDebugMenu( &dbg_sound );	// ダンプデバック登録
}
// 内蔵SEチェック
static void PAR_DbgSE( PARROT* work )
{
    static int nSe     = 0;
    static int nSeMode = GM_SEMODE_NORMAL;
    static int nSeSay  = 0;

    // SEデバック
    if ((GV_PadData[ 0 ].press & PAD_D)){
	nSe--;
	if (nSe < 0)
	    nSe = SE_MAX - 1;
	
	PAR_Say( work, nSe, nSeMode);
    }
    if ((GV_PadData[ 0 ].press & PAD_U)){
	nSe++;
	if (nSe >= SE_MAX)
	    nSe = 0;
	PAR_Say( work, nSe, nSeMode);
    }

    // SEモード
    if ((GV_PadData[ 0 ].press & PAD_L1)){
	if ( nSeMode == GM_SEMODE_NORMAL ) nSeMode = GM_SEMODE_BOMB;
	else				   nSeMode = GM_SEMODE_NORMAL;
    }
    // 発生方式
    if ((GV_PadData[ 0 ].press & PAD_R1)){
	if ( nSeSay == 0 ) 	nSeSay = 1;
	else			nSeSay = 0;
    }
    DEBUG_Locate( 340, 200, 0 );
    DEBUG_Printf( "SE      = %2d \n", nSe);
    DEBUG_Printf( "SE-MODE = %s \n", (nSeMode == GM_SEMODE_NORMAL)? "NORMAL" : "BOMB");
    DEBUG_Printf( "SE-SAY  = %s \n", (nSeMode == 0)? "ON" : "OFF");
    DEBUG_Printf( "\n");
    DEBUG_Printf( "PL-SEE  = %d \n", work->nPlayerAttention);
    DEBUG_Printf( "ERO 	   = %d \n", work->nEroLearn);
    DEBUG_Printf( "EAMES   = %d \n", work->nEamesLearn);
    DEBUG_Printf( "SUBJECT = %d %d %d \n", work->nSubjectRate[0], work->nSubjectRate[1], work->nSubjectRate[2]);
}
// ストリームチェック
static void PAR_DbgStream( PARROT* work )
{
    static int nStrm     = 0;

    // ストリームデバック
    if ((GV_PadData[ 0 ].press & PAD_L)){
	nStrm--;
	if (nStrm < 0)
	    nStrm = work->nStrmDataNum - 1;
	PAR_StreamRequest( work, nStrm);
    }
    if ((GV_PadData[ 0 ].press & PAD_R)){
	nStrm++;
	if (nStrm >= work->nStrmDataNum)
	    nStrm = 0;
	PAR_StreamRequest( work, nStrm);
    }
    DEBUG_Locate( 340, 200, 0 );
    DEBUG_Printf( "STREAM = %2d \n", nStrm);
}

// サウンドデバック
static void PAR_DbgSoundCheck( PARROT* work )
{
    switch ( *dbg_sound.target ){
    case PAR_SOUND_SE:
	PAR_DbgSE( work );
	break;
    case PAR_SOUND_STREAM:
	PAR_DbgStream( work );
	break;
    }

}


enum{
PAR_STRM_NORMAL,
PAR_STRM_BOMB,
};
// デバック情報ダンプ
static int nDbgParStrmMode = 0;

#ifdef PSX2
static GM_DEBUG_MENU dbg_strm = {
    class:	"PARROT",
    menu:	"STRM-MODE",
    max:	2,
    items:( char *[] ){ "NORMAL", "BOMB" },
    values: ( int [] ){ PAR_STRM_NORMAL, PAR_STRM_BOMB },
    target: 	&nDbgParStrmMode,
    mask:	0x00000001,	// 必須
};
#else
static char *dbg_strm_items[] = { "NORMAL", "BOMB" };
static int dbg_strm_values[] = { PAR_STRM_NORMAL, PAR_STRM_BOMB };
static GM_DEBUG_MENU dbg_strm = { 
	NULL,
	"PARROT",
	"STRM-MODE",
	dbg_strm_items,
	dbg_strm_values,
	&nDbgParStrmMode,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

#endif

static void PAR_CallDbgStrmMode(void){
    GM_AddDebugMenu( &dbg_strm );	// ダンプデバック登録
}


static void PAR_DbgStrmMode( PARROT* work )
{
    if ( *dbg_strm.target == PAR_STRM_NORMAL)   work->nStrmMode	 = GM_SEMODE_NORMAL;
    else					work->nStrmMode	 = GM_SEMODE_BOMB;
}
#else
static void PAR_CallDbgSound(void){}
static void PAR_CallDbgStrmMode(void){}
static void PAR_DbgStrmMode( PARROT* work ){}
static void PAR_DbgSoundCheck( PARROT* work ){}
static void PAR_DbgSE( PARROT* work ){}
#endif
