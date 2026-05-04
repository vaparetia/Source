//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ataneak.c
	攻撃兵の潜入モード
	
	1997/07/26 Y.Korekado
	$Id: atsneak.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
	
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

#define KORE_1025 (1)

//#include	"atavoid.h"
//#include	"atavoact.c"
enum	{
	TH2_TRAVEL,TH2_DISCOVERY,TH2_CHASE, TH2_ATTACK, TH2_ACCIDENT,
	TH2_CORPS, TH2_ENE_DAMAGE, TH2_TOUCH, TH2_NOISE, TH2_FOUND,
	TH2_TRACE, TH2_INDISTINCT, TH2_BOX, TH2_DAMAGE, TH2_HOLDUP,
	TH2_UNIFORM, 
} ;
enum	{
	TH3_RETURN_TRAVEL,TH3_GO_NEXT, TH3_POINT_ACTION, TH3_ZONE_CHASE,TH3_DIRECT_CHASE,
	TH3_ATTACK_NEAR, TH3_LISTEN_RADIO
} ;


enum {
	SP_NONE,
	SP_UNREAL,	/* 存在しない */
	SP_LISTEN_RADIO
} ;

#include "modechng.c"
/*----------------------------------------------------------------------------*/
/* キープモーション */
/*----------------------------------------------------------------------------*/
static int	AttackerSneakCheckPad( act )
ACTION	*act ;
{
	if ( act->pad == 0 ) return 0 ;

	switch ( act->pad ) {
		case SP_UNREAL :
			AT_SetModeFromPad( act, ENE_ActUnreal, ENE_StandMotion(act), act->pad ) ;
		break ;
		case SP_LISTEN_RADIO :
			AT_SetModeFromPad( act, ENE_ActKeepMotion, EM_ak_call_team_l, act->pad ) ;
		break ;
	}
	return 1 ;
}
/*----------------------------------------------------------------------------*/
static int RouteCheck( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	
	rnavi = entk->rnavi ;
	if ( rnavi->next_route == rnavi->c_route ) return 0 ;

	printf("ene[%d] Change Root [%d]->[%d]\n",entk->id, rnavi->c_route, rnavi->next_route ) ;
	ENE_ChangeRoute( entk,rnavi->next_route ) ;
#if 1
	ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
#else
	if ( entk->rnavi->chang_node < 0 ) {
		ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		entk->rnavi->next_node = entk->rnavi->chang_node ;
		ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
	}
#endif	
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;

	return  1 ;
}
/*----------------------------------------------------------------------------*/
static void Think3_ListenRadio( entk )
ENETHINK		*entk ;
{
	entk->act->pad = SP_LISTEN_RADIO ;

	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->think3 = TH3_RETURN_TRAVEL ; 
	}

	entk->count3 ++ ;
}


static void Think3_ReturnTravel( entk )
ENETHINK		*entk ;
{
	if ( RouteCheck( entk ) ) return ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_GO_NEXT ; 
		entk->count3 = 0 ;
		entk->act->move_s = MoveRun ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GoNext( entk )
ENETHINK		*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			entk->act->dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
		}
		entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
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
	entk->act->pad = SP_UNREAL ;

	if ( entk->rnavi->p_acttime <= 0) {
		if ( entk->rnavi->n_nodes > 1 ) {
			ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
			entk->think3 = TH3_GO_NEXT ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->rnavi->p_acttime -- ;
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
	    case TH3_LISTEN_RADIO :		   		/* 司令聞くふり */
	    	Think3_ListenRadio( entk ) ;
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
#ifdef KORE_1025
	TravelModeChange( entk ) ;
#else
	if ( entk->notice & ENE_NOTICE_ACCIDENT ) {
		entk->think2 = TH2_ACCIDENT ;
		THK_AccidentModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
#endif
}

static	void	AccidentModeCheack( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ACCIDENT) ) {
		ENE_AttackerStartModeSneak( entk ) ;
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
		ENE_AttackerStartModeSneak( entk ) ;
	}

	CorpsModeChange( entk ) ;
}


static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	EneDamageModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ENE_AttackerStartModeSneak( entk ) ;
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
		ENE_AttackerStartModeSneak( entk ) ;
	}

	NoiseModeChange( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	FoundModeChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		ENE_AttackerStartModeSneak( entk ) ;
	}

	TraceModeChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	DamageModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	IndistinctModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	BoxModeChange( entk ) ;
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_AttackerStartModeSneak( entk ) ;
	}

	TouchModeChange( entk ) ;
}

/*----------------------------------------------------------------------------*/
	/*
		巡回
	*/
void	ENE_Attacker_Think1_Normal( entk )
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
void	ENE_AttackerStartModeSneak( entk )
ENETHINK	*entk ;
{
	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;

	entk->act->CheckPad = AttackerSneakCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->sense.status = RADAR_COLOR_BLUE ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_SNEAK ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

	ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	entk->think1 = ENE_TH1_SNEAK ; 
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;
	entk->act->move_s = MoveRun ;
}

void	ENE_AttackerStartModeSneakDamage( entk )
ENETHINK	*entk ;
{

	entk->think1 = ENE_TH1_SNEAK ; 

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

void	ENE_AttackerStartModeSearchToSneak( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeSneak( entk ) ;
	if ( entk->think2 != TH2_DAMAGE ) {
		entk->think3 = TH3_LISTEN_RADIO ; 
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			if ( entk->act->status & ACT_STATUS_UNREAL ) {
				if ( ENE_InRange( &entk->ctrl->mov, &entk->trgpoint.pos, 350 ) ) {
					entk->act->keep_pad = entk->act->pad = SP_UNREAL ;
					entk->act->dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
					entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
					entk->think3 = TH3_POINT_ACTION ; 
					entk->count3 = 0 ;
				}
			}
		}
	}
}

void	ENE_AttackerStageStartModeToSneak( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeSneak( entk ) ;
	entk->act->pad = SP_UNREAL ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		/* 開始時エフェクト出現防止 */
		SET_FLAG( entk->act->status, ACT_STATUS_UNREAL ) ;
	}
	entk->act->dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
	entk->rnavi->p_acttime = entk->rnavi->pa_time[(int)entk->rnavi->next_node] ;
	entk->think3 = TH3_POINT_ACTION ; 
	AT_SetModeFromPad( entk->act, ENE_ActUnreal, ENE_StandMotion(entk->act), entk->act->pad ) ;
}
