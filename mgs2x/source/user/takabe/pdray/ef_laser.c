//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_laser.c
	量産型ＲＡＹ用ダミーエフェクト（水圧カッター）

	2001/04/17 K.Takabe
	$Id: ef_laser.c,v 1.1.1.3 2002/11/19 11:51:20 Yoshizawa1 Exp $

*/
/*



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


#include	"../other/vec_util.h"

#ifdef KP_XBOX
#define GM_SeSetMode( _a, _b, _c ) GM_SeSetModeAddr( _a, _b, _c, GM_INVALID_ADDR )
#endif

/* ---------------------------------------------------------------- */
#define PRIM_FLAG	(DG_PRIM2_LINE)
#define N_PRIMS		(1)
#define N_VERTS		(2)

#define CHECK_START_RANGE	(5000)
#define ATTACK_RANGE	(400000)
#define ATTACK_DAMAGE	(24)

#define CHECK_SEG_FLAG		(HZX_SEG_RECOIL_TYPE|HZX_SEG_NO_PLAYER|HZX_SEG_NO_ENEMY|HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE|HZX_SEG_NO_RECOIL|HZX_SEG_NO_HARITSUKI|HZX_SEG_NO_DISP_RADAR|HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_SPRAY|HZX_SEG_NO_ENEMY_EYES|HZX_SEG_NO_KNOCK_SE)

#define CHECK_FLOOR_FLAG	(HZX_FLOOR_ALL)
//#define CHECK_FLOOR_FLAG	(HZX_FLOOR_NO_MISSILE)

/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			hzx_id ;

	DG_PRIM2	*prim ;
	FMATRIX		*parent ;
	int			first_flag ;

	TARGET			attack ;
	POWER_TARGET	power_target ;
	FVECTOR			target_force ;
	int				count ;

	FVECTOR			effect_pos[3] ;
	FMATRIX			effect_mat ;

	/* 効果音発生用用ワーク */
	int				sound_phase ;
	int				sound_count ;
} Work ;


/* ---------------------------------------------------------------- */
extern void *NewColWater( FMATRIX *);
/* ---------------------------------------------------------------- */
static void CalcOnline( Work *work )
{
	static FVECTOR		line[3] = { {0,0,0,1},{0,0,ATTACK_RANGE,1},{0,0,CHECK_START_RANGE,1 } } ;
	FVECTOR			abs_pos[3] ;

	GM_CurrentMap = work->map ;

	{/* 床・壁とのあたり判定 */

		/* ２点の絶対座標を取得 */
		GTE_LoadMatrix( work->parent );
		GTE_PutVector( abs_pos, line, 3 );

		work->hzx_id = GM_GetMap( work->map )->hzx_group ;
		if ( HZX_OnlineHazardCheck( work->hzx_id, &abs_pos[2], &abs_pos[1],
								   HZX_CHK_ALL, CHECK_SEG_FLAG, CHECK_FLOOR_FLAG ) ){
			HZX_GetOnlinePoint( &abs_pos[1] );
		}
		abs_pos[0].vw = 1.0f ;
		abs_pos[1].vw = 1.0f ;
		abs_pos[2].vw = 1.0f ;
	}
	work->effect_pos[ 0 ] = abs_pos[ 0 ];
	work->effect_pos[ 1 ] = abs_pos[ 1 ];
	work->effect_pos[ 2 ] = abs_pos[ 2 ];
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR			abs_pos[3] ;
	DG_PRIM2	*prim ;
	FVECTOR		*pos ;

	GM_CurrentMap = work->map ;
	work->count++ ;

	prim = work->prim ;
	//prim->flag &= ~DG_PRIM2_INVISIBLE ;
	DG_SwitchBuffPrim2( prim );
	pos = prim->pos[ prim->buffer_clock ] ;

	CalcOnline( work );
	abs_pos[ 0 ] = work->effect_pos[ 0 ] ;
	abs_pos[ 1 ] = work->effect_pos[ 1 ] ;
	abs_pos[ 2 ] = work->effect_pos[ 2 ] ;

#if 0
	{/* エフェクト用壁反射マトリクス生成 */
		static FVECTOR	y_base = {0,1,0};
		FMATRIX		mat ;
		FVECTOR		tmp_vec, x_vec, y_vec, z_vec ;
		GTE_SubVector( &tmp_vec, &abs_pos[1], &abs_pos[0] );
		GTE_Normalize( &z_vec, &tmp_vec );
		z_vec.vy = -z_vec.vy ;
		GTE_AddVector( &tmp_vec, &tmp_vec, &abs_pos[1] );
		DG_MakeCameraMatrix( &mat, &abs_pos[1], &tmp_vec );
		*(FVECTOR*)mat.m[3] = abs_pos[1] ;
		work->effect_mat = mat ;
	}
#else
	GTE_UnitMatrix( &work->effect_mat );
	*(FVECTOR*)work->effect_mat.m[3] = abs_pos[1] ;
#endif

	/* 攻撃判定チェック */
	GM_MoveOnlineTarget( &work->attack, &abs_pos[2], &abs_pos[1] );
	GM_PutTarget( &work->attack );
	work->attack.damaged = 0 ;
	//AN_Test_Eye( &work->attack.hit, 3 );

	/* 効果音関係 */
	switch ( work->sound_phase ){
	  case 0:
		GM_SeSetMode( SD_E_LASER101, &abs_pos[0], GM_SEMODE_BOMB ) ;
		work->sound_phase++ ;
		break ;
	  case 1:
		if ( work->sound_count > 11*5 ){
			work->sound_phase++ ;
		}
		break ;
	  case 2:
		if ( work->sound_count > 7*5 ){
			GM_SeSetMode( SD_E_LASER201, &abs_pos[0], GM_SEMODE_BOMB ) ;
			work->sound_count = 0 ;
		}
		break ;
	}
	work->sound_count += TIME_BASE ;

	{
		extern void *NewVRFloorBreak( FMATRIX *parent, float scale );
		NewVRFloorBreak( &work->effect_mat, 1.5f );
	}
	{
		extern void	*NewRayWaterCutterSplash( FMATRIX *world );
		NewRayWaterCutterSplash( &work->effect_mat );
	}
#if 0
	if ( ( work->count & 3 ) == 0 ){
		extern void *NewSolTraceFlame( FVECTOR *pos, int life );
		NewSolTraceFlame(  &abs_pos[1], 300 );
	}
#endif
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *parent )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos ;
	int			i, j, k ;

	work->map = GM_CurrentMap ;
	work->parent = parent ;

	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	prim->flag |= DG_PRIM2_INVISIBLE ;

	for ( k = 0 ; k < 2 ; k++ ){

		/* ＵＶ、ＲＧＢ値の設定 */
		uvrgb = prim->uvrgb[ k ];
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j++, uvrgb++ ){
				uvrgb->r = 255 ;
				uvrgb->g = 0 ;
				uvrgb->b = 0 ;
				uvrgb->a = 128 ;
				if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 最初の頂点は描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* それ以降は描画キックを行う */
				}
			}
		}

		/* 頂点の設定 */
		pos = prim->pos[ k ] ;
		GTE_ZeroVector( &pos[ 0 ] );
		GTE_ZeroVector( &pos[ 1 ] );
		//pos[ 1 ].vz = 100000.0f ;
	}

	{/* 攻撃判定初期化 */
		FVECTOR		size ;
		size.vx = 100 ; 
		size.vy = 100 ;
		size.vz = 100 ;
		GM_SetTarget( &work->attack, TARGET_OFFENSE|TARGET_ONLINE|TARGET_ONLINE_MIN,
					 GM_CurrentMap, PLAYER_SIDE, &size, &DG_ZeroVector );
		GM_SetTargetWeaponType( &work->attack, WP_PUNCHL /*| WP_NOGUARD*/ );
		GM_SetPowerTarget( &work->attack, &work->power_target,
						  POWER_ONCE, 255, 0, ATTACK_DAMAGE, &work->target_force );
		//GM_SetTargetCallBack( &work->attack, Func, work );
	}

	/* 最初に一度計算をしておく */
	CalcOnline( work );

	/* エフェクト起動 */
#if 0
//	GV_SetActorChild( work, NewColWater( &work->effect_mat ) );
	{
		extern void *NewRayCrush( FVECTOR *pos, float radius );
		GV_SetActorChild( work, NewRayCrush( &work->effect_pos[ 1 ], 100.0f ) );
	}
#endif
	{
		extern void *NewWaterPollute_game( FMATRIX *root, FVECTOR *to_pos );
		void *ray_eft1;
		ray_eft1 = NewWaterPollute_game( parent, &work->effect_pos[1] );
		GV_SetActorChild( work, ray_eft1 );
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewDummyWaterCutter( FMATRIX *parent )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, parent ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

