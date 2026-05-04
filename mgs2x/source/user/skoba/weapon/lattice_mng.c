//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
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

#include	"gameheader.h"
#include	"camera.h"
#include    "../../kira/2D_action/layout_2d.h"

typedef struct _work {
	GV_ACT_EX       actor;

	int           strcode;          
	int             chanl;
	int           hd_name; // ハンドルネーム
	int               pri; // プライオリティ
	int            action; // 現在のフレーム
	int        action_num; // 総アクション数
	int              name;
	void  ( *act )( struct _work * );
} Work;

typedef struct _list {
    struct _list *pPrev;
	struct _list *pNext;
	Work *data;
} List;

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	
	switch ( signal ){
	case 0x2 :
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( -1 );
	}
	return ( 0 );
}

static void PauseAct( Work *pWork ) // アクションのストップ
{
	L2D_BreakAction( pWork->hd_name );
}

static int NormalAct( Work *pWork ) // 通常実行
{
	if ( L2D_EvokeActionByNumber( pWork->hd_name , pWork->action ) < 0 ){
		return ( -1 );
	} else if ( pWork->action < pWork->action_num ){
		pWork->action++;
	} else {
		pWork->act = (void *)PauseAct;
	}
	return ( 0 );
}

static int LatticeOpen( Work *pWork , int name , int strcode ) // オープン
{
	GV_SetActorSignalFunc( pWork , ReceiveSignal );

	pWork->name = name;
	pWork->strcode = strcode;
	pWork->hd_name = L2D_LoadLayout( strcode , pWork->chanl , pWork->pri , SPR_FLAG_PRIV); // ロード
	pWork->action_num = L2D_GetActionNumber( pWork->hd_name );
	pWork->action = 0;
	pWork->act = (void *)NormalAct;
		
	return ( 0 );
}

static void LatticeFree( Work *pWork ) // 解放
{
	pWork->hd_name = L2D_ReleaseLayout( pWork->hd_name );
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
	LatticeFree( pWork );	
}

void *NewLattice( int name , int strcode )
{
	Work *pWork;

	pWork = ( Work * )GV_NewActor( GV_ACTOR_USER , sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &pWork->actor , Act , Die );
		GV_ActorEX( &pWork->actor );
		if ( LatticeOpen( pWork , name , strcode ) != 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return pWork;
}
