/*
	tst_obj.c
	ノーマルオブジェクト表示実験プログラム
	（シナリオ起動からモデルの表示まで）

	2001/05/07 K.Takabe
	$Id: tst_obj.c,v 1.1.1.3 2002/11/19 11:51:31 Yoshizawa1 Exp $

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

#define CTRL_FLAG	(CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS)/* source/game/g_struct.hを参照 */
#define BODY_FLAG	(DG_FLAG_ONEPIECE)	/* source/system/libdg/libdg.hを参照 */


/* プログラムワーク */
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;		/* コントロール */
	OBJECT		object ;		/* オブジェクト */
	FMATRIX		light[2] ;		/* ライトマトリクス設定用 */
} Work ;



/* ---------------------------------------------------------------- */
/* アクターメイン処理 */
static void Act( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;

	ctrl = &work->control ;
	body = &work->object ;

#if 1
	/* モーションデータがない場合 */
	GM_ActControl( ctrl );
	GM_ActObject( body );
#else
	/* モーションデータがある場合 */
	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActObject( ctrl );
	GM_ActObject2( body );
#endif

	/* ライトマトリクスの取得 */
	DG_GetLightMatrix( &ctrl->mov, work->light );

}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( Work *work )
{
	/* 起動時に確保したものを開放する */
	GM_FreeObject( &work->object );
	GM_FreeControl( &work->control );
}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			model_id = 0 ;

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

	ctrl = &work->control ;
	body = &work->object ;
	/* コントロール初期化 */
	GM_InitControlEx( ctrl, name, where, CTRL_FLAG );	/* コントロール初期化 */
	GM_ConfigControlPosition( ctrl, &pos, &rot );		/* 位置初期化 */
	GM_ConfigControlObject( ctrl, body );				/* オブジェクトとの関連付け */
	//GM_ConfigControlHazard( ctrl, 100, 100, 100 );	/* 当たり関連初期化（今は仮！！） */
	//GM_ConfigControlHzxHeight( ctrl, 100, 100 );		/* 当たり判定逆さ初期化（今は仮！！） */
	//GM_ConfigControlHzxCheckFlag( ctrl, 0, 0 );			/* （今は仮！！） */
	/* オブジェクト初期化 */
	GM_InitObject( body, model_id, BODY_FLAG );			/* オブジェクト初期化 */
	GM_ConfigObjectLight( body, work->light );			/* ライトマトリクス設定 */
	//GM_ConfigObjectStep( body, &ctrl->step );			/* モーション反映用にステップ変数設定 */
	//GM_ConfigObjectMotion( body, 2, motion_id, MT_FLAG_PDRAY );		/* モーション割り当て */
	//GM_ConfigObjectAction( body, 0, 0, 0, ~0, MOTION_INTERP_TIME );

	GM_ActMotion( body );
	GM_ActControl( ctrl );
	GM_ActObject2( body );

	return (0);
}

/* シナリオ起動関数 */
void *NewTestObjectSet( int name, int where )
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
