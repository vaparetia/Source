//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	de_wave5.c
	海面オブジェクト反応デモエフェクト

	2001/04/07 K.Takabe
	$Id: de_wave5.c,v 1.1.1.3 2002/11/19 11:51:13 Yoshizawa1 Exp $

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



/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

/* キャラクタ監視機構用構造体 */
typedef struct _conflict_control{
	int			flag ;
	float		range ;
	float		fall ;
	int			pad ;
	FVECTOR		pos ;
} CONFLICT_CONTROL ;
#define MAX_CONFLICT		(32)

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			type ;
	int			flag ;
	int			max_joint ;
	OBJECT		*body ;
	CONFLICT_CONTROL	conflict_list[ MAX_CONFLICT ] ;

	int			init_flag ;
	FVECTOR		old_pos ;
} Work ;

/* ---------------------------------------------------------------- */
typedef struct {
	int		joint ;
	float	range ;
} CONFLICT_TABLE ;
#if 0
static CONFLICT_TABLE	ray_cf_table[MAX_CONFLICT] = {
	{2,12000},

	{5,4000},
	{10,4000},

	{16,4000},

	{22,4000},
	{25,4000},

	{28,4000},
	{31,4000},

	{36,8000},
	{39,8000},
};
#endif
/* ---------------------------------------------------------------- */
/* wave5.c */
extern int TAKABE_GetSeaSurfaceHeight( FVECTOR *pos, FVECTOR *norm, int n_pos );
extern int TAKABE_AddWaveForce( FVECTOR *pos, float drop_force );
extern int TAKABE_RiseWave( CONFLICT_CONTROL *cf_ctrl );
extern void TAKABE_GetWaveCenter( FVECTOR *wave_center );
extern int TAKABE_RiseWaveEx( FVECTOR *start, FVECTOR *end, float range, int flag );
/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	DG_OBJS		*objs ;
	CONFLICT_CONTROL	*cf_ctrl ;
	//CONFLICT_TABLE		*cf_table ;
	FVECTOR				center, wave_pos ;

	objs = work->body->objs ;

	DEBUG_Locate( 32, 300, 0 );
	DEBUG_Locate( 32, 200, 0 );
#if 0
	DEBUG_Printf("%f %f %f\n",
				 objs->world.m[3][0], 
				 objs->world.m[3][1],
				 objs->world.m[3][2] );
#endif

	center = *(FVECTOR*)objs->world.m[3] ;

	if ( center.vx != 0.0f || center.vy != 0.0f || center.vz != 0.0f ){
#if 0
		/* 座標の設定 */
		cf_ctrl = work->conflict_list ;
		cf_table = ray_cf_table ;
		for ( i = 0 ; i < work->max_joint ; i++, cf_ctrl++, cf_table++ ){
			cf_ctrl->pos = *(FVECTOR*)objs->objs[ cf_table->joint ].world.m[3] ;
			cf_ctrl->range = cf_table->range ;
#if 0
			DEBUG_Printf("%2d %8.2f %8.2f %8.2f (%d)\n", i, 
						 objs->objs[cf_table->joint].world.m[3][0], 
						 objs->objs[cf_table->joint].world.m[3][1],
						 objs->objs[cf_table->joint].world.m[3][2],
						 cf_ctrl->flag );
#endif
		}

		cf_ctrl = work->conflict_list ;
		for ( i = 0 ; i < work->max_joint ; i++, cf_ctrl++ ){
			TAKABE_RiseWave( cf_ctrl );
		}
#else
		/* 強制的にパラメータを設定 */
		if ( work->init_flag == 0 ){
			work->old_pos = center ;
			work->init_flag = 1 ;
		}
		/* 波の高さを取得 */
		wave_pos = center ;
		TAKABE_GetWaveCenter( &wave_pos );
		if ( work->old_pos.vy < wave_pos.vy ){
			if ( center.vy > wave_pos.vy ){
				cf_ctrl = work->conflict_list ;
				cf_ctrl->range = 40000 ;
				cf_ctrl->flag = 0 ;
				cf_ctrl->fall = 0 ;
				cf_ctrl->pos = center ;
				cf_ctrl->pos.vy += cf_ctrl->range - 20000 ;
//cf_ctrl->pos.vy += cf_ctrl->range + 40000 ;
				TAKABE_RiseWave( cf_ctrl );
				//TAKABE_AddWaveForce( &center, 6000 );
			}
		} else {
			if ( center.vy < wave_pos.vy ){
				cf_ctrl = work->conflict_list ;
				cf_ctrl->range = 25000 ;
				cf_ctrl->flag = 1 ;
				cf_ctrl->fall = 0 ;
				cf_ctrl->pos = center ;
				cf_ctrl->pos.vy -= cf_ctrl->range - 15000 ;
//cf_ctrl->pos.vy -= cf_ctrl->range + 40000 ;
				TAKABE_RiseWave( cf_ctrl );
				//TAKABE_AddWaveForce( &center, -6000 );
			}
		}
		work->old_pos = center ;
#endif
	}
	//DG_FrameCount = 1 ;
#if 0
	if ( GV_PadDataDirect[0].press & PAD_A ){
		FVECTOR	start, end ;
		start = work->old_pos ;
		start.vx = -100000.0f ;
		start.vy = 10000.0f ;
		start.vz = -100000.0f ;
		end.vx = 100000.0f ;
		end.vy = 10000.0f ;
		end.vz = 100000.0f ;
		TAKABE_RiseWaveEx( &start, &end, 10000.0f, 0 );
	}
	if ( GV_PadDataDirect[0].press & PAD_B ){
		FVECTOR	start, end ;
		start = work->old_pos ;
		start.vx = -100000.0f ;
		start.vy = -10000.0f ;
		start.vz = -100000.0f ;
		end.vx = 100000.0f ;
		end.vy = -10000.0f ;
		end.vz = 100000.0f ;
		TAKABE_RiseWaveEx( &start, &end, 10000.0f, 1 );
	}
#endif
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, OBJECT *body, int type, int flag )
{
	work->body = body ;
	work->type = type ;
	work->flag = flag ;

	work->max_joint = 10 ;

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewSeaSurfaceReactionObject( OBJECT *body, int type, int flag )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, body, type, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


