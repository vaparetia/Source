//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wp_misle.c
	量産型ＲＡＹ用ミサイルプログラム

	2001/04/18 K.Takabe
	$Id: wp_misle.c,v 1.1.1.3 2002/11/19 11:51:28 Yoshizawa1 Exp $

*/
/*

	ミサイルは与えられたマトリクスから＋Ｚの方向に飛んでいく


*/
/* !NEED_PAL_READJUST! */
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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"


#ifdef KP_XBOX
#define GM_SeSetMode( _a, _b, _c ) GM_SeSetModeAddr( _a, _b, _c, GM_INVALID_ADDR )
#endif
/* ---------------------------------------------------------------- */
extern void *NewRAYMissileSmoke( FMATRIX *parent );
/* シグナル関連 */
#define SIGNAL_RAYMISSILESMOKE			(0x4665dc00)	/* GV_StrCode("RAYMissileSmoke")<<8 */
#define SIGNAL_RAYMISSILESMOKE_STOP		(SIGNAL_RAYMISSILESMOKE|0x01)
#define SIGNAL_RAYMISSILESMOKE_DEBUG	(SIGNAL_RAYMISSILESMOKE|0xff)
/* ---------------------------------------------------------------- */
extern void *NewBlast( FVECTOR *, int, int, int, int, int, int );
extern void RAYSERVER_SetMissile( FVECTOR *pos );

extern int		RAYSERVER_GameLevel ;			/* ゲームレベル（1,2,3,4,5） */
extern int		RAYSERVER_GameLevelOffset ;		/* ゲームレベルオフセット（-2,-1,0,1,2） */
extern int		RAYSERVER_GameDownLevel ;		/* ゲーム難易度低下レベル（2,1,0,0,0） */
extern int		RAYSERVER_GameUpLevel ;			/* ゲーム難易度上昇レベル（0,0,0,1,2） */
/* ---------------------------------------------------------------- */

#define BLAST_DAMAGE		(40)


extern float BP_AdjustTick3(float);
extern float BP_AdjustTick4(float);
#define V_SCALE	(BP_AdjustTick3(1.0f))		/* 速度スケール */
#define A_SCALE	(BP_AdjustTick4(1.0f))		/* 速度スケール */


#define FIRST_VELOCITY			(400.0f*V_SCALE)		/* 初速度 */
//#define MAX_VELOCITY			(400.0f*V_SCALE)		/* 最大速度 */
//#define MAX_VELOCITY			(350.0f*V_SCALE)		/* 最大速度 */
#define MAX_VELOCITY			(work->max_velocity)		/* 最大速度 */
#define MAX_VELOCITY0A			(200.0f*V_SCALE)		/* タイプ１フェーズ１最大速度 */
#define MAX_VELOCITY0B			(500.0f*V_SCALE)		/* タイプ１フェーズ２最大速度 */
#define MAX_VELOCITY1			(350.0f*V_SCALE+RAYSERVER_GameUpLevel*50)		/* タイプ２最大速度 */
//#define MAX_TURN				(GTE_PS2RAD(20)*V_SCALE)	/* １フレームでの最高補正角度 */
#define MAX_TURN				(GTE_PS2RAD(30)*V_SCALE)	/* １フレームでの最高補正角度 */
#define DEFAULT_ACCELERATION	(40.0f*A_SCALE)			/* 標準加速度 */
//#define MAX_HOMING_TIME			( 5 * 120 )				/* ホーミング有効時間 */
#define MAX_HOMING_TIME			( 5 * 180 )				/* ホーミング有効時間 */
#define MAX_VELOCITY_TIME		( 5 * 180 )				/* ホーミング有効時間 */
//#define TYPE2_HOMING_TIME		( 300 * 1 )				/* 場外攻撃用ホーミング開始時間 */
//#define TYPE2_HOMING_DELAY_TIME	( 300 * 1 )				/* 場外攻撃用ホーミング開始遅延時間 */
#define TYPE2_HOMING_TIME		( 300 * 1 )				/* 場外攻撃用ホーミング開始時間 */
#define TYPE2_HOMING_DELAY_TIME	( 300 * 1 )				/* 場外攻撃用ホーミング開始遅延時間 */

#define CHECK_SEG_FLAG		(HZX_SEG_RECOIL_TYPE|HZX_SEG_NO_PLAYER|HZX_SEG_NO_ENEMY|HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE|HZX_SEG_NO_RECOIL|HZX_SEG_NO_HARITSUKI|HZX_SEG_NO_DISP_RADAR|HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_SPRAY|HZX_SEG_NO_ENEMY_EYES|HZX_SEG_NO_KNOCK_SE)

#define CHECK_FLOOR_FLAG	(HZX_FLOOR_ALL)

#define TARGET_SIZE_X		( 100 )
#define TARGET_SIZE_Y		( 100 )
#define TARGET_SIZE_Z		( 100 )
#define ATTACK_DAMAGE		(BLAST_DAMAGE)

#define PLAYER_LEVEL		(4000.0f)

#define FIRE_OFFSET			(-400)		/* ミサイル原点からのバックファイアオフセット */

//#define MODEL_NAME			(4015562)		/* "ray_backmsl_close" */
//#define MODEL_FLAG			(DG_FLAG_ONEPIECE)
//#define MODEL_ROT_CORRECT	(2048)

//#define MODEL_NAME			(10575994)		/* "stg_msl" */
#define MODEL_NAME0			(2591299)		/* "pdray_kneemsl" */
//#define MODEL_NAME1			(640460)		/* "pdray_backmsl_close" */
#define MODEL_NAME1			(690153)		/* "pdray_backmsl" */
#define MODEL_NAME2			(4087014)		/* "pdray_backmsl_open" */
#define MODEL_NAME3			(14755291)		/* "pdray_landmine" */
#define MODEL_FLAG			(DG_FLAG_ONEPIECE)
#define MODEL_ROT_CORRECT	(-1024)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )

#define PRIM_FLAG			(DG_PRIM2_SPRT|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
#define N_PRIMS				(1)
#define N_VERTS				(32)
#define TEXTURE0			(12296685)	/* "light10_msk" */
#define TEXTURE1			(762348)	/* "light05_msk" */

#define UVRGBWH_WH( _e, _w, _h ) { (_e)->w = (_w) ; (_e)->h = (_h) ; }
#define UVRGBWH_RGBA( _e, _r, _g, _b, _a ) {\
											 (_e)->r = (_r) ;\
											 (_e)->g = (_g) ;\
											 (_e)->b = (_b) ;\
											 (_e)->a = (_a) ;\
									   }
#define UVRGBWH_UV( _e, _u0, _v0, _u1, _v1 ) {\
											 (_e)->u0 = (_u0) ;\
											 (_e)->v0 = (_v0) ;\
											 (_e)->q0 = 4096 ;\
											 (_e)->f0 = 0x0fff;\
											 (_e)->u1 = (_u1) ;\
											 (_e)->v1 = (_v1) ;\
											 (_e)->q1 = 4096 ;\
											 (_e)->f1 = 0x0fff;\
									   }

/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			hzx_id ;
	int			type ;

	FMATRIX		world ;			/* マトリクス */
	FVECTOR		pos ;			/* 座標 */
	FVECTOR		velocity ;		/* 速度 */
	FVECTOR		acceleration ;	/* 加速度 */
	FVECTOR		homing_target ;	/* ホーミング座標 */
	FVECTOR		start_trg_dir ;	/* 発射直後のホーミング方向ベクトル */
	float		a_speed ;		/* スピード（＝加速度） */
	float		max_angle ;		/* 最大補正角度 */
	float		max_velocity ;	/* 最大スピード */
	int			count ;			/* カウント */
	int			state ;			/* 動作状態 */
	int			error_count ;	/* 誤差用カウント */
	FVECTOR		old_correct_axis ;	/* 以前の角度補正回転軸ベクトル */
	FVECTOR		old_player_pos ;	/* チャフ使用時用最終記録プレイヤー座標 */

	FMATRIX		light[2] ;
	DG_OBJS		*objs ;
	//DG_PRIM2	*prim ;

	void		*smoke_work ;		/* 煙エフェクトワーク */

	TARGET			target ;					/* ロックオン用ターゲット */
	POWER_TARGET	power_target ;				/* パワーターゲット */
	FVECTOR			target_force ;				/* ターゲット力積ベクトル */
	FVECTOR			damage_pos ;				/* ダメージ座標 */

	TARGET			defense_target ;
	POWER_TARGET	defense_power ;
	int				target_damage_flag ;		/* 食らい判定フラグ */

	int				sound_phase ;
	int				sound_count ;

} Work ;


/* ---------------------------------------------------------------- */
static void TargetCallback( TARGET *offence, TARGET *defense, Work *work )
{
	POWER_TARGET	*power ;

	if ( offence->power == NULL ) return ;
	power = offence->power ;
	if ( offence->weapon_type == WP_BLADE ){
		if ( !( work->type & 2 ) ){
			work->target_damage_flag = 2 ;
		}
	}
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
		if ( work->target_damage_flag != 2 ){
			work->target_damage_flag = 1 ;
		}
		break ;
	  case POWER_ONCE:
	  case POWER_DECREASE:
	  case POWER_THRESHOLD:
	  case POWER_CONST:
	  default:
		break ;
	}
}
/* ---------------------------------------------------------------- */
/* 最大変化距離にリミットをつけてＸＺ平面で近づける */
static void NearPositionLimitXZ( FVECTOR *res, FVECTOR *from, FVECTOR *to, float max_len )
{
	FVECTOR		tmp_vec ;
	float		len ;
	GTE_SubVector( &tmp_vec, to, from );
	tmp_vec.vy = 0 ;
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	//printf("%f %f\n", len, max_len );
	if ( len > max_len ){
		GTE_ScaleVector( &tmp_vec, &tmp_vec, max_len / len );
		GTE_AddVector( res, from, &tmp_vec );
		return ;
	} else {
		*res = *to ;
		return ;
	}
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	static FVECTOR	z_vec = {0,0,1,1} ;
	FVECTOR		speed_vec, old_pos ;
	float		max_angle ;
	int			homing_enable ;
	int			blast_type ;

	GTE_InitGTE();

	blast_type = ( RAYSERVER_GameLevel > 1 ) ? WP_C4Bomb : WP_ShotGun_Far ;

	/* ホーミング座標設定 */
	if ( !( work->type & 2 ) ){
		FVECTOR		tmp_vec ;
		float		t ;

#if 0
		/* ホーミング有効フラグ設定 */
		//homing_enable = !( GM_GameStatus & STATE_CHAFF ) || ( ( work->count & 3 ) != 0 ) ;
		homing_enable = !( GM_GameStatus & STATE_CHAFF ) ;

		/* ホーミング座標更新 */
		if ( homing_enable ){
			work->homing_target = GM_PlayerPosition ;
			work->old_player_pos = work->homing_target ;
		} else {
			work->homing_target = work->old_player_pos ;
		}
		homing_enable = 1 ;
#else
		/* ホーミング有効フラグ設定 */
		homing_enable = !( GM_GameStatus & STATE_CHAFF ) || ( ( work->count & 3 ) != 0 ) ;

		if ( RAYSERVER_GameLevel > 1 ){/* ベリーイージー以下の時にはプレイヤー追従を行わない */
			work->homing_target = GM_PlayerPosition ;
		} else {
			work->homing_target = work->old_player_pos ;
		}
		work->old_player_pos = work->homing_target ;
#endif
		/* ミサイル位置が高い場合にはミサイルを床方向にホーミングさせる */
		if ( work->pos.vy > ( PLAYER_LEVEL + 4000.0f ) ){
			tmp_vec = work->pos ;
			tmp_vec.vy = PLAYER_LEVEL ;
			tmp_vec.vy = PLAYER_LEVEL + 1000.0f  ;
			GTE_InterVector( &work->homing_target, &tmp_vec, &work->homing_target, 0.5f );
		}

		/* 最大パラメータ等の更新 */
		max_angle = MAX_TURN - work->count * MAX_TURN / MAX_HOMING_TIME ;
		max_angle = DG_MAX( 0.0f, max_angle );
		if ( RAYSERVER_GameLevel <= 1 ){/* ベリーイージー以下ではホーミング性能を落とす */
			max_angle /= 2 ;
		}
		t = (float)work->count / MAX_VELOCITY_TIME ;
		t = DG_MIN( 1.0f, t );
		work->max_velocity = MAX_VELOCITY0A + ( MAX_VELOCITY0B - MAX_VELOCITY0A ) * t ;

	} else {
		/* ホーミング有効フラグ設定 */
		//homing_enable = !( GM_GameStatus & STATE_CHAFF ) || ( ( work->count & 1 ) == 0 ) ;
		//homing_enable = 1 ;
		homing_enable = !( GM_GameStatus & STATE_CHAFF ) ;

		/* ホーミング座標更新 */
#if 0
		if ( homing_enable ){
			/* １秒間はホーミング座標更新 */
			if ( work->count < ( TYPE2_HOMING_TIME + TYPE2_HOMING_DELAY_TIME ) ){
				work->homing_target = GM_PlayerPosition ;
				work->old_player_pos = work->homing_target ;
			}
		}
		homing_enable = 1 ;
#else
		if ( homing_enable ){
			if ( RAYSERVER_GameLevel > 1 ){/* ベリーイージー以下の時にはプレイヤー追従を行わない */
				NearPositionLimitXZ( &work->homing_target, &work->homing_target, &GM_PlayerPosition, 30.0f );
			}
			//AN_Test_Eye2( &work->homing_target, 3 );
			//work->homing_target = GM_PlayerPosition ;
		}
		homing_enable = 1 ;
#endif
		/* 最大パラメータ等の更新 */
		if ( work->count < TYPE2_HOMING_DELAY_TIME ){
			max_angle = 0 ;
		} else {
			max_angle = work->max_angle ;
		}
	}
	//work->homing_target.vy = PLAYER_LEVEL ;

	switch ( work->state ){
	  case 0:
		/* 初期ホーミング方向ベクトルの記録 */
		GTE_SubVector( &work->start_trg_dir, &work->homing_target, &work->pos );
		work->state++ ;
	  case 1:/* 通常ホーミング状態 */
		if ( homing_enable ){/* ホーミング処理 */
			FMATRIX		inv_mat, rot_mat ;
			FVECTOR		vec ;
			float		len ;
			GTE_InversMatrix( &inv_mat, &work->world );
			GTE_LoadMatrix( &inv_mat );
			GTE_PutVector1( &vec, &work->homing_target );
			len = DG_SQRT( GTE_InnerProduct( &vec, &vec ) );
			GTE_ScaleVector( &vec, &vec, 1.0f / len );

			{/* ホーミング先までの角度を求める */
				FVECTOR	rot ;
				float	angle ;
				/*  */
				angle = atan2f( DG_SQRT( vec.vx * vec.vx + vec.vy * vec.vy ), vec.vz );
				if ( angle > max_angle ) angle = max_angle ;
				GTE_OuterProduct( &rot, &z_vec, &vec );
				rot.vw = angle ;
				//GTE_InterVector( &rot, &rot, &work->old_correct_axis, 0.9f ); work->old_correct_axis = rot ;
				//angle = rot.vw ;
				GTE_Normalize( &rot, &rot );
				GTE_MakeRotateAxis( &rot_mat, &rot, angle );
				//printf("angle %f\n", angle );
			}
			GTE_MulMatrix( &work->world, &work->world, &rot_mat );
		}
		{/* ホーミング終了チェック */
			FVECTOR		tmp_vec ;
			GTE_SubVector( &tmp_vec, &work->homing_target, &work->pos );
			if ( GTE_InnerProduct( &tmp_vec, &work->start_trg_dir ) < 0.0f ){
				/* 初期のホーミング方向と現在のホーミング方向が反対なのでホーミングを終了させる */
				work->state++ ;
			}
		}
		break ;
	  case 2:/* ホーミング終了状態 */
		break ;
	}

	/* 加速度調整 */
	work->a_speed = DEFAULT_ACCELERATION ;

	/* 加速度生成 */
	GTE_ZeroVector( &speed_vec );
	speed_vec.vz = work->a_speed ;
	GTE_LoadMatrix( &work->world );
	GTE_RotVector1( &work->acceleration, &speed_vec );
	//printf("accel %f %f %f\n", work->acceleration.vx, work->acceleration.vy, work->acceleration.vz );
	/* 速度生成 */
	GTE_AddVector( &work->velocity, &work->velocity, &work->acceleration );

	{/* 速度調整 */
		float	v, scale ;
		v = DG_SQRT( GTE_InnerProduct( &work->velocity, &work->velocity ) );
		if ( v > MAX_VELOCITY ){
			scale = MAX_VELOCITY / v ;
		} else {
			scale = 1.0f ;
		}
		GTE_ScaleVector( &work->velocity, &work->velocity, scale );
	}
	//printf("velo %f %f %f\n", work->velocity.vx, work->velocity.vy, work->velocity.vz );

	/* 座標更新 */
	old_pos = work->pos ;
	GTE_AddVector( &work->pos, &work->pos, &work->velocity );
	work->pos.vw = 1.0f ;
	*(FVECTOR*)work->world.m[3] = work->pos ;
	if ( !( work->type & 2 ) ){/* ミサイル回転処理 */
		FMATRIX		mat ;
		GTE_MakeRotate( &mat, 0, 0, 1, GTE_PS2RAD(380/2) );
		GTE_MulMatrix( &work->world, &work->world, &mat );
	}

	/* ＲＡＹサーバーにミサイル位置を通知 */
	if ( work->type & 2 ){
		RAYSERVER_SetMissile( &work->pos );
	}

	/* 防御チェック（あたり判定） */
	if ( work->target_damage_flag ){
		if ( work->target_damage_flag == 1 ){
			NewBlast( &work->pos, BOTH_SIDE, 2000, 3000, BLAST_DAMAGE, 0, blast_type );
			//NewBombEffect( &work->pos, 2 );
		} else {
			/* ブレードで切られた場合には専用エフェクト */
			extern void *NewPDRayMissileBreak( FMATRIX *world, FVECTOR *vvec );
			NewPDRayMissileBreak( &work->world, &work->velocity );
		}
		if ( work->smoke_work != NULL ){
			GV_CallChildSignalFunc( work, SIGNAL_RAYMISSILESMOKE_STOP, 0 );
			GV_DestroyChild( work->actor.child );/* シグナルを送った後は親子関係を切り離しておく */
		}
		GV_DestroyActor( work );
		return ;
	}
	GM_ClearTargetDamage( &work->defense_target );
	GM_MoveTarget2( &work->defense_target, &work->world );

	/* 移動チェック（あたり判定） */
	work->hzx_id = GM_GetMap( work->map )->hzx_group ;
	if ( HZX_OnlineHazardCheck( work->hzx_id, &old_pos, &work->pos
							   , HZX_CHK_ALL, CHECK_SEG_FLAG, CHECK_FLOOR_FLAG ) ){
		HZX_GetOnlinePoint( &work->pos );
		/* あたったのが床であれば */
		if ( HZX_GetOnlineHazardType() == 2 ){
			//HZX_GetOnlineHazardFloorType();
			if ( !( HZX_GetOnlineHazardAtr() & HZX_FLOOR_DYNAMIC ) ){
				extern void *NewVRFloorBreak( FMATRIX *parent, float scale );
				extern void *NewVRFloorBreakWide( FMATRIX *parent, float scale );
				FMATRIX		mat ;
				GTE_UnitMatrix( &mat );
				*(FVECTOR*)mat.m[3] = work->pos ;
				NewVRFloorBreakWide( &mat, 1.5f );
				NewVRFloorBreakWide( &mat, 1.5f );
			}
#if 1
			if ( !( HZX_GetOnlineHazardAtr() & HZX_FLOOR_DYNAMIC ) ){
				extern void *NewDummyEarthquakeShock( FMATRIX *parent, int in_size, int out_size, int height );
				extern void *NewFogWave( FMATRIX *parent, int size, int speed, int height );
				FMATRIX	mat ;
				GTE_UnitMatrix( &mat );
				*(FVECTOR*)mat.m[3] = work->pos ;
				//NewDummyEarthquakeShock( &mat, 2000, 4000, 1000 );
				NewFogWave( &mat, 1000, 200, 300 );
			}
#endif
		}
		NewBlast( &work->pos, BOTH_SIDE, 2000, 3000, BLAST_DAMAGE, 0, blast_type );
		//NewBombEffect( &work->pos, 2 );
		if ( work->smoke_work != NULL ){
			GV_CallChildSignalFunc( work, SIGNAL_RAYMISSILESMOKE_STOP, 0 );
			GV_DestroyChild( work->actor.child );/* シグナルを送った後は親子関係を切り離しておく */
		}
		GV_DestroyActor( work );
		return ;
	}

#if 1
	/* ターゲット関連処理（あたり判定２） */
	GM_SetTargetWeaponType( &work->target, I64(1) << blast_type ) ;
	GM_MoveTarget( &work->target, &work->pos );
	GM_PutTarget( &work->target );
	if ( work->target.damaged & TARGET_POWER ){
		NewBlast( &work->pos, BOTH_SIDE, 2000, 3000, BLAST_DAMAGE, 0, blast_type );
		//NewBombEffect( &work->pos, 2 );
		if ( work->smoke_work != NULL ){
			GV_CallChildSignalFunc( work, SIGNAL_RAYMISSILESMOKE_STOP, 0 );
			GV_DestroyChild( work->actor.child );/* シグナルを送った後は親子関係を切り離しておく */
		}
		GV_DestroyActor( work );
		return ;
	}
#endif

	//printf("pos %f %f %f\n", work->pos.vx, work->pos.vy, work->pos.vz );
	//AN_Test_Eye2( &work->pos, 2 );
	if ( work->objs != NULL ){
#if 0
		FMATRIX		mat ;
		//GTE_MakeRotate( &mat, 1, 0, 0, GTE_PI );
		GTE_MakeRotate( &mat, 1, 0, 0, GTE_PS2RAD( MODEL_ROT_CORRECT ) );
		GTE_MulMatrix( &work->objs->world, &work->world, &mat );
#else
		work->objs->world = work->world ;
#endif
		/* ライトマトリクスの取得 */
		//DG_GetLightMatrix( &work->pos, work->light );
	}
	//DG_SwitchBuffPrim2( work->prim );

	/* ＳＥ */
	switch ( work->sound_phase ){
	  case 0:
		if ( work->sound_count > 30 * 5 ){
			int		se_code ;
			se_code = ( work->type & 1 ) ? SD_E_LEGMISE1 : SD_E_LEGMISE2 ;
			GM_SeSetMode( se_code, &work->pos, GM_SEMODE_BOMB ) ;
			work->sound_phase++ ;
		}
		break ;
	  case 1:
		if ( work->type & 2 ){
			static	int se_count = 0 ;
			if ( ( work->velocity.vy < 0.0f ) &&
				( ( work->velocity.vy * 350 / TIME_BASE + work->pos.vy ) < PLAYER_LEVEL ) ){
				int		se_code ;
				se_code = ( se_count & 1 ) ? SD_E_LEGMISE3 : SD_E_LEGMISE4 ;
				GM_SeSetMode( se_code, &work->pos, GM_SEMODE_BOMB ) ;
				se_count++ ;
				work->sound_phase++ ;
			}
		}
		break ;	
	}
	work->sound_count += TIME_BASE ;

	work->count += TIME_BASE ;
	if ( work->count > 2000 ){
		NewBlast( &work->pos, BOTH_SIDE, 2000, 3000, BLAST_DAMAGE, 0, blast_type );
		if ( work->smoke_work != NULL ){
			GV_CallChildSignalFunc( work, SIGNAL_RAYMISSILESMOKE_STOP, 0 );
			GV_DestroyChild( work->actor.child );/* シグナルを送った後は親子関係を切り離しておく */
		}
		GV_DestroyActor( work );
	}
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GM_FreeTarget( &work->defense_target );
	if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	//if ( work->prim != NULL ){
	//	GM_FreePrim2( work->prim );
	//}
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *pos, int type )
{
	static FVECTOR		first_velocity = {0,0,0,1};
	int		model_id ;

   first_velocity.z = FIRST_VELOCITY;


	work->map = GM_CurrentMap ;
	work->type = type ;
	work->world = *pos ;
	work->pos = *(FVECTOR*)pos->m[3] ;

	/* 初期設定 */
	work->a_speed = 0 ;
	GTE_InitGTE();
	GTE_ZeroVector( &work->acceleration );
	GTE_LoadMatrix( pos );
	GTE_RotVector1( &work->velocity, &first_velocity );

	if ( ( type & 2 ) == 0 ){
		/* タイプ１ */
		model_id = MODEL_NAME0 ;
		work->max_velocity = MAX_VELOCITY0A ;
	} else {
		/* タイプ２ */
		model_id = MODEL_NAME1 ;
		work->max_velocity = MAX_VELOCITY1 ;
		{
			FMATRIX		mat ;
			GTE_MakeRotate( &mat, RND(256), RND(256), RND(256), GTE_PS2RAD( 60 ) );
			GTE_MulMatrix( &work->world, &work->world, &mat );
		}
	}

	{/* モデル初期化 */
		DG_DEF		*def ;
		def = GV_GetCache( GV_CacheID( model_id, 'k' ) );
		if ( def != NULL ){
			work->objs = DG_MakeObjs( def, MODEL_FLAG, 0 );
			DG_QueueObjs( work->objs );
			GM_GroupObjs( work->objs, GM_CurrentMap );
			//DG_SetLightMatrix( work->objs, work->light );
		}
	}
#if 0
	{/* プリミティブ初期化 */
		DG_PRIM2		*prim ;
		DG_PRIM2_UVRGBWH	*uvrgbwh ;
		DG_TEX			*tex ;
		FVECTOR			*pos ;
		int				i, j, k, r, g, b ;
		int		u0, v0, u1, v1 ;
		float	size_offset ;

		tex = DG_GetTexture( TEXTURE0 );
		work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
		DG_ConfigPrim2Tex( prim, tex );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
		prim->root = &work->world ;
		for ( k = 0 ; k < 2 ; k++ ){
			pos = prim->pos[ k ];
			uvrgbwh = prim->uvrgb[ k ];
			u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
			v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
			u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
			v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;
			r = 32 + k * 8 ;
			g = 32 + k * 8 ;
			b = 64 + k * 8 ;
			size_offset = 70 + k * 5 ;
			for ( i = 0 ; i < N_PRIMS ; i++ ){
				for ( j = 0 ; j < N_VERTS ; j++ ){
					GTE_SetVector( pos, 0, 0, - j * size_offset + FIRE_OFFSET, 1.0f );
					UVRGBWH_WH( uvrgbwh, j * 20 + 50, j * 20 + 50 );
					UVRGBWH_RGBA( uvrgbwh, r, g, b, 128 - j*4 );
					UVRGBWH_UV( uvrgbwh, u0, v0, u1, v1 );
					pos++ ;
					uvrgbwh++ ;
				}
			}
		}
	}
#endif

	/* ホーミング座標設定 */
	work->homing_target = GM_PlayerPosition ;
	//work->homing_target.vy = PLAYER_LEVEL ;
	work->old_player_pos = GM_PlayerPosition ;

	{/* あたり判定設定 */
		FVECTOR		size ;
		int			class ;

#if 1
		/* ロックオン用ターゲット初期化 */
		//class = TARGET_POWER|TARGET_CHILD_ALWAYS|TARGET_DEFENSE|TARGET_ROTATE ;
		class = TARGET_POWER|TARGET_OFFENSE/*|TARGET_LOCKON*/ ;
		size.vx = TARGET_SIZE_X ;
		size.vy = TARGET_SIZE_Y ;
		size.vz = TARGET_SIZE_Z ;
		//GM_SetTarget( &work->target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetTarget( &work->target, class, work->map, PLAYER_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->target,
						  &work->power_target, POWER_ONCE, 1, 0, ATTACK_DAMAGE, &work->target_force );
		GM_SetTargetWeaponType( &work->target, WP_C4BOMB ) ;
		GM_MoveTarget( &work->target, &work->pos );
		//GM_PutTarget( &work->target );
		//GM_SetTargetCallBack( &work->target, (TARGET_CALLBACK)TargetCallback, work );
#endif

		class = TARGET_DEFENSE|TARGET_ROTATE ;
		size.vx = 1000.0f ;
		size.vy = 1000.0f ;
		size.vz = 1000.0f ;
		GM_SetTarget( &work->defense_target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->defense_target, &work->defense_power, POWER_ONCE, 255, 0, 0, &DG_ZeroVector );
		GM_MoveTarget2( &work->defense_target, &work->world );
		GM_SetTargetCallBack( &work->defense_target, (TARGET_CALLBACK)TargetCallback, work );
		GM_PutTarget( &work->defense_target );
	}

	{/* 場外攻撃用ミサイル時の最大補正角度を求める */
		FVECTOR		tmp_vec ;
		float		len ;
		GTE_AddVector( &tmp_vec, &work->pos, &GM_PlayerPosition );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, 0.5f );
		GTE_SubVector( &tmp_vec, &work->pos, &tmp_vec );
		tmp_vec.vy = 0.0f ;
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		work->max_angle = atan2f( MAX_VELOCITY, len * 0.7 );
		//printf("%f %f %f : %f %f\n", tmp_vec.vx, tmp_vec.vz, len, work->max_angle, MAX_TURN );
	}

	work->smoke_work = NewRAYMissileSmoke( &work->world );
	if ( work->smoke_work != NULL ) GV_SetActorChild( work, work->smoke_work );

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewPDRayWeaponMissile( FMATRIX *pos, int type )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

