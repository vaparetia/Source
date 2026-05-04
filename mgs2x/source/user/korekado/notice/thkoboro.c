//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkoboro.c
	ぼんやり（朧）モード
	
	1999/10/15 Y.Korekado
	$Id: thkoboro.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
	
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
	TH3_WAIT, TH3_STOPSIGN, TH3_LOOKDOWN,
	TH3_ZONE_MOVE,TH3_NEAR_MOVE, TH3_PINPOINT_MOVE, TH3_LOCKER_OPEN, /* for move.c */
} ;
enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_CLE_PEEP_D,
} ;

/*-----  --------------------------------------------*/
static void ActPeepD( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActPeepDuct( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	AT_SetActStatus( act, ACT_STATUS_GUNLIGHT_D ) ;

//	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	if ( time > COUNT_VMODE(120) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}
/*----- アクション --------------------------------------------*/
static int	IndistinctModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
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
		case SP_CLE_PEEP_D :
			if ( act->bodyp.type & ENE_TYPE_SHIELD ) {
				AT_SetModeFromPad( act, ActPeepD, EM_shl_nom_cle_seach_r2l, act->pad ) ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActPeepD, EM_htc_ak_nom_cle_seach_r2l, act->pad ) ;
			} else {
//				AT_SetModeFromPad( act, ActPeepD, EM_cle_look_d, act->pad ) ;
				AT_SetModeFromPad( act, ActPeepDuct, EM_squat_fire_intrude, act->pad ) ;
			}
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
		if ( (GM_PlayerStatus & PLAYER_GROUND) && 
			 (entk->pl_eyei.dis < 3000) ) {
			entk->think3 = TH3_LOOKDOWN ;
		} else {
			entk->think3 = TH3_LOOKAROUND ;
		}
	}

	entk->count3 = 0 ;
}

static	void	Think3_IndistinctWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_OBORO, entk ) ; /* んっ */
	}

	if ( entk->act->act_end ) {
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
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_IndistinctWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			COM_SetRadio( EV_OBORO_AVOID, entk ) ;	/* 誰だ？ */
		} else {
			COM_SetRadio( EV_OBORO_SNEAK, entk ) ;	/* 誰だ？ */
		}

		SetThinkMove( entk, MoveEndCondition, -1 ) ;
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
		SetThinkMove( entk, MoveEndCondition, -1 ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_IndistinctDirectMove( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 500 ) < 0 ) {
		if ( (GM_PlayerStatus & PLAYER_GROUND) && 
			 (entk->pl_eyei.dis < 3000) ) {
			entk->think3 = TH3_LOOKDOWN ;
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

static	void	Think3_LookDown( entk )
ENETHINK	*entk ;
{
	if ( entk->act->act_end ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_CLE_PEEP_D ;

	entk->count3 ++ ;
	return ;
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
		entk->c_notice &= ~ENE_NOTICE_INDISTINCT ;
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
		entk->c_notice &= ~ENE_NOTICE_INDISTINCT ;
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

static	void	Think3_End( entk )
ENETHINK	*entk ;
{
	entk->count3 ++ ;
	return ;
}

/*-----  --------------------------------------------*/
void THK_IndistinctMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_IndistinctWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_IndistinctWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

		//move.c
	    case TH3_ZONE_MOVE :
			Think3_ZoneMove( entk ) ;
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

	    case TH3_DIRECT_MOVE :
			Think3_IndistinctDirectMove( entk ) ;
		break ;
	    case TH3_LOOKDOWN :
		    Think3_LookDown( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;
	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_IndistinctModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = IndistinctModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_INDISTINCT ;
	if ( entk->notice & ENE_NOTICE_LIGHT ) {
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	} else {
		if ( (entk->act->bodyp.type & ENE_TYPE_USHOLD) && (GM_PlayerPosition.vy < -17100.0f) ) {
			/*船倉兵床下対策*/
			ENE_SetTrgpPoint( &(entk->trgpoint), &GM_PlayerPosition, GM_PlayerHzxID) ;
		} else {
			printf(" last_map = %x \n",entk->last_map ) ;

			ENE_SetTrgpPoint( &(entk->trgpoint), 
			&(entk->com->plpos_in_zone[PL_POS_CENTER]), entk->com->plmap_in_zone[PL_POS_CENTER] ) ;
		}
//		ENE_SetTrgpPoint( &(entk->trgpoint), &(entk->last_pos), entk->last_map ) ;
	}
//printf("pl pos[%f][%f][%f]\n",GM_PlayerPosition.vx,GM_PlayerPosition.vy,GM_PlayerPosition.vz ) ;
//printf("la pos[%f][%f][%f]\n",entk->trgpoint.pos.vx,entk->trgpoint.pos.vy,entk->trgpoint.pos.vz ) ;
	entk->think3 = TH3_WATCH ; 
	entk->tmp_time = 0 ;
	entk->tmp_count = 0 ;
	entk->count3 = 0 ;

//	entk->act->aim_pos = GM_PlayerPosition ;
	entk->act->aim_pos = GM_PlayerFindPos ;
	if ( HZX_ZoneDistanceCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) > RUN_DISTANCE ){
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionRun : MoveRun ;
	} else {
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;
	}
	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}
