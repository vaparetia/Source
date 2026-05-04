//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   night_sight.c
   ナイトゴーグルサイト
   
   2001/04/27	S.Kobayashi
   $Id: night_layout.c,v 1.2 2002/12/05 18:41:59 takaki Exp $
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
#include "libfs.h"

#define		LAYOUT	     	 (116044)	/* ngl.o2d */
#define     STR_ARROW       (4904541)
#define     STR_CROSS_NULL (16676772)
#define     STR_FIN_1       (9847831)
#define     STR_DEFAULT      (566267)

#define     FIN_MAX ( 5 )
#define     NUMBER_MAX ( 1 )
#define 	NUM_WIDTH (60)

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
};

typedef struct {
	FVECTOR             number_pos;
	FVECTOR             parent_pos;
} Number;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	SPR_OBJ             *arrow;           // 矢印エフェクト
	SPR_OBJ             *cross_null;      // 真中親オブジェクト
	SPR_OBJ             *fin[ FIN_MAX ];           // 一番左オブジェクト
	FVECTOR              fin_pos[ FIN_MAX ];
	Number               Num[ NUMBER_MAX ];
	int                  number;
	int                  flag;
	SVECTOR              cam_rot;
	void  ( *act )( struct _work * );
	void				*menu_print_work ;/* added by K.Takabe 2002.10.03 */
} Work ;

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション

// 子アクター
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

// 初期化関数
static int Initialize( Work *pWork )
{
	GM_CameraSet *pCam;
	float tmp_width , tmp_height;
	int strcode , i;

#if 0 /*yano 2002.03.19*/
	tmp_width = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 );
#else
	tmp_width = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	pWork->arrow = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_ARROW );
	if ( pWork->arrow == NULL ){
		SK_Err( "arrow\0" );
		return ( -1 );
	}
	pWork->cross_null = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_CROSS_NULL );
	if ( pWork->cross_null == NULL ){
		SK_Err( "cross_null\0" );
		return ( -1 );
	}
	strcode = STR_FIN_1;
	for ( i = 0 ; i < FIN_MAX ; i++ ){
		pWork->fin[ i ] = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, strcode );
		if ( pWork->fin[ i ] == NULL ){
			SK_Err( "fin\0" );
			return ( -1 );
		}
		strcode++;
	}
	// fin
	for ( i = 0 ; i < FIN_MAX ; i++ ){
		pWork->fin_pos[ i ].vx = 0.f;
		pWork->fin_pos[ i ].vy = pWork->fin[ i ]->head.child->line.pos[ 0 ].y / 2.f;
		pWork->fin_pos[ i ].vz = 0.f;
		pWork->fin_pos[ i ].vw = 0.f;
	}
	// 数字
	for ( i = 0 ; i < NUMBER_MAX ; i++ ){
		pWork->Num[ i ].number_pos.vx = 0;//-NUM_WIDTH + ( i * 24 );
	}
	// rotの初期化
	pWork->cam_rot = pCam->rotate;

	return ( 0 );	
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 10
static void ArrowMove( Work *pWork )
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

	r = 50.f;
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
	pWork->arrow->empty.pos.y -= ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->arrow->empty.pos.y < -CENTER_MAX ){
			pWork->arrow->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->arrow->empty.pos.y > CENTER_MAX ){
			pWork->arrow->empty.pos.y = CENTER_MAX;
		}
	}
	// 戻す
	if ( stmp.vw == 0 ){
		stmp.vx = 0;
		stmp.vy = pWork->arrow->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->arrow->empty.pos.y += fvec.vy;
	}
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 10
#define CENTER_SPEED_Y 10
static void CrossMove( Work *pWork )
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
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );
	stmp.vy *= ( 360.f / 4069.f );

	r = 50.f;
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vy ) / 360 );
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360 );
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
	pWork->cross_null->empty.pos.x -= ( int )( stmp.vz );
	pWork->cross_null->empty.pos.y += ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		if ( pWork->cross_null->empty.pos.x < -CENTER_MAX ){
			pWork->cross_null->empty.pos.x = -CENTER_MAX;
		}
		if ( pWork->cross_null->empty.pos.x > CENTER_MAX ){
			pWork->cross_null->empty.pos.x = CENTER_MAX;
		}
	}
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->cross_null->empty.pos.y < -CENTER_MAX ){
			pWork->cross_null->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->cross_null->empty.pos.y > CENTER_MAX ){
			pWork->cross_null->empty.pos.y = CENTER_MAX;
		}
	}
	// 戻す
	if ( stmp.vz == 0 && stmp.vw == 0 ){
		stmp.vx = pWork->cross_null->empty.pos.x;
		stmp.vy = pWork->cross_null->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->cross_null->empty.pos.x += fvec.vx;
		pWork->cross_null->empty.pos.y += fvec.vy;
	}
}

static void Fin1Move( Work *pWork ) // 左
{
	GM_CameraSet *pCam;
	SPR_OBJ *pLine;
	FVECTOR ftmp , fvec;
	float   tmp , tmp2 , r;
	int    sign;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	

	tmp = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	tmp *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 100.f );
	tmp2 = 2 * M_PI * r; // 31半径のこと(円周をだす)
	tmp  = ( ( tmp2 * tmp ) / 360 );
	// スピード補正
	sign = tmp < 0.f ? -1 : 1;
	if ( sign < 0 ){
		if ( tmp < -CENTER_SPEED_X ){
			tmp = -CENTER_SPEED_X;
		}
	} else {
		if ( tmp > CENTER_SPEED_X ){
			tmp = CENTER_SPEED_X;
		}
	}
	pWork->fin_pos[ 0 ].vy += tmp;
	// 目標に移動
	pLine = pWork->fin[ 0 ]->head.child;
	ftmp = DG_ZeroVector;
	ftmp.vy = pLine->line.pos[ 0 ].y;
	_sceVu0SubVector( &fvec , &pWork->fin_pos[ 0 ] , &ftmp ); // 加速度
	_sceVu0DivVector( &fvec , &fvec , 20 );
	pLine->line.pos[ 0 ].y += fvec.vy;
	if ( pLine->line.pos[ 0 ].y > 290 ){
		pWork->fin_pos[ 0 ].vy = 90 * 2;
		pLine->line.pos[ 0 ].y = 290;
	}
	if ( pLine->line.pos[ 0 ].y < 90 ){
		pWork->fin_pos[ 0 ].vy = 290 / 2;
		pLine->line.pos[ 0 ].y = 90;
	}
	pLine->line.pos[ 1 ].y = pLine->line.pos[ 0 ].y; 
	fvec.vy = pLine->line.pos[ 0 ].y;
	// tateの終点をいじる
	pLine = pLine->head.next;
	pLine->line.pos[ 0 ].y = fvec.vy;
}

static void Fin4Move( Work *pWork ) // 上
{
	GM_CameraSet *pCam;
	SPR_OBJ *pLine;
	FVECTOR ftmp , fvec;
	float   tmp , tmp2 , r;
	int    sign;

	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	

	tmp = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	tmp *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 100.f );
	tmp2 = 2 * M_PI * r; // 31半径のこと(円周をだす)
	tmp  = ( ( tmp2 * tmp ) / 360 );
	// スピード補正
	sign = tmp < 0.f ? -1 : 1;
	if ( sign < 0 ){
		if ( tmp < -CENTER_SPEED_X ){
			tmp = -CENTER_SPEED_Y;
		}
	} else {
		if ( tmp > CENTER_SPEED_Y ){
			tmp = CENTER_SPEED_Y;
		}
	}
	pWork->fin_pos[ 3 ].vy += tmp;
	// 目標に移動
	pLine = pWork->fin[ 3 ]->head.child;
	ftmp = DG_ZeroVector;
	ftmp.vy = pLine->line.pos[ 0 ].y;
	_sceVu0SubVector( &fvec , &pWork->fin_pos[ 3 ] , &ftmp ); // 加速度
	_sceVu0DivVector( &fvec , &fvec , 20 );
	pLine->line.pos[ 0 ].y += fvec.vy;
	if ( pLine->line.pos[ 0 ].y > 290 ){
		pWork->fin_pos[ 3 ].vy = 290 / 2;
		pLine->line.pos[ 0 ].y = 290;
	}
	if ( pLine->line.pos[ 0 ].y < 90 ){
		pWork->fin_pos[ 3 ].vy = 90 * 2;
		pLine->line.pos[ 0 ].y = 90;
	}
	pLine->line.pos[ 1 ].y = pLine->line.pos[ 0 ].y; 
	fvec.vy = pLine->line.pos[ 0 ].y;
	// tateの終点をいじる
	pLine = pLine->head.next;
	pLine->line.pos[ 0 ].y = fvec.vy;
}

static void Fin3Move( Work *pWork ) // 合成
{
	SPR_OBJ *pLine;
	SPR_OBJ *pLine2;
	SPR_OBJ *pLine3;
	FVECTOR ftmp , fvec;

	// 目標に移動
	pLine = pWork->fin[ 0 ]->head.child;
	pLine2 = pWork->fin[ 1 ]->head.child;
	pLine3 = pWork->fin[ 2 ]->head.child;
	ftmp = DG_ZeroVector;
	ftmp.vy = ( pLine->line.pos[ 0 ].y + pLine2->line.pos[ 0 ].y ) / 4;
	_sceVu0SubVector( &fvec , &ftmp , &pWork->fin_pos[ 2 ] ); // 加速度
	_sceVu0DivVector( &fvec , &fvec , 10 );
	pWork->fin_pos[ 2 ].vy += fvec.vy;
	pLine3->line.pos[ 0 ].y += fvec.vy;
	if ( pLine3->line.pos[ 0 ].y > 290 ){
		pWork->fin_pos[ 2 ].vy  = 90;
		pLine3->line.pos[ 0 ].y = 290;
	}
	if ( pLine3->line.pos[ 0 ].y < 90 ){
		pWork->fin_pos[ 2 ].vy  = 290;
		pLine3->line.pos[ 0 ].y = 90;
	}
	pLine3->line.pos[ 1 ].y = pLine3->line.pos[ 0 ].y; 
	fvec.vy = pLine3->line.pos[ 0 ].y;
	// tateの終点をいじる
	pLine = pLine3->head.next;
	pLine->line.pos[ 0 ].y = fvec.vy;
}

static void Fin2Move( Work *pWork ) // 合成
{
	SPR_OBJ *pLine;
	SPR_OBJ *pLine2;
	SPR_OBJ *pLine3;
	FVECTOR ftmp , fvec;

	// 目標に移動
	pLine = pWork->fin[ 3 ]->head.child;
	pLine2 = pWork->fin[ 2 ]->head.child;
	pLine3 = pWork->fin[ 1 ]->head.child;
	ftmp = DG_ZeroVector;
	ftmp.vy = ( pLine->line.pos[ 0 ].y + pLine2->line.pos[ 0 ].y ) / 2;
	_sceVu0SubVector( &fvec , &ftmp , &pWork->fin_pos[ 1 ] ); // 加速度
	_sceVu0DivVector( &fvec , &fvec , 10 );
	pWork->fin_pos[ 1 ].vy += fvec.vy;
	pLine3->line.pos[ 0 ].y += fvec.vy;
	if ( pLine3->line.pos[ 0 ].y > 290 ){
		pLine3->line.pos[ 0 ].y = 290;
	}
	if ( pLine3->line.pos[ 0 ].y < 90 ){
		pLine3->line.pos[ 0 ].y = 90;
	}
	pLine3->line.pos[ 1 ].y = pLine3->line.pos[ 0 ].y; 
	fvec.vy = pLine3->line.pos[ 0 ].y;
	// tateの終点をいじる
	pLine = pLine3->head.next;
	pLine->line.pos[ 0 ].y = fvec.vy;
}

static void Fin5Move( Work *pWork ) // 合成
{
	SPR_OBJ *pLine;
	SPR_OBJ *pLine2;
	FVECTOR ftmp , fvec;

	// 目標に移動
	pLine = pWork->fin[ 0 ]->head.child;
	pLine2 = pWork->fin[ 4 ]->head.child;
	ftmp = DG_ZeroVector;
	ftmp.vy = ( pLine->line.pos[ 0 ].y ) / 2.0f;
	_sceVu0SubVector( &fvec , &ftmp , &pWork->fin_pos[ 4 ] ); // 加速度
	_sceVu0DivVector( &fvec , &fvec , 10 );
	pWork->fin_pos[ 4 ].vy += fvec.vy;
	pLine2->line.pos[ 0 ].y += fvec.vy;
	if ( pLine2->line.pos[ 0 ].y > 290 ){
		pLine2->line.pos[ 0 ].y = 290;
	}
	if ( pLine2->line.pos[ 0 ].y < 90 ){
		pLine2->line.pos[ 0 ].y = 90;
	}
	pLine2->line.pos[ 1 ].y = pLine2->line.pos[ 0 ].y; 
	fvec.vy = pLine2->line.pos[ 0 ].y;
	// tateの終点をいじる
	pLine = pLine2->head.next;
	pLine->line.pos[ 0 ].y = fvec.vy;
}

#define NUMBER_SPEED_X 4
static void NumberMove( Work *pWork )
{
	GM_CameraSet *pCam;
	float parent_x , parent_y;
	float tmp ;
	int sign;
	float tmp2 , r;
	float diff;

	if ( !( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ) ){
		// カメラの取得
		pCam = GM_GetCurrentCameraSet( 0 );	

		tmp = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
		tmp *= ( 360.f / 4069.f );

		r = 100.f;
		tmp2 = 2 * M_PI * r; // 31半径のこと(円周をだす)
		tmp2  = ( ( tmp2 * tmp ) / 360 );
		// スピード補正
		sign = tmp2 < 0.f ? -1 : 1;
		if ( sign < 0 ){
			if ( tmp2 < -NUMBER_SPEED_X ){
				tmp2 = -NUMBER_SPEED_X;
			}
		} else {
			if ( tmp2 > NUMBER_SPEED_X ){
				tmp2 = NUMBER_SPEED_X;
			}
		}
		tmp = ( 384.f / 448.f );
		pWork->number += tmp2;
		if ( pWork->number >= 100 ){
			diff = pWork->number - 100;
			pWork->number = diff;
		}
		if ( pWork->number < 0 ){
			diff = pWork->number;
			pWork->number = diff + 100;
		}
#if 0
		pWork->Num[ i ].number_pos.vx += tmp2;
		alpha = 60 - ( pWork->Num[ i ].number_pos.vx < 0 ? -pWork->Num[ i ].number_pos.vx : pWork->Num[ i ].number_pos.vx ) * 5;
		if ( alpha <= 0 ){ // 更新
			alpha = 0;
		}
		if ( pWork->Num[ i ].number_pos.vx > NUM_WIDTH ){
			diff = pWork->Num[ i ].number_pos.vx - NUM_WIDTH;
			pWork->Num[ i ].number_pos.vx = diff + ( NUM_WIDTH - ( NUMBER_MAX * 24 ) );
		}
		if ( pWork->Num[ i ].number_pos.vx < NUM_WIDTH - ( NUMBER_MAX * 24 ) ){
			diff = pWork->Num[ i ].number_pos.vx - ( NUM_WIDTH - ( NUMBER_MAX * 24 ) );
			pWork->Num[ i ].number_pos.vx = diff + NUM_WIDTH;
		}
		parent_x = 416 + pWork->Num[ i ].number_pos.vx;
		parent_y = DIRECT_SCREEN_Y( 364.5f ) * tmp;
#endif
	}
	if ( !( pWork->flag & SK_INVISIBLE ) ){
#ifdef PSX2
		parent_x = 416;
#else
		parent_x = 520;/* XBOX  DRAW_WIDTH:512->640   2002.03.19 yano */
#endif

//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
      if ( BP_Area_EU() )
		   parent_y = DIRECT_SCREEN_Y( 313.0f );
//#else
      else
		   parent_y = DIRECT_SCREEN_Y( 311.0f );
//#endif
#if 0
		MENU_S_Locate( parent_x , parent_y , 0 );
		MENU_S_Color( 255 , 48 , 8 , 52 );
		MENU_S_Printf( "%02d" , pWork->number );
#else
		/* changed by K.Takabe 2002.10.03 */
		___MENU_Locate( pWork->menu_print_work, 1, parent_x , parent_y , 0 );
		___MENU_Color( pWork->menu_print_work, 1, 255 , 48 , 8 , 52 );
		___MENU_Printf( pWork->menu_print_work, 1, "%02d" , pWork->number );
#endif
	}
}

#define STR_ROOT (2770484)
static void invisible( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "Root\0" );
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
	NumberMove( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) || pWork->flag & SK_INVISIBLE ){
		return ( -1 );
	}
	ArrowMove( pWork );
	CrossMove( pWork );
	Fin1Move( pWork );
	Fin2Move( pWork );
	Fin3Move( pWork ); // 合成
	Fin4Move( pWork ); // 合成
	Fin5Move( pWork ); // 合成
	// 更新
	pWork->cam_rot = pCam->rotate;

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num ){
		L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
		pWork->action++;
	} else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		if ( Initialize( pWork ) < 0 ){
			return ( -1 );
		}
		pWork->act = (void *)NormalAct;
	}
	return ( 0 );
}

static int Act( Work *pWork )
{
	// デモ等のサイトの表示非表示管理
	if ( SightVisibleInvisible( pWork->handle_2d , STR_ROOT ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
	if ( GM_CheckSightStatus( SGT_NightVision ) || ( GM_IsGameOver() == 1 ) ){
		invisible( pWork );
	}
 	pWork->act( pWork );

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
	work->flag = SK_INITIALIZE_OK;

	/* added by K.Takabe 2002.10.03 */
	work->menu_print_work = NewMenuPrintManager( 512,
												 DG_DMAPACK_NORMAL|
												 DG_DMAPACK_INVISIBLE1|
												 DG_DMAPACK_INVISIBLE2|
												 DG_DMAPACK_INVISIBLE3,
												 DG_DMAPACK_PHASE_AFTER, 145 );
	if ( work->menu_print_work != NULL ){
		GV_SetActorChild( &work->actor, work->menu_print_work );
	}

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewNightVisionSight( void )
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
