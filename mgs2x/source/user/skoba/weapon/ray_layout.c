//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ray_sight.c
   ＲＡＹ主観
   
   2001/04/16	S.Kobayashi
   $Id: ray_layout.c,v 1.2 2002/12/05 18:41:58 takaki Exp $
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

#define		LAYOUT		(3168847)	/* ray_view.o2d */
#define		DEFAULT_SIGHT_SIZE (40)
#define		MAX_SIGHT_SIZE (100)
#define     SEEKER_AFTERIMAGE (1)
#define 	PITCH_MAX  (10)
#define     NUMBER_MAX (PITCH_MAX)
#define     SEEKER_LINE (4)
#define     CONTENA_BOX (12)
#define     MAX_CONTENA  (12)

#define 	STR_ROOT 		   (2770484)
#define     STR_SEEKER         (8665598)
#define     STR_SCALE          (6492652)
#define     STR_EVEN           (3433742)
#define     STR_CONTENA_NULL   (6314366)
#define     STR_VLCN          (10280021)
#define     STR_PITCH          (3562703)
#define     STR_PITCH_TEX      (7628346)
#define     STR_CENTER_CURS     (782477)
#define     STR_SCALE_MATER     (13031663)
#define     STR_SCALE_BAR       (15807006)
#define     STR_SCALE_EDGE      (1560291) 
#define     STR_EVEN_TEX        (7628347)
#define     STR_DEFAULT         (566267)
#define     DEFAULT_PLUS        (26)   // シネマ分ずらす

enum {
	SK_ACTION_START = 0x1,
	SK_ACTION_OK = 0x2,
	SK_INIT_END = 0x4,
};

typedef struct {
	FVECTOR             number_pos;
	FVECTOR             parent_pos;
} Number;

typedef struct {
	FVECTOR pos;
	float frame;
} Sound;

typedef struct {
	float speed;
	float size;
	float frame;
} Bar;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	void				*layout_player;
	SPR_OBJ             *scale;
	SPR_OBJ             *even;
	SPR_OBJ             *contena_null[ MAX_CONTENA ];
	SPR_OBJ             *seeker[ 1 ];
	SPR_OBJ             *vlcn;
	SPR_OBJ             *pitch[ PITCH_MAX ];
	SPR_OBJ             *centercurs;
	SPR_OBJ             *scale_mater;
	SPR_OBJ             *scale_edge;
	SPR_COLOR           color;
	SVECTOR             rot[ 2 ]; // Ω計算用
	FVECTOR             centercurs_pos;
	float               centercurs_scale;
	SPR_POS             seeker_old_pos[ SEEKER_AFTERIMAGE ];
	float               seeker_scale[ SEEKER_AFTERIMAGE ];
	Sound               Sound_work;
	Number              Num[ NUMBER_MAX ];
	float               cam_scale_bak;
	float               cam_scale_bak2;
	int                 lockon_count;
	int					time;
	int                 init_flag;
	int                 action;
	int                 action_num;
	int                 flag;
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

TARGET *RAY_LockonTarget;

// プロトタイプ
extern void *NewRasterEffect( int alpha, int color );
extern void *NewRayUVSight( void );
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );

static void AnimationAct( Work * ); // アニメーション

/*----------------------------------------------------------------*/
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

/* 画面内チェック */
static	int		CheckTargetInSight( TARGET *t )
{
	FVECTOR		max, min, offset ;

	if ( t->class & TARGET_ROTATE ) {
		_sceVu0ApplyMatrix( &offset, &t->world, &t->offset ) ;
		DG_COPY_MAT( WORLD, &t->world ) ;
	} else {
		DG_COPY_VEC( &offset, &t->offset ) ;
		DG_COPY_MAT( WORLD, &DG_UnitMatrix ) ;
	}
	_sceVu0AddVector( CENTER, &t->center, &offset ) ;
	GV_VecToMat( CENTER, WORLD ) ;
	DG_COPY_VEC( &max, &t->size ) ; max.vw = 1.0F ;
	_sceVu0SubVector( &min, &DG_ZeroVector, &max ) ; min.vw = 1.0F ;
	if ( DG_BoundCheck( WORLD, &max, &min ) & 0x01 ){
		return 0;
	}
	DG_TransPersOne( (FVECTOR *)DISP_POS, (FVECTOR *)CENTER );

	return 1 ;
}

/* サイトの表示 */
static void DisplaySight( SPR_OBJ *pSpr )
{
	float			len ;

	SIGHT_POS->x =   DISP_POS->vx ;
	SIGHT_POS->y = ( DISP_POS->vy ) * 384.0F / ( float )DRAW_HEIGHT ;

	SPR_SetPosEmpty( pSpr , SIGHT_POS ) ;
	len = GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) ;
	SCALE = 6.0F - len / 2000.0F ;
	if ( SCALE < 1.0F ){
		SCALE = 1.0F ;
	}
	SPR_MAG( pSpr, SCALE ) ;
	SPR_SHOW( pSpr ) ;
}

/* ロックオンできるか？ */
static float CheckLockonAvailable( Work *pWork , TARGET *t )
{
	float		size ;
	float       len;
	FVECTOR		center ;

	center = *(FVECTOR *)CENTER;
	len = GV_VecLen3F2( &center, &( GM_GetCurrentCamera( 0 )->position ) ) ;
	/* ミサイル飛行中は新たにロックオンしない */
	if ( RAY_LockonTarget != NULL ){
		if ( RAY_LockonTarget == t ){
			size = MAX_SIGHT_SIZE;
			center.vx = ( DRAW_WIDTH / 2 ) - DISP_POS->vx; 
			center.vy = ( DRAW_HEIGHT / 2 ) - DISP_POS->vy; 
			if ( center.vx < - size || center.vx > size ||
				 center.vy < - size || center.vy > size ){
				pWork->lockon_count++;
			} else {
				pWork->lockon_count = 0;
			}
			return ( len );
		} else {
			return ( -1 );
		}
	}
	if ( GM_WeaponAlive & WP_ALIVE_STINGER ){
		return ( -1 );
	}

	SCALE = 10.0F - len / 2000.0F ;
	if ( SCALE < 1.0F ){
		SCALE = 1.0F ;
	}
	size = DEFAULT_SIGHT_SIZE;// * SCALE;

	center.vx = DISP_POS->vx; 
	center.vy = DISP_POS->vy; 
	if ( center.vx < - size || center.vx > size || center.vy < - size || center.vy > size ){
		return ( -1 );
	}

	return ( len );
}

#define PITCH_PLUS_Y 164
static void SetMater( Work *pWork ) // メーターに初期値設定
{
	int i;

	for ( i = 0 ; i < PITCH_MAX ; i++ ){
		// 数値のポジション,枠のポジション
		pWork->pitch[ i ]->empty.pos.x = 512.0f/2.0f ;/* 仮想座標なのに間違えていた　下もそれくさいけど。。。　T.Morita 2002.04.30 */
		pWork->pitch[ i ]->empty.pos.y = ( float )( DRAW_HEIGHT + PITCH_PLUS_Y ) - ( 60 * i );
	}
}

static int SprInit( Work *work )
{
	SPR_OBJ		*spr ;

	// センターカーソルの取得
	spr = L2D_GetObject( work->handle_2d, STR_CENTER_CURS ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	work->centercurs = spr ;
	// シーカーの取得
	spr = L2D_GetObject( work->handle_2d, STR_SEEKER ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	work->seeker[ 0 ] = spr ;
	// 横棒の取得
	spr = L2D_GetObject( work->handle_2d, STR_SCALE ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	work->scale = spr ;
	// のびのびの取得
	spr = L2D_GetObject( work->handle_2d, STR_SCALE_MATER ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	work->scale_mater = spr ;
	// のびのびの取得
	spr = L2D_GetObject( work->handle_2d, STR_SCALE_EDGE ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	work->scale_edge = spr ;
	// エイブン
	spr = L2D_GetObject( work->handle_2d, STR_EVEN );
	if ( spr == NULL ){
		return -1 ;
	}
	work->even = spr;

	spr = L2D_GetObject( work->handle_2d, STR_VLCN );
	if ( spr == NULL ){
		return -1 ;
	}
	work->vlcn = spr;

	// シネマ
	spr = L2D_GetObject( work->handle_2d, STR_ROOT ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	spr->empty.pos.y -= DEFAULT_PLUS;

	SPR_HIDE( work->seeker[ 0 ] ); // 注意
	//	SPR_HIDE( work->vlcn );
	// センターカーソルの初期化
	//	work->centercurs->empty.pos.x = 0;
	//	work->centercurs->empty.pos.y = ( float )( DRAW_HEIGHT / 2 );
	work->centercurs_pos.vz = 0;
	work->centercurs_pos.vw = 0;
	// even
	//	work->even->empty.pos.x = ( float )( DRAW_WIDTH / 2 );
	//	work->even->empty.pos.y = ( float )( DRAW_HEIGHT / 2 ) - 14;
	// vlcn
	work->vlcn->empty.pos.x = 0;
	work->vlcn->empty.pos.y = 0;
	// etc
	work->centercurs_scale = 1.0f;

	return( 0 );
}

static int invisible( Work *pWork ) // 動かすものを消しておく
{
	SPR_OBJ		*spr ;
	int i;

	// ピッチの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_PITCH ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->pitch[ 0 ] = spr ;
	for ( i = 1 ; i < PITCH_MAX ; i++ ){
		pWork->pitch[ i ] = SPR_DuplicateTree( spr );
	}
	// コンテナ
	spr = L2D_GetObject( pWork->handle_2d, STR_CONTENA_NULL );
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->contena_null[ 0 ] = spr;
	for ( i = 1 ; i < MAX_CONTENA ; i++ ){
		pWork->contena_null[ i ] = SPR_DuplicateTree( spr );
	}

	// メーターの初期化
	SetMater( pWork );

	pWork->flag |= SK_INIT_END;

	return ( 0 );
}

#define MATER_SPEED_Y 20
static void PitchMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FMATRIX      fmtmp;
	FVECTOR     vspeed;
	SVECTOR        rot; 
	float         ftmp;
	float            r;
	float          deg;
	float         diff;
	float       height;
	float    tmp_width;
	int           sign;
	int              i;

	pCam = GM_GetCurrentCameraSet( 0 );	
	// Z回転行列生成
	rot = pCam->rotate;
	rot.vx = rot.vy = 0;
	DG_SetPos2( &DG_ZeroVector , &rot );
	DG_GetPos( &fmtmp );

	stmp.vx = pCam->rotate.vx - pWork->rot[ 1 ].vx; // 移動した角度を入力
	stmp.vx /= 1024.f;
	r = 20000.f;

	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360 );
	// スピード補正
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -MATER_SPEED_Y ){
			stmp.vz = -MATER_SPEED_Y;
		}
	} else {
		if ( stmp.vz > MATER_SPEED_Y ){
			stmp.vz = MATER_SPEED_Y;
		}
	}
	pWork->rot[ 1 ] = pCam->rotate; // 更新 + PITCH_PLUS_Y

	height = DRAW_HEIGHT + PITCH_PLUS_Y - ( DRAW_HEIGHT + PITCH_PLUS_Y - 60 * PITCH_MAX );
	tmp_width  = 512.0f/2 ; /* 仮想座標なのに間違えていた　上もそれくさいけど。。。　T.Morita 2002.04.30 */
	// 回転後の速度出し
	deg = ( float )pCam->rotate.vz / 4096.f * 360.f;
	vspeed = DG_ZeroVector;
	vspeed.vy = stmp.vz;		
	_sceVu0ApplyMatrix( &vspeed , &fmtmp , &vspeed );
	for ( i = 0 ; i < PITCH_MAX ; i++ ){
 //		pWork->pitch[ i ]->empty.pos.x += vspeed.vx;
		pWork->pitch[ i ]->empty.pos.y -= vspeed.vy;
		// 傾いた時xの修正
//		height2 = pWork->pitch[ i ]->empty.pos.y - ( float )( DRAW_HEIGHT / 2 );
//		pWork->pitch[ i ]->empty.pos.x = tmp_width + ( vspeed.vx * height2 );	
		SPR_SHOW( pWork->pitch[ i ] );
		// オブジェクトの削除(更新)
		if ( pWork->pitch[ i ]->empty.pos.y < DRAW_HEIGHT + PITCH_PLUS_Y - 60 * PITCH_MAX ){
			diff = pWork->pitch[ i ]->empty.pos.y - ( float )( DRAW_HEIGHT + PITCH_PLUS_Y - 60 * PITCH_MAX );  // 差分
//			pWork->pitch[ i ]->empty.pos.x = pWork->empty.pitch[ i ]->pos.x - ( vspeed.vx * ( height + diff ) );
			pWork->pitch[ i ]->empty.pos.y = DRAW_HEIGHT + PITCH_PLUS_Y + diff;
		}
		if ( pWork->pitch[ i ]->empty.pos.y > DRAW_HEIGHT + PITCH_PLUS_Y ){
			diff = pWork->pitch[ i ]->empty.pos.y - ( float )( DRAW_HEIGHT + PITCH_PLUS_Y );  // 差分
//			pWork->pitch[ i ]->empty.pos.x = pWork->pitch[ i ]->empty.pos.x + ( vspeed.vx * ( height + diff ) );
			pWork->pitch[ i ]->empty.pos.y = ( DRAW_HEIGHT + PITCH_PLUS_Y ) - 60 * PITCH_MAX + diff;
		}
	}
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 3
#define CENTER_SPEED_Y 3
static void CenterMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR       ftmp3;
	float         ftmp;
	float        ftmp2;
	float        scale;
	float        tmp_width;
	float        tmp_height;
	float            r;
	int           sign;

	pCam = GM_GetCurrentCameraSet( 0 );	

	stmp.vx = pCam->rotate.vx - pWork->rot[ 0 ].vx; // 移動した角度を入力
	stmp.vy = pCam->rotate.vy - pWork->rot[ 0 ].vy; // 移動した角度を入力
	tmp_width  = 512.0f/2.0f ; /* 仮想座標なのに間違えていた　下もそれくさいけど。。。　T.Morita 2002.04.30 */
   if ( BP_Area_EU() )
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 56 );
   else
//#else
   	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 32 );
//#endif
	if ( stmp.vx == 0 || stmp.vy == 0 ){ // 一次元か二次元か
		r = 20.f;
	} else {
		r = 20.f;
	}
	ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360 );  
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
	sign = stmp.vz < 0 ? -1 : 1;
	if ( sign < 0 ){
		if ( stmp.vz < -CENTER_SPEED_Y ){
			stmp.vz = -CENTER_SPEED_Y;
		}
	} else {
		if ( stmp.vz > CENTER_SPEED_Y ){
			stmp.vz = CENTER_SPEED_Y;
		}
	}
	pWork->centercurs->empty.pos.x -= ( int )( stmp.vw );
	pWork->centercurs->empty.pos.y += ( int )( stmp.vz );

	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->centercurs->empty.pos.x < -CENTER_MAX + tmp_width ){
			pWork->centercurs->empty.pos.x = -CENTER_MAX + tmp_width;
		}
		if ( pWork->centercurs->empty.pos.x > CENTER_MAX + tmp_width ){
			pWork->centercurs->empty.pos.x = CENTER_MAX + tmp_width;
		}
	}
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		if ( pWork->centercurs->empty.pos.y < -CENTER_MAX + tmp_height ){
			pWork->centercurs->empty.pos.y = -CENTER_MAX + tmp_height;
		}
		if ( pWork->centercurs->empty.pos.y > CENTER_MAX + tmp_height ){
			pWork->centercurs->empty.pos.y = CENTER_MAX + tmp_height;
		}
	}
	// 戻す
	if ( stmp.vw == 0 && stmp.vz == 0 ){
		stmp.vx = pWork->centercurs->empty.pos.x;
		stmp.vy = pWork->centercurs->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = tmp_width;
		ftmp3.vy = tmp_height;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 10 );
		pWork->centercurs->empty.pos.x += fvec.vx;
		pWork->centercurs->empty.pos.y += fvec.vy;
	}
	// 補間
	if ( pWork->cam_scale_bak == pCam->angle ){ // 元に戻す
		scale = ( float )( 1.2f - pWork->centercurs_scale ) / 4.f;
		pWork->centercurs_scale += scale;
	} else {
		scale = pCam->angle - pWork->cam_scale_bak2; // default 2.0だから
		pWork->cam_scale_bak2 = pCam->angle;
		scale = scale < 0 ? 1.5f : 1.0f;
		pWork->centercurs_scale += ( float )( scale - pWork->centercurs_scale ) / 4.f;
		// 拡大
		if ( pWork->centercurs_scale < 1.0F ){
			pWork->centercurs_scale = 1.0F;
		}
		if ( pWork->centercurs_scale > 1.5F ){
			pWork->centercurs_scale = 1.5F;
		}	
	}
	// 更新
	pWork->cam_scale_bak = pCam->angle;
	// etc
	SPR_MAG( pWork->centercurs , pWork->centercurs_scale );
	SPR_SHOW( pWork->vlcn );
	SPR_SHOW( pWork->centercurs ) ;
}

#define STR_SCALE_EDGE (1560291)
static void HeightMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	SPR_OBJ      *pSpr;

	pCam = GM_GetCurrentCameraSet( 0 );	
	stmp.vx = (float)pCam->rotate.vx / 1024.f; // 現在の角度

	// きめきめPALヨウチュウイ
	pSpr = L2D_GetObject( pWork->handle_2d , STR_SCALE_BAR );
	stmp.vy = -160.f * stmp.vx + 193;
	if ( stmp.vy > 120 + 193 ){
		stmp.vy = 120 + 193;
	}	
	pSpr->line.pos[1].y = stmp.vy;

	SPR_SHOW( pWork->scale_mater ) ;

	pSpr = L2D_GetObject( pWork->handle_2d , STR_SCALE_EDGE );
	pSpr->line.pos[0].y = stmp.vy;
	pSpr->line.pos[1].y = stmp.vy;

	SPR_SHOW( pWork->scale_edge ) ;
}

static void EvenMove( Work *pWork )
{
	GM_CameraSet *pCam;
	float          deg;

	pCam = GM_GetCurrentCameraSet( 0 );	
	pWork->rot[ 0 ] = pCam->rotate; // 更新

	deg = ( float )pCam->rotate.vz / 4096.f * 360.f;
	SPR_SHOW( pWork->even );
	SPR_SHOW( pWork->even->head.child ); // ねんのため
}

static void invisible2( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->act = ( void * )AnimationAct;
}

static	void	NormalAct( Work *work )
{
	TARGET		*t, *tnext;
	float       min_length;
	float              len;
	int      n_targets , i;

	min_length = 0xffff;

	if ( !( work->flag & SK_INIT_END ) ){
		return;
	}
	CenterMove( work );
	EvenMove( work );
//	HeightMove( work );
	PitchMove( work );

	tnext = GM_TrgDefListGetTop();
	for ( i = 0; i < MAX_CONTENA; i ++ ){ // サイトの消去
		SPR_HIDE( work->contena_null[ i ] );
	}
	n_targets = 0;
	while( tnext != NULL ) {
		if ( n_targets >= MAX_CONTENA ){
			break;
		}
		t = tnext;
		tnext = t->next;
		if ( !( t->class & TARGET_LOCKON ) ){
			continue ;
		}
		if ( !( t->map & GM_CurrentStageMap ) ){
			continue ;
		}
		if ( t->side == PLAYER_SIDE ){
			continue ;
		}
		if ( !CheckTargetInSight( t ) ){
			continue ;
		}
		len = CheckLockonAvailable( work , t );
		if ( len >= 0 && min_length > len && t != ATHOR_LOCK ){
			LOCKONTRG = t ;
			NEWLOCK = 1 ;
			*TARGET_POS = *DISP_POS;
			min_length = len;
		}
		DisplaySight( work->contena_null[ n_targets ] ) ;
		n_targets++;
	}
	work->time++;
}

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_RayView ) ){
		invisible2( pWork );
		return;
	}
	AnimationAct( pWork );
 	pWork->act( pWork );
}

#define STR_DEFAULT (566267)
#define STR_ACTION (13328255)
static void AnimationAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT :
			if ( !( pWork->flag & SK_INIT_END ) ){
				SprInit( pWork );
				invisible( pWork );
			}
			break;
		}
		pWork->action = STR_ACTION;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d ) ;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	GM_CameraSet *pCam;
	int			handle;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		SK_Err("ray miss\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	// サウンド
	work->Sound_work.frame = 60;
	// etc
	work->cam_scale_bak = 0;
	work->cam_scale_bak2 = 0;
	work->time = 0;
	work->init_flag = 0;
	work->color.r = 180;
	work->color.g = 250;
	work->color.b = 160;
	work->rot[ 0 ] = work->rot[ 1 ] = pCam->rotate; 

	work->action = STR_DEFAULT;
	work->act = (void *)NormalAct;
	work->flag = SK_ACTION_START; 
	GV_SetActorChild( work , NewRasterEffect( SCE_GS_SET_ALPHA(  0 , 1 , 0 , 1 , 0x00 ) , 0x36002000 ) );
//	GV_SetActorChild( work , NewRayUVSight() );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewRaySight( void )
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
