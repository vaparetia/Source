/*
	combat.c
	戦闘実験
	
	2002/02/22 Y.Korekado
	$Id: combat.c,v 1.1.1.3 2002/11/19 11:44:29 Yoshizawa1 Exp $
	
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"enemy.h"

enum {
	TH2_READY ,TH2_MOVE, TH2_COMBAT
} ;
enum {
	TH3_WAIT, TH3_RELOAD, TH3_ZONE_MOVE, TH3_DIRECT_MOVE, TH3_TURN_MOVE,
	TH3_BACK_UP, TH3_CHECK_END, TH3_STAND, TH3_TO_HIDE, TH3_APPROACH,
	TH3_RUN_R, TH3_RUN_L
} ;

enum {
	SP_NONE,
	SP_DOWNBACK,
	SP_NSIGHT_ON,	/* 暗視ゴーグルＯＮ */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_READYGUN,	/* 銃構える */
	SP_NEARATK,		/* 近接攻撃 */
	SP_MGUN_SHOT,	/* マシンガン撃つ */
	SP_RELOAD,		/* リロード */
	SP_MEDICATION,	/* 治療 */
	SP_KERI,		/* 蹴り */
	SP_HANG_WATCH,	/* 仲間が首締めされた  */
	SP_AROUND,		/* 首左右  */
	SP_RUN_R,	/* 右走り */
	SP_RUN_L,	/* 左走り */
} ;

#include	"cmbatact.c"

/*----- tmp_buff[0] --------------------------------------------*/
/*	entk->tmp_buff[0] ゴールポイント番号 */

/*----- 補助関数 --------------------------------------------*/
/* next_addr と aim_posからバックで移動するか判断 */
static int	CheckBackMove( ENETHINK *entk )
{
	int aim_dir, to_dir ;
	FVECTOR	next_pos ;
	
	aim_dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->aim_pos ) ;
	ENE_Zoneadd2Pos( &next_pos, entk->znavi->next_addr ) ;
	to_dir = _FVecTrgDir2( &entk->ctrl->mov, &next_pos ) ;

printf("aim_dir =%d  to_dir=%d\n",aim_dir, to_dir ) ;

	if ( entk->act->move_s == MoveBack ) {
		if ( _DiffDirAbs( aim_dir, to_dir ) < 900 ) return 0 ;
	} else {
		if ( _DiffDirAbs( aim_dir, to_dir ) < 1100 ) return 0 ;
	}
	
	return 1 ;
}

static int FriendStateCheck( ENETHINK *entk, ENETHINK *friend )
{
	if ( !(friend->thk_status & THK_STATUS_CHECKZONE) ) return 0 ;
	if ( friend->znavi->going_addr != entk->znavi->going_addr ) return 0 ;
	if ( HZX_GetRouteCrossGroup( friend->ctrl->addr, entk->ctrl->addr ) > 1 ) return 0 ;
	
	return 1 ;
}

static int	ENE_CautionZone( ENETHINK *entk )
{
	ZONENAVI	*znavi ;
	FVECTOR		pos ;
	int			i, g_id, tmp_addr, safe, safe_addr, min_num, num, dir ;

	znavi = entk->znavi ;
	g_id = HZX_ZoneGroupID( znavi->this_addr ) ;

	safe_addr = -1 ;
	min_num = HZX_NO_ZONE ;
	for(i=0;i<SAFE_NUM;i++){
		/* 候補地を取得 */
		safe = ENE_HZX_GetZone( znavi->this_addr )->safes[i];
		/* 候補地と現在地との角度 */
		tmp_addr = HZX_Address( g_id, safe, safe ) ;
		ENE_Zoneadd2Pos( &pos, tmp_addr ) ;
		dir = _FVecTrgDir2( &entk->ctrl->mov, &pos ) ;
		if ( _DiffDirAbs( dir, entk->ctrl->rot.vy ) < 1024 ) {
			if ( !WCOMM_SearchCheckZone( entk, tmp_addr ) ) {
				/* 候補地までのゾーン数 */
				num = HZX_GetRouteCrossGroup( znavi->this_addr, tmp_addr ) ;
				if ( num < min_num ) {
					min_num = num ;
					safe_addr = tmp_addr ;
				}
			}
		}
	}

	return safe_addr ;
}

static int CautionZone( ENETHINK *entk )
{
	ZONENAVI	*znavi ;
	HZX_ZON		*z ;
	u_char		*nears ;
	int			i, g_id, before, near, c_zone ;

	znavi = entk->znavi ;

//	before = HZX_Zone1( entk->before_inzone[1] ) ;
	before = HZX_Zone1( znavi->this_addr ) ;
	z = HZX_GetZoneFromAdd( znavi->next_addr ) ;
	g_id = HZX_ZoneGroupID( znavi->next_addr ) ;
	nears = z->nears ;
	c_zone = -1 ;

	for ( i = 0 ; i < 6 ; i++ ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if ( ENE_ReadOnlinInfo( znavi->this_addr, HZX_Address(g_id,near,near) ) ) {
//printf("	before zone[%x] this zone[%x] caution zone [%x] \n",
			c_zone = HZX_Address( g_id, near, near ) ;
			break ;
		}
	}

	return c_zone ;
}



/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_WaitReady( entk )
ENETHINK	*entk ;
{
//	if ( entk->count3 > 60 ) {
	if ( entk->count3 > 6 ) {
		int z ;
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_MOVE ;
		z = WCOMM_GetCurrentCheckZone( ) ;
		ENE_SetTrgpZoneaddr( &(entk->trgpoint), z ) ;
		entk->count3 = 0 ;
		return ;
	}

	entk->count3 ++ ;
}

static void Think3_ZoneMove( entk )
ENETHINK	*entk ;
{
	int c_zone ;
	ENETHINK	*ffriend ;

	SET_FLAG( entk->thk_status, THK_STATUS_CHECKZONE ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_MOVE ;
		entk->count3 = 0 ;

		return ;
	}

	/* 目標地点が変更されたら */
	if ( entk->trgpoint.addr != WCOMM_GetCurrentCheckZone() ) {
		entk->think2 = TH2_READY ; 
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 仲間チェック */
	if ( (ffriend = WCOMM_GetFrontFriend( entk, entk->znavi->going_addr )) != NULL ) {
		 if ( FriendStateCheck( entk, ffriend ) ) {
			entk->think3 = TH3_BACK_UP ;
			entk->count3 = 0 ;

			return ;
		}
	}

#if 1
	c_zone = ENE_CautionZone( entk ) ;
#else
	c_zone = CautionZone( entk ) ;
#endif
	if ( c_zone != -1 ) {
		ENE_Zoneadd2Pos( &entk->act->aim_pos, c_zone ) ;
		entk->act->aim_pos.vy += 1250.0f ;
		SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;

		if ( HZX_GetRouteCrossGroup( entk->znavi->this_addr, c_zone) <= 2 ) {
//			entk->act->move_s = MoveCautionWalk ;
			entk->act->move_s = MoveCautionRun ;
		} else {
			entk->act->move_s = MoveCautionRun ;
		}
	}
	entk->checkzone = c_zone ;
	{
		int aim_dir, to_dir ;
		FVECTOR	next_pos ;
		

		aim_dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->aim_pos ) ;
		ENE_Zoneadd2Pos( &next_pos, entk->znavi->next_addr ) ;
		to_dir = _FVecTrgDir2( &entk->ctrl->mov, &next_pos ) ;
//printf("[%d] 
		if ( _DiffDirAbs( aim_dir, to_dir ) >= 1024 ) {
//			entk->think3 = TH3_TURN_MOVE ;
//			entk->count3 = 0 ;
//			entk->trgpoint.dir = to_dir ;
			
//			return ;
		}
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectMove( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->think3 = TH3_CHECK_END ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static	void	Think3_TurnMove( entk )
ENETHINK	*entk ;
{
	SET_FLAG( entk->thk_status, THK_STATUS_CHECKZONE ) ;

	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 > 30 ) {
		entk->think3 = TH3_ZONE_MOVE ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* trgpoint.dirに方向が入っている */
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

static void Think3_BackUp( entk )
ENETHINK	*entk ;
{
	int c_zone ;
	ENETHINK	*ffriend ;

	SET_FLAG( entk->thk_status, THK_STATUS_CHECKZONE ) ;

	entk->act->pad = SP_CAUT_STAND ;

	/* 仲間チェック */
	if ( !(((ffriend = WCOMM_GetFrontFriend( entk, entk->znavi->going_addr )) != NULL ) &&
		 ( FriendStateCheck( entk, ffriend ) )) ) {
		entk->think3 = TH3_ZONE_MOVE ;
		entk->count3 = 0 ;

		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
//	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_CheckEnd( entk )
ENETHINK	*entk ;
{
	SET_FLAG( entk->thk_status, THK_STATUS_CHECKZONE ) ;

	if ( entk->count3 > 60 ) {
		if ( entk->ctrl->addr == WCOMM_GetCurrentCheckZone() ) {
			WCOMM_ClearCheckZone() ;
		}
		entk->think2 = TH2_READY ; 
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		return ;
	}

	/* 目標地点が変更されたら */
	if ( entk->trgpoint.addr != WCOMM_GetCurrentCheckZone() ) {
		entk->think2 = TH2_READY ; 
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
		
		return ;
	}

	entk->count3 ++ ;
}

static	void	Think3_ReloadReady( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ){
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_RELOAD ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_StandCombat( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 > 15 ) {
		
		entk->think3 = ( KR_RandU(2) ) ? TH3_RUN_L : TH3_RUN_R ;
		entk->count3 = 0 ;

		return ;
	}

	/* 銃を撃つ */
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_PULL_TRIGGER ) ;
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_TARGET_AIMPOS ) ;

	entk->act->aim_pos = GM_PlayerPosition ;
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_RunRight( entk )
ENETHINK	*entk ;
{
	int	aim_dir ;

	entk->act->pad = SP_RUN_L ;

	if ( entk->count3 > 60 ) {
		entk->think3 = TH3_TO_HIDE ;
		entk->count3 = 0 ;

		return ;
	}

	entk->act->aim_pos = GM_PlayerPosition ;

	aim_dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->aim_pos ) ;
	if ( _DiffDirAbs( aim_dir, entk->ctrl->rot.vy ) > 1024 )  {
		entk->think3 = TH3_TO_HIDE ;
		entk->count3 = 0 ;

		return ;
	}

	/* 銃を撃つ */
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_PULL_TRIGGER ) ;
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_TARGET_AIMPOS ) ;
	SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;

	/* 体に移動命令（あっち行けよ!おらぁ） */
//	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_RunLeft( entk )
ENETHINK	*entk ;
{
	int	aim_dir ;

	entk->act->pad = SP_RUN_L ;

	if ( entk->count3 > 60 ) {
		entk->think3 = TH3_TO_HIDE ;
		entk->count3 = 0 ;

		return ;
	}

	entk->act->aim_pos = GM_PlayerPosition ;

	aim_dir = _FVecTrgDir2( &entk->ctrl->mov, &entk->act->aim_pos ) ;
	if ( _DiffDirAbs( aim_dir, entk->ctrl->rot.vy ) > 1024 )  {
		entk->think3 = TH3_TO_HIDE ;
		entk->count3 = 0 ;

		return ;
	}

	/* 銃を撃つ */
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_PULL_TRIGGER ) ;
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_TARGET_AIMPOS ) ;
	SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;

	/* 体に移動命令（あっち行けよ!おらぁ） */
//	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_ToHide( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 == 0 ) {
		int	hideaddr ;

		hideaddr = ENE_RunawayZoneaddr( entk->ctrl, GM_PlayerControl ) ;
		ENE_SetTrgpZoneaddr( &(entk->trgpoint), hideaddr ) ;
	}

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->pl_eyei.dir ;
		entk->think3 = TH3_APPROACH ;
		entk->count3 = 0 ;
		entk->act->move_s = MoveCautionRun ;

		return ;
	}

	entk->act->aim_pos = GM_PlayerPosition ;
	entk->act->move_s = ( CheckBackMove( entk ) ) ? MoveBack : MoveCautionRun ;

	/* 銃を撃つ */
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_PULL_TRIGGER ) ;
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_TARGET_AIMPOS ) ;
	SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_Approach( entk )
ENETHINK	*entk ;
{
//	entk->act->pad = SP_CAUT_STAND ;

	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->pl_eyei.dir ;
		entk->think3 = TH3_STAND ;
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		if ( entk->pl_eyei.dis < 4000 )  {
			entk->act->dir = entk->pl_eyei.dir ;
			entk->think3 = TH3_STAND ;
			entk->count3 = 0 ;
			return ;
		}
	}

	/* 銃を撃つ */
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_PULL_TRIGGER ) ;
	SET_FLAG( entk->act->actgun.gun_pad, ACTGUN_TARGET_AIMPOS ) ;
	entk->act->aim_pos = GM_PlayerPosition ;

	/* 体に移動命令（あっち行けよ!おらぁ） */
	SET_FLAG( entk->status2, ENE_STATUS2_AIM_GUN ) ;
	entk->act->dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Ready( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_WAIT :
	    	Think3_WaitReady( entk ) ;
		break ;
	    case TH3_RELOAD :
	    	Think3_ReloadReady( entk ) ;
		break ;
	}
}

static	void	Think2_Move( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_MOVE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneMove( entk ) ;
		break ;
	    case TH3_DIRECT_MOVE :			/* 目標ポイントまでいく */
	    	Think3_DirectMove( entk ) ;
		break ;
	    case TH3_TURN_MOVE :			/* 目標ポイントまでいく */
	    	Think3_TurnMove( entk ) ;
		break ;
	    case TH3_BACK_UP :			/* 仲間のバックアップ */
	    	Think3_BackUp( entk ) ;
		break ;
	    case TH3_CHECK_END :			/* チェック終了 */
	    	Think3_CheckEnd( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_COMBAT ; 
		entk->think3 = TH3_STAND ;
		entk->count3 = 0 ;
		
		return ;
	}
}

static	void	Think2_Combat( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_STAND :
	    	Think3_StandCombat( entk ) ;
		break ;
	    case TH3_RUN_R :
	    	Think3_RunRight( entk ) ;
		break ;
	    case TH3_RUN_L :
	    	Think3_RunLeft( entk ) ;
		break ;
	    case TH3_TO_HIDE :
	    	Think3_ToHide( entk ) ;
		break ;
	    case TH3_APPROACH :
	    	Think3_Approach( entk ) ;
		break ;
	}
}

/*----- 高レベル思考モード --------------------------------------------*/
void	ENE_Think1_Combat( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_READY :
			Think2_Ready( entk ) ;
		break ;
	    case TH2_MOVE :
			Think2_Move( entk ) ;
		break ;
	    case TH2_COMBAT :
			Think2_Combat( entk ) ;
		break ;
	}
}

/*----- スタートモード、巡回兵危険モード --------------------------------------------*/
void ENE_StartModeCombat( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	entk->act->CheckPad = WatcherAlertCheckPad ;
	entk->act->dir = -1 ;
	entk->act->pad = SP_CAUT_STAND ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_SNEAK ;

	entk->think2 = TH2_READY ; 
	entk->think3 = TH3_WAIT ; 

	entk->tmp_buff[0] = 0 ;
	entk->count3 = 0 ;
	entk->tmp_time = 0 ;

	entk->act->move_s = MoveCautionRun ;
//	entk->act->move_s = MoveCautionWalk ;
}
