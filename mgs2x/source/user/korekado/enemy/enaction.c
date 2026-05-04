//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	action.c
	敵兵用、汎用アクションコントロール

	1999/07/07 Y.Korekado
	$Id: enaction.c,v 1.1.1.3 2002/11/19 11:44:07 Yoshizawa1 Exp $
	
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

#include	"libgv.h"
#include	"libmt.h"

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

#include	"enemy.h"
#include	"motion.h"
#include	"mt_array.h"

//#define RADIO_PAD_CHECK_NEW	(0)

//#define ZETTAI_JIK
/*-------------------------------------------------------------------*/
#if 0 /* yano 2002.03.11 */
static void PBreakOverHoldUp( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverHoldUp( ACTION *act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;

	layer = LAYER_BASE ;
	mask = MOTION_MASK_FULL ;
	motion = 0 ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_LEGS) ) {
		switch( b_flag ) {
			case PBREAK_LEG_R :
				motion = EM_legr_idle ;
				break ;
			case PBREAK_LEG_L :
				motion = EM_legl_idle ;
				break ;
			case PBREAK_LEGS :
				motion = EM_legs_idle ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		mask = MOTION_MASK_UPPER ;
		layer ++ ;
	}
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = EM_armr_nom_idle_holdup ;
				break ;
			case PBREAK_ARM_L :
				motion = EM_arml_nom_idle_holdup ;
				break ;
			case PBREAK_ARMS :
				motion = EM_arms_nom_idle_holdup ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		layer ++ ;
	} else {
//	if ( layer == LAYER_BASE ) {
		SetMarAction( act, layer, data, 0, mask, interp, mar ) ;
		layer ++ ;
	}
	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
static void PBreakOverHoldUpBase( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverHoldUpBase( ACTION *act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;
	long64 marge ;

	layer = LAYER_BASE ;
	motion = 0; 

	ReSetMarAction( act, layer, data, m_time, basemask, interp, mar ) ;
	layer ++ ;
	marge = 0 ;

	mask = MOTION_MASK_FULL ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_LEGS) ) {
		switch( b_flag ) {
			case PBREAK_LEG_R :
				motion = EM_legr_idle ;
				mask = MOTION_MASK_LEG_R ;
				marge |= 1 << HUMAN21_MIGI_ASHI1 ;
				break ;
			case PBREAK_LEG_L :
				motion = EM_legl_idle ;
				mask = MOTION_MASK_LEG_L ;
				marge |= 1 << HUMAN21_HIDARI_ASHI1 ;
				break ;
			case PBREAK_LEGS :
				motion = data ;
				mask = MOTION_MASK_UPPER ;
				ReSetMarAction( act, LAYER_BASE, EM_legs_idle, m_time, basemask, interp, mar ) ;
				break ;
		}
		ReSetMarAction( act, layer, motion, m_time, mask, interp, mar ) ;
#ifdef ZETTAI_JIK
		act->body->m_ctrl->mt3_ctrl[ layer ].merge_flag |= marge ;
#endif
		layer ++ ;
	}
	marge = 0 ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = EM_armr_nom_idle_holdup ;
				mask = MOTION_MASK_ARM_R ;
				marge |= 1 << HUMAN21_MIGI_KATA ;
				break ;
			case PBREAK_ARM_L :
				motion = EM_arml_nom_idle_holdup ;
				mask = MOTION_MASK_ARM_L ;
				marge |= 1 << HUMAN21_HIDARI_KATA ;
				break ;
			case PBREAK_ARMS :
				motion = EM_arms_nom_idle_holdup ;
				mask = MOTION_MASK_ARMS ;
				break ;
		}
		ReSetMarAction( act, layer, motion, m_time, mask, interp, mar ) ;
#ifdef ZETTAI_JIK
		act->body->m_ctrl->mt3_ctrl[ layer ].merge_flag |= marge ;
#endif
		layer ++ ;
	}

	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
static void PBreakOverStand( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverStand( ACTION *act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;

	layer = LAYER_BASE ;
	mask = MOTION_MASK_FULL ;
	motion = 0 ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_LEGS) ) {
		switch( b_flag ) {
			case PBREAK_LEG_R :
				motion = EM_legr_idle ;
				break ;
			case PBREAK_LEG_L :
				motion = EM_legl_idle ;
				break ;
			case PBREAK_LEGS :
				motion = EM_legs_idle ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		mask = MOTION_MASK_UPPER ;
		layer ++ ;
	}
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = EM_armr_idle ;
				break ;
			case PBREAK_ARM_L :
				motion = EM_arml_idle ;
				break ;
			case PBREAK_ARMS :
				motion = EM_arms_idle ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		layer ++ ;
	}
	if ( layer == LAYER_BASE ) {
		SetMarAction( act, 0, data, 0, basemask, interp, mar ) ;
		layer ++ ;
	}
	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0
static void PBreakOverMove( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverMove( ACTION	*act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;

	layer = LAYER_BASE ;
	mask = MOTION_MASK_FULL ;
	motion = 0 ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_LEGS) ) {
//printf("LEGS DAMAGE base[%d] \n",act->bodyp.pbreak ) ;
		switch( b_flag ) {
			case PBREAK_LEG_R :
				motion = (act->move_s < MoveRun) ? EM_legr_walk : EM_legr_run ;
				break ;
			case PBREAK_LEG_L :
				motion = (act->move_s < MoveRun) ? EM_legl_walk : EM_legl_run ;
				break ;
			case PBREAK_LEGS :
				motion = EM_legs_idle ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		layer ++ ;
		mask = MOTION_MASK_UPPER ;
	}
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
//printf("ARM DAMAGE base[%d] \n",act->bodyp.pbreak ) ;
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = (act->move_s < MoveRun) ? EM_armr_walk : EM_armr_run ;
				break ;
			case PBREAK_ARM_L :
				motion = (act->move_s < MoveRun) ? EM_arml_walk : EM_arml_run ;
				break ;
			case PBREAK_ARMS :
				motion = (act->move_s < MoveRun) ? EM_arms_walk : EM_arms_run ;
				break ;
		}
		SetMarAction( act, layer, motion, 0, mask, interp, mar ) ;
		layer ++ ;
	}
	if ( layer == LAYER_BASE ) {
//printf("NO DAMAGE base[%d] \n",act->bodyp.pbreak ) ;
		SetMarAction( act, LAYER_BASE, data, 0, basemask, interp, mar ) ;
		layer ++ ;
	}
	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
static void PBreakOverBase( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverBase( ACTION	*act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;
	long64 marge ;

	layer = LAYER_BASE ;
	motion = 0; 

	ReSetMarAction( act, layer, data, m_time, basemask, interp, mar ) ;
	layer ++ ;
	marge = 0 ;

	mask = MOTION_MASK_FULL ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_LEGS) ) {
		switch( b_flag ) {
			case PBREAK_LEG_R :
				motion = EM_legr_idle ;
				mask = MOTION_MASK_LEG_R ;
				marge |= 1 << HUMAN21_MIGI_ASHI1 ;
				break ;
			case PBREAK_LEG_L :
				motion = EM_legl_idle ;
				mask = MOTION_MASK_LEG_L ;
				marge |= 1 << HUMAN21_HIDARI_ASHI1 ;
				break ;
			case PBREAK_LEGS :
				motion = data ;
				mask = MOTION_MASK_UPPER ;
				ReSetMarAction( act, LAYER_BASE, EM_legs_idle, m_time, basemask, interp, mar ) ;
				break ;
		}
		ReSetMarAction( act, layer, motion, m_time, mask, interp, mar ) ;
#ifdef ZETTAI_JIK
		act->body->m_ctrl->mt3_ctrl[ layer ].merge_flag |= marge ;
#endif
		layer ++ ;
	}
	marge = 0 ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = EM_armr_idle ;
				mask = MOTION_MASK_ARM_R ;
				marge |= 1 << HUMAN21_MIGI_KATA ;
				break ;
			case PBREAK_ARM_L :
				motion = EM_arml_idle ;
				mask = MOTION_MASK_ARM_L ;
				marge |= 1 << HUMAN21_HIDARI_KATA ;
				break ;
			case PBREAK_ARMS :
				motion = EM_arms_idle ;
				mask = MOTION_MASK_ARMS ;
				break ;
		}
		ReSetMarAction( act, layer, motion, m_time, mask, interp, mar ) ;
#ifdef ZETTAI_JIK
		act->body->m_ctrl->mt3_ctrl[ layer ].merge_flag |= marge ;
#endif
		layer ++ ;
	}

	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
static void PBreakOverBaseHandOnly( act, n_layer, data, m_time, basemask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	basemask ;
int		interp ;
int		mar ;
#else
static void PBreakOverBaseHandOnly( ACTION	*act, int n_layer, int data, int m_time, u_long64 basemask, int interp, int mar )
#endif
{
	int	b_flag, mask, motion, layer ;
	long64 marge ;

	layer = LAYER_BASE ;
	motion = 0; 

	ReSetMarAction( act, layer, data, m_time, basemask, interp, mar ) ;
	layer ++ ;
	marge = 0 ;
	mask = MOTION_MASK_FULL ;
	if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
		switch( b_flag ) {
			case PBREAK_ARM_R :
				motion = EM_armr_idle ;
				mask = MOTION_MASK_ARM_R ;
				marge |= 1 << HUMAN21_MIGI_KATA ;
				break ;
			case PBREAK_ARM_L :
				motion = EM_arml_idle ;
				mask = MOTION_MASK_ARM_L ;
				marge |= 1 << HUMAN21_HIDARI_KATA ;
				break ;
			case PBREAK_ARMS :
				motion = EM_arms_idle ;
				mask = MOTION_MASK_ARMS ;
				break ;
		}
		ReSetMarAction( act, layer, motion, m_time, mask, interp, mar ) ;
#ifdef ZETTAI_JIK
		act->body->m_ctrl->mt3_ctrl[ layer ].merge_flag |= marge ;
#endif
		layer ++ ;
	}

	for ( ;layer<=LAYER_OVER2; layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
void ENE_SetMarActionPBreak( act, n_layer, data, m_time, mask, interp, over, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		over ;	/* オーバーライドフラグ */
int		mar ;
#else
void ENE_SetMarActionPBreak( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp, int over, int mar )
#endif
{
	switch ( over ) {
		case PBREAK_OVER_NONE :
			SetMarAction( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_STAND :
			PBreakOverStand( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_MOVE :
			PBreakOverMove( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_BASE :
			PBreakOverBase( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_HOLDUP :
			PBreakOverHoldUp( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_HOLDUP_BASE :
			PBreakOverHoldUpBase( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
		case PBREAK_OVER_BASE_HAND_ONLY :
			PBreakOverBaseHandOnly( act, n_layer, data, m_time, mask, interp, mar ) ;
			break ;
	}
}

/* ベースモーション使用 */
#if 0 /* yano 2002.03.11 */
void ENE_SetActionPBreak( act, n_layer, data, m_time, mask, interp, over )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		over ;	/* オーバーライドフラグ */
#else
void ENE_SetActionPBreak( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp, int over )
#endif
{
	ENE_SetMarActionPBreak( act, n_layer, data, m_time, mask, interp,over, act->name_id->motion ) ;
}

static void SetMoveStart( act, time )
ACTION	*act ;
int		time ;
{
	if ( act->move_s == MoveWalk || act->move_s == MoveWalkGun ) {
#if 0
		act->tmp_dir = act->dir ;

		diffdir = GV_DiffDirS( act->ctrl->rot.vy, act->dir ) ;
		if ( diffdir > 0 ){
			if ( diffdir < 1024+64 ) {
				act->keep_mot = EM_turn_l_t ;
			} else {
				subdir = diffdir - (1024+64) ;
				act->ctrl->turn.vy = act->ctrl->rot.vy + subdir ;
				act->keep_mot = EM_turn_l_t ;
			}
		} else {
			if ( diffdir > -1024-64 ) {
				act->keep_mot = EM_turn_r_t ;
			} else {
#if 1
				subdir = diffdir + (1024+64) ;
				act->ctrl->turn.vy = act->ctrl->rot.vy + subdir ;
				act->keep_mot = EM_turn_r_t ;
#else
				act->keep_mot = EM_turn_b_t ;
#endif
			}
		}
//printf("move start tmpdir[%d] dir[%d]\n",act->tmp_dir,act->ctrl->turn.vy ) ;
#else
//		act->ctrl->interp = 15 ;
		act->ctrl->interp = 30 ;
		act->ctrl->turn.vy  = act->dir ;
#endif
		SetMode( act, ENE_ActMoveStart ) ;
	} else if ( 
		act->move_s == MoveNoriNoriWalk || 
		act->move_s == MoveCautionWalk || 
		act->move_s == MoveCautionWalkLow ||
		act->move_s == MoveCautionRun || 
		act->move_s == MoveSideL || 
		act->move_s == MoveSideR ||
		act->move_s == MoveBack 
		) {
		act->ctrl->interp = 15  ;
		act->ctrl->turn.vy = act->dir ;
		SetMode( act, ENE_ActMove ) ;
	} else {
		act->ctrl->turn.vy = act->dir ;
		SetMode( act, ENE_ActMove ) ;
	}

}

static int TurnCheck( act )
ACTION	*act ;
{
	int	dir, diff ;
	
	dir = MatToYRot( &BODYWORLD( act->body, HUMAN21_KOSHI ) ) ;
	diff = GV_DiffDirAbs( dir, act->tmp_dir ) ;
//printf("dir[%d] act->tmp_dir[%d] diff [%d] \n",dir,act->tmp_dir,diff ) ;
	if ( diff < 192 ) {
		return 1 ;
	}

	return 0 ;
}

static int TurnDirCheck( act )
ACTION	*act ;
{
	int	diff ;
	
	diff = GV_DiffDirAbs( act->ctrl->rot.vy, act->ctrl->turn.vy ) ;
//printf("dir[%d] act->tmp_dir[%d] diff [%d] \n",dir,act->tmp_dir,diff ) ;
	if ( diff < 192 ) {
		return 1 ;
	}

	return 0 ;
}


static void FootNoise( act )
ACTION	*act ;
{
	MT3_CONTROL *mt3_ctrl ;
	int len, r_foot, l_foot, t ;

	mt3_ctrl = &act->body->m_ctrl->mt3_ctrl[0] ;


   len = COUNT_VMODE(mt3_ctrl->file_header->motion_length) ;

   t = mt3_ctrl->time ;
	
	l_foot = 1 + 6 ;
	r_foot = (len/2) + 1 + 8 ;
	
	if ( t == l_foot || t == l_foot+4 ) {
		int r ;
		
		r = KR_RandU(4) ;
//		if (act->name_id->id % 2) r += 2 ;
		GM_SeSetMode( SD_V_KACHA01+r, &act->ctrl->mov, GM_SEMODE_REAL ) ;
	}


	if ( t == r_foot ) {
		int r ;
		
		r = KR_RandU(4) ;
//		if (act->name_id->id % 2) r += 2 ;
		GM_SeSetMode( SD_V_KACHA01+r, &act->ctrl->mov, GM_SEMODE_REAL ) ;
	}
}

int	ENE_StandMotion( ACTION *act )
{


	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_idle ;

	if ( act->bodyp.pbreak & PBREAK_ARM_R )	return EM_dam_okaji ;

	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_nom_idle ;

	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_m4_nom_idle ;

	if ( ENE_PureAttacker( act ) ) return  EM_cle_stand02 ;

	return EM_stand ;
}

int	ENE_CleStandMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_idle ;

	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_ak_cle_signe_idle ;

	return EM_cle_stand02 ;
}

int	ENE_WalkMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_cle_walk ;

	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_nom_walk ;

	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_ak_nom_walk ;

	return EM_walk ;
}

int	ENE_RunMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_run ;

//	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_nom_run ;
	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_cle_run ;
	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_ak_nom_run ;

	return EM_run ;
}

int	ENE_ReadyGunMotion( ACTION *act )
{
//	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_kamae_gun_high ;
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_idle ;
	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_nom_fire ;

	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_m4_nom_fire_p_high ;

	return EM_kamae_gun_high ;
}
int	ENE_ReloadMotion( ACTION *act )
{

	if ( act->bodyp.type & ENE_TYPE_TNG_A)		return EM_tng_p90_nom_fire_reload ;
	if ( act->bodyp.type & ENE_TYPE_SHIELD)		return EM_shl_nom_mkr_reload ;
	if ( act->bodyp.type & ENE_TYPE_SHOTGUN)	return EM_gbs_sps_nom_fire_reload ;
	if ( act->bodyp.type & ENE_TYPE_HITECH)		return EM_htc_m4_nom_fire_reload ;

	return EM_reload ;
}



int	ENE_PunchDamMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_dam_gun ;
	return EM_dam_punch_l ;
}
int	ENE_NokezoriDamMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_dam_arm_l ;
	return EM_dam_nokezori ;
}

int	ENE_DiscoveryMotion( ACTION *act )
{
	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_gbs_shl_nom_find_ply ;
	return EM_find_ply ;
}

int	ENE_NormalStandMotion( ACTION *act )
{
	if ( ENE_PureAttacker( act ) ) return  EM_stand ;

	if ( act->bodyp.pbreak & PBREAK_ARM_R )	return EM_dam_okaji ;

	if ( act->bodyp.type & ENE_TYPE_SHIELD)	return EM_shl_nom_idle ;

	if ( act->bodyp.type & ENE_TYPE_TNG_A)	return EM_tng_p90_nom_idle ;

	if ( act->bodyp.type & ENE_TYPE_HITECH)	return EM_htc_m4_nom_idle ;

	return EM_stand ;
}


/*-------------------------------------------------------------------*/
/*	その場に立つ	*/
void ENE_ActStandStill( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND  ) ;
	AT_SetActStatus( act, ACT_STATUS_STANDSTILL ) ;

	if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE  ) ;
	}

	if ( time == 0 ) {
		int mot ;
		mot = ENE_StandMotion( act ) ;
		ENE_SetActionPBreak( act, 0, mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_STAND ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->CheckPad( act ) ) {
		return ;
	}

	if ( act->dir >= 0 ) {
#if 1
		SetMoveStart( act, time ) ;
#else
		act->ctrl->turn.vy = act->dir ;
		SetMode( act, ENE_ActMove ) ;
#endif
		return ;
	}

//	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
//	act->ctrl->step.vz = 20.0f * _RcosF( 0 ) ;

}

void ENE_ActStandMotion( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND  ) ;

	if ( (act->bodyp.pbreak & PBREAK_LEGS) == PBREAK_LEGS ) {
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE  ) ;
	}

	if ( time == 0 ) {
		int mot ;
		mot = ENE_StandMotion( act ) ;
		ENE_SetActionPBreak( act, 0, mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_STAND ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
			act->act_end = 1 ;
		}
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActMoveStart( act, time )
ACTION	*act ;
int		time ;
{
	float	speed ;
	int		move_s ;

	
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MOVE | ACT_STATUS_MOVE_START ) ;

#if 1
	if ( act->CheckDamage( act ) ) 	return ;
	if ( act->CheckPad( act ) ) 	return ;

	if ( act->dir < 0 ) {
		SetMode( act, ENE_ActStandStill ) ;
		act->ctrl->turn.vz=0 ;
		return ;
	}

	if ( time == 0 ) {
		if ( act->move_s == MoveWalkGun ) {
			move_s = EM_walk_light ;
		} else {
			move_s = ENE_WalkMotion( act ) ;
		}
		ENE_SetActionPBreak( act, 0, move_s, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
	}

	speed = STEP_VMODE(time * 2.0F) ;
	if ( speed > STEP_VMODE(50.0F) ) speed = STEP_VMODE(50.0F) ;

	if (act->ctrl->interp  == ACT_CTRL_INTERP_SLOWLY ) speed /= 3  ;

	act->ctrl->turn.vy  = act->dir ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step.vx = speed * _RsinF( (int)act->ctrl->turn.vy ) ;
	act->ctrl->step.vz = speed * _RcosF( (int)act->ctrl->turn.vy ) ;

	if ( TurnDirCheck( act ) ) {
		SetMode( act, ENE_ActMove ) ;
		act->ctrl->turn.vz=0 ;
		
		return ;
	}

	ENE_Incline( act->ctrl ) ;
	act->ctrl->turn.vz /= 2 ;
#else

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) 	return ;
	if ( act->CheckPad( act ) ) 	return ;
/*
	if ( act->dir < 0 ) {
		SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
*/
	if ( TurnCheck( act ) ) {
		SetMode( act, ENE_ActMove ) ;
		return ;
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		SetMode( act, ENE_ActMove ) ;
		return ;
	}
#endif

}

//#define STAIR_JIK (1)

int	StairDir( HZX_FLR *flr, int dir )
{
	FVECTOR	norm ;
	int flr_dir, dirabs ;
	
    norm.vx = flr->p1.h ;
    norm.vz = flr->p2.h ;
    norm.vy = 0 ;

	flr_dir = _FVecDir2( &norm ) ;
	dirabs = GV_DiffDirAbs( flr_dir, dir ) ;
	
	if ( dirabs > 1536 )	return SIDE_U ;	/* 上り坂 */
	if ( dirabs < 512 )		return SIDE_D ;	/* 下り坂 */

	return SIDE_R ;
}


/*	歩く	*/
void ENE_ActMove( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	float	speed ;
	int		actnum, turn_dir ;

	ctrl = act->ctrl ;

	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MOVE  ) ;

	actnum = ENE_WalkMotion( act ) ;
	speed = 0.0F ;

	/* 方向転換時の体の傾き */
	ENE_Incline( act->ctrl ) ;

	switch ( act->move_s ) {
		case MoveWalk :
#ifdef STAIR_JIK
			if ( act->ctrl->flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
				int side ;
				
				if( ctrl->level[ 0 ] == NULL ) {
					actnum = ENE_WalkMotion( act ) ;
				} else {
					side = StairDir( ctrl->level[ 0 ], ctrl->rot.vy ) ;
					switch( side ) {
						case SIDE_D :
							actnum = EM_stair_walk_down ;
						break ;
						case SIDE_U :
							actnum = EM_stair_walk_up ;
						break ;
						case SIDE_R :
						case SIDE_L :
							actnum = ENE_WalkMotion( act ) ;
						break ;
					}
				}
			} else {
				actnum = ENE_WalkMotion( act ) ;
			}
#else
			actnum = ENE_WalkMotion( act ) ;
#endif
			speed = (int)KR_MotionStepSpeed( act->body->m_ctrl ) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_R ) speed -= STEP_VMODE(12.5f) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_L ) speed -= STEP_VMODE(12.5f) ;
			if ( speed < 10.0f ) speed = 10.0f ;
			turn_dir = act->dir ;
			act->ctrl->turn.vz /= 2 ;
		break ;
		case MoveRun :
#ifdef STAIR_JIK
			if ( act->ctrl->flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
				int side ;
				
				if( ctrl->level[ 0 ] == NULL ) {
					actnum = ENE_RunMotion( act ) ;
				} else {
					side = StairDir( ctrl->level[ 0 ], ctrl->rot.vy ) ;
					switch( side ) {
						case SIDE_D :
							if ( act->bodyp.type & ENE_TYPE_SHIELD) {
								actnum = EM_shl_nom_stair_run_down ;
							} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
								actnum = EM_htc_ak_nom_stair_run_dowm ;
							} else {
								actnum = EM_stair_run_down ;
							}
						break ;
						case SIDE_U :
							actnum = EM_stair_run_up ;
						break ;
						case SIDE_R :
						case SIDE_L :
							actnum = ENE_RunMotion( act ) ;
						break ;
					}
				}
			} else {
				actnum = ENE_RunMotion( act ) ;
			}
#else
			actnum = ENE_RunMotion( act ) ;
#endif
			speed = STEP_VMODE(50.0F) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_R ) speed -= STEP_VMODE(25.0f) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_L ) speed -= STEP_VMODE(25.0f) ;
			turn_dir = act->dir ;
		break ;
		case MoveWalkGun :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_ak_nom_walk_light ;
			} else {
				actnum = EM_walk_light ;
			}
			turn_dir = act->dir ;
			act->ctrl->turn.vz /= 2 ;
		break ;
		case MoveCautionWalk :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_cle_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_TNG_A) {
				actnum = EM_tng_p90_cle_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_walk_atk ;
			} else {
				actnum = EM_cle_walk ;
			}
			turn_dir = act->dir ;
			act->ctrl->turn.vz /= 2 ;
		break ;
		case MoveCautionWalkLow :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_cle_walk ;
			} else {
				actnum = EM_cle_walk_low ;
			}
			turn_dir = act->dir ;
			act->ctrl->turn.vz = 0 ;
		break ;
		case MoveCautionRun :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_cle_run ;
			} else {
				actnum = EM_cle_run ;
			}
			turn_dir = act->dir ;
		break ;
		case MoveSideL :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_cle_slidewalk_l ;
			} else {
				actnum = EM_cle_kaniwalk_l ;
			}
			turn_dir = (act->dir-1024) & 4095 ;
			act->ctrl->turn.vz = 0 ;
		break ;
		case MoveSideR :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_cle_slidewalk_r ;
			} else {
				actnum = EM_cle_kaniwalk_r ;
			}
			turn_dir = (act->dir+1024) & 4095 ;
			act->ctrl->turn.vz = 0 ;
		break ;
		case MoveBack :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_walk_b_atk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_walk_b_atk ;
			} else {
				actnum = EM_walk_b_atk ;
			}
			turn_dir = (act->dir+2048) & 4095 ;
			act->ctrl->turn.vz = 0 ;
		break ;
		case MoveStairRunD :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_stair_run_down ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_ak_nom_stair_run_dowm ;
			} else {
				actnum = EM_stair_run_down ;
			}
			turn_dir = act->dir ;
		break ;
		case MoveStairRunU :
			actnum = EM_stair_run_up ;
			turn_dir = act->dir ;
		break ;
		case MoveStairWalkD :
			actnum = EM_stair_walk_down ;
			turn_dir = act->dir ;
		break ;
		case MoveStairWalkU :
			actnum = EM_stair_walk_up ;
			turn_dir = act->dir ;
		break ;
		case MoveAttackRun :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_run ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_run_atk ;
			} else {
				actnum = EM_run_atk ;
			}
			speed = STEP_VMODE(60.0F) ;
			turn_dir = act->dir ;
		break ;
		case MoveNoriNoriWalk :
			actnum = EM_norinori_walk ;
			turn_dir = act->dir ;
			act->ctrl->turn.vz /= 2 ;
		break ;
		default :
			actnum = ENE_WalkMotion( act ) ;
			speed = STEP_VMODE(50.0F) ;
			turn_dir = act->dir ;
			act->ctrl->turn.vz /= 2 ;
		break ;
	}

//	if ( act->bodyp.type & ENE_TYPE_SHIELD) {
//		actnum = EM_shl_nom_run ;
//	}
	/*******************/

	if ( 
		act->move_s == MoveSideL || 
		act->move_s == MoveSideR ||
		act->move_s == MoveBack
		) {
//printf("[%d] act[%d] turn[%d] rot[%d] \n",act->move_s, act->dir,ctrl->turn.vy, ctrl->rot.vy ) ;
		ENE_SetActionPBreak( act, 0, actnum, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
	} else {
		ENE_SetActionPBreak( act, 0, actnum, 0, MOTION_MASK_FULL, 60, PBREAK_OVER_MOVE ) ;
	}

FootNoise( act ) ;


	if ( act->CheckDamage( act ) ) {
		act->ctrl->turn.vz = 0 ;
		return ;
	}
	if ( act->CheckPad( act ) ) {
		act->ctrl->turn.vz = 0 ;
		return ;
	}

	if ( act->dir < 0 ) {
		SetMode( act, ENE_ActStandStill ) ;
		act->ctrl->turn.vz = 0 ;
		return ;
	}
	ctrl->turn.vy = turn_dir ;

//if ( actnum == EM_walk ) speed = 0 
#if 0
	speed = 0.0F ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
	ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
#else
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
// printf(" step x= %f step z = %f \n",ctrl->step.vx,ctrl->step.vz);
	}
#endif
}
/*


*/


/* ピンポイント移動 */
void ENE_ActPinpointMove( act, time )
ACTION	*act ;
int		time ;
{
	CONTROL	*ctrl ;
	int actnum, speed ;
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MOVE  ) ;

	ctrl = act->ctrl ;

 	speed = (int)KR_MotionStepSpeed( act->body->m_ctrl ) ;
	switch ( act->move_s ) {
		case MoveWalk :
			actnum = ENE_WalkMotion( act ) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_R ) speed /= 2 ;
			if ( act->bodyp.pbreak & PBREAK_LEG_L ) speed /= 2 ;
			ctrl->turn.vz /= 2 ;
		break ;
		case MoveRun :
			actnum = ENE_RunMotion( act ) ;
			if ( act->bodyp.pbreak & PBREAK_LEG_R ) speed /= 2 ;
			if ( act->bodyp.pbreak & PBREAK_LEG_L ) speed /= 2 ;
		break ;
		case MoveWalkGun :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_ak_nom_walk_light ;
			} else {
				actnum = EM_walk_light ;
			}
		break ;
		case MoveCautionWalk :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_cle_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_TNG_A) {
				actnum = EM_tng_p90_cle_walk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_walk_atk ;
			} else {
				actnum = EM_cle_walk ;
			}
		break ;
		case MoveCautionWalkLow :
			actnum = EM_cle_walk_low ;
		break ;
		case MoveCautionRun :
			actnum = EM_cle_run ;
		break ;
		case MoveSideL :
			actnum = EM_cle_kaniwalk_l ;
		break ;
		case MoveSideR :
			actnum = EM_cle_kaniwalk_r ;
		break ;
		case MoveBack :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_walk_b_atk ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_walk_b_atk ;
			} else {
				actnum = EM_walk_b_atk ;
			}
		break ;
		case MoveStairRunD :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_stair_run_down ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_ak_nom_stair_run_dowm ;
			} else {
				actnum = EM_stair_run_down ;
			}
		break ;
		case MoveStairRunU :
			actnum = EM_stair_run_up ;
		break ;
		case MoveStairWalkD :
			actnum = EM_stair_walk_down ;
		break ;
		case MoveStairWalkU :
			actnum = EM_stair_walk_up ;
		break ;
		case MoveAttackRun :
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				actnum = EM_shl_nom_run ;
			} else if ( act->bodyp.type & ENE_TYPE_HITECH) {
				actnum = EM_htc_m4_nom_run_atk ;
			} else {
				actnum = EM_run_atk ;
			}
		break ;
		case MoveNoriNoriWalk :
printf("norinori??\n");
			actnum = EM_norinori_walk ;
		break ;
		default :
			actnum = ENE_WalkMotion( act ) ;
		break ;
	}

	ENE_SetActionPBreak( act, 0, actnum, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_MOVE ) ;

	if ( act->CheckDamage( act ) ) {
		act->ctrl->turn.vz = 0 ;
		act->act_end = 1 ;
		return ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
			act->act_end = 1 ;
		}
		return ;
	}

	{
		int dir, dis ;
		FVECTOR	vec ;

		_sceVu0SubVector(  &vec, &act->target_pos, &ctrl->mov ) ;
		dir = _FVecDir2( &vec ) ;
		dis = (int)_FVecLen2( &vec ) ;

		if( dis < speed ) {	/* 次のフレームで到着予定 */
			speed = dis ;
			AT_SetMode( act, ENE_ActStandStill ) ;
			act->act_end = 1 ;
		}
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( dir ) ;
		ctrl->step.vz = speed * _RcosF( dir ) ;

printf("pinpoint: step x= %f step z = %f \n",ctrl->step.vx,ctrl->step.vz);
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

/* その場で方向転換 */
void ENE_ActTurn( act, time )
ACTION	*act ;
int		time ;
{
	short	diffdir, subdir ;
	
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MOVE | ACT_STATUS_MOVE_START ) ;


	if ( time == 0 ) {
		act->tmp_dir = act->dir ;

		diffdir = GV_DiffDirS( act->ctrl->rot.vy, act->dir ) ;
		if ( diffdir > 0 ){
			if ( diffdir < 1024+64 ) {
				act->keep_mot = EM_turn_l_t ;
			} else {
				subdir = diffdir - (1024+64) ;
				act->ctrl->turn.vy = act->ctrl->rot.vy + subdir ;
				act->keep_mot = EM_turn_l_t ;
			}
		} else {
			if ( diffdir > -1024-64 ) {
				act->keep_mot = EM_turn_r_t ;
			} else {
#if 1
				subdir = diffdir + (1024+64) ;
				act->ctrl->turn.vy = act->ctrl->rot.vy + subdir ;
				act->keep_mot = EM_turn_r_t ;
#else
				act->keep_mot = EM_turn_b_t ;
#endif
			}
		}
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}
//printf("move start tmpdir[%d] dir[%d]\n",act->tmp_dir,act->ctrl->turn.vy ) ;

	if ( act->CheckDamage( act ) ) 	return ;
	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	if ( act->dir < 0 ) {
		SetMode( act, ENE_ActStandStill ) ;
		act->ctrl->turn.vz=0 ;
		return ;
	}

	if ( TurnCheck( act ) ) {
		act->act_end = 1 ;

act->ctrl->turn.vy = act->tmp_dir ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
act->ctrl->turn.vy = act->tmp_dir ;
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
}


/* キープモーション */
void ENE_ActKeepMotion( act, time )
ACTION	*act ;
int		time ;
{
	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		if ( (act->keep_mot == EM_cle_stand02) ) {
			if ( act->bodyp.type & ENE_TYPE_SHIELD) {
				ENE_SetMarActionPBreak( act, 0, EM_shl_nom_idle, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
			} else if (act->bodyp.type & ENE_TYPE_HITECH) {
				ENE_SetMarActionPBreak( act, 0, EM_htc_ak_cle_signe_idle, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
			} else {
				ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
					MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar  ) ;
			}
		} else {
printf("keep motion keep mar[%d]\n",act->keep_mar  ) ;
			ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
				MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar  ) ;
		}
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

}

/* １タイムモーション */
void ENE_ActOneTimeMotion( act, time )
ACTION	*act ;
int		time ;
{
	if ( time == 0 ) {
		ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
			MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
	}
	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}


/* 無線コール */
#define RIGHT_RADIO	(1)
void ENE_ActSetRadio( ACTION *act, int time )
{
	int b_flag, sw ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		b_flag = act->bodyp.pbreak & PBREAK_ARMS ;
		switch( b_flag ) {
			case PBREAK_ARM_R :
				act->keep_mot = EM_call_team_gun_l_1_start_fast ;
				break ;
			case PBREAK_ARM_L :
				act->keep_mot = EM_arml_cle_call_team_r_1start ;
				break ;
			default :
				act->keep_mot = EM_call_team_gun_l_1_start_fast ;
				break ;
		}
		ENE_SetActionPBreak( act, 0, act->keep_mot,0,MOTION_MASK_FULL,ACT_INTERP_DEF, PBREAK_OVER_BASE  ) ;

		if ( GM_GameStatus & GM_STATUS_DETECT ) {
//体験版スピード			MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*4.0f ) ;
			MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE*2.0f ) ;
		}
	}

	sw = (act->c_motion_num[0] == EM_call_team_gun_l_1_start_fast) ? SW_FLAG_SWITCH1 : SW_FLAG_SWITCH2 ;

	if ( act->c_motion_num[0] != EM_call_team_gun_l_1_start_fast )	{
		if ( time > COUNT_VMODE(8) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		UNSET_FLAG( act->sw->radio, sw ) ;
		return ;
	}

#ifdef RADIO_PAD_CHECK_NEW
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
				act->sw->mouth = MOUTH_NORMAL ;
			}
			return ;
		} else {
			AT_SetMode( act, ENE_ActStandStill ) ;
			UNSET_FLAG( act->sw->radio, sw ) ;
			act->sw->mouth = MOUTH_NORMAL ;
			return ;
		}
	}
#else
	if ( act->pad != act->keep_pad ) {
		/* キープモーションではない */
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
			}
			return ;
		}
	}
#endif

	if ( GM_GameStatus & GM_STATUS_DETECT ) {
		if ( time == COUNT_VMODE(28/4) ) SET_FLAG( act->sw->radio, sw ) ;
	} else {
		if ( time == COUNT_VMODE(28) ) SET_FLAG( act->sw->radio, sw ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 3 ;	/* モーションあと2個、続き有り */
		AT_SetMode( act, ENE_ActCallRadio ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}



void ENE_ActCallRadio( ACTION *act, int time )
{
	int b_flag, sw ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		b_flag = act->bodyp.pbreak & PBREAK_ARMS ;
		switch( b_flag ) {
			case PBREAK_ARM_R :
				act->keep_mot = EM_call_team_gun_l_2_talk ;
				break ;
			case PBREAK_ARM_L :
				act->keep_mot = EM_arml_cle_call_team_r_2talk ;
				break ;
			default :
				act->keep_mot = EM_call_team_gun_l_2_talk ;
				break ;
		}

		ENE_SetActionPBreak( act, 0, act->keep_mot,0,MOTION_MASK_FULL,ACT_INTERP_DEF, PBREAK_OVER_BASE  ) ;
	}

	sw = (act->c_motion_num[0]==EM_call_team_gun_l_2_talk) ? SW_FLAG_SWITCH1 : SW_FLAG_SWITCH2 ;
	if ( act->c_motion_num[0] != EM_call_team_gun_l_2_talk )	{
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		UNSET_FLAG( act->sw->radio, sw ) ;
		act->sw->mouth = MOUTH_NORMAL ;
		return ;
	}

#ifdef RADIO_PAD_CHECK_NEW
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
				act->sw->mouth = MOUTH_NORMAL ;
			}
			return ;
		} else {
			AT_SetMode( act, ENE_ActStandStill ) ;
			UNSET_FLAG( act->sw->radio, sw ) ;
			act->sw->mouth = MOUTH_NORMAL ;
			return ;
		}
	}
#else
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
				act->sw->mouth = MOUTH_NORMAL ;
			}
			return ;
		}
	}
#endif

	if ( time > MAX_VOL_TIME ) {	/* 最大10秒継続 thinkでの制御はtimeで行う */
		act->act_end = 2 ;
		AT_SetMode( act, ENE_ActReturnRadio ) ;
		act->sw->mouth = MOUTH_NORMAL ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActRadioBreak( act, time )
ACTION	*act ;
int		time ;
{
	int b_flag, sw ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		b_flag = act->bodyp.pbreak & PBREAK_ARMS ;
		switch( b_flag ) {
			case PBREAK_ARM_R :
				act->keep_mot = EM_call_break_l ;
				break ;
			case PBREAK_ARM_L :
				act->keep_mot = EM_arml_cle_call_break_r ;
				break ;
			default :
				act->keep_mot = EM_call_break_l ;
				break ;
		}

		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	sw = (act->c_motion_num[0] == EM_call_break_l) ? SW_FLAG_SWITCH1 : SW_FLAG_SWITCH2 ;
	if ( act->c_motion_num[0] != EM_call_break_l )	{
		AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		UNSET_FLAG( act->sw->radio, sw ) ;
		return ;
	}

#ifdef RADIO_PAD_CHECK_NEW
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
				act->sw->mouth = MOUTH_NORMAL ;
			}
			return ;
		} else {
			AT_SetMode( act, ENE_ActStandStill ) ;
			UNSET_FLAG( act->sw->radio, sw ) ;
			act->sw->mouth = MOUTH_NORMAL ;
			return ;
		}
	}
#else
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			UNSET_FLAG( act->sw->radio, sw ) ;
			return ;
		}
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 2 ;
		AT_SetMode( act, ENE_ActReturnRadio ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActReturnRadio( ACTION *act, int time )
{
	int b_flag, sw ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		b_flag = act->bodyp.pbreak & PBREAK_ARMS ;
		switch( b_flag ) {
			case PBREAK_ARM_R :
				act->keep_mot = EM_call_team_gun_l_3_end ;
				break ;
			case PBREAK_ARM_L :
				act->keep_mot = EM_arml_cle_call_team_r_3end ;
				break ;
			default :
				act->keep_mot = EM_call_team_gun_l_3_end ;
				break ;
		}

		ENE_SetActionPBreak( act, 0, act->keep_mot, 0,MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	sw = (act->c_motion_num[0] == EM_call_team_gun_l_3_end) ? SW_FLAG_SWITCH1 : SW_FLAG_SWITCH2 ;
	if ( act->c_motion_num[0] != EM_call_team_gun_l_3_end )	{
		if ( time < COUNT_VMODE(90) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
	}
	
	if ( act->CheckDamage( act ) ) {
		UNSET_FLAG( act->sw->radio, sw ) ;
		return ;
	}

#ifdef RADIO_PAD_CHECK_NEW
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->action != ENE_ActRadioBreak ) {
				UNSET_FLAG( act->sw->radio, sw ) ;
				act->sw->mouth = MOUTH_NORMAL ;
			}
			return ;
		} else {
			AT_SetMode( act, ENE_ActStandStill ) ;
			UNSET_FLAG( act->sw->radio, sw ) ;
			act->sw->mouth = MOUTH_NORMAL ;
			return ;
		}
	}
#else
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			UNSET_FLAG( act->sw->radio, sw ) ;
			return ;
		}
	}
#endif

	if ( time == COUNT_VMODE(62) ) UNSET_FLAG ( act->sw->radio, sw ) ;
//printf("return radio time[%d]\n",time ) ;
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActRadioCall( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
//	if ( time > 15 && time < 250 ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}

	if ( act->CheckDamage( act ) ) {
		UNSET_FLAG( act->sw->radio, SW_FLAG_SWITCH1 ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			UNSET_FLAG( act->sw->radio, SW_FLAG_SWITCH1 ) ;
			return ;
		}
	}

	if ( act->keep_pad == EM_cle_call_team ) {
		if ( time == COUNT_VMODE(40) ) SET_FLAG( act->sw->radio, SW_FLAG_SWITCH1 ) ;
		if ( time == COUNT_VMODE(262) ) UNSET_FLAG ( act->sw->radio, SW_FLAG_SWITCH1 ) ;
	} else {
		if ( time == COUNT_VMODE(47) ) SET_FLAG( act->sw->radio, SW_FLAG_SWITCH1 ) ;
		if ( time == COUNT_VMODE(168) ) UNSET_FLAG( act->sw->radio, SW_FLAG_SWITCH1 ) ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActNSight_ON( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( time == COUNT_VMODE(15) ) {
		act->sw->n_sight = 2 ;/* 暗視ゴーグルON */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActNSight_OFF( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
	if ( time == COUNT_VMODE(33) ) {
		act->sw->n_sight = 1 ;/* 暗視ゴーグルOFF */
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}


void ENE_ActMedication( act, time )
ACTION	*act ;
int		time ;
{
	int b_flag ;
	
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_MEDICATION) ;
	
	if ( time == 0 ) {
		if ( (b_flag = act->bodyp.pbreak & PBREAK_ARMS) ) {
			switch( b_flag ) {
				case PBREAK_ARM_R :
					act->keep_mot = EM_legl_morph_legl ;
					break ;
				case PBREAK_ARM_L :
					act->keep_mot = EM_legr_morph_legr ;
					break ;
				default :
					act->keep_mot = EM_legl_morph_legl ;
					break ;
			}
		}
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	switch ( act->keep_mot ) {
		case EM_legl_morph_legl :
			if ( time < COUNT_VMODE(312) )  AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
			if ( time == COUNT_VMODE(160) )  ENE_ClearPDamage( act ) ;
		case EM_legr_morph_legr :
			if ( time < COUNT_VMODE(376) )  AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
			if ( time == COUNT_VMODE(160) )  ENE_ClearPDamage( act ) ;
			break ;
	}


	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}


void ENE_ActReadyGun( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
//		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
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

}

void ENE_ActNearAtk( act, time )
ACTION	*act ;
int		time ;
{
	static FVECTOR size = { 500.0F,500.0F,500.0F } ;

//	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
//		AT_SetAction( act, 0, EM_ak_attack_near_kick_l, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		/*気合声*/
//		GM_SeSetMode( SD_V_GBSFUN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		GM_SeSetMode( SD_V_GBSFUN01+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}


	switch( act->keep_mot ) {
#if 0
		case EM_near_shoulder :	/* タックル */
//			if( time > 20 && time < 28 ) {
			if( time > 1 && time < COUNT_VMODE(4) ) {
				ENE_SetOffenseTarget( act, EM_near_shoulder ) ;
			}
			if ( time > COUNT_VMODE(4) && time < COUNT_VMODE(76) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		break ;
		case EM_near_punch :	/* パンチ */
//			if( time > 20 && time < 25 ) {
			if( time > 1 && time < COUNT_VMODE(4) ) {
				ENE_SetOffenseTarget( act, EM_near_punch ) ;
			}
			if ( time > COUNT_VMODE(4) && time < COUNT_VMODE(56) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		break ;
#endif
		case EM_attack_near :	/* キック */
//			if( time > 20 && time < 25 ) {
			if( time > 1 && time < COUNT_VMODE(4) ) {
				ENE_SetOffenseTarget( act, EM_ak_attack_near_kick_l ) ;
			}
		break ;
		/*新攻撃 */
		case EM_ak_attack_near_kick_r :
		case EM_ak_attack_near_kick_l :
			if( time > 1 && time < COUNT_VMODE(6) ) {
				ENE_SetOffenseTarget( act, act->keep_mot ) ;
			}
		break ;
		case EM_ak_attack_near_punch_r :
		case EM_ak_attack_near_punch_l :
			if( time > 1 && time < COUNT_VMODE(6) ) {
				ENE_SetOffenseTarget( act, act->keep_mot ) ;
			}
			/*1で銃放し 49で復帰*/
			if ( time > 1 && time < COUNT_VMODE(49) ) AT_SetActStatus( act, ACT_STATUS_GUN_FREE ) ;
		break ;
		case EM_nom_stomp :
		case EM_shl_nom_stomp :
			if( time == COUNT_VMODE(22) ) {
//				ENE_SetOffenseTarget( act, act->keep_mot ) ;
				ENE_SetOffenseTarget2(act,WP_PUNCH,HUMAN21_HIDARI_TSUMASAKI,&size );
			}
		break ;

	}
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActKeri( act, time )
ACTION	*act ;
int		time ;
{
#if 1
	act->keep_mot = (act->bodyp.type&ENE_TYPE_HITECH)?EM_htc_m4_nom_attack_near:EM_ak_attack_near_kick_r ;
	ENE_ActNearAtk( act, time ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_SetOffenseTarget( act, 0 ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
#endif
}

void ENE_ActMgunShot( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;

	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		ENE_ShootBullet( act, ENE_BULLET_NORMAL , &act->aim_pos ) ;
		AT_SetActStatus( act, ACT_STATUS_SHOT_PIKU ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

#if 1	//11.16
	if ( time > COUNT_VMODE(4) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
#else
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	}
	if ( GM_CheckObject_PlayEnd( act->body, 0 ) ) {
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
#endif

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActReload( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->magg = 1 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->magg = 1 ;
			return ;
		}
	}

	if ( time == COUNT_VMODE(19) ) act->sw->magg = 4 ;
	if ( time == COUNT_VMODE(66) ) act->sw->magg = 2 ;
	if ( time == COUNT_VMODE(112) ) act->sw->magg = 1 ;


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->sw->magg = 1 ;
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

#if 1
static SVECTOR	glass_rot = {0,0,0,0} ;
static FVECTOR	glass_sft = {0.0f,0.0f,0.0f,0.0f} ;
#else
static SVECTOR	glass_rot = {-440,-120,635} ;
static FVECTOR	glass_sft = {-80.0f,-60.0f,-80.0f} ;
#endif
void ENE_ActSetGlass( act, time )
ACTION	*act ;
int		time ;
{
	extern void *NewConnectObject( OBJECT *, int, FVECTOR *, SVECTOR *, int ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		act->tmp_item = NULL ;
	}

	if ( time == COUNT_VMODE(18) ) {
	    GV_SetActorChild( act->w, 
	    	( act->tmp_item = NewConnectObject( act->body, HUMAN21_HIDARI_TE, &glass_sft, &glass_rot, GV_StrCode("sougan") ))) ;
//	    	(act->tmp_item = NewConnectObject( act->body, HUMAN21_HIDARI_TE, &DG_ZeroVector, E_GLASS_NAME ) ) ) ;
	}

	if ( act->CheckDamage( act ) ) {
		if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActGlass ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActGlass( act, time )
ACTION	*act ;
int		time ;
{
	extern	void		*NewConnectObject( OBJECT *, int, FVECTOR *, SVECTOR *, int ) ;

	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
		if ( act->tmp_item == NULL ){
		    GV_SetActorChild( act->w, (act->tmp_item = NewConnectObject( 
		    	act->body, HUMAN21_HIDARI_TE, &glass_sft, &glass_rot, GV_StrCode("sougan") ))) ;
		}
	}

	if ( act->CheckDamage( act ) ) {
		if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		if ( act->tmp_item )	KR_DestroyActor( &act->tmp_item ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;

}

#define FINGER_SIGN_GU	SW_FLAG_SWITCH1		//6fl
#define FINGER_SIGN_PA	SW_FLAG_SWITCH2		//18fl
#define FINGER_SIGN_HITO	SW_FLAG_SWITCH3	//6fl
#define FINGER_SIGN_OYA	SW_FLAG_SWITCH4		//6fl

void ENE_ActCleSign( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	/* フィンガーサイン操作 */
	switch( act->keep_mot ) {
		case EM_cle_signe_go :
		case EM_htc_ak_cle_signe_go :
			if( time == COUNT_VMODE(5) ) act->sw->finger = FINGER_SIGN_PA ;
			if( time == COUNT_VMODE(35) ) act->sw->finger = 0 ;
			break ;
		case EM_cle_signe_clear :
			if( time == COUNT_VMODE(5) ) act->sw->finger = FINGER_SIGN_GU ;
			if( time == COUNT_VMODE(37) ) act->sw->finger = FINGER_SIGN_OYA ;
			if( time == COUNT_VMODE(80) ) act->sw->finger = 0 ;
			break ;
		case EM_cle_signe_move :
			if( time == COUNT_VMODE(20) ) act->sw->finger = FINGER_SIGN_HITO ;
			if( time == COUNT_VMODE(74) ) act->sw->finger = 0 ;
			break ;
		case EM_cle_signe_stop :
			if( time == COUNT_VMODE(18) ) act->sw->finger = FINGER_SIGN_GU ;
			if( time == COUNT_VMODE(86) ) act->sw->finger = 0 ;
			break ;
	}


	if ( act->CheckDamage( act ) ) {
		act->sw->finger = 0 ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			act->sw->finger = 0 ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		act->sw->finger = 0 ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}


void ENE_ActLockerOpen( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetActionPBreak( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActUnreal( act, time )
ACTION	*act ;
int		time ;
{
	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( act->pad != act->keep_pad ) {
//printf("unreaaaaaaaaaaaaaaaaal end pad[%d] keep[%d]\n",act->pad, act->keep_pad ) ;
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		return ;
	}

	AT_SetActStatus( act, ACT_STATUS_UNREAL ) ;
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}

void ENE_ActZzz( ACTION	*act, int time )
{
	int	mt_time ;

	/* 注！！キープモーションは移動しない */
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
	act->ctrl->step = DG_ZeroVector ;

	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( time == 0 ) {
		ENE_SetMarActionPBreak( act, 0, act->keep_mot, 0, 
			MOTION_MASK_FULL, ACT_INTERP_DEF, PBREAK_OVER_BASE, act->keep_mar ) ;
		ENE_SetHeadMark( act, 0, HEADMARK_ZZZ ) ;
	}

	mt_time = act->body->m_ctrl->mt3_ctrl[0].time ;
	if ( mt_time == COUNT_VMODE(30) ) {
		act->sw->eye_anim = EYE_TUBIRI ;
	}
	if ( mt_time == COUNT_VMODE(34) ) {
		act->sw->eye_anim = EYE_NORMAL ;
	}
	if ( mt_time == COUNT_VMODE(38) ) {
		act->sw->eye_anim = EYE_TUBIRI ;
	}
	if ( mt_time == COUNT_VMODE(42) ) {
		act->sw->eye_anim = EYE_NORMAL ;
	}
	if ( mt_time == COUNT_VMODE(60) ) {
		act->sw->eye_anim = EYE_TUBIRI ;
	}
	if ( mt_time > COUNT_VMODE(60) && mt_time < COUNT_VMODE(487) ) {
		AT_SetActStatus( act, ACT_STATUS_EYE_CLOSE|ACT_STATUS_STAND_ZZZ ) ;
	}

	if ( mt_time==COUNT_VMODE(70) || mt_time==COUNT_VMODE(226) || mt_time == COUNT_VMODE(393) ) {
		int snore ;

		snore = SD_V_GBSIBK01+(act->name_id->id%4) ;
		if ( *(act->ene_status) & ENE_STATUS_BIG_SNORE ) {
			snore = SD_V_GBSIBKS1 ;

			if ( Ply_GetPlayerWeapon() != WP_Mic ) {
				GM_SeSetMode( snore, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
			} else {
            int pan;
            float bp_angle;
            pan = GM_SeGetPan( &act->ctrl->mov, GM_SEMODE_BOMB, &bp_angle);
				GM_SeSet3D( pan, GM_MAX_VOL, snore, bp_angle ) ;
			}
		} else {
			GM_SeSetMode( snore, &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
		}
	}

	if ( mt_time == COUNT_VMODE(487) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
	}

	if ( act->CheckDamage( act ) ) {
		act->sw->eye_anim = EYE_MABATAKI ;
		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		return ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( !act->CheckPad( act ) ) {
			AT_SetMode( act, ENE_ActStandStill ) ;
		}
		ENE_SetHeadMark( act, 0, HEADMARK_CLEAR ) ;
		act->sw->eye_anim = EYE_MABATAKI ;
		return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		/* モーション終了待ちにも対応できるように
			モーションエンドにフラグをたる*/
		act->act_end = 1 ;
	}

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
}
