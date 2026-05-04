//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_stage.c
	量産型ＲＡＹ戦ステージ用環境設定キャラ

	2001/06/30 K.Takabe
	$Id: pdr_stage.c,v 1.1.1.3 2002/11/19 11:51:25 Yoshizawa1 Exp $

*/
/*

chara	ＲＡＹステージ環境設定[NewPDRayStageEnvironmentSet] $s:name \
	-flag $w:フラグ
// デモでゲーム中と同じ環境を実現するキャラです。
// これは「ＲＡＹサーバー設置」キャラの内部でも呼ばれるためＲＡＹ戦
// ゲーム時には呼ばないようにしてください


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
#include	"camera.h"

#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
#define DEFAULT_NAME	(12758322)	/* "ＲＡＹステージ環境設定" */
/* ---------------------------------------------------------------- */

#ifdef DEBUG_MODE
//#define BG_COLOR_DEBUG
#endif

#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF)
#define N_PRIMS		(16)
#define N_VERTS		(10)

#define WALL_TOP	(100000.0f)
#define WALL_CENTER	(8000.0f)
#define WALL_BOTTOM	(0.0f)

/* ---------------------------------------------------------------- */
#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE)

/* ---------------------------------------------------------------- */
typedef struct _work{
	GV_ACT_EX		actor ;
	int				name ;
	int				map ;
	int				hzx_id ;

	int				end_proc ;					/* 終了プロック */

	/* ハザード管理 */
	HZX_D_FLOOR	*dynamic_base_floor ;
	HZX_D_FLOOR	*dynamic_stage_floor[2] ;

	/* debug */
	DG_PRIM2		*prim ;
	DG_PRIM2		*prim2 ;

} Work ;


/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
#ifdef BG_COLOR_DEBUG
	{
		DG_PRIM2	*prim ;
		DG_PRIM2_UVRGB		*uvrgb ;
		int			i, j ;
		static int	data[4] ;

		if ( GV_PadData[ 1 ].press & PAD_A ) printf("%08x\n", data );
		prim = work->prim ;
		DG_SwitchBuffPrim2( prim );
		uvrgb = prim->uvrgb[ prim->buffer_clock ];
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j+=2, uvrgb+=2 ){
				uvrgb[0].r = data[0] ;
				uvrgb[0].g = data[1] ;
				uvrgb[0].b = data[2] ;
				uvrgb[0].a = 0 ;
				uvrgb[1].r = data[0] ;
				uvrgb[1].g = data[1] ;
				uvrgb[1].b = data[2] ;
				uvrgb[1].a = 128 ;
			}
		}
	}
#endif


#if 0
	if ( GV_PadData[0].press & PAD_X ){
		//NewFogWave( &GM_PlayerBody->objs->world, 1000, 500, 300 );/*でかい*/
		//NewFogWave( &GM_PlayerBody->objs->world, 250, 150, 100 );//でかい
		NewFogWave( &GM_PlayerBody->objs->world, 100, 150, 300 );//でかい
	}
#endif
}
/* ---------------------------------------------------------------- */
	/*
		終了
	*/
static void Die( Work *work )
{
	/* 仮ハザード */
	HZX_RemoveDynamicFloor( work->dynamic_base_floor );
	/* debug */
	GM_FreePrim2( work->prim );
	GM_FreePrim2( work->prim2 );
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	//if ( GCL_GetOption( 'p' ) != NULL ){
	//	work->end_proc = GCL_GetNextInt();
	//}

	{/* 仮ハザードのプログラム生成 */
		static IVECTOR	base_floor[4] = {
			{-500000,0,-500000,1},{500000,0,-500000,1},{500000,0,500000,1},{-500000,0,500000,1}
		};
		work->hzx_id = GM_GetHzxGroupID( where );
		work->dynamic_base_floor = 
		  HZX_AddDynamicFloor( work->hzx_id, &base_floor[0], &base_floor[1], &base_floor[2], &base_floor[3], 
							  4, FLOOR_FLAG );
	}

	{/* debug */
		DG_PRIM2	*prim ;
		DG_PRIM2_UVRGB	*uvrgb ;
		FVECTOR		*pos ;
		int			i, j ;
		prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		pos = prim->pos[ 0 ];
		uvrgb = prim->uvrgb[ 0 ];
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j+=2, pos+=2, uvrgb+=2 ){
				float angle ;
				angle = ( i * 8 + j ) * (float)M_PI / 64 ;
				pos[0].vx = cosf( angle ) * 400000.0f;
				pos[0].vz = sinf( angle ) * 400000.0f;
				uvrgb[0].r = 0 ;
				uvrgb[0].g = 35 ;
				uvrgb[0].b = 32 ;
				uvrgb[0].a = 0 ;
				pos[1] = pos[0] ;
				uvrgb[1] = uvrgb[0] ;
				pos[0].vy = WALL_TOP ;
				pos[1].vy = WALL_CENTER ;
				uvrgb[1].a = 128 ;
				uvrgb[0].f = 0x0fff ;
				uvrgb[1].f = 0x0fff ;
				if ( j == 0 ) uvrgb[0].f = 0x8fff ;
			}
		}
	}

	{/* debug */
		DG_PRIM2	*prim ;
		DG_PRIM2_UVRGB	*uvrgb ;
		FVECTOR		*pos ;
		int			i, j ;
		prim = work->prim2 = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		pos = prim->pos[ 0 ];
		uvrgb = prim->uvrgb[ 0 ];
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j+=2, pos+=2, uvrgb+=2 ){
				float angle ;
				angle = ( i * 8 + j ) * (float)M_PI / 64 ;
				pos[0].vx = cosf( angle ) * 400000.0f;
				pos[0].vz = sinf( angle ) * 400000.0f;
				uvrgb[0].r = 0 ;
				uvrgb[0].g = 35 ;
				uvrgb[0].b = 32 ;
				pos[1] = pos[0] ;
				uvrgb[1] = uvrgb[0] ;
				pos[0].vy = WALL_CENTER ;
				pos[1].vy = WALL_BOTTOM ;
				uvrgb[0].a = 128 ;
				uvrgb[1].a = 0 ;
				uvrgb[0].f = 0x0fff ;
				uvrgb[1].f = 0x0fff ;
				if ( j == 0 ) uvrgb[0].f = 0x8fff ;
			}
		}
	}

	{
		extern void *NewRAYStageCloud( void );
		NewRAYStageCloud();
	}

	{
		extern void *NewSmokeReactionServer( void );
		NewSmokeReactionServer();
	}

	{
		extern void *NewPDRayStageFence( void );
		NewPDRayStageFence();
	}


	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewPDRayStageEnvironmentSet( int name, int where )
{
	Work *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );
}

	/*
		プログラム起動インターフェイス
	*/
void *NewPDRayStageEnvironment( void )
{
	Work *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, DEFAULT_NAME, GM_CurrentMap ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );
}


