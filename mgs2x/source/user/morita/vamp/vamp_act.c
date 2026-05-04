//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_act.c
  ヴァンプ行動

  2001/03/23 T.Morita
  $Id: vamp_act.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/vamp.h"


/* コントロール処理 */
void VMPS_ActControl( Work *work )
{
    NPCWORK *npc  = &work->npc ;
    OBJECT  *body = npc->body ;
    CONTROL *ctrl = npc->ctrl ;

    npc->old_body_height = body->height ;
    GM_ActMotion( body ) ;
    ctrl->height = body->height ;

    work->d_floor->atr |=  HZX_FLOOR_SKIP ;
    GM_ActControl( ctrl ) ;
    work->d_floor->atr &= ~HZX_FLOOR_SKIP ;

    GM_ActObject2( body );
    NPC_IK_Control( npc ) ;

    DG_GetLightMatrix( &ctrl->mov, npc->lights );
}


/*----- ノーマルアクション -----------------------------------------------------*/
void VMPS_ActStandStill( NPCWORK *npc, int time )
{
    NPCACT *act ;

    act = &npc->action ;
    NPC_ActStatus( act, NPC_ACT_STATUS_STAND ) ;

    if ( npc->CheckDamage( npc ) )
	return ;
    if ( npc->CheckPad( npc ) )
	return ;
}

/*----- ダメージアクション -----------------------------------------------------*/
#define ARMS_MASK ((1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
		   (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
		   (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
		   (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ))
#define BODY_MASK ((1<<HUMAN21_ONAKA)|(1<<HUMAN21_MUNE)|(1<<HUMAN21_KUBI)|(1<<HUMAN21_ATAMA))

void VMPS_ActDamage( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;

    if ( time == 0 )
    {
	NPC_SetActMotion( npc, act->set_mar, HANG_IDLE ) ;
	GM_ConfigObjectAction( npc->body, 1, act->set_mot, 5*4, BODY_MASK, 120/5*TIME_BASE ) ;
    }

    /* 33フレームで止める */
    if ( !npc->CheckDamage( npc ) )
	if ( npc->body->m_ctrl->mt3_ctrl[0].play_time >= 33*5 )
	{
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, VMPS_ActStandStill ) ;
	    GM_ConfigObjectAction( npc->body, 1, -1, 0, BODY_MASK|ARMS_MASK, 120*5/TIME_BASE ) ;
	}
}


void VMPS_ActDamageHead( NPCWORK *npc, int time )
{
    NPCACT *act = &npc->action ;

    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;

    if ( time == 0 )
    {
	NPC_SetActMotion( npc, act->set_mar, HANG_IDLE ) ;
	GM_ConfigObjectAction( npc->body, 1, act->set_mot, 0, BODY_MASK, 8*5 ) ;
    }

    /* 23フレームで止める */
    if ( !npc->CheckDamage( npc ) )
	if ( npc->body->m_ctrl->mt3_ctrl[0].play_time >= 25*5 )
	{
	    act->act_end = 1 ;
	    NPC_SetActMode( npc, VMPS_ActStandStill ) ;
	    GM_ConfigObjectAction( npc->body, 1, -1, 0, BODY_MASK|ARMS_MASK, 8*5 ) ;
	}
}

/*-------------------------------------------------------------*/
int VMPS_ActCheckPad( NPCWORK *npc )
{
    NPCACT *act = &npc->action ;

    switch( act->pad )
    {
    case 0 :
	return 0 ;
    case HANG_TIEUP:
    case HANG_KILL:
	NPC_SetModeFromPad( npc, NPC_ActOneTimeMotion, npc->base_mar, act->pad, act->pad ) ;
	break ;
    default:
	NPC_SetModeFromPad( npc, NPC_ActLoopMotion, npc->base_mar, act->pad, act->pad ) ;
	break ;
    }
    return 1 ;
}



/*----------------------------------------------------------------*/
int VMPS_ActCheckDamage( NPCWORK *npc )
{
    TARGET	*def ;
    NPCACT	*act ;
    NPCTARGET	*trg ;
    long64	weapon ;
    int		dam_child_num ;
    Work       *work ;
    int         damage, damg_id ;
    int         se ;

    act = &npc->action ;
    trg = &npc->target ;
    def = npc->target.deftrg ;
    work = (Work *)npc->character ;
    weapon = 0 ;
    damage = 0 ;
    damg_id = GV_Time & 1 ? DAMG_RIGHT : DAMG_LEFT ;

    if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 )
    {
	trg->dam_trg = trg->def_child + dam_child_num ;
	weapon = trg->dam_trg->weapon_type ;

	/* このフレームではダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_DamageFlagClear( npc ) ;

	printf( "Vamp: damage%d dam_child_num%d %lx\n", damage, dam_child_num, weapon ) ;

	if ( weapon & (WP_M92|WP_PSG1T) )
	{
	    damage = dam_child_num ? work->damage_m9_body : work->damage_m9 ;
	    NPC_SetNeedl( npc->body, trg->connect_obj[dam_child_num],
			  &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
	    damage = VMPS_DamageVitalityM9( work, damage, DAMG_HEAD/*DAMG_SLEEP*/ ) ;
	}
	else if ( weapon & (WP_BULLET|WP_BLAST) )
	{
	    damage = dam_child_num ? work->damage_body : work->damage ;
	    damage = VMPS_DamageVitality( work, damage, DAMG_HEAD/*DAMG_SLEEP*/ ) ;
	}
	else
	    npc->action.adj_piku_time = NPC_ADJ_PIKU_TIME ;

	if ( damage )
	{
	    if ( dam_child_num )
	    {
		se = SD_V_VMPDMG01 ;/*「ウッ！」*/
		NPC_SetModeFromPad( npc, VMPS_ActDamage, npc->base_mar, damg_id, damg_id ) ;
	    }
	    else
	    {
		se = work->npc.action.life > work->vital_max/2 ?
		    SD_V_VMPDMH02 :	/*強ダメージ１「ウアッ」 */
		    SD_V_VMPDMH03 ;	/*強ダメージ２「アァア」 */
		printf( "Vamp: DAMG_HEAD\n" ) ;

		NPC_SetModeFromPad( npc, VMPS_ActDamageHead, npc->base_mar, DAMG_HEAD, DAMG_HEAD ) ;
	    }
	}
	else
	    se = SD_V_VMPDMH01 ;

	GM_SeSetMode( se, &work->control.mov, GM_SEMODE_BOMB ) ;
	return 1 ;
    }

    if ( TARGET_POWER & def->damaged )
    {
	weapon = def->weapon_type ;

	NPC_DamageFlagClear( npc ) ;

	if ( !(work->flag & VMPS_F_NON_DAMAGE) )
	{
	    /* このフレームではダメージを受けない */
	    NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

	    if ( weapon & (WP_PUNCHR|WP_PUNCHL| WP_BLOW|WP_BULLET) )
	    {
		if ( VMPS_DamageVitality( work, damage, DAMG_HEAD/*DAMG_SLEEP*/ ) )
		    NPC_SetModeFromPad( npc, VMPS_ActDamage, npc->base_mar, DAMG_HEAD, DAMG_HEAD ) ;
	    }
	    else if ( weapon & (WP_M92|WP_PSG1T) )
	    {
		if ( VMPS_DamageVitalityM9( work, damage, DAMG_HEAD/*DAMG_SLEEP*/ ) )
		    NPC_SetModeFromPad( npc, VMPS_ActDamage, npc->base_mar, DAMG_HEAD, DAMG_HEAD ) ;
		//NPC_SetNeedl( npc->body, HUMAN21_MUNE, &trg->dam_trg->hit, VMPS_MASUIDAN_MODEL ) ;
	    }
	    else if ( weapon & (WP_BULLET|WP_BLAST) )
	    {
		damage = VMPS_DamageVitality( work, 3, DAMG_HEAD/*DAMG_SLEEP*/ ) ;
	    }
	    return 1 ;
	}
    }

    return 0 ;
}

/*----------------------------------------------------------------*/
void VMPS_Action( Work *work )
{
    NPCWORK	*npc = &work->npc ;

    //NPC_ActInit( npc ) ;
    NPC_Action( npc ) ;
    NPC_ActStatusCheck( npc ) ;

    NPC_Gravitation( npc ) ;
}
