/*
	anmtex.c
	アニメーションテクスチャ展開実験プログラム

	2001/06/02 K.Takabe
	$Id: anmtex.c,v 1.1.1.3 2002/11/19 11:51:28 Yoshizawa1 Exp $

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

#include	"utl_anmtex.h"




/* ------------------------------------------------------------ */
/* プログラムワーク */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			tri_id ;				/* ＴＲＩファイルＩＤ */
	int			tex_id ;				/* テクスチャＩＤ */
	int			anmtex_id ;				/* アニメテクスチャデータＩＤ */

	/* アニメテクスチャ展開ワーク */
	ANMTEX_WORK	anmtex_work ;			/* アニメテクスチャ再生ワーク */
} Work ;


/* ---------------------------------------------------------------- */
/* アクターメイン処理 */
static void Act( Work *work )
{
	/* アニメテクスチャ展開（１回の呼び出しで１フレーム進む） */
	UTL_ActAnmtex( &work->anmtex_work );
}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( Work *work )
{
	/* アニメテクスチャ展開ワークの開放 */
	UTL_FreeAnmtexWork( &work->anmtex_work );
}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( Work *work, int name, int where )
{
	DG_TEX	*tex ;
	int		size ;

	/* シナリオ起動時のオプションを解析 */
	if ( GCL_GetOption( 't' ) != NULL ){
		work->tri_id = GCL_GetNextInt();
		work->tex_id = GCL_GetNextInt();
	}
	if ( GCL_GetOption( 'a' ) != NULL ){
		work->anmtex_id = GCL_GetNextInt();
	}

	/* アニメテクスチャ展開ワーク初期化 */
	if ( UTL_InitAnmtexWork( &work->anmtex_work, work->tri_id, work->tex_id, work->anmtex_id, ANMTEX_FLAG_LOOP ) < 0 ){
		/* 初期化失敗 */
		return ( -1 );
	}

	return (0);
}

/* シナリオ起動関数 */
void *NewTestAnmtexSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;	/* 旧形式 */
	//work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );	/* 新形式 */
	work = GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(Work), 0 );	/* 新形式 */
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
