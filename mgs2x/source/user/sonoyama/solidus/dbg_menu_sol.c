//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dbg_menu_sol.c
   ソリダス／デバッグメニュー

   2001/06/03	M.Sonoyama
   $Id: dbg_menu_sol.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE
/* デバッグメニュー */
#include	"debugmenu.h"

static	int		SOL_DebugMenuStatus ;

enum {
	SDMS_NONE			=		0x0000,
	SDMS_MOTIONVIEW		=		0x0001,
} ;

static char *dbgmenu_items[] = 
{ "ON", "OFF" } ;
static int  dbgmenu_values[] = 
{ SDMS_MOTIONVIEW, SDMS_NONE } ;
static	GM_DEBUG_MENU	dbgmenu1 = {
	NULL, /* next */
	"SOLIDUS", /* class */
	"MOTION VIEW", /* menu */
	dbgmenu_items,/*items*/
	dbgmenu_values, /* values */
	&SOL_DebugMenuStatus,
	SDMS_MOTIONVIEW,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX		actor ;
	Work			*work ;
} DBG_Work ;

static	void	ActDebugMenu( DBG_Work *dbgwork )
{
	int			status ;
	Work		*work ;

	status = SOL_DebugMenuStatus ;
	work = dbgwork->work ;

	if ( status & SDMS_MOTIONVIEW ) {
		MENU_Locate( 160, 240, 0 ) ;
		MENU_Color( 128, 128, 128, 64 ) ;
		MENU_Printf( "Sol Motion %d %d\n", work->motion1, MotionTime( work ) ) ;
	}
}

static	void	StartDebugMenu( Work *work )
{
	SOL_DebugMenuStatus = 0 ;
	
	GM_AddDebugMenu( &dbgmenu1 ) ;

	{
		DBG_Work		*dbgwork ;
		
		dbgwork = GV_NewActor( GV_ACTOR_AFTER2, sizeof( DBG_Work ) ) ;
		if ( dbgwork != NULL ) {
			GV_SetActor( &dbgwork->actor, ActDebugMenu, NULL ) ;
			GV_ActorEX( &dbgwork->actor ) ;
			GV_SetActorChild( work, dbgwork ) ;
			dbgwork->work = work ;
		}
	}
}

#endif

