//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_vib.c
   プレイヤーのモーションにあわせて
   パッド振動
   デバッグ用として使用すること

   2000/09/06 M.Sonoyama
   $Id: pl_vib.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
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
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"

typedef	struct	{
	GV_ACT			actor ;
	int				marfile ;
	int				motion ;
	int				file ;
	int				phase ;
} Work ;

extern	void	*NewPadVibration2( int, int ) ;

/*----------------------------------------------------------------*/

/* カットオフ機能はつけていません */

static	void	Act( Work *work )
{
	int			marfile, motion ;

	if ( GM_PlayerWork == NULL ) return ;

	marfile = GM_PlayerWork->current_mar ;
	motion = GM_PlayerWork->motion1 ;

	switch( work->phase ) {
	case 0 :
		if ( motion != work->motion || work->marfile != marfile ) return ;
		NewPadVibration2( work->file, 0 ) ;
		work->phase = 1 ;
		break ;
	case 1 :
		if ( motion == work->motion && work->marfile == marfile ) return ;
		work->phase = 0 ;
	}
}

/*----------------------------------------------------------------*/

void	*NewSetPlayerVibration( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, NULL ) ;
		work->marfile = GCL_GetOptionValue( 'm', 0 ) ;
		work->motion = GCL_GetOptionValue( 'o', -1 ) ;
		work->file = GCL_GetOptionValue( 'v', 0 ) ;
		work->phase = 0 ;
	}
	return work ;
}
