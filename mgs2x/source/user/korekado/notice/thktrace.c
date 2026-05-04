//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thktrace.c
	血、足跡、追跡モード
	
	2000/06/21 Y.Korekado
	$Id: thktrace.c,v 1.1.1.3 2002/11/19 11:44:22 Yoshizawa1 Exp $
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

#include "BP_Misc.h"

/*-----  --------------------------------------------*/
#define SHUSEI_901	(1)
/*-----  --------------------------------------------*/
#define DIRECT_TRACE_RANGE	(500) /* 近づいて扉が開かないとそこで止まってしまうので注意 */
/*-----  --------------------------------------------*/

#define ENE_FIND_TYPE	(0)

enum {
	TH3_WATCH,TH3_NSIGHT_ON,TH3_ZONETRACE,TH3_LOOKAROUND,TH3_WAIT,
	TH3_DIRECTTRACE, TH3_END,TH3_LOOK,TH3_LOOK_AHEAD,TH3_STOPSIGN,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_ZONETRACE2,TH3_DIRECTTRACE2,TH3_LOOK_LV3, TH3_DIRECTMOVE, TH3_PINPOINT_MOVE,
	TH3_LOCKER_OPEN, TH3_LOOK_DOWN, TH3_CLE_WATCH, TH3_WAIT_CAUTION, TH3_RETURN_SIGN,
	TH3_CLE_WATCH_FAST, TH3_SEARCH_SIGN
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_SURPRISE_BOMB,	/* ボムでびっくり */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,	/* ストップサイン */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
	SP_PINPOINT,	/* ピンポイント移動 */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_CLE_PEEP_D,	/* 下覗き込み */
	SP_GO_SIGN,		/* ゴーサイン */
	SP_NORMAL_STAND,		/* 動かない立ち */
} ;

/*----- アクション --------------------------------------------*/
static	void ActLockerOpen( act, time )
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

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}
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

/*----- アクションパッド --------------------------------------------*/

static int	TraceModeCheckPad( act )
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
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActTurn ) ;
			return 1 ;
		break ;
		case SP_CLE_STOP_SIGN :
			AT_SetModeFromPad( act, ENE_ActCleSign, EM_cle_signe_stop, act->pad ) ;
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

		case SP_LOCKER_OPEN :
			AT_SetModeFromPad( act, ActLockerOpen, EM_cle_locker_open, act->pad ) ;
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
				AT_SetModeFromPad( act, ActPeepD, EM_cle_look_d, act->pad ) ;
			}
		break ;
		case SP_GO_SIGN :
			AT_SetModeFromPad( act, ENE_ActCleSign, EM_cle_signe_move, act->pad ) ;
		break ;
		case SP_NORMAL_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
	}
	return 1 ;
}
/*----- 補助 --------------------------------------------*/
static int RintrptCheck( int zoneaddr )
{
	R_INTRPT	*r_intrpt ;

	r_intrpt = GM_GetRIntrptOne( zoneaddr ) ;
	if ( r_intrpt != NULL ) {
		if ( ((r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED))
					 == (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ) 
			&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
			return 1 ;
		}
	}
	return 0 ;
}

/* ターゲットアドレスがロッカー前ならターゲットポスに移動場所を与えて１を返す */
int	RintrptCheckAndTrgSet( entk )
ENETHINK	*entk ;
{
	R_INTRPT	*r_intrpt ;
	int			dir ;
	r_intrpt = GM_GetRIntrptOne( entk->trgpoint.addr ) ;
	if ( r_intrpt != NULL ) {
		if ( ((r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED))
					 == (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) ) 
			&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
			ENE_StandPosRintrpt( r_intrpt, MOT_RINTRPT_LOCKER_OPEN,
						&entk->act->target_pos, &dir ) ;
			entk->act->aim_dir = dir ;

			return 1 ;
		}
	}
	return 0 ;
}

static ENEFIND	*GetEneFind( int efl_id, int ef_id )
{
	ENEFINDLIST	*efl ;
	ENEFIND	*ef ;
	
	efl = GM_GetEneFindList( efl_id )  ;
	if ( efl == NULL ) return NULL ;
	ef = GM_GetEneFindFromList( efl, ef_id ) ;
	
	return ef ;
}


static	int	OnlineEneFind( entk, ef )
ENETHINK	*entk ;
ENEFIND	*ef ;
{
	if ( ENE_ReadOnlinInfo( entk->ctrl->addr, ef->zoneaddr ) ) {
		HZX_ZON	*zone ;

		zone = HZX_GetZoneFromAdd( ef->zoneaddr ) ;
		/* リンクゾーン先じゃないのに見えない */
		if ( !(zone->flag & HZX_ZON_LINK) ) return 0 ;
	}

	/* オンラインチェックで見えない */
	if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &(entk->ctrl->mov), &ef->pos ) ) return 0 ;

	/* 段ボールで見えない */
	if ( GM_PlayerStatus & PLAYER_CB_BOX ) {
		if ( ENE_InRange( &ef->pos, &GM_PlayerPosition, 500 ) )	return 0 ;
	}

	return 1 ;
}

static	void	SetEneFind( entk, ef )
ENETHINK	*entk ;
ENEFIND	*ef ;
{
	SET_FLAG( ef->type, EF_TYPE_FOUND ) ;
	entk->ef_id = ef->id ;
	ENE_SetTrgp( &(entk->trgpoint), &ef->pos, ef->zoneaddr ) ;
	entk->act->aim_pos = entk->trgpoint.pos ;
printf("set next efid[%d] addr[%x]\n",entk->ef_id, ef->zoneaddr ) ;
}

static	int	SetNextEneFindInSight( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;
	int dir, dis ;

	ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
	if( ef != NULL && ef->next != NULL ) {
		/* 自分の位置から跡が直線で見える */
		if ( OnlineEneFind( entk, ef->next ) ) {
			dis = _FVecTrgDis( &entk->ctrl->mov, &ef->next->pos ) ;
			if ( dis < entk->sense.eye_s ) {
				SetEneFind( entk, ef->next ) ;
				dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
				if( _DiffDirAbs( dir , entk->ctrl->rot.vy ) > 512 ) {
					entk->act->dir = dir ;
				}
				return dis ;
			}
		}
	}

	return -1 ;
}

static	int	SetSearchPlaceEneFind( ENETHINK *entk, ENEFIND *ef )
{
	R_INTRPT	*r_intrpt ;
	if( ef != NULL ) {
		if( ef->next != NULL && ef->next->type & EF_TYPE_INTRUDE ) {
			SetEneFind( entk, ef->next ) ;
			return 1 ;
		}
		if ( (r_intrpt = GM_GetRIntrptOne( ef->zoneaddr )) != NULL ) {
			if ( ( (r_intrpt->status&(ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED))
					 == (ROOT_INTRPT_CLOSE|ROOT_INTRPT_HINGED) )
				&& !(r_intrpt->status & (ROOT_INTRPT_NOENE|ROOT_INTRPT_BREAK)) ) {
				SetEneFind( entk, ef ) ;
				return 1 ;
			}
		}
		if ( GM_PlayerStatus & PLAYER_CB_BOX ) {	/* 段ボール */
			if ( ENE_InRangeToRange( &ef->pos, &GM_PlayerPosition, 500, 1000 ) ) return 1 ;
		}
	}


	return 0 ;
}

/*----- 思考 --------------------------------------------*/
#include	"notice_radio.c"

static	void	Think3_TraceWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_FOOT, entk ) ; /* んっ */
		KR_FindCameraCall( entk->act->body, &entk->trgpoint.pos, entk->ctrl->map ) ;
	}

	if ( entk->count3 == 4 ) {
//		GM_SeSetMode( SD_S_IDISP02, &entk->ctrl->mov, GM_SEMODE_NORMAL ) ;/* 「カチャッ」 */
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


static	void	Think3_CleTraceWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_STOPSIGN ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_TraceWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_ZONETRACE2 ; 
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
		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) {
			/* はずしていたら */
			entk->think3 = TH3_NSIGHT_ON ; 
		} else {
			entk->think3 = TH3_WAIT_CAUTION ; 
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
		entk->think3 = TH3_ZONETRACE2 ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_TraceMove2( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;
	int dis ;
	int	intrpt ;
	
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X|ENE_STATUS2_AIM_FACE_Y ;
	}

	dis = _FVecTrgDis( &entk->ctrl->mov, &entk->trgpoint.pos ) ;
	if( dis < (entk->sense.eye_s/2) ) {
	}

#ifdef  SHUSEI_901	
	intrpt = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ;
	if (  intrpt < 0 ) {
		if ( intrpt == -2 ) {	/* 次のゾーンが進入禁止ゾーン */
			entk->think3 = TH3_LOOK_AHEAD ; 
		} else {
			if( ENE_InZone( &entk->trgpoint.pos, entk->trgpoint.addr ) ) {
				entk->act->dir = entk->ctrl->turn.vy ;
				entk->think3 = TH3_DIRECTTRACE2 ;	/* 歩きつづける */
			} else {
				ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
				if ( (ef != NULL) && (ef->type & EF_TYPE_INTRUDE) ) {
					entk->think3 = TH3_LOOK_DOWN ;	/* 下覗き込み */
				} else {
					entk->think3 = TH3_LOOKAROUND ;	/* 周りを見渡す */
				}
			}
		}
		entk->count3 = 0 ;
		return ;
	} else if ( intrpt == 1 ) {	/* R_INTRPT */
		/* 今いるゾーンと次のゾーンが遮断されていたら */
		if( RintrptCheckAndTrgSet( entk ) ) {	/* Ｒインタラプトの前だったら */
			entk->think3 = TH3_DIRECTMOVE ;		/* ロッカー開けるための座標あわせ */
		} else {								/* それ以外 */
			R_INTRPT	*r_intrpt ;
			r_intrpt = GM_GetRIntrpt( entk->znavi->this_addr, entk->znavi->next_addr ) ;
			if ( r_intrpt != NULL ) {
				entk->think3 = TH3_LOOKAROUND ;		/* 周りを見渡す */
			}
		}
	}
#else
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if( ENE_InZone( &entk->trgpoint.pos, entk->trgpoint.addr ) ) {
			entk->act->dir = entk->ctrl->turn.vy ;	/* 歩きつづける */
			entk->think3 = TH3_DIRECTTRACE2 ;
		} else {
			ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
			if ( ef != NULL && ef->type & EF_TYPE_INTRUDE ) {
				entk->think3 = TH3_LOOK_DOWN ;
			} else {
				entk->think3 = TH3_LOOKAROUND ;
			}
		}
		entk->count3 = 0 ;
		
		return ;
	}
#endif

	/* 定期的に次の跡をチェック*/
	if ( !(entk->count3%COUNT_VMODE(14)) ) {
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if( ef != NULL && ef->next != NULL ) {
			if ( OnlineEneFind( entk, ef->next ) ) {
				dis = _FVecTrgDis( &entk->ctrl->mov, &ef->next->pos ) ;
				if ( dis < entk->sense.eye_s ) {
					/* 目標位置を更新*/
					SetEneFind( entk, ef->next ) ;
					if ( dis < (entk->sense.eye_s/2) ) {
						entk->think3 = TH3_LOOK_AHEAD ; 
						entk->count3 = 0 ;
						
						return ;
					}
				}
			}
		}
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectTrace( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;
	int dis ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), DIRECT_TRACE_RANGE ) < 0 ) {
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if( ef != NULL && ef->next != NULL ) {
			/* 跡から次の跡が直線で見える */
			if ( OnlineEneFind( entk, ef->next ) ) {
				SetEneFind( entk, ef->next ) ;

				entk->act->dir = entk->ctrl->turn.vy ;	/* 歩きつづける */
				entk->think3 = TH3_ZONETRACE2 ; 
				entk->count3 = 0 ;
				return ;
			}
		}
		if ( entk->tmp_buff[ ENE_FIND_TYPE ] & EF_TYPE_LV3 ) {
			entk->think3 = TH3_LOOK_LV3 ;
		} else {
			entk->think3 = TH3_LOOKAROUND ;
		}
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	/* 定期的に次の跡をチェック */
	if ( !(entk->count3%COUNT_VMODE(14)) ) {
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if( ef != NULL && ef->next != NULL ) {
			/* 跡から次の跡が直線で見える */
			if ( OnlineEneFind( entk, ef->next ) ) {
				dis = _FVecTrgDis( &entk->ctrl->mov, &ef->next->pos ) ;
				if ( dis < entk->sense.eye_s ) {
					SetEneFind( entk, ef->next ) ;
					if ( dis < (entk->sense.eye_s/2) ) {
						entk->think3 = TH3_LOOK_AHEAD ; 
					} else {
						entk->think3 = TH3_ZONETRACE2 ; 
					}
					
					entk->count3 = 0 ;
					return ;
				}
			}
		}
	}

	entk->count3 ++ ;
}

static	void	Think3_TraceMove( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X|ENE_STATUS2_AIM_FACE_Y ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if( ENE_InZone( &entk->trgpoint.pos, entk->trgpoint.addr ) ) {
			entk->act->dir = entk->ctrl->turn.vy ;	/* 歩きつづける */
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

static void Think3_DirectTrace2( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), DIRECT_TRACE_RANGE ) < 0 ) {
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if( ef != NULL && ef->next != NULL ) {
			/* 跡から次の跡が直線で見える */
			if ( OnlineEneFind( entk, ef->next ) ) {
				SetEneFind( entk, ef->next ) ;
				entk->think3 = TH3_LOOK_AHEAD ;
				entk->count3 = 0 ;
				return ;
			}

			/* 次がイントルードだったら */
			if ( ef->next->type & EF_TYPE_INTRUDE ) {
				SetEneFind( entk, ef->next ) ;
				entk->think3 = TH3_LOOK_DOWN ;
				entk->count3 = 0 ;
				return ;
			}
		}
		
		/* 最後の跡が・・・ */
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if ( ef != NULL && ef->type & EF_TYPE_INTRUDE ) {			/* イントルードだったら */
printf(" trace mode intrude!!\n");
			entk->think3 = TH3_LOOK_DOWN ;
		} else {
			if( RintrptCheckAndTrgSet( entk ) ) {	/* Ｒインタラプトの前だったら */
printf(" trace mode R intrpt!!\n");
				entk->think3 = TH3_DIRECTMOVE ;
			} else {								/* それ以外 */
printf(" trace mode Normal!!\n");
				entk->think3 = TH3_LOOKAROUND ;
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}


/* ロッカー開ける為の座標あわせ */
static void Think3_DirectMove( entk )
ENETHINK	*entk ;
{
//	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	if( ENE_InRange( &entk->ctrl->mov, &entk->act->target_pos, 350 ) ) {
		entk->think3 = TH3_PINPOINT_MOVE ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->target_pos ) ;
	entk->count3 ++ ;
}

static void Think3_PinpointMove( entk )
ENETHINK		*entk ;
{
	entk->act->pad = SP_PINPOINT ;

//	if ( entk->act->act_end ) {
	if ( entk->act->act_end || entk->count3 > COUNT_VMODE(300) ) {
		entk->think3 = TH3_LOCKER_OPEN ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->act->aim_dir ;

	entk->count3 ++ ;

}

static void Think3_LockerOpen( entk )
ENETHINK		*entk ;
{
	if ( entk->count3 == 0 ) {
		R_INTRPT	*r_intrpt ;

		r_intrpt = GM_GetRIntrptOne( entk->trgpoint.addr ) ;
		if ( r_intrpt != NULL ) {
			GM_RIntrptCallBack( r_intrpt, 100, 5, RINTRP_ORDER_OPEN ) ;
		}
		entk->act->pad = SP_LOCKER_OPEN ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;

}

static	void	Think3_LookAhead( entk )
ENETHINK	*entk ;
{
	ENEFIND	*ef ;
	int dis ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

   if ( (BP_IsPAL()==TRUE && ((entk->count3 % 16)==15)) ||
        (BP_IsPAL()!=TRUE && ((entk->count3 % 20)== 19)) )
   {
#ifdef  SHUSEI_901	
		/* 次のゾーンをチェック */
		if ( (dis = SetNextEneFindInSight( entk )) >= 0 ) {
			/* 見えたら */
			if ( dis > (entk->sense.eye_s/2) ) {
				/* 少し離れていたら */
				ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
				if( GM_GetZIntrptZ2Z2( entk->ctrl->addr, ef->zoneaddr ) < 0 ) {
					/* 間に進入禁止ゾーンがなかったら */
					entk->think3 = TH3_ZONETRACE2 ; 
				}
			}
			entk->count3 = 0 ;
			return ;
		}
		
		/* 今のゾーンをチェック */
		ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
		if ( ef != NULL ) {		/* 消えていなかったら */
			if( GM_GetZIntrptZ2Z2( entk->ctrl->addr, ef->zoneaddr ) < 0 ) {
				/* 間に進入禁止ゾーンがなかったら */
				entk->think3 = TH3_ZONETRACE2 ; /*今のenefindまで移動*/
				entk->count3 = 0 ;
				return ;
			}
			if ( ef->type & EF_TYPE_INTRUDE ) {
				/* イントルードだったら */
				entk->think3 = TH3_LOOK_DOWN ;
				entk->count3 = 0 ;
				return ;
			}
		}
		
		entk->think3 = TH3_LOOKAROUND ;	/* 周りを見渡す */
		entk->count3 = 0 ;
		return ;
#else
		/* 次のゾーンをチェック */
		if ( (dis = SetNextEneFindInSight( entk )) >= 0 ) {
			if ( dis > (entk->sense.eye_s/2) ) {
				ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
				if( !ENE_ZoneIntrptCheck( ef->zoneaddr ) ) {
					/* 進入禁止ゾーンじゃなかったら*/
					entk->think3 = TH3_ZONETRACE2 ; 
				}
			}
			entk->count3 = 0 ;
			return ;
		}
		
		if ( (ef = GetEneFind( entk->efl_id, entk->ef_id )) != NULL ) {
			if( ENE_ZoneIntrptCheck( ef->zoneaddr ) ) {
				if ( ef->type & EF_TYPE_INTRUDE ) {			/* イントルードだったら */
					entk->think3 = TH3_LOOK_DOWN ;
				} else {
					entk->think3 = TH3_LOOKAROUND ;
				}
				entk->count3 = 0 ;
				return ;
			}
		} else if( ENE_ZoneIntrptCheck( entk->trgpoint.addr ) ) {
			entk->think3 = TH3_LOOKAROUND ;
			entk->count3 = 0 ;
			return ;
		}

		entk->think3 = TH3_ZONETRACE2 ; /*今のenefindまで移動*/
		entk->count3 = 0 ;
		return ;
#endif
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_LookLv3( entk )
ENETHINK	*entk ;
{
	int 	type ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 == COUNT_VMODE(60) ) {
		type = entk->tmp_buff[ENE_FIND_TYPE] ;
		if ( type & EF_TYPE_ADULT ) {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BP ) ;
		} else if ( type & EF_TYPE_LV3 ) {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		} else {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
			if(	GM_AlertMode != ALERT_MODE_AVOID ){
				COM_SetSpeak( EV_WHAT_FOOT, entk ) ; /* うーん */
			}
		}
	}

	if ( entk->count3 == COUNT_VMODE(180) ) {
		if( RintrptCheckAndTrgSet( entk ) ) {
			entk->think3 = TH3_DIRECTMOVE ;
			entk->count3 = 0 ;

			return ;
		}

		if((entk->tmp_buff[ENE_FIND_TYPE] & EF_TYPE_LV3) && !(GM_AlertMode == ALERT_MODE_AVOID)) {
			SetThinkRadioSet( entk, EV_SEARCH_10, ENE_NOTICE_TRACE ) ;
		} else {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_TRACE ) ;
			entk->think3 = TH3_END ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_LookDown( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 1 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 == COUNT_VMODE(60) ) {
		entk->act->pad = SP_CLE_PEEP_D ;
	}
	if ( entk->count3 > COUNT_VMODE(60) ) {
		if ( entk->act->act_end ) {
			entk->think3 = TH3_LOOKAROUND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->count3 ++ ;
	return ;
}


static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
		if ( !(entk->iknow_flag & IKNOW_CLEARING) ) {
			ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
			COM_SetSpeak( EV_WHAT_FOOT, entk ) ; /* うーん */
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
		if ( entk->iknow_flag & IKNOW_CLEARING ) {
			/* クリアリングは終了 */
			UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;
		}
		entk->c_notice &= ~ENE_NOTICE_TRACE ;
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


static	void	Think3_WaitCaution( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;

	if( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( (entk->count3 % COUNT_VMODE(6)) == (COUNT_VMODE(6)-1) ) {
		if ( SetNextEneFindInSight( entk ) >= 0 ) {
			entk->count3 = 0 ;
			return ;
		} else {
			ENEFIND	*ef ;

			ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
			if ( SetSearchPlaceEneFind( entk, ef ) ) {
				entk->think3 = TH3_SEARCH_SIGN ;
				entk->count3 = 0 ;
				return ;
			}
		}
		entk->think3 = TH3_RETURN_SIGN ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_CleWatchFast( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;

	if( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( (entk->count3 % COUNT_VMODE(6)) == (COUNT_VMODE(6)-1) ) {
		if ( SetNextEneFindInSight( entk ) >= 0 ) {
			entk->count3 = 0 ;
			return ;
		} else {
			ENEFIND	*ef ;

			ef = GetEneFind( entk->efl_id, entk->ef_id ) ;
			if ( SetSearchPlaceEneFind( entk, ef ) ) {
				entk->think3 = TH3_SEARCH_SIGN ;
				entk->count3 = 0 ;
				return ;
			}
		}
	}

	if ( (entk->count3 > COUNT_VMODE(60)) ) {
		entk->c_notice &= ~ENE_NOTICE_TRACE ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
	return ;
}


static void Think3_ReturnSign( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_GO_SIGN ;
	}

	if ( entk->count3 >= 0 ) {
		if ( entk->act->act_end ) {
			entk->c_notice &= ~ENE_NOTICE_TRACE ;
			entk->think3 = TH3_END ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->count3 ++ ;
}

static void Think3_SearchSign( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_GO_SIGN ;
	}
	if ( entk->count3 == 0 ) COM_SetSpeak( EV_STOP_1, entk ) ;

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ZONETRACE2 ; 
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
void THK_TraceMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_TraceWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_TraceWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;
	    case TH3_ZONETRACE :
			Think3_TraceMove( entk ) ;
		break ;
	    case TH3_DIRECTTRACE :
			Think3_DirectTrace( entk ) ;
		break ;
	    case TH3_ZONETRACE2 :
			Think3_TraceMove2( entk ) ;
		break ;
	    case TH3_DIRECTTRACE2 :
			Think3_DirectTrace2( entk ) ;
		break ;
	    case TH3_LOOK_AHEAD :
		    Think3_LookAhead( entk ) ;
		break ;
	    case TH3_LOOK_LV3 :
		    Think3_LookLv3( entk ) ;
		break ;
	    case TH3_LOOK_DOWN :
			Think3_LookDown( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;

	    case TH3_DIRECTMOVE :
			Think3_DirectMove( entk ) ;
		break ;
	    case TH3_PINPOINT_MOVE :
	    	Think3_PinpointMove( entk ) ;
		break ;

	    case TH3_LOCKER_OPEN :
			Think3_LockerOpen( entk ) ;
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


	    case TH3_CLE_WATCH :
			Think3_CleTraceWatch( entk ) ;
		break ;
	    case TH3_WAIT_CAUTION :
			Think3_WaitCaution( entk ) ;
		break ;
	    case TH3_RETURN_SIGN :
			Think3_ReturnSign( entk ) ;
		break ;
	    case TH3_CLE_WATCH_FAST :
			Think3_CleWatchFast( entk ) ;
		break ;
	    case TH3_SEARCH_SIGN :
			Think3_SearchSign( entk ) ;
		break ;

	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}


//AN_Test_Eye2( &entk->trgpoint.pos, 1 );


}
/*----- 不審物発見モードスタート --------------------------------------------*/
void	THK_TraceModeStart( entk )
ENETHINK	*entk ;
{
	ENEFINDLIST	*efl ;
	ENEFIND	*ef, *cur ;


	entk->act->CheckPad = TraceModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->c_notice |= ENE_NOTICE_TRACE ;

	efl = GM_GetEneFindList( entk->efl_id )  ;
//printf("trace efl[%d] \n",entk->efl_id ) ;
	ASSERT( efl!=NULL ) ;
	SET_FLAG( efl->listtype, EF_LSIT_TYPE_TRACE ) ;

	ef = GM_GetEneFindFromList( efl, entk->ef_id ) ;
printf("notice:trace ef[%d] type[0x%x]\n",entk->ef_id, ef->type ) ;
	ASSERT( ef!=NULL ) ;

	cur = efl->start ;
	while ( cur != ef ) {	/* 発見した跡以前の跡は、全て発見済みにする */
		SET_FLAG( cur->type, EF_TYPE_FOUND ) ;
		cur = cur->next ;
	}
	SET_FLAG( ef->type, EF_TYPE_FOUND ) ;

	ENE_SetTrgp( &(entk->trgpoint), &ef->pos, ef->zoneaddr ) ;

	entk->tmp_buff[ ENE_FIND_TYPE ] = ef->type ;
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

	entk->act->aim_pos = entk->trgpoint.pos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->think3 = TH3_WATCH ; 
	entk->sense.status = RADAR_COLOR_YELOW ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}

void	THK_TraceModeStartCle( entk )
ENETHINK	*entk ;
{
	THK_TraceModeStart( entk ) ;
	entk->think3 = TH3_CLE_WATCH ; 
}

void	THK_TraceModeStartCleFast( entk )
ENETHINK	*entk ;
{
	THK_TraceModeStart( entk ) ;
	entk->think3 = TH3_CLE_WATCH_FAST ; 
}
