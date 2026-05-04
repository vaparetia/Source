//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dg_cam_shuter_layout.c
   デジタルカメラシャッターサイト
   
   2001/05/12	S.Kobayashi
   $Id: dg_cam_shuter_layout.c,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
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

#define		LAYOUT	     	 (3040820)	// dcm_shat.l2d
#define     STR_DEFAULT      (566267)
#define 	STR_ROOT         (2770484)

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	float                angle_max;
	float                angle_min;
	char                 visible_flag;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_VISIBLE = 0,
	SK_INVISIBLE,
	SK_SHUTER_OPEN = 0x10,
	SK_SHUTER_FULL_OPEN = 0x20,
};

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション

// 子アクター
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )workp;

	switch ( signal ){
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( 1 );
	}

	return ( 0 );
}

#if 0
	case 0x2 :
		if ( value == SK_INVISIBLE ){
			pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
			if ( pRoot == NULL ){
				SK_Err( "root\0" );
				return ( -1 );
			}
			SPR_HIDE( pRoot );
			pWork->visible_flag = SK_INVISIBLE;
		} else {
			pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
			if ( pRoot == NULL ){
				SK_Err( "root\0" );
				return ( -1 );
			}
			SPR_SHOW( pRoot );
			pWork->visible_flag = SK_VISIBLE;
		}
		break;
static void invisible( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->action = 0;
	pWork->act = ( void *)AnimetionAct;
}
#endif

static int NormalAct( Work *pWork )
{
	GV_DestroyActor( pWork );

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( pWork->action < pWork->action_num ){
		if ( L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action ) != L2D_STAT_BUSY ){
			pWork->action++;
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_SHUTER_OPEN , 0 );
		}
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		GV_CallParentSignalFunc( pWork , SK_SHUTER_FULL_OPEN , 0 );
		pWork->act = (void *)NormalAct;
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	if ( pWork->visible_flag == SK_INVISIBLE ){
		return;
	}
 	pWork->act( pWork );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d ) ;
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;

	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MAIN , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		printf("shut_data not found\n");
		// 親に送信
		GV_CallParentSignalFunc( work , SK_SHUTER_OPEN , 0 );
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

void	*NewDgCamShuterSight( void )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
