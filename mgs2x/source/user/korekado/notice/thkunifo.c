//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkunifo.c
	変装モード
	
	2001/3/27 Y.Korekado
	$Id: thkunifo.c,v 1.1.1.3 2002/11/19 11:44:22 Yoshizawa1 Exp $
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
	TH3_WATCH,TH3_NSIGHT_ON, TH3_DIRECT_MOVE, TH3_LOOKAROUND,TH3_END,
	TH3_WAIT, TH3_STOPSIGN, TH3_VOICE_WAIT, TH3_JIROJIRO, TH3_CLEAR,
	TH3_ATTACK,
	TH3_ZONE_MOVE,TH3_NEAR_MOVE, TH3_PINPOINT_MOVE, TH3_LOCKER_OPEN, /* for move.c */
} ;
enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_NONE_STAND,	/* 普通立ち */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_JIROJIRO,	/* じろじろ見る */
	SP_KERI,		/* 蹴り */
} ;

/*----- アクション --------------------------------------------*/
static	void ActKeri( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}
	if ( time == COUNT_VMODE(26) ) {
		static FVECTOR force = { 0.0F,-100.0F,100.0F } ;
		static FVECTOR size = { 1000.0F,1000.0F,1000.0F } ;

		ENE_SetOffenseTarget3( act, WP_PUNCH, HUMAN21_HIDARI_TSUMASAKI, &size, &force) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActJirojiro( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_check_disguise_start, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( act->c_motion_num[0] == EM_check_disguise_start ) {
		if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) { /*モーションの終了*/
			ENE_SetActionPBreak( act, 0, EM_check_disguise_idle, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		}
	} else if ( act->c_motion_num[0] == EM_check_disguise_idle ) {
		if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) { /*モーションの終了*/
			ENE_SetActionPBreak( act, 0, EM_check_disguise_end, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
			MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*2.0f ) ;
		}
	} else if ( act->c_motion_num[0] == EM_check_disguise_end ) {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->act_end = 1 ;
			AT_SetMode( act, ENE_ActStandStill ) ;
			return ;
		}
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/*----- アクション --------------------------------------------*/
static int	UniformModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_NONE_STAND :
			AT_SetModeFromPad( act, ENE_ActStandMotion, ENE_StandMotion(act), act->pad ) ;
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
		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ENE_ActLockerOpen, EM_cle_locker_open, act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
		break ;
		case SP_JIROJIRO :
			AT_SetModeFromPad( act, ActJirojiro, EM_check_disguise_start, act->pad ) ;
		break ;
		case SP_KERI :
			AT_SetModeFromPad( act, ActKeri, EM_nom_stomp, act->pad ) ;
		break ;
	}
	return 1 ;
}
/*----- 思考 --------------------------------------------*/
#include "move.c"

static void MoveEndCondition( ENETHINK *entk )
{
	if( entk->trgpoint.addr == entk->ctrl->addr ) {
		entk->think3 = TH3_DIRECT_MOVE ;
	} else {
		if ( entk->pl_eyei.sight_real == EYE_INFO_SIGHT_IN 
			&& entk->pl_eyei.dis < 2000 ) {
			if ( GM_PlayerStatus & (PLAYER_SQUAT|PLAYER_GROUND|PLAYER_ROLLING) 
				|| (GM_CheckPlayerStatusEX( I64(0), (PLAYER2_COMBO|PLAYER2_KNOCK_WALL) ))
				|| ((PL_GetPlayerWeapon() == WP_Aks)&&(GM_PlayerStatus&PLAYER_HOLD)) ) {
				entk->think3 = TH3_ATTACK ;
			} else {
				entk->think3 = TH3_JIROJIRO ;
			}
		} else {
			entk->think3 = TH3_LOOKAROUND ;
		}
	}

	entk->count3 = 0 ;
}

static	void	Think3_UniformWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_OBORO, entk ) ; /* んっ */
	}

	if ( entk->act->act_end == 1 ) {
//		KR_FindCameraCall( entk->act->body, &entk->trgpoint.pos, entk->ctrl->map ) ;
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
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &GM_PlayerPosition ) ;
//	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_UniformWatchWait( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		ENE_SetTrgpPoint( &(entk->trgpoint), 
		&(entk->com->plpos_in_zone[PL_POS_CENTER]), entk->com->plmap_in_zone[PL_POS_CENTER] ) ;
		entk->act->aim_pos = GM_PlayerPosition ;
		entk->tmp_time = COM_SetRadio( EV_UNIFORM1_SNEAK, entk ) ;	/* お前、ちょっと待て */
		if ( entk->tmp_time < 0 ) {
			entk->tmp_time = COUNT_VMODE(100) ;
		}
		entk->think3 = TH3_VOICE_WAIT ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = (GM_AlertMode == ALERT_MODE_AVOID)?SP_CAUT_STAND:SP_NONE_STAND ;
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &GM_PlayerPosition ) ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_UniformWatchVoiceWait( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( entk->count3 > entk->tmp_time ) {
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			if ( COM_StageKind() & ENE_STAGE_GPS ) {
				COM_SetRadio( EV_UNIFORM6_AVOID, entk ) ;	/* お前、何故ここにいる？ */
			} else if ( PL_GetPlayerWeapon() == WP_None ) {
				COM_SetRadio( EV_UNIFORM2_AVOID, entk ) ;	/* 武器はどうした？ */
			} else if ( PL_GetPlayerWeapon() != WP_Aks ) {
				COM_SetRadio( EV_UNIFORM3_AVOID, entk ) ;	/* なんだその武器は？ */
			} else if ( GM_PlayerStatus & PLAYER_BLOOD_DROP) {
				COM_SetRadio( EV_UNIFORM4_AVOID, entk ) ;	/* その血はどうした？ */
			} else {
				COM_SetRadio( EV_UNIFORM5_AVOID, entk ) ;	/* お前、何をしてる？ */
			}
		} else {
			if ( COM_StageKind() & ENE_STAGE_GPS ) {
				COM_SetRadio( EV_UNIFORM6_SNEAK, entk ) ;	/* お前、何故ここにいる？ */
			} else if ( PL_GetPlayerWeapon() == WP_None ) {
				COM_SetRadio( EV_UNIFORM2_SNEAK, entk ) ;	/* 武器はどうした？ */
			} else if ( PL_GetPlayerWeapon() != WP_Aks ) {
				COM_SetRadio( EV_UNIFORM3_SNEAK, entk ) ;	/* なんだその武器は？ */
			} else if ( GM_PlayerStatus & PLAYER_BLOOD_DROP) {
				COM_SetRadio( EV_UNIFORM4_SNEAK, entk ) ;	/* その血はどうした？ */
			} else {
				COM_SetRadio( EV_UNIFORM5_SNEAK, entk ) ;	/* お前、何をしてる？ */
			}
		}

		SetThinkMove( entk, MoveEndCondition, 1300 ) ;
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
		SetThinkMove( entk, MoveEndCondition, 1300 ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_UniformDirectMove( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 1300 ) < 0 ) {
		if ( entk->pl_eyei.sight_real == EYE_INFO_SIGHT_IN 
			&& entk->pl_eyei.dis < 2000 ) {
			if ( GM_PlayerStatus & (PLAYER_SQUAT|PLAYER_GROUND|PLAYER_ROLLING) 
				|| (GM_CheckPlayerStatusEX( I64(0), (PLAYER2_COMBO|PLAYER2_KNOCK_WALL) ))
				|| ((PL_GetPlayerWeapon() == WP_Aks)&&(GM_PlayerStatus&PLAYER_HOLD)) ) {
				entk->think3 = TH3_ATTACK ;
			} else {
				entk->think3 = TH3_JIROJIRO ;
			}
		} else {
			entk->think3 = TH3_LOOKAROUND ;
		}
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
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

#if 1
	if ( entk->count3 == COUNT_VMODE(120) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
			COM_SetSpeak( EV_WHAT_OBORO, entk ) ; /* 異常なしか */
		}
	}
	if ( entk->count3 == COUNT_VMODE(150) ) {
		entk->c_notice &= ~ENE_NOTICE_UNIFORM ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}
#else
	if ( entk->count3 == COUNT_VMODE(200) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
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
	}

	if ( entk->count3 == COUNT_VMODE(240) ) {
		entk->c_notice &= ~ENE_NOTICE_UNIFORM ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}
#endif

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}
	entk->count3 ++ ;
	return ;
}

static	void	Think3_JiroJiro( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_JIROJIRO ;
	}

	if ( entk->act->act_end == 1 ) {
		entk->think3 = TH3_CLEAR ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_Attack( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_KERI ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_CLEAR ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Clear( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
			COM_SetSpeak( EV_WHAT_OBORO, entk ) ; /* 異常なしか */
		}
	}

	if ( entk->count3 == COUNT_VMODE(120) ) {
		entk->c_notice &= ~ENE_NOTICE_UNIFORM ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_End( entk )
ENETHINK	*entk ;
{
	entk->count3 ++ ;
	return ;
}

/*-----  --------------------------------------------*/
static	void	PreZoneMove( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight_real >= EYE_INFO_SIGHT_BLURR ) {
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		entk->act->aim_pos = GM_PlayerPosition ;
	} else {
		if ( entk->status & ENE_STATUS_ENDLES_CHASE ) {
			ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
			entk->act->aim_pos = GM_PlayerPosition ;
		}
		entk->act->move_s = MoveRun ;
	}
	if ( (GM_PlayerStatus & PLAYER_DASH)  ) {
		entk->act->move_s = MoveRun ;
	}
	SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE ) ;
}
/*-----  --------------------------------------------*/
static void RunAwayCheck( ENETHINK	*entk )
{
	int dis ;

	dis = _FVecTrgDis( &entk->tmp_pos, &GM_PlayerPosition ) ;

//	if ( HZX_ZoneDistanceCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) > RUN_DISTANCE ){

	if ( dis > entk->sense.eye_s ) {
		entk->alert = DEF_EYE_DELAY ;
	}
}

/*-----  --------------------------------------------*/

void THK_UniformMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_UniformWatch( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_UniformWatchWait( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_VOICE_WAIT :
			Think3_UniformWatchVoiceWait( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
			RunAwayCheck( entk ) ;
		break ;

		//move.c
	    case TH3_ZONE_MOVE :
			PreZoneMove( entk ) ;
			Think3_ZoneMove( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_NEAR_MOVE :
			Think3_NearMove( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_PINPOINT_MOVE :
	    	Think3_PinpointMove( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_LOCKER_OPEN :
			Think3_LockerOpen( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
		//move.c

	    case TH3_DIRECT_MOVE :
			Think3_UniformDirectMove( entk ) ;
			RunAwayCheck( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;
	    case TH3_JIROJIRO :
		    Think3_JiroJiro( entk ) ;
		break ;
	    case TH3_ATTACK :
		    Think3_Attack( entk ) ;
		break ;
	    case TH3_CLEAR :
		    Think3_Clear( entk ) ;
		break ;
	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_UniformModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = UniformModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_UNIFORM ;

	ENE_SetTrgpPoint( &(entk->trgpoint), 
	&(entk->com->plpos_in_zone[PL_POS_CENTER]), entk->com->plmap_in_zone[PL_POS_CENTER] ) ;

	/* 逃亡基準地点 */
	entk->tmp_pos = entk->trgpoint.pos ;

//printf("pl pos[%f][%f][%f]\n",GM_PlayerPosition.vx,GM_PlayerPosition.vy,GM_PlayerPosition.vz ) ;
//printf("la pos[%f][%f][%f]\n",entk->trgpoint.pos.vx,entk->trgpoint.pos.vy,entk->trgpoint.pos.vz ) ;
	entk->think3 = TH3_WATCH ; 
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = GM_PlayerPosition ;
	if ( HZX_ZoneDistanceCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) > RUN_DISTANCE ){
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionRun : MoveRun ;
	} else {
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;
	}
	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}
