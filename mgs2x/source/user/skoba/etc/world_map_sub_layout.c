//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   world_map_layout.c
   全体マップL2D

   2001/07/11 S.Kobayashi
   $Id: world_map_sub_layout.c,v 1.1.1.3 2002/11/19 11:50:23 Yoshizawa1 Exp $
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

#define		LAYOUT		(3674722)	/* mapb.l2d */
#define 	STR_ROOT         (2770484)
#define CENTER_MAX (153)
#define CENTER_SPEED_X (153)
#define GRAPH_MAX (3)
#define SK_CLOSE_LAYOUT (0x1)
#define SK_A (64)

// action
#define STR_DEFAULT (566267)
#define STR_MAPBACTION (7359328)
#define STR_MAPBOPEN (4967988)
#define STR_MAPBCLOSE (12054413)

#define SK_PARENT_CALL (0x2)

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
} Graph;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;

	Graph               graph[ GRAPH_MAX ];
	int                 graph_counter;
	SVECTOR             cam_rot;
	SPR_OBJ             *null;
	int                 action;
	int                 flag;
	int                 *position;
	int                 position_bak;
	float               raute;
	void  ( *act )( struct _work * );
} Work ;

// my
static int AnimationAct( Work *pWork ); // 通常実行

#define STR_NULL (3727852)
#define STR_GRAPH1 (5393926)

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;

	switch ( signal ){
	case SK_CLOSE_LAYOUT :
		pWork->action = STR_MAPBCLOSE;
 		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
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

static void SprInit( Work *pWork )
{
	SPR_OBJ	   *spr;
	int     strcode;
	int           i;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return;
	}
	// nullの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NULL );
	if ( spr == NULL ){
		SK_Err( "null\0" );
		return;
	}
	SPR_SHOW( spr );
	pWork->null = spr;
	pWork->null->empty.pos.x = 325;
	pWork->null->empty.pos.y = 247;
	// nullの取得
	strcode = STR_GRAPH1;
	for ( i = 0 ; i < 3 ; i ++ ){
		spr = L2D_GetObject( pWork->handle_2d, strcode );
		if ( spr == NULL ){
			SK_Err( "graph\0" );
			return;
		}
		SPR_SHOW( spr );
		pWork->graph[ i ].obj = spr;
		pWork->graph[ i ].end_position = 0.0f;
		pWork->graph[ i ].obj->line.col[ 0 ].a = SK_A;
		pWork->graph[ i ].obj->line.col[ 1 ].a = SK_A;
		strcode++;
	}
	// alpha ダウン
//	U2D_TreeMulAlpha( SPR_OBJ * obj ,  );

	// 初期化終了
	pWork->flag |= SK_INITIALIZE_OK;	
}

static inline void NullControl( Work *pWork )
{
	pWork->null->empty.pos.x = 325;
	pWork->null->empty.pos.y = 247;

	if ( pWork->position != NULL ){
		if ( pWork->position_bak != *pWork->position ){
			pWork->raute = 1.0f;
			pWork->position_bak = *pWork->position;
		} else if ( pWork->raute > 0.05f ){
			pWork->raute -= 0.01f;
		}
	}
}

static void GraphControl( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	float         ftmp;
	float            r;
	float          add;
	int           sign;
	int              i;
	int            tmp;
	SPR_OBJ       *spr;


	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vy *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 4000.f );
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
	for ( i = 0 ; i < GRAPH_MAX ; i ++ ){
		spr = pWork->graph[ i ].obj;
		if ( ( ( pWork->graph[ i ].end_position > spr->line.pos[ 1 ].x - 4.0f ) && 
			   ( pWork->graph[ i ].end_position < spr->line.pos[ 1 ].x + 4.0f ) ) || pWork->graph[ i ].end_position == 0.0f || 
			 ( spr->line.pos[ 0 ].x == spr->line.pos[ 1 ].x ) ){
			pWork->graph[ i ].add = 1;
			pWork->graph[ i ].add = ( pWork->graph[ pWork->graph_counter ].obj->line.pos[ 1 ].x - 
									  pWork->graph[ pWork->graph_counter ].obj->line.pos[ 0 ].x ) / 24.0f;
			tmp = 16;
			switch ( i ) {
			case 0 : // 左
				pWork->graph[ i ].div = ( rnd() * 1.0f ) + 1.6f;
				pWork->graph[ i ].end_position = spr->line.pos[ 0 ].x + ( ( int )( stmp.vz + stmp.vw ) % 128 ) + ( tmp * 3 );
				pWork->graph[ i ].end_position -= ( pWork->graph[ i ].add * pWork->raute );
				pWork->graph[ i ].end_position += ( rnd() * 6.0f * ( float )tmp ) * pWork->raute;
				break;
			case 1 : // 真中
				pWork->graph[ i ].div = ( rnd() * 3.0f ) + 1.6f;
				pWork->graph[ i ].end_position = spr->line.pos[ 0 ].x + ( ( int )( pWork->graph[ 0 ].end_position / 2 ) % 128 ) + ( tmp );
				pWork->graph[ i ].end_position -= ( pWork->graph[ i ].add * pWork->raute );
				pWork->graph[ i ].end_position += ( rnd() * 10.0f * ( float )tmp ) * pWork->raute;
				break;
			case 2 : // 右
			    pWork->graph[ i ].div = ( pWork->graph[ 0 ].div + pWork->graph[ 1 ].div ) * rnd() + 1.0f;
				pWork->graph[ i ].end_position = spr->line.pos[ 0 ].x + ( ( int )stmp.vw % 128 ) + tmp * 2;
				pWork->graph[ i ].end_position -= ( pWork->graph[ i ].add * pWork->raute );
				pWork->graph[ i ].end_position += ( rnd() * 24.0f * ( float )tmp ) * pWork->raute;
				break;
			}
			pWork->graph[ i ].end_position = pWork->graph[ i ].end_position < 0.0f ? 0.0f : pWork->graph[ i ].end_position;
			pWork->graph[ i ].end_position = pWork->graph[ i ].end_position > 128.0f ? 128.0f : pWork->graph[ i ].end_position;
		}
		// 更新
	    add = ( pWork->graph[ i ].end_position - spr->line.pos[ 1 ].x ) / pWork->graph[ i ].div;
		sign = add < 0.0f ? -1 : 1;
	    add = add * ( float )sign < 1.0f ? 1.0f * ( float )sign : add;
		spr->line.pos[ 1 ].x += add;
		// はみ出し制御
		if ( spr->line.pos[ 1 ].x >= 128.0f ){
			spr->line.pos[ 1 ].x = 128.0f;
		}
		if ( spr->line.pos[ 0 ].x >= spr->line.pos[ 1 ].x ){
			spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
		}
		pWork->graph[ i ].obj->line.col[ 0 ].a = SK_A;
		pWork->graph[ i ].obj->line.col[ 1 ].a = SK_A;
	}
	pWork->graph_counter++;
	pWork->graph_counter %= GRAPH_MAX;

}

static	void	NormalAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	AnimationAct( pWork );
	NullControl( pWork );
	GraphControl( pWork );
	// 更新
	pWork->cam_rot = pCam->rotate;
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
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT : 
			pWork->action = STR_MAPBOPEN;
	 		pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			SprInit( pWork );
			break;
		case STR_MAPBOPEN :
			pWork->action = STR_MAPBACTION;
	 		pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->act = ( void * )NormalAct;
		case STR_MAPBACTION :
			pWork->action = STR_MAPBACTION;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			break;
		case STR_MAPBCLOSE :
//			GV_CallParentSignalFunc( pWork , SK_PARENT_CALL , 1 );
			GV_DestroyActor( pWork );
			break;
		}
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag &= ~SK_ACTION_START;
			pWork->flag |= SK_ACTION_OK;
		}
	}
	return ( 0 );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ) L2D_ReleaseLayout( work->handle_2d ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 1 , 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;

	// etc
	work->action = STR_DEFAULT;
	work->flag = SK_ACTION_START;
	work->act = (void *)AnimationAct;
	work->position_bak = 0xff;
	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewWorldMapSubLayout( int *position )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 200 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->position = position;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
