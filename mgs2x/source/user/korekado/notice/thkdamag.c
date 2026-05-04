//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkdamag.c
	ダメージモード
	
	1999/09/21 Y.Korekado
	$Id: thkdamag.c,v 1.1.1.3 2002/11/19 11:44:20 Yoshizawa1 Exp $
	
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
	TH3_WAIT, TH3_TURN, TH3_WATCH,TH3_NSIGHT_ON, TH3_WATCH_PLAYER,
	TH3_GLASSWATCH_PLAYER, TH3_NO_WATCH, TH3_MOVE_SAFEAREA,	TH3_MEDICAL, TH3_LOOKAROUND,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL, TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_MORTALLY, TH3_LOOKAROUND_CHECK_ZONE, TH3_END
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */

	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,

	SP_SETGLASS,	/* 双眼鏡セット */
	SP_GLASS,		/* 双眼鏡覗く */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_MEDICATION	/* 治療 */
} ;

/*-----  --------------------------------------------*/
/*----- アクション --------------------------------------------*/
/*----- チェックパッド --------------------------------------------*/
static int	DamageModeCheckPad( act )
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
		case SP_SETGLASS :
			AT_SetModeFromPad( act, ENE_ActSetGlass, EM_glasses, act->pad ) ;
		break ;
		case SP_GLASS :
			AT_SetModeFromPad( act, ENE_ActGlass, EM_glasses_p, act->pad ) ;
		break ;
		case SP_NSIGHT_ON :
			AT_SetModeFromPad( act, ENE_ActNSight_ON, EM_gogle_on, act->pad ) ;
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

		case SP_MEDICATION :
			AT_SetModeFromPad( act, ENE_ActMedication, EM_legl_morph_legl, act->pad ) ;
		break ;
	}

	return 1 ;
}
/*----- 思考 --------------------------------------------*/
#include "notice_radio.c"

static void Think3_Turn( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
	}
	if ( entk->count3 == 1 ) {
		if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) != PBREAK_LEGS ) {
			entk->act->dir = entk->pl_eyei.dir ;
		}
	}

	if ( entk->count3 > COUNT_VMODE(30) ) {
#if 1
		if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
#else
		if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) != PBREAK_LEGS ) {
#endif
			if ( entk->mess_notice == MES_NOTICE_FAINT ) {
				entk->think3 = TH3_WATCH_PLAYER ; 
			} else if ( entk->status & ENE_STATUS_NIGHT_SIGHT ) {
				if ( entk->act->sw->n_sight == 1 ) { /* はずしていたら */
					entk->think3 = TH3_NSIGHT_ON ; 
				} else {
					entk->think3 = TH3_WATCH_PLAYER ; 
				}
			} else if ( entk->pl_eyei.dis < entk->sense.eye_s + (entk->sense.eye_s/2) ) {
				entk->think3 = TH3_WATCH_PLAYER ; 
			} else {
				if ( entk->act->bodyp.type & ENE_TYPES_NO_GLASS ) {
					entk->think3 = TH3_WATCH_PLAYER ; 
				} else {
					entk->think3 = TH3_WATCH ; 
				}
			}
		} else {
			entk->think3 = TH3_WATCH_PLAYER ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static void Think3_GlassSet( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_SETGLASS ;
	}

	if ( entk->act->act_end ) {
		entk->act->pad = SP_GLASS ;
		entk->think3 = TH3_GLASSWATCH_PLAYER ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_GlassWatchPlayer( entk )
ENETHINK	*entk ;
{

	entk->act->pad = SP_GLASS ;

	/* 眼鏡装着 */
	entk->status2 = ENE_STATUS_GLASSES ;
//printf("pl dis = [%d] sight[%d] \n",entk->pl_eyei.dis,entk->pl_eyei.sight ) ;
	if ( entk->count3 > COUNT_VMODE(90) ) {
		entk->think3 = TH3_NO_WATCH ; 
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
		entk->think3 = TH3_WATCH_PLAYER ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_WatchPlayer( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;
//printf("pl dis = [%d] sight[%d] \n",entk->pl_eyei.dis,entk->pl_eyei.sight ) ;

	if ( entk->count3 == 0 ) {
//		entk->act->dir = entk->pl_eyei.dir ;
//		entk->act->aim_pos = GM_PlayerPosition ;
	}

//	entk->status2 |= ENE_STATUS2_AIM_FACE ;

	if ( entk->count3 > COUNT_VMODE(30) ) {
		if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) != PBREAK_LEGS ) {
			entk->think3 = TH3_NO_WATCH ; 
		} else {
			if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
				entk->think3 = TH3_MORTALLY ; 
			} else {
				out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
				if ( out != NULL && dis < entk->sense.hearing ) {
					entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
				} else {
					SetThinkRadioSet( entk, EV_RAD_DAMAGE, ENE_NOTICE_DAMAGE ) ;
				}
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_NoWatch( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;

	if ( entk->count3 == 0 ) {
	}

	if ( entk->count3 == COUNT_VMODE(4) ) {
		if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ||
			entk->iknow_flag & IKNOW_RADIO_BREAK ) {
			SetThinkRadioSet( entk, EV_RAD_DAMAGE, ENE_NOTICE_DAMAGE ) ;
			entk->count3 = 0 ;
		} else {
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				if ( entk->act->bodyp.pbreak != 0 ) {
					ENE_SetTrgpSafeArea( &(entk->trgpoint), entk->ctrl->addr&255,entk->ctrl->hzx_id ) ;
					entk->think3 = TH3_MOVE_SAFEAREA ; 
					entk->act->move_s = MoveRun ;
				} else {
					UNSET_FLAG( entk->c_notice, ENE_NOTICE_DAMAGE ) ;
					CLEAR_FLAG( entk->iknow_flag ) ;
					entk->think3 = TH3_END ; 
				}
			} else {
				out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
				if ( out != NULL && dis < entk->sense.hearing ) {
					entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
				} else {
// bui damage motion wait
// kataude musen
					SetThinkRadioSet( entk, EV_RAD_DAMAGE, ENE_NOTICE_DAMAGE ) ;
				}
			}
			entk->count3 = 0 ;
		}
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_LookAroundCheckZone( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;
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
			if ( entk->act->bodyp.pbreak != 0 ) {
				if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) != PBREAK_LEGS ) {
					ENE_SetTrgpSafeArea( &(entk->trgpoint), entk->ctrl->addr&255,
									 GM_GetBit(HZX_ZoneMapNo( entk->ctrl->addr )) ) ;
					entk->think3 = TH3_MOVE_SAFEAREA ; 
					entk->act->move_s = MoveRun ;
				} else {
					entk->think3 = TH3_MEDICAL ;
				}
			} else {
				UNSET_FLAG( entk->c_notice, ENE_NOTICE_DAMAGE ) ;
				CLEAR_FLAG( entk->iknow_flag ) ;
				entk->think3 = TH3_END ; 
			}
		} else {
			out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
			if ( out != NULL && dis < entk->sense.hearing ) {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			} else {
				SetThinkRadioSet( entk, EV_RAD_DAMAGE, ENE_NOTICE_DAMAGE ) ;
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


static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_MEDICAL ;
		entk->count3 = 0 ;
		
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_Medical( entk )
ENETHINK	*entk ;
{
//	if ( entk->count3 == 0 ) {
//	if ( entk->count3 == 1 ) {
		entk->act->pad = SP_MEDICATION ;
//		UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;
//	}

	if ( (entk->act->act_end) || entk->count3 > 500 ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_DAMAGE ) ;
		CLEAR_FLAG( entk->iknow_flag ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_LookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	if ( entk->count3 == COUNT_VMODE(180) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	
	if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(120) && entk->count3 < COUNT_VMODE(180) ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}

	if ( entk->count3 == COUNT_VMODE(240) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_DAMAGE ) ;
		CLEAR_FLAG( entk->iknow_flag ) ;
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_Mortally( entk )
ENETHINK	*entk ;
{

//printf("pl dis = [%d] sight[%d] \n",entk->pl_eyei.dis,entk->pl_eyei.sight ) ;

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
//		entk->act->aim_pos = GM_PlayerPosition ;
//		entk->status2 |= ENE_STATUS2_AIM_FACE ;
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
void THK_Damage( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_TURN :
			Think3_Turn( entk ) ;
	    break ;
	    case TH3_WATCH :
	    	Think3_GlassSet( entk ) ;
		break ;
	    case TH3_GLASSWATCH_PLAYER :
	    	Think3_GlassWatchPlayer( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

		/* radio.c*/
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
		/* radio.c end */

	    case TH3_WATCH_PLAYER :
			Think3_WatchPlayer( entk ) ;
		break ;
	    case TH3_NO_WATCH :
	    	Think3_NoWatch( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
		    Think3_LookAroundCheckZone( entk ) ;
		break ;

	    case TH3_MOVE_SAFEAREA :
			Think3_MoveSafeArea( entk ) ;
		break ;
	    case TH3_MEDICAL :
			Think3_Medical( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
			Think3_LookAround( entk ) ;
		break ;
	    case TH3_MORTALLY :
		    Think3_Mortally( entk ) ;
		break ;
	    case TH3_END :
	    	Think3_End( entk ) ;
		break ;
	}

}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_DamageModeStart( entk )
ENETHINK	*entk ;
{
printf("start damage mode\n");
	entk->act->CheckPad = DamageModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->c_notice = ENE_NOTICE_DAMAGE ;

	entk->think3 = TH3_TURN ; 
//	if ( entk->iknow_flag & (IKNOW_TARGET|IKNOW_DETECT) ) {
	if ( entk->iknow_flag & (IKNOW_DETECT) ) {
		entk->count3 = 1 ;/* ヘッドマーク無し  */
	} else {
		entk->count3 = 0 ;
	}

	entk->tmp_time = 0 ;
	entk->act->move_s = MoveRun ;

	entk->sense.status = RADAR_COLOR_RED ;

	SET_FLAG( entk->iknow_flag, IKNOW_TARGET ) ;
}

/* モードの変わり目等で怪我をしていたら */
void	THK_DamageMedicalModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = DamageModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_DAMAGE ;
	entk->tmp_time = 0 ;
	entk->act->move_s = MoveWalk ;

	entk->sense.status = RADAR_COLOR_RED ;
	if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
		/* 待機所に直しに行く */
		SetThinkRadioSet( entk, EV_RAD_DAMAGE, ENE_NOTICE_DAMAGE ) ;
		return ;
	} else {
		entk->think3 = TH3_MEDICAL ;
	}
	entk->count3 = 0 ;

}
