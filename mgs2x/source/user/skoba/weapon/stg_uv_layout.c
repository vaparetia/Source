//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   Sthinger_uv_layout.c
   Sthinger(UV Anime)
   
   2001/05/17	S.Kobayashi
   $Id: stg_uv_layout.c,v 1.1.1.3 2002/11/19 11:50:31 Yoshizawa1 Exp $
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

#define		LAYOUT		(10563490)	/* stg_alt.l2d */
#define     STR_DEFAULT  (566267)
#define STR_ROOT (2770484)

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action;
	int                 action_num;
	char                togle; // モデルありモード or モデルなしモード
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_STHINGER_MODEL_OFF = 0x4, // モデルなし通常モードにいこう
	SK_STHINGER_MODEL_ON = 0x8, // モデル有直ねらいモードにいこう
};

// プロトタイプ
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );
static int AnimetionAct( Work * ); // アニメーション

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;
	SPR_OBJ *pRoot;

	pWork = ( Work * )workp;

	switch ( signal ){
	case SK_STHINGER_MODEL_ON :
		pRoot = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( pRoot == NULL ){
			SK_Err( "root\0" );
			return ( 0 );
		}
		SPR_HIDE( pRoot ); 
		pWork->togle = SK_STHINGER_MODEL_ON;
		break;
	case SK_STHINGER_MODEL_OFF :
		pRoot = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( pRoot == NULL ){
			SK_Err( "root\0" );
			return ( 0 );
		}
		SPR_SHOW( pRoot ); 
		pWork->togle = SK_STHINGER_MODEL_OFF;
		break;
	case 0x2 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

static void invisible2( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->act = ( void * )AnimetionAct;
}

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_Stinger ) || !GM_CheckGameStatus( STATE_CHAFF ) ){
		invisible2( pWork );
		return;
	}
 	pWork->act( pWork );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) || pWork->togle == SK_STHINGER_MODEL_ON ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num ){
		if ( L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action ) != L2D_STAT_BUSY ){
			pWork->action++;
		}
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		pWork->action = 1;
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

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;

	// etc
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	work->act = (void *)AnimetionAct;

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewSthingerUVSight( void )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
