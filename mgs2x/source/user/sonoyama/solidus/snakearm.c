//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   snakearm.c
   ソリダス／スネークアーム
   
   2001/03/28	M.Sonoyama
   $Id: snakearm.c,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"rand.h"
#include	"sol_common.h"

/*----------------------------------------------------------------*/

typedef	struct	{
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[ 4 ] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} RMT_HEADER ;

typedef struct {
	int			name ;
	int			count ;
	int			length ;
	int			joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} RMT_DATA ;

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[ 64 ] ;
	FVECTOR	quat_buffer[ 4 ] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[ 64 ] ;
} ScrPadWork ;

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX		actor ;
	
	TARGET			offense ;
	TARGET			defense ;
	POWER_TARGET	off_power ;

	FVECTOR			root ;
	FVECTOR			tip ;
	FVECTOR			tip_right ;
	FVECTOR			tip_left ;

	FVECTOR			prev_tip_right ;
	FVECTOR			prev_tip_left ;

	DG_EVMOBJ		*evmobj ;
	MOTION_CONTROL	*m_ctrl ;
	int				name ;
	int				interp_count ;
	RMT_DATA		rmt_data ;
	int				model ;
	int				cur_motion ;
	int				next_motion ;

	OBJECT			*pbody ;
	int				unit ;

	int				flag ;
	int				data ;
	int				data2 ;

	void			*flow ;
	int				flow_flag ;
} Work ;

static	Work		*SnakeArmWork = NULL ;

DG_EVMOBJ			*SnakeArmEvmObj = NULL ;

enum {
	FLAG_NONE				= 0x0000,
	FLAG_ONLINE_ATTACK		= 0x0001,
	FLAG_SET_DEFENSE_TARGET	= 0x0002,
	FLAG_MISSILE_FIRE_RIGHT = 0x0010,
	FLAG_MISSILE_FIRE_LEFT =  0x0020,
	FLAG_TRIP_R =			  0x0100,
	FLAG_TRIP_L =			  0x0200,
	FLAG_CLAW_ATTACK =		  0x0400,
	FLAG_ARM_UNDO =			  0x1000,
} ;

/* かわしＳＥ */
static	int		KawashiSE[] = {
	SD_V_SOLWHY01, SD_V_SOLWHY02, SD_V_SOLWHY03
} ;


/* グローバル */
FVECTOR	SOL_SnakeArmHangPos ;

void		SOL_SnakeArmDestroy( void )
{
	extern	void	*NewBlast2( FMATRIX *, int, int, int, int, int, int ) ;
	if ( SnakeArmWork == NULL ) return ;
	//NewBlast2( &SnakeArmWork->evmobj->world, BOTH_SIDE, 2000, 4000, 10, 0, WP_C4Bomb ) ;
	GV_DestroyActor( SnakeArmWork ) ;
}

/*----------------------------------------------------------------*/

/* 先端位置更新 */
static	void	UpdateRootAndTip( Work *work )
{
	FMATRIX		*skel_mats ;
	FVECTOR		to, to1, to2 ;

	skel_mats = work->evmobj->matrix[ work->evmobj->use_buffer ] ;	
	to1.vx = work->evmobj->def->skeleton[ 14 ].rt_tx ;
	to1.vy = work->evmobj->def->skeleton[ 14 ].rt_ty ;
	to1.vz = work->evmobj->def->skeleton[ 14 ].rt_tz ;
	to2.vx = work->evmobj->def->skeleton[ 30 ].rt_tx ;
	to2.vy = work->evmobj->def->skeleton[ 30 ].rt_ty ;
	to2.vz = work->evmobj->def->skeleton[ 30 ].rt_tz ;	

	DG_SetPos( &skel_mats[ 14 ] ) ;
	DG_PutVector( &to1, &to1, 1 ) ;
	DG_SetPos( &skel_mats[ 30 ] ) ;
	DG_PutVector( &to2, &to2, 1 ) ;

	DG_COPY_VEC( &work->prev_tip_right, &work->tip_right ) ;
	DG_COPY_VEC( &work->prev_tip_left, &work->tip_left ) ;
	
	DG_COPY_VEC( &work->tip_right, &to2 ) ;
	DG_COPY_VEC( &work->tip_left, &to1 ) ;
	DG_COPY_VEC( &SOL_SnakeArmHangPos, &to1 ) ;

	_sceVu0AddVector( &to, &to1, &to2 ) ;
	_sceVu0ScaleVector( &to, &to, 0.50F ) ;
	GV_MatToVec( &skel_mats[ 0 ], &work->root ) ;

	DG_COPY_VEC( &work->tip, &to ) ;
	//DG_COPY_VEC( &SOL_SnakeArmHangPos, &to ) ;
}




/* ミサイル発射 */
static	void	FireMissile( Work *work )
{
	FMATRIX		world ;
	FMATRIX		*skel_mats ;
	FVECTOR		to ;

	if ( !( work->flag & ( FLAG_MISSILE_FIRE_RIGHT | FLAG_MISSILE_FIRE_LEFT ) ) ) return ;
	skel_mats = work->evmobj->matrix[ work->evmobj->use_buffer ] ;	
	if ( work->flag & FLAG_MISSILE_FIRE_LEFT ) {
		to.vx = work->evmobj->def->skeleton[ 13 ].rt_tx ;
		to.vy = work->evmobj->def->skeleton[ 13 ].rt_ty ;
		to.vz = work->evmobj->def->skeleton[ 13 ].rt_tz ;	
		DG_SetPos( &skel_mats[ 13 ] ) ;
		work->flag &= ~FLAG_MISSILE_FIRE_LEFT ;
	} else {
		to.vx = work->evmobj->def->skeleton[ 29 ].rt_tx ;
		to.vy = work->evmobj->def->skeleton[ 29 ].rt_ty ;
		to.vz = work->evmobj->def->skeleton[ 29 ].rt_tz ;	
		DG_SetPos( &skel_mats[ 29 ] ) ;
		work->flag &= ~FLAG_MISSILE_FIRE_RIGHT ;
	}
	DG_MovePos( &to ) ;
	DG_GetPos( &world ) ;
	NewSnakeArmMissile( &world, work->pbody->map_name ) ;
}

/*----------------------------------------------------------------*/

/* 転ばし攻撃 */
static	void	TripAttack( Work *work )
{
	int			flag ;

	flag = work->flag & ( FLAG_TRIP_R | FLAG_TRIP_L ) ;
	if ( !flag ) return ;
	work->flag &= ~( FLAG_TRIP_R | FLAG_TRIP_L ) ;
	if ( !CheckSolStatus( SOL_CS_SNAKEARM_TRIP_SUCCESS ) ) {
		FVECTOR			force, *to ;

		if ( flag & FLAG_TRIP_R ) {
			_sceVu0SubVector( &force, &work->tip_right, &work->prev_tip_right ) ;
			to = &work->tip_right ;
		} else {
			_sceVu0SubVector( &force, &work->tip_left, &work->prev_tip_left ) ;
			to = &work->tip_left ;
		}
		DG_COPY_VEC( &work->off_power.force, &force ) ;
		GM_MoveOnlineTargetMap( &work->offense, &work->root, to, work->pbody->map_name ) ;
		//ViewFromTo( &work->root, to, 32, 232, 32 ) ;
		GM_PutTarget( &work->offense ) ;
	}
}

static	void	TripAttackHit( TARGET *off, TARGET *def, void *ptr )
{	
	if ( def != GM_PlayerTarget ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
	if ( GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DAMAGED | 
							   PLAYER_DOWNED | PLAYER_ROLLING |
							   PLAYER_INVINCIBLE ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}	
#if 0
	if ( PL_SlashNow() ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
	if ( PL_SlashGuardNow() ) {
		GM_SeSetMode( KawashiSE[ irnd() % 3 ], &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		PL_AttackGuardBlade( &off->center ) ;
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
#endif
	SetSolStatus( SOL_CS_SNAKEARM_TRIP_SUCCESS ) ;
}

/* 爪攻撃 */
static	void	ClawAttack( Work *work )
{
	if ( !( work->flag & FLAG_CLAW_ATTACK ) ) return ; 
	work->flag &= ~( FLAG_CLAW_ATTACK ) ;
	if ( !CheckSolStatus( SOL_CS_SNAKEARM_CLAW_SUCCESS ) ) {
		FVECTOR			force, *to ;

		_sceVu0SubVector( &force, &work->tip_left, &work->prev_tip_left ) ;
		to = &work->tip_left ;
		DG_COPY_VEC( &work->off_power.force, &force ) ;
		GM_MoveOnlineTargetMap( &work->offense, &work->root, to, work->pbody->map_name ) ;
		GM_PutTarget( &work->offense ) ;
	}
}

static	void	ClawAttackHit( TARGET *off, TARGET *def, void *ptr )
{
	if ( def != GM_PlayerTarget ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
	if ( GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DAMAGED | 
							   PLAYER_DOWNED | PLAYER_ROLLING |
							   PLAYER_INVINCIBLE ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}	
	if ( PL_SlashNow() ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
	if ( PL_SlashGuardNow() ) {
		GM_SeSetMode( KawashiSE[ irnd() % 3 ], &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		return ;
	}
	SetSolStatus( SOL_CS_SNAKEARM_CLAW_SUCCESS ) ;
}

/* OnlineAttack */
static	void	OnlineAttack( Work *work ) 
{
	if ( !( work->flag & FLAG_ONLINE_ATTACK ) ) return ;
	work->flag &= ~FLAG_ONLINE_ATTACK ;	

	if ( !CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ) {
		/* まだ捕まえていない */
		//GM_MoveOnlineTargetMap( &work->offense, &work->root, &work->tip, work->pbody->map_name ) ;
		GM_MoveOnlineTargetMap( &work->offense, &work->root, 
							    &work->tip_left, work->pbody->map_name ) ;
		GM_PutTarget( &work->offense ) ;
		//NewTargetView2( &work->offense, 32, 32, 232 ) ;
	}
}

/* SetDefenseTarget */
static	void	SetDefenseTarget( Work *work )
{
	FVECTOR		size, offset, diff ;
	FMATRIX		world ;
	SVECTOR		rot ;
	int			len ;

	if ( !( work->flag & FLAG_SET_DEFENSE_TARGET ) ) return ;
	work->flag &= ~FLAG_SET_DEFENSE_TARGET ;
	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ) return ;

	GM_TargetResetSkip( &work->defense ) ;
	//_sceVu0SubVector( &diff, &work->tip, &work->root ) ;
	_sceVu0SubVector( &diff, &work->tip_left, &work->root ) ;
	len = GV_VecLen3F( &diff ) ;
	GV_SetVec3( &size, 250.0F, 100.0F, len / 2.0F ) ;
	GV_SetVec3( &offset, 0.0F, 0.0F, len / 2.0F ) ;
	GM_SetTargetSize( &work->defense, &size ) ;
	DG_COPY_VEC( &work->defense.offset, &offset ) ;
	GV_VecToRot( &diff, &rot ) ;
	DG_SetPos2( &work->root, &rot ) ;
	DG_GetPos( &world ) ;
	GM_MoveTarget2Map( &work->defense, &world, work->pbody->map_name ) ;
	//NewTargetView2( &work->defense, 32, 232, 32 ) ;
}

static	void	DefenseHit( TARGET *off, TARGET *def, void *ptr )
{
	SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) ;
	GM_SeSetMode( SD_E_GURDHEB1, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
}

static	void	OnlineHit( TARGET *off, TARGET *def, void *ptr )
{
	if ( GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DAMAGED | 
							   PLAYER_DOWNED | PLAYER_ROLLING |
							   PLAYER_BEYOND | PLAYER_FORCE |
							   PLAYER_INVINCIBLE ) ) {
		return ;
	}	
	if ( PL_SlashNow() ) {
		return ;
	}
	if ( PL_SlashGuardNow() ) {
		GM_SeSetMode( KawashiSE[ irnd() % 3 ], &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		PL_AttackGuardBlade( &off->center ) ;
		return ;
	}

	SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
	NewPadVibration2( GV_StrCode( "snake_arm" ), 0 ) ;

	{
		GV_MSG	msg ;
		int		mesg ;

		mesg = 17 ;	/* PL_MSG_LIFTUP */
		msg.address = GM_PlayerControl->name ;
		msg.message = &mesg ;
		msg.message_len = 1 ;
		GV_SendMessage( &msg ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	float		p_height ;

	ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) ;
	ResetSolStatus( SOL_CS_SNAKEARM_TRIP_SUCCESS | SOL_CS_SNAKEARM_CLAW_SUCCESS ) ;

	if ( work->next_motion != work->cur_motion ) {
		//printf( "next motion %d\n", work->next_motion ) ;
		MT_SetMotionData( work->m_ctrl, 0, work->next_motion, 0, -1 ) ;
		MT_SetMotionInterp( work->m_ctrl, work->interp_count, I64(0xfffffffffff) ) ;
		work->cur_motion = work->next_motion ;
		//printf( "motion set ok!!\n" ) ;
	}

	if ( work->flag & FLAG_ARM_UNDO ) {
		DG_SetPos( &work->evmobj->world ) ;
	} else {
		DG_SetPos( &work->pbody->objs->objs[ work->unit ].world ) ;
		DG_GetPos( &work->evmobj->world ) ;
	}

	p_height = work->m_ctrl->height ;
   DG_Arm_SwitchEvmBuffer(work->evmobj);
	MT_ActMotion( work->m_ctrl, NULL, work->evmobj ) ;

	if ( work->flag & FLAG_ARM_UNDO ) {
		FVECTOR		step ;
		/* 位置の更新 */
		DG_COPY_VEC( &step, &work->m_ctrl->mt3_ctrl[ 0 ].move->step ) ;
		DG_SetPos( &work->evmobj->world ) ;
		DG_RotVector( &step, &step, 1 ) ;

		step.vy = work->m_ctrl->height - p_height ;

		work->evmobj->world.m[ 3 ][ 0 ] += step.vx ;
		work->evmobj->world.m[ 3 ][ 1 ] += step.vy ;
		work->evmobj->world.m[ 3 ][ 2 ] += step.vz ;
		DG_SetPos( &work->evmobj->world ) ;
	}

	MT_EvmActMotion( work->m_ctrl, work->evmobj ) ;
	MT_ActSequenceSEV( work->m_ctrl->sar_ctrl, work->m_ctrl, NULL, work->evmobj ) ;

	if ( ( work->flag & FLAG_ARM_UNDO ) && 
		MT_CHECK_LAST1( work->m_ctrl, 0 ) ) {
		SOL_SnakeArmDestroy() ;
		return ;
	}

	UpdateRootAndTip( work ) ;

	GM_TargetSetSkip( &work->defense ) ;

	FireMissile( work ) ;
	OnlineAttack( work ) ;
	TripAttack( work ) ;
	ClawAttack( work ) ;
	SetDefenseTarget( work ) ;
}

static	void	Die( Work *work )
{
	MT_FreeMotion( work->m_ctrl ) ;
	DG_DequeueEvmObj( work->evmobj ) ;
	DG_FreeEvmObj( work->evmobj ) ;
	GM_FreeTarget( &work->defense ) ;
	SnakeArmWork = NULL ;
	SnakeArmEvmObj = NULL ;
}

/*----------------------------------------------------------------*/

static	void	SetDefTarget( Work *work )
{
	TARGET		*t ;
	
	t = &work->defense ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER, 0,
				  ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, DefenseHit, work ) ;
	GM_TargetSetSkip( t ) ;
	GM_PutTarget( t ) ;
}

static	int	GetResources( Work *work, int model )
{
	EVM_DEF			*def ;

	def = ( EVM_DEF * )GV_GetCache( GV_CacheID( model, 'e' ) ) ;
	ASSERT( def != NULL ) ;
	work->evmobj = DG_MakeEvmObj( def, 0, 0 ) ;
	ASSERT( work->evmobj != NULL ) ;
	DG_QueueEvmObj( work->evmobj ) ;
	work->evmobj->light = work->pbody->objs->light ;

	work->cur_motion = 0 ;
	work->next_motion = 0 ;

	work->m_ctrl = MT_InitMotion( NULL, 1, GV_StrCode( "sol_snakearm" ), MT_FLAG_TRANS ) ;
	work->m_ctrl->sar_ctrl = MT_InitSequence( 1, GV_StrCode( "sol_snakearm" ), 0 );
	work->m_ctrl->se_table_id = -1 ;
	MT_SetMotionData( work->m_ctrl, 0, 0, 0, -1 ) ;

	SetDefTarget( work ) ;

	work->flow = NewSolidusSnakearmFlow( work->evmobj, &work->flow_flag ) ;
	GV_SetActorChild( work, work->flow ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void		*NewSolidusSnakeArm( int model, OBJECT *pbody, int unit )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PREV, GV_CLASS_OBJECT,
									 sizeof( Work ), 16 ) ; /* solidus本体より後 */
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->model = model ;
		work->pbody = pbody ;
		work->unit = unit ;
		if ( GetResources( work, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	SnakeArmEvmObj = work->evmobj ;
	SnakeArmWork = work ;
	return work ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/* グローバル */

#define	NTSC_TIME_BASE 	(5)

void		SOL_SetSnakeArmMotionEX( int nNo, int interp, int mode )
{
	if ( SnakeArmWork == NULL ) return ;
	SnakeArmWork->next_motion = nNo ;
	SnakeArmWork->interp_count = interp * NTSC_TIME_BASE ;
	if ( mode == 1 ) SnakeArmWork->cur_motion = -1 ;
}

void		SOL_SetSnakeArmMotion( int nNo, int interp )
{
	SOL_SetSnakeArmMotionEX( nNo, interp, 0 ) ;
}

void		SOL_SnakeArmAttackOnline( int joint1, int joint2 )
{
	Work		*work ;
	TARGET		*t ;

	if ( SnakeArmWork == NULL ) return ;

	work = SnakeArmWork ;
	t = &work->offense ;
	GM_SetTarget( t, TARGET_OFFENSE|TARGET_ONLINE|TARGET_THROUGH|TARGET_POWER, 0,
				  PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, OnlineHit, work ) ;
	GM_SetTargetWeaponType( t, I64(0) ) ;
	work->data = joint1 ;
	work->data2 = joint2 ;
	work->flag |= FLAG_ONLINE_ATTACK ;
}

void		SOL_SnakeArmAttackClaw( void )
{
	Work		*work ;
	TARGET		*t ;
	int			damage = 4 ;

	if ( SnakeArmWork == NULL ) return ;

	work = SnakeArmWork ;
	t = &work->offense ;
	GM_SetTarget( t, TARGET_OFFENSE|TARGET_ONLINE|TARGET_POWER, 0,
				  PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;

	/* 難易度別 */
	switch( SOL_GameLevel ) {
	case GM_LEVEL_VERYEASY :
		damage -= 2 ;
		break ;
	case GM_LEVEL_EASY :
		damage -= 1 ;
		break ;
	case GM_LEVEL_HARD :
		damage += 1 ;
		break ;
	case GM_LEVEL_EXTREME :
		damage += 2 ;
		break ;
	case GM_LEVEL_NORMAL :
	default :
	  ;
	}
	if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
	if ( damage <= 0 ) damage = 1 ;

	GM_SetPowerTarget( t, &SnakeArmWork->off_power, POWER_CONST, 255, 0, damage, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, ClawAttackHit, work ) ;
	GM_SetTargetWeaponType( t, WP_SHOTGUN_NEAR | WP_BLADEGUARD ) ;
	work->flag |= FLAG_CLAW_ATTACK ;
}

void		SOL_SnakeArmAttackTrip( int which )
{
	TARGET	*t ;

	if ( SnakeArmWork == NULL ) return ;
	if ( which == 0 ) SnakeArmWork->flag |= FLAG_TRIP_R ;
	else 			  SnakeArmWork->flag |= FLAG_TRIP_L ;
	t = &SnakeArmWork->offense ;
	GM_SetTarget( t, TARGET_OFFENSE|TARGET_ONLINE|TARGET_POWER|TARGET_NAME_IS_SE, 0,
				  PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetPowerTarget( t, &SnakeArmWork->off_power, POWER_CONST, 255, 0, 1, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, TripAttackHit, SnakeArmWork ) ;
	GM_SetTargetWeaponType( t, WP_TUMBLE | WP_NOMUTEKI | WP_NOBLOOD ) ;	
	t->name = SD_E_HEBHIT01 ;
}

void		SOL_SnakeArmSetDefenseTarget( void )
{
	if ( SnakeArmWork == NULL ) return ;
	SnakeArmWork->flag |= FLAG_SET_DEFENSE_TARGET ;
}

void		SOL_SnakeArmFireMissile( int dir )
{
	if ( SnakeArmWork == NULL ) return ;
	if ( dir == 0 ) {
		SnakeArmWork->flag |= FLAG_MISSILE_FIRE_RIGHT ;
	} else {
		SnakeArmWork->flag |= FLAG_MISSILE_FIRE_LEFT ;
	}
}

void		SOL_SnakeArmSetFlowFlag( int flag )
{
	if ( SnakeArmWork == NULL ) return ;
	SnakeArmWork->flow_flag = flag ;

}

void		SOL_SnakeArmUndo( void )
{
	if ( SnakeArmWork == NULL ) return ;
	SnakeArmWork->flag |= FLAG_ARM_UNDO ;
}
