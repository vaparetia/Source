//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wctravel.c
	巡回兵 潜入モード
	
	1997/07/29 Y.Korekado
	$Id: spcautio.c,v 1.1.1.3 2002/11/19 11:44:25 Yoshizawa1 Exp $
	
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

extern void ENE_PosWarp( ENETHINK	*entk, FVECTOR		*pos, int			hzx_id );

enum {
	TH2_TRAVEL ,TH2_SUPPORT, TH2_BUDDY_PINCH, TH2_NOISE, TH2_FOUND,
	TH2_TRACE, TH2_DAMAGE, TH2_INDISTINCT, TH2_BOX, TH2_TOUCH,
	TH2_ACCIDENT, TH2_UNIFORM,  TH2_CORPS, TH2_ENE_DAMAGE, TH2_HOLDUP,
	TH2_DISCOVERY,
} ;
enum {
	TH3_RETURN_TRAVEL,TH3_GO_NEXT,TH3_POINT_ACTION,TH3_WATCH,TH3_ZONETRACE,
	TH3_LOOKAROUND,TH3_DISCOVERY_POSE,TH3_RADIO_CALL,TH3_WAIT, TH3_ZONE_CHASE,
	TH3_DIRECT_CHASE, TH3_WATCH_PLAYER, TH3_NO_WATCH, TH3_CHECK_ZONES_READY, TH3_MOVE_SAFEAREA,
	TH3_LOOKAROUND_CHECK_ZONE, TH3_OUTOF_WAY, TH3_RETURN_WAIT, TH3_TURN, TH3_START_WAIT
} ;
enum {
	SP_NONE,
	SP_DISCOVERY,	/* 発見ポーズ */
	SP_READYGUN,	/* 銃構える */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_RADIOCALL,	/* 無線連絡 */
	SP_OUTOF_WAY_R,	/* 避ける */
	SP_OUTOF_WAY_L,	/* 避ける */
	SP_NORMAL_STAND, /* 銃降ろし立ち */
	SP_UNREAL,	/* 存在しない */
} ;

#include	"spcauact.c"
#include 	"../attacker/modechng.c"
/*----- --------------------------------------------*/
//#define	BUDDY_DIS 500
#define	BUDDY_DIS 2000
#define	OUTOFTHEWAY_DIS 1500

/*----- --------------------------------------------*/
static int FrontCheck ( ENETHINK	*entk )
{
	int diff ;

	diff = GV_DiffDirS( entk->ctrl->rot.vy, entk->bd_eyei.dir ) ;

	if ( (diff > 256) || (diff < -256) ) return 0 ;
	if( entk->bd_eyei.dis > BUDDY_DIS+500 ) return 0 ;

	return 1 ;
}

static int BuddyDirOpposite ( ENETHINK	*entk )
{
	int diff, dir ;

	if ( entk->spbudy->act->status & ACT_STATUS_MOVE ) return 0 ;

	diff = GV_DiffDirS( entk->ctrl->rot.vy, entk->spbudy->ctrl->rot.vy ) ;

	/* 反対方向を見ていない */
	if ( (diff < 1536) && (diff > -1536) ) return 0 ;

	dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->spbudy->ctrl->mov ) ;
	diff = GV_DiffDirS( entk->ctrl->rot.vy, dir ) ;

	/* 正面にいない */
	if ( (diff > 512) && (diff < -512) ) return 0 ;

	return 1 ;
}

static int OutOfTheWaySide ( ENETHINK	*entk )
{
	int diff ;
	
	diff = GV_DiffDirS( entk->ctrl->rot.vy, entk->bd_eyei.dir ) ;
	
	return (diff > 0 ) ? 0 : 1 ;//( 右、左)
}

static int OutOfTheWayCheck ( ENETHINK	*entk )
{
	if ( !(entk->spbudy->act->status & ACT_STATUS_MOVE) ) return 0 ;

	if( entk->bd_eyei.dis < OUTOFTHEWAY_DIS ) {
		return 1 ;
	}
	return 0 ;
}

static int BuddyCheck ( ENETHINK	*entk)
{
	if( entk->spbudy->act->status & (ACT_STATUS_FAINT|ACT_STATUS_DEATH) ){
		return 1 ;
	}
	return 0 ;
}

static void NextBuddySearch ( ENETHINK	*entk)
{
	printf(" Next Buddy Search Route[%d] Node[%d]\n",entk->spbudy->rnavi->c_route, entk->tmp_count ) ;
	entk->tmp_count = ENE_SetTrgpFromRnaviNode( entk->spbudy->rnavi, &(entk->trgpoint),  entk->tmp_count ) ;

	entk->count3 = 0 ;
}

static void SetCheckPad ( entk )
ENETHINK	*entk ;
{
	entk->act->CheckPad = SupportSneakCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
}
/*----- 低レベル思考モード --------------------------------------------*/
static	void	Think3_SupportStartWait( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 >= (entk->id*COUNT_VMODE(30)) ) {
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->act->pad = SP_UNREAL ;

	entk->count3 ++ ;
}

static	void	Think3_BuddyPinchStartWait( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_UNREAL ;

	if ( entk->count3 > (entk->id*30) ) {
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_ZoneChaseBuddy( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->bd_eyei.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}
	if( entk->bd_eyei.dis < BUDDY_DIS ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}
	
	if ( BuddyCheck ( entk ) ) {
ENE_SetTrgpNode( entk->spbudy->rnavi, &(entk->trgpoint) ) ;//rev 02.09.17
		entk->tmp_count = entk->spbudy->rnavi->next_node ;
		entk->act->dir = entk->trgpoint.dir ;
		entk->think2 = TH2_BUDDY_PINCH ; 
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}
	
	if( entk->bd_eyei.dis < BUDDY_DIS+2000 ) {
		entk->act->move_s = MoveWalkGun ;
	} else {
		entk->act->move_s = MoveRun ;
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
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	if ( entk->bd_eyei.sight != EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( BuddyCheck ( entk ) ) {
ENE_SetTrgpNode( entk->spbudy->rnavi, &(entk->trgpoint) ) ;//rev 02.09.17
		entk->tmp_count = entk->spbudy->rnavi->next_node ;
		entk->act->dir = entk->trgpoint.dir ;
		entk->think2 = TH2_BUDDY_PINCH ; 
		entk->think3 = TH3_DIRECT_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->bd_eyei.dis < BUDDY_DIS+2000 ) {
		entk->act->move_s = MoveWalkGun ;
	} else {
		entk->act->move_s = MoveRun ;
	}


	/* 体に移動命令 */
	entk->act->dir = entk->bd_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_ChaseWait( entk )
ENETHINK	*entk ;
{
	if ( FrontCheck ( entk ) ) {
		entk->act->pad = SP_NORMAL_STAND ;
	} else {
		entk->act->pad = SP_CAUT_STAND ;
	}

	if ( BuddyDirOpposite ( entk ) ) {
		entk->think3 = TH3_TURN ; 
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		if ( OutOfTheWayCheck ( entk ) ) {
			entk->think3 = TH3_OUTOF_WAY ; 
			entk->count3 = 0 ;
			
			return ;
		}
	}

	if( entk->bd_eyei.dis > BUDDY_DIS+1000 ) {
		entk->think3 = TH3_DIRECT_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if ( BuddyCheck ( entk ) ) {
ENE_SetTrgpNode( entk->spbudy->rnavi, &(entk->trgpoint) ) ;//rev 02.09.17
		entk->tmp_count = entk->spbudy->rnavi->next_node ;
		entk->act->dir = entk->trgpoint.dir ;
		entk->think2 = TH2_BUDDY_PINCH ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}
	
	entk->count3 ++ ;
}

static	void	Think3_OutOfTheWay( entk )
ENETHINK	*entk ;
{

	if( entk->count3 == 0 ) {
		if( OutOfTheWaySide ( entk ) ) {
			entk->act->pad = SP_OUTOF_WAY_L ;
		} else {
			entk->act->pad = SP_OUTOF_WAY_R ;
		}
	}

//	if ( entk->act->act_end ) {
	if( entk->count3 > COUNT_VMODE(60) ) {
		entk->act->pad = SP_CAUT_STAND ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_Turn( entk )
ENETHINK	*entk ;
{

	if( entk->count3 == 1 ) {
		entk->act->dir = entk->spbudy->ctrl->rot.vy ;
	}

	if( entk->count3 > 4 ) {
		entk->act->pad = SP_CAUT_STAND ;
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}


static void Think3_PinchZoneChase( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}
	if( entk->bd_eyei.dis < BUDDY_DIS ) {
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_PinchDirectChase( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), BUDDY_DIS ) < 0 ) {
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_PinchWait( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if( entk->count3  > COUNT_VMODE(60*10) ) {
		/* この時点でバディへの目標地点が入っているはず */
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static	void	Think3_LookAround( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	if ( entk->count3 == COUNT_VMODE(200) ) {
//		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_QW ) ;
//		if(	GM_AlertMode != ALERT_MODE_AVOID ){
//			COM_SetSpeak( EV_WHAT_NOISE, entk ) ; /* うーん */
//		}
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
		entk->think3 = TH3_CHECK_ZONES_READY ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->pad = SP_CAUT_STAND ;

	entk->count3 ++ ;
	return ;
}

static	void	Think3_CheckZonesReady( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 > COUNT_VMODE(4) ) {
		entk->avoinfo.check_num = ENE_GetSafeZoneNum( entk ) ;
printf(" SAFE ZONE NUM = %d \n",entk->avoinfo.check_num ) ;
		if ( entk->avoinfo.check_num > 0 ) {
			ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[0], entk->avoinfo.check_map ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		} else {
			/* 見回る場所がないので次のゾーンへ */
			NextBuddySearch( entk ) ;
			entk->think3 = TH3_ZONE_CHASE ; 
		}
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	int	route, intrpt ;
	
	route = ENE_GetTrgRoute( entk ) ;
	
	if ( route <= 1 ){
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
		entk->act->aim_pos = entk->trgpoint.pos ;
		if ( route < 1 ){
			entk->act->aim_pos.vy += 1000 ;	/* ゾーンなので高さ＋ */
		}
		entk->act->move_s = MoveCautionWalk ;
	} else {
		entk->act->move_s = MoveWalk ;
	}
	
#if 1
	intrpt = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ;

	if (  intrpt != 0 ) {
printf("intrp===============[%d]\n",intrpt ) ;
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		
		return ;
	}
#else
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		
		return ;
	}
#endif

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_LookAroundCheckZone( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	entk->thk_status |= THK_STATUS_WATCH ;

	/* ルートのつながっている方向を見る 最大４回 */
	if ( entk->count3 == 0 ) {
		entk->act->aim_dir = entk->ctrl->rot.vy ;
	} else if ( !(entk->count3%COUNT_VMODE(60)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, (entk->count3/COUNT_VMODE(60))-1, entk->ctrl->hzx_id ) ;
		if ( HZX_ZoneMapNo( entk->ctrl->addr ) == HZX_ZoneMapNo( entk->before_inzone[1] ) &&
				near == HZX_Zone1( entk->before_inzone[1] ) ) {
			/* 今来た道なら振り向かない */
			entk->count3 += COUNT_VMODE(60) - 1 ;
		} else {
			if ( near != 255 ) {
				entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
			} else {
				entk->count3 = COUNT_VMODE(300) ;
			}
		}
	}

	if ( entk->count3 >= COUNT_VMODE(300) ) {
		int count ;
		
		count = ++entk->avoinfo.check_count ;
		if ( entk->avoinfo.check_num > count ) {
			ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[count], entk->avoinfo.check_map ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		} else {
			/* 見回る場所がないので次のゾーンへ */
			NextBuddySearch( entk ) ;
			entk->think3 = TH3_ZONE_CHASE ; 
		}
		entk->count3 = 0 ;
		
		return ;
	}
	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->count3 ++ ;
}

static void Think3_ReturnWait( entk )
ENETHINK		*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_ReturnTravel( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->think3 = TH3_GO_NEXT ;
		entk->count3 = 0 ;
//		entk->rnavi->next_node -- ;
		ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		entk->act->move_s = MoveWalkGun ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_GoNext( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->act->dir = entk->rnavi->pa_dir[(int)entk->rnavi->next_node] ;
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
ENETHINK	*entk ;
{
	if ( entk->rnavi->p_acttime <= 0) {
		ENE_SetTrgpNextnode( entk->rnavi, &(entk->trgpoint) ) ;
		entk->think3 = TH3_GO_NEXT ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->rnavi->p_acttime -- ;
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Travel( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_WAIT :		    /* 指定位置に移動 */
	    	Think3_ReturnWait( entk ) ;
		break ;
	    case TH3_ZONE_CHASE :		    /* 指定位置に移動 */
	    	Think3_ReturnTravel( entk ) ;
		break ;
	    case TH3_GO_NEXT :					/* 次の巡回ポイントへ移動 */
			Think3_GoNext( entk ) ;
		break ;
	    case TH3_POINT_ACTION :				/* ポイントアクション */
	    	Think3_PointAction( entk ) ;
		break;
	}
}

static	void	Think2_Support( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_WAIT :		    /* 指定位置に移動 */
	    	Think3_ReturnWait( entk ) ;
		break ;
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChaseBuddy( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseBuddy( entk ) ;
		break ;
	    case TH3_WAIT :					/* 止まる */
	    	Think3_ChaseWait( entk ) ;
		break ;
	    case TH3_OUTOF_WAY :			/* 避ける */
	    	Think3_OutOfTheWay( entk ) ;
		break ;
	    case TH3_TURN :					/* 同じ方向を向く */
	    	Think3_Turn( entk ) ;
		break ;
	    case TH3_START_WAIT :		    /* ずらし待ち */
		   	Think3_SupportStartWait( entk ) ;
		break ;
	}

}


static	void	Think2_BuddyPinch( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_RETURN_WAIT :		    /* 指定位置に移動 */
	    	Think3_ReturnWait( entk ) ;
		break ;
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_PinchZoneChase( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_PinchDirectChase( entk ) ;
		break ;
	    case TH3_WAIT :					/* 止まる */
	    	Think3_PinchWait( entk ) ;
		break ;
	    case TH3_LOOKAROUND :			/* 見回す */
		   	Think3_LookAround( entk ) ;
		break ;
	    case TH3_CHECK_ZONES_READY :			/* 安全地帯探索 */
		   	Think3_CheckZonesReady( entk ) ;
		break ;
	    case TH3_MOVE_SAFEAREA :
	    	Think3_MoveSafeArea( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
	    	Think3_LookAroundCheckZone( entk ) ;
		break ;
	    case TH3_START_WAIT :		    /* ずらし待ち */
		   	Think3_SupportStartWait( entk ) ;
		break ;
	}

	if ( !BuddyCheck( entk ) && (entk->bd_eyei.sight == EYE_INFO_SIGHT_IN) ) {
		/* 元気なバディをみかけたら */
		entk->think2 = TH2_SUPPORT ; 
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
}

/*----- 高レベル思考チェック --------------------------------------------*/
static	void	SupportModeCheack( entk )
ENETHINK	*entk ;
{
	TravelModeChange( entk ) ;
}

static	void	AccidentModeCheack( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ACCIDENT) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	AccidentModeChange( entk ) ;
}

static	void	UniformModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_UNIFORM) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	/* 他のモードへ */
	UniformModeChange( entk ) ;
}

static	void	CorpsModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	CorpsModeChange( entk ) ;
}


static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	EneDamageModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	HoldUpModeChange( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	NoiseModeChange( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	FoundModeChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		ENE_SupportStartModeCaution( entk ) ;
	}

	TraceModeChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	DamageModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	IndistinctModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	BoxModeChange( entk ) ;
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_SupportStartModeCaution( entk ) ;
	}

	TouchModeChange( entk ) ;
}

static	void	DiscoveryModeCheack( entk )
ENETHINK	*entk ;
{
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		潜入
	*/
void	ENE_Support_Think1_Caution( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_TRAVEL :		/* 巡回 */
			Think2_Travel( entk ) ;
			SupportModeCheack( entk ) ;
		break ;
	    case TH2_SUPPORT :		/* サポート */
			Think2_Support( entk ) ;
			SupportModeCheack( entk ) ;
		break ;
	    case TH2_BUDDY_PINCH :		/* サポート対象異常 */
			Think2_BuddyPinch( entk ) ;
			SupportModeCheack( entk ) ;
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

	    case TH2_DISCOVERY :	/* 発見 */
			THK_Discovery( entk ) ;
			DiscoveryModeCheack( entk ) ;
		break ;
	}

	entk->count1 ++ ;
}

/*----- スタートモード、巡回兵潜入モード --------------------------------------------*/
void	ENE_SupportStartModeCaution( entk )
ENETHINK	*entk ;
{
	ENE_TraceClear( entk ) ;
	entk->c_notice = 0 ;

	SetCheckPad( entk ) ;

	entk->sense.status = RADAR_COLOR_BLUE ;
	entk->bd_eyei.flag &= ~EYE_INFO_FLAG_SKIP ;
	ENE_SetEyeInfo( &(entk->bd_eyei), &entk->spbudy->ctrl->mov,
									 &entk->spbudy->ctrl->addr, NULL, &entk->spbudy->ctrl->hzx_id ) ;
	ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_SEARCH ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;

		return ;
	}

	entk->think1 = ENE_TH1_SEARCH ; 
	if ( BuddyCheck ( entk ) ) {
		entk->tmp_count = entk->spbudy->rnavi->next_node ;
		/* バディが最後に向かおうとしたポイントへ行く */
		ENE_SetTrgpNode( entk->spbudy->rnavi, &(entk->trgpoint) ) ;
printf("Buddy Lost Go Route[%d] Node[%d]\n", entk->spbudy->rnavi->c_route, entk->spbudy->rnavi->next_node ) ;
		entk->think2 = TH2_BUDDY_PINCH ; 
		entk->think3 = TH3_ZONE_CHASE ; 
	} else {
		entk->think2 = TH2_SUPPORT ; 
		entk->think3 = TH3_ZONE_CHASE ; 
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( (entk->act->status & ACT_STATUS_UNREAL) ) {
			entk->act->keep_pad = entk->act->pad = SP_UNREAL ;
			entk->think3 = TH3_START_WAIT ; 
		}
	}

	entk->count1 = 0 ;
	entk->count3 = 0 ;
}

void	ENE_SupportStartModeCautionDamage( entk )
ENETHINK	*entk ;
{
	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
			entk->mess_notice == MES_NOTICE_GOOFY ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
		return ;
	}

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

void	ENE_SupportStartModeAvoidToSearch( entk )
ENETHINK	*entk ;
{
	ENE_SupportStartModeCaution( entk ) ;
	entk->think3 = TH3_RETURN_WAIT ; 
}

void	ENE_SupportStageStartModeToSearch( entk )
ENETHINK	*entk ;
{
	ENE_SupportStartModeCaution( entk ) ;

	if ( BuddyCheck ( entk ) ) {
		FVECTOR pos ;
		pos.vx = entk->trgpoint.pos.vx ;
		pos.vy = entk->trgpoint.pos.vy + 1000.0f ;
		pos.vz = entk->trgpoint.pos.vz ;
		ENE_PosWarp( entk, &pos, entk->trgpoint.map ) ;
	} else {
		ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;
	}
}
/* サポート兵はワープで登場しない必ず出てくる */
