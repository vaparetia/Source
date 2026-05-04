//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	spavoid.c
	巡回兵 回避モード
	
	2000/01/11 Y.Korekado
	$Id: spavoid.c,v 1.1.1.3 2002/11/19 11:44:24 Yoshizawa1 Exp $
	
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
	TH2_MOVE,TH2_SEARCH,TH2_NOISE,TH2_TOUCH,TH2_DAMAGE,
	TH2_DISCOVERY,TH2_DEFENSE_ZONE,TH2_FOLLOW,TH2_NEAR_ZONE,TH2_CROSS
} ;
enum {
	TH3_ZONE_CHASE, TH3_DIRECT_CHASE, TH3_SEARCH_STAND, TH3_MOVE_SAFEAREA,TH3_WAIT,
	TH3_LOOK_CHECK_ZONES, TH3_LOOKAROUND, TH3_LOOKAROUND_CHECK_ZONE, TH3_MOVE_DEFENSE, TH3_RELOAD, 
	TH3_SUPPORT_BUDDY, TH3_WATCH , TH3_READY, TH3_BUDDYSEARCH_STAND
} ;
enum {
	SP_NONE,
	SP_RELOAD,
	SP_READYGUN,	/* 銃構える */
} ;


#include	"spavoact.c"

/*----- 低レベル思考モード --------------------------------------------*/
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
		switch( 3 ){
			case 0:
				entk->avoinfo.check_zone[0] = 
					ENE_GetWideNearZone( entk->ctrl->addr&255, 1, entk->ctrl->hzx_id ) ;
				entk->avoinfo.check_map = entk->ctrl->hzx_id ;
printf("Sup Get Wide Near Zone [%d] \n",entk->avoinfo.check_zone[0] ) ;
				entk->think2 = TH2_NEAR_ZONE ; 
				entk->think3 = TH3_READY ;
				entk->act->move_s = MoveWalk ;
				break ;
			case 1:
				entk->think2 = TH2_DEFENSE_ZONE ; 
				entk->think3 = TH3_SUPPORT_BUDDY ; 
				entk->act->move_s = MoveWalk ;
				break ;
			case 2:
				entk->think2 = TH2_FOLLOW ; 
				entk->think3 = TH3_ZONE_CHASE ; 
				entk->act->move_s = MoveWalk ;
				entk->tmp_time = 100 ;
				break ;
			case 3:
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_BUDDYSEARCH_STAND ; 
				entk->act->move_s = MoveWalk ;
				break ;
			case 4:
				entk->think2 = TH2_CROSS ; 
				entk->think3 = TH3_SEARCH_STAND ; 
				entk->act->move_s = MoveWalk ;
				break ;
		}
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static void Think3_SearchReady( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		/* 今いるゾーンの安全地帯数を調べて、確認するゾーン数とする */
		entk->avoinfo.check_num = ENE_GetSafeZoneNum( entk ) ;
	}

	if ( entk->count3 > COUNT_VMODE(20) ) {
printf(" SAFE ZONE NUM = %d \n",entk->avoinfo.check_num ) ;
		if ( entk->avoinfo.check_num > 0 ) {
			entk->think3 = TH3_LOOK_CHECK_ZONES ; 
		} else {
			entk->think3 = TH3_LOOKAROUND ; 
		}
		entk->count3 = 0 ;

		return ;
	}

	entk->count3 ++ ;
}

static void Think3_BuddySearchReady( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		/* バディと別々に調べる場所とその数 */
		entk->avoinfo.check_num =  ENE_GetPearSafeZone( entk, 1 ) ;
	}

	if ( entk->count3 > COUNT_VMODE(20) ) {
printf(" SAFE ZONE NUM = %d \n",entk->avoinfo.check_num ) ;
		if ( entk->avoinfo.check_num > 0 ) {
			entk->think3 = TH3_LOOK_CHECK_ZONES ; 
		} else {
			entk->think3 = TH3_LOOKAROUND ; 
		}
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_LookCheckZones( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

	/* ルートのつながっている方向を見る 最大４回 */
	if ( !(entk->count3%COUNT_VMODE(60)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, entk->count3/COUNT_VMODE(60), entk->ctrl->hzx_id ) ;
		if ( near != 255 ) {
			entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
		} else {
			entk->count3 = COUNT_VMODE(240) ;
		}
	}

	if ( entk->count3 >= COUNT_VMODE(240) ) {
		if ( entk->avoinfo.check_num > 0 ) {
			if ( entk->bullet > 5 ) {
				entk->think3 = TH3_RELOAD ; 
			} else {
				ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[0], entk->avoinfo.check_map ) ;
				entk->think3 = TH3_MOVE_SAFEAREA ; 
			}
		} else {
			entk->think3 = TH3_LOOKAROUND ; 
		}
		entk->count3 = 0 ;
		
		return ;
	}

	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->count3 ++ ;
}

static void Think3_LookAroundCheckZone( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;

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
			ENE_SetTrgpDefense( &(entk->trgpoint), &entk->spbudy->def_pos, entk->spbudy->def_mapbit ) ;
			entk->think3 = TH3_MOVE_DEFENSE ; 
		}
		entk->count3 = 0 ;
		
		return ;
	}
	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->count3 ++ ;
}

static void Think3_LookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > 0 && entk->count3 < COUNT_VMODE(60)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
	
	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_Reload( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_RELOAD ;
	}

	if ( entk->act->act_end ) {
		if ( entk->avoinfo.check_num > 0 ) {
			ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[0], entk->avoinfo.check_map ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		} else {
			entk->think3 = TH3_LOOKAROUND ; 
		}
		entk->count3 = 0 ;
		
		return ;
	}

	entk->act->dir = ENE_ZoneDir( &entk->ctrl->mov, entk->avoinfo.check_zone[ 0 ], entk->ctrl->hzx_id ) ;

	entk->count3 ++ ;
}


static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	int	route ;
	
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
	
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		
		return ;
	}


	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_MoveDefense( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		
		return ;
	}


	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_SupportBuddy( entk )
ENETHINK	*entk ;
{
#if 1
	int	diff_dir ;
	
	entk->act->pad = SP_READYGUN ;

	entk->act->aim_dir = entk->bd_eyei.dir ;
	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
#else
	entk->act->pad = SP_READYGUN ;
	entk->act->dir = entk->bd_eyei.dir ;
#endif
	entk->count3 ++ ;
}

static void Think3_DefenseLookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > 0 && entk->count3 < COUNT_VMODE(60)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
	
	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_NearZoneReady( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > 0 && entk->count3 < COUNT_VMODE(60)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
	
	if ( entk->count3 > COUNT_VMODE(180) ) {
		ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[0], entk->avoinfo.check_map ) ;
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_MoveNearZone( entk )
ENETHINK	*entk ;
{
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND ; 
		entk->count3 = 0 ;
		
		return ;
	}


	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_NearZoneLookAround( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > 0 && entk->count3 < COUNT_VMODE(60)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( entk->count3 > COUNT_VMODE(60) && entk->count3 < COUNT_VMODE(120)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
	
	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}


#define	BUDDY_DIS 400
static void Think3_ZoneChaseBuddy( entk )
ENETHINK	*entk ;
{
	if( ++entk->tmp_time > COUNT_VMODE(90) ) {
		entk->act->move_s = MoveWalk ;
	} else {
		entk->act->move_s = MoveBack ;
	}

	if( entk->bd_eyei.dis < BUDDY_DIS ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		
		return ;
	}
	
	ENE_SetTrgpEyei( &entk->bd_eyei, &entk->trgpoint ) ;
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->bd_eyei.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
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
	int	route ;
	
	if( ++ entk->tmp_time > COUNT_VMODE(90) ) {
		entk->act->move_s = MoveWalk ;
	} else {
		entk->act->move_s = MoveBack ;
	}

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
	route = HZX_GetRouteCrossGroup( entk->ctrl->addr, entk->spbudy->ctrl->addr ) ;
	if ( route > 1 ) {
		entk->act->dir = entk->bd_eyei.dir ;
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->bd_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_ChaseWait( entk )
ENETHINK	*entk ;
{
	if( entk->bd_eyei.dis > BUDDY_DIS+100 ) {
		entk->think3 = TH3_DIRECT_CHASE ; 
		entk->count3 = 0 ;
		
		return ;
	} else if ( entk->spbudy->thk_status & THK_STATUS_WATCH ) {
		entk->think3 = TH3_WATCH ; 
		entk->count3 = 0 ;
		
		return ;
	}

}

static	void	Think3_Watch( entk )
ENETHINK	*entk ;
{
	if ( !(entk->spbudy->thk_status & THK_STATUS_WATCH) ) {
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		entk->tmp_time = 0;
		
		return ;
	}

}


/*----- 中レベル思考モード --------------------------------------------*/
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
}

static	void	Think2_Search( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_SEARCH_STAND :
	    	Think3_SearchReady( entk ) ;
		break ;
	    case TH3_BUDDYSEARCH_STAND :
	    	Think3_BuddySearchReady( entk ) ;
		break ;
	    case TH3_LOOK_CHECK_ZONES :
	    	Think3_LookCheckZones( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
	    	Think3_LookAround( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
	    	Think3_LookAroundCheckZone( entk ) ;
		break ;
	    case TH3_MOVE_SAFEAREA :
	    	Think3_MoveSafeArea( entk ) ;
		break ;
	    case TH3_MOVE_DEFENSE :
	    	Think3_MoveDefense( entk ) ;
		break ;
	    case TH3_RELOAD :
	    	Think3_Reload( entk ) ;
		break ;
		case TH3_WAIT :
		break ;
	}
//	PlayerCheack( entk ) ;
}

static	void	Think2_Cross( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_SEARCH_STAND :
	    	Think3_SearchReady( entk ) ;
		break ;
	    case TH3_LOOK_CHECK_ZONES :
	    	Think3_LookCheckZones( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
	    	Think3_LookAround( entk ) ;
		break ;
	    case TH3_LOOKAROUND_CHECK_ZONE :
	    	Think3_LookAroundCheckZone( entk ) ;
		break ;
	    case TH3_MOVE_SAFEAREA :
	    	Think3_MoveSafeArea( entk ) ;
		break ;
	    case TH3_MOVE_DEFENSE :
	    	Think3_MoveDefense( entk ) ;
		break ;
	    case TH3_RELOAD :
	    	Think3_Reload( entk ) ;
		break ;
		case TH3_WAIT :
		break ;
	}
//	PlayerCheack( entk ) ;
}

static	void	Think2_DefenseZone( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_SUPPORT_BUDDY :
	    	Think3_SupportBuddy( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
	    	Think3_DefenseLookAround( entk ) ;
		break ;
	}
}

static	void	Think2_NearZone( entk )
ENETHINK	*entk ;
{
	
	switch ( entk->think3 ) {
	    case TH3_READY :
	    	Think3_NearZoneReady( entk ) ;
		break ;
	    case TH3_ZONE_CHASE :			/* 目標隣接ゾーンまで行く */
	    	Think3_MoveNearZone( entk ) ;
		break ;
	    case TH3_LOOKAROUND :
	    	Think3_NearZoneLookAround( entk ) ;
		break ;
	}
}

static	void	Think2_Follow( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChaseBuddy( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseBuddy( entk ) ;
		break ;
	    case TH3_WAIT :					/* 止まる */
	    	Think3_ChaseWait( entk ) ;
		break ;
	    case TH3_WATCH :				/* 確認 */
	    	Think3_Watch( entk ) ;
		break ;
	}

}

/*----- 高レベル思考チェック --------------------------------------------*/
static	void	SearchModeCheack( entk )
ENETHINK	*entk ;
{
	/* 優先度の低いものから判定していく */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}

	if ( entk->notice & ENE_NOTICE_NOISE ) {
		entk->think2 = TH2_NOISE ;
		THK_NoiseModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
	
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_TOUCH|ENE_NOTICE_NEAR_TOUCH) ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
	}
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	}
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	}
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
		/* 探索モードになるから死体モードへは移項しない */

	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStart( entk ) ;
	}
}


static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		回避
	*/
void	ENE_Support_Think1_Avoid( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_MOVE :
			Think2_MoveDefensePoint( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
	    case TH2_DEFENSE_ZONE :
			Think2_DefenseZone( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
	    case TH2_NEAR_ZONE :
			Think2_NearZone( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
	    case TH2_FOLLOW :
			Think2_Follow( entk ) ;
			SearchModeCheack( entk ) ;
		break ;

	    case TH2_SEARCH :			/* 安全地帯確認 */
			Think2_Search( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
	    case TH2_CROSS :			/* 安全地帯確認2 */
			Think2_Cross( entk ) ;
			SearchModeCheack( entk ) ;
		break ;
	    case TH2_NOISE :			/* 物音モード */
			THK_NoiseMode( entk ) ;
			NoiseModeCheck( entk ) ;
		break ;
	    case TH2_TOUCH :			/* タッチモード */
			THK_TouchMode( entk ) ;
			TouchModeCheck( entk ) ;
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
}

/*----- スタートモード、巡回兵潜入モード --------------------------------------------*/
void	ENE_SupportStartModeAvoid( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	entk->act->CheckPad = SupportAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->avoinfo.check_num = 0 ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_AVOID ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

	ENE_SetTrgpDefense( &(entk->trgpoint), &entk->spbudy->def_pos, entk->spbudy->def_mapbit ) ;
//printf("supp def_pos %f %f %f \n",entk->spbudy->def_pos.vx,entk->spbudy->def_pos.vy,entk->spbudy->def_pos.vz);
	entk->think1 = ENE_TH1_AVOID ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 

	entk->count3 = 0 ;
}

void	ENE_SupportReturnModeAvoid( entk )
ENETHINK	*entk ;
{
	int count ;
	
	entk->c_notice = 0 ;

	entk->act->CheckPad = SupportAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_AVOID ; 

	if ( entk->avoinfo.check_num > 0 ) {
		entk->think2 = TH2_SEARCH ; 
		
		count = entk->avoinfo.check_count ;
		if ( entk->avoinfo.check_num > count ) {
			ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[count], entk->avoinfo.check_map ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		} else {
			ENE_SetTrgpDefense( &(entk->trgpoint), &entk->spbudy->def_pos, entk->spbudy->def_mapbit ) ;
			entk->think3 = TH3_MOVE_DEFENSE ; 
		}
	} else {
		ENE_SetTrgpDefense( &(entk->trgpoint), &entk->spbudy->def_pos, entk->spbudy->def_mapbit ) ;
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
	}
	entk->count3 = 0 ;
}


void	ENE_SupportStartModeAvoidDamage( entk )
ENETHINK	*entk ;
{

	entk->think1 = ENE_TH1_AVOID ; 

	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
			entk->mess_notice == MES_NOTICE_GOOFY ) {
		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
		return ;
	}

	if ( entk->c_notice == ENE_NOTICE_HOLDUP ) {
//		entk->think2 = TH2_HOLDUP ; 
//		THK_HoldUpModeStartDamage( entk ) ;
	} else {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageModeStart( entk ) ;
	}
}
