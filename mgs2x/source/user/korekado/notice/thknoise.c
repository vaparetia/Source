//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thknoise.c
	物音モード
	
	1999/09/02 Y.Korekado
	$Id: thknoise.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
	
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
	TH3_WATCH,TH3_NSIGHT_ON,TH3_WAIT,TH3_END,TH3_STOPSIGN,
	TH3_LOOKAROUND, TH3_WATCH_AGAIN, TH3_WAIT_CAUTION, TH3_RETURN_SIGN, TH3_ZONE_MOVE,
	TH3_NEAR_MOVE, TH3_PINPOINT_MOVE, TH3_LOCKER_OPEN, TH3_CLE_WATCH
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
} ;

/*----- アクション --------------------------------------------*/
/*----- アクション --------------------------------------------*/
static int	NoiseModeCheckPad( act )
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
	}

	return 1 ;
}
/*----- 思考 --------------------------------------------*/
#include "move.c"

static int LockerZone( R_INTRPT *r_intrpt )
{
	HZX_ZON	*zone ;
	u_char		*nears ;
	int i ;

//開かないロッカーでも調べに行く	if ( r_intrpt->zone1 == r_intrpt->zone2 ) return 0 ;

	zone = HZX_GetZoneFromAdd( r_intrpt->zone1 ) ;
	nears = zone->nears ;
	for ( i=0 ; i<6 ; i++ ) {
		if ( *(nears++) == HZX_NO_ZONE ) break ;
	}
printf("LockerZone zone1 near num[%d]\n",i) ;
	if ( i == 1 ) return 1 ;

	zone = HZX_GetZoneFromAdd( r_intrpt->zone2 ) ;
	nears = zone->nears ;
	for ( i=0 ; i<6 ; i++ ) {
		if ( *(nears++) == HZX_NO_ZONE ) break ;
	}
printf("LockerZone zone2 near num[%d]\n",i) ;
	if ( i == 1 ) return 2 ;

	return 0 ;
}

static void MoveEndCondition( ENETHINK *entk )
{
#if 1
	if ( entk->iknow_flag & IKNOW_CLEARING ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;
		entk->c_notice &= ~ENE_NOTICE_NOISE ;
		entk->think3 = TH3_END ; 
	} else {
		entk->think3 = TH3_LOOKAROUND ;
	}
#else
	entk->think3 = TH3_LOOKAROUND ;
#endif
	entk->count3 = 0 ;
}

static	void	Think3_NoiseWatch( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_NOISE, entk ) ; /* んっ */
	}

	if ( entk->act->act_end ) {
		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) { 
			/* はずしていたら */
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

static	void	Think3_NoiseWatchAgain( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->act->aim_pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_NoiseWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		if ( (entk->notice_retry == 1) && (COM_GetGroupNum( entk ) > 1)) {
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				if ( !(entk->iknow_flag & IKNOW_CLEARING) ) {
					if ( entk->tmp_count == NOISE_SCREAM ) {
						COM_SetRadio( EV_DOWNEME_AVOID, entk ) ;	/* どうした？ */
					} else {
						COM_SetRadio( EV_NOISE_AVOID, entk ) ;	/* 音がした？ */
					}
				}
			} else {
				if ( entk->tmp_count == NOISE_SCREAM ) {
					COM_SetRadio( EV_DOWNEME_AVOID, entk ) ;	/* どうした？ */
				} else {
					COM_SetRadio( EV_NOISE_SNEAK, entk ) ;	/* 音がした？ */
				}
			}
		}
		entk->act->aim_pos = entk->trgpoint.pos ;
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

static	void	Think3_ClearingNoiseWatch( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->act->act_end ) {
		if ( entk->notice_retry == 0 ) {
			entk->think3 = TH3_STOPSIGN ; 
		} else {
			R_INTRPT *r_intrpt ;
			int		lz ;
			if ( (r_intrpt = GM_GetRIntrptOne( entk->trgpoint.addr ))  == NULL ) {
				entk->think3 = TH3_WAIT_CAUTION ; 
printf("noise mode Think3_ClearingNoiseWatch   kore1\n");
			} else {
				if ( (lz = LockerZone( r_intrpt )) > 0 ) {
					HZX_ZONE_ADD zoneaddr ;

printf("noise mode Think3_ClearingNoiseWatch   kore2\n");
					zoneaddr = ( lz == 1 ) ? r_intrpt->zone1 : r_intrpt->zone2 ;
					ENE_SetTrgpZoneaddr( &(entk->trgpoint), zoneaddr ) ;
					entk->act->aim_pos = entk->trgpoint.pos ;
					entk->act->aim_pos.vy += 1500.0f ;

					entk->think3 = TH3_WATCH_AGAIN ;
//					SetThinkMove( entk, MoveEndCondition, -1 ) ;
				} else {
printf("noise mode Think3_ClearingNoiseWatch   kore3\n");
					entk->think3 = TH3_WAIT_CAUTION ; 
				}
			}
		}
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

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
		R_INTRPT *r_intrpt ;
		int		lz ;
printf("noise: addr [%x] \n", entk->trgpoint.addr ) ;
		if ( (r_intrpt = GM_GetRIntrptOne( entk->trgpoint.addr ))  == NULL ) {
printf("noise mode aaaaaaaaaaaaa   kore1\n");
			entk->think3 = TH3_WAIT_CAUTION ; 
		} else {
printf("r_intrpt: zone1 [%x] zone2[%x] \n", r_intrpt->zone1, r_intrpt->zone2 ) ;
			if ( (lz = LockerZone( r_intrpt )) > 0 ) {
				HZX_ZONE_ADD zoneaddr ;

printf("noise mode aaaaaaaaaaaaa   kore2\n");
				zoneaddr = ( lz == 1 ) ? r_intrpt->zone1 : r_intrpt->zone2 ;
				ENE_SetTrgpZoneaddr( &(entk->trgpoint), zoneaddr ) ;
				entk->act->aim_pos = entk->trgpoint.pos ;
				entk->act->aim_pos.vy += 1500.0f ;

				SetThinkMove( entk, MoveEndCondition, -1 ) ;
			} else {
printf("noise mode aaaaaaaaaaaaa   kore3\n");
				entk->think3 = TH3_WAIT_CAUTION ; 
			}
		}

		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_WaitCaution( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 > COUNT_VMODE(120) ) {
		entk->think3 = TH3_RETURN_SIGN ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_ReturnSign( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_CLE_STOP_SIGN ;
	}

	if ( entk->act->act_end ) {
		entk->c_notice &= ~ENE_NOTICE_NOISE ;
		entk->think3 = TH3_END ; 
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

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
			COM_SetSpeak( EV_WHAT_NOISE, entk ) ; /* うーん */
		}
	}

	/* ルートのつながっている方向を見る 最大４回 */
	if ( !(entk->count3%COUNT_VMODE(60)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, entk->count3/COUNT_VMODE(60), entk->ctrl->hzx_id ) ;
		if ( near != 255 ) {
			entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
#if 1
			ENE_ZonePos( &entk->act->aim_pos, near, entk->ctrl->hzx_id ) ;
#endif
		} else {
			if ( entk->count3 < COUNT_VMODE(180) ) entk->count3 = COUNT_VMODE(180) ;
		}
	}

	diff_dir = GV_DiffDirAbs( entk->ctrl->turn.vy, entk->act->aim_dir ) ;
	if ( diff_dir < 1024 ) {
		float f ;

		if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;

#if 1
		f = entk->ctrl->levels[0] - entk->act->aim_pos.vy ;
		if ( f > 300.0f || f < -300.0f ) {
			entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
			entk->act->aim_pos.vy += 500.0 ;
		} else {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		}
#else
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
#endif
	} else {
		if ( entk->count3 > COUNT_VMODE(180) ) {
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
			}
		}
	}

	if ( entk->count3 > COUNT_VMODE(320) ) {
		entk->c_notice &= ~ENE_NOTICE_NOISE ;
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
static void NoiseCheack( ENETHINK *entk )
{
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		int power ;

		power = COM_GetNoisePower( ) ;
		if ( power == NOISE_S || power == NOISE_MM || power == NOISE_SCREAM ) {
			entk->notice_retry ++ ;

			if ( entk->iknow_flag & IKNOW_CLEARING ) {
				R_INTRPT *r_intrpt ;
				int		lz ;
				int	 hzx_id, zone ;
				HZX_ZONE_ADD zoneaddr ;

				entk->act->aim_pos = GM_NoisePosition ;	/* 振り向きの一時バッファに利用 */
#ifdef GMDEF_NOISE_MAP_RENEW
				zoneaddr = HZX_GetAddress( GM_NoiseHzxID, &GM_NoisePosition, -1 ) ;
#else
				hzx_id = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(GM_NoiseMap), &GM_NoisePosition, &zone ) ;
				zoneaddr = HZX_GetAddress( hzx_id, &GM_NoisePosition, -1 ) ;
#endif
				if ( (r_intrpt = GM_GetRIntrptOne( zoneaddr ))  != NULL ) {
					if ( (lz = LockerZone( r_intrpt )) > 0 ) {
						zoneaddr = ( lz == 1 ) ? r_intrpt->zone1 : r_intrpt->zone2 ;
						ENE_SetTrgpZoneaddr( &(entk->trgpoint), zoneaddr ) ;
						entk->trgpoint.pos.vy += 1000.0f ;
						entk->act->aim_pos = entk->trgpoint.pos ;
					}
				}
				entk->think3 = TH3_WATCH_AGAIN ;
				entk->count3 = 0 ;

				if ( entk->notice_retry > CLE_NOISE_ALERT_NUM ) {
					/* 発見モードになるように */
					entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
				}
			} else {
#ifdef GMDEF_NOISE_MAP_RENEW
				ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
				ENE_SetTrgpPosMap( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseMap ) ;
#endif
				entk->act->aim_pos = GM_NoisePosition ;
				if ( entk->notice_retry == 2 ) {
					entk->act->move_s = 
						( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionRun : MoveRun ;
				}
				if ( entk->notice_retry < 3 ) {
					entk->think3 = TH3_WATCH_AGAIN ;
					entk->count3 = 0 ;
				}
			}
		}

	}
}

static void CautionNoiseCheack( ENETHINK *entk )
{
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		int power ;

		power = COM_GetNoisePower( ) ;
		if ( power == NOISE_S || power == NOISE_MM || power == NOISE_SCREAM ) {

#ifdef GMDEF_NOISE_MAP_RENEW
			ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
			ENE_SetTrgpPosMap( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseMap ) ;
#endif
			entk->act->aim_pos = GM_NoisePosition ;
		}
		entk->think3 = TH3_CLE_WATCH ;
		entk->count3 = 0 ;
		entk->notice_retry ++ ;

		if ( (GM_AlertMode == ALERT_MODE_AVOID) && (GM_GameStatus & STATE_CLEARING) ) {
			if ( entk->notice_retry > CLE_NOISE_ALERT_NUM ) {
				/* 発見モードになるように */
				entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
			}
		}
	}
}

void THK_NoiseMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_NoiseWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_NoiseWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

	    case TH3_WATCH_AGAIN :
			Think3_NoiseWatchAgain( entk ) ;
		break ;

		//move.c
	    case TH3_ZONE_MOVE :
			Think3_ZoneMove( entk ) ;
			NoiseCheack( entk ) ;
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

	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
			NoiseCheack( entk ) ;
		break ;

	    case TH3_CLE_WATCH :
			Think3_ClearingNoiseWatch( entk ) ;
		break ;
	    case TH3_WAIT_CAUTION :
			Think3_WaitCaution( entk ) ;
		break ;
	    case TH3_RETURN_SIGN :
			Think3_ReturnSign( entk ) ;
			CautionNoiseCheack( entk ) ;
		break ;

	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_NoiseModeStart( entk )
ENETHINK	*entk ;
{
	int power ;

	power = COM_GetNoisePower( ) ;

	switch( power ) {
		case NOISE_S :
		case NOISE_SCREAM :
			/* 見に行くだけ */
			entk->think3 = TH3_WATCH ; 
			break ;
		default :
			entk->think3 = TH3_WATCH ; 
			break ;
	}

	entk->act->CheckPad = NoiseModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_NOISE ;
printf("noise: play hzx_id [%x] noismap[%x] noisehzxid[%x] GM_NoiseHzxID[%x]\n"
	,GM_PlayerControl->hzx_id,GM_NoiseMap, GM_GetHzxGroupID(GM_NoiseMap),GM_NoiseHzxID  ) ;
#ifdef GMDEF_NOISE_MAP_RENEW
	ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
	ENE_SetTrgpPosMap( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseMap ) ;
#endif

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->notice_retry = 0 ;

	entk->tmp_count = power ;

	entk->act->aim_pos = GM_NoisePosition ;
	if ( HZX_ZoneDistanceCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) > RUN_DISTANCE ){
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionRun : MoveRun ;
	} else {
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;
	}

	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}

void	THK_NoiseModeStartClearing( entk )
ENETHINK	*entk ;
{
	int power ;

	power = COM_GetNoisePower( ) ;

	switch( power ) {
		case NOISE_S :
		case NOISE_SCREAM :
			/* 見に行くだけ */
			entk->think3 = TH3_CLE_WATCH ; 
			break ;
		default :
			entk->think3 = TH3_CLE_WATCH ; 
			break ;
	}

	entk->act->CheckPad = NoiseModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_NOISE ;
printf("noise: play hzx_id [%x] noismap[%x] noisehzxid[%x] \n"
	,GM_PlayerControl->hzx_id,GM_NoiseMap, GM_GetHzxGroupID(GM_NoiseMap) ) ;
#ifdef GMDEF_NOISE_MAP_RENEW
	ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
	ENE_SetTrgpPosMap( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseMap ) ;
#endif

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->notice_retry = 0 ;
	entk->tmp_count = power ;

	entk->act->aim_pos = GM_NoisePosition ;
	if ( HZX_ZoneDistanceCrossGroup( entk->ctrl->addr, entk->trgpoint.addr ) > RUN_DISTANCE ){
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionRun : MoveRun ;
	} else {
		entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;
	}

	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}
