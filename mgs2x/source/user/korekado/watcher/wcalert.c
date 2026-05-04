//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wctravel.c
	巡回兵 危険モード
	
	1999/07/29 Y.Korekado
	$Id: wcalert.c,v 1.1.1.3 2002/11/19 11:44:30 Yoshizawa1 Exp $
	
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
	TH2_READY ,TH2_CHASE ,TH2_MOVE,TH2_ATTACK,TH2_SEARCH, 
	TH2_PBREAK, TH2_MOVE_AREA, TH2_NPC_CHASE, TH2_NPC_ATTACK
} ;
enum {
	TH3_NSIGHT_ON, TH3_ZONE_CHASE, TH3_DIRECT_CHASE, TH3_ATTACK_SETUP, TH3_ATTACK_MGUN,
	TH3_ATTACK_RELOAD, TH3_ATTACK_NEAR, TH3_MEDICAL, TH3_MOVE_WAITAREA, TH3_WATCH,
	TH3_MORTALLY, TH3_HANG_WATCH, TH3_WARNING, TH3_ATTACK_SEARCH
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
} ;


#include	"wcaleact.c"

#define SHOOT_DIS	( entk->sense.eye_s_s[ALERT_MODE_ALERT] - 4000 )
//#define SHOOT_DIS	( entk->sense.eye_s_s[ALERT_MODE_ALERT] - 1000 )

//#define SHOOT_AREA_JIK	(1)
/*----- tmp_buff[0] --------------------------------------------*/
#define WC_ALERT_HANG_WATCH		0x00000001	/* 首絞められる仲間みた*/
#define WC_ALERT_ONE_SHOT		0x00000002	/* 一度は撃つ*/

/*----- 補助関数 --------------------------------------------*/
static int NpcChaseCheck( ENETHINK *entk )
{
	/* NPC追いかけ兵 */
	if ( !(entk->status & ENE_STATUS_NPC) )	return 0 ;

	/* NPC発見 */
	if ( !(COM_AlertStatus()&COM_ALERT_ATK_NPC) )	return 0 ;

	return 1 ;
}

static void SetAttackThink( ENETHINK *entk )
{
	if ( entk->status & ENE_STATUS_NIGHT_SIGHT && entk->act->sw->n_sight == 1 ) {
		entk->think2 = TH2_READY ; 
		entk->think3 = TH3_NSIGHT_ON ;
#ifdef SHOOT_AREA_JIK
	} else if ( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ) {
#else
	} else if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
#endif
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
	} else if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
	} else {
		if ( entk->def_mapbit ) {
			entk->think2 = TH2_MOVE ;
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit ) ;
		} else {
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		}
	}
//printf(" def_map = %x \n",entk->def_mapbit );
//printf("id[%d] def [%.2f][%.2f][%.2f]\n",entk->id, entk->def_pos.vx,entk->def_pos.vy,entk->def_pos.vz ) ;
	entk->count3 = 0 ;
}

static void SetPBreakThink ( entk )
ENETHINK *entk ;
{
	entk->think2 = TH2_PBREAK ;
	entk->count3 = 0 ;

printf(" SetPBreakThink	entk->pl_eyei.sight [%d]\n",entk->pl_eyei.sight ) ;

	/* 両足故障していたら */
	if ( (entk->act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
		entk->think3 = TH3_MORTALLY ;
		return ;
	}

	/* 両腕故障していたら */
	if ( (entk->act->bodyp.pbreak & PBREAK_ARMS) == PBREAK_ARMS ) {
		ENE_SetTrgpWaitPoint( entk, &(entk->trgpoint) ) ;
		entk->think3 = TH3_MOVE_WAITAREA ; 
		return ;
	}
	
	/* たま切れなら治療 */
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
printf("mag max\n");
		entk->think3 = TH3_MEDICAL ; 
		return ;
	}
	
	/* 片足なら */
	if ( entk->act->bodyp.pbreak & PBREAK_LEGS ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			/* 右手が無事なら */
			if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
				entk->think3 = TH3_ATTACK_SETUP ; 
				return ;
			}
printf("sight in but LEG & ARMR \n");
			entk->think3 = TH3_MEDICAL ; 
			return ;
		}

		entk->think3 = TH3_MEDICAL ; 
		return ;
	}

	/* 右手故障していたら */
	if ( entk->act->bodyp.pbreak & PBREAK_ARM_R ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
				!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
				!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
				entk->think3 = TH3_ATTACK_NEAR ;
				entk->count3 = 0 ;
				return ;
			}
		}
		entk->think3 = TH3_MEDICAL ; 
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		return ;
	}

	entk->think3 = TH3_MEDICAL ; 
}
static int HangCheck( ENETHINK *entk )
{
	int diff ;

	if ( !( GM_PlayerStatus & PLAYER_ENEMY_HANG ) ) return 1 ;

	/* 方向に隙 */
	diff = GV_DiffDirS( GM_PlayerControl->rot.vy, entk->ctrl->rot.vy ) ;
	if ( diff > 1024 ) return 0 ;
	if ( diff < -1024 ) return 0 ;

	return 1 ;
}


/*----- 低レベル思考モード --------------------------------------------*/
static void Think3_NSight_ON( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_NSIGHT_ON ;
	}

	if ( entk->act->act_end ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ; 
		} else {
			if ( NpcChaseCheck( entk ) ) {
				entk->think2 = TH2_NPC_CHASE ; 
				entk->think3 = TH3_ZONE_CHASE ;
				ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
			} else if ( entk->def_mapbit ) {
				entk->think2 = TH2_MOVE ;
				entk->think3 = TH3_ZONE_CHASE ;
				ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit ) ;
			} else {
				entk->think2 = TH2_CHASE ;
				entk->think3 = TH3_ZONE_CHASE ;
				ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
			}
		}
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( !(entk->tmp_buff[0] & WC_ALERT_ONE_SHOT) ) {
#ifdef SHOOT_AREA_JIK
		if ( entk->pl_eyei.sight < EYE_INFO_SIGHT_BLURR ) {
#else
		if ( (entk->pl_eyei.sight != EYE_INFO_SIGHT_IN) &&
			 (entk->pl_eyei.dis > 1500 ) ) {
#endif
			if ( NpcChaseCheck( entk ) ) {
				entk->think2 = TH2_NPC_CHASE ; 
				entk->think3 = TH3_ZONE_CHASE ;
				ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
			} else 
			if ( entk->def_mapbit ) {
				ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit ) ;
				if ( ENE_SameZone( &(entk->trgpoint.pos), 
						&(entk->znavi->flore_pos), entk->ctrl->hzx_id ) ) {
#if 0
static int a=0 ;
printf(" zone side [%d] \n",a ) ;
ENE_SetTrgpZoneSide( &(entk->trgpoint), entk->trgpoint.addr, a++ ) ;
if( a>=4 ) a = 0 ;
					entk->think2 = TH2_MOVE_AREA ; 
					entk->think3 = TH3_DIRECT_CHASE ; 
#else
					entk->think2 = TH2_SEARCH ; 
					entk->think3 = TH3_WATCH ; 
#endif
				} else {
					entk->think2 = TH2_MOVE ; 
					entk->think3 = TH3_ZONE_CHASE ;
				}
			} else {
				entk->think2 = TH2_CHASE ;
				entk->think3 = TH3_ZONE_CHASE ;
			}
			entk->count3 = 0 ;
			return ;
		}

		if( !HangCheck( entk ) ) {
			if ( entk->tmp_buff[0] & WC_ALERT_HANG_WATCH ) {
				entk->think3 = TH3_WARNING ; 
			} else {
				SET_FLAG( entk->tmp_buff[0], WC_ALERT_HANG_WATCH ) ;
				entk->think3 = TH3_HANG_WATCH ;
			}
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( !(GM_PlayerStatus & PLAYER_DEAD) ) {
		if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
			!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
			if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
				UNSET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
				entk->think3 = TH3_ATTACK_NEAR ;
				entk->count3 = 0 ;
				return ;
			}
		}

		if ( entk->count3 > COUNT_VMODE(20) ) {
			UNSET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
			entk->think3 = TH3_ATTACK_MGUN ;
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_AttackWatch( entk )
ENETHINK	*entk ;
{
	if ( (entk->pl_eyei.sight == EYE_INFO_SIGHT_IN) ||
		 (entk->pl_eyei.dis < 1000 ) ) {
		entk->act->pad = SP_READYGUN ;
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}

	if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
		entk->count3 = 0 ;

		return ;
	}

	entk->act->pad = SP_CAUT_STAND ;
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_ZoneChasePlayer( entk )
ENETHINK	*entk ;
{
//	ENE_SetTrgpEnemy( entk->znavi->hzd, entk->id+1, UNIT_WATCHER, &(entk->trgpoint) ) ;
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;

	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}

	if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN && entk->pl_eyei.dis < SHOOT_DIS ) {
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ;
			entk->count3 = 0 ;
			
			return ;
		} else {
			entk->act->aim_pos = GM_PlayerPosition ;
			entk->status2 |= ENE_STATUS2_AIM_GUN ;
		}
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChasePlayer( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_NEAR ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &entk->znavi->flore_pos, -1 ) ;
	}

	/* いつまでも直線じゃいられない */
	if ( ENE_DirectReachCheck( entk ) == 0 ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		if ( entk->pl_eyei.dis < SHOOT_DIS ) {
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ;
			entk->count3 = 0 ;
			
			return ;
		} else {
			entk->act->aim_pos = GM_PlayerPosition ;
			entk->status2 |= ENE_STATUS2_AIM_GUN ;
		}
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

static	void	Think3_DirectMoveArea( entk )
ENETHINK	*entk ;
{
	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 250 ) < 0 ) {
		entk->think2 = TH2_SEARCH ; 
		entk->think3 = TH3_WATCH ; 
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
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->count3 < 1 ) {
		entk->act->dir = entk->trgpoint.dir ;
	}

	entk->count3 ++ ;
}

static	void	Think3_HangWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 == 0 ) {
		ENE_SetHeadMark( entk->act, BODY21_HEAD, HEADMARK_BR ) ;
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		entk->think3 = TH3_WARNING ; 
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_HANG_WATCH ;
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_Warning( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( (entk->pl_eyei.sight != EYE_INFO_SIGHT_IN) ||
		 ( HangCheck( entk ) ) ) {
		if ( NpcChaseCheck( entk ) ) {
			entk->think2 = TH2_NPC_CHASE ; 
			entk->think3 = TH3_ZONE_CHASE ;
			ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
			entk->count3 = 0 ;

			return ;
		} else if ( entk->def_mapbit ) {
			ENE_SetTrgpDefense( &(entk->trgpoint), &entk->def_pos, entk->def_mapbit ) ;
			if ( ENE_SameZone( &(entk->trgpoint.pos), 
						&(entk->znavi->flore_pos), entk->ctrl->hzx_id ) ) {
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_WATCH ; 
			} else {
				entk->think2 = TH2_MOVE ; 
				entk->think3 = TH3_ZONE_CHASE ;
			}
		} else {
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
		}
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
		!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
		entk->think3 = TH3_ATTACK_NEAR ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}


#define SHOT_BRANK (4)	/*ＰＡＬでも同じにする*/
static	void	Think3_AttackMgun( entk )
ENETHINK	*entk ;
{
	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		if ( entk->act->bodyp.pbreak & PBREAK_ARMS ) {	/*片腕怪我していたら */
			SetPBreakThink( entk ) ;
		} else {
			entk->think3 = TH3_ATTACK_RELOAD ; 
			entk->count3 = 0 ;
		}
		return ;
	}

	if ( !(entk->count3 % SHOT_BRANK ) ) {
		if ( !(BP_PS2_rand()%(SHOT_BRANK-1)) ) {
			entk->act->pad = SP_READYGUN ;
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
	if ( entk->count3 == 0 ){
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

/* この時点ではダメージ部位は確定しているはず */
static void Think3_PB_ReadyGun( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_READYGUN ;

	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		SetPBreakThink( entk ) ;
		return ;
	}

	if ( entk->pl_eyei.dis < NEAR_ATK_DIS && 
		!(entk->act->bodyp.pbreak & PBREAK_LEGS) && /* 両足が無事 */
		!(GM_PlayerStatus & (PLAYER_GROUND|PLAYER_DAMAGED|PLAYER_DOWNED)) ) {
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
		SetPBreakThink( entk ) ;
	}
	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_Medical( entk )
ENETHINK	*entk ;
{

#if 1	//11.15
//	if ( entk->count3 == 0 ) {
		entk->act->pad = SP_MEDICATION ;
//	}
#else
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
#endif

	if ( entk->act->act_end ) {
		entk->act->pad = 0 ;
		SetAttackThink( entk ) ;
		return ;
	}

	entk->count3 ++ ;
	return ;
}

static	void	Think3_MoveWaitArea( entk )
ENETHINK	*entk ;
{
#if 0
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		
		return ;
	}
#endif

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

static void Think3_PBAttackWatch( entk )
ENETHINK	*entk ;
{
	if ( entk->count3 > COUNT_VMODE(60) ) {
		SetPBreakThink ( entk ) ;
		return ;
	}

	if ( !(entk->act->bodyp.pbreak & PBREAK_ARM_R) ) {
		entk->act->pad = SP_CAUT_STAND ;
		entk->act->dir = entk->pl_eyei.dir ;
	}

	entk->count3 ++ ;
}


static void Think3_ZoneChaseNpc( entk )
ENETHINK	*entk ;
{
	int intrpt ;

	ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;

#if 1
	intrpt = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ;

	if (  intrpt < 0 ) {
		if ( intrpt == -2 ) {	/* 次のゾーンが進入禁止ゾーン */
			entk->think2 = TH2_NPC_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ;
		} else {
			entk->act->dir = entk->trgpoint.dir ;
			entk->think3 = TH3_DIRECT_CHASE ;
		}
		entk->count3 = 0 ;
		return ;
	}
#else
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint), entk->ctrl, entk->count3 ) ) {
		entk->act->dir = entk->trgpoint.dir ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		
		return ;
	}
#endif

	if ( (entk->pl_eyei.sight == EYE_INFO_SIGHT_IN) ) {
		if ( (entk->pl_eyei.dis < NEAR_ATK_DIS)  || 
			!(COM_UnitThkStatus( entk->g_id, entk->u_id )&THK_STATUS_ATTACK_PLAYER) ) {
			SET_FLAG( entk->thk_status, THK_STATUS_ATTACK_PLAYER ) ;
			entk->act->pad = SP_READYGUN ;
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( entk->npc_eyei.sight >= EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
		entk->think2 = TH2_NPC_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;
		
		return ;
	}

	/* 体に移動命令（あっち行けよ!おらぁ） */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static	void	Think3_DirectChaseNpc( entk )
ENETHINK	*entk ;
{
	ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;

	if ( ENE_ZoneIntrptCheck( entk->trgpoint.addr ) ) {
		entk->think2 = TH2_NPC_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( ENE_DirectTrace( &(entk->trgpoint), &(entk->ctrl->mov), 350 ) < 0 ) {
		SET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
		entk->think2 = TH2_NPC_ATTACK ; 
		entk->think3 = TH3_ATTACK_NEAR ; 
		entk->count3 = 0 ;
		
		return ;
	}

	if( entk->count3 & 0x20){
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &entk->znavi->flore_pos, -1 ) ;	}

	/* いつまでも直線じゃいられない */
	if ( ENE_DirectReachCheck( entk ) == 0 ) {
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->act->pad = SP_READYGUN ;
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}

	if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
		SET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
		entk->think2 = TH2_NPC_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ;
		entk->count3 = 0 ;

		return ;
	}

	
	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

static void Think3_NpcReadyGun( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->pad = SP_READYGUN ;

	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( !(entk->tmp_buff[0] & WC_ALERT_ONE_SHOT) ) {
		if ( entk->count3 > COUNT_VMODE(30) ) {
			if ( entk->npc_eyei.sight != EYE_INFO_SIGHT_IN ) {
//printf("Think3_NpcReadyGun from[%x] to[%x]\n",entk->ctrl->addr, *entk->npc_eyei.addr ) ;
				if( GM_GetZIntrptZ2Z( entk->ctrl->addr, *entk->npc_eyei.addr ) < 0 ) {
					entk->think2 = TH2_NPC_CHASE ;
					entk->think3 = TH3_ZONE_CHASE ;
					entk->count3 = 0 ;
					return ;
				}
			}
		}

		if ( entk->count3 > COUNT_VMODE(40) ) {
			if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
				SET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
				entk->act->pad = SP_READYGUN ;
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_ATTACK_SETUP ; 
				entk->count3 = 0 ;

				return ;
			}
		}
	}

	if ( !(GM_PlayerStatus & PLAYER_DEAD) ) {
		if ( entk->npc_eyei.dis < NEAR_ATK_DIS ) {
			UNSET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
			entk->think3 = TH3_ATTACK_NEAR ;
			entk->count3 = 0 ;
			return ;
		}
		if ( entk->count3 > COUNT_VMODE(60) ) {
			UNSET_FLAG( entk->tmp_buff[0], WC_ALERT_ONE_SHOT ) ;
			entk->think3 = TH3_ATTACK_MGUN ;
			entk->count3 = 0 ;
			return ;
		}
	}

	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_NpcAttackMgun( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( entk->bullet >= MAX_MAGAZINE_MG ) {
		if ( entk->act->bodyp.pbreak & PBREAK_ARMS ) {	/*片腕怪我していたら */

/*後で*/
			SetPBreakThink( entk ) ;
		} else {
			entk->think3 = TH3_ATTACK_RELOAD ; 
			entk->count3 = 0 ;
		}
		return ;
	}

	if ( !(entk->count3 % SHOT_BRANK ) ) {
		if ( !(BP_PS2_rand()%(SHOT_BRANK-1)) ) {
			entk->act->pad = SP_READYGUN ;
//			entk->think3 = TH3_ATTACK_SEARCH ; 
			entk->think3 = TH3_ATTACK_SETUP ; 
			entk->count3 = 0 ;
			return ;
		}
		entk->act->pad = SP_MGUN_SHOT ;
		entk->bullet ++ ;
	} else {
		entk->act->pad = SP_READYGUN ;
	}

	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

static	void	Think3_NpcAttackReload( entk )
ENETHINK	*entk ;
{
	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->act->aim_pos = *(entk->npc_eyei.pos) ;

	if ( entk->count3 == 0 ){
		entk->act->pad = SP_RELOAD ;
		entk->bullet = 0 ;
	}

	if ( entk->act->act_end ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->act->dir = entk->npc_eyei.dir ;

	entk->count3 ++ ;
}

static void Think3_NpcAttackSearch( entk )
ENETHINK	*entk ;
{
//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
//	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->act->pad = SP_READYGUN ;

	if ( entk->count3 > COUNT_VMODE(180) ) {
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;
		return ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->act->pad = SP_READYGUN ;
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_ATTACK_SETUP ; 
		entk->count3 = 0 ;

		return ;
	}

	entk->act->dir = entk->pl_eyei.dir ;

	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
static	void	Think2_Ready( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_NSIGHT_ON :
	    	Think3_NSight_ON( entk ) ;
		break ;
	}
}


static	void	Think2_Chase( entk )
ENETHINK	*entk ;
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

	if ( (entk->pl_eyei.sight == EYE_INFO_SIGHT_IN) && 
		 !(GM_PlayerStatus & (PLAYER_BEYOND)) ) {
		if ( (entk->pl_eyei.dis < NEAR_ATK_DIS)  || 
			!(COM_UnitThkStatus( entk->g_id, entk->u_id )&THK_STATUS_ATTACK_PLAYER) ) {
			SET_FLAG( entk->thk_status, THK_STATUS_ATTACK_PLAYER ) ;
			entk->think2 = TH2_ATTACK ; 
			entk->think3 = TH3_ATTACK_SETUP ;
			entk->count3 = 0 ;
			return ;
		}
	}

	if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
		entk->count3 = 0 ;

		return ;
	}
}

static	void	Think2_MoveDefenseArea( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMoveArea( entk ) ;
		break ;
	}

	if ( NpcChaseCheck( entk ) ) {
		entk->think2 = TH2_NPC_CHASE ; 
		entk->think3 = TH3_ZONE_CHASE ;
		ENE_SetTrgpEyei( &entk->npc_eyei, &entk->trgpoint ) ;
		entk->count3 = 0 ;

		return ;
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
	SET_FLAG( entk->thk_status, THK_STATUS_ATTACK_PLAYER ) ;

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
	    case TH3_HANG_WATCH :
	    	Think3_HangWatch( entk ) ;
		break ;
	    case TH3_WARNING :
	    	Think3_Warning( entk ) ;
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
	    case TH3_WATCH :
	    	Think3_PBAttackWatch( entk ) ;
		break ;

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

static	void	Think2_NpcChase( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :			/* 目標ゾーンまで行く */
	    	Think3_ZoneChaseNpc( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :			/* 同じゾーン内で追いかける */
	    	Think3_DirectChaseNpc( entk ) ;
		break ;
	}
}

static	void	Think2_NpcAttack( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ATTACK_SETUP :
	    	Think3_NpcReadyGun( entk ) ;
		break ;
	    case TH3_ATTACK_MGUN :
	    	Think3_NpcAttackMgun( entk ) ;
		break ;
	    case TH3_ATTACK_RELOAD :
	    	Think3_NpcAttackReload( entk ) ;
		break ;
	    case TH3_ATTACK_NEAR :
	    	Think3_AttackNear( entk ) ;
		break ;

	    case TH3_ATTACK_SEARCH :
	    	Think3_NpcAttackSearch( entk ) ;
		break ;
	}
}


/*----- 高レベル思考モード --------------------------------------------*/
	/*
		危険
	*/
void	ENE_Watcher_Think1_Alert( entk )
ENETHINK	*entk ;
{
	switch ( entk->think2 ) {
	    case TH2_READY :
			Think2_Ready( entk ) ;
		break ;
	    case TH2_CHASE :
			Think2_Chase( entk ) ;
		break ;
	    case TH2_MOVE :
			Think2_MoveDefensePoint( entk ) ;
		break ;
	    case TH2_MOVE_AREA :
			Think2_MoveDefenseArea( entk ) ;
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

	    case TH2_NPC_CHASE :
			Think2_NpcChase( entk ) ;
		break ;
		case TH2_NPC_ATTACK :
			Think2_NpcAttack( entk ) ;
		break ;
	}

	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = MAX_ALERT_LEVEL ;
		COM_SetAlertStatus( COM_ALERT_PLAYER_DETECT ) ;
	}
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		if ( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN ) {
			entk->alert = MAX_ALERT_LEVEL ;
			COM_SetAlertStatus( COM_ALERT_ATK_NPC ) ;
		}
	}

	if ( entk->notice & (ENE_NOTICE_NOISE_M|ENE_NOTICE_NOISE_L) ) {
		entk->alert = MAX_ALERT_LEVEL ;
		if ( !(COM_AlertStatus() & (COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)) ) {
#ifdef GMDEF_NOISE_MAP_RENEW
			COM_SetPlayerLastPos( &GM_NoisePosition, GM_NoiseHzxID ) ;/*ノイズの場所*/
#else
			COM_SetPlayerLastPos( &GM_NoisePosition, GM_GetHzxGroupID(GM_NoiseMap) ) ;/*ノイズの場所*/
#endif
		}
	}

	/* 警備兵はイントルード等になったプレイヤーは１秒で見えなくなる */
	if ( entk->iknow_flag & IKNOW_HIDDEN_PLAYER ) {
		entk->tmp_time ++ ;
		if ( entk->tmp_time > COUNT_VMODE(60) ) {
			entk->tmp_time = 0 ;
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
		}
	}

}

/*----- スタートモード、巡回兵危険モード --------------------------------------------*/
void ENE_WatcherStartModeAlert( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	entk->act->CheckPad = WatcherAlertCheckPad ;
	entk->act->dir = -1 ;
//	entk->act->pad = 0 ;
	entk->act->pad = SP_CAUT_STAND ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_ALERT ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_PBREAK ;
		entk->think3 = TH3_WATCH ;
		entk->count3 = 0 ;
	} else {
		if ( !(entk->def_mapbit) ) {	/* 追跡兵なら */
			ENE_WarpNearPos( entk->ctrl, GM_PlayerAddress, 2000/250 ) ;
		}
		SetAttackThink( entk ) ;
	}

	entk->tmp_buff[0] = 0 ;
	entk->count3 = 0 ;
	entk->tmp_time = 0 ;

	entk->act->move_s = MoveRun ;
//UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;/* 仮 */
	CLEAR_FLAG( entk->iknow_flag ) ;

	COM_StopRadioNoAccident( entk ) ;
}

void ENE_WatcherStartModeAlertWarp( entk )
ENETHINK	*entk ;
{
	entk->c_notice = 0 ;

	entk->act->CheckPad = WatcherAlertCheckPad ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->act->keep_pad = -1 ;

	entk->think1 = ENE_TH1_ALERT ;

	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_PBREAK ;
		entk->think3 = TH3_WATCH ;
		entk->count3 = 0 ;
	} else {
		HZX_ZONE_ADD	addr ;

		if ( entk->def_mapbit ) {
			addr = HZX_GetAddress( entk->def_mapbit, &entk->def_pos, -1 ) ;
		} else {
			addr = GM_PlayerAddress ;
		}
		ENE_WarpNearPos( entk->ctrl, addr, 2000/250 ) ;
		SetAttackThink( entk ) ;
	}

printf("wcalllllert[%f][%f][%f]\n",entk->ctrl->mov.vx,entk->ctrl->mov.vy,entk->ctrl->mov.vz ) ;
	entk->tmp_buff[0] = 0 ;
	entk->count3 = 0 ;
	entk->tmp_time = 0 ;

	entk->act->move_s = MoveRun ;
//UNSET_FLAG( entk->sw.radio, SW_FLAG_BREAK ) ;/* 仮 */
	CLEAR_FLAG( entk->iknow_flag ) ;
}
