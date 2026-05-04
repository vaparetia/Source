//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_act.c
  エマ行動

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_act.c,v 1.1.1.3 2002/11/19 11:45:59 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"

void EMA_ActLayerMotionRelease( Work *work )
{
    int mar_mtn ;

    mar_mtn = MAR( work->npc.action.current_mar,
		   work->npc.action.current_mot ) ;
    if ( work->mar_mtn != mar_mtn )
    {
#if 0
	if ( work->mar_mtn == MAR( EMA_BASE, RAI_WALK_CORPS ) )
	    GM_ConfigObjectAction( work->npc.body, 1, -1,
				   0, 0xfffffffe, 120*5/TIME_BASE ) ;
#endif
	work->time = 0 ;
    }
    work->mar_mtn = mar_mtn ;
}

void EMA_ActControl( Work *work )
{
    NPCWORK *npc = &work->npc ;
    OBJECT  *body = npc->body ;
    CONTROL *ctrl = npc->ctrl ;
    extern void NPC_IK_Control( NPCWORK *npc ) ;

    /* モーションが変更された時のみ コントロールリセットをする(モーショ
       ンの最初に実行するため) */
    if ( npc->action.pad != body->m_ctrl->mt3_ctrl->motion_num )
	if ( EMA_Flag( EMA_F_RESET_CONTROL ) )
	{
	    ctrl->skip_flag &= ~(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK) ;
	    ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
	    body->m_ctrl->interp_flag &= 0xfffffffe ;
	    EMA_ResetFlag( EMA_F_RESET_CONTROL|EMA_F_NON_DAMAGE ) ;
	}
    /* このフレームの位置を取っておく */
    _sceVu0CopyVector( &work->prev_pos, &work->control.mov ) ;

    /* アクトモーションの前に高さを調整 */
    npc->old_body_height = body->height + work->height ;
    GM_ActMotion( body ) ;

    /* アクトコントロールの前に高さを調整 */
    ctrl->height         = body->height + work->height ;
    GM_ActControl( ctrl ) ;

    /* 足のIK処理 */
    if ( EMA_Flag( EMA_F_ENB_IK ) )
    {
	work->control.flr_flag |=  HZX_FLOOR_IK ;
	Leg_IKcalcForEmma( ctrl, body ) ;
	work->control.flr_flag &= ~HZX_FLOOR_IK ;
    }

    /* アクトオブジェクト */
    GM_ActObject2( body ) ;

    /* NPCのIK処理 おもに倒れたときなど */
    NPC_IK_Control( npc ) ;

    DG_GetLightMatrix( &ctrl->mov, npc->lights ) ;

    /* SEを鳴らす */
    if ( npc->se_id >= 0 )
	MT_SetMotionSeTable( body->m_ctrl,
			     GM_CurrentMap,
			     npc->se_id, 
			     (npc->ctrl->flr_atrs[ 0 ] & 0xf0000000) >> 28,
			     0 ) ;

    work->height = 0.0f ; /* すぐに０に戻す */
}


/*-----   標準アクション   ------------------------------------*/
#include "action/act.h"
/*----- ノーマルアクション ------------------------------------*/
#include "action/act_normal.h"
/*----- ダメージアクション ------------------------------------*/
#include "action/act_damage.h"
/*-----   狙撃アクション   ------------------------------------*/
#include "action/act_sniper.h"


/*-------------------------------------------------------------*/
int EMA_ActCheckPad( NPCWORK *npc )
{
    NPCACT	*act = &npc->action ;

    switch ( act->pad )
    {
    case 0 :
	return 0 ;

    case PAL_SITDOWN :
	NPC_SetModeFromPad( npc, ActSitDown, npc->base_mar, act->pad, act->pad ) ;
	break ;
    case PAL_SQUAT_IDLE :
	NPC_SetModeFromPad( npc, ActSquat, npc->base_mar, act->pad, act->pad ) ;
	break ;
    case SNIPE_GO_DOWN:
	NPC_SetModeFromPad( npc, ActGoDown, npc->base_mar, act->pad, act->pad ) ;
	break ;
    case SNIPE_GO_UP:
	NPC_SetModeFromPad( npc, ActGoUp, npc->base_mar, act->pad, act->pad ) ;
	break ;

    case DAMG_DROPOFF:
	NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar, act->pad, act->pad ) ;
	break ;
    case SNIPE_DOWN_FALL:
	NPC_SetModeFromPad( npc, ActSleptDropFromBridge, npc->base_mar, act->pad, act->pad ) ;
	break ;

    case RAI_WALK_CORPS:
	NPC_SetModeFromPad( npc, ActWalkCorpse, npc->base_mar, act->pad, act->pad ) ;
	break ;

    case DAMG_TUMBLE:
	NPC_SetModeFromPad( npc, ActDownTumble, npc->base_mar, act->pad, act->pad ) ;
	break ;
    case DAMG_WAKE :
	NPC_SetModeFromPad( npc, ActDamageWakeUp, npc->base_mar, act->pad, act->pad ) ;
	break ;

    case PAL_PUSHED_LF :
    case PAL_PUSHED_LB :
    case PAL_EROKICK:
    case PAL_DISLIKE:

    case RAI_REACH :
    case RAI_GETUP :
    case DAMG_OVER :
    case DAMG_SQUAT:
    case DAMG_IDLE:
    case SNIPE_SITDOWN:
    case SNIPE_SITAFRAID:
    case SNIPE_STANDUP:
	NPC_SetModeFromPad( npc, EMA_ActOneTimeMotion, npc->base_mar, act->pad, act->pad ) ;
	break ;

    default:
	NPC_SetModeFromPad( npc, EMA_ActLoopMotion, npc->base_mar, act->pad, act->pad ) ;
	break ;
    }
    return 1 ;
}


#if 0
15931293 /*rai_kick_01 */:キックヒット（1回目 WP_KICK | WP_KICK1 が入ってます。
15931294 /*rai_kick_02 */:キックヒット（2回目 WP_KICK が入ってます。
15227620 /*rai_punch_01*/:パンチヒット弱 WP_PUNCH
15227621 /*rai_punch_02*/:パンチヒット中 WP_PUNCH、Ｍ４、ＡＫ、クレイモア、ＲＧＢ装備時
15227622 /*rai_punch_03*/:ニキータアタック WP_KICKがはいっているので、装備で見てください。
#endif

/*----------------------------------------------------------------*/

static void DamagePrePro( Work *work, NPCWORK *npc, int child_num )
{
    PLY_EE_ResetGraspModeAndTrigger() ;

    NPC_CaptureFlagClear( npc ) ;

    /* スネーク音声以外は,必ず止める */
    if ( work->voice_flg != EMA_VCF_SNIPER_SNAKE_WISP )
	EMA_UtilStopStream( work ) ;
}

static void DamagePostPro( Work *work, NPCWORK *npc, int child_num )
{
    NPCACT *act   = &npc->action ;

    work->time = 0 ;/* モーションが変わったら時間をクリア */
    EMA_ResetFlag( EMA_F_NEAR_BY_PLY ) ;
    /* このフレームではダメージを受けない */
    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF );
}

static void DamagePunch( Work *work, NPCWORK *npc, int child_num )
{
    CONTROL *ctrl = npc->ctrl ;
    NPCACT *act   = &npc->action ;

    if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_L )
    {
	act->faint = -20 ;
	GM_SeSetMode( SD_P_GUNPNC01, &ctrl->mov, GM_SEMODE_BOMB ) ;
	NewPadVibration2( 15227622 /*rai_punch_03*/, 0 ) ;
    }
    else if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_M )
    {
	act->faint -= EMA_PUNCH_DAM*3 ;
	GM_SeSetMode( SD_P_GUNPNC01, &ctrl->mov, GM_SEMODE_BOMB ) ;
	NewPadVibration2( 15227621 /*rai_punch_02*/, 0 ) ;
    }
    else
    {
	act->faint -= EMA_PUNCH_DAM ;
	GM_SeSetMode( SD_P_PUNCH02, &ctrl->mov, GM_SEMODE_BOMB ) ;
	NewPadVibration2( 15227620 /*rai_punch_01*/, 0 ) ;
    }

    work->voice_id  = SD_V_EMADMG01 ;
    work->voice_tim = 1 ;
    if ( act->faint > 0 )
	NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
			    DAMG_IDLE, DAMG_IDLE ) ;
    else
	NPC_SetModeFromPad( npc, ActFall, npc->base_mar,
			    DAMG_FALL, DAMG_FALL ) ;

    EMA_Pushed( work, SD_V_EMADMG01 ) ;
}

static void DamageKick( Work *work, NPCWORK *npc, int child_num )
{
    CONTROL *ctrl  = npc->ctrl ;
    NPCACT *act    = &npc->action ;

    if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_LL )
    {
	act->faint = -20 ;
	GM_SeSetMode( SD_P_NKTPNC01, &ctrl->mov, GM_SEMODE_BOMB ) ;
    }
    else
    {
	act->faint -= EMA_KICK_DAM ;
	GM_SeSetMode( SD_P_KICK02, &ctrl->mov, GM_SEMODE_BOMB ) ;
    }
    work->non_damage = 10 ;
    work->voice_id  = SD_V_EMADMG02 ;
    work->voice_tim = 1 ;
    NPC_SetModeFromPad( npc, ActFall, npc->base_mar, DAMG_FALL, DAMG_FALL ) ;

    EMA_Pushed( work, SD_V_EMASCR01 ) ;
}

static void DamageKick1( Work *work, NPCWORK *npc, int child_num )
{
    NewPadVibration2( 15931293 /*rai_kick_01 */, 0 ) ; 
    DamageKick( work, npc, child_num ) ;
}

static void DamageKick2( Work *work, NPCWORK *npc, int child_num )
{
    NewPadVibration2( 15931294 /*rai_kick_02 */, 0 ) ; 
    DamageKick( work, npc, child_num ) ;
}

static void DamageM92( Work *work, NPCWORK *npc, int child_num )
{
    CONTROL   *ctrl = npc->ctrl ;
    NPCTARGET *trg  = &npc->target ;

    GM_SeSetMode( SD_A_M_NINE01, &ctrl->mov, GM_SEMODE_BOMB ) ;
    work->non_damage = EMA_INVINCIBLE_TIME ;

    EMA_ResetFlag( EMA_F_HURT_BY_PLY ) ;

    work->voice_id  = SD_V_EMAAFR01 ;
    work->voice_tim = 1 ;
    if ( child_num >= 0 )
    {
	NPC_SetNeedl( npc->body, trg->connect_obj[child_num],
		      &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
	if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	{
	    switch ( work->npc.action.set_pad )
	    {
	    case DAMG_TUMBLE:
		if ( !MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 17*5 ) )
		    NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar,
					SNIPE_DOWN_FALL, SNIPE_DOWN_FALL ) ;
		else
		    NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar,
					SNIPE_SLEEP_FALL, SNIPE_SLEEP_FALL ) ;
		break ;
	    case DAMG_WAKE:
		if ( work->body.m_ctrl->mt3_ctrl[0].play_time < 170*5 )
		    NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar,
					SNIPE_SLEEP_FALL, SNIPE_SLEEP_FALL ) ;
		else
		    NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar,
					SNIPE_DOWN_FALL, SNIPE_DOWN_FALL ) ;
		break ;
	    default:
		NPC_SetModeFromPad( npc, ActDropFromBridge, npc->base_mar,
				    SNIPE_DOWN_FALL, SNIPE_DOWN_FALL ) ;
		break ;
	    }
	}
	else
	    NPC_SetModeFromPad( npc, ActSleepDown, npc->base_mar,
				DAMG_SLEEP, DAMG_SLEEP ) ;
    }
}

static void DamageBullet( Work *work, NPCWORK *npc, int child_num )
{
    int damage ;

    //NewBlood( &def->world, &def->hit, &trg->force, 0, 0 ) ;
    work->voice_id  = SD_V_EMADMG01 ;
    work->voice_tim = 1 ;

    /* ヘッドショットか？ */
    if ( child_num == 0 && EMA_Flag(EMA_F_HURT_BY_PLY) )
	damage = work->vital_max ;
    else
	damage = work->damage_gun ;

    if ( EMA_DamageVitality( work, damage, DAMG_SLEEP ) )
	switch ( work->npc.action.set_pad )
	{
	case SNIPE_SITAFRAID:
	    if ( EMA_Flag(EMA_F_NEAR_BY_PLY) && !EMA_Flag(EMA_F_IS_ATTACKED) )
		NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				    DAMG_IDLE , DAMG_IDLE  ) ;
	    else
		NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				    DAMG_SQUAT, DAMG_SQUAT ) ;
	    break ;

	case DAMG_TUMBLE:
	    if ( !MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, 17*5 ) )
		NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				    DAMG_IDLE , DAMG_IDLE  ) ;
	    else
		NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				    DAMG_SQUAT, DAMG_SQUAT ) ;
	    break ;

	case DAMG_WAKE:
	    if ( work->body.m_ctrl->mt3_ctrl[0].play_time < 170*5 )
		npc->action.adj_piku_time = NPC_ADJ_PIKU_TIME ;
	    else
		NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				    DAMG_SQUAT, DAMG_SQUAT ) ;
	    break ;

	case SNIPE_AFRAID:
	case SNIPE_SQUAT_IDLE:
	case SNIPE_SITDOWN:
	case DAMG_IDLE:
	    NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				DAMG_SQUAT, DAMG_SQUAT ) ;
	    break ;

	default:
	    NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar,
				DAMG_IDLE , DAMG_IDLE  ) ;
	    break ;
	}
}

static void DamageBlast( Work *work, NPCWORK *npc, int child_num )
{
    int motion ;
    work->voice_id  = SD_V_EMADMG01 ;
    work->voice_tim = 1 ;

    if ( work->flag & EMA_F_EVENT_SNIPE )
    {
	motion = EMA_Flag( EMA_F_IS_ON_BRIDGE ) ? DAMG_DROPOFF : DAMG_OVER ;
	if ( !EMA_DamageGameOver( work, motion ) )
	{
	    if ( EMA_Flag(EMA_F_HURT_BY_VMP) )
		NPC_SetModeFromPad( npc,
				    ActDropFromBridge,
				    work->npc.capture_mar,
				    CAP_TIE, DAMG_DOWN_OVER ) ;
	    else
		NPC_SetActMode( npc, ActDropFromBridge ) ;
	}
    }
    else if ( EMA_DamageVitality( work, work->damage_bomb, DAMG_OVER ) )
	NPC_SetModeFromPad( npc, ActFall, npc->base_mar,
			    DAMG_FALL, DAMG_FALL ) ;
}

static void DamageThrowG( Work *work, NPCWORK *npc, int child_num )
{
    work->voice_id  = SD_V_EMADMG01 ;
    work->voice_tim = 1 ;

    npc->action.adj_piku_time = NPC_ADJ_PIKU_TIME ;
}

static void DamageStunFar( Work *work, NPCWORK *npc, int child_num )
{
    NPCACT *act   = &npc->action ;

    work->voice_id  = SD_V_EMADMG01 ;
    work->voice_tim = 1 ;
    if ( EMA_Flag( EMA_F_ENB_STUNGRE) )
    {
	if ( EMA_Flag(EMA_F_EVENT_SNIPE) )
	    NPC_SetModeFromPad( npc, EMA_ActOneTimeMotion,
				npc->base_mar,
				SNIPE_SITDOWN, SNIPE_SITDOWN ) ;
	else 
	{
	    NPC_SetModeFromPad( npc, ActFall, npc->base_mar,
				DAMG_FALL, DAMG_FALL ) ;
	    act->faint = 0 ;
	}
    }
}

static void CapturePrePro( Work *work, NPCWORK *npc, CAPTURE_TARGET *cap )
{
    NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */

    PLY_EE_ResetGraspModeAndTrigger() ;

    /* スネーク音声以外は,必ず止める */
    if ( work->voice_flg != EMA_VCF_SNIPER_SNAKE_WISP )
	EMA_UtilStopStream( work ) ;
}

static void CapturePostPro( Work *work, NPCWORK *npc, CAPTURE_TARGET *cap )
{
    cap->flag = 0 ;
    work->time = 0 ;/* モーションが変わったら時間をクリア */
    EMA_ResetFlag( EMA_F_NEAR_BY_PLY ) ;
}

static void CaptureHang( Work *work, NPCWORK *npc, CAPTURE_TARGET *cap )
{
    NPCACT	*act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;
    NPC_SetModeFromPad( npc, NPC_ActHang, npc->capture_mar,
			CAP_TIE, DAMG_FALL ) ;
}

static void CaptureThrow( Work *work, NPCWORK *npc, CAPTURE_TARGET *cap )
{
    NPCACT	*act = &npc->action ;

    work->voice_id = SD_V_EMADWN01 ;
    work->voice_tim = 1 ;
    act->faint -= EMA_THROW_DAM ;
    NPC_SetModeFromPad( npc, NPC_ActThrow, npc->capture_mar,
			CAP_THROW, DAMG_FALL ) ;
}

int EMA_ActCheckDamage( NPCWORK *npc )
{
    TARGET	*def    = npc->target.deftrg ;
    NPCTARGET	*trg    = &npc->target ;
    long64	 weapon = 0 ;
    int		 dam_child_num ;
    Work        *work   = (Work *)npc->character ;


    EMA_ResetFlag( EMA_F_ATTACKED_BY ) ;

    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 )
    {
	trg->dam_trg  = trg->def_child + dam_child_num ;
	weapon        = trg->dam_trg->weapon_type ;

	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */

	if ( !EMA_Flag(EMA_F_NON_DAMAGE) && !work->non_damage )
	{
	    printf( "Emma: I'm Hit %lx\n", weapon ) ;

	    DamagePrePro( work, npc, dam_child_num ) ;

	    if ( weapon & (WP_PUNCHR| WP_PUNCHL) )
		DamagePunch( work, npc, dam_child_num ) ;
	    else if ( weapon & WP_KICK )
	    {
		if ( weapon & WP_KICK1 )
		    DamageKick1( work, npc, dam_child_num ) ;
		else
		    DamageKick2( work, npc, dam_child_num ) ;
	    }
	    else if ( weapon & (WP_M92|WP_PSG1T) )
		DamageM92( work, npc, dam_child_num ) ;
	    else if ( weapon & WP_BULLET )
		DamageBullet( work, npc, dam_child_num ) ;
	    else if ( weapon & WP_BLAST )
		DamageBlast( work, npc, dam_child_num ) ;
	    else if ( weapon & WP_THROWG )
		DamageThrowG( work, npc, dam_child_num ) ;
	    else
		goto parent_target ;
	    //NPC_SetModeFromPad( npc, EMA_ActDamage, npc->base_mar, DAMG_IDLE, DAMG_IDLE ) ;

	    DamagePostPro( work, npc, dam_child_num ) ;
	    return 1 ;
	}
	EMA_ResetFlag( EMA_F_NEAR_BY_PLY ) ;
    }

parent_target:
    if ( TARGET_POWER & def->damaged )
    {
	trg->dam_trg = def ;
	weapon = def->weapon_type ;
	NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */

	/* 首締めイベントの時はスティンガー一発死に */
	if ( EMA_Flag(EMA_F_EVENT_SNIPE) && EMA_Flag(EMA_F_HURT_BY_VMP) )
	    if ( weapon & WP_STINGER )
	    {
		if ( !EMA_DamageGameOver( work, DAMG_OVER ) )
		    NPC_SetActMode( npc, ActDropFromBridge ) ;
		return 1 ;
	    }

	if ( !EMA_Flag(EMA_F_NON_DAMAGE) && !work->non_damage )
	{
	    printf( "Emma: Parent Hit weap%lx %x\n", weapon, def ) ;

	    DamagePrePro( work, npc, -1 ) ;
	    if ( weapon & (WP_PUNCHR| WP_PUNCHL) )
		DamagePunch( work, npc, -1 ) ;
	    else if ( weapon & WP_KICK )
	    {
		if ( weapon & WP_KICK1 )
		    DamageKick1( work, npc, -1 ) ;
		else
		    DamageKick2( work, npc, -1 ) ;
	    }
	    else if ( weapon & (WP_M92|WP_PSG1T) )
		DamageM92( work, npc, -1 ) ;
	    else if ( weapon & WP_BULLET )
		DamageBullet( work, npc, -1 ) ;
	    else if ( weapon & WP_STUNFAR )
		DamageStunFar( work, npc, -1 ) ;
	    else if ( weapon & WP_THROWG )
		DamageThrowG( work, npc, -1 ) ;
	    else if ( weapon & WP_BLAST )
		DamageBlast( work, npc, -1 ) ;
	    else if ( weapon & WP_COLDSPRAY )
		return 0 ;
	    else
		goto capture_target ;
	    DamagePostPro( work, npc, -1 ) ;

	    return 1 ;
	}
	EMA_ResetFlag( EMA_F_NEAR_BY_PLY ) ;
    }

capture_target:
    if ( TARGET_CAPTURE & def->damaged )
    {
	CapturePrePro( work, npc, trg->capture ) ;
	if ( trg->capture->flag & CAPTURE_HANG )	/* 首絞め */
	    CaptureHang( work, npc, trg->capture ) ;
	else if ( trg->capture->flag & CAPTURE_THROW )	/* 投げ */
	    CaptureThrow( work, npc, trg->capture ) ;
	CapturePostPro( work, npc, trg->capture ) ;

	return 1 ;
    }

    return 0 ;
}

/*----------------------------------------------------------------*/
void EMA_Action( Work *work )
{
    NPCWORK	*npc = &work->npc ;

    if ( !( EMA_Flag( EMA_F_GAMEOVER ) &&
	    GM_CheckObject_IsEnd( &work->body, 0 ) ) )
    {
	NPC_Gravitation( npc ) ;
	NPC_Action( npc ) ;

	EMA_Display( work ) ;   /* 表示処理 */

	NPC_ActStatusCheck( npc ) ;

	EMA_ActLayerMotionRelease( work ) ;
    }

#if DEBUG_MODE
    EMA_DbgDispEvent( work ) ;
#endif
}
