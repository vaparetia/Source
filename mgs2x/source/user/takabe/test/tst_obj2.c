/*
	tst_obj2.c
	ノーマルオブジェクト表示実験プログラム（DG_OBJS直接操作）
	（シナリオ起動からモデルの表示まで）

	2001/05/07 K.Takabe
	$Id: tst_obj2.c,v 1.1.1.3 2002/11/19 11:51:32 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdarg.h>

#include	"gameheader.h"

#define MODEL_FLAG	(DG_FLAG_ONEPIECE)	/* source/system/libdg/libdg.hを参照 */


/* プログラムワーク */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			map ;			/* キャラの存在するマップ */
	int			name ;			/* キャラ名 */
	DG_OBJS		*objs ;			/* モデルオブジェクト */
	FVECTOR		pos ;			/* 表示位置 */
	SVECTOR		rot ;			/* 回転量 */
	FMATRIX		light[2] ;		/* ライトマトリクス設定用 */
} Work ;



/* ---------------------------------------------------------------- */
/* アクターメイン処理 */
static void Act( Work *work )
{
	/* 処理マップを設定 */
	GM_CurrentMap = work->map ;	/* このプログラムではあまり意味はない */

	/* モデル表示用マトリクスの生成 */
	DG_SetPos2( &work->pos, &work->rot );
	/* モデルにマトリクスを設定 */
	DG_PutObjs( work->objs );
	/* モデルに表示マップを設定 */
	GM_GroupObjs( work->objs, work->map );

	/* ライトマトリクスの取得 */
	DG_GetLightMatrix( &work->pos, work->light );

}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( Work *work )
{
	/* 起動時に確保したものを開放する */
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );
}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( Work *work, int name, int where )
{
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			model_id = 0 ;
	DG_OBJS		*objs ;
	DG_DEF		*def ;

	work->map = where ;
	work->name = name ;

	/* シナリオ起動時のオプションを解析 */
	/* 初期位置取得 */
	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = GCL_GetNextInt();
		pos.vy = GCL_GetNextInt();
		pos.vz = GCL_GetNextInt();
	}
	/* 初期角度取得 */
	if ( GCL_GetOption( 'd' ) != NULL ){
		rot.vx = GCL_GetNextInt();
		rot.vy = GCL_GetNextInt();
		rot.vz = GCL_GetNextInt();
	}
	/* モデルＩＤ取得 */
	if ( GCL_GetOption( 'm' ) != NULL ){
		model_id = GCL_GetNextInt();
	}

	work->pos = pos ;
	work->rot = rot ;

	/* モデルの初期化 */
	def = GV_GetCache( GV_CacheID( model_id, 'k' ) );
	if ( def == NULL ){
		printf("%s:model not found(%d)\n", __FILE__, model_id );
		return ( -1 );
	}
	work->objs = objs = DG_MakeObjs( def, MODEL_FLAG, 0 );
	DG_QueueObjs( objs );
	/* モデルに表示マップを設定 */
	GM_GroupObjs( objs, work->map );

	return (0);
}

/* シナリオ起動関数 */
void *NewTestObject2Set( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;	/* 旧形式 */
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );	/* 新形式 */
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
