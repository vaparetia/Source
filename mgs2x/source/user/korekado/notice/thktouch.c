//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thktouch.c
	タッチモード
	
	1999/09/02 Y.Korekado
	$Id: thktouch.c,v 1.1.1.3 2002/11/19 11:44:22 Yoshizawa1 Exp $
	
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
	TH3_DOKI,TH3_WATCH,TH3_LOOKAROUND,TH3_AKUBI,TH3_GOOFY,
	TH3_END
} ;
enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_AKUBI,		/* あくび */
	SP_TURN,		/* 方向転換 */
	SP_GOOFY_RETURN,		/* 呆けから復帰 */
} ;

/*----- アクション --------------------------------------------*/
static int	TouchModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_AKUBI :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_akubi, act->pad ) ;
		break ;
		case SP_TURN :
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActTurn ) ;
			return 1 ;
		break ;
		case SP_GOOFY_RETURN :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_dam_okaji_end, act->pad ) ;
		break ;
	}
	return 1 ;
}
/*----- 思考 --------------------------------------------*/
static	void	Think3_Doki( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 == COUNT_VMODE(30) ) {
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_TouchWatch( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;
	if ( entk->count3 == COUNT_VMODE(6) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
/* ストリーミングここで停止アクシデントにはならない暫定 */
COM_StopRadioNoAccident( entk ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
		if ( entk->act->act_end ) {
			entk->think3 = TH3_LOOKAROUND ; 
			entk->count3 = 0 ;
			return ;
		}
		entk->act->pad = SP_TURN ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == COUNT_VMODE(160) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	
	if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(120) && entk->count3 < COUNT_VMODE(180)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}

	if ( entk->count3 == COUNT_VMODE(240) ) {
		entk->c_notice &= ~ENE_NOTICE_TOUCH ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_Akubi( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_AKUBI ;
	}
	if ( entk->count3 == COUNT_VMODE(120) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(120) && entk->count3 < COUNT_VMODE(180)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}

	if ( entk->act->act_end ) {
		entk->c_notice &= ~ENE_NOTICE_TOUCH ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_Goofy( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_GOOFY_RETURN ;
	}
	if ( entk->count3 == COUNT_VMODE(120) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}
/*
	if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(120) && entk->count3 < COUNT_VMODE(180)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
*/
	if ( entk->act->act_end ) {
		entk->c_notice &= ~ENE_NOTICE_TOUCH ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_End( entk )
ENETHINK	*entk ;
{
	entk->count3 ++ ;
	return ;
}

/*-----  --------------------------------------------*/
static void TouchCheack( ENETHINK *entk )
{
	if ( entk->notice & ENE_NOTICE_TOUCH ) {
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_PlayerPosition, GM_PlayerHzxID) ;
		entk->think3 = TH3_WATCH ; 
		entk->tmp_time = 0 ;
		entk->count3 = 0 ;
	}
}

/*-----  --------------------------------------------*/
void THK_TouchMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_DOKI :
			Think3_Doki( entk ) ;
		break ;
	    case TH3_WATCH :
			Think3_TouchWatch( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		    TouchCheack( entk ) ;
		break ;
	    case TH3_AKUBI :
		    Think3_Akubi( entk ) ;
		    TouchCheack( entk ) ;
		break ;
	    case TH3_GOOFY :
		    Think3_Goofy( entk ) ;
		    TouchCheack( entk ) ;
		break ;
	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、タッチモード --------------------------------------------*/
void	THK_TouchModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = TouchModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_TOUCH ;
	if ( entk->mess_notice == MES_NOTICE_ZZZ ) {
		/* 居眠り目覚め */
		entk->mess_notice = MES_NOTICE_NONE ;
		entk->think3 = TH3_AKUBI ; 
	} else if ( entk->mess_notice == MES_NOTICE_GOOFY ) {
		entk->mess_notice = MES_NOTICE_NONE ;
		entk->think3 = TH3_GOOFY ; 
	} else {
		ENE_SetTrgpPoint( &(entk->trgpoint), &GM_PlayerPosition, GM_PlayerHzxID) ;
		entk->think3 = TH3_WATCH ; 
//		entk->think3 = TH3_DOKI ; 
	}
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;

	entk->sense.status = RADAR_COLOR_YELOW ;

/* ストリーミングここで停止アクシデントにはならない暫定 */
//COM_StopRadioNoAccident( entk ) ;
}
