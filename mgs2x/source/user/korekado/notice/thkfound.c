//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkfound.c
	不審物発見モード
	
	2000/06/12 Y.Korekado
	$Id: thkfound.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
	
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

#include "BP_TrophySystem.h"

enum {
	TH3_WATCH,TH3_NSIGHT_ON,TH3_LOOKAROUND,TH3_WAIT, TH3_ADULT,
	TH3_END,TH3_LOOK,TH3_STOPSIGN,TH3_STAND_ADULT,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_ZONE_MOVE,TH3_DIRECT_MOVE,TH3_NEAR_MOVE,TH3_PINPOINT_MOVE,TH3_LOCKER_OPEN,
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_SURPRISE_BOMB,	/* ボムでびっくり */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,	/* ストップサイン */
	SP_ADULT,		/* 大人の本鑑賞 */
	SP_STAND_ADULT,		/* 立って大人の本鑑賞 */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
} ;

/*-----  --------------------------------------------*/
static void ActAdultFound( ACTION *, int ) ;
static void ActAdultWatch( ACTION *, int ) ;
static void ActAdultDeep( ACTION *, int ) ;
/*----- アクション --------------------------------------------*/

static void ActAdultFound( act, time )
ACTION	*act ;
int		time ;
{
   //BP - this gets spammed every frame but so does the condition that sets this state.
   BP_TrophySystem_UnlockTrophy( kTRP_LureEnemyGirlieMagazine );

	if (act->pad != SP_STAND_ADULT) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( (time > COUNT_VMODE(152)) && (time < COUNT_VMODE(166)) ) {
		AT_SetActStSt( act, ACT_STST_ERO_GETCHU ) ;
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
	if (act->pad != SP_STAND_ADULT) {
		AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
	}

	if ( time == 0 ) {
		int mot ;
		
		mot = (act->pad == SP_STAND_ADULT) ? EM_look_erotica_stand_loop2 : EM_look_erotica_loop2 ;
		ENE_SetActionPBreak( act, 0, mot ,0,MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;

//		act->tmp_time = (60*4) + KR_RandS( 60*2 ) ;
		act->tmp_time = (COUNT_VMODE(60)*10) + KR_RandS( COUNT_VMODE(60)*4 ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
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
	if (act->pad != SP_STAND_ADULT) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}
	AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE ) ;

	if ( time == 0 ) {
		int mot ;
		
		mot = (act->pad == SP_STAND_ADULT) ? EM_look_erotica_stand_loop1 : EM_look_erotica_loop1 ;
		ENE_SetActionPBreak( act, 0, mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_TUBIRI ;/* つぶる */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !(act->CheckPad( act )) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 2 ;	/* モーション後１つ、続きあり */
		AT_SetMode( act, ActAdultWatch ) ;
		return ;
	}
}

/*----- アクションパッド --------------------------------------------*/
static int	FoundModeCheckPad( act )
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
			AT_SetModeFromPad( act, ActAdultFound, EM_look_erotica, act->pad ) ;
		break ;
		case SP_STAND_ADULT :
			AT_SetModeFromPad( act, ActAdultFound, EM_look_erotica_stand, act->pad ) ;
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

		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ENE_ActLockerOpen, EM_cle_locker_open, act->pad ) ;
		break ;
		case SP_PINPOINT :
			AT_SetModeFromPad( act, ENE_ActPinpointMove, ENE_WalkMotion(act), act->pad ) ;
		break ;
	}

	return 1 ;
}
/*----- 思考補助 --------------------------------------------*/
#define USE_DIRECT_MOVE	(1) //move.c で DirectMove 使用
#include	"notice_radio.c"
#include	"move.c"

static void MoveEndCondition( ENETHINK *entk )
{
	entk->think3 = TH3_LOOK ;
	entk->count3 = 0 ;
}

/*----- 思考 --------------------------------------------*/
static	void	Think3_FoundWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_FIND_LV2, entk ) ; /* んっ */
		KR_FindCameraCall( entk->act->body, &entk->trgpoint.pos, entk->ctrl->map ) ;
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

static	void	Think3_FoundWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}
	if ( entk->count3 > COUNT_VMODE(60) ) {
		SetThinkDirectMove( entk, MoveEndCondition, -1, TH3_DIRECT_MOVE ) ;
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
		SetThinkDirectMove( entk, MoveEndCondition, -1, TH3_DIRECT_MOVE ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	int StandCheck( ENETHINK *entk, FVECTOR *pos )
{
	if ( KR_FVecTrgDis2( &entk->ctrl->mov, pos ) > 500 ) {
		return 1 ;
	}

	if ( ENE_ABSf( entk->ctrl->levels[0] - pos->vy ) > 250.0f ) return 1 ;

	return 0 ;
}

static	void	Think3_Look( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 == COUNT_VMODE(60) ) {
		ef = GM_GetEneFind( entk->now_found_ef_id )  ;
		if ( ef != NULL && (ef->type & EF_TYPE_ADULT) ) {
			COM_SetSpeak( EV_NOTICE_ADULT, entk ) ; /* あっ */
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BP ) ;
			if ( StandCheck( entk, &ef->pos ) ) {
				entk->think3 = TH3_STAND_ADULT ; 
			} else {
				entk->think3 = TH3_ADULT ; 
			}
			entk->count3 = 0 ;
			return ;
		} else if ( ef != NULL && (ef->type & EF_TYPE_LV3) ) {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		} else {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
			if(	GM_AlertMode != ALERT_MODE_AVOID ){
				COM_SetSpeak( EV_WHAT_FIND_LV2, entk ) ; /* うーん */
			}
		}
	}

	if ( entk->count3 == COUNT_VMODE(180) ) {
		ef = GM_GetEneFind( entk->now_found_ef_id )  ;
		if ( ef != NULL && (ef->type & EF_TYPE_ADULT) ) {
			if ( StandCheck( entk, &ef->pos ) ) {
				entk->think3 = TH3_STAND_ADULT ; 
			} else {
				entk->think3 = TH3_ADULT ; 
			}
		} else if ( ef != NULL && (ef->type & EF_TYPE_LV3) ) {
			SetThinkRadioSet( entk, EV_RAD_FIND, ENE_NOTICE_FOUND ) ;
		} else {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_FOUND ) ;
			entk->think3 = TH3_END ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}


static void Think3_Adult( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;

	if ( !(entk->count3 > COUNT_VMODE(85) && entk->count3 < COUNT_VMODE(180)) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	ef = GM_GetEneFind( entk->now_found_ef_id )  ;
	if ( (ef == NULL) ||
		 (ef != NULL && (ef->type & EF_TYPE_MOVE)) ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_ADULT ;
/*
	if ( entk->act->act_end ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}
*/
	entk->count3 ++ ;
}

static void Think3_StandAdult( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;

	if ( !(entk->count3 > COUNT_VMODE(85) && entk->count3 < COUNT_VMODE(180)) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	ef = GM_GetEneFind( entk->now_found_ef_id )  ;
	if ( (ef == NULL) ||
		 (ef != NULL && (ef->type & EF_TYPE_MOVE)) ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_STAND_ADULT ;
/*
	if ( entk->act->act_end ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}
*/
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
void THK_FoundMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_FoundWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_FoundWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

		/* move.c */
	    case TH3_ZONE_MOVE :
			Think3_ZoneMove( entk ) ;
		break ;
	    case TH3_DIRECT_MOVE :
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

	    case TH3_LOOK :
		    Think3_Look( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;
	    case TH3_ADULT :
		    Think3_Adult( entk ) ;
		break ;
	    case TH3_STAND_ADULT :
		    Think3_StandAdult( entk ) ;
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
/*----- 不審物発見モードスタート --------------------------------------------*/
void	THK_FoundModeStart( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;

	/* 見に行くだけ */
	entk->think3 = TH3_WATCH ; 

	entk->act->CheckPad = FoundModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_FOUND ;
	entk->now_found_ef_id = entk->found_ef_id ;

printf("START FIOUNT  entk->now_found_ef_id[%d]\n",entk->now_found_ef_id )  ;
	ef = GM_GetEneFind( entk->now_found_ef_id )  ;
	ASSERT( ef!=NULL ) ;
//SET_FLAG(ef->type, EF_TYPE_ADULT ) ;
//SET_FLAG(ef->type, EF_TYPE_LV3 ) ;

	SET_FLAG( ef->type, EF_TYPE_FOUND ) ;
	ENE_SetTrgp( &(entk->trgpoint), &ef->pos, ef->zoneaddr ) ;

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = entk->trgpoint.pos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}

void	THK_FoundModeFastStart( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;
	ef = GM_GetEneFind( entk->found_ef_id )  ;
	ASSERT( ef!=NULL ) ;
	SET_FLAG( ef->type, EF_TYPE_FOUND ) ;
}
