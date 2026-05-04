/********************************************************************************/
/*	har_dbg.c								*/
/*	ハリアデバッグメニュー							*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/
#include "debugmenu.h"	

int HAR_DbgViewSnkStat = 0;
int HAR_KacatcaMoveflg = 0;
int HAR_ViewKacRout = 0;
int HAR_DbgMode = 0;
int HAR_KRD = 0;
int HAR_Muteki = 0;
int HAR_TGT_VIEW = 0;
int HAR_RoutView = 0;
int HAR_ItemDbg = 0;

#if 0
static GM_DEBUG_MENU DbgInvisible = 
{
    class:	"HAR",
    menu:	"View Snake Status",
    max:	3,
    items:( char *[] ){ "0", "1", "2" },
    values: ( int [] ){  0,   1,   2  },
    target: 	&EMA_DbgInvincible,
    mask:	0x00000003,	// 必須
};
#endif


// yano
#ifdef PSX2
static GM_DEBUG_MENU HarMutekiSet = 
{
    class:	"Harrier",
    menu:	"Har Muteki Set",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_Muteki,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HAR_ITEM_DBG = 
{
    class:	"Harrier",
    menu:	"   ITEM DEBUG",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_ItemDbg,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HarRVIEW = 
{
    class:	"Harrier",
    menu:	"    ROUT VIEW",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_RoutView,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HarDbgSetMode = 
{
    class:	"Harrier",
    menu:	"    Mode Set",
    max:	8,
    items:( char *[] ){ "None", "Claster", "Mis Pod", "Burn", "Hover", "Amram", "Vulc", "STOP"},
    values: ( int [] ){  0, 1, 2, 3, 4, 5, 6, 99 },
    target: 	&HAR_DbgMode,
    mask:	0x000000FF,	// 必須
};

static GM_DEBUG_MENU HarTargView = 
{
    class:	"Harrier",
    menu:	"    TARG VIEW",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_TGT_VIEW,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HarKRD = 
{
    class:	"Harrier",
    menu:	"Kac Rout Control",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_KRD,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU ViewKckR = 
{
    class:	"Harrier",
    menu:	"    View Rout",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_ViewKacRout,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HarKckMove = 
{
    class:	"Harrier",
    menu:	"    Move Set",
    max:	2,
    items:( char *[] ){ "GO", "STOP" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_KacatcaMoveflg,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU HarDbgSnkStat = 
{
    class:	"Harrier",
    menu:	"View Snk Stat",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){  0,   1  },
    target: 	&HAR_DbgViewSnkStat,
    mask:	0x00000001,	// 必須
};
#else
static char *HarMutekiSet_items[] = { "OFF", "ON" };
static int HarMutekiSet_values[] = {  0,   1  };
static GM_DEBUG_MENU HarMutekiSet = { 
	NULL,
	"Harrier",
	"Har Muteki Set",
	HarMutekiSet_items,
	HarMutekiSet_values,
	&HAR_Muteki,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};
static char *HAR_ITEM_DBG_items[] = { "OFF", "ON" };
static int  HAR_ITEM_DBG_values[] = {  0,   1  };
static GM_DEBUG_MENU HAR_ITEM_DBG = { 
	NULL,
	"Harrier",
	"   ITEM DEBUG",
	HAR_ITEM_DBG_items,
	HAR_ITEM_DBG_values,
	&HAR_ItemDbg,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *HarRVIEW_items[] = { "OFF", "ON" };
static int  HarRVIEW_values[] = {  0,   1  };
static GM_DEBUG_MENU HarRVIEW = { 
	NULL,
	"Harrier",
	"   ROUT VIEW",
	HarRVIEW_items,
	HarRVIEW_values,
	&HAR_RoutView,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *HarDbgSetMode_items[] = { "None", "Claster", "Mis Pod", "Burn", "Hover", "Amram", "Vulc", "STOP"};
static int  HarDbgSetMode_values[] = {  0, 1, 2, 3, 4, 5, 6, 99 };
static GM_DEBUG_MENU HarDbgSetMode = { 
	NULL,
	"Harrier",
	"   Mode Set",
	HarDbgSetMode_items,
	HarDbgSetMode_values,
	&HAR_DbgMode,
	0x000000FF,	 /* 必須 */
	NULL,
	0,
	0,
	8,
	0,
	0
};

static char *HarTargView_items[] = { "OFF", "ON" };
static int  HarTargView_values[] = {  0,   1  };
static GM_DEBUG_MENU HarTargView = { 
	NULL,
	"Harrier",
	"   TRG VIEW",
	HarTargView_items,
	HarTargView_values,
	&HAR_TGT_VIEW,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};


static char *HarKRD_items[] = { "OFF", "ON" };
static int  HarKRD_values[] = {  0,   1  };
static GM_DEBUG_MENU HarKRD = { 
	NULL,
	"Harrier",
	"Kac Rout Control",
	HarKRD_items,
	HarKRD_values,
	&HAR_KRD,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *ViewKckR_items[] = { "OFF", "ON" };
static int  ViewKckR_values[] = {  0,   1  };
static GM_DEBUG_MENU ViewKckR = { 
	NULL,
	"Harrier",
	"    View Rout",
	ViewKckR_items,
	ViewKckR_values,
	&HAR_ViewKacRout,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};
static char *HarKckMove_items[] = { "GO", "STOP" };
static int  HarKckMove_values[] = {  0,   1  };
static GM_DEBUG_MENU HarKckMove = { 
	NULL,
	"Harrier",
	"    Move Set",
	HarKckMove_items,
	HarKckMove_values,
	&HAR_KacatcaMoveflg,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *HarDbgSnkStat_items[] = { "GO", "STOP" };
static int  HarDbgSnkStat_values[] = {  0,   1  };
static GM_DEBUG_MENU HarDbgSnkStat = { 
	NULL,
	"Harrier",
	"View Snk Stat",
	HarDbgSnkStat_items,
	HarDbgSnkStat_values,
	&HAR_DbgViewSnkStat,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};
#endif


void Har_InitDbgConfiguration()
{
    GM_AddDebugMenu( &HarMutekiSet ) ;
    GM_AddDebugMenu( &HarRVIEW ) ;
    GM_AddDebugMenu( &HarDbgSetMode );
    GM_AddDebugMenu( &HarTargView ) ;

    GM_AddDebugMenu( &HarKRD ) ;
    GM_AddDebugMenu( &ViewKckR ) ;
    GM_AddDebugMenu( &HarKckMove ) ;
    GM_AddDebugMenu( &HarDbgSnkStat ) ;
    GM_AddDebugMenu( &HAR_ITEM_DBG ) ;

}

char snk_mod[9][8]= {
"IDEL","M4FIRE","M4END","GLFIRE","DAM","M4IDEL","ITEM","DAMING","ENABLE"
};

void Kas_DbgDispSnkStat( Kas_Work *work )
{
    if ( HAR_DbgViewSnkStat )
    {
	MENU_Locate( 10, 420, 0 ) ;
	MENU_SetColor( 200, 0, 0 ) ;
	MENU_Printf( "SNK M: %s   S: %s   I: %d   S: %d  ", snk_mod[work->snake_motion], snk_mod[work->set_snake],
		     work->snake_ikari,work->m4time);
    }
    if ( HAR_KacatcaMoveflg ){
	work->ks_speed = 0.001f;
    }
}
