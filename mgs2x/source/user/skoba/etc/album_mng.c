//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   albam.c
   アルバムモード
   
   2001/06/07	S.Kobayashi
   $Id: album_mng.c,v 1.1.1.3 2002/11/19 11:50:18 Yoshizawa1 Exp $
*/

/* メモリーカードがやばいかも */

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
	
typedef	struct _work {
	GV_ACT_EX			actor;
	MCMAN_WORK          mcman; // メモりーカード
	int                  name;
    int           first_start;
	int             handle_2d;

	int             prev_proc;
	int             next_proc;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_DESTROY = 0x1 ,
	SK_NEW_ACT  = 0x2 ,
};

enum {
	SK_PREV_PROC = 0x1 ,
	SK_NEXT_PROC = 0x2,
};


// my
extern void *NewAlbumMcMng( int , int );

static inline void ExecProc( Work *pWork , char mode )
{
	switch ( mode ){
	case SK_PREV_PROC :
		if ( pWork->prev_proc == 0 ){
			return;
		}
		GM_ExecProc( pWork->prev_proc , NULL );
		break;
	case SK_NEXT_PROC :
		if ( pWork->next_proc == 0 ){
			return;
		}
		GM_ExecProc( pWork->next_proc , NULL );
		break;
	}
}

static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_NEW_ACT :
		GV_SetActorChild( pWork , NewAlbumMcMng( pWork->handle_2d , pWork->first_start ) );
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

	// memory card
	NewMCMan( &work->mcman );
	work->mcman.file_kind = MCMAN_FILE_KIND_PHOTO;
	GV_SetActorChild( work , &work->mcman );
	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );

	// port 
	handle = -1 ;
	handle = L2D_LoadLayout( STR_SAVE_LOAD , DG_CHANL_MENU , 0, 0 ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		return -1 ;
	}
	L2D_EvokeAction( handle , STR_DEFAULT );
	work->handle_2d = handle;

	GV_SetActorChild( work , NewAlbumMcMng( work->handle_2d , 0 ) );
	work->first_start = 1;
	// gcl 
	work->prev_proc = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み
	work->next_proc = GCL_GetOptionValue( 'n' , 0 ); // procの読み込み

	// sound
	GM_SdSet( SNG_SYUKAN_ON );

	return 0 ;
}

/*----------------------------------------------------------------*/
#ifdef BP_360
extern void bp_set_current_savegame( const char * const dirsuffix );
int gBP_CanSelectNewDevice;
#endif

extern void MGS_SaveStatus_ClearWrongUser();

#ifdef PSX2
void *NewAlbumMng( int name )
{
	Work		*work ;
#ifdef BP_360
   bp_set_current_savegame( "P" );
   gBP_CanSelectNewDevice = 1;
#endif

   MGS_SaveStatus_ClearWrongUser();

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

#endif
