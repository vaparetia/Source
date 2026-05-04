//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	action.c
	敵兵のアクションコントロール

	1997/07/07 Y.Korekado
	$Id: action.c,v 1.1.1.3 2002/11/19 11:43:57 Yoshizawa1 Exp $
	
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
#include	"action.h"

#include	"../conv/korekado.x"

#include "BP_TrophyLogicMGS2.h"
#include "BP_TrophySystem.h"
#include "BP_Misc.h"

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
void AT_Action( act )
ACTION	*act ;
{
	int	time ;

	act->act_end = 0 ;
	act->old_status = act->status ;
	act->status = 0 ;
	act->status_status = 0 ;

	time = act->time ++ ;
#if 1
	if ( act->action == NULL ) return ;
	( *act->action )( act, time ) ;
#else
	if ( act->action == NULL ) act->action = AT_ActStandStill ;
	( *act->action )( act, time ) ;
#endif
}

/*-------------------------------------------------------------------*/
static	int	__ErrMessage( act )
ACTION	*act ;
{
	return 0 ;
}

void AT_InitAction( act, ctrl, body )
ACTION	*act ;
CONTROL	*ctrl ;
OBJECT	*body ;
{
	int	i ;
	
	act->ctrl = ctrl ;
	act->body = body ;
	act->dir = -1 ;
	act->motion_num = 0 ;
	act->c_motion_num[0] = 0 ;
	act->c_motion_num[1] = 0 ;
	act->c_motion_num[2] = 0 ;
	act->act_end = 0 ;
	act->status = 0 ;
	act->status_status = 0 ;
	act->adj_flag_old = 0 ;
	act->time = 0 ;
	act->tmp_time = 0 ;
	act->adj_piku_time = 0 ;
   act->adj_piku_value = 0 ;

	act->move_s = 0 ;
	act->down_s = 0 ;
	
	act->CheckDamage = __ErrMessage ;
	act->CheckPad = __ErrMessage ;
	act->pad = 0 ;

	act->h_mark  = NULL ;
	act->h_mark2  = NULL ;
	act->headmark2  = ACT_HEADMARK2_NONE ;

	act->bandage_parts = 0 ;
	act->head_blood = NULL ;

	act->se_time = NULL ;				/* ＳＥタイムバッファ */
	act->se_num = NULL ;				/* ＳＥ番号バッファ */
	act->se_next = 0 ;				/* 次のＳＥバッファ番号 */

	act->motion = NULL ;
	act->motion_table = NULL ;
	act->action = NULL ;
	act->adj_rot = DG_ZeroSVector ;

	/*重野追加 モーションのstep値を加工する*/
	act->mot_speed_correct = 0.0F;	/*モーション移動量を加工*/
	act->mot_dir_correct = -1;		/*モーション移動方向を加工*/

	for(i=0; i<MAX_DURABLE_AREA; i++ ) {
		act->bodyp.durable[i] = 0 ;
		act->bodyp.max_durable[i] = 0 ;
    }

	AT_SetHeartBeat( act, ENE_HEART_BEAT_NORMAL ) ;
}

void	AT_SetType( act, type )
ACTION	*act ;
int		type ;
{
	act->bodyp.type |= type ;
}

void	AT_SetBodyParam( bodyp, life, faint, blood )
BODYPARAM	*bodyp ;
short		life ;
short	 	faint ;
short	 	blood ;
{
	bodyp->life = bodyp->m_life = life ;
	bodyp->faint = bodyp->m_faint = faint ;
	bodyp->blood = bodyp->m_blood = blood ;
   if ( BP_IsPAL() == TRUE )
   	bodyp->anesthesia = bodyp->m_anesthesia = blood*5/12 ;	/* 血液量の半分 */
   else
	   bodyp->anesthesia = bodyp->m_anesthesia = blood/2 ;	/* 血液量の半分 */

	bodyp->stand = 0 ;
	bodyp->pbreak = 0 ;
	bodyp->loss_blood = 0 ;
	bodyp->faint_time = 0 ;
	bodyp->dam_level_num[0]=0 ;
	bodyp->dam_level_num[1]=0 ;
	bodyp->dam_level_num[2]=0 ;
	bodyp->dam_level_num[3]=0 ;
	bodyp->ane_level_num[0]=0 ;
	bodyp->ane_level_num[1]=0 ;
	bodyp->ane_level_num[2]=0 ;
	bodyp->ane_level_num[3]=0 ;
}

void	AT_SetDurable( bodyp, lv0, lv1, lv2, lv3 )
BODYPARAM	*bodyp ;
int			lv0, lv1, lv2, lv3 ;
{
	bodyp->max_durable[0] = bodyp->durable[0] = lv0 ;
	bodyp->max_durable[1] = bodyp->durable[1] = lv1 ;
	bodyp->max_durable[2] = bodyp->durable[2] = lv2 ;
	bodyp->max_durable[3] = bodyp->durable[3] = lv3 ;
}

void AT_SetActionMotion( act, motion, motion_table )
ACTION	*act ;
int		*motion ;
int		*motion_table ;
{
//	act->motion = motion ;
	act->motion_table = motion_table ;
}

extern void ActFaintStandLocker( ACTION *, int ) ;
extern void ActCorpStandLocker( ACTION *, int ) ;
extern void ActCorpsStandLocker( ACTION *, int ) ;
extern void ActHangDie( ACTION *, int ) ;
extern void endamact_ActDown( ACTION *, int ) ;

void AT_SetMode( act, action )
ACTION			*act ;
ACTIONMODE		action ;
{
	act->keep_mar = act->name_id->motion ;	/* base モーションセット */
	SetMode( act, action ) ;

   //BP - trophy logic hooks.
   if( action==ActFaintStandLocker || action==ActCorpStandLocker || action==ActCorpsStandLocker )
   {
      //These states are reused b/t stuffing an enemy in a locker and in a bathroom stall,
      //but there's a separate motion for the bathroom stall case (there are several for the locker case).
      if( act->c_motion_num[0] != 222 ) //EM_dead_carry_put_toilet
      {
         BP_TrophySystem_UnlockTrophy( kTRP_PutEnemyInLocker );
      }
   }
   else if( action == ActHangDie )
   {
      bp_trophy_broke_enemy_neck();
   }
   else if( action == endamact_ActDown )
   {
      if ( act->bodyp.anesthesia < 0 || act->bodyp.faint <= 0 ) {
         if( act->bodyp.last_weapon & WP_PLAYER || act->bodyp.last_weapon & WP_KICK )
         {
            if( act->bodyp.last_weapon & ( WP_PUNCH | WP_PUNCHR | WP_PUNCHL | WP_KICK | WP_THROW | WP_HANG | WP_TUMBLE | WP_BODY | WP_ONCORPSE ) )
            {
               bp_trophy_ko_enemy();
            }
         }
      }
   }
}

void AT_SetModeFromPad( act, action, keep_mot, keep_pad )
ACTION			*act ;
ACTIONMODE		action ;
int				keep_mot ;
int				keep_pad ;
{
	act->keep_mot = keep_mot ;
	act->keep_pad = keep_pad ;
	act->keep_mar = act->name_id->motion ;	/* base モーションセット */
	SetMode( act, action ) ;
}

void AT_SetModeFromPadMar( act, action, keep_mot, keep_pad, keep_mar )
ACTION			*act ;
ACTIONMODE		action ;
int				keep_mot ;
int				keep_pad ;
int				keep_mar ;
{
	act->keep_mot = keep_mot ;
	act->keep_pad = keep_pad ;
	act->keep_mar = keep_mar ;
	SetMode( act, action ) ;
}

#if 0 /* XBOXだと、u_long64がきちんと取れないため  yano 2002.03.11 */
void AT_SetMarAction( act, n_layer, data, m_time, mask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		mar ;
#else
void AT_SetMarAction( ACTION *act, int n_layer, int data, int m_time, u_long64 mask, int interp, int mar )
#endif
{
	SetMarAction( act, n_layer, data, m_time, mask, interp, mar ) ;
	
	n_layer++ ;
	for ( ;n_layer<=LAYER_OVER2; n_layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, n_layer ) ;
		act->c_motion_num[n_layer] = -1 ;
//		act->body->m_ctrl->mt3_ctrl[ n_layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
void AT_SetAction( act, n_layer, data, m_time, mask, interp )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
#else
void AT_SetAction( ACTION *act, int n_layer, int data, int m_time, u_long64 mask, int interp )
#endif
{
	SetAction( act, n_layer, data, m_time, mask, interp ) ;
	
	n_layer++ ;
	for ( ;n_layer<=LAYER_OVER2; n_layer ++ ) {
		act->c_motion_num[n_layer] = -1 ;
		MT_ResetMotionData( act->body->m_ctrl, n_layer ) ;
//		act->body->m_ctrl->mt3_ctrl[ n_layer ].flag &= ~MT3_ACTIVE ;
	}
}

#if 0 /* yano 2002.03.11 */
void AT_ReSetMarAction( act, n_layer, data, m_time, mask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		mar ;
#else
void AT_ReSetMarAction( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp, int mar )
#endif
{
	ReSetMarAction( act, n_layer, data, m_time, mask, interp, mar ) ;

	n_layer++ ;
	for ( ;n_layer<=LAYER_OVER2; n_layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, n_layer ) ;
		act->c_motion_num[n_layer] = -1 ;
	}
}

#if 0 /* yano 2002.03.11 */
void AT_ReSetAction( act, n_layer, data, m_time, mask, interp )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
#else
void AT_ReSetAction( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp )
#endif
{
	ReSetAction( act, n_layer, data, m_time, mask, interp ) ;

	n_layer++ ;
	for ( ;n_layer<=LAYER_OVER2; n_layer ++ ) {
		MT_ResetMotionData( act->body->m_ctrl, n_layer ) ;
		act->c_motion_num[n_layer] = -1 ;
	}
}

void	AT_SetTargetClass( act, flag )
ACTION	*act ;
int		flag ;
{
	act->bodyp.deftrg.class |= flag ;
}

void	AT_UnSetTargetClass( act, flag )
ACTION	*act ;
int		flag ;
{
	act->bodyp.deftrg.class &= ~flag ;
}

void	AT_SetAllChildTargetClass( act, flag )
ACTION	*act ;
int		flag ;
{
	TARGET *t ;
	TARGET_PARTS *tp ;
	int i, n ;

	tp = act->bodyp.deftrg.parts ;

	while ( tp != NULL ) {
		n = tp->n_parts ;
		t = (TARGET *)tp->parts ;
		for( i=0; i<n; i++ ) {
			t[i].class |= flag ;
		}
		tp = tp->next ;
	}
}

void	AT_UnSetAllChildTargetClass( act, flag )
ACTION	*act ;
int		flag ;
{
	TARGET *t ;
	TARGET_PARTS *tp ;
	int i, n ;

	tp = act->bodyp.deftrg.parts ;

	while ( tp != NULL ) {
		n = tp->n_parts ;
		t = (TARGET *)tp->parts ;
		for( i=0; i<n; i++ ) {
			t[i].class &= ~flag ;
		}
		tp = tp->next ;
	}
}

#if 0 /* yano 2002.03.11 */
void	AT_SetActStatus( act, flag )
ACTION	*act ;
long64	flag ;
#else
void	AT_SetActStatus( ACTION	*act, long64 flag )
#endif
{
	act->status |= flag ;
}

void	AT_SetActStSt( act, flag )
ACTION	*act ;
int	flag ;
{
	act->status_status |= flag ;
}

void	AT_SetHeartBeat( ACTION *act, int beat )
{
	act->bodyp.heart_beat = beat + KR_RandS( 16 )  ;
}

