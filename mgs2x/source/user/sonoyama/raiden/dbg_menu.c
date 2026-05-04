//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dbg_menu.c
   プレイヤー／デバッグメニュー

   2001/03/14	M.Sonoyama
   $Id: dbg_menu.c,v 1.1.1.3 2002/11/19 11:50:54 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE

#include	"debugmenu.h"

/*----------------------------------------------------------------*/

extern void *NewTrapView( CONTROL *, u_char, u_char, u_char, int * ) ;
extern void *NewBehindView( CONTROL *, u_char, u_char, u_char, int * ) ;
extern void *NewPlayerDebugView( Work *work ) ;
extern void	SigNearZone( int, int ) ;

/*----------------------------------------------------------------*/

static char *event_items[] = { "TRAP", "BEHIND", "HAZARD", "TARGET", "HZDCHK", "HZDCHK_R",
								 "BLOODFLOOR", "BLOODEVERY", "OFF" }; /*items*/
static int  event_values[] = { PDMS_TRAPVIEW, PDMS_BEHINDVIEW, PDMS_HAZARDVIEW, PDMS_TARGETVIEW, 
								 PDMS_HAZARDCHECK, PDMS_HAZARDCHECK_R, PDMS_BLOODFLOORCHK, 
								 PDMS_BLOODEVERY, PDMS_NONE } ;/*values*/
static GM_DEBUG_MENU event_view = {
	NULL, /* next */
	"PLAYER", /* class */
	"EVENT VIEW", /* menu */
	event_items,/*items*/
	event_values, /* values */
 	&PlayerDebugMenuStatus,/*target*/
	(PDMS_TRAPVIEW|PDMS_BEHINDVIEW|PDMS_HAZARDVIEW|PDMS_TARGETVIEW|PDMS_HAZARDCHECK|
	  PDMS_HAZARDCHECK_R|PDMS_BLOODFLOORCHK|PDMS_BLOODEVERY),/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	9, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *muteki_items[] =
	{ "MODE 1", "MODE 2", "OFF" };
static int  muteki_values[] =
	{ PDMS_MUTEKIMODE1, PDMS_MUTEKIMODE2, PDMS_NONE };
static GM_DEBUG_MENU muteki_mode = {
	NULL, /* next */
	"PLAYER", /* class */
	"MUTEKI MODE",
	muteki_items,/*items*/
	muteki_values, /* values */
	&PlayerDebugMenuStatus,
	PDMS_MUTEKIMODE1|PDMS_MUTEKIMODE2,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	3, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *zoneinfo_items[] =
	{ "ZONE", "ONLINEINFO", "FINDPOS", "OFF" };
static int  zoneinfo_values[] =
	{ PDMS_ZONEVIEW, PDMS_ONLINEINFO, PDMS_FINDPOSVIEW, PDMS_NONE };
static GM_DEBUG_MENU zoneinfo_view = {
	NULL, /* next */
	"PLAYER", /* class */
	"ZONE VIEW",
	zoneinfo_items,/*items*/
	zoneinfo_values, /* values */
	&PlayerDebugMenuStatus,
	PDMS_ZONEVIEW|PDMS_ONLINEINFO|PDMS_FINDPOSVIEW,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	4, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *display_items[] =
	{ "OFF", "ON" };
static int  display_values[] =
	{ PDMS_DISPLAY_OFF, PDMS_NONE };
static GM_DEBUG_MENU display_off = {
	NULL, /* next */
	"PLAYER", /* class */
	"DISPLAY",
	display_items,/*items*/
	display_values, /* values */
	&PlayerDebugMenuStatus,
	PDMS_DISPLAY_OFF,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *behind_items[] =
	{ "Default", "Type2", "Type3", "Type4", "Type5" };
static int  behind_values[] =
	{ 0, 1, 2, 3, 4 };
static GM_DEBUG_MENU behind_type = {
	NULL, /* next */
	"PLAYER", /* class */
	"BEHIND TYPE",
	behind_items,/*items*/
	behind_values, /* values */
	&PlayerDebugBehindType,
	0,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	5, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *stealth_items[] =
	{ "NORMAL", "DEEP" };
static int  stealth_values[] =
	{ 0, PDMS_DEEPSTEALTH };
static GM_DEBUG_MENU stealth_type = {
	NULL, /* next */
	"PLAYER", /* class */
	"STEALTH TYPE",
	stealth_items,/*items*/
	stealth_values, /* values */
	&PlayerDebugMenuStatus,
	PDMS_DEEPSTEALTH,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/*
static GM_DEBUG_MENU behind_view = {
 class:		"PLAYER",
 menu:		"BEHIND VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_BEHINDVIEW, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_BEHINDVIEW,
} ;

static GM_DEBUG_MENU hazard_view = {
 class:		"PLAYER",
 menu:		"HAZARD VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_HAZARDVIEW, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_HAZARDVIEW,
} ;

static GM_DEBUG_MENU target_view = {
 class:		"PLAYER",
 menu:		"TARGET VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_TARGETVIEW, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_TARGETVIEW,
} ;

static GM_DEBUG_MENU zone_view = {
 class:		"PLAYER",
 menu:		"ZONE VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_ZONEVIEW, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_ZONEVIEW,
} ;

static GM_DEBUG_MENU onlineinfo_view = {
 class:		"PLAYER",
 menu:		"ONLINEINFO VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_ONLINEINFO, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_ONLINEINFO,
} ;

static GM_DEBUG_MENU findpos_view = {
 class:		"PLAYER",
 menu:		"FINDPOS VIEW",
 max:		2,
 items:		( char *[] ){ "ON", "OFF" },
 values:	( int [] ){ PDMS_FINDPOSVIEW, PDMS_NONE },
 target: 	&PlayerDebugMenuStatus,
 mask:		PDMS_FINDPOSVIEW,
} ;
*/

/*----------------------------------------------------------------*/

static	void	StartPlayerDebugMenu( Work *work )
{
	//PlayerDebugMenuStatus = 0 ;
    work->trap_view = work->behind_view = 0 ;
	NewTrapView( &work->control, 255, 255, 0, &work->trap_view ) ;
	NewBehindView( &work->control, 0, 0, 255, &work->behind_view ) ;	
	
	GM_AddDebugMenu( &event_view ) ; 	
	GM_AddDebugMenu( &muteki_mode ) ;
	GM_AddDebugMenu( &zoneinfo_view ) ;
	GM_AddDebugMenu( &display_off ) ;
	GM_AddDebugMenu( &behind_type ) ;
	GM_AddDebugMenu( &stealth_type ) ;
}

static	void	ActPlayerDebugMenu( Work *work )
{
	int			status ;

	status = PlayerDebugMenuStatus ;
	/* トラップビュー */
	if ( status & PDMS_TRAPVIEW ) work->trap_view = 1 ;
	else						  work->trap_view = 0 ;
	/* ビハインドビュー */
	if ( status & PDMS_BEHINDVIEW ) work->behind_view = 1 ;
	else							work->behind_view = 0 ;
	/* ハザードビュー */
	if ( status & PDMS_HAZARDVIEW ) {
		{
			int			i ;

			for ( i = 0; i < HZX_N_MapLinks; i ++ ) {
				HZX_ViewHazard( &HZX_MapLink[ i ].segment ) ;
			}
		}
		if ( work->control.level_found & 1 ) {
			HZX_ViewFloor( work->control.level[ 0 ], 
						   ( int )work->control.level[ 0 ]->p4.h ) ;
			HZX_ViewSeNo( work->control.level[ 0 ], NULL ) ;
			if ( work->pad->press & PAD_L1 ) {
				DumpFloor( work->control.level[ 0 ] ) ;
			}
		}
		if ( work->control.n_touches ) {
			HZX_ViewSegment( work->control.segs[ 0 ] ) ;
			HZX_ViewSeNo( work->control.segs[ 0 ], NULL ) ;
			if ( work->pad->press & PAD_L1 ) {
				DumpSegment( work->control.segs[ 0 ] ) ;
			}
			if ( work->control.n_touches == 2 ) {	    
				HZX_ViewSegment( work->control.segs[ 1 ] ) ;
				HZX_ViewSeNo( work->control.segs[ 1 ], NULL ) ;
				if ( work->pad->press & PAD_L1 ) {
					DumpSegment( work->control.segs[ 1 ] ) ;
				}
			}
		}
		if ( MT_IsSeListRegist() == 0 ){
		    printf( "raiden/dbg_menu.c : Kabe SE Settei SHITE!!!\n" ) ;
		}
	}

	if ( status & PDMS_HAZARDCHECK ) {
		if ( work->control.level_found & 1 ) {
			if ( work->pad->status & PAD_L1 ) {
				HZX_FLR		*flr ;
				flr = work->control.level[ 0 ] ;
				MENU_Locate( 4, 128, 0 ) ;
				MENU_Color( 128, 128, 128, 0 ) ;
				MENU_Printf( "Floor\n" ) ;
				MENU_Printf( "%.0f %.0f %.0f : ", flr->p1.x, flr->p1.y,
					   flr->p1.z ) ;
				MENU_Printf( "%.0f %.0f %.0f\n", flr->p2.x, flr->p2.y,
					   flr->p2.z ) ;
				MENU_Printf( "%.0f %.0f %.0f : ", flr->p3.x, flr->p3.y,
					   flr->p3.z ) ;
				MENU_Printf( "%.0f %.0f %.0f\n", flr->p4.x, flr->p4.y,
					   flr->p4.z ) ;
			}
		}
		if ( work->control.n_touches ) {
			HZX_SEG		*seg ;

			if ( work->pad->status & PAD_L1 ) {
				MENU_Locate( 4, 196, 0 ) ;
				MENU_Color( 128, 128, 128, 0 ) ;
				seg = work->control.segs[ 0 ] ;
				MENU_Printf( "SEG1\n" ) ;
				MENU_Printf( "%.0f %.0f %.0f : ", seg->p1.x, seg->p1.y,
					   seg->p1.z ) ;
				MENU_Printf( "%.0f %.0f %.0f\n", seg->p2.x, seg->p2.y,
					   seg->p2.z ) ;
			}
			if ( work->control.n_touches == 2 ) {	    
				if ( work->pad->status & PAD_L1 ) {
					seg = work->control.segs[ 1 ] ;
					MENU_Printf( "SEG2\n" ) ;
					MENU_Printf( "%.0f %.0f %.0f : ", seg->p1.x, seg->p1.y,
						   seg->p1.z ) ;
					MENU_Printf( "%.0f %.0f %.0f\n", seg->p2.x, seg->p2.y,
						   seg->p2.z ) ;
				}
			}
		}
		if ( MT_IsSeListRegist() == 0 ){
		    printf( "raiden/dbg_menu.c : Kabe SE Settei SHITE!!!\n" ) ;
		}
	}

	/* 血のつく床チェック */
	if ( status & PDMS_BLOODFLOORCHK ) {
		if ( work->control.level_found & 1 ) {
			if ( work->control.flr_atrs[ 0 ] & 
				( HZX_FLOOR_NO_BLOOD | HZX_FLOOR_NO_OBJECT ) ) {
				HZX_ViewHazard( work->control.level[ 0 ] ) ;
			}
		}
	}

	/* ゾーンビュー */
	if ( status & PDMS_ZONEVIEW ) {
		HZX_ViewZone( work->control.hzx_id, work->control.addr ) ;
	}
	/* オンライン情報 */
	if ( status & PDMS_ONLINEINFO ) {
		SigNearZone( GM_PlayerAddress , 5 ) ;		
	}
	/* ターゲットビュー */
    if ( status & PDMS_TARGETVIEW ) {
		NewTargetView2( &( work->def ), 32, 232, 186 ) ;	
	}
	/* 発見位置ビュー */
	if ( status & PDMS_FINDPOSVIEW ) {
		PosBox( &GM_PlayerFindPos, 150.0F, NULL ) ;
	}
	/* 無敵モード */
	if ( !( status & PDMS_DISPLAY_OFF ) ) {
		MENU_Locate( 320, 200 * Y_ADJ, 0 ) ;
		MENU_SetColor( 200, 200, 200 ) ;	
		if ( status & PDMS_MUTEKIMODE1 ) MENU_Printf( "MUTEKI MODE 1\n" ) ;
		else if ( status & PDMS_MUTEKIMODE2 ) MENU_Printf( "MUTEKI MODE 2\n" ) ;
	}

	/* 血だしまくり */
	if ( status & PDMS_BLOODEVERY ) {
		extern void *NewBlood( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int mode, int white );
		FVECTOR	fvtemp={ 1000.0f * sinf( 112.5f/180.0f*PI ) * sinf( (float)(GV_Time%256)/128.0f * PI ),
						 1000.0f * cosf( 112.5f/180.0f*PI ),
						 1000.0f * sinf( 112.5f/180.0f*PI ) * cosf( (float)(GV_Time%256)/128.0f * PI ),
						 1.0f };
		FVECTOR	fvtemp2={ 1000.0f * sinf( 112.5f/180.0f*PI ) * sinf( (float)((GV_Time+128)%256)/128.0f * PI ),
						  1000.0f * cosf( 112.5f/180.0f*PI ),
						  1000.0f * sinf( 112.5f/180.0f*PI ) * cosf( (float)((GV_Time+128)%256)/128.0f * PI ),
						 1.0f };
		NewBlood( &work->body.objs->objs[ HUMAN21_KUBI ].world,
				 (FVECTOR *)work->body.objs->objs[ HUMAN21_KUBI ].world.m[3], &fvtemp, 0, 0 );
		NewBlood( &work->body.objs->objs[ HUMAN21_KUBI ].world,
				 (FVECTOR *)work->body.objs->objs[ HUMAN21_KUBI ].world.m[3], &fvtemp2, 0, 0 );
	}
}

#endif
