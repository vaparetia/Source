//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkcorps.c
	死体モード
	
	1999/09/16 Y.Korekado
	$Id: thkcorps.c,v 1.1.1.3 2002/11/19 11:44:20 Yoshizawa1 Exp $
	
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
	TH3_NSIGHT_ON, TH3_WATCH, TH3_LOOKAROUND, TH3_RADIOCALL,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_LOOKAROUND_CHECK_ZONE, TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_ZONE_MOVE,TH3_DIRECT_MOVE,TH3_NEAR_MOVE,TH3_PINPOINT_MOVE,TH3_LOCKER_OPEN,

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
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_WATCH_CORP,	/* 死体見つめる */
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
} ;

/*----- アクション --------------------------------------------*/
static int	CorpsModeCheckPad( act )
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

		case SP_NSIGHT_ON :
			AT_SetModeFromPad( act, ENE_ActNSight_ON, EM_gogle_on, act->pad ) ;
		break ;
		case SP_WATCH_CORP :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_see_patient, act->pad ) ;
		break ;
		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ENE_ActLockerOpen, EM_cle_locker_open, act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
		break ;
	}
	return 1 ;
}

/*----- 補助 --------------------------------------------*/
#define USE_DIRECT_MOVE	(1) //move.c で DirectMove 使用
#include	"notice_radio.c"
#include	"move.c"

static int OtherEnemyCorpAction( ENETHINK *entk )
{
	ENETHINK	*other ;
	int dis ;

	other = COM_NearEnemyThkStatus( entk, THK_STATUS_CORP_WATCH, &dis ) ;
	if ( other == NULL ) return 0 ;
	if ( entk->corp_id != other->corp_id ) return 0 ;
	if ( dis > 2000 ) return 0 ;
	
	return 1 ;
}

static void MoveEndCondition( ENETHINK *entk )
{
	if ( OtherEnemyCorpAction( entk ) ) {
		if ( ENE_OtherEnemyRadioAction( entk ) ) {
			entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		} else {
			if ( ENE_PureAttacker( entk->act ) ) {
				SetThinkRadioSet( entk, EV_RAD_CORP_AT, ENE_NOTICE_CORP ) ;
			} else {
				SetThinkRadioSet( entk, EV_RAD_CORP, ENE_NOTICE_CORP ) ;
			}
		}
	} else {
		entk->think3 = TH3_LOOKAROUND ;
	}
	entk->count3 = 0 ;
}

/*----- 思考 --------------------------------------------*/
static	void	Think3_CorpsWatch( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
		/* 顔の上下 */
	}
	
	if ( entk->count3 == COUNT_VMODE(6) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		COM_SetSpeak( SD_V_MAKI27, entk ) ; /* んっ */
	}

	if ( entk->count3 == COUNT_VMODE(120) ) {
		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) { /* はずしていたら */
			entk->think3 = TH3_NSIGHT_ON ; 
		} else {
			SetThinkDirectMove( entk, MoveEndCondition, -1, TH3_DIRECT_MOVE ) ;
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_NSight_ON( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_ON ;
	}

	if ( entk->act->act_end ) {
		SetThinkDirectMove( entk, MoveEndCondition, -1, TH3_DIRECT_MOVE ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	SET_FLAG( entk->thk_status, THK_STATUS_CORP_WATCH ) ;

	if ( entk->count3 == 0 ) {
		NEWCORP	*cur ;
		if ( (cur = CP_GetCorp( entk->corp_id )) != NULL ) {
			cur->count = 0 ;	/* 腐らす */
		}
		entk->act->pad = SP_WATCH_CORP ;
	}

	if ( entk->act->act_end ) {
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_CORP ) ;
			entk->think3 = TH3_END ; 
		} else {
			if ( ENE_OtherEnemyRadioAction( entk ) ) {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			} else {
				if ( ENE_PureAttacker( entk->act ) ) {
					SetThinkRadioSet( entk, EV_RAD_CORP_AT, ENE_NOTICE_CORP ) ;
				} else {
					SetThinkRadioSet( entk, EV_RAD_CORP, ENE_NOTICE_CORP ) ;
				}
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_LookAroundCheckZone( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	entk->thk_status |= THK_STATUS_WATCH ;

	/* ルートのつながっている方向を見る 最大４回 */
	if ( entk->count3 == 0 ) {
		entk->act->aim_dir = entk->ctrl->rot.vy ;
	} else if ( !(entk->count3%COUNT_VMODE(60)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, (entk->count3/COUNT_VMODE(60))-1, entk->ctrl->hzx_id ) ;
		if ( near != 255 ) {
			entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
		}
	}

	if ( entk->count3 >= LV1_STRM_LENGTH ) {
//		if( COM_GetSearchLevel( ) > (DEF_SEARCH_LEVEL - LV1_STRM_LENGTH) ) {
		if( ENE_NoRadio( ) ) {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_CORP ) ;
			entk->think3 = TH3_END ; 
		} else {
			if ( ENE_OtherEnemyRadioAction( entk ) ) {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			} else {
				if ( ENE_PureAttacker( entk->act ) ) {
					SetThinkRadioSet( entk, EV_RAD_CORP_AT, ENE_NOTICE_CORP ) ;
				} else {
					SetThinkRadioSet( entk, EV_RAD_CORP, ENE_NOTICE_CORP ) ;
				}
			}
		}
		entk->count3 = 0 ;
		
		return ;
	}
	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->count3 ++ ;
}

static	void	Think3_End( entk )
ENETHINK	*entk ;
{
	entk->count3 ++ ;
	return ;
}

/*-----  --------------------------------------------*/
static void ResetTarget( ENETHINK *entk )
{
	NEWCORP	*cur ;

	cur = CP_GetCorp( entk->corp_id ) ;
	if ( cur != NULL ) {
		ENE_SetTrgpPoint( &(entk->trgpoint), &cur->ctrl->mov, cur->ctrl->hzx_id ) ;
	}
}

static void OtherActionCheck( ENETHINK *entk )
{
	if ( OtherEnemyCorpAction( entk ) ) {
		if ( ENE_OtherEnemyRadioAction( entk ) ) {
			entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		} else {
			if ( ENE_PureAttacker( entk->act ) ) {
				SetThinkRadioSet( entk, EV_RAD_CORP_AT, ENE_NOTICE_CORP ) ;
			} else {
				SetThinkRadioSet( entk, EV_RAD_CORP, ENE_NOTICE_CORP ) ;
			}
		}
		entk->count3 = 0 ;
	}
}
/*-----  --------------------------------------------*/
void THK_CorpsMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_CorpsWatch( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;

		//move.c
	    case TH3_ZONE_MOVE :
	    	ResetTarget( entk ) ;
			Think3_ZoneMove( entk ) ;
			OtherActionCheck( entk ) ;
		break ;
	    case TH3_DIRECT_MOVE :
	    	ResetTarget( entk ) ;
			Think3_MoveDirect( entk ) ;
		break ;
	    case TH3_NEAR_MOVE :
			Think3_NearMove( entk ) ;
		break ;
	    case TH3_PINPOINT_MOVE :
	    	Think3_PinpointMove( entk ) ;
		break ;
	    case TH3_LOCKER_OPEN :
			Think3_LockerOpen( entk ) ;
		break ;
		//move.c

	    case TH3_RADIOSET :
		    Think3_RadioSet( entk ) ;
		break ;
	    case TH3_RADIOCALL :
		    Think3_RadioCall( entk ) ;
		break ;
	    case TH3_RADIOBREAK :
		    Think3_RadioBreak( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
		    Think3_LookAroundCheckZone( entk ) ;
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

#if 1
/* 増員 */
if(	entk->act->pad == SP_RETURNRADIO ){
	ENETHINK *e ;

//	e = COM_GetEnemyFromOldUniqID( entk->corp_id ) ;
	e = COM_GetEnemyFromName( entk->corp_alive_name ) ;
	if ( e != NULL ) {
		SET_FLAG( e->receive, ENE_ORDER_RESURRECT ) ;
	}
}
#endif
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_CorpsModeStart( entk )
ENETHINK	*entk ;
{
	NEWCORP	*cur ;

	entk->act->CheckPad = CorpsModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_CORP ;
	cur = CP_GetCorp( entk->corp_id ) ;
	entk->tmp_pos = cur->ctrl->mov ;
	entk->corp_alive_name = cur->entk->alive_name ;
	cur->flag |= CORPS_ST_WATCH ;
	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->tmp_pos, cur->ctrl->hzx_id ) ;
	entk->think3 = TH3_WATCH ; 
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;

	entk->sense.status = RADAR_COLOR_RED ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}
