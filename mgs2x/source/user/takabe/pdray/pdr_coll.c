//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_coll.c
	量産型ＲＡＹあたり判定処理ルーチン

	2001/04/11 K.Takabe
	$Id: pdr_coll.c,v 1.1.1.3 2002/11/19 11:51:23 Yoshizawa1 Exp $

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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"

#ifndef PSX2
#undef DEBUG_MODE
#endif

#include	"../other/vec_util.h"

#include "pdray.h"
#include "r_common.h"

/* ---------------------------------------------------------------- */
#define CHECK_FLOOR_FLAG	(HZX_FLOOR_ALL)
#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_PLAYER|HZX_FLOOR_NO_ENEMY|\
HZX_FLOOR_NO_MISSILE|HZX_FLOOR_NO_C4|HZX_FLOOR_NO_CLAYMORE|HZX_FLOOR_NO_SPRAY)

#define HEAD_DAMAGE_STATUS (\
							RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|RAY_STATUS_ATTACK4|\
							RAY_STATUS_MOVE1|RAY_STATUS_MOVE2|RAY_STATUS_ROAR|RAY_STATUS_GUARD|RAY_STATUS_DRINK|\
							RAY_STATUS_DAMAGE2|RAY_STATUS_DAMAGE3\
)
#define RLEG_DAMAGE_STATUS (\
							RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|RAY_STATUS_ATTACK4|\
							RAY_STATUS_MOVE1|RAY_STATUS_MOVE2|RAY_STATUS_ROAR|RAY_STATUS_GUARD|RAY_STATUS_DRINK\
)
#define LLEG_DAMAGE_STATUS (\
							RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|RAY_STATUS_ATTACK4|\
							RAY_STATUS_MOVE1|RAY_STATUS_MOVE2|RAY_STATUS_ROAR|RAY_STATUS_GUARD|RAY_STATUS_DRINK\
)

/* ---------------------------------------------------------------- */
enum {
	COLL_BODY			= 0x00000001,
	COLL_HEAD			= 0x00000002,
	COLL_MOUTH			= 0x00000004,
	COLL_RLEG			= 0x00000008,
	COLL_LLEG			= 0x00000010,
	COLL_RARM			= 0x00000020,
	COLL_LARM			= 0x00000040,
	/* あたり判定タイプ */
	COLL_TYPE_STG		= 0x00010000,
	COLL_TYPE_RGB		= 0x00020000,
};
/* ---------------------------------------------------------------- */
typedef ALIGN16_DECL(struct) {
	int			joint ;		/* 付随関節 */
	int			type ;		/* タイプ */
	SVECTOR		rot ;		/* 回転補正量（初期化時にマトリクスに展開される） */
	FVECTOR		min ;		/* 判定バウンディングボックス */
	FVECTOR		max ;		/* 判定バウンディングボックス */
	FMATRIX		mat ;		/* 関節からのバウンディングボックス位置補正マトリクス */
} COLLISION_PARAM  ;
/* 当たり判定テーブル */
#define NULL_MAT {{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}}
static COLLISION_PARAM	collision_table[PDRAY_HAZARD_JOINTS+1] = {
	{0, COLL_BODY, {360,0,0}, {-1400,-2000,-1800}, {1400,3000,1400}, NULL_MAT },/* body */
	{1, COLL_BODY, {491,0,0}, {-1400,200,-2040}, {1400,4190,2200}, NULL_MAT },/* body */
	{2, COLL_BODY, {656,0,0}, {-1850,-550,-2300}, {1854,3500,1260}, NULL_MAT },/* body */
	{3, COLL_RARM, {0,0,0}, {-4600,-1000,-1000}, {100,900,1000}, NULL_MAT },/* right arm1 */
//	{5, COLL_RARM, {0,0,0}, {-10000,-600,-1500}, {-500,1600,1500}, NULL_MAT },/* right arm2 */
	{5, COLL_RARM, {0,0,0}, {-8500,-600,-1500}, {-500,1600,1500}, NULL_MAT },/* right arm2 */
	{7, COLL_RARM, {0,0,0}, {-1600,-700,-1000}, {700,1000,1000}, NULL_MAT },/* right arm3 */
	{8, COLL_LARM, {0,0,0}, {-100,-1000,-1000}, {4600,900,1000}, NULL_MAT },/* left arm1 */
//	{10, COLL_LARM, {0,0,0}, {500,-600,-1500}, {10000,1600,1500}, NULL_MAT },/* left arm2 */
	{10, COLL_LARM, {0,0,0}, {500,-600,-1500}, {8500,1600,1500}, NULL_MAT },/* left arm2 */
	{12, COLL_LARM, {0,0,0}, {-700,-700,-1000}, {1600,1000,1000}, NULL_MAT },/* left arm3 */
	{14, COLL_BODY, {0,0,0}, {-1000,-900,-2300}, {1000,1500,2600}, NULL_MAT },/* head1 */
	{16, COLL_BODY, {0,0,0}, {-900,-200,-4000}, {1000,1300,900}, NULL_MAT },/* head2 */
//	{14, COLL_HEAD, {0,0,0}, {-800,500,1400}, {800,1600,3200}, NULL_MAT },/* head3(target) */
	{14, COLL_HEAD, {0,0,0}, {-1000,-1400,500}, {1000,1700,3600}, NULL_MAT },/* head3(target) *//* 頭全体をあたり判定に */
	{14, COLL_MOUTH, {0,0,0}, {-800,-1400,1400}, {800,500,3200}, NULL_MAT },/* head4(target) */
	{22, COLL_RLEG, {200,0,0}, {-2383,-200,-1200}, {-100,2900,7000}, NULL_MAT },/* rihgt leg1 */
	{24, COLL_RLEG, {700,0,0}, {-950,-1000,-1200}, {900,1400,7000}, NULL_MAT },/* rihgt leg2 */
	{28, COLL_LLEG, {200,0,0}, {100,-200,-1200}, {2383,2900,7000}, NULL_MAT },/* rihgt leg1 */
	{30, COLL_LLEG, {700,0,0}, {-900,-1000,-1200}, {950,1400,7000}, NULL_MAT },/* rihgt leg2 */
	{-1, 0, {0,0,0}, {0,0,0}, {0,0,0}, NULL_MAT },/* end */
};
#undef NULL_MAT
//static FVECTOR	lock_on_offset = {(-800+800)/2,(0+1600)/2,(1600+3200)/2} ;
static FVECTOR	lock_on_offset = {(-800+800)/2,0,(1600+3200)/2} ;
#ifdef DEBUG_MODE
#define COLLISION_EDIT
#endif
#ifdef COLLISION_EDIT
static int PDRAY_DebugParamBuffer[ 32 ] ;
#endif
/* ---------------------------------------------------------------- */
extern void TAKABE_OozeBloodAdd( void *_work, int joint, FVECTOR *pos, float range );
/* ---------------------------------------------------------------- */
static void ExecProc( int proc_id, int param0 )
{
	GCL_ARGS	arg ;
	int			data[4] ;

	if ( proc_id == 0 ) return ;
	arg.argc = 1 ;
	arg.argv = data ;
	data[0] = param0 ;
	GCL_ExecProc( proc_id, &arg );
}
/* ---------------------------------------------------------------- */
/* あたり判定パラメータの回転ベクトルをマトリクスに変換する */
static void InitCollision( Work *work )
{
	int		i ;

   // BP_WARNING - This looks like a memory overwrite, but it's not
   //              Even though collision_table is smaller than PDRAY_MAX_JOINTS,
   //              the last real entry is -1 so it will punt in time.
   //              The middle of this for loop should be ( collision_table[i].joint != -1 )
	for ( i = 0 ; i < PDRAY_MAX_JOINTS ; i++ ){
		if ( collision_table[ i ].joint == -1 ) break ;
		DG_SetPos( &DG_UnitMatrix );
		DG_RotatePos( &collision_table[ i ].rot );
		DG_GetPos( &collision_table[ i ].mat );
	}
}
/* ---------------------------------------------------------------- */
/* ターゲット用コールバック */
#if 0
static void TargetCallback( TARGET *offence, TARGET *defence, Work *work )
{
	POWER_TARGET	*power ;

	if ( work->invinsible_time ) return ;
	if ( offence->power == NULL ) return ;
	power = offence->power ;
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
		if ( offence->weapon_type != WP_Claymore ){
			work->damage_pos = offence->center ;
			work->invinsible_time = 60 ;
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

static void DummyTargetCallback( TARGET *offence, TARGET *defence, Work *work )
{
	POWER_TARGET	*power ;

	if ( work->invinsible_time ) return ;
	if ( offence->power == NULL ) return ;
	power = offence->power ;
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
	  case POWER_ONCE:
	  case POWER_DECREASE:
	  case POWER_THRESHOLD:
	  case POWER_CONST:
	  default:
		break ;
	}
}
#endif
static void EarthquakeCallback( TARGET *offence, TARGET *defence, Work *work )
{
	if ( defence != GM_PlayerTarget ) return ;
	work->player_tumble_wait_time = PLAYER_TUMBLE_WAIT_TIME ;
}

/* ---------------------------------------------------------------- */
static void MoveDynamicFloor( HZX_D_FLOOR *flr, FVECTOR *pos )
{
	FVECTOR		norm, tmp_vec1, tmp_vec2, bound_max, bound_min ;
	IVECTOR		inorm ;
	float		len ;
	/* 法線算出 */
	GTE_SubVector( &tmp_vec1, &pos[1], &pos[0] );
	GTE_SubVector( &tmp_vec2, &pos[2], &pos[1] );
	GTE_OuterProduct( &norm, &tmp_vec1, &tmp_vec2 );
	len = DG_RSQRT( GTE_InnerProduct( &norm, &norm ) );
	GTE_ScaleVector( &norm, &norm, len * 32000.0f );
	GTE_FTOI0Vector( &inorm, &norm );
	/* 頂点＆法線設定 */
	GTE_FTOI0Vector( &flr->p1, &pos[0] );
	GTE_FTOI0Vector( &flr->p2, &pos[1] );
	GTE_FTOI0Vector( &flr->p3, &pos[2] );
	GTE_FTOI0Vector( &flr->p4, &pos[3] );
	flr->p1.vw = inorm.vx ;
	flr->p2.vw = inorm.vy ;
	flr->p3.vw = inorm.vz ;
	flr->atr &= ~( HZX_FLOOR_RECT | HZX_FLOOR_FLAT | HZX_FLOOR_SKIP );
	/* バウンディング設定 */
#ifdef PSX2
	bound_min.vx = DG_MIN( DG_MIN( pos[0].vx, pos[1].vx ), DG_MIN( pos[2].vx, pos[3].vx ) );
	bound_min.vy = DG_MIN( DG_MIN( pos[0].vy, pos[1].vy ), DG_MIN( pos[2].vy, pos[3].vy ) );
	bound_min.vz = DG_MIN( DG_MIN( pos[0].vz, pos[1].vz ), DG_MIN( pos[2].vz, pos[3].vz ) );
	bound_max.vx = DG_MAX( DG_MAX( pos[0].vx, pos[1].vx ), DG_MAX( pos[2].vx, pos[3].vx ) );
	bound_max.vy = DG_MAX( DG_MAX( pos[0].vy, pos[1].vy ), DG_MAX( pos[2].vy, pos[3].vy ) );
	bound_max.vz = DG_MAX( DG_MAX( pos[0].vz, pos[1].vz ), DG_MAX( pos[2].vz, pos[3].vz ) );
	GTE_FTOI0Vector( &flr->b1, &bound_min );
	GTE_FTOI0Vector( &flr->b2, &bound_max );
#else
	flr->b1.vx = (int)DG_MIN( DG_MIN( pos[0].vx, pos[1].vx ), DG_MIN( pos[2].vx, pos[3].vx ) );
	flr->b1.vy = (int)DG_MIN( DG_MIN( pos[0].vy, pos[1].vy ), DG_MIN( pos[2].vy, pos[3].vy ) );
	flr->b1.vz = (int)DG_MIN( DG_MIN( pos[0].vz, pos[1].vz ), DG_MIN( pos[2].vz, pos[3].vz ) );
	flr->b2.vx = (int)DG_MAX( DG_MAX( pos[0].vx, pos[1].vx ), DG_MAX( pos[2].vx, pos[3].vx ) );
	flr->b2.vy = (int)DG_MAX( DG_MAX( pos[0].vy, pos[1].vy ), DG_MAX( pos[2].vy, pos[3].vy ) );
	flr->b2.vz = (int)DG_MAX( DG_MAX( pos[0].vz, pos[1].vz ), DG_MAX( pos[2].vz, pos[3].vz ) );
#endif
	flr->tag[ 5 ] = 0 ;/* 常に非正方形とみなす */
	flr->tag[ 6 ] = 0 ;/* 常に非水平とみなす */
	flr->tag[ 8 ] = flr->b1.vy ;
	flr->tag[ 9 ] = flr->b2.vy ;
	/* 跳弾当たりだけなのでコールバック関連の処理は行わない */
}
/* バウンディングボックスに対応するハザードを設定する */
static void SetHazardBox( HZX_D_FLOOR **d_floors, FMATRIX *mat, FVECTOR *bound_max, FVECTOR *bound_min )
{
	static short	poly_list[6][4] = {{0,1,3,2},{0,2,6,4},{2,3,7,6},{3,1,5,7},{1,0,4,5},{4,6,7,5}};
	FVECTOR		verts[8], *v ;
	int			i ;

	/* バウンディングボックスを頂点に変換 */
	v = verts ;
	for ( i = 0 ; i < 8 ; i++ ){
		v->vx = ( i & 1 ) ? bound_max->vx : bound_min->vx ;
		v->vy = ( i & 2 ) ? bound_max->vy : bound_min->vy ;
		v->vz = ( i & 4 ) ? bound_max->vz : bound_min->vz ;
		v->vw = 1.0f ;
		v++ ;
	}
	GTE_LoadMatrix( mat );
	GTE_PutVector( verts, verts, 8 );
	/* ６枚のダイナミックフロアを設定 */
	for ( i = 0 ; i < 6 ; i++ ){
#if 0
		FVECTOR		pos[4] ;
		IVECTOR		ipos[4] ;
		pos[0] = verts[ poly_list[i][0] ] ;
		pos[1] = verts[ poly_list[i][1] ] ;
		pos[2] = verts[ poly_list[i][2] ] ;
		pos[3] = verts[ poly_list[i][3] ] ;
		ipos[0].vx = DG_FTOI( pos[0].vx );
		ipos[0].vy = DG_FTOI( pos[0].vy );
		ipos[0].vz = DG_FTOI( pos[0].vz );
		ipos[1].vx = DG_FTOI( pos[1].vx );
		ipos[1].vy = DG_FTOI( pos[1].vy );
		ipos[1].vz = DG_FTOI( pos[1].vz );
		ipos[2].vx = DG_FTOI( pos[2].vx );
		ipos[2].vy = DG_FTOI( pos[2].vy );
		ipos[2].vz = DG_FTOI( pos[2].vz );
		ipos[3].vx = DG_FTOI( pos[3].vx );
		ipos[3].vy = DG_FTOI( pos[3].vy );
		ipos[3].vz = DG_FTOI( pos[3].vz );
		HZX_MoveDynamicFloor( *d_floors, &ipos[0], &ipos[1], &ipos[2], &ipos[3] );
#else
		FVECTOR		pos[4] ;
		pos[0] = verts[ poly_list[i][0] ] ;
		pos[1] = verts[ poly_list[i][1] ] ;
		pos[2] = verts[ poly_list[i][2] ] ;
		pos[3] = verts[ poly_list[i][3] ] ;
		MoveDynamicFloor( *d_floors, pos );
#endif
		d_floors++ ;
	}
	
}

/* ---------------------------------------------------------------- */
static void SetTargetMode( Work *work, int lock_mode, int lock_mode2, int def_mode )
{
	if ( lock_mode ){
		work->lock_on_target.class |= TARGET_LOCKON ;
	} else {
		work->lock_on_target.class &= ~TARGET_LOCKON ;
	}
	if ( lock_mode2 ){
		work->lock_on_target_r_leg.class |= TARGET_LOCKON ;
		work->lock_on_target_l_leg.class |= TARGET_LOCKON ;
	} else {
		work->lock_on_target_r_leg.class &= ~TARGET_LOCKON ;
		work->lock_on_target_l_leg.class &= ~TARGET_LOCKON ;
	}
	if ( def_mode ){
		work->collision_dummy_target.class |= TARGET_POWER ;
	} else {
		work->collision_dummy_target.class &= ~TARGET_POWER ;
	}
}
/* ---------------------------------------------------------------- */
/* あたり判定と座標をチェックする */
static int CalcCollision( Work *work, FVECTOR *check_pos )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	FVECTOR		vec ;
	float		len ;
	int			res = 0 ;

	ctrl = &work->control ;
	body = &work->object ;

	GTE_SubVector( &vec, (FVECTOR*)body->objs->world.m[3], check_pos );
	len = GTE_InnerProduct( &vec, &vec );

	if ( len < work->collision_check_range ){
		DG_OBJS		*objs = body->objs ;
		COLLISION_PARAM		*c_param ;
		FMATRIX		target_mat, target_inv_mat ;
		FVECTOR		pos, trans_pos ;
		int			i ;

		pos = *check_pos ;
		pos.vw = 1.0f ;
		c_param = collision_table ;
		for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++, c_param++ ){
			if ( c_param->joint == -1 ) break ;
			GTE_MulMatrix( &target_mat, &objs->objs[ c_param->joint ].world, &c_param->mat );
			GTE_InversMatrix( &target_inv_mat, &target_mat );
			GTE_ApplyMatrix( &trans_pos, &target_inv_mat, &pos );
			if ( c_param->min.vx < trans_pos.vx && c_param->max.vx > trans_pos.vx &&
				c_param->min.vy < trans_pos.vy && c_param->max.vy > trans_pos.vy &&
				c_param->min.vz < trans_pos.vz && c_param->max.vz > trans_pos.vz ){
				res |= c_param->type ;
				work->oozeblood_hit_joint = c_param->joint ;
				{/* 出血用パラメータを求める */
					static FVECTOR	y_base = {0,1,0,1};
					FMATRIX		mat ;
					FVECTOR		tmp_vec, x_vec, y_vec, z_vec ;
					float		r_len ;
					GTE_AddVector( &tmp_vec, &c_param->max, &c_param->min );
					GTE_ScaleVector( &tmp_vec, &tmp_vec, 0.5f );
					GTE_SubVector( &z_vec, &trans_pos, &tmp_vec );
					r_len = DG_RSQRT( GTE_InnerProduct( &z_vec, &z_vec ) );
					GTE_ScaleVector( &z_vec, &z_vec, r_len );
					GTE_OuterProduct( &x_vec, &z_vec, &y_base );
					GTE_OuterProduct( &y_vec, &z_vec, &x_vec );
					x_vec.vw = 0.0f ;
					y_vec.vw = 0.0f ;
					z_vec.vw = 0.0f ;
					*(FVECTOR*)mat.m[0] = x_vec ;
					*(FVECTOR*)mat.m[1] = y_vec ;
					*(FVECTOR*)mat.m[2] = z_vec ;
					*(FVECTOR*)mat.m[3] = trans_pos ;
					GTE_MulMatrix( &mat, &c_param->mat, &mat );
					work->blood_joint = c_param->joint ;
					work->blood_offset = mat ;
					//work->blood_flag = 1 ;
				}
			}
			//NewBoundingBoxView_1( &target_mat, &c_param->min, &c_param->max, 0x000080ff );
		}
		return ( res );
	}
	return ( 0 );
}

/* ---------------------------------------------------------------- */
void PDRAY_InitCollision( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	{/* あたり判定設定 */
		FVECTOR		size ;
		int			class ;
		TARGET		*tgt ;

		/* ロックオン用ターゲット初期化 */
		//class = TARGET_POWER|TARGET_CHILD_ALWAYS|TARGET_DEFENSE|TARGET_ROTATE ;
		//class = TARGET_POWER|TARGET_DEFENSE|TARGET_LOCKON ;
		//class = TARGET_LOCKON ;
		class = TARGET_LOCKON_AUTOSIZE ;
		size.vx = TARGET_HEAD_SIZE_X ;
		size.vy = TARGET_HEAD_SIZE_Y ;
		size.vz = TARGET_HEAD_SIZE_Z ;
		GM_SetTarget( &work->lock_on_target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_MoveTarget2( &work->lock_on_target, &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
		GM_PutTarget( &work->lock_on_target );
		GM_SetTargetName( &work->lock_on_target, work->name );	/* ターゲットに名前をつけておく */

		tgt = &work->lock_on_target_r_leg ;
		GM_SetTarget( tgt, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetTargetName( tgt, work->name );	/* ターゲットに名前をつけておく */
		GM_PutTarget( tgt );
		tgt = &work->lock_on_target_l_leg ;
		GM_SetTarget( tgt, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetTargetName( tgt, work->name );	/* ターゲットに名前をつけておく */
		GM_PutTarget( tgt );

		/* スティンガーミサイル爆破用ダミーターゲット */
		class = TARGET_POWER|TARGET_DEFENSE ;
		size.vx = TARGET_SIZE_X*V_SCALE ;
		size.vy = TARGET_SIZE_Y*V_SCALE ;
		size.vz = TARGET_SIZE_Z*V_SCALE ;
		GM_SetTarget( &work->collision_dummy_target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->collision_dummy_target,
						  &work->power_target, POWER_CONST, 1, 0, 0, &work->target_force );
		//GM_TargetSetSkip( &work->collision_dummy_target );	/* 当たり判定を無効に */
		GM_MoveTarget2( &work->collision_dummy_target, &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
		GM_PutTarget( &work->collision_dummy_target );
		//GM_SetTargetCallBack( &work->collision_dummy_target, (TARGET_CALLBACK)TargetCallback, work );

		/* 地震用ターゲット初期化 */
		//class = TARGET_POWER|TARGET_OFFENSE ;
		class = TARGET_OFFENSE ;
		size.vx = 1000 ;
		size.vy = 1000 ;
		size.vz = 1000 ;
		GM_SetTarget( &work->earthquake_target, class, work->map, PLAYER_SIDE, &size, &DG_ZeroVector );
		GM_SetTargetWeaponType( &work->earthquake_target, WP_STEAM | WP_NOBLOOD );
		GM_SetPowerTarget( &work->earthquake_target,
						  &work->earthquake_power_target, POWER_EXPLODE, 255, 10, 0, &work->target_force );
		GM_SetTargetCallBack( &work->earthquake_target, (TARGET_CALLBACK)EarthquakeCallback, work );
		//GM_MoveTarget2( &work->earthquake_target, &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
		//GM_PutTarget( &work->earthquake_target );

		/* 足攻撃判定初期化 */
		size.vx = 500 ; 
		size.vy = 1500 ;
		size.vz = 1000 ;
		/* 右足初期化 */
		GM_SetTarget( &work->right_leg_attack, TARGET_OFFENSE,
					 GM_CurrentMap, PLAYER_SIDE, &size, &DG_ZeroVector );
		//GM_SetTargetWeaponType( &work->right_leg_attack, WP_STAMP );
		GM_SetTargetWeaponType( &work->right_leg_attack, WP_C4BOMB /*| WP_NOGUARD*/ );
		/* 左足初期化 */
		GM_SetTarget( &work->left_leg_attack, TARGET_OFFENSE,
					 GM_CurrentMap, PLAYER_SIDE, &size, &DG_ZeroVector );
		//GM_SetTargetWeaponType( &work->left_leg_attack, WP_STAMP );
		GM_SetTargetWeaponType( &work->left_leg_attack, WP_C4BOMB /*| WP_NOGUARD*/ );
	}

	InitCollision( work );

	/* あたり判定開始距離を求める */
	work->collision_check_range = body->objs->bound_max.vx * body->objs->bound_max.vx ;

	{/* ゲージの初期化 */
		static char	*name_list[] = {
			"RAY-A01E","RAY-A02E","RAY-A03E","RAY-A04E","RAY-A05E",
			"RAY-B01F","RAY-B02F","RAY-B03F","RAY-B04F","RAY-B05F",
			"RAY-C01H","RAY-C02H","RAY-C03H","RAY-C04H","RAY-C05H",
			"RAY-D01G","RAY-D02G","RAY-D03G","RAY-D04G","RAY-D05G",
			"RAY-E01L","RAY-E02L","RAY-E03L","RAY-E04L","RAY-E05L",
		};
		GM_GageSet	*gs ;
		//sprintf( work->disp_name, "RAY-%02d", work->no );
		sprintf( work->disp_name, "%s", name_list[ work->no ] );
		gs = &work->life_gauge ;
		//GM_InitGageSet( gs, work->disp_name, 16, 240, GM_DEFAULT_GAGE_HEIGHT,
		//			   work->life, work->life_max, 0, 30, 2 ) ;
		GM_InitGageSet( gs, work->disp_name, 16, 1, GM_DEFAULT_GAGE_HEIGHT,
					   work->life, work->life_max, 0, 30, 2 ) ;
		GM_SetGageColor( gs, 0,0,0, 40,128,118, 110,190,118, 255,0,0 ) ;
		GM_AppendGageSet( &work->life_gauge ) ;
		GM_InvisibleGage( &work->life_gauge ) ;
	}

	work->hzx_id = GM_GetHzxGroupID( work->map );
#if 0
	/* ダイナミックフロアの作成 */
	for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
		IVECTOR	ipos[4] = {{0,0,0,1},{1,0,0,1},{0,0,1,1},{1,0,1,1}};
		for ( j = 0 ; j < 6 ; j++ ){
			if ( work->d_floors[ i ][ j ] == NULL ){
				work->d_floors[ i ][ j ] = HZX_AddDynamicFloor( work->hzx_id,
															   &ipos[0], &ipos[1], &ipos[2], &ipos[3], 4, FLOOR_FLAG );
			}
		}
	}
#endif

}

void PDRAY_EndCollision( Work *work )
{
	int		i, j ;
	GM_RemoveGageSet( &work->life_gauge ) ;
	GM_FreeTarget( &work->lock_on_target );
	GM_FreeTarget( &work->lock_on_target_r_leg );
	GM_FreeTarget( &work->lock_on_target_l_leg );
	GM_FreeTarget( &work->collision_dummy_target );
	GM_FreeTarget( &work->earthquake_target );

	for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
		for ( j = 0 ; j < 6 ; j++ ){
			if ( work->d_floors[ i ][ j ] ){
				HZX_RemoveDynamicFloor( work->d_floors[ i ][ j ] );
			}
		}
	}
}
/* ---------------------------------------------------------------- */
void PDRAY_ActCollision( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	int		defence_enable = 0 ;
	int		lockon_enable = 1 ;
	int		lockon_enable2 = 1 ;

	ctrl = &work->control ;
	body = &work->object ;

	/* ターゲット処理 */
	if ( work->target_damage_flag ){
		work->target_damage_flag = 0 ;
	}
	/* 無敵時間調整 */
	if ( work->invinsible_time > 0 ){
		if ( ( work->invinsible_time -= TIME_BASE ) < 0 ) work->invinsible_time = 0 ;
	}
	if ( work->mini_invinsible_time > 0 ){
		if ( ( work->mini_invinsible_time -= TIME_BASE ) < 0 ) work->mini_invinsible_time = 0 ;
	}
	GM_ClearTargetDamage( &work->collision_dummy_target );
	GM_TargetSetSkip( &work->collision_dummy_target );	/* 当たり判定を無効に */

	if ( work->active_flag ){
		{/* ロックオンターゲット移動処理 */
			FMATRIX		mat ;
			DG_SetPos( &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
			DG_MovePos( &lock_on_offset );
			DG_GetPos( &mat );
			GM_MoveTarget2( &work->lock_on_target, &mat );
			GM_ClearTargetDamage( &work->lock_on_target );

			DG_SetPos( &body->objs->objs[ PDRAY_JOINT_RIGHT_LEG1 ].world );
			//DG_MovePos( &lock_on_offset );
			DG_GetPos( &mat );
			GM_MoveTarget2( &work->lock_on_target_r_leg, &mat );
			GM_ClearTargetDamage( &work->lock_on_target_r_leg );

			DG_SetPos( &body->objs->objs[ PDRAY_JOINT_LEFT_LEG1 ].world );
			//DG_MovePos( &lock_on_offset );
			DG_GetPos( &mat );
			GM_MoveTarget2( &work->lock_on_target_l_leg, &mat );
			GM_ClearTargetDamage( &work->lock_on_target_l_leg );

			if ( work->on_stage_flag ){
				work->lock_on_target.class |= TARGET_LOCKON_HIGH_PRIO ;
				work->lock_on_target_r_leg.class |= TARGET_LOCKON_HIGH_PRIO ;
				work->lock_on_target_l_leg.class |= TARGET_LOCKON_HIGH_PRIO ;
			} else {
				work->lock_on_target.class &= ~TARGET_LOCKON_HIGH_PRIO ;
				work->lock_on_target_r_leg.class &= ~TARGET_LOCKON_HIGH_PRIO ;
				work->lock_on_target_l_leg.class &= ~TARGET_LOCKON_HIGH_PRIO ;
			}
		}

		/* 各部位とスティンガーのあたり判定処理 */
		if ( work->force_stg_break_flag == 0 ){
			int		res ;
			if ( GM_WeaponAlive & WP_ALIVE_STINGER ){
				res = CalcCollision( work, &GM_StingerPosition ) ;
				if ( res ){
					/* あたり判定にＨＩＴ！ */
					GM_MoveTarget( &work->collision_dummy_target, &GM_StingerPosition );
					GM_TargetResetSkip( &work->collision_dummy_target );	/* 当たり判定を有効に */
					if ( work->oozeblood_hit_joint < 13 || work->oozeblood_hit_joint > 20 ){
						if ( work->blood_wait_time <= 0 ){
							TAKABE_OozeBloodAdd( work->oozeblood_effect_ptr,
												work->oozeblood_hit_joint, &GM_StingerPosition, 2000.0f );
							work->blood_flag = 1 ;				/* 出血の許可 */
							work->blood_wait_time = BLOOD_WAIT_TIME ;
						}
					}
				}
				defence_enable = res ;
			}
		} else {
			/* 強制的にスティンガーミサイルの破壊を行う */
			GM_MoveTarget( &work->collision_dummy_target, &GM_StingerPosition );
			GM_TargetResetSkip( &work->collision_dummy_target );	/* 当たり判定を有効に */
			work->force_stg_break_flag = 0 ;
		}
		{/* 各部位とＲＧＢ６のあたり判定処理 */
			int		i, res ;
			for ( i = 0 ; i < GM_N_RGB6S ; i++ ){
				res = CalcCollision( work, &GM_RGB6Position[ i ] ) ;
				if ( res ){
					/* あたり判定にＨＩＴ！ */
					GM_MoveTarget( &work->collision_dummy_target, &GM_RGB6Position[ i ] );
					GM_TargetResetSkip( &work->collision_dummy_target );	/* 当たり判定を有効に */
					if ( work->oozeblood_hit_joint < 13 || work->oozeblood_hit_joint > 20 ){
						if ( work->blood_wait_time <= 0 ){
							TAKABE_OozeBloodAdd( work->oozeblood_effect_ptr,
												work->oozeblood_hit_joint, &GM_RGB6Position[ i ], 1200.0f );
							work->blood_flag = 1 ;				/* 出血の許可 */
							work->blood_wait_time = BLOOD_WAIT_TIME ;
						}
					}
					res |= COLL_TYPE_RGB ;
				}
				defence_enable |= res ;
			}
		}
		{/* 各部位とニキータのあたり判定処理 */
			if ( GM_WeaponAlive & WP_ALIVE_NIKITA && GM_NikitaAlive[0] != NKT_BLAST ){
				int			res ;
				res = CalcCollision( work, &GM_NikitaPosition[0] ) ;
				if ( res ){
					/* あたり判定にＨＩＴ！ */
					GM_MoveTarget( &work->collision_dummy_target, &GM_NikitaPosition[0] );
					GM_TargetResetSkip( &work->collision_dummy_target );	/* 当たり判定を有効に */
					if ( work->oozeblood_hit_joint < 13 || work->oozeblood_hit_joint > 20 ){
						if ( work->blood_wait_time <= 0 ){
							TAKABE_OozeBloodAdd( work->oozeblood_effect_ptr,
												work->oozeblood_hit_joint, &GM_NikitaPosition[0], 1200.0f );
							work->blood_flag = 1 ;				/* 出血の許可 */
							work->blood_wait_time = BLOOD_WAIT_TIME ;
						}
					}
				}
				defence_enable |= res ;
			}
		}

		{/* 開口チェック */
			FVECTOR	*chin_root_rot, *head_rot, quat, inv_quat ;
			head_rot = &body->m_ctrl->abs_rots[ PDRAY_JOINT_HEAD ] ;
			chin_root_rot = &body->m_ctrl->abs_rots[ 15 ] ;	/* 下あご根元の回転クォータニオン取得 */
			MT_QuatInverse( &inv_quat, head_rot );
			MT_QuatMul( &quat, &inv_quat, chin_root_rot );
			if ( quat.vw < 0.996494114398956f/* = cosf( 4.8deg ) */ ){
				work->open_mouth_flag = 1 ;
			} else {
				work->open_mouth_flag = 0 ;
			}
		}

		/* ダメージ処理 */
		if ( ( defence_enable & ( COLL_HEAD | COLL_MOUTH ) ) &&
			( work->invinsible_time == 0 ) &&
			( work->mini_invinsible_time == 0 ) &&
			( work->true_action_bit & HEAD_DAMAGE_STATUS ) ){
			int		damage_param ;
			work->mini_invinsible_time = MINI_INVINSIBLE_TIME ;
			work->invinsible_time = INVINSIBLE_TIME ;
			/* ダメージ量計算 */
			if ( defence_enable & COLL_TYPE_RGB ){
				damage_param = ( work->open_mouth_flag ) ? DAMAGE_RGB6_B : DAMAGE_RGB6_A ;
			} else {
				damage_param = ( work->open_mouth_flag ) ? DAMAGE_STINGER_B : DAMAGE_STINGER_A ;
			}
			if ( RAYSERVER_GameLevel <= 1 ){/* ベリーイージー用ダメージ量調整 */
				damage_param *= 4 ;
			}
			if ( work->life < damage_param ) damage_param = work->life ;
			work->life -= damage_param ;
			RAYSERVER_SetGameSignal( GAME_SIGNAL_DAMAGE, damage_param );
			if ( work->life <= 0 ){
				/* 死亡 */
				work->life = 0 ;
				if ( ( ++GM_MecaKillCount ) & 0x8000 ) GM_MecaKillCount = 0x7fff ;/* 破壊カウントの最大は１５ビット */
				ExecProc( work->proc_id, 0 );
				PDRAY_SetDemandActionBit( work, RAY_STATUS_DAMAGE );
				if ( GV_Time & 1 ){
					GM_SeSetMode( SD_E_GROWL001, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
				} else {
					GM_SeSetMode( SD_E_GROWL004, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
				}
			} else {
				/* 通常ダメージ */
				PDRAY_SetDemandActionBit( work, RAY_STATUS_DAMAGE );
				if ( GV_Time & 1 ){
					GM_SeSetMode( SD_E_GROWL001, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
				} else {
					GM_SeSetMode( SD_E_GROWL004, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
				}
			}
			if ( !( work->true_action_bit & RAY_STATUS_DAMAGE ) ){
				work->damaged_flag = 1 ;
			}
			{/* ダメージによりエフェクトを起動 */
				int		level ;
				if ( work->life < ( work->life_max / 3 ) ){
					level = 3 - work->life * 3 * 4 / work->life_max ;
					PDRAY_SetBreakSmoke( work, level );
				}
			}
		} else if ( ( defence_enable & COLL_RLEG ) &&
				   ( work->mini_invinsible_time == 0 ) &&
				   ( work->true_action_bit & RLEG_DAMAGE_STATUS ) ){
			int		damage_param ;
			work->mini_invinsible_time = MINI_INVINSIBLE_TIME ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_DAMAGE2 );
			if ( GV_Time & 1 ){
				GM_SeSetMode( SD_E_GROWL001, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			} else {
				GM_SeSetMode( SD_E_GROWL004, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			}
#if 0
			/* ベリーイージー用ダメージ量処理 */
			if ( RAYSERVER_GameLevel <= 1 ){
				/* ダメージ量計算 */
				if ( defence_enable & COLL_TYPE_RGB ){
					damage_param = DAMAGE_RGB6_A ;
				} else {
					damage_param = DAMAGE_STINGER_A ;
				}
				damage_param *= 3 ;
				if ( work->life < damage_param ) damage_param = work->life ;
				work->life -= damage_param ;
				RAYSERVER_SetGameSignal( GAME_SIGNAL_DAMAGE, damage_param );
				if ( work->life <= 0 ){
					/* 死亡 */
					work->life = 0 ;
					if ( ( ++GM_MecaKillCount ) & 0x8000 ) GM_MecaKillCount = 0x7fff ;/* 破壊カウントの最大は１５ビット */
					ExecProc( work->proc_id, 0 );
				}
				if ( !( work->true_action_bit & RAY_STATUS_DAMAGE2 ) ){
					work->damaged_flag = 1 ;
				}
				{/* ダメージによりエフェクトを起動 */
					int		level ;
					if ( work->life < ( work->life_max / 3 ) ){
						level = 3 - work->life * 3 * 4 / work->life_max ;
						PDRAY_SetBreakSmoke( work, level );
					}
				}
			}
#endif
		} else if ( ( defence_enable & COLL_LLEG ) &&
				   ( work->mini_invinsible_time == 0 ) &&
				   ( work->true_action_bit & LLEG_DAMAGE_STATUS ) ){
			int		damage_param ;
			work->mini_invinsible_time = MINI_INVINSIBLE_TIME ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_DAMAGE3 );
			if ( GV_Time & 1 ){
				GM_SeSetMode( SD_E_GROWL001, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			} else {
				GM_SeSetMode( SD_E_GROWL004, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			}
#if 0
			/* ベリーイージー用ダメージ量処理 */
			if ( RAYSERVER_GameLevel <= 1 ){
				/* ダメージ量計算 */
				if ( defence_enable & COLL_TYPE_RGB ){
					damage_param = DAMAGE_RGB6_A ;
				} else {
					damage_param = DAMAGE_STINGER_A ;
				}
				damage_param *= 3 ;
				if ( work->life < damage_param ) damage_param = work->life ;
				work->life -= damage_param ;
				RAYSERVER_SetGameSignal( GAME_SIGNAL_DAMAGE, damage_param );
				if ( work->life <= 0 ){
					/* 死亡 */
					work->life = 0 ;
					if ( ( ++GM_MecaKillCount ) & 0x8000 ) GM_MecaKillCount = 0x7fff ;/* 破壊カウントの最大は１５ビット */
					ExecProc( work->proc_id, 0 );
				}
				if ( !( work->true_action_bit & RAY_STATUS_DAMAGE3 ) ){
					work->damaged_flag = 1 ;
				}
				{/* ダメージによりエフェクトを起動 */
					int		level ;
					if ( work->life < ( work->life_max / 3 ) ){
						level = 3 - work->life * 3 * 4 / work->life_max ;
						PDRAY_SetBreakSmoke( work, level );
					}
				}
			}
#endif
		}
	}

	/* ライフゲージのアニメーション及び表示・非表示チェック */
	if ( ( ( work->life > 0 ) || ( work->life_gauge.w > 1 ) ) && work->action_work.mode != ACTION_SLEEP ){
		if ( work->life > 0 ){
			if ( work->life_gauge.w < 240 ){
				work->life_gauge.w += 1 ;
				if ( work->life_gauge.w > 240 ) work->life_gauge.w = 240 ;
			}
		} else {
			if ( work->life_gauge.w > 1 ){
				work->life_gauge.w -= 1 ;
				if ( work->life_gauge.w < 1 ) work->life_gauge.w = 1 ;
			}
		}
		work->life_gauge.value = work->life ;
		GM_VisibleGage( &work->life_gauge ) ;
	} else {
		if ( work->life_gauge.w > 1 ){
			work->life_gauge.w -= 1 ;
			if ( work->life_gauge.w < 1 ) work->life_gauge.w = 1 ;
		}
		work->life_gauge.value = work->life ;
		GM_InvisibleGage( &work->life_gauge );
		lockon_enable = 0 ;
		lockon_enable2 = 0 ;
	}

	/* 無敵中はロックオンを無効に設定 */
	if ( work->invinsible_time != 0 ) lockon_enable = 0 ;
	if ( ( work->true_action_bit & HEAD_DAMAGE_STATUS ) == 0 ) lockon_enable = 0 ;
	if ( work->mini_invinsible_time != 0 ) lockon_enable2 = 0 ;
	if ( ( work->true_action_bit & RLEG_DAMAGE_STATUS ) == 0 ) lockon_enable2 = 0 ;
	if ( ( work->true_action_bit & LLEG_DAMAGE_STATUS ) == 0 ) lockon_enable2 = 0 ;
	/* 未動作中はロックオンを無効に */
	if ( work->active_flag == 0 ) lockon_enable = 0 ;
	/* チャフ有効中はロックオンを無効に設定 */
	//if ( GM_GameStatus & STATE_CHAFF ) lockon_enable = 0 ;
	/* 死亡時にはロックオンを無効に */
	if ( work->life <= 0 ) lockon_enable = 0 ;

	/* ターゲット状態変更 */
	SetTargetMode( work, lockon_enable, lockon_enable2, defence_enable );

	{/* 地震エフェクトの発生処理 */
		if ( work->earthquake_effect_flag ){
			/* 2種類の音ランダム */
			//if ( GV_Time & 1 ) GM_SeSetMode( SD_W_EXPLOS01, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			//else 			   GM_SeSetMode( SD_W_EXPLOS02, &work->earthquake_pos, GM_SEMODE_BOMB ) ;
			{
				extern void *NewDummyEarthquakeShock( FMATRIX *parent, int in_size, int out_size, int height );
				extern void *NewFogWave( FMATRIX *parent, int size, int speed, int height );
				extern void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size );
				FMATRIX		mat ;
				GTE_UnitMatrix( &mat );
				*(FVECTOR*)mat.m[3] = work->earthquake_pos ;
				switch ( work->earthquake_effect_flag ){
				  case 1:
					//NewDummyEarthquakeShock( &mat, 2000, 4000, 500 );
					NewFogWave( &mat, 1000, 200, 300 );
					PDRAY_CLOUD_DeleteDensity( (FVECTOR*)mat.m[3], 6000 );
					break ;
				  case 2:
					//NewDummyEarthquakeShock( &mat, 4000, 8000, 500 );
					NewFogWave( &mat, 2000, 400, 300 );
					PDRAY_CLOUD_DeleteDensity( (FVECTOR*)mat.m[3], 9000 );
					break ;
				}
			}
		}
		work->earthquake_effect_flag = 0 ;
	}

	/* 地震発生処理 */
	if ( work->earthquake_flag ){
		if ( !GM_CheckPlayerStatus( PLAYER_ROLLING ) ){
			/* 転がっていない場合に発生 */
			//GM_MoveTarget( &work->earthquake_target, &work->earthquake_pos );
			GM_MoveTargetMap( &work->earthquake_target, &GM_PlayerPosition, work->map );
			switch ( work->earthquake_flag ){
			  case 1:
				GM_SetTargetWeaponType( &work->earthquake_target, WP_TUMBLE | WP_NOBLOOD | WP_NOMUTEKI );
				break ;
			  case 2:
				GM_SetTargetWeaponType( &work->earthquake_target, WP_TUMBLE | WP_NOBLOOD | WP_NOMUTEKI );
				break ;
			  case 3:
				break ;
			  case 4:
				break ;
			}
#if 0	/* コールバックにて */
			if ( work->earthquake_target.damaged ){
				/* 猶予時間を設定 */
				work->player_tumble_wait_time = PLAYER_TUMBLE_WAIT_TIME ;
			}
#endif
			GM_PutTarget( &work->earthquake_target );
			work->earthquake_target.damaged = 0 ;
		}
		work->earthquake_flag = 0 ;

	}
	/* 転ばせ時の攻撃猶予時間のチェック */
	if ( work->player_tumble_wait_time ){
		if ( ( work->player_tumble_wait_time -= TIME_BASE ) < 0 ) work->player_tumble_wait_time = 0 ;
	}
	


	if ( work->active_flag ){/* 足攻撃ターゲット処理 */
		FMATRIX		*world ;
		FVECTOR		force, tmp_vec ;
		float		len, scale, power ;
		int			attack_flag = 0 ;

		world = &body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world ;
		/* 右足吹っ飛びパワー計算 */
		GTE_SubVector( &force, &GM_PlayerPosition, (FVECTOR*)world->m[3] );
		force.vy = 0.0f ;
		len = DG_SQRT( GTE_InnerProduct( &force, &force ) );
		scale = ( KICKDAMAGE_RANGE - len ) / KICKDAMAGE_RANGE ;
		scale = DG_MAX( scale, 0.0f );
		GTE_ScaleVector( &force, &force, scale * KICKDAMAGE_SPEED / len );
		/* ターゲット移動量の算出 */
		GTE_SubVector( &tmp_vec, (FVECTOR*)world->m[3], &work->right_leg_attack.center );
		attack_flag = ( tmp_vec.vy < 0.0f ) ? 1 : 0 ;	/* 下方向に移動するときだけ攻撃ＯＮ */
		power = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) ) ;
		/* ターゲットの設置 */
		GM_SetPowerTarget( &work->right_leg_attack, &work->right_leg_power,
						  POWER_ONCE, 255, 0, KICK_DAMAGE, &force );
		GM_MoveTarget2( &work->right_leg_attack, world );
		if ( power > KICKDAMAGE_THRESHOLD && attack_flag ){
			GM_PutTarget( &work->right_leg_attack );
		} else {
			/* プレイヤーの押し出し処理 */
			float	dh ;
			dh = DG_FABS( GM_PlayerPosition.vy - world->m[3][1] );
			if ( dh < LEG_PUSH_HEIGHT ){
				if ( len < LEG_PUSH_RANGE ){
					GTE_SubVector( &force, &GM_PlayerPosition, (FVECTOR*)world->m[3] );
					force.vy = 0.0f ;
					GTE_ScaleVector( &tmp_vec, &force, LEG_PUSH_RANGE / len );
					GTE_AddVector( &tmp_vec, &tmp_vec, (FVECTOR*)world->m[3] );
					GTE_SubVector( &tmp_vec, &tmp_vec, &GM_PlayerControl->mov );
					GM_PlayerControl->step.vx += tmp_vec.vx ;
					GM_PlayerControl->step.vz += tmp_vec.vz ;
				}
			}
		}
		//printf("r_power:%f\n", power );

		world = &body->objs->objs[PDRAY_JOINT_LEFT_TOE].world ;
		/* 左足吹っ飛びパワー計算 */
		GTE_SubVector( &force, &GM_PlayerPosition, (FVECTOR*)world->m[3] );
		force.vy = 0.0f ;
		len = DG_SQRT( GTE_InnerProduct( &force, &force ) );
		scale = ( KICKDAMAGE_RANGE - len ) / KICKDAMAGE_RANGE ;
		scale = DG_MAX( scale, 0.0f );
		GTE_ScaleVector( &force, &force, scale * KICKDAMAGE_SPEED / len );
		/* ターゲット移動量の算出 */
		GTE_SubVector( &tmp_vec, (FVECTOR*)world->m[3], &work->left_leg_attack.center );
		attack_flag = ( tmp_vec.vy < 0.0f ) ? 1 : 0 ;	/* 下方向に移動するときだけ攻撃ＯＮ */
		power = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) ) ;
		/* ターゲットの設置 */
		GM_SetPowerTarget( &work->left_leg_attack, &work->left_leg_power,
						  POWER_ONCE, 255, 0, KICK_DAMAGE, &force );
		GM_MoveTarget2( &work->left_leg_attack, world );
		if ( power > KICKDAMAGE_THRESHOLD && attack_flag ){
			GM_PutTarget( &work->left_leg_attack );
		} else {
			/* プレイヤーの押し出し処理 */
			float	dh ;
			dh = DG_FABS( GM_PlayerPosition.vy - world->m[3][1] );
			if ( dh < LEG_PUSH_HEIGHT ){
				if ( len < LEG_PUSH_RANGE ){
					GTE_SubVector( &force, &GM_PlayerPosition, (FVECTOR*)world->m[3] );
					force.vy = 0.0f ;
					GTE_ScaleVector( &tmp_vec, &force, LEG_PUSH_RANGE / len );
					GTE_AddVector( &tmp_vec, &tmp_vec, (FVECTOR*)world->m[3] );
					GTE_SubVector( &tmp_vec, &tmp_vec, &GM_PlayerControl->mov );
					GM_PlayerControl->step.vx += tmp_vec.vx ;
					GM_PlayerControl->step.vz += tmp_vec.vz ;
				}
			}
		}
		//printf("l_power:%f\n", power );
	}

	if ( work->active_flag ){/* カメラめり込みチェック */
		if ( CalcCollision( work, (FVECTOR*)DG_Chanl(0)->eye.m[3] ) ){
			body->objs->flag |= DG_FLAG_SEMITRANS ;
			work->semitrans_count = 10 ;
		} else {
			if ( work->semitrans_count ){
				work->semitrans_count-- ;
			} else {
				body->objs->flag &= ~DG_FLAG_SEMITRANS ;
			}
		}
	}

	if ( work->active_flag ){/* ダイナミックフロア設定 */
		COLLISION_PARAM		*c_param ;
		FMATRIX		target_mat ;
		int			i, j ;

		/* ダイナミックフロアの作成 */
		if ( work->d_floors[ 0 ][ 0 ] == NULL ){
			for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
				IVECTOR	ipos[4] = {{0,0,0,1},{1,0,0,1},{0,0,1,1},{1,0,1,1}};
				for ( j = 0 ; j < 6 ; j++ ){
					work->d_floors[ i ][ j ] = HZX_AddDynamicFloor( work->hzx_id,
																   &ipos[0], &ipos[1], &ipos[2], &ipos[3], 4,
																   FLOOR_FLAG );
				}
			}
		}

		c_param = collision_table ;
		for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
			GTE_MulMatrix( &target_mat, &body->objs->objs[ c_param->joint ].world, &c_param->mat );
			SetHazardBox( work->d_floors[ i ], &target_mat, &c_param->max, &c_param->min );
			c_param++ ;
		}
	} else {
		/* とりあえず無効にしておく */
		int		i, j ;
#if 0
		for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
			for ( j = 0 ; j < 6 ; j++ ){
				work->d_floors[i][j]->atr |= HZX_FLOOR_SKIP ;
			}
		}
#else
		for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++ ){
			for ( j = 0 ; j < 6 ; j++ ){
				if ( work->d_floors[ i ][ j ] ){
					HZX_RemoveDynamicFloor( work->d_floors[ i ][ j ] );
					work->d_floors[ i ][ j ] = NULL ;
				}
			}
		}
#endif
	}


	{/* ライフゲージの色を調整 */
		static CVECTOR color_list[3] = { {0,154,255,128},{40,218,120,128},{218,210,40,128} };
		GM_GageSet	*gs ;
		CVECTOR	*col ;

		gs = &work->life_gauge ;
		col = &color_list[ work->info.color_id ] ;
		GM_SetGageColor( gs, 0,0,0, 40,128,118, col->r,col->g,col->b, 255,0,0 ) ;
	}

}
/* ---------------------------------------------------------------- */
/* スティンガーの近づきをチェック（攻撃警戒時チェック用） */
/* 注意）テスト中のためチェック用定数は直書き！！ */
int PDRAY_CheckNearStinger( Work *work )
{
	extern TARGET	*PL_LockonTarget ;
	float		len ;

	/* スティンガー存在チェック */
	if ( !( GM_WeaponAlive & WP_ALIVE_STINGER ) ){
		return ( -1 );
	}
	if ( PL_LockonTarget == NULL ){
		/* ロックオンしていない場合（弾は直線的に進む） */
		FVECTOR		stg_speed, vec ;
		float		speed, near_len ;
		RAYSERVER_GetStingerVelocity( &stg_speed );
		/* スティンガーのスピードと方向単位ベクトルの取得 */
		speed = DG_SQRT( GTE_InnerProduct( &stg_speed, &stg_speed ) );
		if ( speed < 0.1f ) return ( -1 );
		GTE_ScaleVector( &stg_speed, &stg_speed, 1.0f / speed );
		/* ターゲットとの最近点までの距離を求める */
		GTE_SubVector( &vec, &work->lock_on_target.center, &GM_StingerPosition );
		len = GTE_InnerProduct( &stg_speed, &vec );
		if ( len < 0.0f ) return ( -1 );	/* 逆方向なので */
		/* そのまま行けばあたる可能性があるかをチェック */
		near_len = GTE_InnerProduct( &vec, &vec ) - len * len ;
		printf("%f %f \n", near_len, len );
		if ( near_len > ( 5000.0f * 5000.0f ) ){
			/* あたる可能性がない場合 */
			return ( -1 );
		}
		/* あたる可能性がある場合 */
		return ( 1 );
	} else {
		/* ロックオンされている場合（弾はホーミングされる可能性がある） */
		FVECTOR		vec ;
		if ( &work->lock_on_target != PL_LockonTarget ) return ( 0 );
		GTE_SubVector( &vec, &work->lock_on_target.center, &GM_StingerPosition );
		len = DG_SQRT( GTE_InnerProduct( &vec, &vec ) );
		if ( len < 13000.0f ) return ( 1 );
		return ( -1 );
	}
	
}

/* ---------------------------------------------------------------- */
/* 簡易床チェック */
int PDRAY_GetFloor( Work *work, FVECTOR *pos )
{
	HZX_LevelHazardCheck( work->control.hzx_id, pos,
						 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR, CHECK_FLOOR_FLAG );
	pos->vy = HZX_GetFloorLevel();
	return ( DG_FTOI( pos->vy ) );
}


/* ---------------------------------------------------------------- */
void PDRAY_CollisionEdit( Work *work )
{
#ifdef COLLISION_EDIT
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	/* debug */
	if ( work->name == GV_StrCode("ray1") || 1 ){
		static int		param_num = 0 ;
		static int		flag = 1 ;
		if ( GV_PadData[1].press & PAD_X ){
			if ( collision_table[ param_num + 1 ].joint != -1 ){
				param_num += 1 ;
				flag = 1 ;
			}
		}
		if ( GV_PadData[1].press & PAD_Y ){
			if ( param_num > 0 ){
				param_num -= 1 ;
				flag = 1 ;
			}
		}
		if ( flag ){
			PDRAY_DebugParamBuffer[0] = collision_table[ param_num ].joint ;
			PDRAY_DebugParamBuffer[1] = collision_table[ param_num ].rot.vx ;
			PDRAY_DebugParamBuffer[2] = collision_table[ param_num ].rot.vy ;
			PDRAY_DebugParamBuffer[3] = collision_table[ param_num ].rot.vz ;
			PDRAY_DebugParamBuffer[4] = collision_table[ param_num ].min.vx ;
			PDRAY_DebugParamBuffer[5] = collision_table[ param_num ].max.vx ;
			PDRAY_DebugParamBuffer[6] = collision_table[ param_num ].min.vy ;
			PDRAY_DebugParamBuffer[7] = collision_table[ param_num ].max.vy ;
			PDRAY_DebugParamBuffer[8] = collision_table[ param_num ].min.vz ;
			PDRAY_DebugParamBuffer[9] = collision_table[ param_num ].max.vz ;
			flag = 0 ;
		}
		DEBUG_Printf("%08x : %d , %d %d %d \n",
			   PDRAY_DebugParamBuffer, PDRAY_DebugParamBuffer[0],
			   PDRAY_DebugParamBuffer[1], PDRAY_DebugParamBuffer[2], PDRAY_DebugParamBuffer[3]);
		DEBUG_Printf("%d %d %d, %d %d %d\n",
			   PDRAY_DebugParamBuffer[4], PDRAY_DebugParamBuffer[5], PDRAY_DebugParamBuffer[6],
			   PDRAY_DebugParamBuffer[7], PDRAY_DebugParamBuffer[8], PDRAY_DebugParamBuffer[9]);
		collision_table[ param_num ].joint = PDRAY_DebugParamBuffer[0] ;
		collision_table[ param_num ].rot.vx = PDRAY_DebugParamBuffer[1] ;
		collision_table[ param_num ].rot.vy = PDRAY_DebugParamBuffer[2] ;
		collision_table[ param_num ].rot.vz = PDRAY_DebugParamBuffer[3] ;
		collision_table[ param_num ].min.vx = PDRAY_DebugParamBuffer[4] ;
		collision_table[ param_num ].max.vx = PDRAY_DebugParamBuffer[5] ;
		collision_table[ param_num ].min.vy = PDRAY_DebugParamBuffer[6] ;
		collision_table[ param_num ].max.vy = PDRAY_DebugParamBuffer[7] ;
		collision_table[ param_num ].min.vz = PDRAY_DebugParamBuffer[8] ;
		collision_table[ param_num ].max.vz = PDRAY_DebugParamBuffer[9] ;
		{
			DG_SetPos( &DG_UnitMatrix );
			DG_RotatePos( &collision_table[ param_num ].rot );
			DG_GetPos( &collision_table[ param_num ].mat );
		}
		if ( GV_PadData[1].press & PAD_A ){
			printf("{");
			printf("%d, 0, ", collision_table[ param_num ].joint );
			printf("{%d,%d,%d}, ",
				   collision_table[ param_num ].rot.vx,
				   collision_table[ param_num ].rot.vy,
				   collision_table[ param_num ].rot.vz );
			printf("{%d,%d,%d}, ",
				   (int)collision_table[ param_num ].min.vx,
				   (int)collision_table[ param_num ].min.vy,
				   (int)collision_table[ param_num ].min.vz );
			printf("{%d,%d,%d}, ",
				   (int)collision_table[ param_num ].max.vx,
				   (int)collision_table[ param_num ].max.vy,
				   (int)collision_table[ param_num ].max.vz );
			printf(" {0} },\n");
		}
		{
			DG_OBJS		*objs = body->objs ;
			COLLISION_PARAM		*c_param ;
			FMATRIX		target_mat ;
			int			i ;

			c_param = collision_table ;
			for ( i = 0 ; i < PDRAY_HAZARD_JOINTS ; i++, c_param++ ){
				extern void *NewBoundingBoxView_1( FMATRIX*, FVECTOR *, FVECTOR *, int );
				int		color ;
				if ( c_param->joint == -1 ) break ;
				GTE_MulMatrix( &target_mat, &objs->objs[ c_param->joint ].world, &c_param->mat );
				color = ( i == param_num ) ? 0x000000ff : 0x000080ff ;
				NewBoundingBoxView_1( &target_mat, &c_param->min, &c_param->max, color );
			}
		}
	}
#endif
}
