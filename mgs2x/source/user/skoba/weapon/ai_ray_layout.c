//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   stg_sight.c
   ロックオンサイト
   
   2001/07/18	S.Kobayashi
   $Id: ai_ray_layout.c,v 1.2 2002/12/05 18:41:59 takaki Exp $
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
#include	"../../mode/demo/eft_con.h"

#define		LAYOUT		(11492320)	// ai_ray_view.l2d
#define     MAX_OBJ     (8)
#define 	MAX_SIGHTS  (64)
#define		MAX_SIGHT_SIZE (100)
#define     SEEKER_LINE (4)
#define     SEEKER_AFTERIMAGE (8)
#define     CONTENA_DOT (4)
#define     CONTENA_BOX (1)
#define 	MEATER_MAX  (10)
#define     NUMBER_MAX (MEATER_MAX * 2)
#define     MT_BAR_MAX  (2)
//#define     STR_SEEKER       (8665598)
#define		STR_SEEKER (15204321)
#define     STR_CONTENA_NULL (6684498)
#define     STR_CURSOR_T     (7113343)
#define     STR_CURSOR_B     (7113325)
#define     STR_DEFAULT      (566267)
#define     STR_AIMAREA      (15887897) // GV_StrCode( "aimArea" )
#define     STR_SEEK01       (8663837)  // GV_StrCode( "seek01" );
#define     STR_CONTENA1     (7829748)  //GV_StrCode( "contena1" );
#define     STR_DOT          (3394225)  // GV_StrCode( "dot1" );
#define 	STR_FIX          (74104)
#define 	STR_ROOT         (2770484)
#define     STR_PITCH          (3562703)
#define     STR_SCALE_BAR       (15807006)
#define     STR_SCALE_EDGE      (1560291) 
#define     STR_CENTER_CURS     (782477)
#define     STR_VLCN          (10280021)
#define     DEFAULT_PLUS        (26)   // シネマ分ずらす

#define 	PITCH_MAX  (10)
#define     DIE_FLG                (1)
#define     ROCK_SPEED             (1) // 16
#define     ROCK_PERFORMANCE       (1.0f) // 0.1f
#define 	ANGLE_MAX (8.0f)
#define     ZOOM_DW (6)

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
};

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

typedef struct {
	float               sights_z;
	int                 position;
} Zbuf;

typedef struct {
	int sights;
	int seeker;
	int pitch;
} DupCount;

typedef	struct _work {
	GV_ACT_EX			actor ;
	SPR_EMPTY           *cover; // test
	SPR_OBJ			    *centercurs;
	SPR_OBJ				*sights[ MAX_SIGHTS ];
	SPR_OBJ				*seeker[ SEEKER_AFTERIMAGE ]; // empty
	SPR_OBJ             *seeker_sub[ SEEKER_LINE ];    // 色を変える時に使用
	SPR_OBJ             *aimArea;
	SPR_OBJ             *contena_box[ CONTENA_BOX ];
	SPR_OBJ             *contena_dot[ CONTENA_DOT ];
	SPR_OBJ             *pitch[ PITCH_MAX ];
	SPR_OBJ             *vlcn;
	SPR_POS             seeker_old_pos[ SEEKER_AFTERIMAGE ];
	Zbuf                Zbuf; // 優先を決める時に使用
	Bar                 mt_bar_work;
	Sound               Sound_work;
	TARGET              *sights_color[ MAX_SIGHTS ]; // コンテナの色を決定するために使う
	SPR_COLOR           color;
	SVECTOR             rot[ 2 ];
	float               seeker_scale[ SEEKER_AFTERIMAGE ];
	int                 alive_flag; // スティンガーが当たったかどうか
	int                 lock_flag;
	int                 x , y;
	int					time;
	int					lockon_count;
	int                 after_image_count;
	int                 action;
	int                 action_num;
	int                 name;
	int					handle_2d;
	char                togle; // モデルありモード or モデルなしモード
	void				*layout_player;
	int                 lockon_size;
	int                 flag;

	FVECTOR             *mov; // デモ専用
	TARGET              target; // デモ Dumy Target
	FVECTOR             centercurs_pos;
	float               centercurs_scale;
	float               cam_scale_bak;
	float               cam_scale_bak2;

	float               zoom_raute;
	float               raute;
	DupCount            dup_count;
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

enum {
	SK_STHINGER_MODEL_OFF = 0x4, // モデルなし通常モードにいこう
	SK_STHINGER_MODEL_ON = 0x8, // モデル有直ねらいモードにいこう
};

// グローバル
static TARGET *LockonTarget = NULL;
static SPR_COLOR default_color = SPR_COLOR_CTOR( 160 , 250 , 90 , 64 );

// プロトタイプ
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );
extern void *NewRasterEffect( int alpha, int color );

static void AnimationAct( Work * ); // アニメーション
static void BugAnimationAct( Work *pWork );

/*----------------------------------------------------------------*/
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork;
	SPR_OBJ *pRoot;

	pWork = ( Work * )workp;

	switch ( signal ){
	case SK_STHINGER_MODEL_ON :
		pRoot = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( pRoot == NULL ){
			SK_Err( "root\0" );
			return ( 0 );
		}
		SPR_HIDE( pRoot ); 
		pWork->togle = SK_STHINGER_MODEL_ON;
		break;
	case SK_STHINGER_MODEL_OFF :
		pRoot = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( pRoot == NULL ){
			SK_Err( "root\0" );
			return ( 0 );
		}
		SPR_SHOW( pRoot ); 
		pWork->togle = SK_STHINGER_MODEL_OFF;
		break;
	case 0x2 :
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

#define BUG_FLG (0x2)
#define STR_RAY_BUG (12661840)
// メッセージを受けとると殺す
static int MsgDie( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == DIE_FLG ){
			GV_DestroyActor( pWork );
			return ( -1 );
		}
		if ( num == BUG_FLG ){ // cheng the bug mode
			pWork->act = ( void * )BugAnimationAct;
			pWork->action = STR_RAY_BUG;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			// 強制終了
			L2D_BreakAction( pWork->handle_2d );
		}
		msg--;
	}
	return ( 0 );
}

#define PITCH_PLUS_Y 164
static void SetMater( Work *pWork ) // メーターに初期値設定
{
	int i;

	for ( i = 0 ; i < PITCH_MAX ; i++ ){
		// 数値のポジション,枠のポジション
		pWork->pitch[ i ]->empty.pos.x = 512.0f/2.0f ;/* 仮想座標なのに間違えていた　下もそれくさいけど。。。　T.Morita 2002.05.21 */
		pWork->pitch[ i ]->empty.pos.y = ( float )( DRAW_HEIGHT + PITCH_PLUS_Y ) - ( 60 * i );
	}
}

#define MATER_SPEED_Y 20
static void PitchMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR     vspeed;
	float         ftmp;
	float            r;
	float         diff;
	float       height;
	float    tmp_width;
	int           sign;
	int              i;

	pCam = GM_GetCurrentCameraSet( 0 );	

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
	tmp_width = ( float )( DRAW_WIDTH / 2 );
	vspeed.vy = stmp.vz;		
	for ( i = 0 ; i < PITCH_MAX ; i++ ){
		pWork->pitch[ i ]->empty.pos.y -= vspeed.vy;
		// 傾いた時xの修正
		SPR_SHOW( pWork->pitch[ i ] );
		// オブジェクトの削除(更新)
		if ( pWork->pitch[ i ]->empty.pos.y < DRAW_HEIGHT + PITCH_PLUS_Y - 60 * PITCH_MAX ){
			diff = pWork->pitch[ i ]->empty.pos.y - ( float )( DRAW_HEIGHT + PITCH_PLUS_Y - 60 * PITCH_MAX );  // 差分
			pWork->pitch[ i ]->empty.pos.y = DRAW_HEIGHT + PITCH_PLUS_Y + diff;
		}
		if ( pWork->pitch[ i ]->empty.pos.y > DRAW_HEIGHT + PITCH_PLUS_Y ){
			diff = pWork->pitch[ i ]->empty.pos.y - ( float )( DRAW_HEIGHT + PITCH_PLUS_Y );  // 差分
			pWork->pitch[ i ]->empty.pos.y = ( DRAW_HEIGHT + PITCH_PLUS_Y ) - 60 * PITCH_MAX + diff;
		}
	}
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

	pSpr = L2D_GetObject( pWork->handle_2d , STR_SCALE_EDGE );
	pSpr->line.pos[0].y = stmp.vy;
	pSpr->line.pos[1].y = stmp.vy;
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
	DISP_POS->vy = ( DISP_POS->vy ) * 384.0F / ( float )DRAW_HEIGHT;

	return 1 ;
}

/* サイトの表示 */
static	void	DisplaySight( Work *pWork , SPR_OBJ *spr , TARGET *pTrg )
{
	GM_CameraSet *pCam;
    SPR_OBJ     *spr_box;
	FVECTOR     fvtmp;
	FVECTOR     getpos_left;
	FVECTOR     getpos_right;
	FVECTOR     worktmp;
	FVECTOR     size_value;
	FVECTOR     buf_vec;
	float	    len;

	SIGHT_POS->x = ( DISP_POS->vx )/DRAW_WIDTH*512;
	SIGHT_POS->y = ( DISP_POS->vy );

	pCam = GM_GetCurrentCameraSet( 0 );

	SPR_SetPosEmpty( spr , SIGHT_POS ) ;
	if ( pTrg->class & TARGET_LOCKON_AUTOSIZE ){ // AutoMation Size
		if ( pTrg->class & TARGET_ROTATE ) { // 回転計算
			_sceVu0ApplyMatrix( &fvtmp , &pTrg->world , &pTrg->offset );
			DG_SetPos( &pTrg->world );
			DG_RotVector( &pTrg->size , &size_value , 1 );
		} else { // 通常
			DG_COPY_VEC( &fvtmp , &pTrg->offset );
			DG_COPY_VEC( &size_value , &pTrg->size );
			_sceVu0AddVector( &fvtmp , &pTrg->center , &fvtmp );
		}
		spr_box = spr->head.child->head.child;
		// サイズの2D化
		size_value.vx = size_value.vy = size_value.vz = ( fpu_Sqrt( _sceVu0InnerProduct( &size_value , &size_value ) ) / 4.0f );
		size_value.vw = 1.0f;
		// 左上
		_sceVu0SubVector( &worktmp , &fvtmp , &size_value );
		DG_TransPersOne( &getpos_left , &worktmp );
		// 右下
		_sceVu0AddVector( &worktmp , &fvtmp , &size_value );
		DG_TransPersOne( &getpos_right , &worktmp );
		// swap
		if ( getpos_left.vx > getpos_right.vx ){
			DG_COPY_VEC( &buf_vec , &getpos_left );
			DG_COPY_VEC( &getpos_left , &getpos_right );
			DG_COPY_VEC( &getpos_right , &buf_vec );
		}
		// ベクトルの大きさから幅を出す
		_sceVu0SubVector( &fvtmp , &getpos_right , &getpos_left );
		fvtmp.vx = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );
		if ( fvtmp.vx < 28.0f ){
			fvtmp.vx = 28.0f;
		}

		spr_box->box.rect.begin.x = -fvtmp.vx;
		spr_box->box.rect.begin.y = -fvtmp.vx;
		spr_box->box.rect.end.x = fvtmp.vx;
		spr_box->box.rect.end.y = fvtmp.vx;
		// ドットの位置修正
		len = ( spr_box->box.rect.end.x - spr_box->box.rect.begin.x ) / 2.0f;
		len = 1.0f;
		if ( len < 1.0F ){
			len = 1.0F;
		}
		// 左上
		spr->head.child->head.next->head.next->sprite.pos.x = spr_box->box.rect.begin.x - 4.0f - 4.0f * len;
		spr->head.child->head.next->head.next->sprite.pos.y = spr_box->box.rect.begin.y - 0.0f * len;
		// 左下
		spr->head.child->head.next->sprite.pos.x = spr_box->box.rect.begin.x - 4.0f - 4.0f * len;
		spr->head.child->head.next->sprite.pos.y = spr_box->box.rect.end.y   - 4.0f - 0.0f * len;
		// 右上
		spr->head.child->head.next->head.next->head.next->head.next->sprite.pos.x = spr_box->box.rect.end.x + 4.0f * len;
		spr->head.child->head.next->head.next->head.next->head.next->sprite.pos.y = spr_box->box.rect.begin.y - 0.0f * len;
		// 右下
		spr->head.child->head.next->head.next->head.next->sprite.pos.x = spr_box->box.rect.end.x + 4.0f * len;
		spr->head.child->head.next->head.next->head.next->sprite.pos.y = spr_box->box.rect.end.y - 4.0f - 0.0f * len;
	} else { // Normal Scale Size
		len = GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) ;
		SCALE = 4.0F + ( pCam->angle * pWork->zoom_raute ) - len / 2000.0F ;
		if ( SCALE < 1.0F ){
			SCALE = 1.0F ;
		}
		SPR_MAG( spr->head.child , SCALE ) ;
		// ドットの位置修正
		spr_box = spr->head.child->head.child;
		// 左上
		spr->head.child->head.next->head.next->sprite.pos.x = ( spr_box->box.rect.begin.x * SCALE ) - 4.0f - 4.0f;
		spr->head.child->head.next->head.next->sprite.pos.y = ( spr_box->box.rect.begin.y * SCALE ) - 0.0f;
		// 左下
		spr->head.child->head.next->sprite.pos.x = ( spr_box->box.rect.begin.x * SCALE ) - 4.0f - 4.0f;
		spr->head.child->head.next->sprite.pos.y = ( spr_box->box.rect.end.y * SCALE ) - 4.0f;
		// 右上
		spr->head.child->head.next->head.next->head.next->head.next->sprite.pos.x = ( spr_box->box.rect.end.x * SCALE ) + 4.0f;
		spr->head.child->head.next->head.next->head.next->head.next->sprite.pos.y = ( spr_box->box.rect.begin.y * SCALE );
		// 右下
		spr->head.child->head.next->head.next->head.next->sprite.pos.x = ( spr_box->box.rect.end.x * SCALE ) + 4.0f;
		spr->head.child->head.next->head.next->head.next->sprite.pos.y = ( spr_box->box.rect.end.y * SCALE ) - 4.0f;
	}
	SPR_SHOW( spr ) ;
}

static void ColorChenge_Contena( SPR_OBJ *spr_box , SPR_OBJ *spr_dot , SPR_COLOR *color )
{
	int i;
	SPR_OBJ *spr;

	SPR_SetColorBox( spr_box , color->r , color->g , color->b , color->a );

	spr = spr_dot;
	for ( i = 0 ; i < CONTENA_DOT ; i++ ){
		spr->sprite.col.r = color->r;
		spr->sprite.col.g = color->g;
		spr->sprite.col.b = color->b;
		spr->sprite.col.a = color->a / 2;
		// 次へ
		spr = spr->head.next; 
	}
}

static SPR_OBJ *SearchContena( Work *pWork , TARGET *pTrg , SPR_COLOR *pCol) // 任意のコンテナの色を変える
{
	SPR_OBJ *spr;
	int i;

	if ( pTrg == NULL ){
		return ( NULL );
	}
	for ( i = 0 ; i < MAX_SIGHTS ; i++ ){
		if ( pWork->sights_color[ i ] == pTrg ){
			spr = pWork->sights[ i ]->head.child->head.child;
			ColorChenge_Contena( spr , pWork->sights[ i ]->head.child->head.next , pCol );
			return ( pWork->sights[ i ] );
		}
	}
	return ( NULL );
}

static SPR_OBJ *SearchContenaNormal( Work *pWork , TARGET *pTrg )
{
	int i;

	if ( pTrg == NULL ){
		return ( NULL );
	}
	for ( i = 0 ; i < MAX_SIGHTS ; i++ ){
		if ( pWork->sights_color[ i ] == pTrg ){
			return ( pWork->sights[ i ] );
		}
	}
	return ( NULL );
}

// コンテナの色替（ Lock On = Red , UnLock On = Green ）
static void ColorChenge_Seeker( SPR_OBJ **spr , SPR_COLOR *color )
{
	int i , j;

	for ( j = 0 ; j < SEEKER_LINE ; j++ ){
		for ( i = 0 ; i < 2 ; i++ ){
			spr[ j ]->line.col[ i ].r = color->r;
			spr[ j ]->line.col[ i ].g = color->g;
			spr[ j ]->line.col[ i ].b = color->b;
		}
	}
}

// サイトプライオリティ
static inline void SightsZbuf( Zbuf *pZbuf , float add , int position ) // 挿入選択
{
	if ( pZbuf->sights_z > add ){
		pZbuf->sights_z = add;
		pZbuf->position = position;
		return;
	}
}

static inline void SightsZbufInitialize( Zbuf *pZbuf )
{
	pZbuf->sights_z = 0xffffff;
	pZbuf->position = 0xff;
}

// ロックオンできるか？
static float CheckLockonAvailable( Work *pWork , TARGET *t , int target_point  )
{
	GM_CameraSet *pCam;
	SPR_OBJ     *spr;
	float		size;
	float		size_max;
	float       len;
	float       lock_position_y = 0.f; /* BP_WARNING added 0.f - lock_positive_y is used below */
	FVECTOR		center;
	FVECTOR		fvtmp;

	spr = NULL; // koba4 2001/07/09
	size = 0.0f;
	center = *(FVECTOR *)CENTER;
	pCam = GM_GetCurrentCameraSet( 0 );
	len = GV_VecLen3F2( &center, &( GM_GetCurrentCamera( 0 )->position ) ) ;
	if ( t != NULL && t->class & TARGET_LOCKON_AUTOSIZE ){ // 動的サイズ
		spr = SearchContenaNormal( pWork , t );
		if ( spr != NULL ){
			spr = spr->head.child->head.child;
			len = ( spr->box.rect.end.x - spr->box.rect.begin.x ) < ( spr->box.rect.end.y - spr->box.rect.begin.y ) ? 
					spr->box.rect.end.y - spr->box.rect.begin.y : spr->box.rect.end.x - spr->box.rect.begin.x;
			len /= ( 18.0f * 2.0f );
			if ( len < 1.0F ){
				len = 1.0F;
			}
		} else {
			len = 0.0f;
		}
		size = pWork->lockon_size * len;
	} else {
		spr = SearchContenaNormal( pWork , t );
		if ( spr != NULL ){
			spr = spr->head.child->head.child;
			len = ( spr->box.rect.end.x - spr->box.rect.begin.x ) < ( spr->box.rect.end.y - spr->box.rect.begin.y ) ? 
					spr->box.rect.end.y - spr->box.rect.begin.y : spr->box.rect.end.x - spr->box.rect.begin.x;
			len /= ( 18.0f * 2.0f );
			len *= ( 7.0f - GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) / 1800.0f );
			len *= pCam->angle;
			if ( len < 1.0F ){
				len = 1.0F;
			}
		} else {
			len = 0.0f;
		}
		size = pWork->lockon_size * len;
	}

	if ( LockonTarget != NULL ){
		if ( LockonTarget == t ){
			size_max = MAX_SIGHT_SIZE;
#ifdef PSX2
			center.vx = ( DRAW_WIDTH / 2 ) - DISP_POS->vx;
			center.vy = ( DRAW_HEIGHT / 2 ) - DISP_POS->vy - DIRECT_SCREEN_Y( lock_position_y );
#else
			center.vx = (SPR_SCRN_WIDTH/2) - DISP_POS->vx ;/*仮想座標なのでこっちでいいはず　なんだけど... T.Morita 2002.05.21 */
			center.vy = (SPR_SCRN_HEIGHT/2) - DISP_POS->vy ;
#endif
			center.vz = 0.0f;
			center.vw = 0.0f;
			if ( center.vx < - size_max || center.vx > size_max ||
				 center.vy < - size_max || center.vy > size_max ){
				pWork->lockon_count++;
			} else {
				pWork->lockon_count = 0;
			}
			SEEKER_ON = 1;
			if ( ( center.vx > -size ) && ( center.vx < size ) && ( center.vy > -size ) && ( center.vy < size ) ){
				_sceVu0SubVector( &fvtmp , CENTER , &GM_PlayerPosition );
				len = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );
				SightsZbuf( &pWork->Zbuf , len  , target_point ); // 挿入選択
			}
			// koba444
			return ( fpu_Sqrt( _sceVu0InnerProduct( &center , &center ) ) );
		} else {
			/* 現在のロック先がHIGH_PRIOか、
			   検査ターゲットがHIGH_PRIOでないならリターン */
			if ( ( LockonTarget->class & TARGET_LOCKON_HIGH_PRIO ) ){
				return ( 0xffffff );
			}
		}
	}
	/* ミサイル飛行中は新たにロックオンしない */
	if ( GM_WeaponAlive & WP_ALIVE_STINGER ) {
		if ( LockonTarget != NULL && 	//ロックオンしていて且つ
			( ( LockonTarget->class & TARGET_LOCKON_HIGH_PRIO ) ||	//それがHIGH_PRIOであるか、
			  !( t->class & TARGET_LOCKON_HIGH_PRIO ) ) ) {				//検査ターゲットがHIGHでないなら、
			return ( 0xffffff );										//あらたにロックはしない。
		}

	}

#ifdef PSX2
	center.vx = ( DRAW_WIDTH / 2 ) - DISP_POS->vx; 
	center.vy = ( DRAW_HEIGHT / 2 ) - DISP_POS->vy - DIRECT_SCREEN_Y( lock_position_y );
#else
	center.vx = (512/2) - DISP_POS->vx ;/*仮想座標なのでこっちでいいはず　なんだけど... T.Morita 2002.05.21 */
	center.vy = (384/2) - DISP_POS->vy ;
#endif
	center.vz = 0.0f;//CENTER->vz - GM_PlayerPosition.vz;
	center.vw = 0.0f;
	if ( center.vx < -size || center.vx > size || center.vy < -size || center.vy > size ){
		if ( SEEKER_ON != 1 ){
			return ( 0xffffff );
		}
	} else {
		_sceVu0SubVector( &fvtmp , CENTER , &GM_PlayerPosition );
		len = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );
		SightsZbuf( &pWork->Zbuf , len  , target_point ); // 挿入選択
	}
	SEEKER_ON = 1;

	return ( fpu_Sqrt( _sceVu0InnerProduct( &center , &center ) ) );
}

static void AfterImageColor( Work *pWork ) // 残像のカラー設定
{
	SPR_OBJ *spr;
	SPR_COLOR col;
	float tmp;
	int i , j;

	tmp = 1.0f;
	for ( i = 0 ; i < SEEKER_AFTERIMAGE ; i++ ){
		spr = pWork->seeker[ i ]->head.child;
		col = pWork->color;
		col.a *= tmp;
		tmp -= 0.02;
		for ( j = 0 ; j < SEEKER_LINE ; j++ ){
			spr->line.col[ 0 ] = col;
			spr->line.col[ 1 ] = col;
			spr = spr->head.next;
			if ( spr == NULL ){
				break;
			}
		}
	}
}

static void InitAfterImage( Work *pWork )
{
	int i;

	pWork->after_image_count = 1;

#if 0 /*yano 2002.03.19*/
	pWork->seeker[ 0 ]->empty.pos.x = ( float )( DRAW_WIDTH / 2 );
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	pWork->seeker[ 0 ]->empty.pos.y = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 56 );
#else
	pWork->seeker[ 0 ]->empty.pos.y = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 32 );
#endif

#else
	pWork->seeker[ 0 ]->empty.pos.x = ( float )( SPR_SCRN_WIDTH / 2 );
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	pWork->seeker[ 0 ]->empty.pos.y = ( float )( SPR_SCRN_HEIGHT / 2 );
//#else
//	pWork->seeker[ 0 ]->empty.pos.y = ( float )( SPR_SCRN_HEIGHT / 2 );
//#endif
#endif
	for ( i = 0 ; i < SEEKER_AFTERIMAGE ; i++ ){
		pWork->seeker_old_pos[ i ].x = 0xff;
		pWork->seeker_old_pos[ i ].y = 0xff;
      // BP_WARNING: Below was pWork->seeker_scale[ SEEKER_AFTERIMAGE ] = 0
      // This is clearly a bug, but the effect is that pWork->alive_flag is set
      // to 0, so we'll change the code to say that.
		// pWork->seeker_scale[ SEEKER_AFTERIMAGE ] = 0;
      pWork->alive_flag = 0;
		SPR_HIDE( pWork->seeker[ i ] );
	}
	AfterImageColor( pWork );
}

static int invisible( Work *pWork ) // 動かすものを消しておく
{
	GM_CameraSet *pCam;
	SPR_OBJ		*spr ;
	SPR_OBJ		*tmp ;
	int i;

	// サイトの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CONTENA_NULL ) ;
	if ( spr == NULL ){
		SK_Err( "spr 446 line\0" );
		return ( -1 );
	}
	// 緑にする
	tmp = spr->head.child->head.child;
	ColorChenge_Contena( tmp , spr->head.child->head.next , &default_color );
	for ( i = pWork->dup_count.sights ; i < MAX_SIGHTS ; i++ ){
		pWork->sights[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->sights[ i ] == NULL ){
			SK_Err("sights\0");
			return ( -1 );
		} else {
			pWork->dup_count.sights++;
		}
	}
	// シーカーの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_SEEKER ) ;
	if ( spr == NULL ){
		SK_Err( "pWork->seeker\0" );
		return ( -1 );
	}
	for( i = pWork->dup_count.seeker ; i < SEEKER_AFTERIMAGE ; i++ ){
		pWork->seeker[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->seeker[ i ] == NULL ){
			SK_Err("seeker\0");
			return ( -1 );
		} else {
			pWork->dup_count.seeker++;
		}
	}
	// センターカーソルの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CENTER_CURS ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->centercurs = spr ;
	// ピッチの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_PITCH ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	for ( i = pWork->dup_count.pitch ; i < PITCH_MAX ; i++ ){
		pWork->pitch[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->pitch[ i ] == NULL ){
			SK_Err("pitch\0");
			return ( -1 );
		} else {
			pWork->dup_count.pitch++;
		}
	}
	// シネマ
	spr = L2D_GetObject( pWork->handle_2d, STR_ROOT ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	spr->empty.pos.y -= DEFAULT_PLUS;
	
	// 残像の初期化
	InitAfterImage( pWork );

	// 回転角のinitialize
	pCam = GM_GetCurrentCameraSet( 0 );
	pWork->rot[ 0 ] = pWork->rot[ 1 ] = pCam->rotate; 

	// 比率
	pWork->zoom_raute = ( ANGLE_MAX / pWork->raute );

	pWork->flag |= SK_INIT_END;

	return ( 0 );
}

// 残像のコントロール
static void AfterImageControl( Work *pWork )
{
	SPR_OBJ *spr;
	int i , j;

	// αの減退
	for ( i = 1 ; i < SEEKER_AFTERIMAGE ; i++ ){
		spr = pWork->seeker[ i ]->head.child;
		for ( j = 0 ; j < SEEKER_LINE ; j++ ){
			spr->line.col[ 0 ].r /= 2;
			spr->line.col[ 0 ].g /= 2;
			spr->line.col[ 0 ].b /= 2;
			spr->line.col[ 0 ].a /= 2;
			spr->line.col[ 1 ].r /= 2;
			spr->line.col[ 1 ].g /= 2;
			spr->line.col[ 1 ].b /= 2;
			spr->line.col[ 1 ].a /= 2;
			if ( spr->line.col[ 0 ].a <= 0 ){	// 削除
				pWork->seeker_old_pos[ i ].x = 0xff;
				pWork->seeker_old_pos[ i ].y = 0xff;
				pWork->seeker_scale[ i ] = 0;
			}
			spr = spr->head.next;
			if ( spr == NULL ){
				break;
			}
		}
	}
	// スケール戻し
	for ( i = 1 ; i < SEEKER_AFTERIMAGE ; i++ ){
		pWork->seeker_scale[ i ] += 0.2;
		if ( pWork->seeker_scale[ i ] >= pWork->seeker_scale[ 0 ] ){	// 削除
			pWork->seeker_old_pos[ i ].x = 0xff;
			pWork->seeker_old_pos[ i ].y = 0xff;
			pWork->seeker_scale[ i ] = 0;
		}
	}
}

static void SeekerMove( Work *pWork , SPR_COLOR *pColor )
{
	GM_CameraSet *pCam;
	SPR_POS *pos , *tmp;
	SPR_OBJ *spr;
	FVECTOR ftmp;
	FVECTOR ftmp2;
	FMATRIX fmtmp;
	TARGET   *trg;
	float    len;
	float  scale;
	int        i;

	// 一応
	spr = NULL;
	pCam = GM_GetCurrentCameraSet( 0 );	
	// ターゲットが画面ないにいるか！
	if ( LOCKONTRG != NULL && !CheckTargetInSight( LOCKONTRG ) ){
		SearchContena( pWork , LOCKONTRG , &default_color );
		for ( i = 0 ; i < SEEKER_AFTERIMAGE ; i++ ){
			SPR_HIDE( pWork->seeker[ i ] ) ; // 非表示
		}
		return;
	}
	pos = &pWork->seeker[ 0 ]->empty.pos;
	tmp = (SPR_POS *)TARGET_POS;

	// 残像用に今の座標を登録
	pWork->seeker_old_pos[ pWork->after_image_count ] = pWork->seeker[ 0 ]->empty.pos;
	pWork->seeker_scale[ pWork->after_image_count++ ] = pWork->seeker_scale[ 0 ];
	if ( pWork->after_image_count >= SEEKER_AFTERIMAGE ){
		pWork->after_image_count = 1;
	}
	pWork->time = 0;
   	if ( LockonTarget != NULL ){ // ロックしたらぴったり
		*pos = *( SPR_POS * )TARGET_POS;
//		LOCKONTRG = PL_LockonTarget;
		ftmp2.vx = 0.f;
		ftmp2.vy = 0.f;
		pColor->r = 255;
		pColor->g = 5;
		pColor->b = 0;
		pColor->a = 128;
		SearchContena( pWork , LockonTarget , pColor );
		LockonTarget = LOCKONTRG ;
	} else {		
		_sceVu0SubVector( &ftmp , tmp , pos );
		_sceVu0DivVector( &ftmp , &ftmp , (float)ROCK_SPEED );
		// 加速度ゼロでロックオン
		ftmp2.vx = ftmp.vx < 0 ? -ftmp.vx : ftmp.vx;
		ftmp2.vy = ftmp.vy < 0 ? -ftmp.vy : ftmp.vy;
		// 加算
		_sceVu0AddVector( pos , pos , &ftmp );
		// ロックオン
//		if ( ftmp.vy <= ROCK_PERFORMANCE && ftmp2.vy <= ROCK_PERFORMANCE ){
			pColor->r = 255;
			pColor->g = 5;
			pColor->b = 0;
			pColor->a = 128;
			pWork->Sound_work.frame = 0; // ロックキャンセル
			LockonTarget = LOCKONTRG ;
			//		ATHOR_LOCK = NULL;
//		}
   	}	

	// 拡大
	trg = LOCKONTRG;
	if ( ( trg != NULL ) && ( trg->class & TARGET_LOCKON_AUTOSIZE ) ){
		DG_SetPos( &trg->world );
		DG_MovePos( &trg->offset );
		DG_GetPos( &fmtmp );
		DG_COPY_VEC( CENTER , ( FVECTOR * )fmtmp.m[ 3 ] );
		spr = SearchContenaNormal( pWork , LockonTarget );
		if ( spr != NULL ){
			spr = spr->head.child->head.child;
			len = ( spr->box.rect.end.x - spr->box.rect.begin.x ) < ( spr->box.rect.end.y - spr->box.rect.begin.y ) ? 
					spr->box.rect.end.y - spr->box.rect.begin.y : spr->box.rect.end.x - spr->box.rect.begin.x;
			len /= ( 18.0f * 2.0f );
			SCALE = len;
			if ( SCALE < 1.0F ){
				SCALE = 1.0F;
			}
		} else {
			SCALE = 0.0f;
		}
	} else { // 通常
		_sceVu0AddVector( CENTER, &trg->center, &trg->offset ) ;
		len = GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) ;
		SCALE = 4.0F + ( pCam->angle * pWork->zoom_raute ) - len / 2200.0F ;
		if ( SCALE < 1.0F ){
			SCALE = 1.0F;
		}
	}
	// 補間
	scale = ( SCALE - pWork->seeker_scale[ 0 ] ) / 10.f;
	pWork->seeker_scale[ 0 ] += scale;
	SPR_MAG( pWork->seeker[ 0 ] , pWork->seeker_scale[ 0 ] );
	// 残像
	SPR_SHOW( pWork->seeker[ 0 ] ) ; // メインは表示
	for ( i = 1 ; i < SEEKER_AFTERIMAGE ; i++ ){
		if ( pWork->seeker_old_pos[ i ].x != 0xff ){
			pWork->seeker[ i ]->empty.pos = pWork->seeker_old_pos[ i ];
			SPR_MAG( pWork->seeker[ i ] , pWork->seeker_scale[ i ] );
			SPR_SHOW( pWork->seeker[ i ] ) ; // メインは表示
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
#if 0 /*yano 2002.03.19*/
	tmp_width  = ( float )( DRAW_WIDTH / 2 );
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 56 );
#else
	tmp_height = ( float )( DRAW_HEIGHT / 2 ) - DIRECT_SCREEN_Y( 32 );
#endif

#else
	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
//#else
//	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
//#endif
#endif
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
	pWork->rot[ 0 ] = pCam->rotate;
	// etc
	SPR_MAG( pWork->centercurs , pWork->centercurs_scale );
	SPR_SHOW( pWork->centercurs ) ;
	SPR_HIDE( pWork->vlcn );
}

/*----------------------------------------------------------------*/
static void LockControl( Work *work )
{
	TARGET		*t, *tnext, *prelock;
	float       min_length, min_length_h;
	float       len;
	int			n_targets, i ;

	min_length = 0xffff;
	min_length_h = min_length ;

	SightsZbufInitialize( &work->Zbuf );
	prelock = LockonTarget ;
	for ( i = 0 ; i < MAX_SIGHTS ; i++ ){
		work->sights_color[ i ] = NULL;
	}
	if ( prelock != NULL && !( GM_WeaponAlive & WP_ALIVE_STINGER ) ) {
		if ( ++work->lockon_count > 60 ) {	/* 時間切れ */
			SearchContena( work , LockonTarget , &default_color );
			prelock = NULL;
			LockonTarget = NULL;
			SEEKER_ON = 0;
			work->alive_flag = 0;
			work->lockon_count = 0;
		}
	}
	LOCKONTRG = prelock;

	NEWLOCK = 0;
	n_targets = 0;
	tnext = &work->target;// GM_TrgDefListGetTop();
	for ( i = 0; i < MAX_SIGHTS; i ++ ){ // サイトの消去
		SPR_HIDE( work->sights[ i ] );
	}
	while( tnext != NULL ) {
		if ( n_targets >= MAX_SIGHTS ){
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
		work->sights_color[ n_targets ] = t; // このコンテナにはこの敵ということを教える
		len = CheckLockonAvailable( work , t , n_targets );
		if ( len != 0xffffff && SEEKER_ON == 1 ) {
			if ( LOCKONTRG != NULL && 
				 ( LOCKONTRG->class & TARGET_LOCKON_HIGH_PRIO ) ) {
				/* すでにHIGHが選択されているとき */
				if ( ( t->class & TARGET_LOCKON_HIGH_PRIO ) &&
					min_length_h > len && 
					t != ATHOR_LOCK ) {
					LOCKONTRG = t ;
					NEWLOCK = 1 ;
					*TARGET_POS = *DISP_POS;				
					min_length_h = len ;
					if ( min_length > len ) min_length = len ;
				}
			} else {
				/* NULLか、通常が選択されているとき */
				if ( min_length > len && t != ATHOR_LOCK ) {
					LOCKONTRG = t ;	
					NEWLOCK = 1 ;
					SEEKER_ON = 0;
					if ( n_targets == work->Zbuf.position || work->Zbuf.position == 0xff ){
						*TARGET_POS = *DISP_POS;				
					}
					if ( t->class & TARGET_LOCKON_HIGH_PRIO ) {
						min_length = min_length_h = len ;
					} else {
						min_length = len ;
					}
				}
			}
		} 
		DisplaySight( work , work->sights[ n_targets ] , t );
		SearchContena( work , t , &default_color );
		n_targets++;
	}
	// サウンド関係
	if ( !GM_CheckGameStatus( STATE_CHAFF ) ){
		if ( work->Sound_work.frame < 0 ){
			if ( LockonTarget != NULL ){
				work->Sound_work.frame = 2.f;
			} else {
				work->Sound_work.frame = DIRECT_TICK( 30.f );
			}
			work->Sound_work.frame = work->Sound_work.frame < 2.f ? 2.f : work->Sound_work.frame;
			work->Sound_work.frame = work->Sound_work.frame > 30.f ? 30.f : work->Sound_work.frame;
			GM_SeSetMode( SD_W_SIGNAL01, &DG_ZeroVector , GM_SEMODE_BOMB ) ;
		} else {
			work->Sound_work.frame--;
		}
	}
	if ( LOCKONTRG != NULL && prelock != LOCKONTRG ) {
		/* 違う奴にロックした */
		// 色を戻す
		work->color.r = 160;
		work->color.g = 250;
		work->color.b = 90;
		work->color.a = 48;
		work->lockon_count = 0 ;
		work->alive_flag = 0;
		SEEKER_ON = 0;
	}
	// サイト
	if ( min_length != 0xffff || work->lockon_count > 0 ){
		// 一番近い物を探す
		if ( work->Zbuf.position != 0xff ){
			LockonTarget = LOCKONTRG = work->sights_color[ work->Zbuf.position ];
			NEWLOCK = 1 ;
			SEEKER_ON = 0;
		}
		SeekerMove( work , &work->color );
		ColorChenge_Seeker( work->seeker_sub , &work->color );
		AfterImageColor( work );
		AfterImageControl( work );
	} else {
		// 色を戻す
		work->color.r = 160;
		work->color.g = 250;
		work->color.b = 90;
		work->color.a = 48;
		SEEKER_ON = 0;
		work->alive_flag = 0;
		// 残像を消す
		InitAfterImage( work );
	}
	if ( LOCKONTRG != NULL && NEWLOCK == 0 ) {
		int		exist = 0 ;
		/* 以前からのロック先が選ばれた場合、
		   それがＴＡＲＧＥＴリストに載っているか調べる */
		tnext = GM_TrgDefListGetTop() ;		
		while( tnext != NULL ) {
			t = tnext ;
			tnext = t->next ;
			if ( t == LOCKONTRG ) {
				exist = 1 ;
				break ;
			}
		}
		if ( exist == 0 ){
			LOCKONTRG = NULL ;
			SEEKER_ON = 0;
		}
	}
	CenterMove( work );
	if ( LOCKONTRG != NULL ){
		if ( LOCKONTRG->class & ( TARGET_DEAD | TARGET_SKIP ) ){
			// 色を戻す
			work->color.r = 160;
			work->color.g = 250;
			work->color.b = 90;
			work->color.a = 48;
			work->lockon_count = 0 ;
			work->alive_flag = 0;
			SearchContena( work , LockonTarget , &default_color );
			LockonTarget = NULL; // 解除
			LOCKONTRG = NULL ;
			InitAfterImage( work );
			SPR_HIDE( work->seeker[ 0 ] ) ; // 非表示
		}
	}
	if ( !( GM_WeaponAlive & WP_ALIVE_STINGER ) && ( work->alive_flag ) ){ // 爆発したということ
		SearchContena( work , LockonTarget , &default_color );
		prelock = NULL;
		LockonTarget = NULL;
		LOCKONTRG = NULL ;
		work->lockon_count = 0;
		work->alive_flag = 0;
		SEEKER_ON = 0;
		// 色を戻す
		work->color.r = 160;
		work->color.g = 250;
		work->color.b = 90;
		work->color.a = 48;
		ColorChenge_Seeker( work->seeker_sub , &work->color );
		// 残像を消す
		InitAfterImage( work );
	} else {
		work->alive_flag = GM_WeaponAlive & WP_ALIVE_STINGER;
	}
	SightsZbufInitialize( &work->Zbuf );
	work->time++;
}


static	void	NormalAct( Work *pWork )
{
	AnimationAct( pWork );
	if ( !( pWork->flag & SK_INIT_END ) ){
		return;
	}
	pWork->target.center = *pWork->mov; // 毎frame更新が必要
	LockControl( pWork );
	PitchMove( pWork );
	HeightMove( pWork );
}

#if 0
static void visible_frame( Work *pWork ) // ずれていた奴を補正
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle_2d, STR_SEEKER );
	spr->empty.pos.y += 256;
}
#endif

static void invisible2( Work *pWork )
{
	SPR_OBJ *pRoot;

	pRoot = ( SPR_OBJ * )L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( pRoot == NULL ){
		SK_Err( "root\0" );
		return;
	}
	SPR_HIDE( pRoot ); 
	pWork->action = 0;
	pWork->act = ( void *)AnimationAct;
}

static void BugAnimationAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
#if 0
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		pWork->action = STR_RAY_BUG;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
	}
#endif
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}
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
//				visible_frame( pWork );
				if ( invisible( pWork ) < 0 ){
					return;
				}
				SetMater( pWork );
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

static void Act( Work *pWork )
{
	if ( GM_CheckSightStatus( SGT_Stinger ) ){
		invisible2( pWork );
		return;
	}
	// デモ等のサイトの表示非表示管理
	if ( SthingerSightVisibleInvisible( pWork->handle_2d , STR_ROOT , ( pWork->togle & SK_STHINGER_MODEL_ON ? 1 : 0 ) ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
	if ( MsgDie( pWork ) < 0 ){
		return;
	}
	pWork->act( pWork );
}


static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
}

// static FVECTOR test = { 13055.0f , 5165.0f , -53716.0f , 0.0f};
/*----------------------------------------------------------------*/
static	int	GetResources( Work *work , int con_name )
{
	int			handle;
	int        strcode;
	int              i;
	EFTCONTROL    *ctrl = DM_GetEftControl( con_name );
	void          *ptr;

	if( !ctrl ){
		return ( -1 );
	}
	work->mov = &ctrl->mov;
	work->target.center = *work->mov; // 毎frame更新が必要
	work->target.offset = DG_ZeroVector;
	work->target.class = TARGET_LOCKON;// | TARGET_LOCKON_AUTOSIZE;
	work->target.map = GM_CurrentStageMap;
	work->target.side = ENEMY_SIDE;
	work->target.next = NULL;
	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		SK_Err( "kunibe----------------\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	// 謎？
	strcode = STR_SEEK01;
	for( i = 0 ; i < SEEKER_LINE ; i++ ){
		work->seeker_sub[ i ] = L2D_GetObject( handle, strcode );
		if ( work->seeker_sub[ i ] == NULL ){
			SK_Err( "seeker_sub\0" );
			return -1 ;
		}
		strcode++;
	}
	// コンテナ
	strcode = STR_CONTENA1;
	work->contena_box[ 0 ] = L2D_GetObject( handle, strcode );
	if ( work->contena_box[ 0 ] == NULL ){
		SK_Err( "contena_box\0" );
		return -1 ;
	}
	// バルカン
	work->vlcn = L2D_GetObject( work->handle_2d, STR_VLCN );
	if ( work->vlcn == NULL ){
		return -1 ;
	}
	// ドット
	strcode = STR_DOT;
	for( i = 0 ; i < CONTENA_DOT ; i++ ){
		work->contena_dot[ i ] = L2D_GetObject( handle, strcode );
		if ( work->contena_dot[ i ] == NULL ){
			SK_Err( "contena_dot\0" );
			return -1 ;
		}
		strcode++;
	}
	work->flag = SK_UN_INITIALIZE;
	work->color.r = 180;
	work->color.g = 250;
	work->color.b = 160;
	// 上バーの初期化
	work->mt_bar_work.speed = 0.f;
	work->mt_bar_work.size  = 29.f;
	work->mt_bar_work.frame = 0.f;
	// サウンド
	work->Sound_work.frame = 60;
	// etc
	work->lockon_size = 15;
	work->flag = SK_ACTION_START;
	work->time = 0;
	work->alive_flag = 0;
	LockonTarget = NULL;
	ATHOR_LOCK = NULL;
	SightsZbufInitialize( &work->Zbuf );

	work->action = STR_DEFAULT;
	work->act = (void *)NormalAct;
	// dup
	work->dup_count.seeker = 0;
	work->dup_count.pitch = 0;
	work->dup_count.sights = 0;
	// laster
	ptr = NewRasterEffect( SCE_GS_SET_ALPHA(  0 , 1 , 0 , 1 , 0x00 ) , 0x36000020 );
	if ( ptr == NULL ){
		SK_Err("Non Memory\0");
		return ( -1 );
	}
	GV_SetActorChild( work , ptr );

	return ( 0 );
}

/*----------------------------------------------------------------*/

void	*NewAiRaySight( int name , int con_name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	work->name = name;
	work->raute = 24.0f;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work , con_name ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void    *NewScnAiRaySight( int name , int con_name )
{
	return NewAiRaySight( name , con_name );
}

