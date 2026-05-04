//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkradio.c
	無線連絡モード
	
	2001/08/23 Y.Korekado
	$Id: thkradio.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
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
	TH3_RADIOCALL, TH3_RADIOSET, TH3_RADIOBREAK, TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_END
} ;
enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
} ;

/*----- アクション --------------------------------------------*/
static int	RadioModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_RADIOCALL :
			AT_SetModeFromPad( act, ENE_ActSetRadio, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_RETURNRADIO :
			act->keep_mot = EM_call_team_l_3end ;
			act->keep_pad = act->pad ;
			act->time = MAX_VOL_TIME ;
			return 0 ;
		break ;
		case SP_RADIOCALL_BREAK :
			AT_SetModeFromPad( act, ENE_ActRadioBreak, EM_call_break_l, act->pad ) ;
		break ;
		case SP_RADIOCALL_ATTACKER :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_ak_call_team_l, act->pad ) ;
		break ;
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
	}
	return 1 ;
}

/*----- 思考 --------------------------------------------*/
static	void	Think3_RadioSet( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL ;
	}

	if ( entk->act->act_end == 3 ) {
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			entk->think3 = TH3_RADIOBREAK ; 
		} else {
			entk->think3 = TH3_RADIOCALL ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_RadioCall( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( EV_RAD_MUDAASI, entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV1_STRM_LENGTH ;
		}
		ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
	}

	if ( ENE_PureAttacker( entk->act ) ) {
		entk->act->pad = SP_RADIOCALL_ATTACKER ;

		if ( entk->count3 > entk->tmp_time ) {
			entk->act->pad = SP_RETURNRADIO ;
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_RADIO ) ;
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		}
	} else {
		if ( entk->count3 == entk->tmp_time ) {
			entk->act->pad = SP_RETURNRADIO ;
		}
		if ( entk->count3 < entk->tmp_time ) {
			if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
				COM_StopRadio( entk ) ;
				entk->think3 = TH3_RADIOBREAK ; 
				entk->count3 = 0 ;
				return ;
			}
		}

		if ( entk->act->act_end == 1 ) {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_RADIO ) ;
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_RadioBreak( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL_BREAK ;
	}

	if ( entk->count3 == COUNT_VMODE(115) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_RADIO_BREAK ) ;
	}

	if ( entk->act->act_end == 1 ) {
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		entk->act->move_s = MoveRun ;
		entk->count3 = 0 ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_MoveWaitArea( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			entk->think3 = TH3_MOVE_WAITAREA_DIRECT ;
			entk->count3 = 0 ;
		} else {
			entk->think3 = TH3_REPAIR ;
			entk->count3 = 0 ;
		}
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveWaitAreaDirect( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 250 ) < 0 ) {
		entk->think3 = TH3_REPAIR ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

/* 修理＆連絡 */
static	void	Think3_Repair( entk )
ENETHINK	*entk ;
{
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->act->pad = SP_UNREAL ;
	}

	if ( entk->count3 == 0 ) {
		UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;
		ENE_ClearPDamage( entk->act ) ;
	}
printf("repair radio count[%d] \n",entk->count3 ) ;

	if ( entk->count3 > COUNT_VMODE(120) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_RADIO ) ;
		CLEAR_FLAG( entk->iknow_flag ) ;
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
/*-----  --------------------------------------------*/
void THK_RadioMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RADIOSET :
		    Think3_RadioSet( entk ) ;
		break ;
	    case TH3_RADIOCALL :
		    Think3_RadioCall( entk ) ;
		break ;
	    case TH3_RADIOBREAK :
		    Think3_RadioBreak( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA :
			Think3_MoveWaitArea( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA_DIRECT :
			Think3_MoveWaitAreaDirect( entk ) ;
		break ;
	    case TH3_REPAIR :
			Think3_Repair( entk ) ;
		break ;

	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_RadioModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = RadioModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_RADIO ;
	entk->think3 = TH3_RADIOSET ; 
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}
