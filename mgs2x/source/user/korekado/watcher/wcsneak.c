//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wcsneak.c
	巡回兵 潜入モード
	
	1999/07/29 Y.Korekado
	$Id: wcsneak.c,v 1.1.1.3 2002/11/19 11:44:32 Yoshizawa1 Exp $
	
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
#include	"watcher.h"

//#define COMBAT_TEST (1) /* 戦闘実験 */

#include "BP_Misc.h"

#define FACE_ONLY_EX	(1)
#define TEST_ROUTE_CHECK	(1)
#define LAST_RADIOPOS_ROUTE	(1)


enum {
	TH2_TRAVEL, TH2_DAMAGE, TH2_ENE_DAMAGE, TH2_NOISE, TH2_CORPS, 
	TH2_HOLDUP, TH2_INDISTINCT, TH2_TOUCH, TH2_DISCOVERY, TH2_FOUND,
	TH2_TRACE, TH2_BOX, TH2_UNIFORM, TH2_INVESTIGATED, TH2_ACCIDENT_REPO,
	TH2_RADIO, 
} ;
enum {
	TH3_RETURN_TRAVEL,TH3_GO_NEXT,TH3_POINT_ACTION,TH3_DISCOVERY_POSE,TH3_RADIO_CALL,
	TH3_WAIT, TH3_GO_NEXT_END, TH3_POINT_MOTION, TH3_POINT_FLAG, TH3_NSIGHT_OFF,
	TH3_PINPOINT_MOVE
} ;
enum {
	SP_NONE,
	SP_DISCOVERY,	/* 発見ポーズ */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_GOTO_END,	/* 体の向きを変えながら直進 */
	SP_AKUBI,
	SP_SENOBI,
	SP_ZZZ,
	SP_PORIPORI,
	SP_OTTOTTO,
	SP_NORINORI,
	SP_GLASS,		/*10*/
	SP_NSIGHT_OFF,
	SP_ZZZ_NORINORI,
	SP_AROUND,		/* 左右(45度)を確認 */
	SP_FUNAYOI,
	SP_PINPOINT,
	SP_STAND_01,
	SP_STAND_02,
	SP_ONETIME_01,
	SP_ONETIME_02,
	SP_ONETIME_03,	/*20*/
	SP_IJIIJI,
	SP_UNREAL,
	SP_KATAGURU,
	SP_CLEAR_SIGN,
	SP_RADIO_CALL,	/* 無線連絡、*/
	SP_RADIO_RETURN, /* 無線しまう */
	SP_HOUNYOU, /* おしっこ */
	SP_HOUNYOU_END, /* 放尿終わり */
	SP_PEEP_R, /* 覗き込み右 */
	SP_PEEP_L, /* 覗き込み左 */
	SP_ROLLING_R, /* 横転右 */
	SP_ROLLING_L, /* 横転左 */

	/* 注！！移動変更アクションパッドはこれ以上の値にすること */
	SP_MV_SIDE_L,	/* 左横移動 */
	SP_MV_SIDE_R,	/* 30 右横移動 */
	SP_MV_BACK,		/* バック */
	SP_MV_STAIR_RUN_D,	/* 階段走り降り */
	SP_MV_STAIR_RUN_U,	/* 階段走り降り */
	SP_MV_STAIR_WALK_D,	/* 階段走り降り */
	SP_MV_STAIR_WALK_U,	/* 階段走り降り */
	SP_MV_BACK_CAUTION,		/* バック */
	SP_MV_RUN,		/* 走る */
	SP_MV_ATK_RUN,		/* 攻撃兵走り */
	SP_MV_NORMAL,		/* 通常移動 */
} ;

#define SP_STAGE_ONETIME	0x1000
#define SP_STAGE_LOOP		0x2000
#define SP_STAGE_SPHERE100	0x4000

static int	PointAction[]={
	SP_NONE,
	SP_AKUBI,
	SP_SENOBI,
	SP_ZZZ,
	SP_GLASS,
	SP_PORIPORI,
	SP_OTTOTTO,
	SP_NORINORI,
	SP_AROUND,		/* 左右(45度)を確認 */
	SP_FUNAYOI,
	SP_STAND_01,	/* 10 */
	SP_STAND_02,
	SP_ONETIME_01,
	SP_ONETIME_02,
	SP_IJIIJI,
	SP_UNREAL,
	SP_KATAGURU,
	SP_CLEAR_SIGN,
	SP_CAUT_STAND,	/* 警戒立ち 18 */
	SP_ONETIME_03,
	SP_MV_SIDE_L,	/* 左横移動 */
	SP_MV_SIDE_R,	/* 右横移動 */
	SP_MV_RUN,		/* 走る 22*/
	SP_RADIO_CALL,	/* 無線連絡、*/
	SP_HOUNYOU, /* おしっこ 24*/
	SP_MV_ATK_RUN,		/* 攻撃兵走り25 */

	SP_PEEP_R, /* 覗き込み右 */
	SP_PEEP_L, /* 覗き込み左 */
	SP_ROLLING_R, /* 横転右 */
	SP_ROLLING_L, /* 横転左29 */
	SP_MV_NORMAL,		/* 通常移動 */
	SP_MV_BACK,		/* バック */
} ;

#include	"wcsneact.c"

/*----- --------------------------------------------*/
#if 0	//no use
static void SetThinkNearRootPoint( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	entk->think1 = ENE_TH1_SNEAK ; 
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;
}
#endif
/*----- --------------------------------------------*/
#define _AROUND_BRANK COUNT_VMODE(270)
static void SetAroundAim( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

#if 0
	if ( (entk->count3%270) == 90 ) {
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( (entk->count3%270) == 180 ) {
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	} else if ( (entk->count3%270) == 1 ) {	/* turn.vyにセットされてから */
		entk->act->aim_dir = entk->ctrl->turn.vy  ;
	}
#endif
// printf(" aim_dir[%d] turn[%d]\n",entk->act->aim_dir,entk->ctrl->turn.vy ) ;
}

static void RouteGoal( ENETHINK *entk )
{
	/* proc call */
	COM_RouteProcCall( entk->rnavi->c_route, entk->rnavi->next_node, entk ) ;
#ifdef LAST_RADIOPOS_ROUTE
	ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
#endif
}

#define WC_ACT_RETURN_RADIO_TIME	COUNT_VMODE(99)

static void RadioCall( entk )
ENETHINK	*entk ;
{

	SET_FLAG( entk->thk_status, THK_STATUS_ROOT_RADIO ) ;

	if ( entk->count3 == 0 ) {
		entk->rnavi->p_acttime = MAX_VOL_TIME ;
		if ( entk->sw.radio & SW_FLAG_BREAK || GM_GameStatus & STATE_CHAFF ) {
			SET_FLAG( entk->notice, ENE_NOTICE_RADIO ) ;
		}
	}

	if ( entk->act->act_end == 3 ) {
		entk->rnavi->p_acttime = COM_CallRouteVoice(entk->rnavi->c_route,entk->rnavi->next_node,EV_RAD_ROUTE,entk ) ;
		entk->rnavi->p_acttime += WC_ACT_RETURN_RADIO_TIME ;
//printf("RadioCall:COM_CallRouteVoice acttime[%d]\n",entk->rnavi->p_acttime);
		if ( entk->rnavi->p_acttime > 0 ) {
#ifndef LAST_RADIOPOS_ROUTE
			ENE_SetLastRadioPos( entk, &entk->ctrl->mov, entk->ctrl->hzx_id ) ;
#endif
		}
	}
//printf("RadioCall:entk->rnavi->p_acttime[%d]\n",entk->rnavi->p_acttime);

//	if ( entk->rnavi->p_acttime == 0 ) {
	if ( entk->rnavi->p_acttime == WC_ACT_RETURN_RADIO_TIME ) {
		entk->rnavi->p_action = SP_RADIO_RETURN ;
	}
}

#define HOUNYOU_END_TIME	COUNT_VMODE(100)	/* 放尿を止めるのに掛る時間 */
static void Hounyou( entk )
ENETHINK	*entk ;
{
	/* シナリオからのメッセージ待ちなら */
	if ( entk->rnavi->p_acttime < 0 ) return ;

	if ( entk->rnavi->p_acttime < HOUNYOU_END_TIME ) {
		entk->act->pad = SP_HOUNYOU_END ;
	}
}

static void PointActionManage( entk )
ENETHINK	*entk ;
{
	int pad ;

	entk->act->pad = entk->rnavi->p_action ;

	pad = entk->act->pad & 0xfff ;

	if ( pad >= SP_MV_SIDE_L ) {	/*移動モーションなら立ち*/
		entk->act->pad = SP_NONE ;
	}

	switch ( entk->act->pad ) {
		case SP_AROUND :
			SetAroundAim( entk ) ;
		break ;
		case SP_RADIO_CALL :
			RadioCall( entk ) ;
		break ;
		case SP_HOUNYOU :
			Hounyou( entk ) ;
		break ;
	}
}

static int PointTimeManage( entk )
ENETHINK	*entk ;
{
	if ( entk->rnavi->p_acttime == 0 ) return 1 ;

	if ( entk->rnavi->p_acttime == PTIME_MESWAIT ) {
		if ( entk->receive&ENE_ORDER_ACTION_END  ) return 1 ;
	}
	
	return 0 ;
}

static void RouteVoice( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		int time ;
		if ( entk->rnavi->p_actstatus & PA_CON_TALK_ONESELF ) {
				time = COM_CallRouteVoice(entk->rnavi->c_route,
						entk->rnavi->next_node,EV_TALK_ONESELF,entk ) ;
			if ( entk->rnavi->p_acttime == PTIME_VOICEEND ) {
				entk->rnavi->p_acttime = time ;
printf("RouteVoice Time[%d]\n",time ) ;
			}
		}
	}
}

static int RouteCheck( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;

	rnavi = entk->rnavi ;
	if ( rnavi->next_route == rnavi->c_route ) return 0 ;

	printf("ene[%d] Change Root [%d]->[%d]\n",entk->id, rnavi->c_route, rnavi->next_route ) ;
	ENE_ChangeRoute( entk,rnavi->next_route ) ;

	if ( entk->rnavi->chang_node < 0 ) {
		ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		entk->rnavi->next_node = entk->rnavi->chang_node ;
		ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
	}
	entk->think1 = ENE_TH1_SNEAK ; 
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;

	return  1 ;
}

static int RouteCheckContinue( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;

	rnavi = entk->rnavi ;
	if ( rnavi->next_route == rnavi->c_route ) return 0 ;

	printf("ene[%d] Change Root [%d]->[%d]\n",entk->id, rnavi->c_route, rnavi->next_route ) ;
	ENE_ChangeRoute( entk,rnavi->next_route ) ;

	if ( entk->rnavi->chang_node < 0 ) {
		ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		entk->rnavi->next_node = entk->rnavi->chang_node ;
		ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
	}
	return  1 ;
}


static void SetPointAction( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;

	rnavi = entk->rnavi ;

#if 1
	rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node] ;
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node] ;
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node] ;

	if ( rnavi->p_actstatus & PA_CON_STAGE_ONE ) {
printf("wcsneak: stage one time action\n");
		if ( rnavi->n_nodes == 1 ) {
			/* アクションポイント１の場合はLoopMotion再生 */
			rnavi->p_action = SP_STAGE_LOOP | rnavi->pa_action[(int)rnavi->next_node] ;
printf("wcsneak: stage one time action and n_point = 1!!!!!!!!!!\n");
		} else {
			rnavi->p_action = SP_STAGE_ONETIME | rnavi->pa_action[(int)rnavi->next_node] ;
		}
		if ( rnavi->p_actstatus & PA_CON_NEAR_WALL ) {
			rnavi->p_action |= SP_STAGE_SPHERE100 ;
		}
	} else if ( rnavi->p_actstatus & PA_CON_STAGE_LOOP ) {
printf("wcsneak: stage loop action\n");
		rnavi->p_action = SP_STAGE_LOOP | rnavi->pa_action[(int)rnavi->next_node] ;
		if ( rnavi->p_actstatus & PA_CON_NEAR_WALL ) {
			rnavi->p_action |= SP_STAGE_SPHERE100 ;
		}
	} else {
		rnavi->p_action = PointAction[ rnavi->pa_action[(int)rnavi->next_node] ];
	}
#else
	rnavi->p_action = SP_GLASS ;
//	rnavi->p_acttime = PTIME_MOTIONEND ;
	rnavi->p_acttime = COUNT_VMODE(180) ;
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node] ;
	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node] ;
#endif

	/* 独り言で音声終了待ちだったら */
//	if ( rnavi->p_acttime == PTIME_VOICEEND ) 	rnavi->p_acttime = MAX_VOL_TIME ;

   if ( BP_IsPAL()==TRUE )
   {
      if ( rnavi->p_acttime > 0 ) rnavi->p_acttime = COUNT_VMODE(rnavi->p_acttime) ;
   }

	if ( entk->status & ENE_STATUS_NORINORI ) {
		if ( GM_AlertMode != ALERT_MODE_SEARCH ) {
			if ( rnavi->p_action == 0 ) {
				rnavi->p_action = SP_NORINORI ;
			} else if ( rnavi->p_action == 3 ) {
				rnavi->p_action = SP_ZZZ_NORINORI ;
			}
		}
	}

	if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
		if ( rnavi->p_actstatus & PA_CON_COUTION_STAND ) {
			rnavi->p_action = SP_CAUT_STAND ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_STAGE_ONE ) ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_STAGE_LOOP ) ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_NEAR_WALL ) ;
			if ( rnavi->p_acttime == -1 ) 	rnavi->p_acttime = COUNT_VMODE(180) ;
		}
		if ( rnavi->p_actstatus & PA_CON_COUTION_LOOK ) {
			rnavi->p_action = SP_AROUND ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_STAGE_ONE ) ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_STAGE_LOOP ) ;
			UNSET_FLAG( rnavi->p_actstatus, PA_CON_NEAR_WALL ) ;
			if ( rnavi->p_acttime == -1 ) 	rnavi->p_acttime = COUNT_VMODE(180) ;
		}
		if ( (rnavi->p_action == SP_AKUBI) ||
			 (rnavi->p_action == SP_SENOBI) ||
			 (rnavi->p_action == SP_ZZZ) ) {
			rnavi->p_action = SP_AROUND ;
			if ( rnavi->p_acttime == -1 ) 	rnavi->p_acttime = COUNT_VMODE(180) ;
		}
	}
	if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
		if ( entk->act->bodyp.type & ENE_TYPE_SUPPORT ) {
//			rnavi->p_action = SP_UNREAL ;
		}
	}

#ifdef DEBUG_MODE
if ( entk->g_id==0 && entk->u_id==1 && entk->id==0 ) {
	printf("[%d]:act[%d] time[%d] dir[%d]",(int)rnavi->next_node,rnavi->p_action,rnavi->p_acttime,rnavi->p_dir);
	printf("status[%x]\n",rnavi->p_actstatus ) ;
}
#endif
}

static int SelectPAMode( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;

	rnavi = entk->rnavi ;

	if ( rnavi->p_action == SP_GLASS ) {
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	}

	if ( (rnavi->p_acttime == PTIME_MOTIONEND) || 
		( entk->rnavi->p_actstatus & PA_CON_STAGE_ONE ) ) {
		return TH3_POINT_MOTION ;
	} else if ( rnavi->p_acttime == PTIME_FLAGWAIT ) {
		return TH3_POINT_FLAG ;
	}

	return TH3_POINT_ACTION ;
}

/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_ReturnTravel( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {

		if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 2 ) {
			entk->think3 = TH3_NSIGHT_OFF ; 
		} else {
			entk->think3 = TH3_GO_NEXT ;
		}
//		ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		ENE_SetTrgpNode( entk->rnavi, &(entk->trgpoint) ) ;
		SetPointAction( entk ) ; 
		entk->act->move_s = ( entk->status & ENE_STATUS_NORINORI )? MoveNoriNoriWalk : MoveWalk ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_NSightOff( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_OFF ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static int NextNode( rnavi )
ROUTENAVI	*rnavi ;
{
	int next ;

	next = rnavi->next_node + 1 ;
	if ( next >= rnavi->n_nodes ) {
		next = ( COM_StageKind() & ENE_STAGE_ROOT_NO_LOOP )? rnavi->n_nodes-1 : 0 ;
	}

	return next ;
}

static void ChangeMoveCheck( entk, sp_data )
ENETHINK	*entk ;
int	sp_data ;
{
	if ( sp_data < SP_MV_SIDE_L ) {
		if ( entk->rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
			entk->act->move_s = MoveWalkGun ;
		} else if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
			entk->act->move_s = MoveCautionWalk ;
		} else {
			entk->act->move_s = (entk->status & ENE_STATUS_NORINORI)? MoveNoriNoriWalk : MoveWalk ;
		}
		return ;
	}

	switch( sp_data ) {
		case SP_MV_SIDE_L :		/* 左横移動 */
			entk->act->move_s = MoveSideL ;
		break ;
		case SP_MV_SIDE_R :		/* 右横移動 */
			entk->act->move_s = MoveSideR ;
		break ;
		case SP_MV_RUN :		/* 走る */
			entk->act->move_s = MoveRun ;
		break ;
		case SP_MV_BACK :		/* バック */
			entk->act->move_s = MoveBack ;
		break ;
		case SP_MV_ATK_RUN :	/* 攻撃兵走り */
			entk->act->move_s = MoveAttackRun ;
		break ;
		case SP_MV_NORMAL :	/* 通常移動 */
			entk->act->move_s = MoveWalk ;
		break ;
	}
}

static int AutoAimAction( ENETHINK *entk )
{
	/* ＶＲのみ */
	if ( !(GM_GameStatus & STATE_VR_ANOTHER) ) return 1 ;

	if ( entk->rnavi->p_action == SP_MV_SIDE_L ) return 0 ;
	if ( entk->rnavi->p_action == SP_MV_SIDE_R ) return 0 ;
	if ( entk->rnavi->p_action == SP_MV_BACK ) return 0 ;

	return 1 ;
}

static int NoTimeAction( ENETHINK *entk )
{
	if ( entk->rnavi->p_action == 0 ) return 1 ;
	
	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		if ( !(COM_StageKind() & ENE_STAGE_NO_RUN_SMOOTH) ) {
			/* バグっていたが本編の影響を考えＶＲのみ修正 */
			if ( entk->rnavi->p_action >= SP_MV_SIDE_L ) return 1 ;
		}
	}

	return 0 ;
}

static void Think3_GoNext( entk )
ENETHINK	*entk ;
{
	float	f ;

	/* 巡回ポイントが１つ以上で、アクション無し、時間０なら */
	if ( ( entk->rnavi->n_nodes > 1 ) && entk->rnavi->p_acttime == 0 && NoTimeAction( entk ) ) {
		if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 500 ) < 0 ) {
			RouteGoal( entk ) ;
			ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
			SetPointAction( entk ) ;
			entk->act->dir = entk->ctrl->turn.vy ;
			entk->ctrl->interp = 30 ;

			if ( entk->rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_FACE ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_AIM_GUN_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_FACE_ONLY_MOVE ) {
				SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else {
				if ( AutoAimAction( entk ) ) {
					entk->status2 |= ENE_STATUS2_AIM_FACE_Y ;
					f = entk->ctrl->levels[0] - entk->act->aim_pos.vy ;
					if ( f > 1500.0f || f < -1000.0f ) {
						entk->status2 |= ENE_STATUS2_AIM_FACE_X ;
						entk->act->aim_pos.vy += 1000 ; /* ルートは床の高さではられるため */
					}
					entk->act->aim_pos = entk->rnavi->nodes[ (int)entk->rnavi->next_node ] ;
				}
			}

			ChangeMoveCheck( entk, entk->rnavi->p_action ) ;

			entk->think3 = TH3_GO_NEXT ; 
			entk->count3 = 0 ;
			return ;
		} else {
			if ( entk->rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_FACE ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_AIM_GUN_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_FACE_ONLY_MOVE ) {
				SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else {
				if ( AutoAimAction( entk ) ) {
					if ( entk->trgpoint.h_dis < 800 ) {
						if ( RouteCheck( entk ) ) return ;

						entk->status2 |= ENE_STATUS2_AIM_FACE_Y ;
						entk->act->aim_pos = entk->rnavi->nodes[ (int)NextNode( entk->rnavi ) ] ;
					} else if ( entk->count3 < COUNT_VMODE(30) ) {
						entk->status2 |= ENE_STATUS2_AIM_FACE_Y ;
						entk->act->aim_pos = entk->rnavi->nodes[ (int)entk->rnavi->next_node ] ;
					} else {
						entk->act->aim_pos = entk->rnavi->nodes[ (int)entk->rnavi->next_node ] ;
					}

					f = entk->ctrl->levels[0] - entk->act->aim_pos.vy ;
					if ( f > 1500.0f || f < -1000.0f ) {
						entk->status2 |= ENE_STATUS2_AIM_FACE_X ;
						if ( GM_GameStatus & STATE_VR_ANOTHER ) {
							if ( f > 0 ) entk->status2 |= ENE_STATUS2_AIM_S_LIMIT ;
						}
						entk->act->aim_pos.vy += 1000 ; /* ルートは床の高さではられるため */
					}
				}
			}
		}
	} else {
		if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 450 ) < 0 ) {
			if ( entk->rnavi->p_acttime == 0 ) {
				FVECTOR	pos ;
				int next ;

				if ( RouteCheck( entk ) ) return ;

				next = NextNode( entk->rnavi ) ;
				pos = entk->rnavi->nodes[ next ] ;
				/* 体の向きを次のポイントの方向 */
				entk->trgpoint.tmp_dirbuff = _FVecTrgDir2( &(entk->trgpoint.pos), &pos ) ;
			} else {
				/* 体の向きをアクション方向 */
				entk->trgpoint.tmp_dirbuff = entk->rnavi->p_dir ;
			}
			if ( entk->ctrl->mov.vx != entk->trgpoint.pos.vx ||
				entk->ctrl->mov.vz != entk->trgpoint.pos.vz  ) {
				/* 移動方向を次のポイントの方向へ */
				entk->act->dir  = _FVecTrgDir2( &(entk->ctrl->mov), &(entk->trgpoint.pos) ) ;
			} else {
				/* 移動方向をアクション方向へ */
				entk->act->dir  = entk->rnavi->p_dir ;
			}
			entk->act->body_dir = entk->trgpoint.tmp_dirbuff ;

			if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
				entk->status2 |= ENE_STATUS2_AIM_FACE ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
				SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else {
				if ( AutoAimAction( entk ) ) {
					entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
					entk->act->aim_dir = entk->rnavi->p_dir ;
				}
			}

			if ( entk->rnavi->p_actstatus & PA_CON_PINPOINT  ) {
				entk->think3 = TH3_PINPOINT_MOVE ;

			} else if ( (entk->act->move_s == MoveSideL) ||
						(entk->act->move_s == MoveSideR) ||
						(entk->act->move_s == MoveBack) ) {
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					/* バグっていたが本編の影響を考えＶＲのみ修正 */
					RouteGoal( entk ) ;
					entk->act->dir = entk->rnavi->p_dir ;
					if ( entk->act->move_s == MoveSideL ) {
						entk->act->dir += 1024 ;
					} else if ( entk->act->move_s == MoveSideR ) {
						entk->act->dir -= 1024 ;
					} else {
						entk->act->dir += 2048 ;
					}
					entk->think3 = SelectPAMode( entk ) ; 
				} else {
					entk->think3 = TH3_GO_NEXT_END ; 
				}
			} else {
				entk->think3 = TH3_GO_NEXT_END ; 
			}
			entk->count3 = 0 ;
			return ;
		} else {
			if ( entk->rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_FACE ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_AIM_GUN_MOVE ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & PA_CON_FACE_ONLY_MOVE ) {
				SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else {
				if ( AutoAimAction( entk ) ) {
					if ( entk->count3 < COUNT_VMODE(30) ) {
						entk->status2 |= ENE_STATUS2_AIM_FACE_Y ;
						entk->act->aim_pos = entk->rnavi->nodes[ (int)entk->rnavi->next_node ] ;
					} else {
						entk->act->aim_pos = entk->rnavi->nodes[ (int)entk->rnavi->next_node ] ;
					}

					f = entk->ctrl->levels[0] - entk->act->aim_pos.vy ;
					if ( f > 1500.0f || f < -1000.0f ) {
						entk->status2 |= ENE_STATUS2_AIM_FACE_X ;
						entk->act->aim_pos.vy += 1000 ; /* ルートは床の高さではられるため */
					}
				}
			}
		}
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;

//if ( entk->id == 2 ) printf("go next dir[%d]  \n",entk->act->dir);
}

static void Think3_GoNextEnd( entk )
ENETHINK	*entk ;
{
	int flag ;

	flag = 0 ;
	if ( !(_PosInRangeXZ( &entk->ctrl->mov, &(entk->trgpoint.pos), 500 )) ) {
		flag = 1 ;
	}
	if( entk->count3 > COUNT_VMODE(60) ) {
		flag = 1 ;
	}
	if( entk->count3 > COUNT_VMODE(10) && GV_DiffDirAbs( entk->ctrl->rot.vy, entk->trgpoint.tmp_dirbuff ) < 64 ) {
		flag = 1 ;
	}
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 50 ) < 0 ) {
		flag = 1 ;
	}

	if ( flag ) {
		RouteGoal( entk ) ;
		entk->act->dir = entk->rnavi->p_dir ;
		entk->think3 = SelectPAMode( entk ) ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->pad = SP_GOTO_END ;

	if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->rnavi->p_dir ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->act->body_dir = entk->trgpoint.tmp_dirbuff ;

//printf("dir[%d] bodydir[%d] \n",entk->act->dir,entk->act->body_dir);

	entk->count3 ++ ;
}

static void Think3_PinpointMove( entk )
ENETHINK		*entk ;
{
	if( entk->count3 == 0 ) {
		entk->act->target_pos = entk->trgpoint.pos ;
	}

	entk->act->pad = SP_PINPOINT ;

	if ( entk->act->act_end ) {
		RouteGoal( entk ) ;
		entk->think3 = SelectPAMode( entk ) ; 
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		entk->status2 |= ENE_STATUS2_AIM_FACE ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->rnavi->p_dir ;
	}

	entk->act->dir = entk->rnavi->p_dir ;

	entk->count3 ++ ;

}

static void Think3_PointAction( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	int old_pad ;

	old_pad = entk->act->pad ;
	rnavi = entk->rnavi ;

	SET_FLAG( entk->thk_status, THK_STATUS_WATCH ) ;

	if ( entk->rnavi->p_acttime < 0 ) {
		if ( GM_CheckObject_IsEnd( entk->act->body, 0 ) ) {
			if ( RouteCheck( entk ) ) return ;
		}
		if ( entk->rnavi->p_action == SP_UNREAL ) {
			if ( RouteCheck( entk ) ) return ;
		}
	}

//	if ( rnavi->p_acttime <= 0) {
	if ( PointTimeManage( entk ) ) {
#ifdef TEST_ROUTE_CHECK
		if ( RouteCheckContinue( entk ) ) {
			if ( !ENE_InRange( &entk->ctrl->mov, &entk->trgpoint.pos, 450 ) ) {
				entk->think3 = TH3_RETURN_TRAVEL ; 
				entk->count3 = 0 ;
				return ;
			} else if ( entk->rnavi->n_nodes == 1 ) {
				SetPointAction( entk ) ;
				entk->think3 = SelectPAMode( entk ) ; 
			}
		}
#else
		if ( RouteCheck( entk ) ) return ;
#endif
		if ( rnavi->n_nodes > 1 ) {
			ENE_SetTrgpNextnode( rnavi, &(entk->trgpoint) ) ;
			SetPointAction( entk ) ;
			/* 目標地点が一定範囲いないなら */
			if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 450 ) < 0 ) {
				entk->think3 = SelectPAMode( entk ) ; 
				/* 向いている方向がアクションポイントの方向と違っていたら */
				if ( entk->ctrl->turn.vy != rnavi->p_dir ) {
					entk->act->dir = rnavi->p_dir ;
					entk->trgpoint.tmp_dirbuff = rnavi->p_dir ;
#if 1
					entk->think3 = TH3_GO_NEXT ; 
#else
					entk->act->body_dir = 4095 & GV_NearExp4P( entk->ctrl->turn.vy, entk->trgpoint.tmp_dirbuff ) ;
					entk->think3 = TH3_GO_NEXT_END ; 
#endif
				}
				if ( rnavi->p_action == SP_GLASS ) {
					entk->act->pad = rnavi->p_action ;
					SET_FLAG( entk->status2, (ENE_STATUS2_AIM_GUN|ENE_STATUS2_AIM_TURN) ) ;
					entk->count3 = 0 ;
					if ( (entk->think3 != TH3_GO_NEXT) && 
						 (entk->think3 != TH3_GO_NEXT_END) ) {
						RouteGoal( entk ) ;
					}
					return ;
				}
			} else {
				entk->think3 = TH3_GO_NEXT ;
			}
		}
#if 1 	// 特殊アクション処理
		if ( entk->rnavi->p_action == SP_UNREAL ) {
			/*アンリアル*/
			entk->act->pad = entk->rnavi->p_action ;
		}
		if ( old_pad == SP_RADIO_CALL ) {
			/* 無線モーションはパッドが０なら無視している */
			if ( entk->rnavi->p_action != SP_RADIO_CALL ) {
				entk->act->pad = SP_CAUT_STAND ;
			}
		}
#endif
		if ( rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE ) ;
			entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
		} else if ( rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ||
					( rnavi->p_action == SP_GLASS ) ) {
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;
			entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
			entk->count3 = 0 ;

			if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
				/* バグっていたが本編の影響を考えＶＲのみ修正 */
				ChangeMoveCheck( entk, entk->rnavi->p_action ) ;
			}
			if ( (entk->think3 != TH3_GO_NEXT) && 
				 (entk->think3 != TH3_GO_NEXT_END) ) {
				RouteGoal( entk ) ;
			}
			return ;
		} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
			entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
		} else {
			SET_FLAG( entk->status2, ENE_STATUS2_AIM_DIR_FACE ) ;
			entk->act->aim_dir = rnavi->p_dir ;
		}
		entk->count3 = 0 ;

		ChangeMoveCheck( entk, entk->rnavi->p_action ) ;

		if ( (entk->think3 != TH3_GO_NEXT) && 
			 (entk->think3 != TH3_GO_NEXT_END) ) {
			RouteGoal( entk ) ;
		}
		return ;
	}

	if ( rnavi->p_action == SP_GLASS ) {
		SET_FLAG( entk->status2, (ENE_STATUS2_AIM_GUN|ENE_STATUS2_AIM_TURN) ) ;
	} else if ( rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE ) ;
		entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
		entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
	} else if ( rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;
		entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
	}

	RouteVoice( entk ) ;
	PointActionManage( entk ) ;

	if ( rnavi->p_acttime > 0 ) rnavi->p_acttime -- ;

	entk->count3 ++ ;
}

static void Think3_PointMotion( entk )
ENETHINK	*entk ;
{
	ROUTENAVI	*rnavi ;
	
	rnavi = entk->rnavi ;

	SET_FLAG( entk->thk_status, THK_STATUS_WATCH ) ;

	/* ステージモーションでルートアクションが１っ個しかないものだけ特別 */
	if ( rnavi->p_actstatus & PA_CON_STAGE_ONE ) {
		if ( rnavi->n_nodes == 1 ) {
			if ( GM_CheckObject_IsEnd( entk->act->body, 0 ) ) {
				if ( RouteCheck( entk ) ) return ;
			}
		}
	}

	if ( entk->act->act_end == 1 ) {
#ifdef TEST_ROUTE_CHECK
if ( entk->id == 2 ) {
	printf(" act end !!!!!!!!!!!!\n");
}
		if ( RouteCheckContinue( entk ) ) {
			if ( !ENE_InRange( &entk->ctrl->mov, &entk->trgpoint.pos, 450 ) ) {
				entk->think3 = TH3_RETURN_TRAVEL ; 
				entk->count3 = 0 ;
				return ;
			} else if ( entk->rnavi->n_nodes == 1 ) {
				SetPointAction( entk ) ;
				entk->think3 = SelectPAMode( entk ) ; 
	printf(" act end next entk->think3[%d]\n",entk->think3);
			}
		}
#else
		if ( RouteCheck( entk ) ) return ;
#endif
		if ( entk->rnavi->n_nodes > 1 ) {
			ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
			SetPointAction( entk ) ;
			/* 目標地点が一定範囲いないなら */
			if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 450 ) < 0 ) {
				int y ;
				
				SetPointAction( entk ) ; 
				entk->think3 = SelectPAMode( entk ) ; 
				/* 向いている方向がアクションポイントの方向と違っていたら */

				if (entk->act->body->m_ctrl->mt3_ctrl[ 0 ].file_header->flag & MT3_FLAG_TURN_FLAG){
					y = MatToYRot( &BODYWORLD( entk->act->body, HUMAN21_KOSHI) ) ;
//printf("wcsneak.c:correct[%d]\n",y ) ;
				} else {
					y = entk->ctrl->turn.vy ;
				}

				if ( y != entk->rnavi->p_dir ) {
					entk->act->dir = entk->rnavi->p_dir ;
					entk->trgpoint.tmp_dirbuff = entk->rnavi->p_dir ;
					entk->act->body_dir = 4095 & GV_NearExp4P( entk->ctrl->turn.vy, entk->trgpoint.tmp_dirbuff ) ;
					entk->think3 = TH3_GO_NEXT_END ; 
				}
			} else {
				entk->think3 = TH3_GO_NEXT ; 
			}

			if ( entk->rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
				entk->status2 |= ENE_STATUS2_AIM_FACE ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
				entk->status2 |= ENE_STATUS2_AIM_GUN ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
				SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
				entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
			} else {
				entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
				entk->act->aim_dir = entk->rnavi->p_dir ;
			}
		}
		entk->count3 = 0 ;
		ChangeMoveCheck( entk, entk->rnavi->p_action ) ;

		if ( (entk->think3 != TH3_GO_NEXT) && 
			 (entk->think3 != TH3_GO_NEXT_END) ) {
			RouteGoal( entk ) ;
		}
		return ;
	}

	if ( rnavi->p_action == SP_GLASS ) {
		SET_FLAG( entk->status2, (ENE_STATUS2_AIM_GUN|ENE_STATUS2_AIM_TURN) ) ;
	} else if ( rnavi->p_actstatus & (PA_CON_AIM_FACE_MOVE|PA_CON_AIM_FACE) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE ) ;
		entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
	} else if ( entk->rnavi->p_actstatus & (PA_CON_FACE_ONLY_MOVE|PA_CON_FACE_ONLY) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_FACE_X ) ;
		entk->act->aim_pos = entk->rnavi->aimnodes[ (int)entk->rnavi->next_node ] ;
	} else if ( rnavi->p_actstatus & (PA_CON_AIM_GUN_MOVE|PA_CON_AIM_GUN) ) {
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;
		entk->act->aim_pos = rnavi->aimnodes[ (int)rnavi->next_node ] ;
	}

	RouteVoice( entk ) ;
	PointActionManage( entk ) ;

	entk->rnavi->p_acttime -- ;
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Travel( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_TRAVEL :		    /* 指定位置に移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_MOVE ) ;
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_NSIGHT_OFF :				/* 暗視ゴーグルはずす */
			Think3_NSightOff( entk ) ;
		break ;
	    case TH3_GO_NEXT :					/* 次の巡回ポイントへ移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_TRACE ) ;
			Think3_GoNext( entk ) ;
		break ;
	    case TH3_GO_NEXT_END :				/* 次の巡回ポイントへ移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_TRACE ) ;
			Think3_GoNextEnd( entk ) ;
		break ;
	    case TH3_PINPOINT_MOVE :			/* ピンポイント移動 */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_TRACE ) ;
			Think3_PinpointMove( entk ) ;
		break ;
	    case TH3_POINT_ACTION :				/* ポイントアクション */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_ACTION ) ;
	    	Think3_PointAction( entk ) ;
		break;
	    case TH3_POINT_MOTION :				/* ポイントアクション */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_ACTION ) ;
	    	Think3_PointMotion( entk ) ;
		break;
	    case TH3_POINT_FLAG :				/* ポイントアクション */
			SET_FLAG( entk->thk_status, THK_STATUS_ROOT_ACTION ) ;
	    	Think3_PointAction( entk ) ;
		break;
	}
}
/*----- 高レベル思考モード --------------------------------------------*/
#include "modechng.c"

enum {
	NOTICE_NONE,
	NOTICE_NOISE,	/* 物音 */
	NOTICE_BOX,		/* ダンボール */
	NOTICE_TRACE,	/* 足、血跡 */
	NOTICE_FOUND,	/* 不審物 */
	NOTICE_INDISTINCT,	/* おぼろ */
	NOTICE_UNIFORM,	/* 変装 */
	NOTICE_INVESTIGATED,	/* 調査 */
	NOTICE_ACCIDENT_REPO,	/* 調査 */
	NOTICE_RADIO,	/* 調査 */
} ;
static	void	TravelModeCheack( entk )
ENETHINK	*entk ;
{
	int	notice_flag ;
	/* 優先度の低いものから判定していく */

	notice_flag = 0 ;

	if ( entk->notice & ENE_NOTICE_RADIO ) {
		entk->think2 = TH2_RADIO ;
		THK_RadioModeStart( entk ) ;
		notice_flag = NOTICE_RADIO ;
	}

	if ( entk->notice & ENE_NOTICE_INVESTIGATED ) {
		entk->think2 = TH2_INVESTIGATED ;
		THK_InvestigatedModeStart( entk ) ;
		notice_flag = NOTICE_INVESTIGATED ;
	}

	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
		notice_flag = NOTICE_NOISE ;
	}

	if ( entk->notice & ENE_NOTICE_FOUND ) {
		entk->think2 = TH2_FOUND ;
		THK_FoundModeStart( entk ) ;
		notice_flag = NOTICE_FOUND ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
		notice_flag = NOTICE_TRACE ;
	}
//	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_BLURR ) {
	if ( entk->alert > 0 || (entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION)) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
		notice_flag = NOTICE_INDISTINCT ;
	}

	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
		notice_flag = NOTICE_UNIFORM ;
	}
	if ( entk->notice & ENE_NOTICE_ACCDNT_REPO ) {/*注！！CORPより先にチェック*/
		entk->think2 = TH2_ACCIDENT_REPO ;
		THK_AccidentReportModeStart( entk ) ;
		notice_flag = NOTICE_ACCIDENT_REPO ;
	}
	if ( entk->notice & 	ENE_NOTICE_CORP ) {
		entk->think2 = TH2_CORPS ;
		THK_CorpsModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		entk->think2 = TH2_ENE_DAMAGE ; 
		THK_EneDamageModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_BOX|ENE_NOTICE_NEAR_BOX) ) {
		entk->think2 = TH2_BOX ;
		THK_BoxModeStart( entk ) ;
		notice_flag = NOTICE_BOX ;
	}
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}

	/* 発見 */
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->alert >= DEF_EYE_DELAY ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	} else {
		if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
			if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
				entk->think2 = TH2_DISCOVERY ; 
				THK_DiscoveryNpcModeStart( entk ) ;
			}
		}
	}

	if ( entk->think2 != TH2_TRAVEL ) {/*モードチェンジ*/
printf("entk->think2 = %d\n",entk->think2);
		/* ストリーミングここで停止 */
//		COM_StopRadio( entk ) ;
		/* アクシデントにはしない */
//		COM_UnsetAccident( entk->uniq_id ) ;
	}

	if ( ENE_PureWatcher( entk->act ) ) {
		if ( notice_flag ) {
			WATCHWORK	*wcw ;
			ENE_ARGS	*call_proc ;

			wcw = (WATCHWORK *)entk->character ;

			call_proc = &wcw->notice_proc ;
			call_proc->argv[0] = (int)entk->ctrl->mov.vx ;
			call_proc->argv[1] = (int)entk->ctrl->mov.vy ;
			call_proc->argv[2] = (int)entk->ctrl->mov.vz ;
			call_proc->argv[3] = notice_flag ;

			call_proc->argc = 4 ;

			ENE_ProcCall( call_proc ) ;
		}
	}
}

static	void	RadioModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_RADIO) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	RadioModeChange( entk ) ;
}

static	void	InvestigatedModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INVESTIGATED) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	InvestigatedModeChange( entk ) ;
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他モードへ移行 */
	TouchModeChange( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	NoiseModeChange( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	FoundModeChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	TraceModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	IndistinctModeChange( entk ) ;
}

static	void	UniformModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_UNIFORM) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	UniformModeChange( entk ) ;
}

static	void	AccidentRepoModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ACCDNT_REPO) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	AccidentRepoModeChange( entk ) ;
}

static	void	CorpsModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	CorpsModeChange( entk ) ;
}

static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	EneDamageModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	BoxModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	HoldUpModeChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_WatcherStartModeSneak( entk ) ;
	}

	/* 他のモードへ */
	DamageModeChange( entk ) ;
}

static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( entk->iknow_flag & IKNOW_DETECT_SEARCH ) {
		CLEAR_FLAG( entk->iknow_flag ) ;
		ENE_WatcherStartModeSneak( entk ) ;
	}
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		潜入
	*/
void	ENE_Watcher_Think1_Sneak( entk )
ENETHINK	*entk ;
{
#ifdef COMBAT_TEST	///combat test
	ENE_Think1_Combat( entk ) ;
#else
	switch ( entk->think2 ) {
	    case TH2_TRAVEL :			/* 巡回 */
			Think2_Travel( entk ) ;
			TravelModeCheack( entk ) ;
		break ;

	    case TH2_RADIO :			/* 定時連絡無線壊れた */
			THK_RadioMode( entk ) ;
			RadioModeCheck( entk ) ;
		break ;
	    case TH2_INVESTIGATED :		/* 調査モード */
			THK_InvestigatedMode( entk ) ;
			InvestigatedModeCheck( entk ) ;
		break ;
	    case TH2_TOUCH :			/* タッチモード */
			THK_TouchMode( entk ) ;
			TouchModeCheck( entk ) ;
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
	    case TH2_INDISTINCT :		/* 朧モード */
			THK_IndistinctMode( entk ) ;
			IndistinctModeCheck( entk ) ;
		break ;
	    case TH2_UNIFORM :			/* 変装モード */
			THK_UniformMode( entk ) ;
			UniformModeCheck( entk ) ;
		break ;
	    case TH2_ACCIDENT_REPO :		/* アクシデント報告モード */
			THK_AccidentReportMode( entk ) ;
			AccidentRepoModeCheck( entk ) ;
		break ;
	    case TH2_CORPS :			/* 死体モード */
			THK_CorpsMode( entk ) ;
			CorpsModeCheck( entk ) ;
		break ;
	    case TH2_ENE_DAMAGE :		/* 味方ダメージ発見 */
			THK_EneDamage( entk ) ;
			EneDamageModeCheck( entk ) ;
		break ;
	    case TH2_BOX :				/* ダンボールモード */
			THK_BoxMode( entk ) ;
			BoxModeCheck( entk ) ;
		break ;
	    case TH2_HOLDUP :			/* ホールドアップモード */
			THK_HoldUpMode( entk ) ;
			HoldUpModeCheck( entk ) ;
		break ;

	    case TH2_DAMAGE :			/* ダメージモード */
			THK_Damage( entk ) ;
			DamageModeCheck( entk ) ;
		break ;

	    case TH2_DISCOVERY :		/* 発見 */
			THK_Discovery( entk ) ;
			DiscoveryModeCheck( entk ) ;
		break ;
	}
#endif
}

/*----- スタートモード、巡回兵潜入モード --------------------------------------------*/
void	ENE_WatcherStartModeSneak( entk )
ENETHINK	*entk ;
{
	entk->think1 = ENE_TH1_SNEAK ; 

	if ( !ENE_PureWatcher( entk->act ) ) {
printf("keizoku[%d] mess_notice[%x] c_notice[%x] \n",entk->id, entk->mess_notice,entk->c_notice);
//		if ( entk->mess_notice != MES_NOTICE_ZZZ &&
//				entk->mess_notice != MES_NOTICE_GOOFY ) {
			if ( entk->c_notice == ENE_NOTICE_HOLDUP ) {
				entk->think2 = TH2_HOLDUP ; 
//iran			THK_HoldUpModeStartDamage( entk ) ;
				return ;
			}
//		}
	}

	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;	/* notice clear 重要！！*/

	entk->act->CheckPad = WatcherSneakCheckPad ;
	entk->sense.status = RADAR_COLOR_BLUE ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;

		return ;
	}

	if ( RouteCheck( entk ) ) return ;

	if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) {	/* 反応無し */
		if ( entk->rnavi->next_node ) ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		else ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	} else {
		ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;
	}

	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = TH3_RETURN_TRAVEL ; 
	entk->count3 = 0 ;
#ifdef COMBAT_TEST	///combat test
	ENE_StartModeCombat( entk ) ;
#endif
}

void		ENE_WatcherStartModeSneakDamage( entk )
ENETHINK	*entk ;
{

	entk->think1 = ENE_TH1_SNEAK ; 

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
		return ;
	}

	if ( entk->mess_notice != MES_NOTICE_ZZZ &&
			entk->mess_notice != MES_NOTICE_GOOFY ) {
		if ( entk->c_notice == ENE_NOTICE_HOLDUP ) {
			entk->think2 = TH2_HOLDUP ; 
			THK_HoldUpModeStartDamage( entk ) ;
			return ;
		}
	}

	if ( ENE_PlayerHoldCheck( entk ) ) {
		entk->think2 = TH2_HOLDUP ; 
		THK_HoldUpModeStart( entk ) ;
		return ;
	}

	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
		entk->mess_notice == MES_NOTICE_GOOFY ||
		entk->mess_notice == MES_NOTICE_SMOKE ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
		return ;
	}

	entk->think2 = TH2_DAMAGE ; 
	entk->c_notice = 0 ;
	THK_DamageModeStart( entk ) ;
}

void	ENE_WatcherStageStartModeSneak( entk )
ENETHINK	*entk ;
{
	entk->think1 = ENE_TH1_SNEAK ; 

	if ( !ENE_PureWatcher( entk->act ) ) {
		if ( entk->c_notice == ENE_NOTICE_HOLDUP ) {
			entk->think2 = TH2_HOLDUP ; 
			return ;
		}
	}

	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;	/* notice clear 重要！！*/

	entk->act->CheckPad = WatcherSneakCheckPad ;
	entk->sense.status = RADAR_COLOR_BLUE ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;

		return ;
	}
	if ( RouteCheck( entk ) ) return ;

	ENE_SetTrgpNearRootPoint( entk->rnavi, entk->znavi, &(entk->trgpoint), &(entk->ctrl->mov) ) ;

	SetPointAction( entk ) ; 
	if ( entk->rnavi->p_action == SP_UNREAL ) {
		/* 開始時エフェクト出現防止 */
		SET_FLAG( entk->act->status, ACT_STATUS_UNREAL ) ;
	}
	if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) {	/* 反応無し */
		entk->act->pad = SP_UNREAL ;
		AT_SetModeFromPad( entk->act, ENE_ActUnreal, ENE_StandMotion(entk->act), entk->act->pad ) ;
	}

	RouteGoal( entk ) ;
	entk->act->dir = entk->rnavi->p_dir ;
	entk->think2 = TH2_TRAVEL ; 
	entk->think3 = SelectPAMode( entk ) ; 
	entk->count3 = 0 ;
}

