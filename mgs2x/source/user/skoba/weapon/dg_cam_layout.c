//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dg_cam_layout_sight.c
   デジタルカメラサイト
   
   2001/04/19	S.Kobayashi
   $Id: dg_cam_layout.c,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
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

#include "bp_vector.h"

#define		LAYOUT	     	 (105677)	/* dcm.o2d */
#define     STR_CENTER       (3367603)//9100063)
#define     STR_CENTER_CURS  (3481206)
#define     STR_CENTER_CURS1 (5989410)
#define     STR_S_CURS       (2319263)
#define     STR_DEFAULT      (566267)
#define     STR_HORIZON      (13039795)
#define 	STR_ROOT         (2770484)

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
};

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	SPR_OBJ             *horizon;      // 枠エフェクト
	SPR_OBJ             *center;      // 真中親オブジェクト
	SPR_OBJ             *center_s_curs;      // 真中オブジェクト
	SPR_OBJ             *center_curs; // 回転オブジェクト
	float                center_curs1_scale; // 回転しない方の拡縮
	float                cam_scale_bak; // 1フレーム前のカメラのスケール値
	SVECTOR              cam_rot;
	float                angle_max;
	float                angle_min;
	char                 visible_flag;
	char                 hold_flag;
	int                  flag;
	void                 *menuprint_work_ptr;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_ANDER_STAY = 0x1 ,
	SK_SHUTER_ON        = 0x8,
	SK_SHUTER_OPEN      = 0x10,
	SK_SHUTER_FULL_OPEN = 0x20,
	SK_SHUTER_READY = 0x40,
	SK_ANDER_MOVING = 0x80,
};

enum {
	SK_HOLD_OFF = 0 ,
	SK_HOLD_ON,
};

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション

// 子アクター
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;
	SPR_OBJ *pRoot;

	pWork = ( Work * )workp;

	switch ( signal ){
	case 0x2 :
		if ( value == SK_INVISIBLE ){
			if ( !( pWork->flag & SK_INVISIBLE ) ){
				pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
				if ( pRoot == NULL ){
					SK_Err( "root\0" );
					return ( -1 );
				}
				SPR_HIDE( pRoot );
			}
			pWork->visible_flag = SK_INVISIBLE;
		} else {
			if ( !( pWork->flag & SK_INVISIBLE ) ){
				pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
				if ( pRoot == NULL ){
					SK_Err( "root\0" );
					return ( -1 );
				}
				SPR_SHOW( pRoot );
			}
			pWork->hold_flag = SK_HOLD_OFF;
			pWork->visible_flag = SK_VISIBLE;
		}
		break;
	case SK_SHUTER_ON : // ホールド状態に持ち込む
		pWork->hold_flag = SK_HOLD_ON;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( 1 );
	}

	return ( 0 );
}

// 初期化関数
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

	pWork->center_curs = L2D_GetObject( pWork->handle_2d, STR_CENTER_CURS );
	if ( pWork->center_curs == NULL ){
		SK_Err( "center_curs\0" );
		return ( -1 );
	}
   // BP FIX: These are correct - no need to break them!
//	pWork->center_curs->spin.center.x = 4;
//	pWork->center_curs->spin.center.y = 44;
	pWork->center_curs->spin.pos.x = 0;
	pWork->center_curs->spin.pos.y = 0;
	pWork->center_s_curs = L2D_GetObject( pWork->handle_2d, STR_S_CURS );
	if ( pWork->center_s_curs == NULL ){
		SK_Err( "center_s_curs\0" );
		return ( -1 );
	}
	pWork->horizon = L2D_GetObject( pWork->handle_2d, STR_HORIZON );
	if ( pWork->horizon == NULL ){
		SK_Err( "horizon\0" );
		return ( -1 );
	}
	pWork->cam_rot = pCam->rotate;

	return ( 0 );	
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 10
static void HorizonMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
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
	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 1000.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360 );
	// スピード補正
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -CENTER_SPEED_X ){
			stmp.vw = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vw > CENTER_SPEED_X ){
			stmp.vw = CENTER_SPEED_X;
		}
	}
	pWork->horizon->empty.pos.y -= ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->horizon->empty.pos.y < -CENTER_MAX ){
			pWork->horizon->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->horizon->empty.pos.y > CENTER_MAX ){
			pWork->horizon->empty.pos.y = CENTER_MAX;
		}
	}
	// 戻す
	if ( stmp.vw == 0 ){
		stmp.vx = 0;
		stmp.vy = pWork->horizon->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->horizon->empty.pos.y += fvec.vy;
	}
}

static void CenterCursor1Move( Work *pWork )
{
	SPR_OBJ *pObj;
	GM_CameraSet *pCam;
	float scale;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	pObj = pWork->center_s_curs;
	
	// 補間
	if ( pWork->cam_scale_bak == pCam->angle ){ // 元に戻す
		scale = ( float )( 1.0f - pWork->center_curs1_scale ) / 4.f;
		pWork->center_curs1_scale += scale;
	} else {
		scale = pCam->angle - pWork->cam_scale_bak; // default 2.0だから
		scale = scale < 0 ? 1.125f : 0.906f;
		pWork->center_curs1_scale += ( float )( scale - pWork->center_curs1_scale ) / 10.f;
		// 拡大
		if ( pWork->center_curs1_scale < 0.906F ){
			pWork->center_curs1_scale = 0.906F;
		}
		if ( pWork->center_curs1_scale > 1.125F ){
			pWork->center_curs1_scale = 1.125F;
		}	
	}
	// 更新
	SPR_MAG( pObj , pWork->center_curs1_scale );
	SPR_SHOW( pObj ) ;
}

#define ROT_MAX ( 30.f * PI / 360.f )
static void CenterCursMove( Work *pWork )
{
	GM_CameraSet *pCam;
	SPR_OBJ *pObj;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR      ftmp3;
	float         ftmp;
	float        ftmp2;
	float            r;
	int           sign;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	pObj = pWork->center_curs;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 1000.f ) * pCam->angle;
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vx  = ( ( ftmp * stmp.vx ) / 360 );
	// スピード補正
	sign = stmp.vx < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vx < -CENTER_SPEED_X ){
			stmp.vx = -CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vx > CENTER_SPEED_X ){
			stmp.vx = CENTER_SPEED_X;
		}
	}
	pObj->spin.spin -= ( stmp.vx * PI / 360 );
	// 範囲ない制御
	ftmp2 = stmp.vx < 0 ? -stmp.vx : stmp.vx;
	if ( stmp.vx != 0 ){
		if ( pObj->spin.spin < -ROT_MAX ){
			pObj->spin.spin = -ROT_MAX;
		}
		if ( pObj->spin.spin > ROT_MAX ){
			pObj->spin.spin = ROT_MAX;
		}
	}
	// 戻す
	if ( stmp.vx == 0 ){
		stmp.vx = pObj->spin.spin;
		stmp.vy = 0;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3 = DG_ZeroVector;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 10 );
		pObj->spin.spin += fvec.vx;
	}
}

static void NumberMove( Work *pWork )
{
	GM_CameraSet *pCam;
	char text_buffer[ 12 ]; // BP FIX - was 10, but that's not enough for VX 4 digits and VY 4 digits + sign
	float parent_x , parent_y;
	int high , low;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	

   // BP FIX
   //parent_x = 298.0f; // PSX2
	//parent_x = 372.5f;
   parent_x = 290.0f;
	parent_y = DIRECT_SCREEN_Y( 221.0f );
	sprintf( text_buffer , "%04d" , ( int )( ( pCam->angle - 1 ) * 100.f) );
	___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
	___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, 60 );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );

   // BP FIX
   //parent_x = 384.0f; // PSX2
   //parent_x = 480.0f;
   parent_x = 405.0f;
   parent_y = DIRECT_SCREEN_Y( 350.0f );
	high = ( int )pCam->rotate.vx % 6000;
   // this produces very odd values for "low" that can be positive or negative but don't loop as expected
	low = ( int )pCam->rotate.vy % 6000;

	sprintf( text_buffer , "%04d %04d" , high , low );
	___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
	___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, 60 );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
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

static int NormalAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
    if ( pWork->hold_flag == SK_HOLD_ON || pWork->flag & SK_INVISIBLE ){
		return ( 1 );
	}
	NumberMove( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return( 1 );
	}
	CenterCursor1Move( pWork );
	CenterCursMove( pWork );
	HorizonMove( pWork );
	// 更新
	pWork->cam_scale_bak = pCam->angle;
	pWork->cam_rot = pCam->rotate;

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num - 1 ){
		if ( L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action ) != L2D_STAT_BUSY ){
			pWork->action++;
		}
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		if ( Initialize( pWork ) < 0 ){
			return ( -1 );
		}
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_ANDER_STAY , 0 );
		pWork->act = (void *)NormalAct;
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_Camera ) ){
		invisible( pWork );
		return;
	}
	// デモ等のサイトの表示非表示管理
	if ( SthingerSightVisibleInvisible( pWork->handle_2d , STR_ROOT , ( pWork->visible_flag & SK_INVISIBLE ? 1 : 0 ) ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
	if ( pWork->visible_flag == SK_INVISIBLE ){
		return;
	}
 	pWork->act( pWork );
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
		return ( -1 );
	}
	work->handle_2d = handle ;
	// etc
 	work->center_curs1_scale = 1.0f;
	work->cam_scale_bak = 2.0f;
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	work->hold_flag = SK_HOLD_OFF;
	work->flag = SK_INITIALIZE_OK;
	
	work->act = (void *)AnimetionAct;

	/* MENU_Printfキャラ生成 */
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) { // demo mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_NORMAL | DG_DMAPACK_INVISIBLE1 | DG_DMAPACK_INVISIBLE2 |
														DG_DMAPACK_INVISIBLE3 | DG_DMAPACK_PRIVILEGE , DG_DMAPACK_PHASE_AFTER , 144 );
	} else { // normal mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_MENU, DG_DMAPACK_PHASE_NORMAL, 0 );
	}
	if ( work->menuprint_work_ptr == NULL ){
		SK_Err("menu print\0");
		return ( -1 );
	}
	GV_SetActorChild( work , work->menuprint_work_ptr );

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewDgCamSight( float min , float max )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->angle_max = max;
	work->angle_min = min;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
