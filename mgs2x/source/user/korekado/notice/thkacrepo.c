//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkacrepo.c
	異常報告モード

	2001/07/1 Y.Korekado
	$Id: thkacrepo.c,v 1.1.1.3 2002/11/19 11:44:19 Yoshizawa1 Exp $
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
	TH3_WATCH,TH3_WAIT,TH3_WATCH_ENEDAM,TH3_WAIT_ENEDAM,TH3_END,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_TURN,		/* 方向転換 */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
} ;

/*-----  --------------------------------------------*/
/*----- アクション --------------------------------------------*/
/*----- アクションパッド --------------------------------------------*/
static int	AccidentReportModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_TURN :
			AT_SetModeFromPad( act, ENE_ActTurn, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_RADIOCALL :
			AT_SetModeFromPad( act, ENE_ActSetRadio, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_RADIOCALL_BREAK :
			AT_SetModeFromPad( act, ENE_ActRadioBreak, EM_call_break_l, act->pad ) ;
		break ;
		case SP_RETURNRADIO :
			act->keep_mot = EM_call_team_l_3end ;
			act->keep_pad = act->pad ;
			act->time = MAX_VOL_TIME ;
			return 0 ;
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
#include	"notice_radio.c"

static	void	Think3_Watch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
//		COM_SetSpeak( EV_NOTICE_FIND_LV2, entk ) ; /* んっ */
	}

	if ( entk->count3 == COUNT_VMODE(4) ) {
//		GM_SeSetMode( SD_S_IDISP02, &entk->ctrl->mov, GM_SEMODE_NORMAL ) ;/* 「カチャッ」 */
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_WatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}
	if ( entk->count3 > COUNT_VMODE(60) ) {
		SetThinkRadioSet( entk, EV_ACCIDENT_DIRECT, ENE_NOTICE_ACCDNT_REPO ) ;
		entk->count3 = 0 ;
		{	/* 無線の途中に変更する予定*/
			NEWCORP	*corp ;
			
			/* 報告済みにする */
			if ( (corp = CP_GetCorp( entk->corp_id )) != NULL ) {
				SET_FLAG( corp->flag, CORPS_ST_REPORT ) ;
			}
		}
		COM_SetAccident( entk->corp_id, &entk->trgpoint.pos, entk->trgpoint.map,
		ACCIDENT_DELAY_TIME-300 , ENE_ACCIDENT_DIRECT ) ;

		return ;
	}

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_WatchEnedam( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_WAIT_ENEDAM ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_WatchWaitEnedam( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}
	if ( entk->count3 > COUNT_VMODE(60) ) {
		SetThinkRadioSet( entk, EV_ACCIDENT_DIRECT, ENE_NOTICE_ACCDNT_REPO ) ;
		entk->count3 = 0 ;
		/* 無線の途中に変更する予定*/
		COM_SetAccident( entk->dam_entk->uniq_id, &entk->trgpoint.pos, entk->trgpoint.map,
		ACCIDENT_DELAY_TIME-COUNT_VMODE(300) , ENE_ACCIDENT_DIRECT ) ;

		return ;
	}

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
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
void THK_AccidentReportMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_Watch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_WatchWait( entk ) ;
		break ;

	    case TH3_WATCH_ENEDAM :
			Think3_WatchEnedam( entk ) ;
		break ;
	    case TH3_WAIT_ENEDAM :
			Think3_WatchWaitEnedam( entk ) ;
		break ;

		/* radio.c */
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
	    case TH3_NO_RADIO :
			Think3_NoRadio( entk ) ;
		break ;

	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}
/*----- 調査モードスタート --------------------------------------------*/
void	THK_AccidentReportCorp( entk )
ENETHINK	*entk ;
{
	NEWCORP	*corp ;

	entk->think3 = TH3_WATCH ; 

	entk->act->CheckPad = AccidentReportModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->c_notice |= ENE_NOTICE_ACCDNT_REPO ;

	corp = CP_GetCorp( entk->corp_id ) ;
	entk->tmp_pos = corp->ctrl->mov ;
	SET_FLAG( corp->flag, CORPS_ST_WATCH ) ;
	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->tmp_pos, corp->ctrl->hzx_id ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = entk->trgpoint.pos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->sense.status = RADAR_COLOR_RED ;
}

void	THK_AccidentReportEnedam( entk )
ENETHINK	*entk ;
{
	entk->think3 = TH3_WATCH_ENEDAM ; 

	entk->act->CheckPad = AccidentReportModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->c_notice |= ENE_NOTICE_ACCDNT_REPO ;

	ENE_SetTrgpPoint( &(entk->trgpoint),&entk->dam_entk->ctrl->mov,entk->dam_entk->ctrl->hzx_id ) ;
	SET_FLAG( entk->dam_entk->iknow_flag, IKNOW_HELP_ME_REPO ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = entk->trgpoint.pos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->sense.status = RADAR_COLOR_RED ;
}

void	THK_AccidentReportModeStart( entk )
ENETHINK	*entk ;
{
	if ( entk->notice & ENE_NOTICE_CORP ) {
		UNSET_FLAG( entk->notice, ENE_NOTICE_CORP ) ;
		THK_AccidentReportCorp( entk ) ;
		return ;
	}
	THK_AccidentReportEnedam( entk ) ;
}
