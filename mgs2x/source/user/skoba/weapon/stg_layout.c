//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   stg_sight.c
   ロックオンサイト
   
   2001/04/12	S.Kobayashi
   $Id: stg_layout.c,v 1.2 2002/12/05 18:41:58 takaki Exp $
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

#include "libfs.h"

#include <libutl.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"

#include "BP_Misc.h"

#define		LAYOUT		(121575)	/* stg.o2d */
#define     MAX_OBJ     (8)
#define 	MAX_SIGHTS  (42)
#define		MAX_SIGHT_SIZE (100)
#define     SEEKER_LINE (4)
#define     SEEKER_AFTERIMAGE (4)
#define     CONTENA_DOT (4)
#define     CONTENA_BOX (1)
#define 	MEATER_MAX  (10)
#define     NUMBER_MAX (MEATER_MAX * 2)
#define     MT_BAR_MAX  (2)
#define     STR_SEEKER       (8665598)
#define     STR_MATER_1      (7630874)
#define     STR_MATER_2      (7630875)
#define     STR_CONTENA_NULL (6314366)
#define     STR_CURSOR_T     (7113343)
#define     STR_CURSOR_B     (7113325)
#define     STR_MATER_2      (7630875)
#define     STR_COVER        (6907160) // not use
#define     STR_DEFAULT      (566267)
#define     STR_AIMAREA      (15887897) // GV_StrCode( "aimArea" )
#define     STR_SEEK01       (8663837)  // GV_StrCode( "seek01" );
#define     STR_CONTENA1     (7829748)  //GV_StrCode( "contena1" );
#define     STR_DOT          (3394225)  // GV_StrCode( "dot1" );
#define 	STR_FIX          (74104)
#define     STR_MT_BAR_NULL  (724185)
#define     STR_MT_BAR2      (2272928)
#define 	STR_ROOT         (2770484)

#define     DIE_FLG                (1)
#define     ROCK_SPEED             (1) // 16
//#define     ROCK_SPEED            (64)
//#define     ROCK_SPEED            (2000)
#define     ROCK_PERFORMANCE       (1.0f) // 0.1f

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
	FVECTOR             pos;
} Zbuf;

typedef struct {
	int sights;
	int seeker;
	int mater_1;
	int mater_2;
} DupCount;

typedef	struct _work {
	GV_ACT_EX			actor ;
	SPR_EMPTY           *cover; // test
	SPR_OBJ			    *center_m;
	SPR_OBJ		    	*center_b;
  	SPR_OBJ			    *mater_2[ MEATER_MAX ]; // empty
	SPR_OBJ				*sights[ MAX_SIGHTS ];
	SPR_OBJ				*seeker[ SEEKER_AFTERIMAGE ]; // empty
	SPR_OBJ             *mater_1[ NUMBER_MAX ]; // empty
	SPR_OBJ				*mt_bar[ MT_BAR_MAX ]; // line
	SPR_OBJ             *seeker_sub[ SEEKER_LINE ];    // 色を変える時に使用
	SPR_OBJ             *aimArea;
	SPR_OBJ             *contena_box[ CONTENA_BOX ];
	SPR_OBJ             *contena_dot[ CONTENA_DOT ];
	FVECTOR             center_b_pos;
	Number              Num[ NUMBER_MAX ];
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
	int                 flag;
	int                 action;
	int                 action_num;
	int                 name;
	int					handle_2d;
	char                togle; // モデルありモード or モデルなしモード
	void				*layout_player;
	void                *menuprint_work_ptr;
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
extern TARGET	*PL_LockonTarget ;
static SPR_COLOR default_color = SPR_COLOR_CTOR( 160 , 250 , 90 , 64 );
static int SK_LockOnSize; // シナリオから変更可能

// プロトタイプ
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );

static int AnimetionAct( Work * ); // アニメーション

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
		msg--;
	}
	return ( 0 );
}

/* 画面内チェック */
static	int		CheckTargetInSight( TARGET *t )
{
	FVECTOR		max, min, offset ;
	FVECTOR     fvtmp;
	float       len;

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
	DISP_POS->vx = ( DISP_POS->vx ) * SPR_SCRN_WIDTH  / ( float )DRAW_WIDTH;
	DISP_POS->vy = ( DISP_POS->vy ) * SPR_SCRN_HEIGHT / ( float )DRAW_HEIGHT;
	//DumpVec( DISP_POS ) ;

	// 距離から
	_sceVu0SubVector( &fvtmp , CENTER , &GM_PlayerPosition );
	len = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );

	if ( len < 1000 ){
		return 0;
	} else {
		return 1 ;
	}
}

/* サイトの表示 */
static	void	DisplaySight( SPR_OBJ *spr , TARGET *pTrg )
{
    SPR_OBJ     *spr_box;
	FMATRIX     fmtmp;
	FVECTOR     fvtmp;
	FVECTOR     getpos_left;
	FVECTOR     getpos_right;
	FVECTOR     worktmp;
	FVECTOR     size_value;
	float	    len;

	SIGHT_POS->x = ( DISP_POS->vx );
	SIGHT_POS->y = ( DISP_POS->vy );

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
		fmtmp = DG_Chanl( 0 )->eye;
		DG_COPY_VEC( ( FVECTOR * )fmtmp.m[ 3 ] , &DG_ZeroVector );
		_sceVu0ApplyMatrix( &size_value , &fmtmp , &size_value );

		// 左上
		_sceVu0SubVector( &worktmp , &fvtmp , &size_value );
		DG_TransPersOne( &getpos_left , &worktmp );
		// 右下
		_sceVu0AddVector( &worktmp , &fvtmp , &size_value );
		DG_TransPersOne( &getpos_right , &worktmp );

		getpos_left.vw = 1.0f;
		getpos_right.vw = 1.0f;
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
		SCALE = 6.0F - len / 2000.0F ;
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
static inline void SightsZbuf( Zbuf *pZbuf , float vx , float vy , float add , int position ) // 挿入選択
{
	if ( pZbuf->sights_z > add ){
		pZbuf->sights_z = add;
		pZbuf->position = position;
		pZbuf->pos.vx = vx;
		pZbuf->pos.vy = vy;
		pZbuf->pos.vz = 0.0f;
		pZbuf->pos.vw = 0.0f;

		return;
	}
}

static inline void SightsZbufInitialize( Zbuf *pZbuf )
{
	pZbuf->sights_z = 0xffffff;
	pZbuf->position = 0xff;
	DG_COPY_VEC( &pZbuf->pos , &DG_ZeroVector );
}

// ロックオンできるか？
static float CheckLockonAvailable( Work *pWork , TARGET *t , int target_point  )
{
	SPR_OBJ     *spr;
	float		size;
	float		size_max;
	float       len;
	FVECTOR		center;
	FVECTOR		fvtmp;

	spr = NULL; // koba4 2001/07/09
	size = 0.0f;
	center = *(FVECTOR *)CENTER;
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
		size = SK_LockOnSize * len;
	} else {
		spr = SearchContenaNormal( pWork , t );
		if ( spr != NULL ){
			spr = spr->head.child->head.child;
			len = ( spr->box.rect.end.x - spr->box.rect.begin.x ) < ( spr->box.rect.end.y - spr->box.rect.begin.y ) ? 
					spr->box.rect.end.y - spr->box.rect.begin.y : spr->box.rect.end.x - spr->box.rect.begin.x;
			len /= ( 18.0f * 2.0f );
			len *= ( 7.0f - GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) / 1800.0f );
			if ( len < 1.0F ){
				len = 1.0F;
			}
		} else {
			len = 0.0f;
		}
		size = SK_LockOnSize * len;
	}

	if ( PL_LockonTarget != NULL ){
		if ( PL_LockonTarget == t ){
			size_max = MAX_SIGHT_SIZE;
			center.vx = ( SPR_SCRN_WIDTH / 2 ) - DISP_POS->vx; 
			center.vy = ( SPR_SCRN_HEIGHT / 2 ) - DISP_POS->vy;
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
				SightsZbuf( &pWork->Zbuf , DISP_POS->vx , DISP_POS->vy , len  , target_point ); // 挿入選択
			}
			// koba444
			return ( fpu_Sqrt( _sceVu0InnerProduct( &center , &center ) ) );
		} else {
			/* 現在のロック先がHIGH_PRIOか、
			   検査ターゲットがHIGH_PRIOでないならリターン */
#if 0 // by koba4
			if ( ( PL_LockonTarget->class & TARGET_LOCKON_HIGH_PRIO ) ||
				!( t->class & TARGET_LOCKON_HIGH_PRIO ) ) return ( 0xffffff );
#endif
			if ( ( PL_LockonTarget->class & TARGET_LOCKON_HIGH_PRIO ) ){
				return ( 0xffffff );
			}
		}
	}
	/* ミサイル飛行中は新たにロックオンしない */
	if ( GM_WeaponAlive & WP_ALIVE_STINGER ) {
		if ( PL_LockonTarget != NULL && 	//ロックオンしていて且つ
			( ( PL_LockonTarget->class & TARGET_LOCKON_HIGH_PRIO ) ||	//それがHIGH_PRIOであるか、
			  !( t->class & TARGET_LOCKON_HIGH_PRIO ) ) ) {				//検査ターゲットがHIGHでないなら、
			return ( 0xffffff );										//あらたにロックはしない。
		}

	}
	center.vx = ( SPR_SCRN_WIDTH / 2 ) - DISP_POS->vx;
	if ( pWork->togle == SK_STHINGER_MODEL_ON ){
		center.vy = ( SPR_SCRN_HEIGHT / 2 ) - DISP_POS->vy - DIRECT_SCREEN_Y( 64.f );
	} else {
		center.vy = ( SPR_SCRN_HEIGHT / 2 ) - DISP_POS->vy ;
	}
	center.vz = 0.0f;//CENTER->vz - GM_PlayerPosition.vz;
	center.vw = 0.0f;
	if ( center.vx < -size || center.vx > size || center.vy < -size || center.vy > size ){
		if ( SEEKER_ON != 1 ){
			return ( 0xffffff );
		}
	} else {
		_sceVu0SubVector( &fvtmp , CENTER , &GM_PlayerPosition );
		len = fpu_Sqrt( _sceVu0InnerProduct( &fvtmp , &fvtmp ) );
		SightsZbuf( &pWork->Zbuf , DISP_POS->vx , DISP_POS->vy , len  , target_point ); // 挿入選択
	}
	SEEKER_ON = 1;

	return ( fpu_Sqrt( _sceVu0InnerProduct( &center , &center ) ) );
}

static void MtBarMove( Work *pWork )
{
	float     tmp;

	if ( pWork->mt_bar_work.frame <= 0 ){ // 新規
		tmp = ( int )( frnd() * 440 ); 
		tmp = tmp < 244 ? 244 : tmp;
		tmp -= rnd() * 20;
		pWork->mt_bar_work.frame = rnd() * 60 + 20;
		pWork->mt_bar_work.speed = ( tmp - pWork->mt_bar_work.size ) / pWork->mt_bar_work.frame;
	}
	pWork->mt_bar[ 0 ]->line.pos[ 0 ].x = 29;
	pWork->mt_bar[ 0 ]->line.pos[ 0 ].y = 31;
	pWork->mt_bar[ 0 ]->line.pos[ 1 ].x = pWork->mt_bar_work.size; 
	pWork->mt_bar[ 0 ]->line.pos[ 1 ].y = 31;
	pWork->mt_bar[ 1 ]->line.pos[ 0 ].x = pWork->mt_bar_work.size; 
	pWork->mt_bar[ 1 ]->line.pos[ 0 ].y = 31;
	pWork->mt_bar[ 1 ]->line.pos[ 1 ].x = pWork->mt_bar_work.size;
	pWork->mt_bar[ 1 ]->line.pos[ 1 ].y = 41;
	
	pWork->mt_bar_work.size += pWork->mt_bar_work.speed;
	if ( pWork->mt_bar_work.size < 244 ){
		pWork->mt_bar_work.size = 244;
		pWork->mt_bar_work.frame = 0; 
	} else {
		pWork->mt_bar_work.frame--;
	}
	SPR_SHOW( pWork->mt_bar[ 0 ] );
	SPR_SHOW( pWork->mt_bar[ 1 ] );
}

static void SetMater_2( Work *pWork ) // メーターに初期値設定
{
	int i;
	float tmp , tmp2;

	tmp = ( 384.f / 448.f );
	for ( i = 0 ; i < MEATER_MAX ; i++ ){
		// 数値のポジション,枠のポジション
		pWork->mater_2[ i ]->empty.pos.x = 0;
		pWork->mater_2[ i ]->empty.pos.y = ( float )( DRAW_HEIGHT ) - ( 60 * i );
	}
	for ( i = 0 ; i < NUMBER_MAX ; i++ ){
		tmp2 =  ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) - ( float )( DIRECT_SCREEN_Y( 41 ) * ( float )i ) ) * tmp;
		pWork->Num[ i ].parent_pos.vx = 0;
		pWork->Num[ i ].parent_pos.vy = tmp2;
		pWork->Num[ i ].number_pos.vx = 51;
		pWork->Num[ i ].number_pos.vy = tmp2;
		pWork->mater_1[ i ]->empty.pos.x = 0;
      if ( BP_Area_EU() )
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   		pWork->mater_1[ i ]->empty.pos.y = ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 104 ) ) - ( float )( DIRECT_SCREEN_Y( 20 ) * ( float )i ) );
      else
//#else
		   pWork->mater_1[ i ]->empty.pos.y = ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 102 ) ) - ( float )( DIRECT_SCREEN_Y( 30 ) * ( float )i ) );
//#endif
	}
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
		tmp -= 0.02f;
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
	pWork->seeker[ 0 ]->empty.pos.x = ( float )( SPR_SCRN_WIDTH / 2 );
	pWork->seeker[ 0 ]->empty.pos.y = ( float )( SPR_SCRN_HEIGHT / 2 );
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
	int  strcode_child;
	int i;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return ( 0 );
	}
	if ( pWork->flag & SK_UN_INITIALIZE ){
		pWork->flag |= SK_INITIALIZE_START;
		pWork->flag &= ~SK_UN_INITIALIZE;
		return ( -1 );
	}

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
			SK_Err( "sights\0" );
//			SK_FreeMemory( pWork->sights , i );
			return ( -1 );
		} else {
			pWork->dup_count.sights++;
		}
	}
	SPR_HIDE( spr );
	// シーカーの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_SEEKER ) ;
	if ( spr == NULL ){
		SK_Err( "pWork->seeker\0" );
		return ( -1 );
	}
	for( i = pWork->dup_count.seeker ; i < SEEKER_AFTERIMAGE ; i++ ){
		pWork->seeker[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->seeker[ i ] == NULL ){
			SK_Err( "seeker\0" );
			return ( -1 );
		} else {
			pWork->dup_count.seeker++;
		}
	}
	SPR_HIDE( spr );
	// メーターの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_MATER_2 ) ;
	if ( spr == NULL ){
		SK_Err( "pWork->mater_2\0" );
		return -1 ;
	}
	for( i = pWork->dup_count.mater_2 ; i < MEATER_MAX ; i++ ){
		pWork->mater_2[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->mater_2[ i ] == NULL ){
			SK_Err( "mater_2\0" );
			return ( -1 );
		} else {
			pWork->dup_count.mater_2++;
		}
	}
	SPR_HIDE( spr );
	// メーターの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_MATER_1 ) ;
	if ( spr == NULL ){
		SK_Err( "pWork->mater_1\0" );
		return -1 ;
	}
	for( i = pWork->dup_count.mater_1 ; i < NUMBER_MAX ; i++ ){
		pWork->mater_1[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->mater_1[ i ] == NULL ){
			SK_Err( "mater_1\0" );
			return ( -1 );
		} else {
			pWork->dup_count.mater_1++;
		}
	}
	SPR_HIDE( spr );
	// 上の棒の取得
	strcode_child = STR_MT_BAR2;
	for ( i = 0 ; i < 2 ; i++ ){
		spr = L2D_GetObject( pWork->handle_2d, strcode_child ) ;
		if ( spr == NULL ){
			SK_Err( "pWork->mt_bar\0" );
			return ( -1 );
		}
		pWork->mt_bar[ i ] = spr ;
		strcode_child++;
	}
	// メーターの初期化
	SetMater_2( pWork );
	// 残像の初期化
	InitAfterImage( pWork );

	pWork->flag |= SK_INITIALIZE_OK;
	pWork->flag &= ~SK_INITIALIZE_START;

	// 回転角のinitialize
	pCam = GM_GetCurrentCameraSet( 0 );
	pWork->rot[ 0 ] = pWork->rot[ 1 ] = pCam->rotate; 

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
		pWork->seeker_scale[ i ] += 0.2f;
		if ( pWork->seeker_scale[ i ] >= pWork->seeker_scale[ 0 ] ){	// 削除
			pWork->seeker_old_pos[ i ].x = 0xff;
			pWork->seeker_old_pos[ i ].y = 0xff;
			pWork->seeker_scale[ i ] = 0;
		}
	}
}

// 数字情報の描画
static void NumberDisp( Work *pWork )
{
	char text_buffer[ 4 ];
	int i;

	if ( pWork->togle == SK_STHINGER_MODEL_ON || pWork->flag & SK_INVISIBLE ){ // 特殊動作
		return;
	}
	if ( pWork->mater_2[ 0 ]->head.flags & SPR_FLAG_HIDDEN ){ // BOX初期化まち
		return;
	}
	for ( i = 0 ; i < NUMBER_MAX ; i++ ){
		sprintf( text_buffer , "%02d" , ( i * 5 ) - 40 );
		___MENU_Locate( pWork->menuprint_work_ptr, 2, pWork->Num[ i ].number_pos.vx , pWork->Num[ i ].number_pos.vy , 1 );
		___MENU_Color( pWork->menuprint_work_ptr, 2, 240, 240, 120, ( u_char )32 );
		___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
#if 0
		MENU_S_Locate( pWork->Num[ i ].number_pos.vx , pWork->Num[ i ].number_pos.vy , 1 );
		MENU_S_Color( 200 , 240 , 120 , 32 );
		MENU_S_Printf( "%d" , ( i * 5 ) - 40 );
#endif
	}
}

static void SeekerMove( Work *pWork , SPR_COLOR *pColor )
{
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
   	if ( PL_LockonTarget != NULL ){ // ロックしたらぴったり
		*pos = *( SPR_POS * )TARGET_POS;
//		LOCKONTRG = PL_LockonTarget;
		ftmp2.vx = 0.f;
		ftmp2.vy = 0.f;
		pColor->r = 255;
		pColor->g = 5;
		pColor->b = 0;
		pColor->a = 128;
		SearchContena( pWork , PL_LockonTarget , pColor );
		PL_LockonTarget = LOCKONTRG ;
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
			PL_LockonTarget = LOCKONTRG ;
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
		spr = SearchContenaNormal( pWork , PL_LockonTarget );
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
		SCALE = 6.0F - len / 2000.0F ;
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

#define AIM_CENTER_MAX 110
#define AIM_WIDTH  188
#define AIM_HEIGHT 140
#define AIM_RATIO  ( float )( (float)AIM_HEIGHT / (float)AIM_WIDTH )
static void AimMove( Work *pWork , FVECTOR *pFtmp )
{
	FVECTOR ftmp , ftmp2;
	float   tmp_width , tmp_height;
	float   tmp;


	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2 );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2 );
	_sceVu0DivVector( pFtmp , pFtmp , 2.0f );
	// エイム
	pWork->aimArea->box.rect.begin.x += pFtmp->vw;
	pWork->aimArea->box.rect.begin.y -= pFtmp->vz;
	pWork->aimArea->box.rect.end.x += pFtmp->vw;
	pWork->aimArea->box.rect.end.y -= pFtmp->vz;

	// 抑制
	ftmp.vw = pFtmp->vw < 0 ? -pFtmp->vw : pFtmp->vw;
	if ( pFtmp->vw != 0 ){
		if ( pWork->aimArea->box.rect.begin.x < -AIM_CENTER_MAX + tmp_width ){
			pWork->aimArea->box.rect.begin.x = -AIM_CENTER_MAX + tmp_width;
			pWork->aimArea->box.rect.end.x = -AIM_CENTER_MAX + AIM_WIDTH + tmp_width;
		}
		if ( pWork->aimArea->box.rect.end.x > AIM_CENTER_MAX + tmp_width ){
			pWork->aimArea->box.rect.begin.x = AIM_CENTER_MAX - AIM_WIDTH + tmp_width;
			pWork->aimArea->box.rect.end.x = AIM_CENTER_MAX + tmp_width;
		}
	}
	ftmp.vz = pFtmp->vz < 0 ? -pFtmp->vz : pFtmp->vz;
	if ( pFtmp->vz != 0 ){
		tmp = (float)-AIM_CENTER_MAX * AIM_RATIO;
		if ( pWork->aimArea->box.rect.begin.y < tmp + tmp_height ){
			pWork->aimArea->box.rect.begin.y = tmp + tmp_height;
			pWork->aimArea->box.rect.end.y = tmp + AIM_HEIGHT + tmp_height;
		}
		tmp = (float)AIM_CENTER_MAX * AIM_RATIO;
		if ( pWork->aimArea->box.rect.end.y > tmp + tmp_height ){
			pWork->aimArea->box.rect.begin.y = tmp - AIM_HEIGHT + tmp_height;
			pWork->aimArea->box.rect.end.y = tmp + tmp_height;
		}
	}
	// 戻す
	if ( pFtmp->vw == 0 && pFtmp->vz == 0 ){
		ftmp.vx = pWork->aimArea->box.rect.begin.x + (float)AIM_WIDTH / 2;
		ftmp.vy = pWork->aimArea->box.rect.begin.y + (float)AIM_HEIGHT / 2;
		ftmp.vz = 0;
		ftmp.vw = 0;
		ftmp2.vx = tmp_width;
		ftmp2.vy = tmp_height;
		ftmp2.vz = 0;
		ftmp2.vw = 0;
		_sceVu0SubVector( &ftmp , &ftmp2 , &ftmp ); // 加速度
		_sceVu0DivVector( &ftmp , &ftmp , 10 );
		pWork->aimArea->box.rect.begin.x += ftmp.vx; 
		pWork->aimArea->box.rect.end.x   += ftmp.vx;
		pWork->aimArea->box.rect.begin.y += ftmp.vy;
		pWork->aimArea->box.rect.end.y   += ftmp.vy;
	}

	SPR_SHOW( pWork->aimArea ) ;
}

#define CENTER_MAX 20
#define CENTER_SPEED_X 3
#define CENTER_SPEED_Y 3
#define OFFSET_CENTER_B 0
static void CenterMove( Work *pWork )
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	FVECTOR       fvec;
	FVECTOR       ftmp3;
	FVECTOR        pos;
	float         ftmp;
	float        ftmp2;
	float        tmp_width;
	float        tmp_height;
	float            r;
	int           sign;

	pCam = GM_GetCurrentCameraSet( 0 );	

	stmp.vx = pCam->rotate.vx - pWork->rot[ 0 ].vx; // 移動した角度を入力
	stmp.vy = pCam->rotate.vy - pWork->rot[ 0 ].vy; // 移動した角度を入力
	tmp_width  = ( float )( SPR_SCRN_WIDTH / 2.0f );
	tmp_height = ( float )( SPR_SCRN_HEIGHT / 2.0f );
	if ( stmp.vx == 0 || stmp.vy == 0 ){ // 一次元か二次元か
		r = 20.f;
	} else {
		r = 20.f;
	}
	ftmp = 2.0f * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360.0f );  
	stmp.vw  = ( ( ftmp * stmp.vy ) / 360.0f );
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
	pWork->center_m->empty.pos.x -= ( int )( stmp.vw );
	pWork->center_m->empty.pos.y += ( int )( stmp.vz );
	pWork->rot[ 0 ] = pCam->rotate; // 更新

	// 範囲ない制御
	ftmp2 = stmp.vw < 0 ? -stmp.vw : stmp.vw;
	if ( stmp.vw != 0 ){
		if ( pWork->center_m->empty.pos.x < -CENTER_MAX ){
			pWork->center_m->empty.pos.x = -CENTER_MAX;
		}
		if ( pWork->center_m->empty.pos.x > CENTER_MAX ){
			pWork->center_m->empty.pos.x = CENTER_MAX;
		}
	}
	ftmp2 = stmp.vz < 0 ? -stmp.vz : stmp.vz;
	if ( stmp.vz != 0 ){
		if ( pWork->center_m->empty.pos.y < -CENTER_MAX ){
			pWork->center_m->empty.pos.y = -CENTER_MAX;
		}
		if ( pWork->center_m->empty.pos.y > CENTER_MAX ){
			pWork->center_m->empty.pos.y = CENTER_MAX;
		}
	}
	// 下のカーソルの慣性移動
	fvec.vx = pWork->center_m->empty.pos.x;
	fvec.vy = pWork->center_m->empty.pos.y;// + OFFSET_CENTER_B;
	fvec.vz = fvec.vw = 0;
	_sceVu0SubVector( &pos , &fvec , &pWork->center_b_pos ); // 加速度
	_sceVu0DivVector( &pos , &pos , 10.0f );
	// 更新
	_sceVu0AddVector( &pWork->center_b_pos , &pWork->center_b_pos , &pos );
	pWork->center_b->empty.pos.x = pWork->center_b_pos.vx;
	if ( pWork->center_b_pos.vy < pWork->center_m->empty.pos.y + OFFSET_CENTER_B ){
		pWork->center_b_pos.vy = pWork->center_m->empty.pos.y + OFFSET_CENTER_B;
		pWork->center_b->empty.pos.y = pWork->center_b_pos.vy;
	} else {
	  pWork->center_b->empty.pos.y = pWork->center_b_pos.vy;
	}
	// エイムの移動
	AimMove( pWork , &stmp );
	// 戻す
	if ( stmp.vw == 0 && stmp.vz == 0 ){
		stmp.vx = pWork->center_m->empty.pos.x;
		stmp.vy = pWork->center_m->empty.pos.y;
		stmp.vz = 0;
		stmp.vw = 0;
		ftmp3.vx = 0;//tmp_width;
		ftmp3.vy = 0;//tmp_height;
		ftmp3.vz = 0;
		ftmp3.vw = 0;
		_sceVu0SubVector( &fvec , &ftmp3 , &stmp ); // 加速度
		_sceVu0DivVector( &fvec , &fvec , 10 );
		pWork->center_m->empty.pos.x += fvec.vx;
		pWork->center_m->empty.pos.y += fvec.vy;
	}
	SPR_SHOW( pWork->center_m ) ;
	SPR_SHOW( pWork->center_b ) ;
}

 
#define CENTER_MAX 20
#define MATER_SPEED_Y 20

static void NumberMove( Work *pWork ) // 数字移動
{
	GM_CameraSet *pCam;
	FVECTOR       stmp;
	float        speed;
	float         ftmp;
	float            r;
	float          tmp;
	float         diff;
	int           sign;
	int              i;

	pCam = GM_GetCurrentCameraSet( 0 );	

	stmp.vx = pCam->rotate.vx - pWork->rot[ 1 ].vx; // 移動した角度を入力
	stmp.vx /= 1024.f;
	r = 4000.f;

	ftmp = 2.0f * M_PI * r; // 31半径のこと(円周をだす)
	stmp.vz  = ( ( ftmp * stmp.vx ) / 360.0f );
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
	pWork->rot[ 1 ] = pCam->rotate; // 更新
	
	for ( i = 0 ; i < MEATER_MAX ; i++ ){
		pWork->mater_2[ i ]->empty.pos.y += stmp.vz;
		SPR_SHOW( pWork->mater_2[ i ] );
		// オブジェクトの削除(更新)
		if ( pWork->mater_2[ i ]->empty.pos.y < DRAW_HEIGHT - 60 * ( MEATER_MAX ) ){
			diff = pWork->mater_2[ i ]->empty.pos.y - ( DRAW_HEIGHT - 60 * ( MEATER_MAX ) );
			pWork->mater_2[ i ]->empty.pos.y = DRAW_HEIGHT + diff;
		}
		if ( pWork->mater_2[ i ]->empty.pos.y > DRAW_HEIGHT){
			diff = pWork->mater_2[ i ]->empty.pos.y - DRAW_HEIGHT; 
			pWork->mater_2[ i ]->empty.pos.y = DRAW_HEIGHT - 60 * ( MEATER_MAX ) + diff;
		}
	}
	tmp = ( 384.f / 448.f );
	for ( i = 0 ; i < NUMBER_MAX ; i++ ){
		pWork->Num[ i ].parent_pos.vy += ( stmp.vz / tmp );
		// オブジェクトの削除(更新)
		if ( pWork->Num[ i ].number_pos.vy < ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) -
														( float )( DIRECT_SCREEN_Y( 41 ) * ( NUMBER_MAX ) ) ) * tmp ){
		  	diff = pWork->Num[ i ].parent_pos.vy - ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) -
															  ( float )( DIRECT_SCREEN_Y( 41 ) * ( NUMBER_MAX ) ) ) * tmp;
			pWork->Num[ i ].parent_pos.vy = ( float )( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) * tmp + diff;
			pWork->Num[ i ].number_pos.vy = ( float )( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) * tmp + diff;
		}
		if ( pWork->Num[ i ].number_pos.vy > ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) * tmp ){
			diff = pWork->Num[ i ].parent_pos.vy - ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) * tmp;
			pWork->Num[ i ].parent_pos.vy = ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) -
													   ( float )( DIRECT_SCREEN_Y( 41 ) * ( NUMBER_MAX ) ) ) * tmp + diff;
			pWork->Num[ i ].number_pos.vy = ( float )( ( DRAW_HEIGHT + DIRECT_SCREEN_Y( 136 ) ) -
													   ( float )( DIRECT_SCREEN_Y( 41 ) * ( NUMBER_MAX ) ) ) * tmp + diff;
		}
		// 数字の更新
		speed = ( float )( pWork->Num[ i ].parent_pos.vy - pWork->Num[ i ].number_pos.vy ) / 10.f;

		pWork->Num[ i ].number_pos.vy += speed;
      if ( BP_Area_EU() )
      {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   		pWork->mater_1[ i ]->empty.pos.y = ( ( pWork->Num[ i ].number_pos.vy + 3 ) * 384 ) / DRAW_HEIGHT;
      }
//#else
      else
      {
		   pWork->mater_1[ i ]->empty.pos.y = ( pWork->Num[ i ].number_pos.vy + 6 ) * tmp;
      }
//#endif
		SPR_SHOW( pWork->mater_1[ i ] );
	}

	//	pWork->cover->head.alpha = SCE_GS_SET_ALPHA( 1 , 2 , 2 , 1 , 0x00 );
	//	SPR_SHOW( pWork->cover );
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

	// initialize
	if ( invisible( work ) < 0 ){
		return;
	}

	SightsZbufInitialize( &work->Zbuf );
	NumberDisp( work );
	// ポーズ中かどうか
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return;
	}
	prelock = PL_LockonTarget ;
	for ( i = 0 ; i < MAX_SIGHTS ; i++ ){
		work->sights_color[ i ] = NULL;
	}
	if ( prelock != NULL && !( GM_WeaponAlive & WP_ALIVE_STINGER ) ) {
		if ( ++work->lockon_count > DIRECT_TICK(60) ) {	/* 時間切れ */
			SearchContena( work , PL_LockonTarget , &default_color );
			prelock = NULL;
			PL_LockonTarget = NULL;
			SEEKER_ON = 0;
			work->alive_flag = 0;
			work->lockon_count = 0;
		}
	}
#if 0 
	// ロックオンチェンジ
	if ( GV_PadData[ 0 ].press & PAD_X ){
		SearchContena( work , PL_LockonTarget , &default_color );
		ATHOR_LOCK = PL_LockonTarget;
		prelock = NULL ;
		PL_LockonTarget = NULL ;
		work->lockon_count = 0;
	}
	//test
	if ( GV_PadData[ 0 ].press & PAD_L1 ){
		GM_SetSightStatus( SGT_Invisible );
	}
	if ( GV_PadData[ 0 ].press & PAD_R1 ){
		GM_ResetSightStatus( SGT_Invisible );
	}
#endif
	LOCKONTRG = prelock;

	NEWLOCK = 0;
	n_targets = 0;
	tnext = GM_TrgDefListGetTop();
	for ( i = 0; i < MAX_SIGHTS; i ++ ){ // サイトの消去
		SPR_HIDE( work->sights[ i ] );
	}
	while( ( tnext != NULL ) && !( work->flag & SK_INVISIBLE ) ) {
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
/*
		if ( len != 0xffffff && SEEKER_ON == 1 && min_length > len && t != ATHOR_LOCK ){
			LOCKONTRG = t ;
			NEWLOCK = 1 ;
			*TARGET_POS = *DISP_POS;
			min_length = len;
		}
*/
		DisplaySight( work->sights[ n_targets ] , t );
		SearchContena( work , t , &default_color );
		n_targets++;
	}
	/*
	// 一匹もロックオンチェンジの時に敵がいなかった場合
	if ( ATHOR_LOCK != NULL && NEWLOCK != 1 ){
		min_length = CheckLockonAvailable( work , ATHOR_LOCK );
		LOCKONTRG = ATHOR_LOCK ;
		NEWLOCK = 1 ;
		*TARGET_POS = *DISP_POS;
	}
	*/
	// サウンド関係
	if ( !( GM_CheckGameStatus( STATE_CHAFF ) ) && !( work->flag & SK_INVISIBLE ) ){
		if ( work->Sound_work.frame < 0 ){
			if ( PL_LockonTarget != NULL )
         {
            if ( BP_IsPAL()==TRUE )
   				work->Sound_work.frame = 1.f;
            else 
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
			PL_LockonTarget = LOCKONTRG = work->sights_color[ work->Zbuf.position ];
			TARGET_POS->vx = work->Zbuf.pos.vx;
			TARGET_POS->vy = work->Zbuf.pos.vy;
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
//		SPR_HIDE( work->root[ 0 ] );
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
	NumberMove( work );
	MtBarMove( work );
#if 0
	if ( PL_LockonTarget != NULL ){
		printf( "<%d> %x \n", work->lockon_count, LOCKONTRG ) ;
	}
#endif
	if ( LOCKONTRG != NULL ){
		if ( LOCKONTRG->class & ( TARGET_DEAD | TARGET_SKIP ) ){
			// 色を戻す
			work->color.r = 160;
			work->color.g = 250;
			work->color.b = 90;
			work->color.a = 48;
			work->lockon_count = 0 ;
			work->alive_flag = 0;
			SearchContena( work , PL_LockonTarget , &default_color );
			PL_LockonTarget = NULL; // 解除
			LOCKONTRG = NULL ;
			InitAfterImage( work );
			SPR_HIDE( work->seeker[ 0 ] ) ; // 非表示
		}
	}
	if ( !( GM_WeaponAlive & WP_ALIVE_STINGER ) && ( work->alive_flag ) ){ // 爆発したということ
		SearchContena( work , PL_LockonTarget , &default_color );
		prelock = NULL;
		PL_LockonTarget = NULL;
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
#if 0
	// Chaffを使用している時はロックが出来無い
	if ( GM_CheckGameStatus( STATE_CHAFF ) ){
		// 色を戻す
		work->color.r = 160;
		work->color.g = 250;
		work->color.b = 90;
		work->color.a = 48;
		SEEKER_ON = 0;
		work->lockon_count = 0 ;
		SearchContena( work , PL_LockonTarget , &default_color );
		PL_LockonTarget = NULL; // 解除
		LOCKONTRG = NULL ;
		InitAfterImage( work );
		SPR_HIDE( work->seeker[ 0 ] ) ; // 非表示
	}
#endif
//  SightsZbufInitialize( &work->Zbuf );
	work->time++;
}


static	void	NormalAct( Work *pWork )
{
	LockControl( pWork );
}

static void visible_frame( Work *pWork ) // ずれていた奴を補正
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle_2d, STR_CURSOR_T );
	spr->empty.pos.y += 256;
	spr = L2D_GetObject( pWork->handle_2d, STR_CURSOR_B );
	spr->empty.pos.y += 256;
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
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	if ( pWork->action < pWork->action_num ){
		L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
		pWork->action++;
	}  else if ( L2D_ActionStatus( pWork->handle_2d ) == L2D_STAT_ACK ){
		visible_frame( pWork );
		pWork->act = (void *)NormalAct;
		return ( 0 );
	}
	return ( 0 );
}


static void Act( Work *pWork )
{
	// デモ等のサイトの表示非表示管理
	if ( SthingerSightVisibleInvisible( pWork->handle_2d , STR_ROOT , ( pWork->togle & SK_STHINGER_MODEL_ON ? 1 : 0 ) ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		if ( ( pWork->togle & SK_STHINGER_MODEL_ON ? 1 : 0 ) == 0 ){
			pWork->flag |= SK_INVISIBLE;
			// ロック解除
			if ( PL_LockonTarget != NULL ){
				SearchContena( pWork , PL_LockonTarget , &default_color );
				PL_LockonTarget = NULL;
				LOCKONTRG = NULL ;
				pWork->lockon_count = 0;
				pWork->alive_flag = 0;
				SEEKER_ON = 0;
				// 色を戻す
				pWork->color.r = 160;
				pWork->color.g = 250;
				pWork->color.b = 90;
				pWork->color.a = 48;
				ColorChenge_Seeker( pWork->seeker_sub , &pWork->color );
				// 残像を消す
				InitAfterImage( pWork );
			}
		}
	}
	if ( MsgDie( pWork ) < 0 ){
		return;
	}
	if ( GM_CheckSightStatus( SGT_Stinger ) ){
		invisible2( pWork );
	}
	pWork->act( pWork );
}


static	void	Die( Work *pWork )
{
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	//PL_LockonTarget = NULL;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle;
	int        strcode;
	int              i;

	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0 , 0 ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;

	// センターカーソルの取得
	work->center_m = L2D_GetObject( handle, STR_CURSOR_T ) ;
	if ( work->center_m == NULL ){
		SK_Err( "center_m\0" );
		return ( -1 );
	}
	// センターカーソル下の取得
	work->center_b = L2D_GetObject( handle, STR_CURSOR_B ) ;
	if ( work->center_b == NULL ){
		SK_Err( "center_b\0" );
		return -1 ;
	}
	// エイムの取得
	work->aimArea = L2D_GetObject( handle, STR_AIMAREA ) ;
	if ( work->aimArea == NULL ){
		SK_Err( "aimArea\0" );
		return -1 ;
	}
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
	// センターカーソルの初期化
	work->center_b_pos.vx = 0;
	work->center_b_pos.vy = 0;
	work->center_b_pos.vz = 0;
	work->center_b_pos.vw = 0;
	// 上バーの初期化
	work->mt_bar_work.speed = 0.f;
	work->mt_bar_work.size  = 29.f;
	work->mt_bar_work.frame = 0.f;
	work->center_m->empty.pos.x = 0;
	work->center_m->empty.pos.y = 0;
	work->center_b->empty.pos.x = 0;
	work->center_b->empty.pos.y = 0;
	// サウンド
	work->Sound_work.frame = 60;
	// dup
	work->dup_count.seeker = 0;
	work->dup_count.sights = 0;
	work->dup_count.mater_1 = 0;
	work->dup_count.mater_2 = 0;
	// etc
	work->time = 0;
	work->alive_flag = 0;
	PL_LockonTarget = NULL;
	ATHOR_LOCK = NULL;
	SightsZbufInitialize( &work->Zbuf );

	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	work->act = (void *)AnimetionAct;

	/* MENU_Printfキャラ生成 */
	if ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) { // demo mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 6 * 1024 ) , DG_DMAPACK_NORMAL | DG_DMAPACK_INVISIBLE1 | DG_DMAPACK_INVISIBLE2 |
														DG_DMAPACK_INVISIBLE3 | DG_DMAPACK_PRIVILEGE , DG_DMAPACK_PHASE_AFTER , 144 );
	} else { // normal mode
		work->menuprint_work_ptr = NewMenuPrintManager( ( 6 * 1024 ) , DG_DMAPACK_MENU, DG_DMAPACK_PHASE_NORMAL, 0 );
	}
	if ( work->menuprint_work_ptr == NULL ){
		return ( -1 );
	}
	GV_SetActorChild( work , work->menuprint_work_ptr );

	return ( 0 );
}

/*----------------------------------------------------------------*/

void	*NewStgSight( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	work->name = name;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void    *NewStgScnSight( int name )
{
	return NewStgSight( name );
}

#define		DEFAULT_SIGHT_SIZE (15) // 80
void NewStingerSightFlag( void )
{
	SK_LockOnSize = GCL_GetOptionValue( 'l' , 15 ); // default lockon size
}	
