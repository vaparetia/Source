//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   vtr_sight.c
   テレビサイト
   
   2001/04/19	S.Kobayashi
   $Id: psg_layout.c,v 1.1.1.3 2002/11/19 11:50:29 Yoshizawa1 Exp $
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

#define		LAYOUT		(118471)	/* psg.o2d */
#define     STR_SIGHT   (6695291)
#define     STR_PSGBG   (13277809)   // psgBgTex
#define     STR_DEFAULT (566267)
#define 	STR_ROOT    (2770484)

#define 	DEMO_FLAG       (0x1)
#define     VISIBLE_FLAG    (0x2)
#define 	INVISIBLE_FLAG  (0x4)
#define     DEFAULT_Y       (-26)

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
};

typedef	struct _work {
	GV_ACT_EX			actor ;
	SPR_OBJ             *sight;
	SPR_OBJ             *psgbg;
	SVECTOR             cam_rot;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	int                 flag;
	int                 name;
	int                 mode;
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション
// extern 
extern void *NewScrSightMorph( int mode, int camera_num );

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

// メッセージ
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
		if( num == VISIBLE_FLAG ){
			GM_ResetSightStatus( SGT_Invisible );
		}
		if( num == INVISIBLE_FLAG ){
			GM_SetSightStatus( SGT_Invisible );
		}
		msg--;
	}
	return ( 0 );
}

static int Initialize( Work *pWork )
{
	GM_CameraSet *pCam;
	float tmp_width , tmp_height;

#if 0 /*yano 2002.03.19*/
	tmp_width = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 );
#else
	tmp_width = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	pWork->sight = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_SIGHT );
	if ( pWork->sight == NULL ){
		SK_Err( "sight\0" );
		return ( -1 );
	}
	pWork->psgbg = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_PSGBG );
	if ( pWork->psgbg == NULL ){
		SK_Err( "psgbg\0" );
		return ( -1 );
	}

	pWork->cam_rot = pCam->rotate;

	return ( 0 );
}

#define CENTER_MAX 5
#define CENTER_SPEED_X 3
#define CENTER_SPEED_Y 3
static void SightMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	FVECTOR    old_pos;
	float         ftmp;
	float    tmp_width;
	float   tmp_height;
	float            r;
	int           sign;

#if 0 /*yano 2002.03.19*/
	tmp_width  = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - 32;
#else
	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif
	old_pos.vx = pWork->sight->empty.pos.x;
	old_pos.vy = pWork->sight->empty.pos.y;
	old_pos.vz = old_pos.vw = 0;
	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );
	stmp.vy *= ( 360.f / 4069.f );
	pWork->cam_rot = pCam->rotate;

	r = 1000.f;
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360 );
	stmp.vz  = ( ( ftmp * stmp.vy ) / 360 );
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
	// スピード補正
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_SPEED_Y ){
			stmp.vw = -CENTER_SPEED_Y;
		}
	} else {
		if ( stmp.vw > CENTER_SPEED_Y ){
			stmp.vw = CENTER_SPEED_Y;
		}
	}
	pWork->sight->empty.pos.x += ( int )( stmp.vz );
	pWork->sight->empty.pos.y -= ( int )( stmp.vw );
	// 範囲ない制御
	if ( stmp.vz != 0 ){
		if ( pWork->sight->empty.pos.x < -CENTER_MAX ){
			pWork->sight->empty.pos.x = -CENTER_MAX;
		}
		if ( pWork->sight->empty.pos.x > CENTER_MAX ){
			pWork->sight->empty.pos.x = CENTER_MAX;
		}
	}
	// 範囲ない制御
	if ( stmp.vw != 0 ){
		if ( pWork->sight->empty.pos.y < -CENTER_MAX ){
			pWork->sight->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->sight->empty.pos.y > CENTER_MAX ){
			pWork->sight->empty.pos.y = CENTER_MAX;
		}
	}
	// 戻す
	if ( stmp.vz == 0 && stmp.vw == 0 ){
		stmp.vx = pWork->sight->empty.pos.x;
		stmp.vy = pWork->sight->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->sight->empty.pos.x += fvec.vx;
		pWork->sight->empty.pos.y += fvec.vy;
	}
	// BGの制御
	{
		FVECTOR fvtmp;

		fvtmp.vx = pWork->sight->empty.pos.x;
		fvtmp.vy = pWork->sight->empty.pos.y;
		fvtmp.vz = fvtmp.vw = 0;
		_sceVu0SubVector( &old_pos , &fvtmp , &old_pos );
		pWork->psgbg->empty.pos.x += old_pos.vx;
		pWork->psgbg->empty.pos.y += old_pos.vy;
	}
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
	NewScrSightMorph( pWork->mode, 0 );
	if ( GM_CheckSightStatus( SGT_Psg1 ) ){
		invisible( pWork );
		return;
	}
	// デモ等のサイトの表示非表示管理
	if ( SightVisibleInvisible( pWork->handle_2d , STR_ROOT ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
		pWork->mode = 0;
	} else {
		pWork->flag |= SK_INVISIBLE;
		pWork->mode = 1;
	}
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return;
	}
 	pWork->act( pWork );
}

static int NormalAct( Work *pWork )
{
	MsgChack( pWork );
	SightMove( pWork );

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	SPR_OBJ     *pRoot;

	if ( pWork->action < pWork->action_num ){
		L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
		if ( pWork->name != 0 && pWork->action == 1 ){ // demo mode
			pRoot = L2D_GetObject( pWork->handle_2d, STR_ROOT );
			if ( pRoot == NULL ){
				SK_Err( "pRoot\0" );
				return ( -1 );
			}
			pRoot->empty.pos.y += DEFAULT_Y;
		}
		pWork->action++;
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		if ( Initialize( pWork ) < 0 ){
			return ( -1 );
		}
		pWork->act = ( void * )NormalAct;
	}
	return ( 0 );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d ) ;
	}
	NewScrSightMorph( -1 , 0 );
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
	work->flag = SK_INITIALIZE_OK;
	work->mode = 0;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewPsg1Sight( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
