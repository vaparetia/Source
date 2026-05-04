//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkendam.c
	味方兵ダメージモード
	
	1999/09/03 Y.Korekado
	$Id: thkendam.c,v 1.1.1.3 2002/11/19 11:44:20 Yoshizawa1 Exp $
	
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
	TH3_WATCH,TH3_SET_GLASS, TH3_WATCH_PLAYER,TH3_GLASSWATCH_PLAYER,TH3_NO_WATCH,
	TH3_WATCH_ENEMY, TH3_ZONETRACE, TH3_ATTACK_NEAR, TH3_WAIT,TH3_LOOKAROUND_CHECK_ZONE,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL, TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR,
	TH3_NO_RADIO, /* radio.c */	TH3_ATTACK_MA, TH3_END, TH3_SEE_ENEMY, TH3_WAIT_WAKEUP, 
	TH3_DIRECTTRACE, TH3_MUDAASI, TH3_NO_PLOBLEM, TH3_CALL_ENEMY, TH3_LOOKAROUND,
	TH3_WATCH_HELPME, TH3_ERACE
} ;
enum {
	SP_NONE,
	SP_SETGLASS,	/* 双眼鏡セット */
	SP_GLASS,		/* 双眼鏡覗く */
	SP_CAUT_STAND,	/* 警戒立ち */

	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,

	SP_KERI,
	SP_SEE_ENEMY,	/* 生きているか確認 */
} ;

#define	WAKE_UP_DIS (1300)

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
	if ( time == COUNT_VMODE(22) ) {
//		ENE_SetOffenseTargetEne( act, 0 ) ;
		ENE_SetTargetEneDirect( act, act->the_target, 0 ) ;
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

/*----- アクションパッド --------------------------------------------*/
static int	EneDamageModeCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_SETGLASS :
			AT_SetModeFromPad( act, ENE_ActSetGlass, EM_glasses, act->pad ) ;
		break ;
		case SP_GLASS :
			AT_SetModeFromPad( act, ENE_ActGlass, EM_glasses_p, act->pad ) ;
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

		case SP_KERI :
			AT_SetModeFromPad( act, ActKeri, EM_nom_stomp, act->pad ) ;
		break ;

		case SP_SEE_ENEMY :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_see_patient, act->pad ) ;
		break ;
	}
	return 1 ;
}


/*----- 補助 --------------------------------------------*/
static int OtherEnemyDownWatchAction( ENETHINK *entk )
{
	ENETHINK	*other ;
	int dis ;

	other = COM_NearEnemyThkStatus( entk, THK_STATUS_DOWN_WATCH, &dis ) ;
	if ( other == NULL ) return 0 ;
	if ( entk->dam_entk != other->dam_entk ) return 0 ;
	if ( dis > 2000 ) return 0 ;

	return 1 ;
}


static int DmageEnemyCheck( ENETHINK *entk )
{
	ENETHINK	*endam ;
	EYEINFO		eyei ;

	if ( (endam = COM_GetEnemyFromUniqID( entk->enedam_id )) != NULL ) {
		ENE_SetEyeInfo( &eyei, &endam->ctrl->mov, &endam->ctrl->addr, 0, &endam->ctrl->hzx_id ) ;
		ENE_EyeInfoCheck( entk, &eyei ) ;

		/*無条件でＯＫ*/
		if ( eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( ENE_ZoneIntrptCheck( endam->ctrl->addr ) ) return 1 ;
		}

		/* 無条件でＯＵＴ*/
		if ( !ENE_InRangeXYZ( &entk->ctrl->mov, &endam->ctrl->mov, 2000, 3000, 2000 ) ) {
			if ( eyei.sight == EYE_INFO_SIGHT_IN ) {
				/* 視界内でゾーン外に捨てられているからＯＫ */
				if( !ENE_InZone( &endam->ctrl->mov, endam->ctrl->addr ) ) return 1 ;
			}
			return 0 ;
		}
		/*無条件でＯＫ*/
		if ( ENE_InRangeXYZ( &entk->ctrl->mov, &endam->ctrl->mov, 1000, 1500, 1000 ) ) {
			return 1 ;
		}

//printf("enedamcorp sight[%d] dis[%d] dir[%d] \n",eyei.sight, eyei.dis, eyei.dir ) ;
		if ( eyei.sight != EYE_INFO_SIGHT_IN ) return 0 ;

		return 1 ;
	}
	
	return 0 ;
}
/*----- 思考 --------------------------------------------*/
#include	"notice_radio.c"

static void Think3_WatchDamageBuddy( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
		COM_SetSpeak( EV_NOTICE_ENEDAM, entk ) ; /* んっ */
		entk->act->pad = SP_CAUT_STAND ;
	}

	if ( entk->count3 > COUNT_VMODE(15) ) {
		if ( entk->dam_entk->iknow_flag & IKNOW_HELP_ME ) {
			UNSET_FLAG( entk->dam_entk->iknow_flag, IKNOW_HELP_ME ) ;
			SetThinkRadioSet( entk, EV_RAD_ENE_DAMAGE, ENE_NOTICE_ENE_DAMAGE ) ;
		} else if ( entk->pl_eyei.dis < entk->sense.eye_s + (entk->sense.eye_s/2) ) {
			entk->think3 = TH3_WATCH_PLAYER ; 
		} else {
			if ( entk->act->bodyp.type & ENE_TYPES_NO_GLASS ) {
				entk->think3 = TH3_WATCH_PLAYER ; 
			} else {
				entk->think3 = TH3_SET_GLASS ; 
			}
		}
		entk->count3 = 0 ;
		return ;
	}


	entk->count3 ++ ;
}

static void Think3_SetGlass( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_SETGLASS ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	if ( entk->act->act_end ) {
		entk->act->pad = SP_GLASS ;
		entk->think3 = TH3_GLASSWATCH_PLAYER ; 
		entk->count3 = 0 ;
		return ;
	}

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

static void Think3_WatchPlayer( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	if ( entk->count3 > COUNT_VMODE(90) ) {
		entk->think3 = TH3_NO_WATCH ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_NoWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QR ) ;
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

//	if ( entk->count3 == COUNT_VMODE(90) ) {
	if ( entk->count3 == COUNT_VMODE(120) ) {
		/* ダメージを受けた兵が無線をかけれる状態ではなかったら */
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
			entk->think3 = TH3_END ;
		} else if ( (entk->dam_entk->act->status & ACT_STATUS_FAINT) ||
			((entk->dam_entk->act->bodyp.pbreak&PBREAK_ARMS) == PBREAK_ARMS) ) {
			if ( ENE_OtherEnemyRadioAction( entk ) ) {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			} else {
				SetThinkRadioSet( entk, EV_RAD_ENE_DAMAGE, ENE_NOTICE_ENE_DAMAGE ) ;
			}
//起こしちゃ駄目、やるなら近づいて起こす様	entk->dam_entk->act->bodyp.faint_time = 20 ;
		} else {
#if 1
			entk->think3 = TH3_WAIT ;
#else
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
			entk->think3 = TH3_END ;
#endif
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
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
			UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
			entk->think3 = TH3_END ; 
		} else {
			if ( ENE_OtherEnemyRadioAction( entk ) ) {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			} else {
				SetThinkRadioSet( entk, EV_RAD_ENE_DAMAGE, ENE_NOTICE_ENE_DAMAGE ) ;
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

static void Think3_WatchDownEnemy( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_ZoneTrace( entk )
ENETHINK	*entk ;
{
	int	dis ;

//printf("enedame: trace act[%lx]\n", entk->dam_entk->act->status ) ;

	if ( !(entk->dam_entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP)) &&
		!(entk->dam_entk->act->status_status&ACT_STST_HANG) 	) {
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		
		return ;
	}

	dis = _FVecTrgDis( &entk->ctrl->mov, &entk->dam_entk->ctrl->mov ) ;
	if ( dis < 3000 ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( dis < WAKE_UP_DIS ) {
		entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->dam_entk->ctrl->mov ) ;

		if ( OtherEnemyDownWatchAction( entk ) ) {
			if ( entk->dam_entk->act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
				entk->think3 = TH3_WAIT ;
			} else {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			}
		} else if ( !DmageEnemyCheck( entk ) ) {
			/* その場にいったのにそいつがいなかったら */
			entk->think3 = TH3_LOOKAROUND ; 
		} else if ( entk->dam_entk->act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
			entk->think3 = TH3_ATTACK_MA ;
		} else if ( entk->dam_entk->act->status & ACT_STATUS_FAINT ) {	/* 気絶していたら */
			entk->think3 = TH3_SEE_ENEMY ;
		} else if ( entk->dam_entk->act->status & ACT_STATUS_HOLD_UP ) {
			entk->think3 = TH3_CALL_ENEMY ;
		} else {
			entk->think3 = TH3_WAIT ;
		}


		entk->count3 = 0 ;
		
		return ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_DIRECTTRACE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectTrace( entk )
ENETHINK	*entk ;
{
//printf("enedame: trace act[%lx]\n", entk->dam_entk->act->status ) ;

	entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;


	if ( !(entk->dam_entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP)) &&
		!(entk->dam_entk->act->status_status&ACT_STST_HANG) 	) {
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		
		return ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), WAKE_UP_DIS ) < 0 ) {
		entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->dam_entk->ctrl->mov ) ;

printf("korewkorekorekore status [%lx]\n",entk->dam_entk->act->status);
		if ( OtherEnemyDownWatchAction( entk ) ) {
			if ( entk->dam_entk->act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
				entk->think3 = TH3_WAIT ;
			} else {
				entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
			}
		} else if ( !DmageEnemyCheck( entk ) ) {
			/* その場にいったのにそいつがいなかったら */
			entk->think3 = TH3_LOOKAROUND ; 
		} else if ( entk->dam_entk->act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
			entk->think3 = TH3_ATTACK_MA ;
		} else if ( entk->dam_entk->act->status & ACT_STATUS_FAINT ) {	/* 気絶していたら */
			entk->think3 = TH3_SEE_ENEMY ;
		} else if ( entk->dam_entk->act->status & ACT_STATUS_HOLD_UP ) {
			entk->think3 = TH3_CALL_ENEMY ;
		} else {
			entk->think3 = TH3_WAIT ;
		}
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}


static void Think3_SeeEnemy( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_DOWN_WATCH ) ;

	entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		entk->sense.status = RADAR_COLOR_RED ;
	}

	if ( entk->count3 == COUNT_VMODE(60) ) {
		entk->act->pad = SP_SEE_ENEMY ;
	}

	if ( entk->count3 >= COUNT_VMODE(60) ) {
		if ( entk->act->act_end ) {
			if ( entk->dam_entk->act->status & ACT_STATUS_FAINT ) {
				/* 気絶している兵を起こす */
				entk->dam_entk->act->bodyp.faint_time = 20 ;
			}
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
				entk->think3 = TH3_END ;
			} else {
				if ( ENE_OtherEnemyRadioAction( entk ) ) {
					entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
				} else {
					SetThinkRadioSet( entk, EV_RAD_ENE_DAMAGE, ENE_NOTICE_ENE_DAMAGE ) ;
				}
			}
			entk->count3 = 0 ;
			return ;
		}
	}
	entk->count3 ++ ;
}


/* 攻撃前の間 */
static	void	Think3_AttackMa( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_DOWN_WATCH ) ;

	entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	
	if ( !(entk->dam_entk->act->headmark2)  ) {/* 起きたら */
		if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
//			SetThinkRadioSet( entk, EV_RAD_MUDAASI, ENE_NOTICE_ENE_DAMAGE ) ;
			entk->think3 = TH3_MUDAASI ; 
		} else {
			entk->think3 = TH3_WAIT ;
		}
		entk->count3 = 0 ;
		
		return ;
	}
	if ( entk->count3 == 0 ) {
printf("aim[%f][%f][%f]\n",entk->act->aim_pos.vx,entk->act->aim_pos.vy,entk->act->aim_pos.vz ) ;
	}

	if ( entk->count3 > COUNT_VMODE(90) ) {
		if ( OtherEnemyDownWatchAction( entk ) ) { /* 念のため */
			UNSET_FLAG( entk->thk_status, THK_STATUS_DOWN_WATCH ) ;
			entk->think3 = TH3_WAIT ;
		} else {
			entk->think3 = TH3_ATTACK_NEAR ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static	void	Think3_AttackNear( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_DOWN_WATCH ) ;

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
		entk->act->aim_pos = entk->dam_entk->ctrl->mov ;
	}

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_KERI ;
		entk->act->the_target = &(entk->dam_entk->act->bodyp.deftrg) ;
	}
/*
	if ( entk->count3 == COUNT_VMODE(60) ) {
		if ( entk->dam_entk->act->status & ACT_STATUS_FAINT ) {
				entk->dam_entk->act->bodyp.faint_time = 20 ;
		}
	}
*/
	if ( entk->act->act_end ) {
		if ( (GM_AlertMode == ALERT_MODE_SNEAK) && 
			(ENE_PureAttacker( entk->act ) ) ) {
//			SetThinkRadioSet( entk, EV_RAD_MUDAASI, ENE_NOTICE_ENE_DAMAGE ) ;
			entk->think3 = TH3_MUDAASI ; 
		} else {
			entk->think3 = TH3_WAIT ; 
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
		ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
	}

	if ( entk->count3 > entk->tmp_time ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Wait( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 < COUNT_VMODE(120) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
		entk->act->aim_pos = entk->dam_entk->ctrl->mov ;
	}
	

	if ( entk->count3 > COUNT_VMODE(180) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_WaitWakeup( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 < COUNT_VMODE(120) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
		entk->act->aim_pos = entk->dam_entk->ctrl->mov ;
	}
	

	if ( entk->count3 > COUNT_VMODE(300) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_NoPloblem( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	entk->act->aim_pos = entk->dam_entk->ctrl->mov ;
	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->dam_entk->ctrl->mov ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_CallEnemy( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_DOWN_WATCH ) ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	if ( entk->count3 > COUNT_VMODE(180) ) {
		SET_FLAG( entk->dam_entk->receive, ENE_ORDER_HOW_DO ) ;
		entk->think3 = TH3_WAIT ; 
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

	if ( entk->count3 > COUNT_VMODE(320) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
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

static	void	Think3_Erace( entk )
ENETHINK	*entk ;
{

	if ( entk->count3 == 0 ) {
		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	if ( entk->count3 > COUNT_VMODE(40) ) {
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
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
#if 0//no use
static void EneDamageCheck( ENETHINK	*entk )
{
	/* 目的の兵の無事を視認すれば */
	if ( !(entk->dam_entk->act->status &  NG_STATUS) ) {
		entk->think3 = TH3_NO_PLOBLEM ; 
		entk->count3 = 0 ;
		return ;
	}
}
#endif

static void OtherActionCheck( ENETHINK *entk )
{
	if ( OtherEnemyDownWatchAction( entk ) ) {
		if ( entk->dam_entk->act->headmark2 == ACT_HEADMARK2_ANES ) {/* 寝ていたら */
			entk->think3 = TH3_WAIT ;
		} else {
			entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		}
		entk->count3 = 0 ;
	}
}
/*-----  --------------------------------------------*/
static void LookAroundCheck( ENETHINK *entk )
{
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		THK_EneDamageModeStart( entk ) ;
	}
}
/*-----  --------------------------------------------*/

void THK_EneDamage( entk )
ENETHINK	*entk ;
{
	ENETHINK *endam ;

//printf("enedame: trace act[%lx]\n", entk->dam_entk->act->status ) ;

	if ( (endam = COM_GetEnemyFromUniqID( entk->enedam_id )) == NULL ) {
#if 1
		if ( (entk->think3 != TH3_ERACE) &&
			 (entk->think3 != TH3_RADIOSET) &&
			 (entk->think3 != TH3_RADIOCALL) &&
			 (entk->think3 != TH3_RADIOBREAK) &&
			 (entk->think3 != TH3_MOVE_WAITAREA) &&
			 (entk->think3 != TH3_MOVE_WAITAREA_DIRECT) &&
			 (entk->think3 != TH3_REPAIR) &&
			 (entk->think3 != TH3_NO_RADIO)
			) {
			entk->think3 = TH3_ERACE ;
			entk->count3 = 0 ;
			return ;
		}
#else
		UNSET_FLAG( entk->c_notice, ENE_NOTICE_ENE_DAMAGE ) ;
		entk->think3 = TH3_END ;
		entk->count3 = 0 ;
		return ;
#endif
	}

	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_WatchDamageBuddy( entk ) ;
		break ;
	    case TH3_SET_GLASS :
	    	Think3_SetGlass( entk ) ;
		break ;
	    case TH3_GLASSWATCH_PLAYER :
	    	Think3_GlassWatchPlayer( entk ) ;
		break ;
	    case TH3_WATCH_PLAYER :
	    	Think3_WatchPlayer( entk ) ;
		break ;
	    case TH3_NO_WATCH :
	    	Think3_NoWatch( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
		    Think3_LookAroundCheckZone( entk ) ;
		break ;
	    case TH3_WATCH_ENEMY :
			Think3_WatchDownEnemy( entk ) ;
		break ;
	    case TH3_ZONETRACE :
			Think3_ZoneTrace( entk ) ;
			OtherActionCheck( entk ) ;
		break ;
	    case TH3_DIRECTTRACE :
			Think3_DirectTrace( entk ) ;
			OtherActionCheck( entk ) ;
		break ;
	    case TH3_SEE_ENEMY :
			Think3_SeeEnemy( entk ) ;
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

	    case TH3_ATTACK_MA :
			Think3_AttackMa( entk ) ;
		break ;
	    case TH3_MUDAASI :
			Think3_Mudaasi( entk ) ;
		break ;

	    case TH3_ATTACK_NEAR :
			Think3_AttackNear( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_Wait( entk ) ;
		break ;
	    case TH3_WAIT_WAKEUP :
			Think3_WaitWakeup( entk ) ;
		break ;
	    case TH3_NO_PLOBLEM :
			Think3_NoPloblem( entk ) ;
		break ;
	    case TH3_CALL_ENEMY :
			Think3_CallEnemy( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
			Think3_LookAround( entk ) ;
			LookAroundCheck( entk ) ;
		break ;

	    case TH3_ERACE :	/*死体になったら*/
			Think3_Erace( entk ) ;
		break ;

	    case TH3_END :
	    	Think3_End( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_EneDamageModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = EneDamageModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_ENE_DAMAGE ;
	entk->enedam_id = entk->dam_entk->uniq_id ;

	ENE_SetTrgpPoint( &(entk->trgpoint), &entk->dam_entk->ctrl->mov, entk->dam_entk->ctrl->hzx_id ) ;
	entk->act->aim_pos = entk->dam_entk->ctrl->mov ;
printf("start aim[%f][%f][%f]\n",entk->act->aim_pos.vx,entk->act->aim_pos.vy,entk->act->aim_pos.vz ) ;

printf("enedame: start act[%lx]\n", entk->dam_entk->act->status ) ;
printf("enedame1: act[%lx]\n", entk->dam_entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP) ) ;
printf("enedame2: start act[%lx]\n", 
(entk->dam_entk->act->status & (ACT_STATUS_FALL|ACT_STATUS_SLEEP)) ) ;
	if ( entk->dam_entk->iknow_flag & IKNOW_HELP_ME ) {
printf("enedam: help me\n");
		entk->think3 = TH3_WATCH ; 
		entk->sense.status = RADAR_COLOR_RED ;
	} else if ( entk->dam_entk->act->old_status & ACT_STATUS_MASUI_SASARU ) {
printf("enedam: masui sasaru me\n");
		entk->think3 = TH3_WATCH ; 
		entk->sense.status = RADAR_COLOR_RED ;
	} else if ( (entk->dam_entk->act->status & (ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP)) || 
		(entk->dam_entk->act->status_status&ACT_STST_HANG) ||
		((entk->dam_entk->act->status&(ACT_STATUS_FALL|ACT_STATUS_SLEEP)) == (ACT_STATUS_FALL|ACT_STATUS_SLEEP)) ) {
printf("enedam: holdup or faint or down sleep \n");
		entk->think3 = TH3_WATCH_ENEMY ;
		entk->sense.status = RADAR_COLOR_YELOW ;
	} else {
printf("enedam: else \n");
		entk->think3 = TH3_WATCH ; 
		entk->sense.status = RADAR_COLOR_RED ;
	}
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}
