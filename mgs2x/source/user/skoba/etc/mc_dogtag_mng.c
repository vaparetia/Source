//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   mc_dogtag_mng.c
   アルバムモード
   
   2001/07/04	S.Kobayashi
   $Id: mc_dogtag_mng.c,v 1.1.1.3 2002/11/19 11:50:20 Yoshizawa1 Exp $
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

#define		LAYOUT		   (4691731)
#define 	STR_NODE_FONT  (10045112)
#define     STR_CURS       (6756848)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)
#define 	STR_R_BAR      (5313689)
#define 	STR_G_BAR      (10556568)
#define		STR_B_BAR      (5400341)
#define 	STR_RGB_SELECT (15807010)
#define 	STR_SAVE_LOAD (10350292)
// out layout
#define		LAYOUT2		  (2448726)

// size
#define JPEG_ENCODE_SIZE_LIMIT ( 30720 ) // 30 * 1024
#define JPEG_SIZE_LIMIT		( 512 * 448 * 2 * sizeof(short) ) //( 24*1024 * sizeof(short) )
#define CAPTURE_MEM_SIZE	(3*1024*1024)
// kanoさん
#define N_FILES			MCSCR_N_FILES
#define NO_SAVEFILE		0xffff

enum {
	MCSCR_CHECK_CARD=0,
	MCSCR_GET_CARDINFO,
	MCSCR_WAIT_GET_CARDINFO,
	MCSCR_LIST_FILE,  // by koba4
	MCSCR_SEL_FILE,

	MCSCR_CONFIRM,
	MCSCR_START_SAVELOAD,
	MCSCR_WAIT_SAVELOAD,

	MCSCR_SUCCESS_SAVELOAD,
	MCSCR_FAILED_SAVELOAD,
};
/////////////////////////////////

	
typedef	struct _work {
	GV_ACT_EX			actor ;
	int                 handle_2d;
	int                 handle_out;
	int                 name;
	int                 flag;
	int                 prev_proc;
	int                 first_mode;

	void                *font_work; // system font 

	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_DESTROY = 0x1 ,
	SK_NEW_ACT  = 0x2 ,
	SK_CHILD_GENERATE = 0x4 ,
	SK_MESSAGE_CHECK = 0x8 ,
	SK_LAYOUT_RERESE = 0x10 ,
	SK_MEMORY_MNG_ON = 0x20 ,
	SK_LAYOUT_KILL_SET = 0x400 ,
};

enum {
	SK_PREV_PROC = 0x1 ,
	SK_NEXT_PROC = 0x2,
};

// my
extern void *NewDogtagMode( void *font_work , int handle_out );
// mc
extern void *NewLoadGameScrForDogtag(int name,int parent_name,int l2d_handle,void *strman , int );
// etc
extern void *NewTextScreenControlForTitle( void );
extern void MENU_ClearTextTexture( void * );

#define STR_PORTSEL_NAME (7665079)
#define STR_HOST_NAME (9709491)

static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;
	SPR_OBJ *spr;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_DESTROY :
		GV_CallParentSignalFunc( pWork , SK_NEW_ACT , 0 );
		GV_DestroyActor( pWork );
		break;
	case SK_LAYOUT_RERESE :
		if ( pWork->flag & SK_LAYOUT_RERESE ){
			spr = L2D_GetObject( pWork->handle_2d , STR_ROOT );
			if ( spr == NULL ){
				return ( -1 );
			}
			SPR_HIDE( spr );
			pWork->flag &= ~SK_LAYOUT_RERESE;
		}
		break;
#if 0
	case SK_LAYOUT_RERESE :
		if ( pWork->flag & SK_LAYOUT_RERESE ){
			if ( pWork->handle_2d >= 0 ){
				L2D_ReleaseLayout( pWork->handle_2d );
				pWork->handle_2d = -1;
				pWork->flag &= ~SK_LAYOUT_RERESE;
			}
		}
		break;
#endif
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		break;
	}
	return ( 0 );
}

static void MsgChack( Work *pWork )
{
    GV_MSG *msg;
    int n_msg ;

    n_msg = GV_ReceiveMessage( pWork->name , &msg );

    while( n_msg > 0 ){
		int sender = *( msg->message + 0 );
		int arg = *( msg->message + 1 );

		if( sender == STR_PORTSEL_NAME ){
			switch( arg ){
			case 1 :
				pWork->flag |= SK_CHILD_GENERATE | SK_LAYOUT_RERESE;
				pWork->flag &= ~SK_MESSAGE_CHECK;
				break;
			case 2 :
				// 親に送信
				GV_CallParentSignalFunc( pWork , GV_SIGNAL_KILL , 0 );
				// 戻る
				GV_DestroyActor( pWork );
				break;
			}
		}
		msg++;
		n_msg--;
    }
}


static void NormalAct( Work *pWork )
{
	if ( pWork->flag & SK_MEMORY_MNG_ON ){
		GV_SetActorChild( pWork , NewLoadGameScrForDogtag( STR_PORTSEL_NAME , STR_HOST_NAME ,
														   pWork->handle_2d , pWork->font_work , pWork->first_mode  ) );
		pWork->flag |= SK_MESSAGE_CHECK;
		pWork->flag &= ~SK_MEMORY_MNG_ON;
	}
	if ( pWork->flag & SK_MESSAGE_CHECK ){
		MsgChack( pWork );
	}
	if ( pWork->flag & SK_CHILD_GENERATE ){
		GV_SetActorChild( pWork , NewDogtagMode( pWork->font_work , pWork->handle_out ) );
		pWork->flag &= ~SK_CHILD_GENERATE;
	}
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
}

static	void	Die( Work *pWork )
{
	if ( pWork->handle_out >= 0 ){
		L2D_ReleaseLayout( pWork->handle_out );
		pWork->handle_out = -1;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	// dogtag
	work->handle_out = -1 ;
	work->handle_out = L2D_LoadLayout( LAYOUT2 , DG_CHANL_MENU , 0 ,  0 ) ;
	if ( work->handle_out < 0 ){
		SK_Err("dogtag l2d\0");
		return -1 ;
	}
	// default action
	L2D_EvokeAction( work->handle_out , STR_DEFAULT );

	work->act = (void *)NormalAct;
	work->flag = SK_MEMORY_MNG_ON;
	work->name = STR_HOST_NAME;

	work->font_work = NewTextScreenControlForTitle();
	if ( work->font_work == NULL ){
		return (-1);
	}
	MENU_ClearTextTexture( work->font_work );
	GV_SetActorChild( work , work->font_work );
	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewDogtagMcMng( int handle , int first_mode ) // マネージャ本体
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->first_mode = first_mode;
	work->handle_2d = handle;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

