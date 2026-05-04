//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkinves.c
	調査モード
	
	2001/07/1 Y.Korekado
	$Id: thkinves.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
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
	TH3_WATCH,TH3_NSIGHT_ON,TH3_ZONETRACE,TH3_LOOKAROUND,TH3_WAIT,
	TH3_DIRECTTRACE, TH3_ADULT, TH3_END,TH3_LOOK,TH3_STOPSIGN,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_SURPRISE_BOMB,	/* ボムでびっくり */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,	/* ストップサイン */
	SP_ADULT,		/* 大人の本鑑賞 */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
} ;

/*-----  --------------------------------------------*/
static void ActAdultInvestigated( ACTION *, int ) ;
static void ActAdultWatch( ACTION *, int ) ;
static void ActAdultDeep( ACTION *, int ) ;
/*----- アクション --------------------------------------------*/

static void ActAdultInvestigated( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			return ;
		}
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 2 ;	/* モーション後１つ、続きあり */
		AT_SetMode( act, ActAdultWatch ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActAdultWatch( ACTION *act, int time )
{
	AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_look_erotica_loop2,0,MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
//		act->tmp_time = (60*4) + KR_RandS( 60*2 ) ;
		act->tmp_time = (COUNT_VMODE(60)*10) + KR_RandS( COUNT_VMODE(60)*4 ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			return ;
		}
	}
	
	if( time > act->tmp_time ) {
		AT_SetMode( act, ActAdultDeep ) ;
		return ;
	}
}

static void ActAdultDeep( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_look_erotica_loop1, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_TUBIRI ;/* つぶる */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			return ;
		}
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 2 ;	/* モーション後１つ、続きあり */
		AT_SetMode( act, ActAdultWatch ) ;
		return ;
	}
}


/*----- アクションパッド --------------------------------------------*/
static int	InvestigatedModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_SURPRISE_BOMB :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_surprised_bomb, act->pad ) ;
		break ;
		case SP_NSIGHT_ON :
			AT_SetModeFromPad( act, ENE_ActNSight_ON, EM_gogle_on, act->pad ) ;
		break ;
		case SP_TURN :
			AT_SetModeFromPad( act, ENE_ActTurn, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_CLE_STOP_SIGN :
			AT_SetModeFromPad( act, ENE_ActCleSign, EM_cle_signe_stop, act->pad ) ;
		break ;
		case SP_ADULT :
			AT_SetModeFromPad( act, ActAdultInvestigated, EM_look_erotica, act->pad ) ;
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

static	void	Think3_InvestigatedWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
//		COM_SetSpeak( EV_NOTICE_FIND_LV2, entk ) ; /* んっ */
	}

	if ( entk->count3 == 4 ) {
//		GM_SeSetMode( SD_S_IDISP02, &entk->ctrl->mov, GM_SEMODE_NORMAL ) ;/* 「カチャッ」 */
	}

	if ( entk->act->act_end ) {
		if ( GM_AlertMode == ALERT_MODE_AVOID && (entk->act->bodyp.type & ENE_TYPE_ATTACKER) ) {
			entk->think3 = TH3_STOPSIGN ; 
		} else if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) { /* はずしていたら */
			entk->think3 = TH3_NSIGHT_ON ; 
		} else {
			entk->think3 = TH3_WAIT ; 
		}
		entk->count3 = 0 ;

		return ;
	}

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_InvestigatedWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}
	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_StopSign( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_CLE_STOP_SIGN ;
	}

	if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_STOP_1, entk ) ;

	if ( entk->act->act_end ) {
		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) { /* はずしていたら */
			entk->think3 = TH3_NSIGHT_ON ; 
		} else {
			entk->think3 = TH3_WAIT ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_NSight_ON( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_ON ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_InvestigatedMove( entk )
ENETHINK	*entk ;
{
	int	dis ;
	
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X|ENE_STATUS2_AIM_FACE_Y ;
	}

	dis = KR_FVecTrgDis2( &entk->ctrl->mov, &entk->trgpoint.pos ) ;
//printf(" dis[%d]\n",dis ) ;
	if ( dis < 450 ) {
		
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		
		return ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if( ENE_InZone( &entk->trgpoint.pos, entk->trgpoint.addr ) ) {
			entk->think3 = TH3_DIRECTTRACE ;
		} else {
			entk->think3 = TH3_LOOKAROUND ;
		}
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectMove( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 450 ) < 0 ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
			COM_SetSpeak( EV_WHAT_FIND_LV2, entk ) ; /* うーん */
		}
	}

	/* ルートのつながっている方向を見る 最大４回 */
	if ( !(entk->count3%COUNT_VMODE(60)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, entk->count3/COUNT_VMODE(60), entk->ctrl->hzx_id ) ;
		if ( near != 255 ) {
			entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
		} else {
			if ( entk->count3 < COUNT_VMODE(180) ) entk->count3 = COUNT_VMODE(180) ;
		}
	}


	diff_dir = GV_DiffDirAbs( entk->ctrl->turn.vy, entk->act->aim_dir ) ;
	if ( diff_dir < 1024 ) {
		if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
	} else {
		if ( entk->count3 > COUNT_VMODE(180) ) {
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
			}
		}
	}

	if ( entk->count3 == COUNT_VMODE(240) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_FOUND ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
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
void THK_InvestigatedMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_InvestigatedWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_InvestigatedWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

	    case TH3_ZONETRACE :
			Think3_InvestigatedMove( entk ) ;
		break ;
	    case TH3_DIRECTTRACE :
			Think3_DirectMove( entk ) ;
		break ;

	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
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
void	THK_InvestigatedModeStart( entk )
ENETHINK	*entk ;
{
	/* 見に行くだけ */
	entk->think3 = TH3_WATCH ; 

	entk->act->CheckPad = InvestigatedModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_INVESTIGATED ;

	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->com->pos_investigated, entk->com->groupid_investigated ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = entk->trgpoint.pos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->sense.status = RADAR_COLOR_YELOW ;
}
