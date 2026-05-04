//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   albam.c
   アルバムモード
   
   2001/06/07	S.Kobayashi
   $Id: mc_mng.c,v 1.1.1.3 2002/11/19 11:50:20 Yoshizawa1 Exp $
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
#define LOADGAME_RESOURCE (0x00873e66)		/* ロードゲームリソース */
// 外部l2d
#define		LAYOUT2		   (4691731)
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

	
typedef	struct _mc_mng_work {
	GV_ACT_EX			actor;
	MCScrWork           mcscr; // メモりーカード
	int                 handle_2d;
	int                 handle_out_2d;
	int                 name;
	int                 flag;
	int                 port;

	void                *font_work;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_DESTROY = 0x1 ,
	SK_NEW_ACT  = 0x2 ,
};

// my
extern void *NewAlbamMode( MCScrWork *mcscr , int port , int handle , int handle_out );

// mc
extern void *NewPortsel(int name,int parent_name,int l2d_handle,
						int title_mode,int start_mode,void *strman,int resname);
// etc
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );

#define STR_PORTSEL_NAME (7665079)
#define STR_HOST_NAME (9709491)

static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_DESTROY :
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_NEW_ACT , 0 );
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
//	SPR_OBJ *spr;
    int n_msg ;

    n_msg = GV_ReceiveMessage( pWork->name , &msg );
    while( n_msg > 0 ){
		int sender = *( msg->message + 0 );
		int arg = *( msg->message + 1 );
		int port = *( msg->message + 2 );

		printf("mc_mng : Recieve Message : %d %d %d\n",sender,arg,port);

		if( sender == STR_PORTSEL_NAME ){
			switch( arg ){
			case 1 :
			  printf("test = %d\n" , MCManExactNFiles() );
				if ( MCManExactNFiles() != 0 ){
					// ポート番号の受取
					pWork->mcscr.step = MCSCR_SEL_FILE;
					pWork->mcscr.port = port;
					pWork->port = port;
				}
				break;
			case 2 :
				printf("kill\n");
				// 親に送信
				GV_CallParentSignalFunc( pWork , GV_SIGNAL_KILL , 0 );
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
	switch ( pWork->flag ){
	case 0 :
		MsgChack( pWork );
		if ( pWork->mcscr.step == MCSCR_SEL_FILE ){
			if ( MCManCheckingOrChecked() != 0 ){
				// 親に送信
				GV_CallParentSignalFunc( pWork , SK_NEW_ACT , 0 );
				GV_DestroyActor( pWork );
			} else {
				GV_SetActorChild( pWork , NewAlbamMode( &pWork->mcscr , pWork->port , pWork->handle_2d , pWork->handle_out_2d ) );
				pWork->flag = 1;
			}
		}
		break;
	}
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
}

static	void	Die( Work *pWork )
{
	if ( pWork->mcscr.data != NULL ){
		GV_DelayedFree( pWork->mcscr.data );
	}
	if ( pWork->handle_out_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_out_2d );
	}
}

#if 0	// MODIFY M.Kobayashi 2002/09/10
#define SK_ALBUM_JAPAN   (8863334) /* ロードゲームリソース （アメリカと同じ） */
#define SK_ALBUM_ENGLISH (8863334) /* ロードゲームリソースアメリカ */
#define SK_ALBUM_FRANCE  (1890613) /* ロードゲームリソースフランス語 */
#define SK_ALBUM_GERMAN  (101055)   /* ロードゲームリソースドイツ語 */
#define SK_ALBUM_ITARY   (5993140)  /* ロードゲームリソースイタリア語 */
#define SK_ALBUM_SPAIN   (1162608) /* ロードゲームリソーススペイン語 */
static int LangChecge( void ) // 言語切替関数
{
	int tmp;

	if ( GM_Language == GM_LANG_JAPANESE ){
		tmp = SK_ALBUM_JAPAN;
#ifdef DEBUG_MODE
		printf("Japanese\n");
#endif
	} else if ( GM_Language == GM_LANG_ENGLISH ){
		tmp = SK_ALBUM_ENGLISH;
#ifdef DEBUG_MODE
		printf("English\n");
#endif
	} else if ( GM_Language == GM_LANG_FRENCH ){
		tmp = SK_ALBUM_FRANCE;
#ifdef DEBUG_MODE
		printf("French\n");
#endif
	} else if ( GM_Language == GM_LANG_GERMANY ){
		tmp = SK_ALBUM_GERMAN;
#ifdef DEBUG_MODE
		printf("Germany\n");
#endif
	} else if ( GM_Language == GM_LANG_ITALY ){
		tmp = SK_ALBUM_ITARY;
#ifdef DEBUG_MODE
		printf("Italy\n");
#endif
	} else if ( GM_Language == GM_LANG_SPANISH ){
		tmp = SK_ALBUM_SPAIN;
#ifdef DEBUG_MODE
		printf("Spanish\n");
#endif
	} else {
		tmp = SK_ALBUM_JAPAN;
	}
	return ( tmp );
}
#else	

#define J_LOADGAME_RESOURCE		0x00f8a329		/* ロードゲームリソース日本語 */
#define E_LOADGAME_RESOURCE		0x00c44fd2		/* ロードゲームリソース英語 */
#define F_LOADGAME_RESOURCE		0x001cd935		/* ロードゲームリソースフランス語 */
#define G_LOADGAME_RESOURCE		0x00018abf		/* ロードゲームリソースドイツ語 */
#define I_LOADGAME_RESOURCE		0x005b72b4		/* ロードゲームリソースイタリア語 */
#define S_LOADGAME_RESOURCE		0x0011bd70		/* ロードゲームリソーススペイン語 */

static const int loadgame_resource_name[]={
	E_LOADGAME_RESOURCE,
	F_LOADGAME_RESOURCE,
	G_LOADGAME_RESOURCE,
	I_LOADGAME_RESOURCE,
	S_LOADGAME_RESOURCE,
	0, //韓国語
	J_LOADGAME_RESOURCE,
};

static int LangChecge( void ) // 言語切替関数
{
	return loadgame_resource_name[GM_Language-GM_LANG_ENGLISH];
}

#endif

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work , int handle_2d , int first_start )
{
	int handle;

	// album l2d
	handle = -1 ;
	handle = L2D_LoadLayout( LAYOUT2 , DG_CHANL_MENU , 1, 0 ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_out_2d = handle ;
	// default action
	L2D_EvokeAction( work->handle_out_2d , STR_DEFAULT );

	work->act = (void *)NormalAct;
	work->flag = 0;
	work->port = 0;
	work->handle_2d = handle_2d;
	// 確保
	work->mcscr.data = GV_Malloc( sizeof( short ) * JPEG_SIZE_LIMIT );
	if ( work->mcscr.data == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( work->mcscr.data , sizeof( short ) * JPEG_SIZE_LIMIT );
	// memory card 
   // TODO: PHOTE_SAVE get rid of this fix file size logic and use varaible size files
	MCScrInit( &( work->mcscr ),MCSCR_MODE_LOADPHOTO,
			  work->mcscr.data, JPEG_SIZE_LIMIT,
			  NULL ,THUMB_W * THUMB_H * sizeof( short ),

			  "XXXX" "XXXX" "XXXX" "XXXX");

	work->name = STR_HOST_NAME;

	work->font_work = NewTextScreenControl();
	if ( work->font_work == NULL ){
		return (-1);
	}
	MENU_ClearTextTexture( work->font_work );
	GV_SetActorChild( work , work->font_work );

	GV_SetActorChild( work , NewPortsel( STR_PORTSEL_NAME , STR_HOST_NAME , handle_2d , 2 , first_start , work->font_work , LangChecge() ) );

	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );

	return 0 ;
}

/*----------------------------------------------------------------*/
#ifdef PSX2

void *NewAlbumMcMng( int handle , int first_start )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 24 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work , handle , first_start ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

#endif
