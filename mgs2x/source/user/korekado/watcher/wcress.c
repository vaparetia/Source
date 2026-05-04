//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wcress.c
	巡回兵 潜入モード
	
	1999/12/01 Y.Korekado
	$Id: wcress.c,v 1.1.1.3 2002/11/19 11:44:31 Yoshizawa1 Exp $
	
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

enum {
	TH2_READY
} ;
enum {
	TH3_INIT, TH3_WAIT
} ;
enum {
	SP_NONE,
	SP_RESURRECTION,
} ;

/*----- 低レベル思考モード --------------------------------------------*/
static	void	Think3_Init( entk )
ENETHINK	*entk ;
{
	FVECTOR	pos ;
	int		hzx_id ;
	
	if ( entk->count3 == 0 ) {
		ENE_RefreshParam( entk ) ;
		COM_ResurrectionPosition( &pos, &hzx_id ) ;

		/* ワープ */
#ifdef WARP_JIK
		/* ワープ */
//		ENE_WarpNearPos( entk->ctrl, GM_PlayerAddress, 2000/250 ) ;
#endif

printf("resurrect MAP[%x]\n",hzx_id ) ;
		HZX_FlashTrap( entk->ctrl->hzx_id, &entk->ctrl->evt ) ;
		GM_ResetControlPositionAndGroup( entk->ctrl, &pos, hzx_id ) ;/* 再発生場所へ移動 */
		DG_SetPos2( &entk->ctrl->mov, &entk->ctrl->rot ) ;
		GM_ActObject2( entk->act->body );
printf("RESURRECT MAP[%x]\n",entk->ctrl->hzx_id ) ;
		/* 固有設定 */

		/* 再発生状態へ */
		entk->act->pad = SP_RESURRECTION ; 
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Wait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->notice |= ENE_NOTICE_RES ;
	}

	entk->count3 ++ ;
}


/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Ready( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_INIT :
	    	Think3_Init( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_Wait( entk ) ;
		break ;
	}
}
/*----- 高レベル思考モード --------------------------------------------*/
void	ENE_WatcherResurrect( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_READY :
			Think2_Ready( entk ) ;
		break ;
	}
}

/*----- スタートモード、巡回兵潜入モード --------------------------------------------*/
static	int	WatcherRessPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_RESURRECTION :
			AT_SetMode( act, ENE_ActStandStill ) ;
			return 1 ;
		break ;
	}
	return 0 ;
}


void	ENE_WatcherResurrectionMode( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	entk->act->CheckPad = WatcherRessPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_RESURRECT ; 
	entk->think2 = TH2_READY ; 
	entk->think3 = TH3_INIT ; 
	entk->count3 = 0 ;
}
