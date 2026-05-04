//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   over.c
   ゲームオーバーロゴ

   2000/05/29 M.Sonoyama
   $Id: over.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#define	ACTOR_PRIO		(254)

typedef	struct	{
	GV_ACT		actor ;
	int			mode ;
	int			count ;
	int			which ;
} Work ;

static	void	Act( work )
Work			*work ;
{
	GV_PAD		*pad ;

	MENU_Locate( 256, 96, MENU_MODE_CENTER ) ;
	MENU_SetColor( 255, 255, 255 ) ;
	MENU_Printf( "GAME OVER\n" ) ;

	if ( work->count > 0 ) {
		if ( -- work->count > 0 ) return ;
		GM_GameOverClear() ;
	}

	if ( work->which == 0 ) {
		MENU_Locate( 256, 128, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 128, 128 ) ;
		MENU_Printf( "CONTINUE" ) ;
		MENU_Locate( 256, 144, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 255, 255 ) ;
		MENU_Printf( "EXIT" ) ;
	} else {
		MENU_Locate( 256, 128, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 255, 255 ) ;
		MENU_Printf( "CONTINUE" ) ;
		MENU_Locate( 256, 144, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 128, 128 ) ;
		MENU_Printf( "EXIT" ) ;
	}
	pad = GV_PadData ;
	if ( pad->press & PAD_U ) work->which = 0 ;
	else if ( pad->press & PAD_D ) work->which = 1 ;
	else if ( pad->press & ( PAD_A | PAD_STA ) ) {
		GM_GameOverRestart( work->which ) ;
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( work )
Work			*work ;
{

}

static	int		GetResources( work )
Work			*work ;
{
	work->count = 60 ;
	return 0 ;
}

void	*NewGameOverWin( mode )
int		mode ;
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->mode = mode ;
	}
	return work ;
}


