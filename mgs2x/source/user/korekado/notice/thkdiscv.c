//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkdiscv.c
	プレイヤー発見モード
	
	1999/09/22 Y.Korekado
	$Id: thkdiscv.c,v 1.1.1.3 2002/11/19 11:44:20 Yoshizawa1 Exp $
	
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

//#define JIK_ONSE (1)
void COM_DiscoverCount(void );

/*重野追加 長廊下兵用の定義*/
#include	"../../sigeno/defender/eve_a.h"

enum {
	TH3_WAIT,TH3_DISCOVERY_POSE_AVOID,TH3_DISCOVERY_POSE,TH3_INDIRECT_DISCOVERY_POSE,TH3_BOMB_DISCOVERY_POSE,
	TH3_NEARBOMB_DISCOVERY_POSE,TH3_RADIO_CALL,	TH3_RADIO_CALL_LOOKABOUT,TH3_ATTACK_SETUP, TH3_ATTACK_MGUN,
	TH3_SUPPORT_ATTACK_SETUP, TH3_SUPPORT_ATTACK_MGUN, TH3_SUPPORT_ATTACK_RELOAD, TH3_SUPPORT_ATTACK_NEAR, TH3_RADIOSET, 
	TH3_RADIOBREAK, TH3_WATCH, TH3_DISCOVERY_DELAY,	TH3_ATTACK_RELOAD, TH3_ATTACK_NEAR, 
	TH3_MOVE_WAITAREA, TH3_MOVE_SAFEAREA, TH3_NPC_DISCOVERY_POSE, TH3_NPC_ATTACK, TH3_NPC_ATTACK_SETUP,
	TH3_NPC_ATTACK_MGUN, TH3_NPC_ATTACK_RELOAD, TH3_NPC_ATTACK_SEARCH, TH3_NPC_ZONE_CHASE, TH3_NPC_DIRECT_CHASE,
	TH3_WAIT_AREA, TH3_DISCOVERY_NO_POSE, TH3_ATTACK_MOVE_WAITAREA, TH3_NPC_MEDICAL, TH3_MOVE_WAITAREA_DIRECT
	
} ;
enum {
	SP_NONE,
	SP_READYGUN,	/* 銃構える */
	SP_DISCOVERY,	/* 発見ポーズ */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_SURPRISE_BOMB,	/* ボムでびっくり */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_NEARATK,		/* 近接攻撃 */
	SP_MGUN_SHOT,	/* マシンガン撃つ */
	SP_RELOAD,		/* リロード */
	SP_KERI,
	SP_MEDICATION,	/* 治療 */
	SP_UNREAL, 
} ;

#define	gAme_lEvel	1	//実験用 

#define LONLY_FIGHT_IS_NO_RADIO	1 //実験用

/*-------------------------------------------------------------*/
extern void *NewDiscovery(FMATRIX *world) ;
/*----- アクション --------------------------------------------*/

static void ActDiscovery( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
#if 1
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
#else
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
#endif
		act->sw->eye_anim = 2 ;/* 見開き */

		GV_SetActorChild( act->w, NewDiscovery( &BODYWORLD(act->body, HUMAN21_ATAMA) ) ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->eye_anim = 1 ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/*----- アクション --------------------------------------------*/
static int	DiscoveryModeCheckPad( act )
ACTION	*act ;
{
	int	mot ;
	
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, ENE_DiscoveryMotion(act), act->pad ) ;
		break ;
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActReadyGun, mot, act->pad ) ;
		break ;
		case SP_NEARATK :
			AT_SetModeFromPad( act, ENE_ActNearAtk, EM_ak_attack_near_kick_l, act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActMgunShot, mot, act->pad ) ;
		break ;
		case SP_RELOAD :
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			AT_SetModeFromPad( act, ENE_ActReload, mot, act->pad ) ;
		break ;
		case SP_KERI :
			AT_SetModeFromPad( act, ENE_ActKeri, EM_shl_nom_attack_near, act->pad ) ;
		break ;
		case SP_RADIOCALL :
// bui damage motion wait
// kataude musen
			AT_SetModeFromPad( act, ENE_ActSetRadio, EM_cle_call_team, act->pad ) ;
		break ;
		case SP_RADIOCALL_BREAK :
			AT_SetModeFromPad( act, ENE_ActRadioBreak, EM_call_break_l, act->pad ) ;
		break ;
		case SP_SURPRISE_BOMB :
			AT_SetModeFromPad( act, ENE_ActOneTimeMotion, EM_surprised_bomb, act->pad ) ;
		break ;
		case SP_MEDICATION :
			AT_SetModeFromPad( act, ENE_ActMedication, EM_legl_morph_legl, act->pad ) ;
		break ;

		case SP_RETURNRADIO :
			act->keep_mot = EM_call_team_l_3end ;
			act->keep_pad = act->pad ;
			act->time = MAX_VOL_TIME ;
			return 0 ;
		break ;
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
	}

	return 1 ;
}
/*----- 思考 --------------------------------------------*/
static void SetAlertParam( ENETHINK *entk )
{
	if( entk->iknow_flag & IKNOW_PLAYER_POS ) COM_SetAlertStatus( COM_ALERT_PLAYER_DETECT ) ;
	if( entk->iknow_flag & IKNOW_NPC_POS ) COM_SetAlertStatus( COM_ALERT_NPC_DETECT ) ;

	entk->alert = MAX_ALERT_LEVEL ;
}

#if 0
static void SetPBreakThink ( entk )
ENETHINK *entk ;
{
	entk->think2 = TH2_PBREAK ;
	entk->count3 = 0 ;

printf(" SetPBreakThink	entk->pl_eyei.sight [%d]\n",entk->pl_eyei.sight ) ;

	/* 両足故障していたら */
	if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
		entk->think3 = TH3_MORTALLY ;
		return ;
	}

	/* 両腕故障していたら */
	if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		return ;
	}
	
	/* たま切れなら治療 */
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
printf("mag max\n");
		entk->think3 = TH3_MEDICAL ; 
		return ;
	}
	
	/* 片足なら */
	if ( entk->act->bodyp.pbreak & PBREAK_LEGS ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			/* 右手が無事なら */
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
				entk->think3 = TH3_ATTACK_SETUP ; 
				return ;
			}
printf("sight in but LEG & ARMR \n");
			entk->think3 = TH3_MEDICAL ; 
			return ;
		}

		entk->think3 = TH3_MEDICAL ; 
		return ;
	}

	/* 右手故障していたら */
	if ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
				!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
				!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
				entk->think3 = TH3_ATTACK_NEAR ;
				entk->count3 = 0 ;
				return ;
			}
		}
		entk->think3 = TH3_MEDICAL ; 
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		return ;
	}

	entk->think3 = TH3_MEDICAL ; 
}
#endif

static void Think3_DiscoveryDelay( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->think3 = TH3_DISCOVERY_POSE_AVOID ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_DiscoveryPoseAvoid( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_DISCOVERY ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
printf("kore GM_GameStatus[%x]\n",GM_GameStatus);
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}

	if ( entk->act->act_end ) {
		if ( ENE_PureAttacker( entk->act ) && (GM_AlertMode!=ALERT_MODE_AVOID) ) {
			COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
		}
		entk->c_notice = ENE_NOTICE_NONE ;
		SetAlertParam( entk ) ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;

		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_DiscoveryNoPose( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 1 ) {
		if ( ENE_PureAttacker( entk->act ) && (GM_AlertMode!=ALERT_MODE_AVOID) ) {
			COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
		}
		entk->c_notice = ENE_NOTICE_NONE ;
		SetAlertParam( entk ) ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;

		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_DiscoveryPose( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;

	if ( entk->count3 == 0 ) {
printf(" Player Detect!! \n ");
		entk->act->pad = SP_DISCOVERY ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		COM_SetSpeak( EV_NOTICE_PLAYER, entk ) ; /* あっ */
		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}

	if ( entk->count3 > COUNT_VMODE(6) ) {
		if ( entk->pl_eyei.dis < NEAR_ATK_DIS 
				&& !(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			GM_SeSetMode( SD_V_C07MAKI+(entk->act->name_id->voice%4) , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			return ;
		}
	}

	if ( entk->act->act_end ) {
		if ( entk->status & ENE_STATUS_LONELY_FIGHT ) {
#ifdef LONLY_FIGHT_IS_NO_RADIO
			if (GM_AlertMode!=ALERT_MODE_AVOID ) {
				COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
			}
			entk->c_notice = ENE_NOTICE_NONE ;
			SetAlertParam( entk ) ;
			entk->think3 = TH3_WAIT ; 
			entk->count3 = 0 ;
#else
			entk->c_notice = ENE_NOTICE_NONE ;
			entk->think3 = TH3_WAIT ; 
#endif
		} else if ( gAme_lEvel == 1 ) {
// 今はすべてここ
			entk->think3 = TH3_ATTACK_SETUP ; 
		} else if ( entk->iknow_flag & IKNOW_RADIO_BREAK ) {
			ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
			entk->think3 = TH3_MOVE_WAITAREA ; 
		} else {
			out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
			if ( out != NULL && dis < entk->sense.hearing  && 
					( COM_GetStrmLv( ) < STR_LEVEL_2 ) ) {
				entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
			} else {
				entk->thk_status |= THK_STATUS_RADIO ;
				entk->think3 = TH3_RADIOSET ; 
printf("ENE_GameStatus = %x\n",ENE_GameStatus ) ;
				if ( ENE_GameStatus & ENE_GMSTATUS_PLAYER_FOUND ) {
					entk->tmp_buff[0] = EV_RAD_FIND_PL ;
				} else {
					entk->tmp_buff[0] = EV_RAD_FIND_PL_FIRST ;
				}
			}
		}
		entk->count3 = 0 ;
		GM_SeSetMode( SD_V_C07MAKI+(entk->act->name_id->voice%4) , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}


static void Think3_NpcDiscoveryPose( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;

	if ( entk->count3 == 0 ) {
printf(" Player Detect!! \n ");
		entk->act->pad = SP_DISCOVERY ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		COM_SetSpeak( EV_NOTICE_PLAYER, entk ) ; /* あっ */
		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}

	if ( entk->act->act_end ) {
		if ( ENE_PureAttacker( entk->act ) ) {
			if (GM_AlertMode!=ALERT_MODE_AVOID) {
				COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
			}
			entk->c_notice = ENE_NOTICE_NONE ;
			SetAlertParam( entk ) ;
			entk->think3 = TH3_WAIT ; 
			entk->count3 = 0 ;
			return ;
		}

		if (GM_AlertMode==ALERT_MODE_AVOID) {
			entk->c_notice = ENE_NOTICE_NONE ;
			SetAlertParam( entk ) ;
			entk->think3 = TH3_WAIT ; 
			entk->count3 = 0 ;
			return ;
		}

		if ( entk->iknow_flag & IKNOW_RADIO_BREAK ) {
			entk->think3 = TH3_NPC_ATTACK_SETUP ; 
		} else {
			out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
			if ( out != NULL && dis < entk->sense.hearing  && 
					( COM_GetStrmLv( ) < STR_LEVEL_2 ) ) {
				entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
			} else {
				if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
					entk->think3 = TH3_ATTACK_SETUP ; 
				} else {
					entk->thk_status |= THK_STATUS_RADIO ;
					entk->think3 = TH3_RADIOSET ; 
printf("ENE_GameStatus = %x\n",ENE_GameStatus ) ;
					if ( ENE_GameStatus & ENE_GMSTATUS_PLAYER_FOUND ) {
//						entk->tmp_buff[0] = EV_RAD_FIND_PL ;
						entk->tmp_buff[0] = EV_RAD_FIND_PL_UNEXP ;
					} else {
						entk->tmp_buff[0] = EV_RAD_FIND_PL_FIRST ;
					}
				}
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->act->dir = entk->pl_eyei.dir ;
	} else if ( entk->iknow_flag & IKNOW_NPC_POS ) {
		entk->act->dir = entk->npc_eyei.dir ;
	}

	entk->count3 ++ ;
}

static	void	Think3_RadioSet( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL ;
	}

/*！！！保険をいれないとやばそう！！！*/
	if ( entk->act->act_end == 3 ) {
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			entk->think3 = TH3_RADIOBREAK ; 
		} else {
			if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
				entk->think3 = TH3_RADIO_CALL ; 
			} else {
				entk->think3 = TH3_RADIO_CALL_LOOKABOUT ; 
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->act->dir = entk->pl_eyei.dir ;
	} else if ( entk->iknow_flag & IKNOW_NPC_POS ) {
		entk->act->dir = entk->npc_eyei.dir ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_RadioCall( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( entk->tmp_buff[0], entk ) ;

		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV2_STRM_LENGTH ;
		}
		ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
//indistinct との行ったり来たり防止
		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
	}

	if ( entk->count3 < entk->tmp_time ) {
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			COM_StopRadio( entk ) ;
			entk->think3 = TH3_RADIOBREAK ; 
			entk->count3 = 0 ;
			return ;
		}
//indistinct との行ったり来たり防止
//		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
	}

	if ( entk->count3 == entk->tmp_time ) {
		entk->act->pad = SP_RETURNRADIO ;
		SetAlertParam( entk ) ;
	}

	if ( entk->act->act_end == 1 ) {
		entk->c_notice = ENE_NOTICE_NONE ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->act->dir = entk->pl_eyei.dir ;
	} else if ( entk->iknow_flag & IKNOW_NPC_POS ) {
		entk->act->dir = entk->npc_eyei.dir ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_RadioCallLookAbout( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( entk->tmp_buff[0], entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV2_STRM_LENGTH ;
		}
		ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->act->dir = entk->pl_eyei.dir ;
	} else {
		if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
		} else if ( entk->count3 > COUNT_VMODE(120) && entk->count3 < COUNT_VMODE(180) ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
		}
	}

	if ( entk->count3 < entk->tmp_time ) {
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			COM_StopRadio( entk ) ;
			entk->think3 = TH3_RADIOBREAK ; 
			entk->count3 = 0 ;
			return ;
		}
//indistinct との行ったり来たり防止
//		GM_SetNoise( NOISE_SCREAM , &entk->act->ctrl->mov, entk->act->ctrl->map ) ;
	}

	if ( entk->count3 == entk->tmp_time ) {
		entk->act->pad = SP_RETURNRADIO ;
		SetAlertParam( entk ) ;
	}

	if ( entk->act->act_end == 1 ) {
		entk->c_notice = ENE_NOTICE_NONE ;
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_RadioBreak( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RADIOCALL_BREAK ;
	}

	if ( entk->count3 == COUNT_VMODE(115) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_RADIO_BREAK ) ;
	}

	if ( entk->act->act_end == 1 ) {
		if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
		} else if ( entk->iknow_flag & IKNOW_NPC_POS ) {
			entk->think3 = TH3_NPC_ATTACK_SETUP ; 
		} else {
			entk->think3 = TH3_ATTACK_SETUP ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_IndirectDiscoveryPose( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}
/*sigeno*/
	if ( entk->count3 > COUNT_VMODE(30) ) {
		entk->act->pad = SP_DISCOVERY ;

		if ( entk->act->act_end ) {
			if ( ENE_PureAttacker( entk->act ) ) {
				if (GM_AlertMode!=ALERT_MODE_AVOID ) {
					COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
				}
				entk->c_notice = ENE_NOTICE_NONE ;
				/* まだプレイヤー見つけていない */
				COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
				entk->alert = MAX_ALERT_LEVEL ;
				entk->think3 = TH3_WAIT ; 
				entk->count3 = 0 ;
				return ;
			}

			if ( entk->status & ENE_STATUS_LONELY_FIGHT ) {
#ifdef LONLY_FIGHT_IS_NO_RADIO
				if (GM_AlertMode!=ALERT_MODE_AVOID ) {
					COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
				}
				entk->c_notice = ENE_NOTICE_NONE ;
				/* まだプレイヤー見つけていない */
				COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
				entk->alert = MAX_ALERT_LEVEL ;
				entk->think3 = TH3_WAIT ; 
				entk->count3 = 0 ;
#else
				entk->c_notice = ENE_NOTICE_NONE ;
				entk->think3 = TH3_WAIT ; 
				entk->count3 = 0 ;
#endif
				return ;
			} else if ( entk->iknow_flag & IKNOW_RADIO_BREAK  ||
				(entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
				ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
				entk->think3 = TH3_MOVE_WAITAREA ; 
				/* まだプレイヤー見つけていない */
				COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
			} else {
				/* まだプレイヤー見つけていない */
				COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
				out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
				if ( out != NULL && dis < entk->sense.hearing  && 
						( COM_GetStrmLv( ) < STR_LEVEL_2 ) ) {
					entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
				} else {
					entk->thk_status |= THK_STATUS_RADIO ;
					entk->think3 = TH3_RADIOSET ; 
printf("ENE_GameStatus = %x\n",ENE_GameStatus ) ;
					if ( ENE_GameStatus & ENE_GMSTATUS_PLAYER_FOUND ) {
//						entk->tmp_buff[0] = EV_RAD_FIND_PL ;
						entk->tmp_buff[0] = EV_RAD_FIND_PL_UNEXP ;
					} else {
						entk->tmp_buff[0] = EV_RAD_FIND_PL_FIRST ;
					}
				}
			}
			entk->count3 = 0 ;
			return ;
		}

		entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
	}

	entk->count3 ++ ;
}

static void Think3_BombDiscoveryPose( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_DISCOVERY ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}

	if ( entk->act->act_end ) {
		entk->c_notice = ENE_NOTICE_NONE ;
		/* まだプレイヤー見つけていない */
		COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
		entk->alert = MAX_ALERT_LEVEL ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;

	entk->count3 ++ ;
}

static void Think3_NearBombDiscoveryPose( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_SURPRISE_BOMB ;
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			GM_SeSetMode( SD_E_BIKKRI01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			COM_DetectVibration( ) ;
		}
	}

	if ( entk->act->act_end ) {
		entk->c_notice = ENE_NOTICE_NONE ;
		/* まだプレイヤー見つけていない */
		COM_SetPlayerLastPos( &(entk->trgpoint.pos),entk->trgpoint.map  ) ;/*ノイズの場所*/
		entk->alert = MAX_ALERT_LEVEL ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_MoveWaitArea( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			entk->think3 = TH3_MOVE_WAITAREA_DIRECT ;
			entk->count3 = 0 ;
		} else {
			entk->think3 = TH3_WAIT_AREA ; 
			entk->count3 = 0 ;
		}

		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
			!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
			!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
			entk->think3 = TH3_ATTACK_MOVE_WAITAREA ;
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveWaitAreaDirect( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 250 ) < 0 ) {
		entk->think3 = TH3_WAIT_AREA ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}



static	void	Think3_WaitArea( entk )
ENETHINK	*entk ;
{
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->act->pad = SP_UNREAL ;
	}

	if ( entk->count3 == 0 ) {
		UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;
		ENE_ClearPDamage( entk->act ) ;
		entk->tmp_time = COM_SetRadio( EV_RAD_DAMAGE, entk ) ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV1_STRM_LENGTH ;
		}
	}

	if ( entk->count3 == entk->tmp_time ) {
		COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
		SET_FLAG( entk->iknow_flag, IKNOW_DETECT_SEARCH ) ;
//ここで終了待ち
//		entk->think3 = TH3_WAIT ; 
//		entk->count3 = 0 ;
//		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->thk_status |= THK_STATUS_RADIO ;
		entk->think3 = TH3_RADIOSET ; 
#ifdef JIK_ONSE
		if ( entk->tmp_buff[0] != EV_RAD_FIND_PL_UNEXP ) {
printf("ENE_GameStatus = %x\n",ENE_GameStatus ) ;
			if ( ENE_GameStatus & ENE_GMSTATUS_PLAYER_FOUND ) {
				entk->tmp_buff[0] = EV_RAD_FIND_PL ;
			} else {
				entk->tmp_buff[0] = EV_RAD_FIND_PL_FIRST ;
			}
		}
#else
		entk->tmp_buff[0] = EV_RAD_FIND_PL_UNEXP ;
#endif
		entk->count3 = 0 ;
		return ;
	}

#ifdef JIK_RUN_ATTACK
	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
		!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) &&
		( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) ) {
		entk->think3 = TH3_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}
#endif
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static	void	Think3_AttackMoveWaitArea( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) ? SP_KERI : SP_NEARATK ;
	}

	if ( (entk->act->act_end) || (entk->count3 > 120) ) {
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_AttackNear( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) ? SP_KERI : SP_NEARATK ;
	}

	if ( entk->act->act_end ) {
#if 1	// 11.15
		if ( entk->status & ENE_STATUS_LONELY_FIGHT ) {
#ifdef LONLY_FIGHT_IS_NO_RADIO
			if (GM_AlertMode!=ALERT_MODE_AVOID ) {
				COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
			}
			entk->c_notice = ENE_NOTICE_NONE ;
			SetAlertParam( entk ) ;
			entk->think3 = TH3_WAIT ; 
			entk->count3 = 0 ;
			return ;
#else
			entk->c_notice = ENE_NOTICE_NONE ;
			entk->think3 = TH3_WAIT ; 
#endif
		} else if ( entk->iknow_flag & IKNOW_RADIO_BREAK ) {
			ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
			entk->think3 = TH3_MOVE_WAITAREA ; 
		} else {
			int	z ;
			z = ENE_RunawayZone( entk->ctrl, GM_PlayerControl ) ;
			ENE_SetTrgpZone( &(entk->trgpoint), z, entk->ctrl->hzx_id ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		}
		entk->act->move_s = MoveRun ;
#else
		entk->think3 = TH3_ATTACK_SETUP ; 
#endif
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
#if 0
		int	z ;
		z = ENE_RunawayZone( entk->ctrl, GM_PlayerControl ) ;
		ENE_SetTrgpZone( &(entk->trgpoint), z, entk->ctrl->hzx_id ) ;
		entk->think3 = TH3_MOVE_SAFEAREA ; 
		entk->act->move_s = MoveRun ;
		entk->count3 = 0 ;
#else
		entk->think3 = TH3_ATTACK_RELOAD ; 
		entk->count3 = 0 ;
#endif
		return ;
	}

	if ( !(entk->count3 % 4) ) {
		if ( entk->count3>6 && !(BP_PS2_rand()%3) ) {
			if ( entk->iknow_flag & IKNOW_RADIO_BREAK ) {
				/* 仲間が見かけたら、代わりに無線連絡してもらえるように */
				SET_FLAG( entk->iknow_flag, IKNOW_HELP_ME ) ;
				ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
				entk->think3 = TH3_MOVE_WAITAREA ; 
			} else {
#if 1
				int	z ;
				z = ENE_RunawayZone( entk->ctrl, GM_PlayerControl ) ;
				ENE_SetTrgpZone( &(entk->trgpoint), z, entk->ctrl->hzx_id ) ;
#else
//				ENE_SetTrgpSafeArea( &(entk->trgpoint), entk->ctrl->addr&255, entk->ctrl->hzx_id ) ;
#endif
				entk->think3 = TH3_MOVE_SAFEAREA ; 
			}
			entk->act->move_s = MoveRun ;
			entk->count3 = 0 ;
			return ;
		}
#if 0
		entk->act->pad = SP_MGUN_SHOT ;
#else
/**retouched sigeno 2002.08.29**/
		if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
			if(entk->count3 == 0){
				entk->act->pad = SP_MGUN_SHOT ;
			}else {
				entk->act->pad = SP_READYGUN ;
			}
		}else {
			entk->act->pad = SP_MGUN_SHOT ;
		}
#endif
		entk->bullet ++ ;
	} else {
		entk->act->pad = SP_READYGUN ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_AttackReload( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}


	if ( entk->act->act_end ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}


static void Think3_Watch( entk )
ENETHINK	*entk ;
{
	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->pad = SP_READYGUN ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->count3 > COUNT_VMODE(15) ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}
	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}

/* 全てはここからスタート */
static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{

	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->pad = SP_READYGUN ;
	}

	if ( entk->count3 == COUNT_VMODE(5) ) {
		if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
			if ( entk->status & ENE_STATUS_LONELY_FIGHT ) {
#ifdef LONLY_FIGHT_IS_NO_RADIO
				if (GM_AlertMode!=ALERT_MODE_AVOID ) {
					COM_SetRadio( EV_RAD_FIND_PL_AT, entk ) ;
				}
				SetAlertParam( entk ) ;
				entk->c_notice = ENE_NOTICE_NONE ;
				entk->alert = MAX_ALERT_LEVEL ;
				entk->think3 = TH3_WAIT ; 
#else
				entk->c_notice = ENE_NOTICE_NONE ;
				entk->think3 = TH3_WAIT ; 
#endif
			} else if ( entk->iknow_flag & IKNOW_RADIO_BREAK ||
				(entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
				/* 仲間が見かけたら、代わりに無線連絡してもらえるように */
				SET_FLAG( entk->iknow_flag, IKNOW_HELP_ME ) ;
				ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
				entk->think3 = TH3_MOVE_WAITAREA ; 
			} else {
				int	z ;
				z = ENE_RunawayZone( entk->ctrl, GM_PlayerControl ) ;
				ENE_SetTrgpZone( &(entk->trgpoint), z, entk->ctrl->hzx_id ) ;
				entk->think3 = TH3_MOVE_SAFEAREA ; 
			}
			entk->act->move_s = MoveRun ;
			entk->count3 = 0 ;

			return ;
		}
	}

	if ( entk->count3 > COUNT_VMODE(6) ) {
		if ( (entk->iknow_flag & IKNOW_RADIO_BREAK) ||
			 ((entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS) ) {
			/* 仲間が見かけたら、代わりに無線連絡してもらえるように */
			SET_FLAG( entk->iknow_flag, IKNOW_HELP_ME ) ;
			ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
			entk->think3 = TH3_MOVE_WAITAREA ; 
		} else if ( (entk->act->bodyp.pbreak & PBREAK_ARM_R) ||
			( entk->bullet >= MAX_MAGAZINE_MG ) ) {
			int	z ;
			z = ENE_RunawayZone( entk->ctrl, GM_PlayerControl ) ;
			ENE_SetTrgpZone( &(entk->trgpoint), z, entk->ctrl->hzx_id ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		}  else {
			entk->think3 = TH3_ATTACK_MGUN ;
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
		!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
		!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) &&
		( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) ) {
		entk->think3 = TH3_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}

	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}


static	void	Think3_SupportAttackNear( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) ? SP_KERI : SP_NEARATK ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static	void	Think3_SupportAttackMgun( entk )
ENETHINK	*entk ;
{
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		entk->think3 = TH3_SUPPORT_ATTACK_RELOAD ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( !(entk->count3 % 4) ) {
		if ( entk->count3>6 && !(BP_PS2_rand()%3) ) {
			entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
		entk->act->pad = SP_MGUN_SHOT ;
		entk->bullet ++ ;
	} else {
		entk->act->pad = SP_READYGUN ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_SupportAttackReload( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_SUPPORT_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}
	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}


static void Think3_SupportReadyGun( entk )
ENETHINK	*entk ;
{
	ENETHINK	*out ;
	int 		dis ;

	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->pad = SP_READYGUN ;
	}

	if ( (COM_GetStrmLv( ) < STR_LEVEL_2) || (COM_GetStrmCount( ) < 0) ) {
		out = COM_NearEnemyThkStatus( entk, THK_STATUS_RADIO, &dis ) ;
		if ( out != NULL && dis < entk->sense.hearing ) {
		} else {
			entk->thk_status |= THK_STATUS_RADIO ;
			entk->think3 = TH3_RADIOSET ; 
printf("ENE_GameStatus = %x\n",ENE_GameStatus ) ;
			if ( ENE_GameStatus & ENE_GMSTATUS_PLAYER_FOUND ) {
				entk->tmp_buff[0] = EV_RAD_FIND_PL ;
			} else {
				entk->tmp_buff[0] = EV_RAD_FIND_PL_FIRST ;
			}
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && !(GM_PlayerStatus & PLAYER_GROUND) ) {
		entk->think3 = TH3_SUPPORT_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(6) ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
				entk->think3 = TH3_SUPPORT_ATTACK_MGUN ;
				entk->count3 = 0 ;
				return ;
			}
		}
	}
	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}

static void Think3_WaitDiscovery( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SetAlertParam( entk ) ;
	} else {
		/* 念のため何か処理を入れておいた方がいいかも */
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

/* NPCへの攻撃 */
static void Think3_NpcReadyGun( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->pad = SP_READYGUN ;

	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(8) ) {
		if ( entk->npc_eyei.sight != EYE_INFO_SIGHT_IN ) {
			entk->think3 = TH3_NPC_ZONE_CHASE ;
			entk->count3 = 0 ;
			return ;
		}
		if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
			/* けがしてたら治す */
			entk->think3 = TH3_NPC_MEDICAL ;
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( !(GM_PlayerStatus & PLAYER_DEAD) ) {
#if 0
		if ( entk->npc_eyei.dis < NEAR_ATK_DIS ) {
			entk->think3 = TH3_NPC_ATTACK_NEAR ;
			entk->count3 = 0 ;
			return ;
		}
#endif
		if ( entk->count3 > COUNT_VMODE(60) ) {
			entk->think3 = TH3_NPC_ATTACK_MGUN ;
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

#define SHOT_BRANK (4)	/*ＰＡＬでも同じにする*/
static	void	Think3_NpcAttackMgun( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		if ( entk->act->bodyp.pbreak & PBREAK_ARMS ) {	/*片腕怪我していたら */
			/* けがしてたら治す */
			entk->think3 = TH3_NPC_MEDICAL ;
			entk->count3 = 0 ;
			return ;
		} else {
			entk->think3 = TH3_NPC_ATTACK_RELOAD ; 
			entk->count3 = 0 ;
		}
		return ;
	}

	if ( !(entk->count3 % SHOT_BRANK ) ) {
		if ( !(BP_PS2_rand()%(SHOT_BRANK-1)) ) {
			entk->act->pad = SP_READYGUN ;
			entk->think3 = TH3_NPC_ATTACK_SEARCH ; 
//			entk->think3 = TH3_NPC_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
		entk->act->pad = SP_MGUN_SHOT ;
		entk->bullet ++ ;
	} else {
		entk->act->pad = SP_READYGUN ;
	}

	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_NpcAttackReload( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( entk->count3 == 0 ){
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_NPC_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_NpcAttackSearch( entk )
ENETHINK	*entk ;
{
//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
//	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->act->pad = SP_READYGUN ;

	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->think3 = TH3_NPC_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ZoneChaseNpc( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_NPC_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	if ( entk->npc_eyei.sight >= EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_NPC_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseNpc( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;

	if ( entk->iknow_flag & IKNOW_PLAYER_POS ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 500 ) < 0 ) {
//		entk->think3 = TH3_NPC_ATTACK_NEAR ; 
		entk->think3 = TH3_NPC_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &entk->znavi->flore_pos, -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	if ( ENE_DirectReachCheck( entk ) == 0 ) {
		entk->think3 = TH3_NPC_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_NPC_ATTACK_SETUP ;
		entk->count3 = 0 ;

		return ;
	}

	
	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_NpcMedical( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_MEDICATION ;

	if ( (entk->act->act_end) || entk->count3 > 500 ) {
		entk->think3 = TH3_NPC_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

/*-----  --------------------------------------------*/
/*-----  --------------------------------------------*/
void THK_Discovery( entk )
ENETHINK	*entk ;
{
	/* 発見中フラグＯＮ */
	if ( entk->think3 != TH3_DISCOVERY_DELAY ) {
		COM_SetFlameFlag( CMFLAG_DETECT ) ;
	}

	switch ( entk->think3 ) {
	    case TH3_DISCOVERY_DELAY :
	    	Think3_DiscoveryDelay( entk ) ;
		break ;
	    case TH3_DISCOVERY_POSE_AVOID :
	    	Think3_DiscoveryPoseAvoid( entk ) ;
		break ;
	    case TH3_DISCOVERY_POSE :
	    	Think3_DiscoveryPose( entk ) ;
		break ;
	    case TH3_DISCOVERY_NO_POSE :
	    	Think3_DiscoveryNoPose( entk ) ;
		break ;

	    case TH3_NPC_DISCOVERY_POSE :
	    	Think3_NpcDiscoveryPose( entk ) ;
		break ;
	    case TH3_NPC_ATTACK_SETUP :
	    	Think3_NpcReadyGun( entk ) ;
		break ;
	    case TH3_NPC_ATTACK_MGUN :
	    	Think3_NpcAttackMgun( entk ) ;
		break ;
	    case TH3_NPC_ATTACK_RELOAD :
	    	Think3_NpcAttackReload( entk ) ;
		break ;
	    case TH3_NPC_ATTACK_SEARCH :
	    	Think3_NpcAttackSearch( entk ) ;
		break ;
	    case TH3_NPC_ZONE_CHASE :			/* NPC追いかけ */
	    	Think3_ZoneChaseNpc( entk ) ;
		break ;
	    case TH3_NPC_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseNpc( entk ) ;
		break ;
	    case TH3_NPC_MEDICAL :			/* 同じゾーン内で追いかける */
	    	Think3_NpcMedical( entk ) ;
		break ;

		/*プレイヤーと絡むのでradio.cでは無理、自前で処理 */
	    case TH3_RADIOSET :
	    	Think3_RadioSet( entk ) ;
		break ;
	    case TH3_RADIO_CALL :
	    	Think3_RadioCall( entk ) ;
		break ;
	    case TH3_RADIOBREAK :
	    	Think3_RadioBreak( entk ) ;
		break ;
	    case TH3_RADIO_CALL_LOOKABOUT :
	    	Think3_RadioCallLookAbout( entk ) ;
		break ;

	    case TH3_INDIRECT_DISCOVERY_POSE :
	    	Think3_IndirectDiscoveryPose( entk ) ;
		break ;


	    case TH3_BOMB_DISCOVERY_POSE :
	    	Think3_BombDiscoveryPose( entk ) ;
		break ;
	    case TH3_NEARBOMB_DISCOVERY_POSE :
	    	Think3_NearBombDiscoveryPose( entk ) ;
		break ;
	    case TH3_ATTACK_SETUP :
	    	Think3_ReadyGun( entk ) ;
		break ;
	    case TH3_ATTACK_MGUN :
	    	Think3_AttackMgun( entk ) ;
		break ;
	    case TH3_ATTACK_RELOAD :
	    	Think3_AttackReload( entk ) ;
		break ;
	    case TH3_ATTACK_NEAR :
	    	Think3_AttackNear( entk ) ;
		break ;

	    case TH3_WATCH :
	    	Think3_Watch( entk ) ;
		break ;

	    case TH3_SUPPORT_ATTACK_SETUP :
	    	Think3_SupportReadyGun( entk ) ;
		break ;
	    case TH3_SUPPORT_ATTACK_MGUN :
	    	Think3_SupportAttackMgun( entk ) ;
		break ;
	    case TH3_SUPPORT_ATTACK_RELOAD :
	    	Think3_SupportAttackReload( entk ) ;
		break ;
	    case TH3_SUPPORT_ATTACK_NEAR :
	    	Think3_SupportAttackNear( entk ) ;
		break ;



	    case TH3_MOVE_WAITAREA :
			Think3_MoveWaitArea( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA_DIRECT :
			Think3_MoveWaitAreaDirect( entk ) ;
		break ;
	    case TH3_ATTACK_MOVE_WAITAREA :
			Think3_AttackMoveWaitArea( entk ) ;
		break ;
	    case TH3_MOVE_SAFEAREA :
			Think3_MoveSafeArea( entk ) ;
		break ;
	    case TH3_WAIT_AREA :
			Think3_WaitArea( entk ) ;
		break ;
	    case TH3_WAIT :
	    	Think3_WaitDiscovery( entk ) ;
	    break ;
	}
//	if ( GM_GameLevel == GM_LEVEL_EXTREME ) {
	if ( GM_Configuration & GM_CONFIG_END_IF_FOUND ) {
		SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_PLAYER_POS ) ;
	}
	if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_NPC_POS ) ;
	}
}

/*----- スタートモード、発見モード --------------------------------------------*/
void	THK_DiscoveryModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = DiscoveryModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_DISCOVERY ;

printf("THK_DiscoveryModeStart aaaaaaaaaaaaaaaaaaaa iknow_detect[%x] [%d]\n",entk->iknow_flag, GV_Time ) ;

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think3 = TH3_WATCH ;
#ifdef JIK_ONSE
		entk->tmp_buff[0] = EV_RAD_FIND_PL_UNEXP ;
#endif
//		entk->think3 = TH3_ATTACK_SETUP ; 
	} else {
		entk->think3 = TH3_DISCOVERY_POSE ; 
		COM_DiscoverCount( ) ;
	}
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;

	entk->sense.status = RADAR_COLOR_RED ;

	/* プレイヤーを発見 */
	SET_FLAG( entk->iknow_flag, IKNOW_PLAYER_POS ) ;

//	if ( GM_GameLevel != GM_LEVEL_EXTREME ) {
	if ( !(GM_Configuration & GM_CONFIG_END_IF_FOUND) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;
	}

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	THK_DiscoveryNoiseModeStart( entk )
ENETHINK	*entk ;
{
	int power, dis ;

	power = COM_GetNoisePower( ) ;

	entk->act->CheckPad = DiscoveryModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	dis = _FVecTrgDis( &entk->ctrl->mov, &GM_NoisePosition ) ;

printf("THK_DiscoveryNoiseModeStart aaaaaaaaaaaaaaaaaaaa iknow_detect[%x] [%d]\n",entk->iknow_flag, GV_Time ) ;
	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_DISCOVERY ;
	switch( power ) {
		case NOISE_M :
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				entk->think3 = TH3_BOMB_DISCOVERY_POSE ; 
			} else {
				entk->think3 = TH3_INDIRECT_DISCOVERY_POSE ; 
			}
			break ;
		default :
			if ( dis < 8000 ) {
				entk->think3 = TH3_NEARBOMB_DISCOVERY_POSE ; 
			} else {
				entk->think3 = TH3_BOMB_DISCOVERY_POSE ; 
			}
			break ;
	}
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;

#ifdef GMDEF_NOISE_MAP_RENEW
	ENE_SetTrgpPoint( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseHzxID ) ;
#else
	ENE_SetTrgpPosMap( &(entk->trgpoint), &GM_NoisePosition, GM_NoiseMap ) ;
#endif

	entk->sense.status = RADAR_COLOR_RED ;

	// 11.15
	SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	THK_DiscoveryNpcModeStart( entk )
ENETHINK	*entk ;
{
	int power ;

	power = COM_GetNoisePower( ) ;

	entk->act->CheckPad = DiscoveryModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_DISCOVERY ;
	entk->think3 = TH3_NPC_DISCOVERY_POSE ; 

	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;

	ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;

	entk->sense.status = RADAR_COLOR_RED ;

	// 11.15
	SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;

	/* NPCを発見 */
	SET_FLAG( entk->iknow_flag, IKNOW_NPC_POS ) ;

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	THK_DiscoveryModeStartAvoid( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = DiscoveryModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->keep_pad = entk->act->pad = 0 ;

printf("THK_DiscoveryModeStartAvoid g[%d] id[%d] iknow[%x]\n",entk->g_id, entk->id, entk->iknow_flag);
	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_DISCOVERY ;

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think3 = TH3_DISCOVERY_NO_POSE ; 
	} else {
		entk->think3 = TH3_DISCOVERY_POSE_AVOID ; 
		COM_DiscoverCount( ) ;
	}
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;

	entk->sense.status = RADAR_COLOR_RED ;

	if ( !(GM_Configuration & GM_CONFIG_END_IF_FOUND) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_DETECT ) ;
	}

	/* プレイヤーを発見 */
	SET_FLAG( entk->iknow_flag, IKNOW_PLAYER_POS ) ;

	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	THK_DiscoveryModeStartAvoidDelay( entk )
ENETHINK	*entk ;
{
	THK_DiscoveryModeStartAvoid( entk ) ;
	entk->think3 = TH3_DISCOVERY_DELAY ; 
}
