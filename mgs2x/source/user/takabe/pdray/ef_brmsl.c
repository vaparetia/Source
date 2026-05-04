//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_brmsl.c
	量産型ＲＡＹ用ミサイル切断エフェクト

	2001/07/18 K.Takabe
	$Id: ef_brmsl.c,v 1.1.1.3 2002/11/19 11:51:20 Yoshizawa1 Exp $

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

/* ---------------------------------------------------------------- */

extern float BP_AdjustTick3(float);
extern float BP_AdjustTick4(float);

#define FALL_ACCEL	(BP_AdjustTick4(10))
#define V_SCALE	   (BP_AdjustTick3(1.0f))		/* 速度スケール */
#define A_SCALE	   (BP_AdjustTick4(1.0f))		/* 速度スケール */

#define FIRST_VELOCITY			(400.0f*V_SCALE)		/* 初速度 */
//#define MAX_VELOCITY			(400.0f*V_SCALE)		/* 最大速度 */
#define MAX_VELOCITY			(350.0f*V_SCALE)		/* 最大速度 */
#define MAX_TURN				(GTE_PS2RAD(20)*V_SCALE)	/* １フレームでの最高補正角度 */
#define DEFAULT_ACCELERATION	(40.0f*A_SCALE)			/* 標準加速度 */
#define MAX_HOMING_TIME			( 5 * 120 )				/* ホーミング有効時間 */
#define TYPE2_HOMING_TIME		( 300 * 1 )				/* 場外攻撃用ホーミング開始時間 */
#define TYPE2_HOMING_DELAY_TIME	( 300 * 1 )				/* 場外攻撃用ホーミング開始遅延時間 */

#define CHECK_SEG_FLAG		(HZX_SEG_RECOIL_TYPE|HZX_SEG_NO_PLAYER|HZX_SEG_NO_ENEMY|HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE|HZX_SEG_NO_RECOIL|HZX_SEG_NO_HARITSUKI|HZX_SEG_NO_DISP_RADAR|HZX_SEG_NO_BULLETHOLE|HZX_SEG_NO_SPRAY|HZX_SEG_NO_ENEMY_EYES|HZX_SEG_NO_KNOCK_SE)

#define CHECK_FLOOR_FLAG	(HZX_FLOOR_ALL)

#define PLAYER_LEVEL		(4000.0f)

#define FIRE_OFFSET			(-400)		/* ミサイル原点からのバックファイアオフセット */


#define MODEL_NAME0			(2693360)		/* "pdray_kneemsl_r" */
#define MODEL_NAME1			(2693354)		/* "pdray_kneemsl_l" */
#define MODEL_FLAG			(DG_FLAG_ONEPIECE)
#define MODEL_ROT_CORRECT	(-1024)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )


/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			hzx_id ;

	int			destroy_count ;		/* 破片爆破数 */

	FMATRIX		world[2] ;			/* マトリクス */
	FVECTOR		vvec[2] ;			/* 加速度 */

	FMATRIX		light[2] ;
	DG_OBJS		*objs[2] ;


} Work ;

/* ---------------------------------------------------------------- */
extern void *NewBombEffect( FVECTOR *pos, int mode );
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR		old_pos, new_pos ;
	int			i ;

	GTE_InitGTE();


	for ( i = 0 ; i < 2 ; i++ ){
		/* 座標更新 */
		old_pos = *(FVECTOR*)work->world[i].m[3] ;
		GTE_AddVector( &new_pos, &old_pos, &work->vvec[i] );
		*(FVECTOR*)work->world[i].m[3] = new_pos ;
		/* 加速度補正 */
		work->vvec[i].vy -= FALL_ACCEL / 2 ;

		/* 落下に伴う方向補正 */

		/* モデルのセット */
		work->objs[i]->world = work->world[i] ;

		/* あたり判定チェック */
		work->hzx_id = GM_GetMap( work->map )->hzx_group ;
		if ( HZX_OnlineHazardCheck( work->hzx_id, &old_pos, &new_pos,
								   HZX_CHK_ALL, CHECK_SEG_FLAG, CHECK_FLOOR_FLAG ) ){
			FVECTOR		pos ;
			HZX_GetOnlinePoint( &pos );
			NewBombEffect( &pos, 1 );
			if ( ++work->destroy_count >= 2 ){
				GV_DestroyActor( work );
				return ;
			}
		}
	}

}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i ;
	for ( i = 0 ; i < 2 ; i++ ){
		if ( work->objs[i] != NULL ){
			DG_DequeueObjs( work->objs[i] );
			DG_FreeObjs( work->objs[i] );
		}
	}
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *world, FVECTOR *vvec )
{
	//static FVECTOR		first_velocity = {0,0,FIRST_VELOCITY,1};
	int		model_id ;
	int		i ;

	work->map = GM_CurrentMap ;

	/* 初期設定 */
	GTE_InitGTE();

	{/* モデル初期化 */
		DG_DEF		*def ;
		for ( i = 0 ; i < 2 ; i++ ){
			work->world[i] = *world ;
			work->vvec[i] = *vvec ;
			{
				FVECTOR		tmp_vec ;
				GTE_SetVector( &tmp_vec, ( i == 0 ) ? -20 : 20 , 0, 0, 1 );
				GTE_LoadMatrix( world );
				GTE_RotVector1( &tmp_vec, &tmp_vec );
				GTE_AddVector( &work->vvec[ i ], vvec, &tmp_vec );
				work->vvec[ i ].vw = 0.0f ;
				work->vvec[ i ].vy += 50 ;
			}
			model_id = ( i == 0 ) ? MODEL_NAME0 : MODEL_NAME1 ;
			def = GV_GetCache( GV_CacheID( model_id, 'k' ) );
			if ( def != NULL ){
				work->objs[i] = DG_MakeObjs( def, MODEL_FLAG, 0 );
				DG_QueueObjs( work->objs[i] );
				GM_GroupObjs( work->objs[i], GM_CurrentMap );
				//DG_SetLightMatrix( work->objs[i], work->light );
			}
		}
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewPDRayMissileBreak( FMATRIX *world, FVECTOR *vvec )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, world, vvec ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

