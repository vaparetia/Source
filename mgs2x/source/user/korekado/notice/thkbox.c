//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	thkbox.c
	ダンボールモード
	trgpoint_cover を使用。他のモードからの２次モードとして扱う
	
	2000/08/09 Y.Korekado
	$Id: thkbox.c,v 1.1.1.3 2002/11/19 11:44:20 Yoshizawa1 Exp $
	
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

// 重野追加 攻撃兵の特殊兵装で射撃、リロードアクションを切り替える
#define ATTACKER_BOX

enum {
	TH3_WATCH,TH3_NSIGHT_ON,TH3_ZONETRACE,TH3_LOOKAROUND,TH3_END,
	TH3_WAIT, TH3_STOPSIGN, TH3_DIRECTTRACE,TH3_LOOKWATCH,TH3_PEEP,
	TH3_KICK, TH3_GET_BOX, TH3_ATTACK_SETUP, TH3_ATTACK_MGUN, TH3_ATTACK_RELOAD,
	TH3_CLE_WATCH,TH3_CAUTION_LOOK, TH3_NEAR,TH3_WAIT_OTHER_ACTION, TH3_LOST_SIGHT,
	TH3_BEFORE_WATCH,

	TH3_ZONE_MOVE,TH3_NEAR_MOVE, TH3_PINPOINT_MOVE, TH3_LOCKER_OPEN, /* for move.c */
} ;

enum {
	SP_NONE,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_TURN,		/* 方向転換 */
	SP_CLE_STOP_SIGN,
	SP_PEEP_BOX,
	SP_KICK,
	SP_GET_BOX,
	SP_READYGUN,
	SP_MGUN_SHOT,
	SP_RELOAD,

	SP_PINPOINT,	/* ピンポイント移動 move.c */
	SP_LOCKER_OPEN,		/* ロッカー開ける move.c */
#ifdef ATTACKER_BOX
	SP_SHOT_SPS,	/*ショットガン発射*/
#endif	
} ;

#define TENGU_NO_ATTACKER	(1)	//天狗は箱を持ち上げる
/*----- アクション --------------------------------------------*/
static	void ActGetBox( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		if ( act->keep_mot == EM_box_lift ) act->tmp_item = NULL ;
	}

	if ( act->CheckDamage( act ) ) {
		if ( act->tmp_item != NULL ){
			act->sw_tmp_item = 1 ;
			act->tmp_item = NULL ;
		}
		return ;
	}

	if ( act->CheckPad( act ) ) {
		if ( act->tmp_item != NULL ){
			act->sw_tmp_item = 1 ;
			act->tmp_item = NULL ;
		}
		return ;
	}

	if ( act->keep_mot == EM_box_lift ) {
		if ( time > COUNT_VMODE(150) ) {
			AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE ) ;
		}
		if ( time > COUNT_VMODE(30) ) {
			AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		}
		/* プレイヤーダンボール解除 */
		if ( time == COUNT_VMODE(150) ) {
			extern void *NewC_BoxLifted( CONTROL *, OBJECT *, int, FVECTOR *, int * ) ;
			FVECTOR	shift ;

			if ( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_CBBOX &&
				!(GM_PlayerStatus & PLAYER_CB_BOX_CANCELED) ) {
				act->sw_tmp_item = 0 ;
				shift.vx = 391.179f ;
//				shift.vy = -585.508f ;
				shift.vy = -485.508f ;
				shift.vz = 735.238f ;
				act->tmp_item = NewC_BoxLifted( act->ctrl, act->body,
					HUMAN21_MIGI_TE, &shift, &act->sw_tmp_item ) ;
			} else {
				act->act_end = 1 ;
				AT_SetMode( act, ENE_ActStandStill ) ;
				return ;
			}
		}
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->keep_mot = EM_box_lift_p ;
			act->time = 0 ;
		}
	} else if ( act->keep_mot == EM_box_lift_p ) {
		AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_GUN_FREE ) ;
		if ( time == COUNT_VMODE(30) ) {
			act->keep_mot = EM_box_lift2throw ;
			act->time = 0 ;
		}
	} else if ( act->keep_mot == EM_box_lift2throw ) {
		if ( time < COUNT_VMODE(40) ) {
			AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE ) ;
		}
		if ( time < COUNT_VMODE(54) ) {
			AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		}

		if ( time == COUNT_VMODE(30) ) {
			act->sw_tmp_item = 1 ;
			act->tmp_item = NULL ;
		}
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->act_end = 1 ;
			AT_SetMode( act, ENE_ActStandStill ) ;
			return ;
		}
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

static void ActKick( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
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
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	/* 覗き穴をこっちに向ける */
	if ( time == COUNT_VMODE(48) ) {
		FVECTOR	size = { 1000.0, 1000.0, 1000.0} ;
		ENE_SetOffenseTarget2( act, WP_BOXKICK, HUMAN21_KOSHI, &size ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		act->sw->eye_anim = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}
/*



*/

static void ActBoxPeep( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
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

	if ( act->keep_mot == EM_box_look ) {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->keep_mot = EM_box_look_idle ;
			act->time = 0 ;
		}
	}
	if ( act->keep_mot == EM_box_look_idle ) {
		if ( time == COUNT_VMODE(120) ) {
			act->keep_mot = EM_box_look2stand ;
			act->time = 0 ;
		}
	}
	if ( act->keep_mot == EM_box_look2stand ) {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->act_end = 1 ;
			act->sw->eye_anim = 1 ;
			AT_SetMode( act, ENE_ActStandStill ) ;
			return ;
		}
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

#ifdef ATTACKER_BOX

static void ActBoxStandReload( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	if ( act->CheckDamage( act ) ) {
		if( act->keep_mot==EM_tng_p90_nom_fire_reload){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		AT_SetMode( act, ENE_ActStandStill ) ;
		if( act->keep_mot==EM_tng_p90_nom_fire_reload){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		return ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if( act->keep_mot==EM_tng_p90_nom_fire_reload){
				act->sw->magg = 0 ;
			}else if(act->keep_mot != EM_shl_nom_mkr_reload){
				act->sw->magg = 1 ;
			}
			return ;
		}
	}

//  case 0:	/* 非表示 */
//  case 1:	/* 銃に接続されている（銃は手に付いていると仮定） */
//  case 2:	/* 左手にもっている */
//  case 3:	/* 右手にもっている */
//  case 4:	/* その場落下開始 */
//  case 5:	/* デストロイ */
	switch(act->keep_mot){
		case EM_gbs_sps_nom_fire_reload :
			if ( time == DIRECT_TICK(40) ) act->sw->magg = 2 ;/*temoti*/
			if ( time == DIRECT_TICK(69) ) act->sw->magg = 1 ;/*juutuke*/
			break ;
		case EM_tng_p90_nom_fire_reload :
			if ( time == DIRECT_TICK(19) ) act->sw->magg = 2 ;
			if ( time == DIRECT_TICK(75) ) act->sw->magg = 1 ;
			break ;
		case EM_htc_m4_nom_fire_reload :
			if ( time == DIRECT_TICK(24) ) act->sw->magg = 4 ; /*sute*/
			if ( time == DIRECT_TICK(130) ) act->sw->magg = 2 ;	/*temoti*/
			if ( time == DIRECT_TICK(181) ) act->sw->magg = 1 ;	/*juutuke*/
			break ;
		default :
			/* AK */
			if ( time == DIRECT_TICK(19) ) act->sw->magg = 4 ; /*sute*/
			if ( time == DIRECT_TICK(66) ) act->sw->magg = 2 ;	/*temoti*/
			if ( time == DIRECT_TICK(112) ) act->sw->magg = 1 ;	/*juutuke*/
			break ;
	}

	if( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		if( act->keep_mot==EM_tng_p90_nom_fire_reload){
			act->sw->magg = 0 ;
		}else if(act->keep_mot != EM_shl_nom_mkr_reload){
			act->sw->magg = 1 ;
		}
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}

static void ActBoxShotSPS( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	if ( time == 0 ) {
		int flag;
		flag = (ENE_BULLET_NORMAL|ENE_BULLET_NOLINE);
		ENE_ShootBullet( act,flag ,NULL ) ;
		*act->sw->bullet = *act->sw->bullet+1 ;
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND );
 		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,ACT_INTERP_M ) ;
	}

/*ショットガン排莢エフェクト*/
	if ( time == COUNT_VMODE(4) )	act->sw->wctrl2.flag = WPEF_FLG_START ;
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
	}
}

#endif

/*----- アクション --------------------------------------------*/
static int	BoxModeCheckPad( act )
ACTION	*act ;
{
	int		mot ;
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_CAUT_STAND :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, ENE_CleStandMotion(act), act->pad ) ;
		break ;
		case SP_NSIGHT_ON :
			AT_SetModeFromPad( act, ENE_ActNSight_ON, EM_gogle_on, act->pad ) ;
		break ;
		case SP_TURN :
			act->keep_pad = act->pad ;
			AT_SetMode( act, ENE_ActTurn ) ;
		break ;
		case SP_CLE_STOP_SIGN :
			AT_SetModeFromPad( act, ENE_ActCleSign, EM_cle_signe_stop, act->pad ) ;
		break ;
		case SP_PEEP_BOX :
			AT_SetModeFromPad( act, ActBoxPeep, EM_box_look, act->pad ) ;
		break ;
		case SP_KICK :
			AT_SetModeFromPad( act, ActKick, EM_box_kick, act->pad ) ;
		break ;
		case SP_GET_BOX :
			AT_SetModeFromPad( act, ActGetBox, EM_box_lift, act->pad ) ;
		break ;
		case SP_READYGUN :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ENE_ActReadyGun, mot, act->pad ) ;
		break ;
		case SP_MGUN_SHOT :
			mot = ENE_ReadyGunMotion(act) ;
			AT_SetModeFromPad( act, ActMgunShot, mot, act->pad ) ;
		break ;
		case SP_RELOAD :
#ifndef ATTACKER_BOX
			mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_fire_reload:EM_reload ;
			AT_SetModeFromPad( act, ENE_ActReload, mot, act->pad ) ;
#else 
			mot = ENE_ReloadMotion( act ) ;
			AT_SetModeFromPad( act, ActBoxStandReload, mot, act->pad ) ;
#endif
		break ;
#ifdef ATTACKER_BOX
		case SP_SHOT_SPS :
			mot = EM_gbs_sps_nom_fire ;
			AT_SetModeFromPad( act, ActBoxShotSPS, mot, act->pad ) ;
		break ;
#endif
	}
	return 1 ;
}
/*----- 補助 --------------------------------------------*/
static int DanjyouCheck( ENETHINK *entk )
{
	if( ENE_InZone( &GM_PlayerPosition, GM_PlayerAddress ) ) return 0 ;

	if ( ENE_ABSf( entk->ctrl->levels[0] - GM_PlayerControl->levels[0] ) < 900.0f ) return 0 ;

	return 1 ;
}

static int BOX_PureAttacker( ACTION *act )
{
	if ( (act->bodyp.type & ENE_TYPE_ATTACKER) &&
		!(act->bodyp.type & ENE_TYPE_CONVERT) &&
		!(act->bodyp.type & ENE_TYPE_EVENT_A) 
		) {
		if ( act->bodyp.type & ENE_TYPE_TNG_A ) {
			if ( GM_AlertMode != ALERT_MODE_AVOID ) {
				return 0 ;
			}
		}
		return 1 ;
	}
	return 0 ;
}

static int OtherEnemyBoxAction( ENETHINK *entk )
{
	ENETHINK	*other ;
	int dis ;

	other = COM_NearEnemyThkStatus( entk, THK_STATUS_CBOX_WATCH, &dis ) ;
	if ( other == NULL ) return 0 ;

	return 1 ;
}

/*----- 思考 --------------------------------------------*/
static	void	Think3_BoxWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
		COM_SetSpeak( EV_NOTICE_BOX, entk ) ; /* んっ */
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
	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint_cover.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_BoxWatchWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		if ( BOX_PureAttacker( entk->act ) ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->bullet = 0 ;	/* １マガジン分ぶっこむ */
		} else {
			ENE_SetTrgpPoint( &(entk->trgpoint_cover),
			&(entk->com->plpos_in_zone[PL_POS_CENTER]), entk->com->plmap_in_zone[PL_POS_CENTER] ) ;
//			entk->act->aim_pos = GM_PlayerPosition ;
			entk->act->aim_pos = GM_PlayerFindPos ;
			entk->think3 = TH3_ZONETRACE ; 
		}
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
		entk->think3 = TH3_ZONETRACE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_BoxMove( entk )
ENETHINK	*entk ;
{
	int intrpt ;

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if( ENE_InRange( &entk->ctrl->mov, &entk->trgpoint_cover.pos, BOX_CHECK_STAND_DIS ) ) {
		entk->think3 = TH3_BEFORE_WATCH ;
		entk->count3 = 0 ;
		return ;
	}

	intrpt = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint_cover), entk->ctrl, entk->count3 ) ;
	if ( intrpt < -1 ) {
		entk->think3 = TH3_BEFORE_WATCH ;
		entk->count3 = 0 ;
		return ;
	} else if (intrpt < 0 ) {
		entk->think3 = TH3_DIRECTTRACE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint_cover.dir ;
	entk->count3 ++ ;
}

static void Think3_BoxMoveDirect( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint_cover), &(entk->ctrl->mov), BOX_CHECK_STAND_DIS ) < 0 ) {		entk->think3 = TH3_BEFORE_WATCH ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint_cover.dir ;

	entk->count3 ++ ;
}


static	void	Think3_BeforeWatch( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 == 0 ) {
		entk->act->dir = entk->pl_eyei.dir ;
	}

	if ( entk->count3 > COUNT_VMODE(30) ) {
		if ( ENE_ZoneIntrptCheck( GM_PlayerAddress ) && 
			 ( entk->pl_eyei.sight_real == EYE_INFO_SIGHT_IN )	) {
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->bullet = 0 ;	/* １マガジン分ぶっこむ */
printf("box:shot sight\n");
		} else if ( DanjyouCheck( entk ) ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->bullet = 0 ;	/* １マガジン分ぶっこむ */
printf("box:danjyou shot sight\n");
		} else if ( entk->notice & ENE_NOTICE_NEAR_BOX ) {
printf("box:near look watch\n");
			entk->act->dir = entk->pl_eyei.dir ;
			entk->think3 = TH3_LOOKWATCH ;
		} else if ( (entk->pl_eyei.dis > (BOX_CHECK_STAND_DIS + 700)) || 
			( entk->pl_eyei.sight_real != EYE_INFO_SIGHT_IN ) ) {
			entk->think3 = TH3_LOST_SIGHT ;
printf("box:lost dis[%d] sight [%d] notice[%x]\n", entk->pl_eyei.dis, entk->pl_eyei.sight_real,entk->notice );
		} else {
printf("box:look watch\n");
			entk->act->dir = entk->pl_eyei.dir ;
			entk->think3 = TH3_LOOKWATCH ;
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}

	entk->count3 ++ ;
	return ;
}



static	void	Think3_LookWatch( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 == COUNT_VMODE(120) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	if ( entk->count3 > COUNT_VMODE(150) ) {
		if ( OtherEnemyBoxAction( entk ) ) {
			entk->think3 = TH3_WAIT_OTHER_ACTION ; 
		} else {
			entk->think3 = TH3_KICK ; 
		}
		entk->count3 = 0 ;
		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}
	entk->count3 ++ ;
	return ;
}

static	void	Think3_Kick( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_CBOX_WATCH ) ;

//	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_KICK ;
//	}

	if ( entk->count3 > 0 ) {
		if ( entk->act->act_end ) {
	//		if ( entk->iknow_flag & IKNOW_CLEARING ) {
			if ( BOX_PureAttacker( entk->act ) ) {
				entk->think3 = TH3_GET_BOX ; 
			} else {
				entk->think3 = TH3_PEEP ; 
			}
			entk->count3 = 0 ;
			return ;
		}
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_PeepBox( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_CBOX_WATCH ) ;

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_PEEP_BOX ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_GET_BOX ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_GetBox( entk )
ENETHINK	*entk ;
{
	/* 俺が調べる */
	SET_FLAG( entk->thk_status, THK_STATUS_CBOX_WATCH ) ;

	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_GET_BOX ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
#if 1
	if ( entk->count3 == COUNT_VMODE(30) ) {
		entk->c_notice &= ~ENE_NOTICE_BOX ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}
#else
	if ( entk->count3 == COUNT_VMODE(120) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		if(	GM_AlertMode != ALERT_MODE_AVOID ){
			COM_SetSpeak( EV_WHAT_BOX, entk ) ; /* 異常なしか */
		}
	}
	if ( entk->count3 == COUNT_VMODE(150) ) {
		entk->c_notice &= ~ENE_NOTICE_BOX ;
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


static	void	Think3_LostSight( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}
	if ( entk->count3 == COUNT_VMODE(150) ) {
		entk->c_notice &= ~ENE_NOTICE_BOX ;
		entk->think3 = TH3_END ; 
		entk->count3 = 0 ;
		return ;
	}


	entk->count3 ++ ;
	return ;
}

static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		if ( entk->bullet >= 1 ) {
			entk->think3 = TH3_ATTACK_RELOAD ; 
			entk->count3 = 0 ;
			return ;
		}
	} else {
		if ( entk->bullet >= entk->max_bullet ) {
			entk->think3 = TH3_ATTACK_RELOAD ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( !(entk->count3 % 4) ) {
#if 0
		if ( entk->count3>6 && !( KR_RandU(3) ) ) {
			entk->act->pad = SP_READYGUN ;
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
#endif
		if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
			entk->act->pad = SP_SHOT_SPS ;
		}else {
			entk->act->pad = SP_MGUN_SHOT ;
		}
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
		entk->bullet = 0 ;
		entk->act->pad = SP_RELOAD ;
	}

	if ( entk->act->act_end ) {
#if 1
		/* １マガジンで終了 */
		entk->think3 = TH3_CAUTION_LOOK ; 
#else
		/* ダンボールが破壊されるまで 撃ちつづける */
		entk->think3 = TH3_ATTACK_SETUP ; 
#endif
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( entk->count3 > COUNT_VMODE(6) ) {
#if 1
			entk->think3 = TH3_ATTACK_MGUN ;
			entk->bullet = 0 ;
#else
		if ( entk->notice & ENE_NOTICE_BOX ) {
			entk->think3 = TH3_ATTACK_MGUN ;
		} else {
			/* １マガジンで終了 */
			entk->think3 = TH3_CAUTION_LOOK ; 
		}
#endif
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_CautionLook( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
	}

	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->c_notice &= ~ENE_NOTICE_BOX ;
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


static	void	Think3_CleWatch( entk )
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

	entk->act->dir = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint_cover.pos) ) ;
	entk->act->pad = SP_TURN ;

	entk->count3 ++ ;
	return ;
}


static	void	Think3_Near( entk )
ENETHINK	*entk ;
{
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ;
	}

	if ( entk->count3 > COUNT_VMODE(150) ) {
		if ( OtherEnemyBoxAction( entk ) ) {
			entk->think3 = TH3_WAIT_OTHER_ACTION ; 
		} else {
			entk->think3 = TH3_KICK ; 
		}
		entk->count3 = 0 ;
		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}

	entk->count3 ++ ;
}

static	void	Think3_WaitOtherAction( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(300) ) {
		entk->c_notice &= ~ENE_NOTICE_BOX ;
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

static void OtherActionCheck( ENETHINK *entk )
{
	if ( OtherEnemyBoxAction( entk ) ) {
		entk->think3 = TH3_WAIT_OTHER_ACTION ; 
		entk->count3 = 0 ;
	}
}

static void CancelCheck( ENETHINK *entk )
{
	if ( (entk->pl_eyei.dis > (BOX_CHECK_STAND_DIS + 700)) || 
		( !(GM_PlayerStatus & PLAYER_CB_BOX) ) ) {
			entk->think3 = TH3_LOST_SIGHT ;
			entk->count3 = 0 ;
printf("box:loast sight dis[%d] notice[%x]\n", entk->pl_eyei.dis, entk->notice );
			return ;
	}

	/* ステルスつけられたらキャンセル */
	if ( GM_PlayerStatus & (PLAYER_STEALTH) ) {
			entk->think3 = TH3_LOST_SIGHT ;
			entk->count3 = 0 ;
printf("box:loast sight dis[%d] notice[%x]\n", entk->pl_eyei.dis, entk->notice );
			return ;
	}
}

static void GetBoxCancelCheck( ENETHINK *entk )
{
	if ( entk->count3 > COUNT_VMODE(130) ) return ;

	if ( (entk->pl_eyei.dis > (BOX_CHECK_STAND_DIS + 700)) || 
		( !(GM_PlayerStatus & PLAYER_CB_BOX) ) ) {
			entk->think3 = TH3_LOST_SIGHT ;
			entk->count3 = 0 ;
printf("box:loast sight dis[%d] notice[%x]\n", entk->pl_eyei.dis, entk->notice );
			return ;
	}

	/* ステルスつけられたらキャンセル */
	if ( GM_PlayerStatus & (PLAYER_STEALTH) ) {
			entk->think3 = TH3_LOST_SIGHT ;
			entk->count3 = 0 ;
printf("box:loast sight dis[%d] notice[%x]\n", entk->pl_eyei.dis, entk->notice );
			return ;
	}
}

/*-----  --------------------------------------------*/
void THK_BoxMode( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
			Think3_BoxWatch( entk ) ;
		break ;
	    case TH3_WAIT :
			Think3_BoxWatchWait( entk ) ;
		break ;
	    case TH3_STOPSIGN :
			Think3_StopSign( entk ) ;
		break ;
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;

	    case TH3_ZONETRACE :
			Think3_BoxMove( entk ) ;
			OtherActionCheck( entk ) ;
		break ;
	    case TH3_DIRECTTRACE :
			Think3_BoxMoveDirect( entk ) ;
			OtherActionCheck( entk ) ;
		break ;
	    case TH3_BEFORE_WATCH :
			Think3_BeforeWatch( entk ) ;
			OtherActionCheck( entk ) ;
		break ;


	    case TH3_LOOKWATCH :
		    Think3_LookWatch( entk ) ;
			CancelCheck( entk ) ;
		break ;
	    case TH3_KICK :
		    Think3_Kick( entk ) ;
			CancelCheck( entk ) ;
		break ;
	    case TH3_PEEP :
		    Think3_PeepBox( entk ) ;
			CancelCheck( entk ) ;
		break ;
	    case TH3_GET_BOX :
		    Think3_GetBox( entk ) ;
			GetBoxCancelCheck( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
		    Think3_LookAround( entk ) ;
		break ;
	    case TH3_LOST_SIGHT :
		    Think3_LostSight( entk ) ;
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
	    case TH3_CAUTION_LOOK :
		    Think3_CautionLook( entk ) ;
		break ;

		/* clearing  */
	    case TH3_CLE_WATCH :
			Think3_CleWatch( entk ) ;
		break ;
	    case TH3_NEAR :
		    Think3_Near( entk ) ;
		break ;
	    case TH3_WAIT_OTHER_ACTION :
			Think3_WaitOtherAction( entk ) ;
		break ;

	    case TH3_END :
		    Think3_End( entk ) ;
		break ;
	}

#if 1
	if ( entk->think3 != TH3_WATCH ) {
		if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
			if ( GM_GameStatus & STATE_VR_ANOTHER ) {
				if ( entk->pl_eyei.sight_real == EYE_INFO_SIGHT_IN ) {
					if ( GM_PlayerStatus & PLAYER_MOVE )	{		/* 動いたら */
printf("box de ugoita!!\n");
						entk->alert = DEF_EYE_DELAY ;
					}
				}
			} else {
				if ( GM_PlayerStatus & PLAYER_MOVE )	{		/* 動いたら */
printf("box de ugoita!!\n");
					entk->alert = DEF_EYE_DELAY ;
				}
			}
		}
	}
#else
	if ( entk->think3 != TH3_WATCH &&
		 entk->think3 != TH3_WAIT ) {
		if ( entk->notice & ENE_NOTICE_BOX ) {
			if ( GM_PlayerStatus & PLAYER_MOVE )	{		/* 動いたら */
printf("box de ugoita!!\n");
				entk->alert = DEF_EYE_DELAY ;
			}
		}
	}
#endif
}

/*----- スタートモード、物音モード --------------------------------------------*/
void	THK_BoxModeStart( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = BoxModeCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	ENE_TraceClear( entk ) ;
	entk->c_notice |= ENE_NOTICE_BOX ;

	ENE_SetTrgpPoint( &(entk->trgpoint_cover), 
	&(entk->com->plpos_in_zone[PL_POS_CENTER]), entk->com->plmap_in_zone[PL_POS_CENTER] ) ;

	entk->think3 = TH3_WATCH ; 
	entk->tmp_time = 0 ;
	entk->count3 = 0 ;

//	entk->act->aim_pos = GM_PlayerPosition ;
	entk->act->aim_pos = GM_PlayerFindPos ;
	entk->act->move_s = ( GM_AlertMode == ALERT_MODE_AVOID )? MoveCautionWalk : MoveWalk ;

	entk->sense.status = RADAR_COLOR_YELOW ;
	COM_SetBoxPos( &GM_PlayerPosition ) ;

	/* ストリーミングここで停止アクシデントにはならない */
	COM_StopRadioNoAccident( entk ) ;
}

void	THK_BoxModeClearingStart( entk )
ENETHINK	*entk ;
{
	THK_BoxModeStart( entk ) ;
	entk->think3 = TH3_CLE_WATCH ; 
}

void	THK_BoxModeNearStart( entk )
ENETHINK	*entk ;
{
printf("box near start\n");
	THK_BoxModeStart( entk ) ;
	if ( OtherEnemyBoxAction( entk ) ) {
		entk->think3 = TH3_WAIT_OTHER_ACTION ; 
	} else {
		entk->think3 = TH3_KICK ; 
	}
}
