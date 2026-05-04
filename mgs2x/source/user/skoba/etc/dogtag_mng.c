//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dogtag_mng.c
   ドッグタグモード
   
   2001/07/11	S.Kobayashi
   $Id: dogtag_mng.c,v 1.1.1.3 2002/11/19 11:50:19 Yoshizawa1 Exp $
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
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../kano/mcman/mcman.h"

#define 	STR_SAVE_LOAD (10350292)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)
	
typedef	struct _work {
	GV_ACT_EX			actor;
	MCMAN_WORK          mcman; // メモりーカード
	int                  name;
	int                  flag;
    int            first_mode;

	int             handle_2d;
	int             prev_proc;
	int             next_proc;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_DESTROY = 0x1 ,
	SK_NEW_ACT  = 0x2 ,
	SK_CHILD_GENERATE = 0x4 ,
	SK_MESSAGE_CHECK = 0x8 ,
};

enum {
	SK_PREV_PROC = 0x1 ,
	SK_NEXT_PROC = 0x2,
};

// my
extern void *NewDogtagMcMng( int , int ); // マネージャ本体

static inline void ExecProc( Work *pWork , char mode )
{
	switch ( mode ){
	case SK_PREV_PROC :
		if ( pWork->prev_proc == 0 ){
			return;
		}
		GM_ExecProc( pWork->prev_proc , NULL );
		pWork->prev_proc = 0;
		break;
	case SK_NEXT_PROC :
		if ( pWork->next_proc == 0 ){
			return;
		}
		GM_ExecProc( pWork->next_proc , NULL );
		pWork->next_proc = 0;
		break;
	}
}

static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;
	SPR_OBJ *spr;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_NEW_ACT :
		pWork->flag |= SK_CHILD_GENERATE;
		// 元に戻す
		spr = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( spr == NULL ){
			return ( -1 );
		}
		SPR_SHOW( spr );
		break;
	case GV_SIGNAL_KILL :
		// prev proc excute
		ExecProc( pWork , SK_PREV_PROC );
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		break;
	}
	return ( 0 );
}


static void Act( Work *pWork )
{
	if ( pWork->flag & SK_CHILD_GENERATE ){
		GV_SetActorChild( pWork , NewDogtagMcMng( pWork->handle_2d , pWork->first_mode ) );
		pWork->first_mode = 1;
		pWork->flag &= ~SK_CHILD_GENERATE;
	}
}

static	void	Die( Work *pWork )
{
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	// sound
	GM_SdSet( SNG_SYUKAN_OFF );
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	int handle;

	handle = -1 ;
	handle = L2D_LoadLayout( STR_SAVE_LOAD , DG_CHANL_MENU , 1, 0 ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		SK_Err("save load l2d\0");
		return -1 ;
	}
	L2D_EvokeAction( handle , STR_DEFAULT );
	work->handle_2d = handle;

	work->flag = SK_CHILD_GENERATE;
	work->first_mode = 0;

	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );
	// gcl 
	work->prev_proc = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み

	// sound
	GM_SdSet( SNG_SYUKAN_ON );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewDogtagMng( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
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

