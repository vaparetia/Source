//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkaccid.c
	異常事態モード
	
	2000/09/23 Y.Korekado
	$Id: thkaccid.c,v 1.1.1.3 2002/11/19 11:44:19 Yoshizawa1 Exp $
	
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


extern int BP_FRAMES_PER_SEC();

#define ACCID_DIRECT_MOVE	(1)

enum {
	TH3_WATCH,TH3_NSIGHT_ON,TH3_LOOKAROUND,TH3_WAIT,TH3_END,
	TH3_STOPSIGN, TH3_ACCIDENT_FREE,TH3_MUDAASI,TH3_NO_PLOBLEM,
	TH3_ZONE_MOVE,TH3_DIRECT_MOVE,TH3_NEAR_MOVE, TH3_PINPOINT_MOVE, TH3_LOCKER_OPEN,	// move.c
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR,	//radio.c
	TH3_NO_RADIO,
} ;
enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_SURPRISE_BOMB,	/* ボムでびっくり */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,	/* ストップサイン */
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_RADIOCALL,
	SP_RETURNRADIO,
	SP_RADIOCALL_BREAK,
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
} ;

/*----- アクション --------------------------------------------*/
static int	AccidentModeCheckPad( act )
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
		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ENE_ActLockerOpen, EM_cle_locker_open, act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
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
/*-----  --------------------------------------------*/
#ifdef ACCID_DIRECT_MOVE
#define USE_DIRECT_MOVE	(1) //move.c で DirectMove 使用
#endif

#include	"notice_radio.c"
#include	"move.c"

static void MoveEndCondition( ENETHINK *entk )
{
	entk->think3 = TH3_LOOKAROUND ;
	entk->count3 = 0 ;
}
/*----- 思考 --------------------------------------------*/
static	void	Think3_AccidentWatch( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
//		COM_SetSpeak( EV_NOTICE_NOISE, entk ) ; /* んっ */
	}

	if ( entk->count3 == COUNT_VMODE(30*entk->id) ) {
		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) { /* はずしていたら */
			entk->think3 = TH3_NSIGHT_ON ; 
		} else {
			entk->think3 = TH3_WAIT ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_AccidentWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
#ifdef ACCID_DIRECT_MOVE
		SetThinkDirectMove( entk, MoveEndCondition, (entk->id*1000) + 500, TH3_DIRECT_MOVE ) ;
#else
		SetThinkMove( entk, MoveEndCondition, (entk->id*1000) + 500 ) ;
#endif
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_CAUT_STAND ;

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
#ifdef ACCID_DIRECT_MOVE
		SetThinkDirectMove( entk, MoveEndCondition, (entk->id*1000) + 500, TH3_DIRECT_MOVE ) ;
#else
		SetThinkMove( entk, MoveEndCondition, -1 ) ;
#endif
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
//			COM_SetSpeak( EV_WHAT_NOISE, entk ) ; /* うーん */
		}
	}

	/* ルートのつながっている方向を見る 最大４回 */
	if ( !(entk->count3%(COUNT_VMODE(60))) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, entk->count3/BP_FRAMES_PER_SEC(), entk->ctrl->hzx_id ) ;
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
		if ( COM_GetAccidentStatus( entk->accident_id ) & (ENE_ACCIDENT_POS|ENE_ACCIDENT_MECA_PLAYER) ) {
			SetThinkRadioSet( entk, EV_RAD_MUDAASI, ENE_NOTICE_ACCIDENT ) ;
		} else {
			SetThinkRadioSet( entk, EV_RAD_NO_ACCIDENT, ENE_NOTICE_ACCIDENT ) ;
		}

		COM_UnsetAccident( entk->accident_id ) ;
		entk->count3 = 0 ;
		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}
	entk->count3 ++ ;
	return ;
}

static	void	Think3_AccidentFree( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ACCIDENT ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_NoPloblem( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
		COM_SetSpeak( EV_NOTICE_ENEDAM, entk ) ; /* んっ */
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		COM_UnsetAccident( entk->accident_id ) ;
		if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
			entk->think3 = TH3_MUDAASI ; 
		} else {
			entk->think3 = TH3_ACCIDENT_FREE ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Mudaasi( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_RADIOCALL_ATTACKER ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( EV_RAD_MUDAASI, entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = COUNT_VMODE(60) ;
		}
	}

	if ( entk->count3 > entk->tmp_time ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ACCIDENT ) ;
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
#define NG_STATUS	(ACT_STATUS_DAM_DIR|ACT_STATUS_DAMAGE|ACT_STATUS_DOWN|ACT_STATUS_FAINT)
static void AccidentCheck( ENETHINK	*entk )
{
	/* アクシデントが解除されれば */
#if 0
	if ( !COM_AccidentToID( entk->accident_id ) ) {
printf( "????????????????????????????????\n");
		entk->think3 = TH3_ACCIDENT_FREE ; 
		entk->count3 = 0 ;
		return ;
	}
#endif

	/* 目的の兵の無事を視認すれば */
	if ( ENE_InSightEnemyFromID( entk, entk->accident_id, -1, NG_STATUS ) ) {
		ENETHINK	*trgentk ;
		trgentk = COM_GetEnemyFromUniqID( entk->accident_id ) ;
		ENE_SetTrgpPoint( &(entk->trgpoint), &trgentk->ctrl->mov, trgentk->ctrl->hzx_id ) ;
		entk->think3 = TH3_NO_PLOBLEM ; 
		entk->count3 = 0 ;
		return ;
	}
}

void THK_AccidentMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_AccidentWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_AccidentWatchWait( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

		//move.c
	    case TH3_ZONE_MOVE :
			Think3_ZoneMove( entk ) ;
			AccidentCheck( entk ) ;
		break ;
#ifdef ACCID_DIRECT_MOVE
	    case TH3_DIRECT_MOVE :
			Think3_MoveDirect( entk ) ;
			AccidentCheck( entk ) ;
		break ;
#endif
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

	    case TH3_ACCIDENT_FREE :
		    Think3_AccidentFree( entk ) ;
		break ;
	    case TH3_MUDAASI :
			Think3_Mudaasi( entk ) ;
		break ;
	    case TH3_NO_PLOBLEM :
			Think3_NoPloblem( entk ) ;
		break ;
	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}

#if 0	/* 増援無し */
	if ( entk->think3 == TH3_END ) {
		ENETHINK *e ;

		e = COM_GetEnemyFromUniqID( entk->accident_id ) ;
		if ( e != NULL ) {
			SET_FLAG( e->receive, ENE_ORDER_RESURRECT ) ;
		}
	}
#endif
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_AccidentModeStart( entk )
ENETHINK	*entk ;
{
	int hzx_id ;
	FVECTOR *pos ;

	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		ENE_RouteWarp( entk ) ;
	}

	entk->act->CheckPad = AccidentModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->c_notice |= ENE_NOTICE_ACCIDENT ;
	ENE_TraceClear( entk ) ;

	entk->accident_id = COM_Accident( ) ;
	ASSERT( entk->accident_id >= 0 ) ;
	pos = COM_GetAccidentPos( entk->accident_id ) ;
	hzx_id = COM_GetAccidentMap( entk->accident_id ) ;
	ASSERT( pos != NULL ) ;
	ENE_SetTrgpPoint( &(entk->trgpoint), pos, hzx_id ) ;

	entk->tmp_time = 0 ;
	entk->think3 = TH3_WATCH ;
	entk->count3 = 0 ;

	entk->act->move_s = MoveCautionRun ;
 
	entk->sense.status = RADAR_COLOR_RED ;
}
