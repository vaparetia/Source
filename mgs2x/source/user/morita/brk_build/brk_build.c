//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_swing.c
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_build.c,v 1.1.1.3 2002/11/19 11:45:25 Yoshizawa1 Exp $


   窓は,「割れていない」＝「表示しない」に同値なので最初の状態ではモデ
   ルは表示されていない。

*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#define MY_DEBUG 0

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

#define SCARSIZE 2000.0f

/*  各々窓のパラメータ */
typedef struct window_t
{
    FVECTOR   pos  ;
    DG_OBJS  *objs ;/* 現在の,窓のモデル */
    DG_DEF   *def  ;/* 壊れモデル */
} WINDOW ;

/* ビルの情報 */
typedef struct work_t
{
    GV_ACT       actor  ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR   lv ;
    FVECTOR   uv ;

    WINDOW   *win    ;
    int       n_win  ;

    DG_DEF   *def   ;
    LIT_DEF  *lit   ;

    int       time ;

    int       name  ;
    int       where ;
} Work ;

extern void AN_Test_Eye2( FVECTOR *mov, int i ) ; /* for test */
extern void *NewBuildGlassPrim( FVECTOR *pos, DG_DEF *def,
				int n_piece, int tex_id ) ;

/* 窓を壊す処理 */
static void BreakWindow( Work *work, WINDOW *w )
{
    /* 壊れモデルを確保して表示 */
    if ( !(w->objs = DG_MakeObjs( w->def,
				  DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	printf( "Cannot DG_OBJS(Maybe no memory) : NewBuildingWindow\n" ) ;
    else
    {
	/* キュー設定とグループ設定 */
	DG_QueueObjs( w->objs ) ;
	GM_GroupObjs( w->objs, work->where ) ;

	/* Z反転したモデルにする。 */
	if ( irnd() & 1 )
	    w->objs->world.m[Z][Z] = -1.0f ;
	/* Y反転したモデルにする。 */
	if ( irnd() & 2 )
	    w->objs->world.m[Y][Y] = -1.0f ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)w->objs->world.m[W], &w->pos ) ;

	/* マトリックスができたのでプリシェードを掛ける */
	DG_MakePreshade( w->objs, work->lit ) ;

	/* ガラス破片の飛び散りキャラを発動させる */
	NewBuildGlassPrim( &w->pos, w->def, 80, 8291175 ) ;

	/* SEを鳴らす */
	GM_SeSetMode( SD_A_GLASS01, &w->pos, GM_SEMODE_BOMB ) ;
    }
}

/* ビルのコールバック。当たった場所から窓の位置を全て検索しどの窓が壊
   れているかを調べる。 */
void BRK_BLD_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work   *work = (Work *)ptr ;
    int     i ;
    WINDOW *w ;
    FVECTOR v ;
    float   size ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	/* 2度来ない*/
	if ( work->time == GM_StagePlayTime )
	    return ;
	work->time = GM_StagePlayTime ;

	/* 影響範囲の変更 */
	if ( off->weapon_type & WP_BLAST )
	    size = SCARSIZE ;
	else
	    size = 0.0f ;

	/* 全ての窓を検索 */
	w = work->win ;
	for ( i=work->n_win ; --i>=0 ; w++ )
	    if ( !w->objs )
	    {
		/* 窓のローカルオフセット位置にする。 */
		_sceVu0SubVector( &v, &off->center, &w->pos ) ;

		/* 窓にあったっているかどうか をモデルのバウンディング
                   から決めている。 */
		if ( v.vz >= w->def->lz - size &&
		     v.vz <= w->def->uz + size &&
		     v.vy >= w->def->ly - size &&
		     v.vy <= w->def->uy + size  )
		    BreakWindow( work, w ) ;
	    }
    }
}

#if MY_DEBUG
static void Act( Work *work )
{
    WINDOW *w = work->win ;
    int i ;

    if ( GV_PadData[1].press & PAD_L1 )
	for ( i=work->n_win ; --i>=0 ; w++ )
	    if ( !w->objs )
	    {
		BreakWindow( work, w ) ;
		break ;
	    }
}
#endif


static void Die( Work *work )
{
    WINDOW *w ;
    int     i ;

    if ( (w = work->win) )
    {
	for ( i=work->n_win ; --i>=0 ; w++ )
	    if ( w->objs )
		DG_DequeueObjs( w->objs ), DG_FreeObjs( w->objs ) ;
	GV_Free( work->win ) ;
    }
#if 0
    if ( work->scar )
	GM_FreePrim2( work->scar ) ;
#endif
    GM_FreeTarget( &work->target ) ;
}

/*

  窓一つ一つのターゲット

  p0,p1の2点を指定することでXYのサイズを算出して,Y回転量も計算する。Z
  サイズは,１にしている。

*/
int BRK_BLD_InitTarget( Work *s, int where, FVECTOR *p0, FVECTOR *p1 )
{
    TARGET       *t = &s->target ;
    POWER_TARGET *p = &s->power  ;
    FVECTOR       t_size, t_pos ;
    FMATRIX       world ;

    /* 中心位置とサイズを計算 */
    _sceVu0SubVector( &t_size, p0, p1 ) ;
    _sceVu0AddVector( &t_pos , p0, p1 ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    /* ZとXのサイズは,回転量として計算する。つまりY回転量が計算される。 */
    _sceVu0RotMatrixY( &world, &DG_UnitMatrix, -atan2f(t_size.vz,t_size.vx) ) ;
    /* Wを壊さないように位置をコピーする。 */
    _sceVu0CopyVectorXYZ( (FVECTOR *)world.m[W], &t_pos ) ;
    /* サイズを計算し直す XZの距離をXのサイズとすることでY回転した板ペ
       ラとなる。 */
    t_size.vx = sceVu0Sqrt( t_size.vx*t_size.vx + t_size.vz*t_size.vz ) ;
    t_size.vy = fpu_Abs( t_size.vy ) ;
    t_size.vz = 1.0f ;

    /* ターゲット設定 */
    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_ROTATE,
		  where, BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE,
		       GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_BLD_TargetCallBack, s ) ;
    /* ターゲット登録 */
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &world, where ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR  buf ;
    int      i   ;
    WINDOW  *w   ;
    DG_DEF  *def[3] ;
    int      n_def ;

    work->where  = where ;

    /* 何枚あるのか数える */
    work->n_win = 0 ;
    while( (i = GCL_GetNextOption()) )
	if ( i == 'p' )
	    work->n_win++ ;
    /* 窓の枚数が分かったら,その分だけメモリを確保する。 */
    if ( work->n_win )
    {
	if ( !(work->win = GV_Malloc( work->n_win * sizeof(WINDOW) )) )
	    PERROR( "No memory :: NewBuildingWindow\n" ) ;
	/* 確保したメモリを０クリアする。 */
	GV_ZeroMemory( work->win, work->n_win * sizeof(WINDOW) ) ;
    }

    /* ターゲットの位置 */
    if ( !GCL_GetOption( 't' ) )
	PERROR( "No target Max&Min position specified : NewBuildingWindow\n" ) ;
    GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
    vu0_IV0toFV( &buf, &work->uv ) ;
    GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
    vu0_IV0toFV( &buf, &work->lv ) ;

    /* ターゲットを設定する。ビル一つ単位で設定する。 */
    BRK_BLD_InitTarget( work, where, &work->lv, &work->uv ) ;

    /* 原点の位置 */
    if ( work->n_win )
    {
	/* モデルの指定は,複数指定することができ,それぞれが壊れ窓にす
           ることができる */
	n_def = 0 ;
	if ( GCL_GetOption( 'm' ) )
	    while( GCL_NextStr() )
		def[n_def++] = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) ) ;

	/* 窓の位置を設定 */
	w = work->win ;
	while( (i = GCL_GetNextOption()) )
	    if ( i == 'p' )
	    {
		w->def = def[i%n_def] ;
		GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
		vu0_IV0toFV( &buf, &w->pos ) ;
		w++ ;
	    }
    }

    /* ライトデータを取得 */
    work->lit = GM_GetMap( where )->light ;

    return 0 ;
}


/*



*/
void *NewBuildingWindow( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
#if MY_DEBUG
        GV_SetActor( &work->actor, Act, Die ) ;
#else
        GV_SetActor( &work->actor, NULL, Die ) ;
#endif
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
