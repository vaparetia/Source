//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkhold.c
	プレイヤー発見モード
	
	1999/10/14 Y.Korekado
	$Id: thkhold.c,v 1.1.1.3 2002/11/19 11:44:21 Yoshizawa1 Exp $
	
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
	TH3_WAIT, TH3_DISCOVERY,TH3_HOLDUPSLOW,TH3_HOLDUP,TH3_HOLDDOWN,
	TH3_RADIOSET, TH3_RADIOBREAK, TH3_RADIOCALL,TH3_MOVE_WAITAREA, TH3_MOVE_WAITAREA_DIRECT,
	TH3_REPAIR, TH3_NO_RADIO, /* radio.c */
	TH3_MEDICALCALL,TH3_MOVE_SAFEAREA, TH3_DURESS, TH3_END, TH3_PUTITEM,
	TH3_MOVE_WAITAREA2, TH3_WATCH, TH3_KYOTON, TH3_ATTACK_HOLDUP, TH3_SEARCH,
	TH3_BLUFF, TH3_HOLD_ATTACK, TH3_ATTACK_MGUN
} ;
enum {
	SP_NONE,
	SP_DISCOVERY,	/* びっくり */
	SP_READYGUN,	/* 銃構える */
	SP_HOLDUPSLOW,	/* ホールドアップポーズスロー */
	SP_HOLDUP,		/* ホールドアップポーズ */
	SP_HOLDDOWN,	/* ホールドアップゆっくり解除 */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_RETURNRADIO,	/* 無線しまう */
	SP_RADIOCALL_BREAK,	/* 故障した無線で連絡 */
	SP_RADIOCALL_ATTACKER,	/* 攻撃兵無線連絡 */
	SP_UNREAL,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_MEDICATION,	/* 治療 */
	SP_DURESS,		/* 強迫 */
	SP_PUTITEM,		/* アイテム出し */
	SP_ATTACKHOLDUP,	/* 攻撃ホールドアップ */
	SP_BLUFF,		/* ブラフ */
	SP_HOLD_ATTACK,	/* ホールドアップからカウンター */
	SP_MGUN_SHOT,
} ;


#define HOLD_UP_ATTCK_COUNT (3)
/*-------------------------------------------------------------*/
extern void *NewDiscovery(FMATRIX *world) ;
/*----- アクション --------------------------------------------*/
static	void ActHoldUpSlow( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, EM_idle_holdup, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP ) ;
		act->sw->eye_anim = EYE_BIKKIURI ;/* 見開き */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
				return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActHoldUp( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
#if 1
		ENE_SetActionPBreak( act, 0, EM_idle_holdup, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP ) ;
#else
		ENE_SetActionPBreak( act, 0, EM_idle_holdup, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
#endif

		if ( act->dogtag_item.c_proc == 0 ) act->sw->dogtag = SW_FLAG_VISIBLE ;
		act->sw->eye_anim = EYE_BIKKIURI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		act->sw->dogtag = 0 ;

		act->c_motion_num[1] = -1 ;

		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		act->sw->eye_anim = EYE_NORMAL ;
		act->sw->dogtag = 0 ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

}

void ActHoldDown( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_HOLD_UP ) ;

	if ( time < COUNT_VMODE(95) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_BIKKIURI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

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
		act->sw->eye_anim = EYE_BIKKIURI ;/* 見開き */
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
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_NORMAL ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActDuress( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
#if 1
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP_BASE ) ;
#else
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
#endif
		act->sw->eye_anim = EYE_DAMAGE ;/* 見開き */
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
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_NORMAL ;
		AT_SetMode( act, ActHoldUp ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static	void ActPutItem( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
#if 1
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP_BASE ) ;
#else
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
#endif
		act->sw->eye_anim = EYE_BIKKIURI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
				return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			act->sw->eye_anim = EYE_NORMAL ;
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActAttackHoldUp( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_BIKKIURI ;/* 見開き */
	}

	if ( (time > COUNT_VMODE(38)) && (time < COUNT_VMODE(167)) ) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
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
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_NORMAL ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActTngAttackHoldUp( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_BIKKIURI ;/* 見開き */
	}

	if ( time == COUNT_VMODE(12) ) {
		static FVECTOR force = { 0.0F,-100.0F,100.0F } ;
		static FVECTOR size = { 300.0F,300.0F,300.0F } ;

		ENE_SetOffenseTarget3( act, WP_KICK, HUMAN21_HIDARI_TSUMASAKI, &size, &force) ;
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
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_NORMAL ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActBluff( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_GUN_FREE|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP_BASE ) ;
		act->sw->eye_anim = EYE_MIHIRAKI ;/* 見開き */
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_NORMAL ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		act->sw->eye_anim = EYE_NORMAL ;
		if ( act->CheckPad( act ) ) {
			return ;
		}
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

/*
	if ( act->c_motion_num[0] == EM_idle_holdup_surprise ) {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			ENE_SetActionPBreak( act, 0, EM_idle_holdup, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_HOLDUP_BASE ) ;
		}
	}
*/

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

#define HOLD_ATTACK_CANCEL_MOTION_TIME (125)
static void ActHoldAttack( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_HOLD_UP ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, TIME_BASE*HOLD_ATTACK_CANCEL_MOTION_TIME,
		 MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->sw->eye_anim = EYE_BIKKIURI ;/* 見開き */
		GV_SetActorChild( act->w, NewDiscovery( &BODYWORLD(act->body, HUMAN21_ATAMA) ) ) ;
	}

	if ( (time < COUNT_VMODE(167-HOLD_ATTACK_CANCEL_MOTION_TIME)) ) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
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
		act->act_end = 1 ;
		act->sw->eye_anim = EYE_NORMAL ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}


	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActMgunShot( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_ShootBullet( act, ENE_BULLET_NORMAL|ENE_BULLET_NOWALL , &act->aim_pos ) ;
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU ) ;
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
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/*----- アクション --------------------------------------------*/
static int	HoldUpModeCheckPad( act )
ACTION	*act ;
{
	int	mot ;
	
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_DISCOVERY :
			AT_SetModeFromPad( act, ActDiscovery, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_HOLDUPSLOW :
			AT_SetModeFromPad( act, ActHoldUpSlow, EM_idle_holdup, act->pad ) ;
		break ;
		case SP_HOLDUP :
			AT_SetModeFromPad( act, ActHoldUp, EM_idle_holdup, act->pad ) ;
		break ;
		case SP_HOLDDOWN :
			AT_SetModeFromPad( act, ActHoldDown, EM_idle_holdup2idle, act->pad ) ;
		break ;
		case SP_MEDICATION :
			AT_SetModeFromPad( act, ENE_ActMedication, EM_legl_morph_legl, act->pad ) ;
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

		case SP_DURESS :
			AT_SetModeFromPad( act, ActDuress, EM_idle_holdup_surprise, act->pad ) ;
		break ;
		case SP_PUTITEM :
			AT_SetModeFromPad( act, ActPutItem, EM_idle_holdup_item, act->pad ) ;
		break ;
		case SP_ATTACKHOLDUP :
			if ( act->bodyp.type & ENE_TYPE_HITECH ) {
				AT_SetModeFromPad( act, ActTngAttackHoldUp, EM_tng_p90_nom_holdup_counterattack, act->pad ) ;
			} else {
				AT_SetModeFromPad( act, ActAttackHoldUp, EM_ak_nom_holdup_counter, act->pad ) ;
			}
		break ;
		case SP_BLUFF :
			AT_SetModeFromPad( act, ActBluff, EM_idle_holdup_bend, act->pad ) ;
		break ;
		case SP_HOLD_ATTACK :
			AT_SetModeFromPad( act, ActHoldAttack, EM_ak_nom_holdup_counter, act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActReadyGun, mot, act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ActMgunShot, mot, act->pad ) ;
		break ;
	}

	return 1 ;
}
/*----- --------------------------------------------*/

#define COUNTER_DIS	(4000)
static int CounterChance( ENETHINK	*entk )
{

	if ( GM_GameStatus & GM_STATUS_DETECT ) return 1 ;

	if ( entk->notice & ENE_NOTICE_BOX ) return 2 ;

	if ( entk->iknow_flag & IKNOW_HOLD_PLAYER ) {
#if 1	//発見にする 
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) return 2 ;
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT_HZD ) return 2 ;
#else
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) return 1 ;
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT_HZD ) return 1 ;
#endif
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		if ( entk->pl_eyei.dis > COUNTER_DIS ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_None ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_Mic ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_Magazine ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_Book ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_ChaffGrenade ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_StunGrenade ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_ColdSpray ) return 1 ;
		if ( PL_GetPlayerWeapon() == WP_Grenade ) return 1 ;
	}
#if 0
	/* プレイヤーが近くで走ったらカウンター（初体験版での仕様)*/
	if ( entk->pl_eyei.dis > COUNTER_DIS-2000 &&
		entk->pl_eyei.dis < COUNTER_DIS && 
		(GM_PlayerStatus & PLAYER_DASH) ) {
		return 1 ;
	}
#endif
	if ( entk->pl_eyei.dis < COUNTER_DIS && COM_GetNoisePower( ) == NOISE_DEC ) return 1 ;

	return 0 ;
}
/*----- 思考 --------------------------------------------*/
#include	"notice_radio.c"


static	void	Think3_Watch( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
	}

	if ( entk->count3 > COUNT_VMODE(8) ) {
		SetThinkRadioSet( entk, EV_RAD_HOLDUP, ENE_NOTICE_HOLDUP ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_Search( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QR ) ;
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
		SetThinkRadioSet( entk, EV_RAD_HOLDUP, ENE_NOTICE_HOLDUP ) ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 < COUNT_VMODE(3) ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_Discovery( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
		GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
		entk->act->pad = SP_DISCOVERY ;
		if ( !ENE_PureAttacker( entk->act ) ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
		}
	}

	if ( !ENE_PureAttacker( entk->act ) ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			SET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
		}
	}
	if ( entk->count3 > COUNT_VMODE(30) ) {
		if ( ENE_PureAttacker( entk->act ) ) {
			if ( entk->act->bodyp.type & ENE_TYPE_SHIELD || 
				( entk->act->bodyp.pbreak != 0 ) ) {
				entk->think3 = TH3_SEARCH ; 
			} else {
				entk->think3 = TH3_ATTACK_HOLDUP ; 
			}
		} else {
			COM_SetSpeak( EV_NOTICE_HOLDUP, entk ) ; /* ひー */
			entk->think3 = TH3_HOLDUPSLOW ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_HoldUpSlow( entk )
ENETHINK	*entk ;
{
	SET_FLAG( entk->thk_status, THK_STATUS_HOLD_UP ) ;

	entk->act->pad = SP_HOLDUPSLOW ;
	if ( entk->count3 == 0 ) {
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_HOLDUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

enum {
	NONE_DURESS ,
	NORMAL_DURESS ,
	HARD_DURESS ,
} ;
static int Duress( ENETHINK *entk )
{
	TARGET	*def ;
	long64	weapon ;
	int diff ;

	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) return NONE_DURESS ;

	diff = GV_DiffDirS( GM_PlayerControl->rot.vy, entk->ctrl->rot.vy ) ;
	if ( (GM_PlayerStatus & PLAYER_HOLD) ) {
		if ( (Ply_GetPlayerWeapon()==WP_Rgb6) ||
			 (Ply_GetPlayerWeapon()==WP_Nikita) ||
			 (Ply_GetPlayerWeapon()==WP_Stinger) ) {
#if 1	//JAPANESE_BP_IGNORE()から
			if ( (diff > 1536) || (diff < -1536) ) {
#else
			if ( (diff > 1536) || (diff < -1536) ) {
#endif
				return HARD_DURESS ;
			}
			else {
				printf("a+");
			}
		}
	}

	/* 体の近くをうったら */
#if 1	//JAPANESE_BP_IGNORE()から
	if ( (diff > 1336) || (diff < -1336) ) {
#else
	if ( (diff > 1736) || (diff < -1736) ) {
#endif
		if ( (GM_WeaponFire==WP_Rgb6) ||
			 (GM_WeaponFire==WP_Nikita) ||
			 (GM_WeaponFire==WP_Stinger) ||
//M9無し			 (GM_WeaponFire==WP_m92) ||
			 (GM_WeaponFire==WP_Usp) ||
			 (GM_WeaponFire==WP_Socom) ||
			 (GM_WeaponFire==WP_Psg1) ||
			 (GM_WeaponFire==WP_Aks) ||
			 (GM_WeaponFire==WP_m4) ||
			 (GM_WeaponFire==WP_Psg1T) ) {
			return HARD_DURESS ;
//			return NORMAL_DURESS ;
		}
	}

	def = &(entk->act->bodyp.def_child1[0]) ;/* 頭 */
	weapon = def->weapon_type ;
	def = &(entk->act->bodyp.def_child1[2]) ;/* ゴールド */
	weapon |= def->weapon_type ;
	if ( !(weapon & WP_LASERSIGHT) )	return NONE_DURESS ;

	return NORMAL_DURESS ;
}

static void Think3_Duress( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		COM_SetSpeak( EV_NOTICE_HELP, entk ) ; /* ひー */
		entk->act->pad = SP_DURESS ;
	}

	if ( ENE_PlayerHoldCheck( entk ) ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}

	if ( entk->act->act_end ) {
		entk->act->pad = SP_HOLDUP ;
		entk->think3 = TH3_PUTITEM ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_PutItem( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_PUTITEM ;

	if ( entk->count3 == COUNT_VMODE(30) ) {
		if ( entk->act->dogtag_id >= 0 && entk->act->dogtag_item.c_proc == 0 ) {
			KRTH_PutDogTagItemProc( &entk->act->dogtag_item, &entk->ctrl->mov, 
				entk->status, 1.0f, entk->act->dogtag_id ) ;
			entk->act->sw->dogtag = -1 ;
		} else {
			KRTH_PutItemProc( &entk->act->hold_item, &entk->ctrl->mov, entk->status, 1.0f ) ;
		}
	}

	if ( ENE_PlayerHoldCheck( entk ) ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}

	if ( entk->count3 > COUNT_VMODE(150) && !Duress( entk ) ) {
		entk->think3 = TH3_HOLDUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static void Think3_HoldUp( entk )
ENETHINK	*entk ;
{
	int duress ;

	SET_FLAG( entk->thk_status, THK_STATUS_HOLD_UP ) ;

	entk->act->pad = SP_HOLDUP ;

	if ( entk->count3 > COUNT_VMODE(100) ) {
		entk->think3 = TH3_HOLDDOWN ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( GM_GameLevel < GM_LEVEL_HARD ) {
		if ( ENE_PlayerHoldCheck( entk ) ) {
			entk->count3 = 0 ;
			UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
		} else {
			if ( entk->iknow_flag & IKNOW_HOLD_PLAYER ) {
				entk->count3 ++ ;
			}
		}
	} else {
		if ( ENE_PlayerHoldCheck( entk ) ) {
			entk->count3 = 0 ;
			UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
		} else {
			entk->count3 ++ ;
		}
	}

	if( (duress = Duress( entk )) ){
#if 0
		if ( (duress < HARD_DURESS) && 
			 ( entk->act->bodyp.pbreak == 0 ) ) {
			if ( entk->tmp_count >= HOLD_UP_ATTCK_COUNT ) {
				entk->think3 = TH3_HOLD_ATTACK ; 
			} else {
				entk->think3 = TH3_BLUFF ; 
			}
		} else {
			entk->think3 = TH3_DURESS ; 
		}
#else
		if ( (entk->status & ENE_STATUS_BLUFF) && (duress < HARD_DURESS) &&
			 ( entk->act->bodyp.pbreak == 0 ) ) {
			if ( entk->tmp_count >= HOLD_UP_ATTCK_COUNT ) {
				entk->think3 = TH3_HOLD_ATTACK ; 
			} else {
				entk->think3 = TH3_BLUFF ; 
			}
		} else {
			entk->think3 = TH3_DURESS ; 
		}
#endif
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
#if 0
		FMATRIX	*w ;

		entk->status2 |= ENE_STATUS2_AIM_FACE_Y|ENE_STATUS2_AIM_FACE_X ;
		w = &(BODYWORLD( GM_PlayerBody, 2 )) ;
		entk->act->aim_pos.vx = w->m[3][0] ;
		entk->act->aim_pos.vy = w->m[3][1] ;
		entk->act->aim_pos.vz = w->m[3][2] ;
#endif
		SET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}
}

static void Think3_HoldDown( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_HOLDDOWN ;

	if ( ENE_PlayerHoldCheck( entk ) ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
		/*もう一度「動くな！」*/
		GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
		entk->think3 = TH3_HOLDUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(120) ) {	/* モーションより短く！！ */
		if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
			SetThinkRadioSet( entk, EV_RAD_HOLDUP, ENE_NOTICE_HOLDUP ) ;
		} else {
			entk->think3 = TH3_WATCH ; 
		}
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}


static	void	Think3_Kyoton( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_HOLDUP ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}


	if ( entk->count3 > COUNT_VMODE(120) ) {
		SetThinkRadioSet( entk, EV_RAD_HOLDUP, ENE_NOTICE_HOLDUP ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}


static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		SetThinkRadioSet( entk, EV_RAD_HOLDUP, ENE_NOTICE_HOLDUP ) ;
		entk->count3 = 0 ;
		
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveWaitArea2( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_MEDICALCALL ;
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
		entk->act->pad = SP_MEDICATION ;
//	}

	if ( entk->act->act_end ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		entk->c_notice &= ~ENE_NOTICE_HOLDUP ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static void Think3_AttackHoldUp( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_ATTACKHOLDUP ;
	if ( entk->count3 == 0 ) {
	}

	if ( entk->act->act_end ==1 ) {
		entk->think3 = TH3_SEARCH ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->act->c_motion_num[0] == EM_tng_p90_nom_holdup_counterattack ) {
		if ( entk->count3 < COUNT_VMODE(14) ) {	/* モーション依存 */
			entk->act->dir = entk->pl_eyei.dir+2048 ;
		}
	} else {
		/* EM_ak_nom_holdup_counter */
		if ( entk->count3 > COUNT_VMODE(180) ) {	/* モーション依存 */
			entk->act->dir = entk->pl_eyei.dir+2048 ;
		}
	}

	entk->count3 ++ ;
}

static void Think3_HoldAttack( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_HOLD_ATTACK ;
	if ( entk->count3 == 0 ) {
	}

	if ( entk->act->act_end ==1 ) {
		entk->act->pad = SP_READYGUN ;
		entk->think3 = TH3_ATTACK_MGUN ;
		entk->count3 = 0 ;
		return ;
	}

	/* EM_ak_nom_holdup_counter */
	if ( entk->count3 > COUNT_VMODE(170-HOLD_ATTACK_CANCEL_MOTION_TIME) ) {	/* モーション依存 */
		entk->act->dir = entk->pl_eyei.dir+2048 ;
	}

	entk->count3 ++ ;
}

static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if ( !(entk->count3 % 4) ) {
		if ( entk->count3>15  ) {
			entk->think3 = TH3_WATCH ; 
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

static void Think3_End( entk )
ENETHINK	*entk ;
{
	entk->count3 ++ ;
	return ;
}

static void Think3_Bluff( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_BLUFF ;

	if ( entk->count3 == 0 ) {
		if ( entk->tmp_count == 0 ) {
			COM_SetRadio( EV_HOLDUP_BLUFF2, entk ) ;	/* 強気発言 */
		} else if ( entk->tmp_count == 2 ) {
			COM_SetRadio( EV_HOLDUP_BLUFF2+1, entk ) ;	/* 強気発言 */
		}
//		if ( ++entk->tmp_count >= 4 ) entk->tmp_count = 0 ;
		entk->tmp_count ++ ;
	}

	if ( ENE_PlayerHoldCheck( entk ) ) {
		UNSET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->iknow_flag, IKNOW_HOLD_PLAYER ) ;
	}

	if( Duress( entk ) >= HARD_DURESS ){
		COM_StopRadio( entk ) ;
		entk->think3 = TH3_DURESS ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(120) ) {
#if 1
		if( !Duress( entk ) ){
#else
		if ( (GV_DiffDirS( GM_PlayerControl->rot.vy, entk->ctrl->rot.vy ) < 1536) ||
			 !(GM_PlayerStatus & PLAYER_HOLD) ) {
#endif
			entk->act->pad = SP_HOLDUP ;
			entk->think3 = TH3_HOLDUP ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->count3 ++ ;
}

/*-----  --------------------------------------------*/
static void CounterCheck( ENETHINK	*entk )
{
	int cc ;

	if ( (cc = CounterChance( entk ))  ) {
		if( cc == 2 ) {
			/* 発見モードになるように */
			entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
		}
		entk->think3 = TH3_WATCH ; 
		entk->count3 = 0 ;
		return ;
	}

	/* 見えなくて距離が一定以上離れたら 変装見破り解除 */
	if ( entk->pl_eyei.sight < EYE_INFO_SIGHT_BLURR ) {
		if ( entk->pl_eyei.dis > COUNTER_DIS+100 ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
		}
	}
	
	if ( ENE_PlayerHoldCheck( entk ) ) {
		SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
	}
}

static void ReceiveCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->receive & ENE_ORDER_HOW_DO ) {
		entk->think3 = TH3_KYOTON ; 
		entk->count3 = 0 ;
		return ;
	}
}

static void KyotonSightCheck( entk )
ENETHINK	*entk ;
{
	/*近くに仲間がいるから反撃*/


	if ( !(GM_PlayerStatus & (PLAYER_STEALTH)) ) {
		if ( ENE_PlayerHoldCheck( entk ) ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			/* 発見モードになるように */
			entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
		}
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		/* 発見モードになるように */
		entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
	}
}

static void SightCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		/* 発見モードになるように */
		entk->alert = ENE_INDISTINCT_ALERT_LEVEL+10 ;
	}
}
/*-----  --------------------------------------------*/
void THK_HoldUpMode( entk )
ENETHINK	*entk ;
{
	if ( !ENE_PureAttacker( entk->act ) ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			/* 危険モードにならないように */
			entk->alert = ENE_INDISTINCT_ALERT_LEVEL ;
		}
	}

	switch ( entk->think3 ) {
	    case TH3_DISCOVERY :
	    	Think3_Discovery( entk ) ;
		break ;
	    case TH3_HOLDUPSLOW :
	    	Think3_HoldUpSlow( entk ) ;
			CounterCheck( entk ) ;
		break ;
	    case TH3_DURESS :
	    	Think3_Duress( entk ) ;
			CounterCheck( entk ) ;
		break ;
	    case TH3_PUTITEM :
	    	Think3_PutItem( entk ) ;
			CounterCheck( entk ) ;
		break ;

	    case TH3_HOLDUP :
	    	Think3_HoldUp( entk ) ;
			ReceiveCheck( entk ) ;
			CounterCheck( entk ) ;
		break ;
	    case TH3_HOLDDOWN :
	    	Think3_HoldDown( entk ) ;
			CounterCheck( entk ) ;
		break ;
	    case TH3_MOVE_SAFEAREA :
			Think3_MoveSafeArea( entk ) ;
			CounterCheck( entk ) ;
		break ;
	    case TH3_KYOTON :
	    	Think3_Kyoton( entk ) ;
	    	KyotonSightCheck( entk ) ;
		break ;

	    case TH3_WATCH :
	    	Think3_Watch( entk ) ;
			SightCheck( entk ) ;
		break ;

		/* radio.c */
	    case TH3_RADIOSET :
		    Think3_RadioSet( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_RADIOCALL :
		    Think3_RadioCall( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_RADIOBREAK :
		    Think3_RadioBreak( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA :
			Think3_MoveWaitArea( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA_DIRECT :
			Think3_MoveWaitAreaDirect( entk ) ;
		break ;
	    case TH3_REPAIR :
			Think3_Repair( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_NO_RADIO :
			Think3_NoRadio( entk ) ;
			SightCheck( entk ) ;
		break ;

	    case TH3_MOVE_WAITAREA2 :
			Think3_MoveWaitArea2( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_MEDICALCALL :
	    	Think3_Medical( entk ) ;
			SightCheck( entk ) ;
		break ;
	    case TH3_END :
	    	Think3_End( entk ) ;
			SightCheck( entk ) ;
	    break ;

	    case TH3_HOLD_ATTACK :
	    	Think3_HoldAttack( entk ) ;
		break ;
	    case TH3_ATTACK_MGUN :
	    	Think3_AttackMgun( entk ) ;
		break ;

	    case TH3_ATTACK_HOLDUP :
	    	Think3_AttackHoldUp( entk ) ;
		break ;
	    case TH3_SEARCH :
	    	Think3_Search( entk ) ;
		break ;

	    case TH3_BLUFF :
	    	Think3_Bluff( entk ) ;
			CounterCheck( entk ) ;
		break ;
	}
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_HoldUpModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = HoldUpModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_HOLDUP ;

	entk->think3 = TH3_DISCOVERY ; 
//	if ( !ENE_PureAttacker( entk->act ) ) {
		if ( entk->pl_eyei.dis < COUNTER_DIS ) SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
//	}

	entk->tmp_time = 0 ;
	entk->tmp_count = 0 ;
	entk->count3 = 0 ;

	entk->sense.status = RADAR_COLOR_YELOW ;
	/* ストリーミングここで停止 */
	COM_StopRadio( entk ) ;
}

void	THK_HoldUpModeStartDamage( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = HoldUpModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice = ENE_NOTICE_HOLDUP ;

	entk->think3 = TH3_HOLDUP ; 
	entk->sense.status = RADAR_COLOR_YELOW ;
	if ( entk->pl_eyei.dis < COUNTER_DIS ) SET_FLAG( entk->iknow_flag, IKNOW_DRESSED ) ;
	entk->tmp_time = 0 ;
	entk->tmp_count = 0 ;
	entk->count3 = 0 ;
	entk->act->move_s = MoveWalk ;
}
