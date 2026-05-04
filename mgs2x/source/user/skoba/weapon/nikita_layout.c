//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   thermal_sight.c
   サーマルゴーグルサイト
   
   2001/04/24	S.Kobayashi
   $Id: nikita_layout.c,v 1.1.1.3 2002/11/19 11:50:29 Yoshizawa1 Exp $
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

#define		LAYOUT	     	  (116180)	/* nkt.o2d */
#define     STR_EYE_NULL     (2894076)
#define     STR_CROSS        (6998745)
#define     STR_ENEMY_FRAME (12786347)
#define     STR_CENTER_NULL  (3841455)
#define     STR_DEFAULT       (566267)
#define     STR_FUEL_BAR    (16016034)

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_ACTION_OK        = 0x10,
	SK_ACTION_START     = 0x20,
};

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	SPR_OBJ            *eye_null;        // 真中
	SPR_OBJ            *cross;           // 十字
	SPR_OBJ            *enemy_frame;     // 敵ロック時に登場
	SPR_OBJ            *center_null;     // 下のドット
	SPR_OBJ            *fuel_bar;        // ライフゲージ
	SVECTOR             cam_rot;
	int                 number[ 2 ];    // pauseの時にもでるように
	int                 flag;
	void  ( *act )( struct _work * );
	void				*menu_print_work ;	/* added by K.Takabe 2002.10.03 */
} Work ;

// プロトタイプ
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );
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

#if 0 /*yano 2002.03.19*/
	tmp_width = ( float )( DRAW_WIDTH / 2 );
	tmp_height = ( float )( DRAW_HEIGHT / 2 );
#else
	tmp_width = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
#endif

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	pWork->eye_null = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_EYE_NULL );
	if ( pWork->eye_null == NULL ){
		SK_Err( "eye_null\0" );
		return ( -1 );
	}
	pWork->cross = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_CROSS );
	if ( pWork->cross == NULL ){
		SK_Err( "cross\0" );
		return ( -1 );
	}
	pWork->enemy_frame = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_ENEMY_FRAME );
	if ( pWork->enemy_frame == NULL ){
		SK_Err( "enemy_frame\0" );
		return ( -1 );
	}
	pWork->center_null = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_CENTER_NULL );
	if ( pWork->center_null == NULL ){
		SK_Err( "center_null\0" );
		return ( -1 );
	}
	pWork->fuel_bar = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d, STR_FUEL_BAR );
	if ( pWork->fuel_bar == NULL ){
		SK_Err( "fuel_bar\0" );
		return ( -1 );
	}
	// rotの初期化
	pWork->cam_rot = pCam->rotate;

	return ( 0 );	
}

#define CENTER_MAX 110
#define CENTER_SPEED_X 10
static void CenterNullMove( Work *pWork )
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

	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vy *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 50.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vw  = ( ( ftmp * stmp.vy ) / 360 );
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
	pWork->center_null->empty.pos.x -= ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->center_null->empty.pos.x < -CENTER_MAX + tmp_width ){
			pWork->center_null->empty.pos.x = -CENTER_MAX + tmp_width;
		}
		if ( pWork->center_null->empty.pos.x > CENTER_MAX + tmp_width){
			pWork->center_null->empty.pos.x = CENTER_MAX + tmp_width;
		}
	}
	// 戻す
	if ( stmp.vw == 0 ){
		stmp.vx = pWork->center_null->empty.pos.x;
		stmp.vy = 0;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = tmp_width;
		ftmp3.vy = 0;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 20 );
		pWork->center_null->empty.pos.x += fvec.vx;
	}
}

#define EYE_CENTER_MAX 20
#define EYE_CENTER_SPEED_X 10
#define EYE_CENTER_SPEED_Y 10
static void EyeMove( Work *pWork )
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

	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
	// カメラの取得
	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = pCam->rotate.vx - pWork->cam_rot.vx; // 移動した角度を入力
	stmp.vy = pCam->rotate.vy - pWork->cam_rot.vy; // 移動した角度を入力
	stmp.vx *= ( 360.f / 4069.f );
	stmp.vy *= ( 360.f / 4069.f );

	r = DIRECT_TICK( 50.f );
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vy ) / 360 );
	stmp.vw  = ( ( ftmp * stmp.vx ) / 360 );
	// スピード補正
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -EYE_CENTER_SPEED_X ){
			stmp.vz = -EYE_CENTER_SPEED_X;
		}
	} else {
		if ( stmp.vz > EYE_CENTER_SPEED_X ){
			stmp.vz = EYE_CENTER_SPEED_X;
		}
	}
	sign = stmp.vw < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vw < -EYE_CENTER_SPEED_Y ){
			stmp.vw = -EYE_CENTER_SPEED_Y;
		}
	} else {
		if ( stmp.vw > EYE_CENTER_SPEED_Y ){
			stmp.vw = EYE_CENTER_SPEED_Y;
		}
	}
	pWork->eye_null->empty.pos.x -= ( int )( stmp.vz );
	pWork->eye_null->empty.pos.y += ( int )( stmp.vw );
	// 範囲ない制御
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		if ( pWork->eye_null->empty.pos.x < -EYE_CENTER_MAX + tmp_width ){
			pWork->eye_null->empty.pos.x = -EYE_CENTER_MAX + tmp_width;
		}
		if ( pWork->eye_null->empty.pos.x > EYE_CENTER_MAX + tmp_width ){
			pWork->eye_null->empty.pos.x = EYE_CENTER_MAX + tmp_width;
		}
	}
	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->eye_null->empty.pos.y < -EYE_CENTER_MAX + tmp_height ){
			pWork->eye_null->empty.pos.y = -EYE_CENTER_MAX + tmp_height;
		}
		if ( pWork->eye_null->empty.pos.y > EYE_CENTER_MAX + tmp_height ){
			pWork->eye_null->empty.pos.y = EYE_CENTER_MAX + tmp_height;
		}
	}
	// 戻す
	if ( stmp.vz == 0 && stmp.vw == 0 ){
		stmp.vx = pWork->eye_null->empty.pos.x;
		stmp.vy = pWork->eye_null->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = tmp_width;
		ftmp3.vy = tmp_height;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 10 );
		pWork->eye_null->empty.pos.x += fvec.vx;
		pWork->eye_null->empty.pos.y += fvec.vy;
	}
}

static void SubMove( Work *pWork ) // 差分からラインの拡縮をする
{
	SPR_OBJ *pLine;

	pLine = pWork->cross->head.child; // 上
	pLine->line.pos[ 0 ].x = pWork->eye_null->empty.pos.x; 
	pLine->line.pos[ 1 ].x = pWork->eye_null->empty.pos.x; 
	pLine->line.pos[ 1 ].y = pWork->eye_null->empty.pos.y - DIRECT_SCREEN_Y( 30 ); 

	pLine = pLine->head.next; // 下
	pLine->line.pos[ 0 ].x = pWork->eye_null->empty.pos.x; 
	pLine->line.pos[ 1 ].x = pWork->eye_null->empty.pos.x; 
	pLine->line.pos[ 0 ].y = pWork->eye_null->empty.pos.y + DIRECT_SCREEN_Y( 30 ); 

	pLine = pLine->head.next; // 左
	pLine->line.pos[ 1 ].x = pWork->eye_null->empty.pos.x - DIRECT_SCREEN_Y( 30 ); 
	pLine->line.pos[ 0 ].y = pWork->eye_null->empty.pos.y; 
	pLine->line.pos[ 1 ].y = pWork->eye_null->empty.pos.y; 

	pLine = pLine->head.next; // 右
	pLine->line.pos[ 0 ].x = pWork->eye_null->empty.pos.x + DIRECT_SCREEN_Y( 30 ); 
	pLine->line.pos[ 0 ].y = pWork->eye_null->empty.pos.y; 
	pLine->line.pos[ 1 ].y = pWork->eye_null->empty.pos.y; 

}


/* yano 2002.03.19
   画面の解像度がXBOX版は
   DRAW_WIDTH :512->640となる */
#ifdef KP_XBOX
#define TO_XBOX640(_a) ((_a)/512.0f*640.0f)
#endif
static void NumberMove( Work *pWork )
{
	float parent_x , parent_y;
	float tmp;

	tmp = ( 384.f / 448.f );

	if ( pWork->flag & SK_INVISIBLE ){
		return;
	}
	if ( !(GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU )) ){ // ポーズ中じゃないとき
		pWork->number[ 0 ] = 10000 + irnd() % 90000;
		pWork->number[ 1 ] = 1000 + irnd() % 9000;
	}
#ifdef PSX2
	parent_x = pWork->eye_null->empty.pos.x + 72;
#else
	parent_x = TO_XBOX640( pWork->eye_null->empty.pos.x + 72 );
#endif
	parent_y = DIRECT_SCREEN_Y( ( ( pWork->eye_null->empty.pos.y + 80.f ) * tmp ) );
#if 0
	MENU_S_Locate( parent_x , parent_y , 1 );
	MENU_S_Color( 120 , 240 , 100 , 35 );
	MENU_S_Printf( "%05d" , pWork->number[ 0 ] );
#else
	/* changed by K.Takabe 2002.10.03 */
	___MENU_Locate( pWork->menu_print_work, 1, parent_x , parent_y , 1 );
	___MENU_Color( pWork->menu_print_work, 1, 120 , 240 , 100 , 35 );
	___MENU_Printf( pWork->menu_print_work, 1, "%02d" , pWork->number[ 0 ] );
#endif

#ifdef PSX2
	parent_x = pWork->eye_null->empty.pos.x + 64;
#else
	parent_x = TO_XBOX640( pWork->eye_null->empty.pos.x + 64 );
#endif
	parent_y = DIRECT_SCREEN_Y( ( ( pWork->eye_null->empty.pos.y + 100.f ) * tmp ) );
#if 0
	MENU_S_Locate( parent_x , parent_y , 1 );
	MENU_S_Printf( "%04d" , pWork->number[ 1 ] );
#else
	/* changed by K.Takabe 2002.10.03 */
	___MENU_Locate( pWork->menu_print_work, 1, parent_x , parent_y , 1 );
	___MENU_Printf( pWork->menu_print_work, 1, "%02d" , pWork->number[ 1 ] );
#endif
}

#define EYE_SIZE (40)
static void LockOn( Work *pWork )
{
	TARGET *t , *tnext;
	FVECTOR		max, min, offset ;
	FVECTOR     lock_target_pos , lock_center;
	FMATRIX     world;
	float       ftmp;
	float       raute;
	int         tmp , tmp2;

	/*
	// 一定のタイミングのみ判定
	if ( pWork->time % 300 ){
		return;
	}
	*/
	ftmp = 384.0F / ( float )DRAW_HEIGHT;
	tnext = GM_TrgDefListGetTop();
	while( tnext->next != NULL ){
		t = tnext;
		tnext = t->next;
		if ( ( t->class & TARGET_DEAD ) || !( t->class & TARGET_LOCKON ) ){
			continue;
		}
		if ( t->class & TARGET_ROTATE ) {
			_sceVu0ApplyMatrix( &offset, &t->world, &t->offset ) ;
			DG_COPY_MAT( &world, &t->world ) ;
		} else {
			DG_COPY_VEC( &offset, &t->offset ) ;
			DG_COPY_MAT( &world , &DG_UnitMatrix ) ;
		}
		_sceVu0AddVector( &lock_center, &t->center, &offset ) ;
		GV_VecToMat( &lock_center , &world ) ;
		DG_COPY_VEC( &max, &t->size );
		max.vw = 1.0F;
		_sceVu0SubVector( &min, &DG_ZeroVector, &max );
		min.vw = 1.0F;
		if ( DG_BoundCheck( &world , &max, &min ) & 0x01 ){
			continue;
		}
		DG_TransPersOne( &lock_target_pos , &lock_center );
		lock_target_pos.vy = ( lock_target_pos.vy - 32 ) * ftmp;
		raute = GV_VecLen3F2( &lock_center , &( GM_GetCurrentCamera( 0 )->position ) ) ;
		raute = 0.5F + raute / 16000.0F ;
		if ( raute < 1.0f ){
			raute = 1.0f;
		}
		if ( raute > 6.0f ){
			raute = 6.0f;
		}
		// 範囲に入っているかどうか
		tmp2 = pWork->eye_null->empty.pos.x + ( EYE_SIZE / 2 );
		tmp = lock_target_pos.vx - tmp2;
		tmp = tmp < 0 ? -tmp : tmp;
#ifdef DEBUG_MODE
		printf( "%d %f\n" , tmp , raute );
#endif
		if ( tmp < EYE_SIZE * raute ){
			tmp2 = pWork->eye_null->empty.pos.y + ( EYE_SIZE / 2 );
			tmp = lock_target_pos.vy - tmp2;
			tmp = tmp < 0 ? -tmp : tmp;
			if ( tmp < EYE_SIZE * raute ){
				// 描画位置の更新
				pWork->enemy_frame->box.rect.begin.x = pWork->eye_null->empty.pos.x + 25;
				pWork->enemy_frame->box.rect.end.x   = pWork->enemy_frame->box.rect.begin.x + 58;
				pWork->enemy_frame->box.rect.begin.y = pWork->eye_null->empty.pos.y - 21;
				pWork->enemy_frame->box.rect.end.y   = pWork->enemy_frame->box.rect.begin.y + 10;
				pWork->enemy_frame->box.col.a = 128;
				pWork->enemy_frame->head.child->sprite.col.a = 128;
				SPR_SHOW( pWork->enemy_frame );
				SPR_SHOW( pWork->enemy_frame->head.child );
#if 0
				{
					SPR_OBJ *pSpr2;
					
					pSpr2 = pWork->enemy_frame->head.child;
					printf("show _ nkt lock sight %f %f \n", pSpr2->sprite.pos.x , pSpr2->sprite.pos.y);
				}
#endif
				SPR_SHOW( pWork->enemy_frame );
				return;
			}
		}
	}
}

#define LIFE_SUB_NUM ( 96.f / GM_NikitaLifeMax ) // 比率
static void LifeSub( Work *pWork )
{
	pWork->fuel_bar->sprite.dw = GM_NikitaLife * LIFE_SUB_NUM;
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
	pWork->flag |= SK_ACTION_START;
	pWork->flag &= ~SK_ACTION_OK;
	pWork->act = ( void *)AnimetionAct;
}

static int NormalAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	NumberMove( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) || pWork->flag & SK_INVISIBLE  ){
		return ( -1 );
	}
	SPR_HIDE( pWork->enemy_frame );
	CenterNullMove( pWork );
	EyeMove( pWork );
	SubMove( pWork );
	LockOn( pWork );
	LifeSub( pWork );
	pWork->time += DIRECT_TICK( 6 );
	// 更新
	pWork->cam_rot = pCam->rotate;

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
    int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
		return (0);
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_OK ){
		switch ( pWork->action ) {
		case 0 : // default
			pWork->action = 1;
			pWork->flag |= SK_ACTION_START;
			break;
		case 1 : 
			if ( Initialize( pWork ) < 0 ){
				return ( -1 );
			}
			pWork->act = (void *)NormalAct;
			break;
		}
		pWork->flag &= ~SK_ACTION_OK;
		return (0);
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
	if ( GM_CheckSightStatus( SGT_Nikita ) ){
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
	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	work->handle_2d = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( work->handle_2d < 0 ){
		return -1 ;
	}

	// etc
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	work->act = (void *)AnimetionAct;
	work->time = 0;
	work->number[ 0 ] = 0;
	work->number[ 1 ] = 0;
	work->flag = SK_INITIALIZE_OK | SK_ACTION_START;

	/* added by K.Takabe 2002.10.03 */
	work->menu_print_work = NewMenuPrintManager( 1280,
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

void	*NewNikitaSight( void )
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
