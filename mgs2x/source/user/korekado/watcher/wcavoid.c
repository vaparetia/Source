//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wcavoid.c
	巡回兵 回避モード
	
	1999/07/29 Y.Korekado
	$Id: wcavoid.c,v 1.1.1.3 2002/11/19 11:44:30 Yoshizawa1 Exp $
	
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
// BP #include <break.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

enum {
	TH2_MOVE,TH2_SEARCH,TH2_DISCOVERY,TH2_DEFENSE_ZONE,TH2_NEAR_ZONE,
	TH2_TOUCH,TH2_TRACE,TH2_NOISE,TH2_FOUND,TH2_INDISTINCT,
	TH2_CORPS,TH2_BOX, TH2_ENE_DAMAGE,TH2_HOLDUP,TH2_DAMAGE,
	TH2_UNIFORM
} ;
enum {
	TH3_ZONE_CHASE, TH3_DIRECT_CHASE, TH3_SEARCH_STAND, TH3_MOVE_SAFEAREA,TH3_WAIT,
	TH3_LOOK_CHECK_ZONES, TH3_LOOKAROUND, TH3_LOOKAROUND_CHECK_ZONE, TH3_MOVE_DEFENSE, TH3_RELOAD,
	TH3_READY, TH3_BUDDYSEARCH_STAND, TH3_CLEARING_LOOKAROUND
} ;
enum {
	SP_NONE,
	SP_RELOAD
} ;


#include	"wcavoact.c"

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
		int mode ;

		mode = (GM_GameStatus & STATE_CLEARING) ? 4 : 3 ;
		switch( mode ){
			case 0:
				entk->avoinfo.check_zone[0] = 
					ENE_GetWideNearZone( entk->ctrl->addr&255, 0, entk->ctrl->hzx_id ) ;
printf(" Get Wide Near Zone [%d] \n",entk->avoinfo.check_zone[0] ) ;
				entk->think2 = TH2_NEAR_ZONE ; 
				entk->think3 = TH3_READY ;
				break ;
			case 1:
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_SEARCH_STAND ; 
				break ;
			case 2:
				entk->think2 = TH2_DEFENSE_ZONE ; 
				entk->think3 = TH3_LOOKAROUND ; 
				break ;
			case 3:
printf(" watcher avoid TH3_BUDDYSEARCH_STAND \n" ) ;
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_BUDDYSEARCH_STAND ; 
				break ;
			case 4:
printf(" watcher avoid TH3_CLEARING_LOOKAROUND \n" ) ;
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_CLEARING_LOOKAROUND ; 
				break ;

		}
		entk->act->move_s = MoveWalk ;
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
		entk->avoinfo.check_num =  ENE_GetPearSafeZone( entk, 0 ) ;
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
			if ( entk->def_mapbit ) {
				ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit) ;
			} else {
				FVECTOR	l_pos ;
				int		l_map ;
				
				COM_GetPlayerLastPos( &l_pos, &l_map ) ;
				ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
			}
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

static void Think3_ClearingLookAround( entk )
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
		entk->bullet = 0 ;
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

	if (  intrpt < 0 ) {
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

static	void	Think3_MoveDefense( entk )
ENETHINK	*entk ;
{
	entk->thk_status |= THK_STATUS_MOVE ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		
		return ;
	}


	entk->act->dir = entk->trgpoint.dir ;
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
	    case TH3_CLEARING_LOOKAROUND :
	    	Think3_ClearingLookAround( entk ) ;
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

static	void	Think2_DefenseZone( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_LOOKAROUND :
	    	Think3_DefenseLookAround( entk ) ;
		break ;
	}
}

/*----- 高レベル思考チェック --------------------------------------------*/
#include "modechng.c"

static	void	AvoidMpdeCheck( entk )
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

	if ( entk->notice & ENE_NOTICE_FOUND ) {
		entk->think2 = TH2_FOUND ;
		THK_FoundModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_TRACE ) {
		entk->think2 = TH2_TRACE ;
		THK_TraceModeStart( entk ) ;
	}
	if ( entk->alert > 0 || entk->notice & (ENE_NOTICE_LIGHT|ENE_NOTICE_REGION) ) {
		entk->think2 = TH2_INDISTINCT ; 
		THK_IndistinctModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_UNIFORM ) {
		entk->think2 = TH2_UNIFORM ;
		THK_UniformModeStart( entk ) ;
	}
	if ( entk->notice & ENE_NOTICE_CORP ) {
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
	}

#ifdef HOLD_AVOID_ALERT
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		if (GM_AlertMode==ALERT_MODE_AVOID ) {
			GM_SeSetMode( SD_V_PDONMU01 , &entk->act->ctrl->mov, GM_SEMODE_BOMB ) ;
			entk->think2 = TH2_DISCOVERY ; 
			THK_DiscoveryModeStartAvoid( entk ) ;
			if ( ENE_CheckPlayerHidden() ) {
				SET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			}
		} else {
			entk->think2 = TH2_HOLDUP ;
			THK_HoldUpModeStart( entk ) ;
		}
	}
#else
	if ( entk->notice & ENE_NOTICE_HOLDUP ) {
		entk->think2 = TH2_HOLDUP ;
		THK_HoldUpModeStart( entk ) ;
	}
#endif

	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->think2 = TH2_DISCOVERY ;
		THK_DiscoveryNoiseModeStart( entk ) ;
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	} else {
		if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
			if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
				entk->think2 = TH2_DISCOVERY ; 
				THK_DiscoveryNpcModeStart( entk ) ;
			}
		}
	}
	
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他モードへ移行 */
	TouchModeChange( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	NoiseModeChange( entk ) ;
}

static	void	FoundModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_FOUND) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
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
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	TraceModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	IndistinctModeChange( entk ) ;
}

static	void	UniformModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_UNIFORM) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	UniformModeChange( entk ) ;
}


static	void	CorpsModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	CorpsModeChange( entk ) ;
}

static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	EneDamageModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	BoxModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	HoldUpModeChange( entk ) ;
}


static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		ENE_WatcherReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	DamageModeChange( entk ) ;
}

static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		回避
	*/
void	ENE_Watcher_Think1_Avoid( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_MOVE :
			Think2_MoveDefensePoint( entk ) ;
			AvoidMpdeCheck( entk ) ;
		break ;
	    case TH2_SEARCH :			/* 安全地帯確認 */
			Think2_Search( entk ) ;
			AvoidMpdeCheck( entk ) ;
		break ;
	    case TH2_NEAR_ZONE :
			Think2_NearZone( entk ) ;
			AvoidMpdeCheck( entk ) ;
		break ;
	    case TH2_DEFENSE_ZONE :		/* 守備位置 */
			Think2_DefenseZone( entk ) ;
			AvoidMpdeCheck( entk ) ;
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
}

/*----- スタートモード、巡回兵潜入モード --------------------------------------------*/
void	ENE_WatcherStartModeAvoid( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_AVOID ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

	entk->act->CheckPad = WatcherAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->avoinfo.check_num = 0 ;

	if ( entk->def_mapbit ) {
		ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit) ;
	} else {
		FVECTOR	l_pos ;
		int		l_map ;
		
		COM_GetPlayerLastPos( &l_pos, &l_map ) ;
		ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
	}
printf("def_pos %f %f %f \n",entk->def_pos.vx,entk->def_pos.vy,entk->def_pos.vz);
	entk->think1 = ENE_TH1_AVOID ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 

	entk->count3 = 0 ;

}

void	ENE_WatcherStartModeAvoidWarp( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think1 = ENE_TH1_AVOID ; 
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}

	entk->act->CheckPad = WatcherAvoidCheckPad ;

	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;
	entk->avoinfo.check_num = 0 ;

	if ( entk->def_mapbit ) {
		ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit) ;
	} else {
		FVECTOR	l_pos ;
		int		l_map ;
		
		COM_GetPlayerLastPos( &l_pos, &l_map ) ;
		ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
	}
	ENE_WarpNearPos( entk->ctrl, entk->trgpoint.addr, 2000/250 ) ;

printf("def_pos %f %f %f \n",entk->def_pos.vx,entk->def_pos.vy,entk->def_pos.vz);
	entk->think1 = ENE_TH1_AVOID ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 

	entk->count3 = 0 ;
}

void	ENE_WatcherReturnModeAvoid( entk )
ENETHINK	*entk ;
{
	int count ;
	
	entk->c_notice = 0 ;

	entk->act->CheckPad = WatcherAvoidCheckPad ;

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
			if ( entk->def_mapbit ) {
				ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit) ;
			} else {
				FVECTOR	l_pos ;
				int		l_map ;
				
				COM_GetPlayerLastPos( &l_pos, &l_map ) ;
				ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
			}
			entk->think3 = TH3_MOVE_DEFENSE ; 
		}
	} else {
		if ( entk->def_mapbit ) {
			ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit) ;
		} else {
			FVECTOR	l_pos ;
			int		l_map ;
			
			COM_GetPlayerLastPos( &l_pos, &l_map ) ;
			ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
		}
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
	}
	entk->count3 = 0 ;
}

void	ENE_WatcherStartModeAvoidDamage( entk )
ENETHINK	*entk ;
{

	entk->think1 = ENE_TH1_AVOID ; 

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think2 = TH2_DISCOVERY ; 
//11.18
#if 1
		THK_DiscoveryModeStartAvoid( entk ) ;
#else
		THK_DiscoveryModeStart( entk ) ;
#endif
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
