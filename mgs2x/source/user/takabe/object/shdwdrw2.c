//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shdwdrw2.c
	影描画モデル設置２

	1999/10/22 K.Takabe
	$Id: shdwdrw2.c,v 1.1.1.3 2002/11/19 11:51:16 Yoshizawa1 Exp $

*/
/*

chara 影投影モデル２[NewShadowDrawObject2Set] $s:名前 \
	-ctrl $s:影投影を重ねるコントロール名
// 影投影用モデルを指定したコントロールの保持するオブジェクトに重ねて設置する
// 「スポットライト投影モデル」キャラとは排他である


*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"



/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	int			name ;
	int			map ;
	int			group ;

	int			ctrl_name ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
} Work ;

/* ---------------------------------------------------------------- */
/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( Work *work, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & work->map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("%s: search faild control !!\n", __FILE__);
	return ( NULL );
}
/* ---------------------------------------------------------------- */
void CheckObject( Work *work )
{
	CONTROL		*ctrl ;

	ctrl = SearchControl( work, work->ctrl_name );
	if ( ctrl == NULL ){
		/* 開放処理 */
		if ( work->objs != NULL ){
			DG_DequeueObjs( work->objs );
			DG_FreeObjs( work->objs );
			work->objs = NULL ;
		}
		work->ctrl = NULL ;
		work->body = NULL ;
		return ;
	}
	if ( ctrl->object == NULL ) return ;
	if ( ctrl->object->objs == NULL ) return ;

	/* 初期化チェック */
	if ( work->ctrl == NULL ){
		work->ctrl = ctrl ;
		work->body = ctrl->object ;
		{/* モデルの生成処理 */
			DG_DEF		*def ;
			DG_OBJS		*objs ;
			int			flag ;

			def = work->body->objs->def ;
			flag = work->body->objs->flag ;

			flag &= ~DG_FLAG_INVISIBLE ;
			flag &= ~DG_FLAG_PAINT ;
			flag |= DG_FLAG_SHADOWWRITE ;
			work->objs = objs = DG_MakeObjs( def, flag, 0 );
			if ( objs == NULL ) return ;
			GM_GroupObjs( objs, work->map ) ;
			DG_QueueObjs( objs );
			objs->shadow_id = work->name ;
		}
	}

	/* オブジェクトマトリクスの設定 */
	if ( work->objs != NULL ){
		int		i ;
		DG_OBJS		*org_objs ;
		DG_OBJS		*objs ;

		objs = work->objs ;
		org_objs = work->body->objs ;

		objs->world = org_objs->world ;
		objs->flag &= ~DG_FLAG_INVISIBLE ;
		objs->flag |= org_objs->flag & DG_FLAG_INVISIBLE ;

		for ( i = 0 ; i < objs->n_models ; i++ ){
			objs->objs[i].world = org_objs->objs[i].world ;
		}
	}
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	CheckObject( work );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	extern int DG_ActiveShadowFlag ;

	work->name = name ;
	work->map = where ;

	/* モデル読み込み */
	/* シナリオ起動時 */
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->ctrl_name = GCL_GetNextInt() ;
	} else {
		return ( -1 );
	}

	DG_ActiveShadowFlag = 1 ;

	CheckObject( work );

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewShadowDrawObject2Set( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		GV_SetActorMessageKill( work, name );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

