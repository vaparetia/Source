//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   world_map_bug_layout.c
   全体マップバグL2D

   2001/07/07	S.Kobayashi
   $Id: world_map_bug_layout.c,v 1.1.1.3 2002/11/19 11:50:22 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"

#define		LAYOUT		(4823015)	/* mapbug.l2d */
#define 	STR_BLOCK1       (16325211)
#define 	STR_ROOT         (2770484)
#define CENTER_MAX (153)
#define CENTER_SPEED_X (153)
#define BLOCK_MAX (3)
#define SK_CLOSE_LAYOUT (0x1)
#define STR_DEFAULT (566267)
#define STR_DISPSTART (14775064)
#define STR_DISPEND (6043958)


enum {
	SK_ACTION_START = 0x1 ,
	SK_ACTION_OK    = 0x2 ,
};

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
	SK_NORMAL           = 0x20,
};

typedef struct {
	SPR_OBJ             *obj;
	float               end_position;
	float               div;
	float               add;
} Block;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;

	Block               block[ BLOCK_MAX ];
	int                 block_counter;
	int                 action;
	int                 flag;
	SVECTOR             cam_rot;
	void  ( *act )( struct _work * );
} Work ;

// my
static int AnimationAct( Work *pWork ); // 通常実行


static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;

	switch ( signal ){
	case SK_CLOSE_LAYOUT :
 		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->action = STR_DISPEND;
		// アクションの強制ストップ
		L2D_BreakAction( pWork->handle_2d );
		pWork->act = ( void * )AnimationAct;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

#if 0
static void SprInit( Work *pWork )
{
	SPR_OBJ	   *spr;
	int     strcode;
	int           i;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return;
	}
	// blockの取得
	strcode = STR_BLOCK1;
	for ( i = 0 ; i < BLOCK_MAX ; i ++ ){
		spr = L2D_GetObject( pWork->handle_2d, strcode );
		if ( spr == NULL ){
			SK_Err( "block\0" );
			return;
		}
		SPR_SHOW( spr );
		pWork->block[ i ].obj = spr;
		strcode++;
	}
	// 初期化終了
	pWork->flag |= SK_INITIALIZE_OK;	
}

static void BlockControl( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	float         ftmp;
	float            r;
	int           sign;
	int              i;
	SPR_OBJ       *spr;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vy *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 3000.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360 );
	stmp.vw  = ( ( ftmp * stmp.vy ) / 360 );
	// スピード補正
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -CENTER_SPEED_X ){
			stmp.vz = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vz > CENTER_SPEED_X ){
			stmp.vz = CENTER_SPEED_X;
		}
	}
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_SPEED_X ){
			stmp.vw = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vw > CENTER_SPEED_X ){
			stmp.vw = CENTER_SPEED_X;
		}
	}
	stmp.vz = DG_FABS( stmp.vz );
	stmp.vw = DG_FABS( stmp.vw );
	// いんちき
	for ( i = 0 ; i < BLOCK_MAX ; i ++ ){
		spr = pWork->block[ i ].obj;
		if ( ( ( pWork->block[ i ].end_position > spr->box.rect.begin.y - 1.0f ) && 
			   ( pWork->block[ i ].end_position < spr->box.rect.begin.y + 1.0f ) ) ||
			   spr->box.rect.begin.y == spr->box.rect.end.y ){
			pWork->block[ i ].add = ( pWork->block[ pWork->block_counter ].obj->box.rect.end.y - 
									  pWork->block[ pWork->block_counter ].obj->box.rect.begin.y ) / 12.0f;
			switch ( i ) {
			case 0 : // 左
				pWork->block[ i ].div = ( rnd() * 1.0f ) + 1.6f;
				pWork->block[ i ].end_position = spr->box.rect.end.y - ( ( int )( stmp.vz + stmp.vw / 4.0f ) % 153 );
				pWork->block[ i ].end_position -= pWork->block[ i ].add;
				pWork->block[ i ].end_position -= rnd() * 60.0f;
				pWork->block[ i ].end_position -= pWork->block[ 2 ].end_position / 10.0f;
				break;
			case 1 : // 真中
				pWork->block[ i ].div = ( rnd() * 3.0f ) + 1.6f;
				pWork->block[ i ].end_position = spr->box.rect.end.y - ( ( int )( ( stmp.vz + stmp.vw ) / 2.0f ) % 153 );
				pWork->block[ i ].end_position -= pWork->block[ i ].add;
				pWork->block[ i ].end_position -= rnd() * 80.0f;
				break;
			case 2 : // 右
				pWork->block[ i ].div = ( pWork->block[ 0 ].div + pWork->block[ 1 ].div ) * rnd() + 1.0f;
				pWork->block[ i ].end_position = spr->box.rect.end.y - ( ( int )stmp.vw % 153 );
				pWork->block[ i ].end_position -= pWork->block[ i ].add;
				pWork->block[ i ].end_position -= rnd() * 110.0f;
				pWork->block[ i ].end_position -= pWork->block[ 1 ].end_position / 20.0f;
				break;
			}
			pWork->block[ i ].end_position = pWork->block[ i ].end_position < 153.0f ? 153.0f : pWork->block[ i ].end_position;
			pWork->block[ i ].end_position = pWork->block[ i ].end_position > 339.0f ? 339.0f : pWork->block[ i ].end_position;
		} else {
			// 更新
			spr->box.rect.begin.y += ( pWork->block[ i ].end_position - spr->box.rect.begin.y ) / pWork->block[ i ].div;
			// はみ出し制御
			if ( spr->box.rect.begin.y <= 153.0f ){
				spr->box.rect.begin.y = 153.0f;
			}
			if ( spr->box.rect.begin.y >= spr->box.rect.end.y ){
				spr->box.rect.begin.y =  spr->box.rect.end.y;
			}
		}
	}
	pWork->block_counter++;
	pWork->block_counter %= BLOCK_MAX;

}
#endif

static	void	NormalAct( Work *pWork )
{
#if 0
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	BlockControl( pWork );
	// 更新
	pWork->cam_rot = pCam->rotate;
#endif
}

static void Act( Work *pWork )
{
	SPR_OBJ *spr;

	if ( ( GM_MenuStatus & MENU_RADIO_ON ) || ( GM_MenuStatus & MENU_NODE_ON ) ||
		 ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) ){ // 無線が始まろうとしたら all hide
		spr = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( spr == NULL ){
			return;
		}
		SPR_HIDE( spr );
	}
 	pWork->act( pWork );
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag &= ~SK_ACTION_START;
			pWork->flag |= SK_ACTION_OK;
		}
		return( 0 );
	}
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT : 
	 		pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->action = STR_DISPSTART;
			break;
		case STR_DISPSTART :
			pWork->action = 0;
			pWork->act = ( void * )NormalAct;
//			SprInit( pWork );
			break;
		case STR_DISPEND :
			GV_DestroyActor( pWork );
			break;
		}
	}

	return ( 0 );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		SK_Err("world_map_bug not found data file\0");
		return ( -1 );
	}
	work->handle_2d = handle ;

	// etc
	work->action = STR_DEFAULT;
	work->flag = SK_ACTION_START;
	work->act = (void *)AnimationAct;
	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewWorldMapBugLayout( void )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 200 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
