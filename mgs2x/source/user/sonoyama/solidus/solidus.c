//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   solidus.c
   ソリダス

   2001/03/21	M.Sonoyama
   $Id: solidus.c,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"rand.h"
#include	"vertex_animation.h"
#include	"sol_common.h"
#include	"solidus.h"
#include	"snakearm_mot.h"
#include	"solidus.x"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

/*----------------------------------------------------------------*/

/* グローバル */
int					SOL_SolStatus ;
int					SOL_GameLevel ;
CONTROL				*SOL_SolControl ;
extern	Work		*SOL_Work ;

/* ローカル */
static	u_char		BladeDamVibH[] = { 127, 4, 0, 0 } ;
static	u_char		BladeDamVibL[] = { 255, 1, 255, 2, 0, 0 } ;

static	int		SlashSE[] = {
	SD_V_SOLATK01, SD_V_SOLATK02, SD_V_SOLATK03, SD_V_SOLATK04
} ;

static	int		KiaiSE[] = {
	SD_V_SOLATK05, SD_V_SOLATK06, SD_V_SOLATK07, SD_V_SOLATK08
} ;

static	int		ReadySE[] = {
	SD_V_SOLKIA11, SD_V_SOLKIA12, SD_V_SOLKIA13, SD_V_SOLKIA14, SD_V_SOLKIA15
} ;

static	int		ChouhatsuSE[] = {
	SD_V_SOLCHO01, SD_V_SOLCHO02, SD_V_SOLCHO03, SD_V_SOLCHO04
} ;

static	int		KasokuSE[] = {
	SD_V_SOLSPE01, SD_V_SOLSPE02, SD_V_SOLSPE03
} ;

static	int		DamageSE[] = {
	SD_V_SOLDMG11, SD_V_SOLDMG12, SD_V_SOLDMG13, SD_V_SOLDMG14
} ;

static	int		KawashiSE[] = {
	SD_V_SOLWHY01, SD_V_SOLWHY02, SD_V_SOLWHY03
} ;

static	int		SeTime ;
static	int		LastPlayerDamageSeTime ;

/*----------------------------------------------------------------*/

#include	"util_sol.c"
#include	"think_sol.c"
#include	"event_sol.c"
#include	"normal_sol.c"
#include	"damage_sol.c"
#include	"attack_sol.c"
#include	"careful_sol.c"
#include	"init_sol.c"

#include	"dbg_menu_sol.c"

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	inline	void	Action( Work *work )
{
	void		( *action )( Work *, int ) ;
	int			time ;

	/* プレイヤーとの位置関係 */
	UpdateDiffPlayer( work ) ;

	/* 間合いカウンター */
	if ( work->len_diff_player < MAAI_LEN ) {
		if ( work->maai_count < 0 ) work->maai_count = 0 ;
		work->maai_count ++ ;
	} else {
		if ( work->maai_count > 0 ) work->maai_count = 0 ;
		work->maai_count -- ;
	}

	/* フラグリセット */
	ResetFlag( work, SOL_FLAG_RESET_ALL ) ;
	ResetStatus( work, SOL_STATE_RESET_ALL ) ;
	ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_END | SOL_CS_SNAKEARM_LIFTUP_FLING |
				    SOL_CS_SNAKEARM_LIFTUP_FLING_GROUND |
				    SOL_CS_SNAKEARM_LIFTUP_CANCEL_ENABLE ) ;
	work->kwt_trg = TRIG_YET ;
	work->mst_trg = TRIG_YET ;
	SOL_SnakeArmSetFlowFlag( 0 ) ;
	/**/
	work->energy_mode = 0;

	/* モーションカウンタ */
	work->prev_mtime1 = work->mtime1 ;
	work->mtime1 = MotionTime( work ) ;

	/* 近距離思考 */
	NearThinkAct( work ) ;
	/* 遠距離思考 */
	FarThinkAct( work ) ;

	/* ダメージチェック */
	CheckDamage( work ) ;

	/* ダメージチェック後にリセット */
	ResetFlag( work, SOL_FLAG_FILLIP_NOW ) ;

	/* アクション実行 */
	time = work->time ;
	work->time += TIME_BASE ;
	work->ftime = work->ftime_count ;
	work->ftime_count ++ ;
	action = work->action ;
	( *action )( work, time ) ;

	/* ターゲット更新 */
	UpdateTarget( work ) ;

	/* ライデンを押してみる */
	PushRaiden( work ) ;

	/* ダメージ中のライデンになんかいう */
	if ( GM_CheckPlayerStatus( PLAYER_DOWNED ) ) {
		if ( !Flag( work, SOL_FLAG_CHOUHATSU ) &&
			 GV_Time - work->last_player_damage_se_time > 10 * ( 300 / TIME_BASE ) ) {
			if ( GV_Time & 1 ) {
				if ( GM_Vitality < GM_VitalityMax / 3 ) {
					SeSet( ChouhatsuSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 0 ) ;
				} else {
					SeSet( ChouhatsuSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB, 0 ) ;
				}
				work->last_player_damage_se_time = GV_Time ;
				LastPlayerDamageSeTime = GV_Time ;
			}
			SetFlag( work, SOL_FLAG_CHOUHATSU ) ;
		}
	} else {
		ResetFlag( work, SOL_FLAG_CHOUHATSU ) ;
	}

}

/*----------------------------------------------------------------*/

extern int gBP_KillCurrentBoss;
extern int gBP_DamageCurrentBoss;

static	void	Act( Work *work )
{
	float		p_height ;

   if (gBP_KillCurrentBoss || gBP_DamageCurrentBoss)
   {
      gBP_KillCurrentBoss = FALSE;
      gBP_DamageCurrentBoss = FALSE;
      work->damage = gBP_KillCurrentBoss ? work->life_gage.value : work->life_gage.value / 2;
      SetStatus(work, SOL_STATE_DAMAGED);
   }

	/* ライフ更新 */
	if ( work->damage != 0 ) {
		work->life_gage.value -= work->damage ;
		work->damage = 0 ;
		if ( work->life_gage.value <= 0 ) work->life_gage.value = 0 ;
	}
	if ( work->faint_damage != 0 ) {
		work->life_gage.m9_value -= work->faint_damage ;
		work->faint_damage = 0 ;
		if ( work->life_gage.m9_value <= 0 ) work->life_gage.m9_value = 0 ;
	}

	p_height = work->body.height ;

	GM_ActMotion( &work->body ) ;

	/* 腰高さ補完なし */
    if ( Flag( work, SOL_FLAG_NO_WAIST_INTERP ) ) {
		int		use_layer ;

		use_layer = work->body.m_ctrl->use_move_layer ;
		work->body.height 
			= work->body.m_ctrl->height 
			= work->body.m_ctrl->old_height 
			= work->body.m_ctrl->mt3_ctrl[ use_layer ].move->step.vw ;
		work->body.m_ctrl->root_height 
			= work->body.m_ctrl->root_old_height 
			= work->body.m_ctrl->mt3_ctrl[ use_layer ].root->step.vw ;
		ResetFlag( work, SOL_FLAG_NO_WAIST_INTERP ) ;
	}

	work->control.height = work->body.height ;

	if ( work->control.grounded & 1 ) work->control.step.vy = 0.0F ;
	if ( !Flag( work, SOL_FLAG_NO_GRAVITY ) ) work->control.step.vy -= 96.0F ;

	if ( Flag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ) {
		work->control.step.vx -= work->body.m_ctrl->step.vx ;
		work->control.step.vz -= work->body.m_ctrl->step.vz ;
	}

	if ( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) {
		work->control.step.vy = work->body.height - p_height ;
	}

	GM_ActControl( &work->control ) ;

	if ( !Flag( work, SOL_FLAG_NO_IK ) ) {
		Leg_IKcalc( &work->control, &work->body ) ;
	}

	GM_ActObject2( &work->body ) ;

	DG_GetLightMatrix( &work->control.mov, work->lights ) ;

	/* アジャスト値リセット */
	ResetRotAdjust( work ) ;

	Action( work ) ;

	/* アジャスト計算 */
	RotAdjust( work ) ;

	ExprFaceGuardPos( work ) ;

	{
		float		t = work->va_t ;
#if 0
		if ( GV_PadData->press & PAD_L1 ) {
			t = 1.0F - work->va_t ;
		}
#endif
		/* 頂点アニメ */
		UpdateVAnime( work, t ) ;
	}
	/* 生体センサー */
	{
		extern	void	GM_VibrateSensor( CONTROL *, int * ) ;
		GM_VibrateSensor( &work->control, &work->vib_time ) ;
	}
}

static	void	Die( Work *work )
{
	int			i ;

	GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeTarget( &work->defense ) ;
	GM_FreeHomingTrg( &work->homing_trg ) ;
	GM_RemoveGageSet( &work->life_gage ) ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		ExitVertexAnimation( work->va_work[ i ] ) ;
	}
	if ( work->gantai != NULL ) {
		DG_DequeueObjs( work->gantai ) ;
		DG_FreeObjs( work->gantai ) ;
	}
	if ( work->faceguard != NULL ) {
		DG_DequeueObjs( work->faceguard ) ;
		DG_FreeObjs( work->faceguard ) ;
	}
	if ( work->kwt_sht != NULL ) {
		DG_DequeueObjs( work->kwt_sht ) ;
		DG_FreeObjs( work->kwt_sht ) ;
	}
	if ( work->mst_sht != NULL ) {
		DG_DequeueObjs( work->mst_sht ) ;
		DG_FreeObjs( work->mst_sht ) ;
	}
	SOL_SolControl = NULL ;
	SOL_Work = NULL ;
}

/*----------------------------------------------------------------*/

extern void *NewSmokeBlurEffect( FMATRIX *world, int start_speed, int end_speed, int start_size, int end_size, int spot_size, int spot_angle, int n_prims, int interval, int color, int flag );

/* 初期化 */
static	int	GetResources( Work *work, int name, int where )
{
	SetSolGameLevel() ;

	if ( InitControl( work, name, where ) < 0 ) return -1 ;
	if ( InitObject( work, where ) < 0 ) return -1 ;
	if ( InitTarget( work, name, where ) < 0 ) return -1 ; 
	if ( InitGage( work ) < 0 ) return -1 ;
	if ( InitVAnime( work ) < 0 ) return -1 ;

	PL_GetOptionFV( 'A', &work->map_center ) ;

	/* 間合いフェーズからスタート */
	work->flag |= SOL_FLAG_CAREFUL ;
	work->careful_time = GV_Time ;

	//SetMode( work, StandStillK ) ;
	SetMode( work, StandStillC ) ;


	/* 蛇手 */
	{
		void	*child ;

		child = NewSolidusSnakeArm( GV_StrCode( "sol_snakearm_mh_mt" ), &work->body, HUMAN21_MUNE ) ;
		GV_SetActorChild( &work->actor, child ) ;
	}	

	/* 刀 */
	{
		void	*child ;

		child = NewKyouwaBlade( &work->control, &work->body, 
							    HUMAN21_MIGI_TE, &work->kwt_trg ) ;
		if ( child != NULL ) GV_SetActorChild( &work->actor, child ) ;
		child = NewMinshuBlade( &work->control, &work->body, 
							    HUMAN21_HIDARI_TE, &work->mst_trg ) ;
		if ( child != NULL ) GV_SetActorChild( &work->actor, child ) ;
	}
	/* 影 */
	GV_SetActorChild( work, NewShadow( &work->body.objs->objs[ 16 ],
									   &work->body.objs->objs[ 20 ],
									   &work->control, work->lights, &work->shadow ) ) ;
	work->shadow = 1 ;

	SOL_SolStatus = 0 ;
	PL_PluginLiftup() ;	/* 蛇手つかみプラグインの登録 */

	work->wakeup_time = GV_Time ;
	work->mode = GCL_GetOptionValue( 's', 0 ) ;

	/* プロック */
	if ( GCL_GetOption( 'O' ) != NULL ) {
		int			i = 0 ;
		while( GCL_NextStr() != NULL ) {
			work->procs[ i ] = GCL_GetNextInt() ;
			if ( ++ i == 8 ) break ;
		}
	}

	work->last_arm_attack = -1 ;
	work->dash_fire_current = -1 ;

	SeTime = GV_Time ;
	LastPlayerDamageSeTime = GV_Time ;

	/* 集束パーティクル呼び出し */
	{
	    extern void *NewSolidusEnergyPrim( OBJECT *body, int *mode );
	    work->energy_mode = 0;
	    GV_SetActorChild( work, NewSolidusEnergyPrim( &work->body, &work->energy_mode ) );    
	}

	/* ジャンプ攻撃範囲 */
	GCL_GetOption( 'j' ) ;
	work->jump_bound1.vx = ( float )GCL_GetNextInt() ;
	work->jump_bound1.vz = ( float )GCL_GetNextInt() ;
	work->jump_bound2.vx = ( float )GCL_GetNextInt() ;
	work->jump_bound2.vz = ( float )GCL_GetNextInt() ;

	/* ＥＸＴＲＥＭＥでは、刀壁チェックきる */
	if ( GM_GameLevel == GM_LEVEL_EXTREME ) {
		extern int PL_COM_BladeNoCheckSegment( void ) ;
		PL_COM_BladeNoCheckSegment() ;
	}

	/* 字幕 */
	if ( GCL_GetOption( 'P' ) != NULL ) {
		int			loop ;

		switch( GM_Language ) {
		case GM_LANG_ENGLISH :
			loop = 1 ;
			break ;
		case GM_LANG_GERMANY :
			loop = 2 ;
			break ;
		case GM_LANG_FRENCH :
			loop = 3 ;
			break ;
		case GM_LANG_ITALY :
			loop = 4 ;
			break ;
		case GM_LANG_SPANISH :
			loop = 5 ;
			break ;
		case GM_LANG_JAPANESE :
			loop = 6 ;
			break ;
		default :
			loop = 1 ;
		}
		while( -- loop >= 0 ) {
			work->caption1 = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString()) ;
			work->caption2 = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		}
	}
	if ( GCL_GetOption( 'I' ) != NULL ) {
		work->caption_time1 = DIRECT_TICK( GCL_GetNextInt() ) ;
		work->caption_time2 = DIRECT_TICK( GCL_GetNextInt() ) ;
		work->caption_end = DIRECT_TICK( GCL_GetNextInt() ) ;
	}

	SOL_Work = work ;
#ifdef DEBUG_MODE
	StartDebugMenu( work ) ;
#endif	
	return 0 ;
}

/* 起動 */
void		*NewSolidus( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PREV, GV_CLASS_OBJECT, 
									 sizeof( Work ), 8 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

