//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atsearch.c
	攻撃兵の探索モード
	
	2000/09/27 Y.Korekado
	$Id: atsearch.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
	
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

enum	{
	TH2_TRAVEL,TH2_DISCOVERY,TH2_CHASE, TH2_ATTACK, TH2_ACCIDENT,
	TH2_CORPS, TH2_ENE_DAMAGE, TH2_TOUCH, TH2_NOISE, TH2_FOUND,
	TH2_TRACE, TH2_INDISTINCT, TH2_BOX, TH2_DAMAGE, TH2_HOLDUP,
	TH2_UNIFORM, 
} ;
enum	{
	TH3_RETURN_TRAVEL,TH3_GO_NEXT, TH3_POINT_ACTION, TH3_ZONE_CHASE,TH3_DIRECT_CHASE,
	TH3_ATTACK_NEAR, TH3_RETURN_WAIT, TH3_START_WAIT
} ;

enum {
	SP_NONE,
	SP_UNREAL,	/* 存在しない */
	SP_AROUND,		/* 左右(45度)を確認 */
} ;

#define AROUND_ACT_NUM	(8)

void ENE_PosWarp( ENETHINK	*entk, FVECTOR		*pos, int			hzx_id );

/*----------------------------------------------------------------------------*/
#include "modechng.c"
/*----------------------------------------------------------------------------*/
#define AROUND_BRANK	COUNT_VMODE(270)
static void ActAround( act, time )
ACTION	*act ;
int		time ;
{
	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

	/* turn.vyにセットされてから */
	if ( (time%AROUND_BRANK) == 0 ) {
		act->aim_dir = act->ctrl->turn.vy  ;
	} else if ( (time%AROUND_BRANK) == AROUND_BRANK/3 ) {
		act->aim_dir = act->ctrl->turn.vy + 512 ;
	} else if ( (time%AROUND_BRANK) == AROUND_BRANK*2/3 ) {
		act->aim_dir = act->ctrl->turn.vy - 512 ;
	}

	if ( (time%AROUND_BRANK) == AROUND_BRANK-1 ) {
		act->act_end = 1 ;
	}
}

/*----------------------------------------------------------------------------*/
static int	AttackerSearchCheckPad( act )
ACTION	*act ;
{
	switch ( act->pad ) {
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_AROUND :
			AT_SetModeFromPad( act, ActAround, ENE_StandMotion(act), act->pad ) ;
		break ;
	}
	return 0 ;
}
/*----------------------------------------------------------------------------*/
static int TimeManage( ENETHINK *entk )
{
	if ( (GM_GameStatus & STATE_VR_ANOTHER) &&
		 (entk->rnavi2->p_acttime == PTIME_MOTIONEND) ) {
		if ( entk->act->act_end ) {
			entk->act->pad = 0 ;
			return 1 ;
		}
	} else {
		if ( entk->rnavi2->p_acttime <= 0) {
			return 1 ;
		}
		entk->rnavi2->p_acttime -- ;
	}
	return 0 ;
}

static int RouteCheck( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;

	rnavi = entk->rnavi2 ;
	if ( entk->search_route == rnavi->c_route ) return 0 ;

	printf("ene[%d] Change Root [%d]->[%d]\n",entk->id, rnavi->c_route, entk->search_route ) ;
	ENE_ChangeRoute2( entk,entk->search_route ) ;

	if ( rnavi->chang_node < 0 ) {
		ENE_SetTrgpNearRootPoint( entk->rnavi2, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		rnavi->next_node = rnavi->chang_node ;
		ENE_SetTrgpNode( rnavi, &(entk->trgpoint) ) ;
	}
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;

	return  1 ;
}
/*----------------------------------------------------------------------------*/
static	void	Think3_TravelStartWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 >= (entk->id*COUNT_VMODE(30)) ) {
		entk->think3 = TH3_RETURN_TRAVEL ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->act->pad = SP_UNREAL ;

	entk->count3 ++ ;
}


static void Think3_ReturnWait( entk )
ENETHINK		*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_RETURN_TRAVEL ; 
		entk->count3 = 0 ;
	}

	entk->count3 ++ ;
}

static void Think3_ReturnTravel( entk )
ENETHINK		*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_GO_NEXT ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveWalk ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GoNext( entk )
ENETHINK		*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		if ( RouteCheck( entk ) ) return ;
		entk->act->dir = entk->rnavi2->pa_dir[(int)entk->rnavi2->next_node] ;
		entk->rnavi2->p_acttime = entk->rnavi2->pa_time[(int)entk->rnavi2->next_node] ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			entk->rnavi2->p_action = entk->rnavi2->pa_action[(int)entk->rnavi2->next_node] ;
			entk->rnavi2->p_actstatus = entk->rnavi2->pa_flag[(int)entk->rnavi2->next_node] ;
		}
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_PointAction( entk )
ENETHINK		*entk ;
{
	if ( entk->search_route == entk->rnavi->c_route ) {
		entk->act->pad = SP_UNREAL ;
	}else if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		switch ( entk->rnavi2->p_action ) {
			case AROUND_ACT_NUM :
			 entk->act->pad = SP_AROUND ;
			 entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			 break ;
		}
	}

//	if ( entk->rnavi2->p_acttime <= 0) {
	if ( TimeManage ( entk ) ) {
		if ( RouteCheck( entk ) ) return ;
		if ( entk->rnavi2->n_nodes > 1 ) {
			ENE_SetTrgpNextnode( entk->rnavi2, &(entk->trgpoint) ) ;
			entk->think3 = TH3_GO_NEXT ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( entk->rnavi2->p_actstatus & PA_CON_AIM_GUN ) {
			entk->status2 |= ENE_STATUS2_AIM_GUN ;
			entk->act->aim_pos = entk->rnavi2->aimnodes[ (int)entk->rnavi2->next_node ] ;
		}
	}

	entk->count3 ++ ;
}

static void Think3_ZoneChasePlayer( entk )
ENETHINK		*entk ;
{
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}
	if( entk->pl_eyei.dis < 9000-(entk->id*700) ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_NEAR ; 
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChasePlayer( entk )
ENETHINK		*entk ;
{
	int		reach ;

	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 9000-(entk->id*700) ) < 0 ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_NEAR ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = 
			HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
	if( reach > HZX_DIRECT_REACH ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


static	void	Think3_StandNear( entk )
ENETHINK		*entk ;
{
	if( entk->pl_eyei.dis > 10000-(entk->id*700) ) {
		entk->think2 = TH2_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
/*----------------------------------------------------------------------------*/
	/*
		巡回
	*/
static	void	Think2_Travel( entk )
ENETHINK		*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_WAIT :		    /* 指定位置に移動 */
	    	Think3_ReturnWait( entk ) ;
		break ;
	    case TH3_RETURN_TRAVEL :		    /* 指定位置に移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_MOVE ) ;
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_GO_NEXT :					/* 次の巡回ポイントへ移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_TRACE ) ;
			Think3_GoNext( entk ) ;
		break ;
	    case TH3_POINT_ACTION :				/* ポイントアクション */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_ACTION ) ;
	    	Think3_PointAction( entk ) ;
		break;

	    case TH3_START_WAIT :		    /* ずらし待ち */
		   	Think3_TravelStartWait( entk ) ;
		break ;
	}
}

static	void	Think2_Trace( entk )
ENETHINK		*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChasePlayer( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChasePlayer( entk ) ;
		break ;
	}
}
/*



*/
static	void	Think2_Stand( entk )
ENETHINK		*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ATTACK_NEAR :
	    	Think3_StandNear( entk ) ;
		break ;
	}
}

/*----------------------------------------------------------------------------*/
static	void	TravelModeCheack( entk )
ENETHINK	*entk ;
{
	TravelModeChange( entk ) ;
}

static	void	AccidentModeCheack( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ACCIDENT) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	AccidentModeChange( entk ) ;
}

static	void	UniformModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_UNIFORM) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	/* 他のモードへ */
	UniformModeChange( entk ) ;
}

static	void	CorpsModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	CorpsModeChange( entk ) ;
}


static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	EneDamageModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	HoldUpModeChange( entk ) ;
}

static	void	DiscoveryModeCheack( entk )
ENETHINK	*entk ;
{
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
printf("noise mode return search \n");
		ENE_AttackerStartModeSearch( entk ) ;
	}

	NoiseModeChange( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	FoundModeChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		ENE_AttackerStartModeSearch( entk ) ;
	}

	TraceModeChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	DamageModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	IndistinctModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	BoxModeChange( entk ) ;
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_AttackerStartModeSearch( entk ) ;
	}

	TouchModeChange( entk ) ;
}


/*----------------------------------------------------------------------------*/
	/*
		巡回
	*/
void	ENE_Attacker_Think1_Search( entk )
ENETHINK		*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_TRAVEL :		/* 巡回 */
			Think2_Travel( entk ) ;
			TravelModeCheack( entk ) ;
		break ;
	    case TH2_CHASE :		/* プレイヤー追っかけ */
			Think2_Trace( entk ) ;
		break ;

	    case TH2_NOISE :			/* 物音モード */
			THK_NoiseMode( entk ) ;
			NoiseModeCheck( entk ) ;
		break ;
	    case TH2_FOUND :			/* 不審物発見モード */
			THK_FoundMode( entk ) ;
			FoundModeCheck( entk ) ;
		break ;
	    case TH2_TRACE :			/* 跡、追跡モード */
			THK_TraceMode( entk ) ;
			TraceModeCheck( entk ) ;
		break ;
	    case TH2_DAMAGE :			/* ダメージモード */
			THK_Damage( entk ) ;
			DamageModeCheck( entk ) ;
		break ;
	    case TH2_INDISTINCT :		/* 朧モード */
			THK_IndistinctMode( entk ) ;
			IndistinctModeCheck( entk ) ;
		break ;
	    case TH2_BOX :				/* ダンボールモード */
			THK_BoxMode( entk ) ;
			BoxModeCheck( entk ) ;
		break ;
	    case TH2_TOUCH :			/* タッチモード */
			THK_TouchMode( entk ) ;
			TouchModeCheck( entk ) ;
		break ;

	    case TH2_ACCIDENT :			/* 異常事態発生 */
			THK_AccidentMode( entk ) ;
			AccidentModeCheack( entk ) ;
		break ;
	    case TH2_UNIFORM :			/* 変装モード */
			THK_UniformMode( entk ) ;
			UniformModeCheck( entk ) ;
		break ;
	    case TH2_CORPS :			/* 死体モード */
			THK_CorpsMode( entk ) ;
			CorpsModeCheck( entk ) ;
		break ;
	    case TH2_ENE_DAMAGE :		/* 味方ダメージ発見 */
			THK_EneDamage( entk ) ;
			EneDamageModeCheck( entk ) ;
		break ;
	    case TH2_HOLDUP :			/* ホールドアップモード */
			THK_HoldUpMode( entk ) ;
			HoldUpModeCheck( entk ) ;
		break ;


	    case TH2_ATTACK :		/* 立ち止まり */
			Think2_Stand( entk ) ;
		break ;
	    case TH2_DISCOVERY :	/* 発見 */
			THK_Discovery( entk ) ;
			DiscoveryModeCheack( entk ) ;
		break ;
	}
}

/*----------------------------------------------------------------------------*/
void	ENE_AttackerStartModeSearch( entk )
ENETHINK	*entk ;
{
	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;

	entk->sense.status = RADAR_COLOR_BLUE ;
	if ( entk->act->status & ACT_STATUS_UNREAL ) {
		ENE_RouteWarp( entk ) ;
	}

	entk->act->CheckPad = AttackerSearchCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	UNSET_FLAG ( entk->iknow_flag, IKNOW_CLEARING ) ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_SEARCH ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

printf("id[%d] entk->rnavi->c_route[%d] entk->search_route = %d\n",
entk->uniq_id, entk->rnavi->c_route, entk->search_route ) ;

	ENE_ReadNodes( entk->rnavi2, entk->search_route ) ;
	entk->rnavi2->c_route = entk->search_route ;
	entk->rnavi2->next_node = 0 ;

	ENE_SetTrgpNearRootPoint( entk->rnavi2, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
printf("entk->trgaddr = %x\n",entk->trgpoint.addr ) ;

	entk->think1 = ENE_TH1_SEARCH ; 
	entk->think2 = TH2_TRAVEL ; 
	/* 待機所でＵＮＲＡＬ状態なら */
	if ( (entk->act->status & ACT_STATUS_UNREAL) && 
		 (entk->search_route == entk->rnavi->c_route) ) {
		entk->act->pad = SP_UNREAL ;
		entk->act->dir = entk->rnavi2->pa_dir[(int)entk->rnavi2->next_node] ;
		entk->rnavi2->p_acttime = entk->rnavi2->pa_time[(int)entk->rnavi2->next_node] ;
		entk->think3 = TH3_POINT_ACTION ; 
		AT_SetModeFromPad( entk->act, ENE_ActUnreal, ENE_StandMotion(entk->act), entk->act->pad ) ;
	}  else {
		entk->think3 = TH3_RETURN_TRAVEL ; 

		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			if ( (entk->act->status & ACT_STATUS_UNREAL) ) {
				entk->act->keep_pad = entk->act->pad = SP_UNREAL ;
				entk->think3 = TH3_START_WAIT ; 
			}
		}
	}
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
}

void	ENE_AttackerStartModeSearchDamage( entk )
ENETHINK	*entk ;
{

	entk->think1 = ENE_TH1_SEARCH ; 

	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
			entk->mess_notice == MES_NOTICE_GOOFY ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
		return ;
	}

#if 0
	if ( entk->c_notice == ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	} else 
#endif

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think2 = TH2_DISCOVERY ; 
//		THK_DiscoveryModeStart( entk ) ;
		THK_DiscoveryModeStartAvoid( entk ) ;
	} else {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageModeStart( entk ) ;
	}
}

void	ENE_AttackerStartModeAvoidToSearch( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeSearch( entk ) ;
	if ( entk->think2 != TH2_DAMAGE ) {
		entk->think3 = TH3_RETURN_WAIT ; 
	}
}

void	ENE_AttackerStartModeSearchWarp( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeSearch( entk ) ;
	ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;
}

void	ENE_AttackerStageStartModeSearchWarp( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeSearch( entk ) ;

	/* 待機所でＵＮＲＡＬ状態なら */
	if ( entk->search_route == entk->rnavi->c_route ) {
		entk->act->pad = SP_UNREAL ;
		entk->act->dir = entk->rnavi2->pa_dir[(int)entk->rnavi2->next_node] ;
		entk->rnavi2->p_acttime = entk->rnavi2->pa_time[(int)entk->rnavi2->next_node] ;
		entk->think3 = TH3_POINT_ACTION ; 
		entk->count3 = 0 ;
		AT_SetModeFromPad( entk->act, ENE_ActUnreal, ENE_StandMotion(entk->act), entk->act->pad ) ;
	} else {
#if 0
		ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;
//		ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 250/250 ) ;
#else
		{
			FVECTOR pos ;
			entk->rnavi2->next_node = 0 ;
			ENE_SetTrgpNode( entk->rnavi2, &(entk->trgpoint) ) ;
			pos.vx = entk->trgpoint.pos.vx ;
			pos.vy = entk->trgpoint.pos.vy + 1000.0f ;
			pos.vz = entk->trgpoint.pos.vz ;
			ENE_PosWarp( entk, &pos, entk->trgpoint.map ) ;
		}
#endif
printf("kokotootayo------------------------------[%d]\n",entk->id );
	}
}
