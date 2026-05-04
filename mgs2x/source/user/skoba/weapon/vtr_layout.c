//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   vtr_sight.c
   テレビサイト
   
   2001/04/19	S.Kobayashi
   $Id: vtr_layout.c,v 1.1.1.3 2002/11/19 11:50:32 Yoshizawa1 Exp $
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

#define		LAYOUT		(124658)	/* vtr.o2d */
#define     STR_DEFAULT (566267)

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	void  ( *act )( struct _work * );
} Work ;

typedef	struct	{
	FMATRIX				world;
	FVECTOR				center;
	FVECTOR				disp_pos;
	FVECTOR				target_pos;
	SPR_POS				sight_pos;
	float				scale;
	TARGET				*lockonTarget;
	TARGET				*athor_lock;
	int					newlock;
	int                 seeker_on;
} ScrPad ;

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	WORLD		(&(SCRPAD->world))
#define	CENTER		(&(SCRPAD->center))
#define	DISP_POS	(&(SCRPAD->disp_pos))
#define	TARGET_POS	(&(SCRPAD->target_pos))
#define	SIGHT_POS	(&(SCRPAD->sight_pos))
#define	SCALE		(SCRPAD->scale)
#define	LOCKONTRG	(SCRPAD->lockonTarget)
#define	ATHOR_LOCK	(SCRPAD->athor_lock)
#define NEWLOCK		(SCRPAD->newlock)
#define SEEKER_ON	(SCRPAD->seeker_on)

// プロトタイプ
extern void *NewRasterEffect( int alpha, int color );
static int AnimetionAct( Work * ); // アニメーション

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

#define STR_ROOT (2770484)
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

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_VTR ) ){
		invisible( pWork );
		return;
	}
 	pWork->act( pWork );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num ){
		L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
		pWork->action++;
	} else {
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

	GV_SetActorChild( work , NewRasterEffect( SCE_GS_SET_ALPHA(  0 , 1 , 0 , 1 , 0x00 ) , 0x301f1f10 ) );

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewVtrSight( void )
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
