//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atavoid.c
	クリアリング（攻撃兵の回避モード）
	
	1999/07/26 Y.Korekado
	$Id: atavoid.c,v 1.1.1.3 2002/11/19 11:43:58 Yoshizawa1 Exp $
	
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

//#include	"atavoid.h"

//#define PRI_DEBUG
#define PRI_DEBUG_MAN	0

#define	PROC_CALL_GOAL_POINT (1)

/* 仮 */
extern	CLEARING	*Cle ;

/*----- 思考状態 --------------------------------------------------*/
enum	{
	TH2_DISCOVERY, TH2_CHASE, TH2_ATTACK, TH2_MOVE, TH2_SEARCH,
	TH2_READY, TH2_DYNAMIC_ENTRY, TH2_READY_CLEARING, TH2_CLEARING, TH2_CLE_COMP,
	TH2_WITHDRAW, TH2_NOISE, TH2_INDISTINCT, TH2_NOTICE_SUPPORT, TH2_TRACE,
	TH2_BOX,TH2_ENE_DAMAGE, TH2_DAMAGE
} ;
enum	{
	TH3_GO_NEXT, TH3_POINT_ACTION, TH3_WAIT, TH3_DISCOVERY_POSE, TH3_ZONE_CHASE,
	TH3_DIRECT_CHASE, TH3_ATTACK_SETUP, TH3_SEARCH_STAND, TH3_RETURN_TRAVEL, TH3_ZONE_RETURN,
	TH3_DIRECT_RETURN, TH3_PINPOINT_MOVE, TH3_RETURN_ORDER, TH3_CLEAR_END, TH3_OYA,
	TH3_LOOKAROUND, TH3_NOISE, TH3_WARP_WAIT
} ;

/*-----	定数定義	-----*/

	/*アクションパッド */
enum {
	SP_NONE,
	SP_DOWNBACK,
	SP_DISCOVERY,	/* 発見ポーズ */
	SP_READYGUN,	/* 銃構える */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_MGUN_SHOT,	/* マシンガン撃つ */
	SP_RELOAD,		/* リロード */
	SP_CLE_GOSIGN,	/* 突撃合図 */
	SP_CLE_PEEP_L,	/* 左覗き込み */
	SP_CLE_PEEP_R,	/* 右覗き込み */
	SP_CLE_PEEP_D,	/* 下覗き込み */
	SP_CLE_AROUND,	/* 左右を確認 */
	SP_CLE_CLEAR_SIGN,	/* クリアサイン */
	SP_CLE_MOVE_SIGN,	/* 動けサイン */
	SP_CLE_STOP_SIGN,	/* とまれサイン */
	SP_MUKADE_WAIT,		/* むかで待ち */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_PINPOINT,
	SP_WAOOOOOO,		/* エッチなもの発見 */
	SP_RADIOCALL,
	SP_RETURNRADIO,
	SP_RADIOCALL_ATTACKER,
	SP_THROW_STUN,		/* スタングレネード投げる */
	SP_PEEP_DUCT,		/* ダクト覗き込み */
	SP_PEEP_TOILET_U,	/* トイレ上覗き込み */
	SP_PEEP_TOILET_D,	/* トイレ下覗き込み */
	SP_TOILET_KNOCK,	/* トイレノック */
	SP_WB,				/* 白！ */
	SP_NORMALSTAND,		/* 銃を降ろした立ち */

	/* 注！！移動変更アクションパッドはこれ以上の値にすること */
	SP_MV_WALK,		/* 歩き */
	SP_MV_RUN,		/* 走り */
	SP_MV_CAUT_WALK,/* 警戒歩き */
	SP_MV_CAUT_RUN,	/* 警戒走り */
	SP_MV_SIDE_L,	/* 左横移動 */
	SP_MV_SIDE_R,	/* 右横移動 */
	SP_MV_BACK,		/* バック */
	SP_MV_STAIR_RUN_D,	/* 階段走り降り */
	SP_MV_STAIR_RUN_U,	/* 階段走り降り */
	SP_MV_STAIR_WALK_D,	/* 階段走り降り */
	SP_MV_STAIR_WALK_U,	/* 階段走り降り */
	SP_MV_BACK_CAUTION,		/* バック */
} ;

#include	"atavoact.c"

/*-----------------------------------------------------------------------------------*/

static int CleActArray[] = {
	SP_CAUT_STAND,	/* 0 */
	SP_CAUT_STAND,
	SP_CLE_GOSIGN,		/* スタン投げ */
	SP_MV_WALK,		/* 歩き */
	SP_MV_RUN,		/* 走り */
	SP_MV_CAUT_WALK,/* 警戒歩き */
	SP_MV_CAUT_RUN,	/* 警戒走り */
	SP_MV_SIDE_L,	/* 左横移動 */
	SP_MV_SIDE_R,	/* 右横移動 */
	SP_MV_BACK,		/* バック */
	SP_CLE_PEEP_L,	/* 10 */
	SP_CLE_PEEP_R,
	SP_CLE_PEEP_D,
	SP_CLE_AROUND,	/* 左右を確認 */
	SP_MV_STAIR_RUN_D,	/* 階段走り降り */
	SP_MV_STAIR_RUN_U,	/* 階段走り上り */
	SP_MV_STAIR_WALK_D,	/* 階段歩き降り */
	SP_MV_STAIR_WALK_U,	/* 階段歩き上り */
	SP_CLE_CLEAR_SIGN,	/* クリアサイン */
	SP_CLE_MOVE_SIGN,	/* 動けサイン */
	SP_CLE_STOP_SIGN,	/* 20 とまれサイン */
	SP_MUKADE_WAIT,		/* むかで待ち */
	SP_LOCKER_OPEN,		/* ロッカー開ける */
	SP_MV_BACK_CAUTION,		/* 左右警戒バック */
	SP_WAOOOOOO,		/* エッチなもの発見 */
	SP_THROW_STUN,
	SP_PEEP_DUCT,		/* ダクト覗き込み */
	SP_PEEP_TOILET_U,	/* トイレ上覗き込み */
	SP_PEEP_TOILET_D,	/* トイレ下覗き込み */
	SP_TOILET_KNOCK,	/* トイレノック */
	SP_WB,				/* 白！ */
	SP_NORMALSTAND,		/* 銃を降ろした立ち */
} ;

enum {
	RDATA_CON_NONE,
	RDATA_CON_START_SIGN,
	RDATA_CON_DE_END,
	RDATA_CON_CLE_SIGN,
	RDATA_CON_END,
	RDATA_CON_END_SIGN
} ;

/*
	unit->buff[]
			0: クリアリングのエリア
			1: クリアリングの種類
			2: クリアリングの状態
*/
#if 0
#define		CLE_BUFF_AREA	0
#define		CLE_BUFF_KIND	1
#define		CLE_BUFF_SCENE	2
#define		CLE_BUFF_STATUS	3
#endif

//static	FVECTOR	armik_shift = {50.0F,   0.0F, -120.0F, 0.0F};
#if 1
static	FVECTOR	armik_shift = {100.0F,   -40.0F, -90.0F, 0.0F};
#else
static	FVECTOR	armik_shift = {50.0F,   -140.0F, -120.0F, 0.0F};
#endif
/*-----------------------------------------------------------------------------------*/
static int NowDamage( ENETHINK *entk )
{
	/* ダメージを受けている */
#if 1
	if ( entk->act->old_status & ACT_STATUS_MASUI_SASARU ) return 1 ;
	if ( entk->act->status_status & ACT_STST_NOW_DAMAGE ) return 1 ;
#else
	if ( entk->act->old_status & (ACT_STATUS_DAMAGE|ACT_STATUS_MASUI_SASARU) ) return 1 ;
#endif

	return 0 ;
}

static int ClearingGoNormalCheck( ENETHINK *entk )
{
	if ( Cle->iknow_flag & CLE_IKNOW_GO_NORMAL ) {
		FVECTOR	pos ;
		int map ;
printf("clearing: id[%d] go normal!!!\n",entk->id ) ;
		COM_GetPlayerLastPos( &pos, &map ) ;
		SIG_AttackerStartModeAvoidPosSet( entk , &pos, map ) ;
		UNSET_FLAG( entk->iknow_flag, IKNOW_AVOID_CLEARING ) ;
		return 1 ;
	}

	return 0 ;
}


static int RouteChangeCheck( ENETHINK *entk )
{
	int route, point ;

	route = COM_GetCleRoute( entk->id ) ;
	point = COM_GetClePoint( entk->id ) ;
	if ( entk->rnavi2->c_route != route ) {
		return 1 ;
	} else if ( point > entk->rnavi2->next_node ) {
		return 1 ;
	}

	return 0 ;
}

static void SetRouteChange( ENETHINK *entk )
{
	int route, point ;

	route = COM_GetCleRoute( entk->id ) ;
	point = COM_GetClePoint( entk->id ) ;

	if ( entk->rnavi2->c_route != route ) {
		ENE_ReadClearing( entk->rnavi2, COM_GetUnitBuff(entk,CLE_BUFF_AREA) , route ) ;
		entk->rnavi2->next_node = point - 1 ;
	} else if ( point > entk->rnavi2->next_node ) {
		entk->rnavi2->next_node = point - 1 ;
	}
}

static int RevCleActData( sp_data )
int	sp_data ;
{
	if ( sp_data >= SP_MV_WALK ) {
		return SP_CAUT_STAND ;
	}
	return sp_data ;
}

static void CleResetMove( entk )
ENETHINK	*entk ;
{
	entk->act->move_s = MoveCautionWalk ;
}

static void CleChangeMoveCheck( entk, sp_data )
ENETHINK	*entk ;
int	sp_data ;
{
	if ( sp_data < SP_MV_WALK ) {
		entk->act->move_s = MoveCautionWalk ;
		return ;
	}

	switch( sp_data ) {
		case SP_MV_WALK :		/* 歩き */
			entk->act->move_s = MoveWalk ;
		break ;
		case SP_MV_RUN :		/* 走り */
			entk->act->move_s = MoveRun ;
		break ;
		case SP_MV_CAUT_WALK :	/* 警戒歩き */
			entk->act->move_s = MoveCautionWalk ;
		break ;
		case SP_MV_CAUT_RUN :	/* 警戒走り */
			entk->act->move_s = MoveCautionRun ;
		break ;
		case SP_MV_SIDE_L :		/* 左横移動 */
			entk->act->move_s = MoveSideL ;
		break ;
		case SP_MV_SIDE_R :		/* 右横移動 */
			entk->act->move_s = MoveSideR ;
		break ;
		case SP_MV_BACK :		/* バック */
		case SP_MV_BACK_CAUTION :		/* バック */
			entk->act->move_s = MoveBack ;
		break ;
		case SP_MV_STAIR_RUN_D :		/* 階段走り降り*/
			entk->act->move_s = MoveStairRunD ;
		break ;
		case SP_MV_STAIR_RUN_U :		/* 階段走り上り */
			entk->act->move_s = MoveStairRunU ;
		break ;
		case SP_MV_STAIR_WALK_D :		/* 階段歩き降り */
			entk->act->move_s = MoveStairWalkD ;
		break ;
		case SP_MV_STAIR_WALK_U :		/* 階段歩き上り */
			entk->act->move_s = MoveStairWalkU ;
		break ;
	}
}

static int NextPointActionCon( rnavi )
ROUTENAVI	*rnavi ;
{
	int		num ;

	num = rnavi->next_node + 1 ;
	if ( num >= rnavi->n_nodes ) {
		return -1 ;
	}

	return rnavi->pa_con[ num ] ;
}

static void SetAroundAim( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	if ( (entk->count3%COUNT_VMODE(270) ) == COUNT_VMODE(90) ) {
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( (entk->count3%COUNT_VMODE(270)) == COUNT_VMODE(180) ) {
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	} else if ( (entk->count3%COUNT_VMODE(270)) == 1 ) {	/* turn.vyにセットされてから */
		entk->act->aim_dir = entk->ctrl->turn.vy  ;
	}

// printf(" aim_dir[%d] turn[%d]\n",entk->act->aim_dir,entk->ctrl->turn.vy ) ;
}

static void Waoooooo( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == COUNT_VMODE(10) ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BP ) ;
	}
}

static void CleHeadMark( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == COUNT_VMODE(10) ) {
		switch ( entk->act->pad ) {
			case SP_WB :
				ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
			break ;
		}
	}
}

static void SetAroundAimFast( entk )
ENETHINK	*entk ;
{
	int dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	dir = (entk->ctrl->turn.vy ) & 4095 ;
	if ( (entk->count3%COUNT_VMODE(270)) == COUNT_VMODE(60) ) {
		entk->act->aim_dir = dir + 512 ;
	} else if ( (entk->count3%COUNT_VMODE(270)) == COUNT_VMODE(120) ) {
		entk->act->aim_dir = dir - 512 ;
	} else if ( (entk->count3%COUNT_VMODE(180)) == 2 ) {	/* turn.vyにセットされてから */
		entk->act->aim_dir = dir  ;
	}

//if( entk->id == 1) printf(" aim_dir[%d] turn[%d]\n",entk->act->aim_dir,entk->ctrl->turn.vy ) ;
}

static void OpenLocker( entk )
ENETHINK	*entk ;
{
	int near ;

	if ( entk->think3 == TH3_POINT_ACTION ) {
		if ( entk->count3 == 0 ) {
			R_INTRPT	*r_intrpt ;

			near = ENE_GetMinDirNearZone( entk->ctrl->addr, entk->ctrl->rot.vy ) ;
			r_intrpt = GM_GetRIntrpt( entk->ctrl->addr, near ) ;
printf("zone[%x] to[%x]\n",entk->ctrl->addr, near );
			if ( r_intrpt != NULL ) {
printf("if[%d]:z1[%x] z2[%x]\n",r_intrpt->id,r_intrpt->zone1,r_intrpt->zone2 );
				GM_RIntrptCallBack( r_intrpt, 100, 5, RINTRP_ORDER_OPEN ) ;
printf("open CALL \n");
			}
		}
	}
}

//#define PA_TIME_OYA	(COUNT_VMODE(400))
#define PA_TIME_OYA	(COUNT_VMODE(180))
#define OYA_STATUS (ACT_STATUS_DEATH|ACT_STATUS_DOWN|ACT_STATUS_FAINT|ACT_STATUS_DOWN| \
						ACT_STATUS_UNREAL|ACT_STATUS_CAPTURE )
static int PointActionTimeManage( entk )
ENETHINK	*entk ;
{
	if ( entk->rnavi2->p_acttime >= PA_TIME_SCENE ) {
		entk->scene = entk->rnavi2->p_acttime ;
		/* 仲間のシナリオが値以上になるまで待つ */
		if ( COM_GetUnitBuff( entk, CLE_BUFF_SCENE ) >= entk->scene  ) return 1 ;

		if ( entk->count3 >= PA_TIME_OYA ) {
			if ( COM_AnyoneStatus_G_U( entk->com, entk->g_id, entk->u_id, OYA_STATUS ) ) {
				return 2 ;
			}
		}
		return 0 ;
	}

	switch( entk->rnavi2->p_acttime ) {
		case PA_TIME_MOTION :
			if ( entk->act->act_end ) {
				return 1 ;
			}
		break ;
		case PTIME_VOICEEND :
		break ;
		default :
			if ( entk->rnavi2->p_acttime <= 0 ) {
				return 1 ;
			}
			entk->rnavi2->p_acttime-- ;
		break ;
	}
	return 0 ;
}

static void ClearingVoice( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		int time ;

		time = COM_CallClearingVoice(CLE_BUFF_AREA, entk->rnavi2->c_route,
				entk->rnavi2->next_node, EV_CLEARING_ONESELF ,entk ) ;
		if ( entk->rnavi2->p_acttime == PTIME_VOICEEND ) {
			entk->rnavi2->p_acttime = time ;
		}
	}
}

static void PointActionManage( entk )
ENETHINK	*entk ;
{
	entk->act->pad = entk->rnavi2->p_action ;
	switch ( entk->act->pad ) {
		case SP_CLE_AROUND :
			SetAroundAim( entk ) ;
		break ;
		case SP_WAOOOOOO :
			Waoooooo( entk ) ;
		break ;
		case SP_LOCKER_OPEN :
			OpenLocker( entk ) ;
		break ;
		case SP_WB :
			CleHeadMark( entk ) ;
		break ;
	}
}

static void MoveActionManage( entk )
ENETHINK	*entk ;
{
	int move_s ;
	
	move_s = CleActArray[ (int)entk->rnavi2->pa_action[(short)entk->rnavi2->next_node] ] ;
	switch ( move_s ) {
		case SP_MV_BACK_CAUTION :
			SetAroundAimFast( entk ) ;
		break ;
	}
}

static void VoiceManage( entk )
ENETHINK	*entk ;
{
	if ( entk->rnavi2->p_actstatus & PA_CON_NO_VOICE  ) {
		return ;
	}

	switch ( entk->act->pad ) {
		case SP_CLE_CLEAR_SIGN :
			if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_CLEAR_1, entk ) ;
		break ;
		case SP_CLE_GOSIGN :
			if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_GO_1, entk ) ;
		break ;
		case SP_CLE_STOP_SIGN :
			if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_STOP_1, entk ) ;
		break ;
		case SP_CLE_MOVE_SIGN :
			if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_GO_1, entk ) ;
		break ;
	}
}

static	void PointDirectionManage( entk )
ENETHINK	*entk ;
{
	int	dir ;

	if ( entk->rnavi2->p_dir == -2 ) {
		/* 注視点の方向を向く */
		dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->aim_pos ) ;
	} else {
		/* 値がー１なら方向はそのままになる */
		dir = entk->rnavi2->p_dir ;	/* 方向指定 */
	}

	if ( entk->ctrl->turn.vy != dir ) {
		entk->act->dir = dir ;
	}

#ifdef PRI_DEBUG
if ( entk->id==PRI_DEBUG_MAN) {
	printf(" PointDirectionManage dir[%d] rot[%d] pad[%d]\n",entk->act->dir, entk->ctrl->rot.vy, entk->rnavi2->p_dir ) ;
}
#endif
}

static void SetClearingAction( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi2 ;
	int			node ;

	rnavi2 = entk->rnavi2 ;
	node = (int)rnavi2->next_node ;

	rnavi2->p_action = RevCleActData( CleActArray[ rnavi2->pa_action[ node ] ] ) ;
	rnavi2->p_acttime = rnavi2->pa_time[ node ] ;
#ifdef PAL
	if ( entk->rnavi2->p_acttime < PA_TIME_SCENE && entk->rnavi2->p_acttime > 0 ) {
		rnavi2->p_acttime = COUNT_VMODE(rnavi2->p_acttime) ;
	}
#endif
	rnavi2->p_dir = rnavi2->pa_dir[ node ] ;
	rnavi2->p_con = rnavi2->pa_con[ node ] ;

	rnavi2->p_actstatus = rnavi2->pa_con[ node ] ;

	entk->act->aim_pos = rnavi2->aimnodes[ node ] ;

#ifdef PRI_DEBUG
if ( entk->id==PRI_DEBUG_MAN) {
	printf("[%d][%d]:act[%d] time[%d] ",entk->id, (int)rnavi2->next_node,rnavi2->p_action,rnavi2->p_acttime);
	printf("dir[%d] con[%d] status[%d]\n",rnavi2->p_dir,rnavi2->p_con,rnavi2->p_actstatus ) ;
}
#endif

#ifndef	PROC_CALL_GOAL_POINT
	COM_CleProc( COM_GetUnitBuff(entk,CLE_BUFF_AREA), COM_GetCleRoute( entk->id ), node ) ;
#endif
}

static void CleStartScene( entk ) 
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	int	i ;

	rnavi = entk->rnavi2 ;
	for( i=0; i<rnavi->n_nodes; i++ ) {
		if( rnavi->pa_time[ i ] == PA_TIME_CLE_START ) break ;
		if( rnavi->pa_time[ i ] > PA_TIME_CLE_START ) {
			entk->scene = PA_TIME_CLE_START ;
			break ;
		}
	}
}

/*-----------------------------------------------------------------------------------*/
/* クリアリングスタート地点まで移動 */
//#define	START_POS_RANGE	(350+(entk->id*200))
#define	START_POS_RANGE	(350)


/* ワープ後重なり防止 */
static void Think3_WarpWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(entk->id*15) ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}


static void Think3_ZoneMoveCSP( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		int route ;
		
		route = COM_GetCleRoute( entk->id ) ;
printf("id[%d] clearing route[%d]\n",entk->id, route ) ;
		ENE_ReadClearing( entk->rnavi2, COM_GetUnitBuff(entk,CLE_BUFF_AREA) , route ) ;

		entk->rnavi2->c_route = route ;
		entk->rnavi2->next_node = COM_GetClePoint( entk->id ) ;
		SetClearingAction( entk ) ;

		ENE_SetTrgpNode( entk->rnavi2, &(entk->trgpoint) ) ;
printf("Think3_ZoneMoveCSP[%d] route[%d] node[%d] addr[%x]\n",
entk->id, route,entk->rnavi2->next_node, entk->trgpoint.addr ) ;
	}

	if ( ENE_InRange( &entk->ctrl->mov, &entk->trgpoint.pos, START_POS_RANGE ) ) {
		CleStartScene( entk ) ;

		entk->think2 = TH2_CLEARING ;
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
		return ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}


	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectMoveCSP( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), START_POS_RANGE ) < 0 ) {
		CleStartScene( entk ) ;

		entk->think2 = TH2_CLEARING ;
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}


/* メンバーが配置に着くまで待機 */
static void Think3_WaitMoveEnd( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->dir = entk->rnavi2->p_dir ;
	} else {
		if ( entk->rnavi2->p_con != RDATA_CON_START_SIGN ) {	/* ゴーサインじゃなかったら */
			entk->act->pad = entk->rnavi2->p_action ;
		}
	}

	if ( entk->rnavi2->p_actstatus & PA_CON_AIM_GUN_MOVE  ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
	}

	if ( COM_GetUnitBuff( entk, CLE_BUFF_SCENE ) == CLE_TH3_GOSIGN ) {
		entk->think2 = TH2_READY ;
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		if ( NextPointActionCon( entk->rnavi2 ) == RDATA_CON_START_SIGN ) {
			ENE_SetNextnode( entk->rnavi2 ) ;
			SetClearingAction( entk ) ;
		}

		return ;
	}

	entk->count3 ++ ;
}


static void Think3_ZoneMoveReturn( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_RETURN ;
		entk->count3 = 0 ;
		
		return ;
	}
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_DirectMoveReturn( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		SetClearingAction( entk ) ;
		entk->think2 = TH2_CLEARING ;
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

/* 突入、ダイナミックエントリー */
static void Think3_ClearingGoNext( entk )
ENETHINK		*entk ;
{
	int	range ;

	if ( entk->rnavi2->p_actstatus & PA_CON_AIM_GUN_MOVE  ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
	}

	/* モーション移動量を考慮し、初回はレンジに幅を持たせる */
	range = (entk->count3 == 0)? 400 : 350 ;
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), range ) < 0 ) {
		if ( entk->rnavi2->p_actstatus & PA_CON_PINPOINT  ) {
			entk->think3 = TH3_PINPOINT_MOVE ;
			entk->count3 = 0 ;
			return ;
		}
#ifdef	PROC_CALL_GOAL_POINT
		/* ポイント到達タイミングでクリアリングプロックコール */
		COM_CleProc( COM_GetUnitBuff(entk,CLE_BUFF_AREA), entk->rnavi2->c_route, entk->rnavi2->next_node ) ;
#endif

		PointDirectionManage( entk ) ;

		if ( entk->rnavi2->p_acttime == 0 ) {	/* アクション無し */
			if ( RouteChangeCheck( entk ) ) {
				SetRouteChange( entk ) ;
			}
			if ( ClearingGoNormalCheck( entk ) ) {
				return ;
			}
			ENE_SetTrgpNextnode( entk->rnavi2, &(entk->trgpoint) ) ;
			SetClearingAction( entk ) ;
			/* 移動方法セット */
			CleChangeMoveCheck( entk, 
				CleActArray[ (int)entk->rnavi2->pa_action[(short)entk->rnavi2->next_node] ] ) ;
			entk->act->dir = -1 ;/* 一時停止 */
		} else {
			/* 基本移動方法に戻す */
			CleResetMove( entk ) ;
			PointActionManage( entk ) ;
			entk->think3 = TH3_POINT_ACTION ;
		}
		entk->count3 = 0 ;

		return ;
	}

	MoveActionManage( entk ) ;

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;

}

static void Think3_PinpointMove( entk )
ENETHINK		*entk ;
{
	if ( entk->rnavi2->p_actstatus & PA_CON_AIM_GUN_MOVE  ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
	}

	if( entk->count3 == 0 ) {
		entk->act->target_pos = entk->trgpoint.pos ;
	}
	PointDirectionManage( entk ) ;
	entk->act->pad = SP_PINPOINT ;

	if ( entk->act->act_end ) {
#ifdef	PROC_CALL_GOAL_POINT
		/* ポイント到達タイミングでクリアリングプロックコール */
		COM_CleProc( COM_GetUnitBuff(entk,CLE_BUFF_AREA), COM_GetCleRoute( entk->id ), entk->rnavi2->next_node ) ;
#endif

		PointDirectionManage( entk ) ;
		if ( entk->rnavi2->p_acttime == 0 ) {	/* アクション無し */
			if ( RouteChangeCheck( entk ) ) {
				SetRouteChange( entk ) ;
			}
			if ( ClearingGoNormalCheck( entk ) ) {
				return ;
			}
			ENE_SetTrgpNextnode( entk->rnavi2, &(entk->trgpoint) ) ;
			SetClearingAction( entk ) ;
			/* 移動方法セット */
			CleChangeMoveCheck( entk, 
				CleActArray[ (int)entk->rnavi2->pa_action[(short)entk->rnavi2->next_node] ] ) ;
			entk->think3 = TH3_GO_NEXT ; 
			entk->act->dir = -1 ;/* 一時停止 */
		} else {
			/* 基本移動方法に戻す */
			CleResetMove( entk ) ;
			PointActionManage( entk ) ;
			entk->think3 = TH3_POINT_ACTION ;
		}
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;

}

static void Think3_ClearingPointAction( entk )
ENETHINK		*entk ;
{
	int	patm ;

	if ( entk->rnavi2->p_actstatus & PA_CON_AIM_GUN_MOVE  ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
	}else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
	}

	patm = PointActionTimeManage( entk ) ;
	if ( patm == 2 ) {	/* 仲間に異常があった */
		entk->think3 = TH3_OYA ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( patm == 1 ) {
		if ( entk->rnavi2->next_node + 1 < entk->rnavi2->n_nodes ) {
			if ( RouteChangeCheck( entk ) ) {
				SetRouteChange( entk ) ;
			}
			if ( ClearingGoNormalCheck( entk ) ) {
				return ;
			}
			ENE_SetTrgpNextnode( entk->rnavi2, &(entk->trgpoint) ) ;
			SetClearingAction( entk ) ;
			CleChangeMoveCheck( entk, 
				CleActArray[ (int)entk->rnavi2->pa_action[(short)entk->rnavi2->next_node] ] ) ;
			/* 方向転換のため一度GoNextに戻る */
			entk->think3 = TH3_GO_NEXT ; 
		} else {
			/* クリアリング終了 */
			if ( RouteChangeCheck( entk ) ) {
				SetRouteChange( entk ) ;
				return ;
			}
			if ( ClearingGoNormalCheck( entk ) ) {
				return ;
			}

			entk->avoid = 0 ;
			ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
			entk->scene = PA_TIME_AVO_END ;
			entk->think2 = TH2_WITHDRAW ; 
			entk->think3 = TH3_RETURN_TRAVEL ; 
			entk->act->move_s = MoveRun ;

//if( entk->id==3) COM_SetRadio( EV_RAD_ATK_RETURN, NULL ) ;
		}
		entk->count3 = 0 ;

		return ;
	}

	ClearingVoice( entk ) ;
	PointActionManage( entk ) ;
	if( entk->count3==0) PointDirectionManage( entk ) ;
	VoiceManage( entk ) ;

	entk->count3 ++ ;
}


static void Think3_ReturnTravel( entk )
ENETHINK		*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}


/* 仲間に異常 */
static void Think3_Oya( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
		COM_SetSpeak( EV_NOTICE_FIND_LV2, entk ) ; /* うーん */
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
//				entk->status2 |= ENE_STATUS2_AIM_GUN ;
			}
		}
	}

	if ( ClearingGoNormalCheck( entk ) ) {
		return ;
	}

	if ( entk->count3 >= COUNT_VMODE(240) ) {
#if 1
		COM_ClearingGoNormal( ) ;
#else
		Cle->scene_flag = CLE_ENE_COMPLETE ;

		entk->think2 = TH2_CLE_COMP ;
//		entk->think3 = TH3_RETURN_ORDER ;
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
#endif
	}

	entk->count3 ++ ;
}

/* クリアリング完了合図 */

static void Think3_ClearingEnd( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_CLE_CLEAR_SIGN ;
	}

	if ( entk->count3 == COUNT_VMODE(20) ) COM_SetSpeak( EV_CLEAR_1, entk ) ;

	if ( entk->act->act_end == 1 ) {
//		entk->think3 = TH3_RETURN_ORDER ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_ClearingOrder( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_RADIO ;

	if ( entk->count3 == 0 ) {
		entk->tmp_time = COM_SetRadio( EV_RAD_END_AVOID, entk ) + entk->count3 ;
		if ( entk->tmp_time < 0 ) {	/* 無線を準備している間に誰かが使用を始めたら */
			/* 無線しているフリをする */
			entk->tmp_time = LV1_STRM_LENGTH + entk->count3 ;
		}
	}

	if ( entk->count3 == entk->tmp_time ) {
		entk->avoid = 0 ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_RADIOCALL_ATTACKER ;
	entk->count3 ++ ;
}


static void Think3_ClearingComp( entk )
ENETHINK	*entk ;
{
	entk->avoid = 0 ;

	if ( ClearingGoNormalCheck( entk ) ) {
		return ;
	}

	if ( entk->com->avoidlevel == 0 ) {
		/* もし全員終了していて 回避規定時間がきてなかったらきょろきょろ*/
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;


	if ( ClearingGoNormalCheck( entk ) ) {
		return ;
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
			entk->status2 |= ENE_STATUS2_AIM_GUN ;
		}
	}

	if ( entk->count3 > COUNT_VMODE(320) ) {
		entk->count3 = 0 ;
		return ;
	}
	if ( GM_AlertMode == ALERT_MODE_AVOID ) {
		entk->act->pad = SP_CAUT_STAND ;
	}
	entk->count3 ++ ;
	return ;
}


/* クリアリング終了合図終了待ち */
static void Think3_WaitCleComp( entk )
ENETHINK	*entk ;
{

	if ( ClearingGoNormalCheck( entk ) ) {
		return ;
	}

	entk->count3 ++ ;
}

/* 見失った地点まで移動 */
static void Think3_ZoneMoveLastPoint( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	if ( _DiffDisAbs( &entk->ctrl->mov, &entk->trgpoint.pos ) < 350 + (500*entk->id) ) {
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		entk->scene = CLE_ENE_WAIT_START ;
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}
static void Think3_DirectMoveLastPoint( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350+(500*entk->id) ) < 0 ) {
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		entk->scene = CLE_ENE_WAIT_START ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}


/* メンバーが配置に着くまで待機 */
static void Think3_MoveLastPointWait( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( COM_GetUnitBuff( entk, CLE_BUFF_SCENE ) == CLE_TH3_GOSIGN ) {
		entk->think2 = TH2_SEARCH ;
		entk->think3 = TH3_SEARCH_STAND ;
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
}
static void Think3_SearchReady( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(150) ) {
		entk->scene = CLE_ENE_WAIT_COMPLETE ;
		entk->avoid = 0 ;
	}

	entk->count3 ++ ;
}


static void Think3_NoticeSupportWait( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	SET_FLAG( entk->thk_status, THK_STATUS_CLE_SUPPORT ) ;

	if ( entk->count3 == COUNT_VMODE(30) ) {
//		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->count3 == COUNT_VMODE(30) ) {
		if ( _FVecTrgDis( &entk->ctrl->mov, &Cle->trg_pos ) < 6000 ) {
			entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &Cle->trg_pos ) ;
		}
	}

	if ( entk->count3 >= PA_TIME_OYA ) {
		if ( COM_AnyoneStatus_G_U( entk->com, entk->g_id, entk->u_id, OYA_STATUS ) ) {
			entk->think2 = TH2_CLEARING ;
			entk->think3 = TH3_OYA ; 
			entk->count3 = 0 ;
			return ;
		}
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		
		entk->act->dir = _FVecTrgDir2( &entk->ctrl->mov, &GM_NoisePosition ) ;
		entk->think3 = TH3_NOISE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_NoticeSupportNoise( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	SET_FLAG( entk->thk_status, THK_STATUS_CLE_SUPPORT ) ;

	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BW ) ;
	}

	if ( entk->count3 >= COUNT_VMODE(120) ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}
/*----- 中レベル思考モード ---------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static	void	Think2_MoveClearingStartP( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WARP_WAIT :
	    	Think3_WarpWait( entk ) ;
		break ;
	    case TH3_ZONE_CHASE :
	    	Think3_ZoneMoveCSP( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMoveCSP( entk ) ;
		break ;
		case TH3_WAIT :
	    	Think3_WaitMoveEnd( entk ) ;
		break ;
	}
}

static	void	Think2_Clearing( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_GO_NEXT :
	    	Think3_ClearingGoNext( entk ) ;
		break ;
	    case TH3_PINPOINT_MOVE :
	    	Think3_PinpointMove( entk ) ;
		break ;
	    case TH3_POINT_ACTION :
	    	Think3_ClearingPointAction( entk ) ;
		break ;

	    case TH3_ZONE_RETURN :
	    	Think3_ZoneMoveReturn( entk ) ;
		break ;
	    case TH3_DIRECT_RETURN :
	    	Think3_DirectMoveReturn( entk ) ;
		break ;

	    case TH3_OYA :
	    	Think3_Oya( entk ) ;
		break ;
	}
}

static	void	Think2_ClearingComp( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_CLEAR_END :
	    	Think3_ClearingEnd( entk ) ;
		break ;
	    case TH3_RETURN_ORDER :
	    	Think3_ClearingOrder( entk ) ;
		break ;
	    case TH3_ATTACK_SETUP :
	    	Think3_ClearingComp( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
	    	Think3_LookAround( entk ) ;
		break ;
		case TH3_WAIT :
	    	Think3_WaitCleComp( entk ) ;
		break ;
	}
}

static	void	Think2_LastPlayerPoint( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :
	    	Think3_ZoneMoveLastPoint( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMoveLastPoint( entk ) ;
		break ;
		case TH3_WAIT :
	    	Think3_MoveLastPointWait( entk ) ;
		break ;
	}
}

static	void	Think2_Search( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_SEARCH_STAND :
	    	Think3_SearchReady( entk ) ;
		break ;
		case TH3_WAIT :
		break ;
	}
}

static	void	Think2_NoticeSupport( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_WAIT :
			Think3_NoticeSupportWait( entk ) ;
		break ;
		case TH3_NOISE :
			Think3_NoticeSupportNoise( entk ) ;
		break ;
	}
}

static	void	Think2_Withdraw( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_TRAVEL :
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_WAIT :
		break ;
	}
}

/*----- 高レベル思考チェック --------------------------------------------*/
#include "clemode.c"

static	void	ClearingModeCheackMove( entk )
ENETHINK	*entk ;
{
//printf("1 id[%d] entk->think2[%d]\n",entk->id,entk->think2);
	ClearingMoveChange( entk ) ;
//printf("2 id[%d] entk->think2[%d]\n",entk->id,entk->think2);
}

static	void	ClearingModeCheack( entk )
ENETHINK	*entk ;
{
	if ( Cle->scene_flag == CLE_ENE_COMPLETE ) {
		/* Notice 及び クリアリングから終了 */
		entk->think2 = TH2_CLE_COMP ;
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
	}

	/* シナリオからキャンセル */
	if ( Cle->iknow_flag & CLE_IKNOW_CANCEL ) {
#if 1
		ENE_AttackerStartModeEndClearingToNormal( entk ) ;
#else
		entk->think2 = TH2_CLE_COMP ;
		if ( entk->id == 0 ) {
			entk->think3 = TH3_CLEAR_END ;
		} else {
			entk->think3 = TH3_ATTACK_SETUP ;
		}
#endif
		entk->count3 = 0 ;
		return ;
	}

	/* 仲間のサポート 他のnoticeよりサポート優先 */
	if ( Cle->c_notice & CLE_NOTICE_SUPPORT ) {
		entk->think2 = TH2_NOTICE_SUPPORT ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ; 
		return ;
	}

	/* 他モードへ移項 */
	ClearingChange( entk ) ;
}

static	void	ClearingCompModeCheack( entk )
ENETHINK	*entk ;
{
	/* 他モードへ移項 */
	ClearingCompChange( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ENE_AttackerStartModeAvoidEndClearing( entk ) ;
	}

	NoiseChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		ENE_AttackerStartModeAvoidEndClearing( entk ) ;
	}

	TraceChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		if ( entk->c_notice & ( ENE_NOTICE_TRACE|ENE_NOTICE_NOISE ) ) {
			/* クリアリング終了 */
			UNSET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;
		}
		ENE_AttackerStartModeAvoidEndClearing( entk ) ;
	}

	BoxChange( entk ) ;
}

static	void	NoticeSupportCheck( entk )
ENETHINK	*entk ;
{
	if ( !(Cle->c_notice) ) {
		if ( Cle->scene_flag == CLE_ENE_COMPLETE ) {
			entk->think2 = TH2_CLE_COMP ;
			entk->think3 = TH3_ATTACK_SETUP ;
			entk->count3 = 0 ;
		} else {
			entk->think2 = TH2_CLEARING ;
			entk->think3 = TH3_GO_NEXT ;
			entk->count3 = 0 ;
		}
	}

	if ( ClearingGoNormalCheck( entk ) ) {
		return ;
	}

	NoticeSupportChange( entk ) ;
}

static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		if ( entk->scene >= PA_TIME_CLE_START ) {
//			ENE_AttackerStartModeAvoidEndClearing( entk ) ;
			/* クリアリングスタートしていたら通常回避に戻る */
			ENE_AttackerStartModeEndClearingToNormal( entk ) ;
		} else {
			/* クリアリングスタートしていなかったらクリアリングに戻る */
			ENE_AttackerStartModeAvoidReturnClearing( entk ) ;
		}
	}

	EneDamageChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		if ( entk->scene >= PA_TIME_CLE_START ) {
//			ENE_AttackerStartModeAvoidEndClearing( entk ) ;
			/* クリアリングスタートしていたら通常回避に戻る */
			ENE_AttackerStartModeEndClearingToNormal( entk ) ;
		} else {
			/* クリアリングスタートしていなかったらクリアリングに戻る */
			ENE_AttackerStartModeAvoidReturnClearing( entk ) ;
		}
	}

	DamageChange( entk ) ;
}

static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		回避（クリアリング）
	*/
void	ENE_Attacker_Think1_Avoid( entk )
ENETHINK		*entk ;
{
	entk->status2 = 0 ;

	SET_FLAG( entk->thk_status, THK_STATUS_CLEARING ) ;
	switch ( entk->think2 ) {
	    case TH2_MOVE :				/* 指定位置まで移動 */
			Think2_MoveClearingStartP( entk ) ;
			ClearingModeCheackMove( entk ) ;
		break ;
	    case TH2_CLEARING :			/* クリアリング */
			Think2_Clearing( entk ) ;
			ClearingModeCheack( entk ) ;
		break ;
	    case TH2_CLE_COMP :			/* クリアリング */
			Think2_ClearingComp( entk ) ;
			ClearingCompModeCheack( entk ) ;
		break ;
	    case TH2_NOTICE_SUPPORT :		/* 他人サポート */
			Think2_NoticeSupport( entk ) ;
			NoticeSupportCheck( entk ) ;
		break ;

	    case TH2_TRACE :			/* 跡、追跡モード */
			THK_TraceMode( entk ) ;
			TraceModeCheck( entk ) ;
		break ;
	    case TH2_NOISE :			/* 物音モード */
			THK_NoiseMode( entk ) ;
			NoiseModeCheck( entk ) ;
		break ;
	    case TH2_BOX :				/* ダンボールモード */
			THK_BoxMode( entk ) ;
			BoxModeCheck( entk ) ;
		break ;
	    case TH2_ENE_DAMAGE :		/* 味方ダメージ発見 */
			THK_EneDamage( entk ) ;
			EneDamageModeCheck( entk ) ;
		break ;
	    case TH2_DAMAGE :			/* ダメージモード */
			THK_Damage( entk ) ;
			DamageModeCheck( entk ) ;
		break ;

	    case TH2_CHASE :			/* プレイヤーを見失った時点まで移動 */
			Think2_LastPlayerPoint( entk ) ;
			ClearingModeCheack( entk ) ;
		break ;
	    case TH2_SEARCH :			/* プレイヤーを見失った時点で探索 */
			Think2_Search( entk ) ;
			ClearingModeCheack( entk ) ;
		break ;

	    case TH2_DISCOVERY :		/* 発見 */
			THK_Discovery( entk ) ;
			DiscoveryModeCheck( entk ) ;
		break ;

	    case TH2_WITHDRAW :			/* 引き上げ */
			Think2_Withdraw( entk ) ;
		break ;
	}

#ifdef DEBUG_MODE
if ( GM_Debug2PMode == GM_DEBUG_MODE_ENEMY_THINK ) {
	extern	CLEARING	*Cle ;
	DEBUG_Locate( 40 + ( 22 ), 15*23, 0 );
	DEBUG_Printf( "flag=%2d know=%2d \n", Cle->scene_flag , Cle->iknow_flag );
}
#endif
}

/*----- スタートモード、攻撃兵回避 --------------------------------------------*/
void	ENE_AttackerStartModeAvoid( entk )
ENETHINK	*entk ;
{
	extern void ReregistTarget(void *,OBJECT *,int,FVECTOR *) ;
	FVECTOR	l_pos ;
	int		l_map ;

	entk->act->CheckPad = AttackerAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->avoid = MAX_AVOID_LEVEL ;
	entk->c_notice = 0 ;
	SET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;

	entk->think1 = ENE_TH1_AVOID ;
	if ( COM_GetUnitBuff(entk,CLE_BUFF_KIND) == CLE_TH2_CLEARING ) {
printf("CLEARING!!!\n") ;
		entk->think2 = TH2_MOVE ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;

		entk->rnavi2->c_route = COM_GetCleRoute( entk->id ) ;
		entk->rnavi2->next_node = COM_GetClePoint( entk->id ) ;
		ENE_SetTrgpCSP(  &(entk->trgpoint), COM_GetCleRoute( entk->id ), COM_GetUnitBuff(entk,CLE_BUFF_AREA) ) ;
//entk->trgpoint.pos.vz -= (float)(150*entk->id) ;

		entk->scene = 0 ;
		entk->act->move_s = MoveCautionRun ;
		SET_FLAG( entk->iknow_flag, IKNOW_AVOID_CLEARING ) ;

		/* クリアリングデータ初期化の後で修理 */
		if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
			entk->think2 = TH2_DAMAGE ; 
			entk->c_notice = 0 ;
			THK_DamageMedicalModeStart( entk ) ;
			return ;
		}
	} else {
		/* 現在は意味無し、この後で重野君のほうで設定される */
printf("SEARCH!!!\n") ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		
		COM_GetPlayerLastPos( &l_pos, &l_map ) ;
		ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
		entk->scene = CLE_ENE_MOVE_START ;
		entk->act->move_s = MoveRun ;
		UNSET_FLAG( entk->iknow_flag, IKNOW_AVOID_CLEARING ) ;
	}

	/* ＩＫ位置補正 */
	printf(" arm ik pointer[%x]\n",entk->arm_ik ) ;
	if ( entk->arm_ik == NULL ) {
		entk->arm_ik = InitArmIK(entk->act->body, HUMAN21_HIDARI_TE,
			entk->buddy->act->body, HUMAN21_HIDARI_KATA,&armik_shift);
	printf("atavoid: arm ik pointer[%x]\n",entk->arm_ik ) ;
	} else {
		ReregistTarget(  entk->arm_ik, NULL, -1 , &armik_shift ) ;
	}

#if 1//ワープ実験
	{
		int old_addr ;
		
		old_addr = entk->ctrl->addr ;
		ENE_CheckWarpNearPos( entk->act->body, entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;
		printf(" attacker avoid warp before[%d] after[%d] \n",old_addr, entk->ctrl->addr ) ;
		if ( old_addr != entk->ctrl->addr ) {
			entk->think3 = TH3_WARP_WAIT ;
		} else {
			ENETHINK *before ;
			int dis ;
			
			before = COM_GetMemberFromGUI( entk->g_id, entk->u_id, entk->id ) ;
			dis = _FVecTrgDis( &entk->ctrl->mov, &before->ctrl->mov ) ;
			printf(" attacker avoid bind dis[%d] \n", dis ) ;
			if ( dis < 300 ) {
				entk->think3 = TH3_WARP_WAIT ;
			}
		}
	}
#endif
}

void	ENE_AttackerStartModeAvoidWarp( entk )
ENETHINK	*entk ;
{
	ENE_AttackerStartModeAvoid( entk ) ;
	ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;
}

void	ENE_AttackerStartModeAvoidEndClearing( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = AttackerAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->keep_pad = -1 ;
	entk->act->pad = 0 ;
	entk->c_notice = 0 ;

	entk->think1 = ENE_TH1_AVOID ;

	/* 怪我したら治療して通常回避へ */
	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

printf("RETURN CLEARING!!!\n") ;
	if ( Cle->scene_flag == CLE_ENE_COMPLETE || entk->avoid == 0 ) {
printf("Clearing End Scene Complete !! flag[%d] avoid[%d]\n", Cle->scene_flag, entk->avoid);
		/* クリアリングが終了したり、ダメージを受けた後 entk->avoid == 0 になる */
		Cle->scene_flag = CLE_ENE_COMPLETE ;
		entk->think2 = TH2_CLE_COMP ;
		entk->think3 = TH3_ATTACK_SETUP ;
	} else if( entk->iknow_flag & IKNOW_CLEARING ) {
printf("Clearing Continue !!\n");
		entk->think2 = TH2_CLEARING ;
		entk->think3 = TH3_ZONE_RETURN ;

		ENE_SetTrgpNode( entk->rnavi2, &(entk->trgpoint) ) ;
		entk->act->move_s = MoveCautionWalk ;
	} else {
printf("Clearing END !! \n");
		/* notice後クリアリング終了条件
			・Traceモードでイントルードやロッカーの中まで調べた後
			・Noiseモードでイントルードやロッカーの中まで調べた後
			・Trace,Noiseに続けて発生したダンボールモードの後
		*/
		SET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;
		Cle->scene_flag = CLE_ENE_COMPLETE ;

		entk->think2 = TH2_CLE_COMP ;
		entk->think3 = TH3_CLEAR_END ;
	}

	entk->count3 = 0 ;
	entk->avoid = MAX_AVOID_LEVEL ;

	if ( ClearingGoNormalCheck( entk ) ) {
		printf("ENE_AttackerStartModeAvoidEndClearing Go Normal \n");
		return ;
	}
}

void	ENE_AttackerStartModeAvoidReturnClearing( entk )
ENETHINK	*entk ;
{
#if 1
	/* クリアリングスタート前なので必ずゾーン移動からはじめる */
	if ( !(entk->iknow_flag & IKNOW_AVOID_CLEARING) ) {
		ENE_AttackerStartModeAvoid( entk ) ;
		return ;
	}
	entk->act->CheckPad = AttackerAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->avoid = MAX_AVOID_LEVEL ;
	entk->c_notice = 0 ;
	SET_FLAG( entk->iknow_flag, IKNOW_CLEARING ) ;

	entk->think1 = ENE_TH1_AVOID ;
printf("CLEARING!!!\n") ;
	entk->think2 = TH2_MOVE ;
	entk->think3 = TH3_ZONE_CHASE ;
	entk->count3 = 0 ;

//まだrnavi2がセットされていないENE_SetTrgpNode( entk->rnavi2, &(entk->trgpoint) ) ;
	entk->act->move_s = MoveCautionRun ;

	/* 怪我したら治療へ */
	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

#else
	if ( !(entk->iknow_flag & IKNOW_CLEARING) ) {
		ENE_AttackerStartModeAvoid( entk ) ;
		return ;
	}

	entk->act->CheckPad = AttackerAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->c_notice = 0 ;

	entk->think1 = ENE_TH1_AVOID ;


printf("RETURN CLEARING!!!\n") ;
	entk->think2 = TH2_CLEARING ;
	entk->think3 = TH3_ZONE_RETURN ;
	entk->count3 = 0 ;

	ENE_SetTrgpNode( entk->rnavi2, &(entk->trgpoint) ) ;
	entk->act->move_s = MoveCautionRun ;
#endif
}

void	ENE_AttackerClearingModeDamage( entk )
ENETHINK	*entk ;
{
	entk->think1 = ENE_TH1_AVOID ;

	if ( ENE_PlayerHoldCheck( entk ) ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}

	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
		entk->mess_notice == MES_NOTICE_GOOFY ||
		entk->mess_notice == MES_NOTICE_SMOKE ) {
		if ( entk->scene >= PA_TIME_CLE_START ) {
			ENE_AttackerStartModeAvoidEndClearing( entk ) ;
		} else {
			ENE_AttackerStartModeAvoidReturnClearing( entk ) ;
		}
		return ;
	}

	/* 起き上がりでなければ発見 */
//	if ( !(entk->iknow_flag & IKNOW_DOWNED) ) {
	if ( (entk->iknow_flag & IKNOW_CLEARIMG_DAMAGE) ) {
		entk->think2 = TH2_DISCOVERY ; 
printf("IKNOW_CLEARIMG_DAMAGE Go Discovery\n");
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}

	entk->think2 = TH2_DAMAGE ; 
	entk->c_notice = 0 ;
	THK_DamageModeStart( entk ) ;
}

void	ENE_AttackerStartModeEndClearingToNormal( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = AttackerAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->keep_pad = -1 ;
	entk->act->pad = 0 ;
	entk->c_notice = 0 ;

	entk->think1 = ENE_TH1_AVOID ;

	if ( !(Cle->iknow_flag & CLE_IKNOW_GO_NORMAL) ) {
		COM_ClearingGoNormal( ) ;
	}
	ClearingGoNormalCheck( entk ) ;
printf("Clearing End To Normal Avoid!![%d]\n",entk->id);
}
