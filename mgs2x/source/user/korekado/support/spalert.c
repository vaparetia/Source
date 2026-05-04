//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wctravel.c
	巡回兵 危険モード
	
	1997/07/29 Y.Korekado
	$Id: spalert.c,v 1.1.1.3 2002/11/19 11:44:24 Yoshizawa1 Exp $
	
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
	TH2_CHASE, TH2_MOVE, TH2_ATTACK, TH2_SEARCH, TH2_PBREAK
} ;
enum {
	TH3_ZONE_CHASE, TH3_DIRECT_CHASE, TH3_ATTACK_SETUP, TH3_ATTACK_MGUN, TH3_ATTACK_RELOAD,
	TH3_ATTACK_NEAR, TH3_MEDICAL, TH3_MOVE_WAITAREA, TH3_WATCH, TH3_MORTALLY
} ;
enum {
	SP_NONE,
	SP_DOWNBACK,
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_READYGUN,	/* 銃構える */
	SP_NEARATK,		/* 近接攻撃 */
	SP_MGUN_SHOT,	/* マシンガン撃つ */
	SP_RELOAD,		/* リロード */
	SP_MEDICATION,	/* 治療  */
	SP_KERI,		/* 蹴り  */
} ;


#include	"spaleact.c"

/*----- --------------------------------------------*/
#define	BUDDY_DIS	1000

/*----- --------------------------------------------*/
static	int	SupportTrgCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->spbudy == NULL ) {
		return 0 ;
	}
	return 1 ;
}

static void SetAttackThink( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
	} else {
		if ( SupportTrgCheck( entk ) ) {
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
		} else {
			entk->think2 = TH2_MOVE ;
			entk->think3 = TH3_ZONE_CHASE ;
		}
	}
	entk->count3 = 0 ;
}

static void SetPBreakThink ( entk )
ENETHINK *entk ;
{
	entk->think2 = TH2_PBREAK ;
	/* 両足故障していたら */
	if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
		entk->think3 = TH3_MORTALLY ;
	} else if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
	} else {
		/* 両腕故障していたら */
		if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
			ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
			entk->think3 = TH3_MOVE_WAITAREA ; 
		} else {
			entk->think3 = TH3_MEDICAL ; 
		}
	}

	entk->count3 = 0 ;
}
/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		ENE_SetTrgpDefense( &(entk->trgpoint), &entk->spbudy->def_pos, entk->spbudy->def_mapbit ) ;

		if ( SupportTrgCheck( entk ) ) {
			if( entk->bd_eyei.dis < BUDDY_DIS ) {
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_WATCH ; 
			} else {
				entk->think2 = TH2_CHASE ; 
				entk->think3 = TH3_ZONE_CHASE ;
			}
		} else {
			if ( ENE_SameZone( &entk->trgpoint.pos, &entk->znavi->flore_pos, entk->ctrl->hzx_id ) ) {
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_WATCH ; 
			} else {
				entk->think2 = TH2_MOVE ; 
				entk->think3 = TH3_ZONE_CHASE ;
			}
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(20) ) {
		entk->think3 = TH3_ATTACK_MGUN ;
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_AttackWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}
	entk->act->pad = SP_CAUT_STAND ;
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ZoneChaseBuddy( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}
	if( entk->bd_eyei.dis < BUDDY_DIS ) {
		entk->think2 = TH2_SEARCH ; 
		entk->think3 = TH3_WATCH ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;

		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseBuddy( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), BUDDY_DIS ) < 0 ) {
		entk->think2 = TH2_SEARCH ; 
		entk->think3 = TH3_WATCH ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	if ( ENE_DirectReachCheck( entk ) == 0 ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;

		return ;
	}
	
	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


static void Think3_ZoneMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}


static	void	Think3_AttackNear( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) ? SP_KERI : SP_NEARATK ;
	}

	if ( entk->act->act_end ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	entk->count3 ++ ;
}

static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		entk->bullet = 0 ;
		entk->think3 = TH3_ATTACK_RELOAD ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( !(entk->count3 % 4) ) {
		if ( !(BP_PS2_rand()%3) ) {
			entk->think3 = TH3_ATTACK_SETUP ; 
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

static	void	Think3_AttackReload( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) entk->act->pad = SP_RELOAD ;

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_PB_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		SetPBreakThink( entk ) ;
		return ;
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && !(GM_PlayerStatus & PLAYER_GROUND) ) {
		entk->think3 = TH3_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 > COUNT_VMODE(20) ) {
		if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
			entk->think3 = TH3_ATTACK_MGUN ;
			entk->count3 = 0 ;
			return ;
		}
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_Medical( entk )
ENETHINK	*entk ;
{
	/* ちょっと待つ */
	if ( entk->count3 < COUNT_VMODE(4) ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			SetPBreakThink( entk ) ;
			return ;
		}
	}

	if ( entk->count3 == COUNT_VMODE(4) ) {
		entk->act->pad = SP_MEDICATION ;
	}

	if ( entk->act->act_end ) {
		SetAttackThink( entk ) ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_MoveWaitArea( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		
		return ;
	}

	ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_MEDICAL ;
		entk->count3 = 0 ;
		
		return ;
	}
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
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

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Chase( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChaseBuddy( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseBuddy( entk ) ;
		break ;
	}
}

static	void	Think2_MoveDefensePoint( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :
	    	Think3_ZoneMove( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMove( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;

		return ;
	}
}

static	void	Think2_Attack( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
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
	}
}
static	void	Think2_AlertSearch( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WATCH :
	    	Think3_AttackWatch( entk ) ;
		break ;
	}
}

static	void	Think2_PBreak( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ATTACK_SETUP :
	    	Think3_PB_ReadyGun( entk ) ;
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
	    case TH3_MEDICAL :
			Think3_Medical( entk ) ;
		break ;
	    case TH3_MOVE_WAITAREA :
			Think3_MoveWaitArea( entk ) ;
		break ;
	    case TH3_MORTALLY :
		    Think3_Mortally( entk ) ;
		break ;
	}
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		危険
	*/
void	ENE_Support_Think1_Alert( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_CHASE :
			Think2_Chase( entk ) ;
		break ;
	    case TH2_MOVE :
			Think2_MoveDefensePoint( entk ) ;
		break ;
	    case TH2_ATTACK :
			Think2_Attack( entk ) ;
		break ;
	    case TH2_SEARCH :	/* 指定場所で警戒する */
			Think2_AlertSearch( entk ) ;
		break ;
	    case TH2_PBREAK :
			Think2_PBreak( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = MAX_ALERT_LEVEL ;
	}
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->alert = MAX_ALERT_LEVEL ;
	}
}
/*----- スタートモード、サポート兵危険モード --------------------------------------------*/
void ENE_SupportStartModeAlert( entk )
ENETHINK	*entk ;
{
	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;

	entk->act->CheckPad = SupportAlertCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	/* 視覚情報チェック開始 */
	entk->bd_eyei.flag &= ~EYE_INFO_FLAG_SKIP ;
	if ( SupportTrgCheck( entk ) ) {
		ENE_SetEyeInfo( &(entk->bd_eyei), &entk->spbudy->ctrl->mov, 
							&entk->spbudy->ctrl->addr, NULL, &entk->spbudy->ctrl->hzx_id ) ;
		ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;
	} else {
	}

	entk->think1 = ENE_TH1_ALERT ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		SetPBreakThink( entk ) ;
	} else {
		SetAttackThink( entk ) ;
	}

	entk->act->move_s = MoveRun ;
}
