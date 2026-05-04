//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gllcom.c
	ゴルルゴンコマンダー

	2002/04/04 Y.Korekado
	$Id: gllcom.c,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/define.h"
/*----------------------------------------------------------------*/
#include	"gll_def.h"
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			gll_num ;
	int			gameover_delay_count ;
} Work ;

/*-------------------------------------------------------------------*/
int		GLL_COM_STATUS ;
/*-------------------------------------------------------------------*/
CONTROL		*GLL_COM_DetectCtrl ;
#define GLL_GAME_OVER_DELAY	COUNT_VMODE(30)
/*-------------------------------------------------------------------*/
void	GLLCOM_Detect( CONTROL	*ctrl )
{
	if ( !(GLL_COM_STATUS & GLL_COM_DETECT) ) {
		GLL_COM_STATUS |= GLL_COM_DETECT ;
		GLL_COM_DetectCtrl = ctrl ;
	}
}

/*-------------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if ( GLL_COM_STATUS & GLL_COM_DETECT ) {
		if ( work->gameover_delay_count == 0 ) {
//printf("gll command detecttttttttttttttttttttttttttttttttt\n");
			GM_GameOverProcStart( NULL ) ;
		}
		
		if ( work->gameover_delay_count == GLL_GAME_OVER_DELAY ) {
			GM_GameOverProcEnd( NULL ) ;
		}
		work->gameover_delay_count ++ ;
	}
}

static	void	Die( work )
Work		*work ;
{
}
/*----------------------------------------------------------------*/
static	int	GetResources( Work *work, int name, int where )
{
	work->gll_num = 0 ;
	work->gameover_delay_count = 0 ;
	GLL_COM_STATUS = 0 ;

	return 0 ;
}

void		*NewGollCom( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor(  GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
