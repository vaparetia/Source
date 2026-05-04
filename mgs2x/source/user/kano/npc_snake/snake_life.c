//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   snake_life.c
   スネークのライフゲージ

   2001/02/07	M.Sonoyama
   2001/03/08   Modefied by K.Kano
   $Id: snake_life.c,v 1.1.1.3 2002/11/19 11:43:22 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "../../korekado/npc/npc.h"


/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX actor;
	GM_GageSet gage;
	NPCWORK *npc;
} Work ;


/*----------------------------------------------------------------*/

static void Act(Work *work)
{
	GM_GageSet *gs ;
	int value;
	NPCACT *act;

	gs=&work->gage;
	if(GM_IsGameOver()){
		GM_InvisibleGage(gs);
		return ;
	}

	act=&(work->npc->action);
	value=gs->value;

	if(act->life>value) value++;
	else if(act->life<value) value--;

	if(value>gs->max) value=gs->max;
	if(value<gs->min) value=gs->min;

	gs->value=value;


#if 0
	work->gage.flag &= ~GM_GAGE_WARNING ;
	if ( GM_CheckPlayerStatus( PLAYER_NO_BREATH ) || 
		 GM_WaterLevel > GM_PlayerBody->objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ) {
//	if ( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) {

		GM_VisibleGage( gs ) ;
		work->invisible_count = 0 ; 
		if ( GM_PlayerWork != NULL && ( GM_PlayerWork->pad->press & PAD_X ) ) {
			work->bear_count = 4 ;	/* PAD_Xは我慢ボタン */
		}

		if ( work->bear_count > 0 ) -- work->bear_count ;

		value = gs->value ;
		if ( PL_PadEnable() && work->bear_count <= 0 ) value -= 1 ; 
		if ( value < gs->min ) value = gs->min ;
		gs->value = value ;
		if ( gs->value == gs->min ) {
			/* 残量なし */
			/* ライフを減らす */
			if ( PL_PadEnable() ) GM_VitalityAdjust -= 1 ;
			if ( ( GV_Time % 30 ) == 0 ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				if ( GM_PlayerWork != NULL ) GM_PlayerWork->life.flag |= GM_GAGE_WARNING ;
			}
		} else if ( gs->value < gs->max / 3 ) {
			/* 1/3以下 */
			if ( ( GV_Time % 30 ) == 0 ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				work->gage.flag |= GM_GAGE_WARNING ;
			}
		} 
	} else {
		if ( gs->value < gs->max ) {
			gs->value += 16 ;
		}
		else {
			gs->value = gs->max ;
			if ( work->invisible_count < ( 300 / TIME_BASE ) * 4 ) {
				if ( ++ work->invisible_count >= ( 300 / TIME_BASE ) * 4 ) {
					GM_InvisibleGage( gs ) ;
				}
			}
		}
		work->bear_count = 0 ;
	}
#endif

}

static void Die( Work *work )
{
	GM_RemoveGageSet( &work->gage ) ;
}

/*----------------------------------------------------------------*/

static int GetResources(Work *work,NPCWORK *npc,int max)
{
	GM_GageSet *gs;

	work->npc=npc;

	if(npc->action.life<=0) npc->action.life=1;

	gs = &work->gage ;
#if 0
	GM_InitGageSet( gs, "SNAKE", 16, 196, 3, DIRECT_TICK( max ),
				    DIRECT_TICK( max ), 0, 30, GM_GAGE_LEVEL_NPC_LIFE ) ;
#else
	GM_InitGageSet( gs, "SNAKE", 16, 196, 3, max,
				    max, 0, 30, GM_GAGE_LEVEL_NPC_LIFE ) ;
#endif
	// gs->text_len = 23 ;
	// GM_SetGageColor( gs, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_NPC_LIFE );
	GM_AppendGageSet( gs ) ;

	GM_VisibleGage( gs ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/

/* スネークライフゲージ */
void *NewNPCSnakeLife(NPCWORK *npc,int max)
{
	Work *work;

	work=(Work *)GV_CreateActor(GV_ACTOR_AFTER,GV_CLASS_OBJECT,
								sizeof(Work),PLAYER_CHECK_ACTOR_PRIO);

	if(work!=NULL){
		GV_SetActor(&work->actor,Act,Die);
		GV_ActorEX(&work->actor);

		if(GetResources(work,npc,max)<0){
			GV_DestroyActor(work);
			return NULL ;
		}
	}
	return work ;
}

