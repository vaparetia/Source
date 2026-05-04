//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dg_cam.c
   デジタルカメラマネージャ
   
   2001/05/11 S.Kobayashi
   $Id: demo_dgcam_mng.c,v 1.1.1.3 2002/11/19 11:50:15 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
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

#include	"memlist.h"
#include	"jpeg.h"
#include	"capture.h"

/*------------------------------------------------------------------*/
#define SHUTER_FLAG     ( 0x1 )
#define VISIBLE_FLAG    ( 0x2 )
#define INVISIBLE_FLAG  ( 0x4 )
#define FONT_ALPHA_FLAG ( 0x8 )
#define FONT_FADE_OUT_FLAG ( 0x10 )
#define SHUTER_OPEN_FLAG ( 0x20 )
// こば４追加
enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
};

enum {
	SK_ANDER_STAY       = 0x1,
	SK_TANKER_FRAME     = 0x2,
	SK_SHUTER_ON        = 0x8,
	SK_SHUTER_OPEN      = 0x10,
	SK_DEMO_FONT_FADE_OUT = 0x20,
	SK_SHUTER_READY     = 0x40,
	SK_DEMO_FONT_ALPHA  = 0x80,
};

enum {
    STATE_NORMAL = 0,
    STATE_SHUTTER_ON,
	STATE_SAVE ,
	STATE_READY ,
} ;

/*------------------------------------------------------------------*/

typedef	struct _work {
    GV_ACT_EX	actor ;

    u_short		state ;
    u_short		time ;
	int         name;
	int         frame_number; // タンカー格納場所
	void        ( * act )( struct _work * );
} Work ;

extern void	*NewDEMODgCamSight( float , float  );
extern void	*NewDEMODgCamShuterSight( void );
/*------------------------------------------------------------------*/
// 受信
static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_SHUTER_OPEN :
		GV_CallChildSignalFunc( pWork , 0x02 , SK_INVISIBLE ); // 非表示
		GM_SeSet3D( 0x00, GM_MAX_VOL, SD_I_CAMERA02, -M_PI/2 ) ;	
		break;
	case SK_SHUTER_READY : // 強制解除
		pWork->state = STATE_NORMAL ;
		break;
	case SK_TANKER_FRAME :
		pWork->frame_number = value ;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

// メッセージを受けとるとシャッターON
static int MsgChack( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;
	int         name;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == SHUTER_FLAG ){
			GV_CallChildSignalFunc( pWork , SK_SHUTER_ON , 0 );
			GV_SetActorChild( pWork , NewDEMODgCamShuterSight() );
			pWork->state = STATE_SHUTTER_ON ;
		}
		if( num == VISIBLE_FLAG ){
			GM_ResetSightStatus( SGT_Invisible );
		}
		if( num == INVISIBLE_FLAG ){
			GM_SetSightStatus( SGT_Invisible );
		}
		if ( num == FONT_ALPHA_FLAG ){
			GV_CallChildSignalFunc( pWork , SK_DEMO_FONT_ALPHA , msg->message[ 1 ] );
		}
		if ( num == FONT_FADE_OUT_FLAG ){
			GV_CallChildSignalFunc( pWork , SK_DEMO_FONT_FADE_OUT , msg->message[ 1 ] );
		}
		if ( num == SHUTER_OPEN_FLAG ){
			GV_CallChildSignalFunc( pWork , 0x02 , SK_VISIBLE ); // 表示
		}			
		msg--;
	}
	return ( 0 );
}


/*------------------------------------------------------------------*/
static void Act( Work *work )
{
    switch ( work->state ) {
    case STATE_NORMAL :
		GV_CallChildSignalFunc( work , 0x02 , SK_VISIBLE );	 // 表示
		break ;
    case STATE_SHUTTER_ON :
		break;
    case STATE_SAVE :
		break;
    case STATE_READY : // メッセージ受取まち
		GV_CallChildSignalFunc( work , 0x02 , SK_VISIBLE );	 // 表示
		break;
    }
	MsgChack( work );
}

static	void	Die( work )
Work		*work ;
{
}

/*------------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
    /* カメラ起動 */
    work->state = STATE_NORMAL ;
	work->time = 0;
	// シグナルの登録
	GV_SetActorSignalFunc( work , SignalFunc );

	// サイトの起動
	GV_SetActorChild( work , NewDEMODgCamSight( 0 , 0 ) );

    return 0 ;
}

/*------------------------------------------------------------------*/
void *NewDEMODigitalCamera( int name )
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WPMNG_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->name = name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}
