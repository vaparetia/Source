//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	spotdraw.c
	影描画モデル設置

	1999/10/22 K.Takabe
	$Id: spotdraw.c,v 1.1.1.3 2002/11/19 11:51:16 Yoshizawa1 Exp $

*/
/*

	void *NewSpotDrawObject( int name, FMATRIX *world, int model );
	int		name ;		キャラクタ名
	FMATRIX	*world ;	付随マトリクスアドレス
	int		model ;		モデル名

	影投影モデルを指定したマトリクスに付随するように生成する。


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
	GV_ACT		actor ;
	FMATRIX		lights[2] ;
	DG_TEX_TRANS	tex_trans ;
	DG_OBJS		*objs ;
	int			name ;
	int			where ;
	int			group ;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_WaitMessage( work, 0 );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeueObjs( work->objs ) ;
    DG_FreeObjs( work->objs );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where, int init_type, FMATRIX *world, int model )
{
	DG_OBJS		*objs ;
	DG_DEF		*def ;
	int			model_id = 0 ;
	extern int DG_ActiveShadowFlag ;

	work->name = name ;
	work->where = where ;

	/* モデル読み込み */
	if ( init_type == 0 ){
		/* シナリオ起動時 */
		if ( GCL_GetOption( 'm' ) != NULL ) {
			model_id = GCL_GetNextInt() ;
		}
	} else {
		/* プログラム起動時 */
		model_id = model ;
	}

	/* 同じモデルを生成する */
	def = GV_GetCache( GV_CacheID( model_id, 'k' ) );
    work->objs = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_SHADOWWRITE, 0 );
	GM_GroupObjs( objs, where );
	DG_QueueObjs( objs );
	objs->shadow_id = name ;

	DG_ActiveShadowFlag = 2 ;

	/* 設置座標 */
	if ( init_type == 0 ){
		/* シナリオ起動時 */
		if ( GCL_GetOption( 'p' ) != NULL ) {
			objs->world.m[3][0] += (float)GCL_GetNextInt() ;
			objs->world.m[3][1] += (float)GCL_GetNextInt() ;
			objs->world.m[3][2] += (float)GCL_GetNextInt() ;
		}
	} else {
		/* プログラム起動時 */
		objs->root = world ;
	}

#if 0
	/* テクスチャパラメータの設定 */
	work->tex_trans = DG_GetTexture( 0 )->tex_trans ;
	work->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ;/* 加算半透明 */
	//work->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0 ) ;/*  */
	work->tex_trans.tex2.reg = SCE_GS_NOP ;
	work->tex_trans.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(2) / 64,
												BUFFER_WIDTH/64,SCE_GS_PSMCT32,8,8,1,0,0,0,0,0,0) ;
	work->tex_trans.clamp.data = SCE_GS_SET_CLAMP(2,2,1,254,1,254) ;

	{/* テクスチャをオブジェクトに設定 */
		DG_OBJ			*obj ;
		DG_OBJ_PACKET	*packet ;
		DG_MDL			*mdl ;

		obj = objs->objs ;
		for ( i = objs->n_models ; i > 0 ; obj++, i-- ){
			/* パケットの修正 */
			mdl = obj->model ;
			packet = obj->packets ;
			for ( j = mdl->n_packs ; j > 0 ; packet++, j-- ){
				packet->tex_ptr[0] = &work->tex_trans ;
			}
		}
	}
#endif

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewSpotDrawObjectSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where, 0, NULL, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

	/*
		プログラム起動インターフェイス
	*/
void *NewSpotDrawObject( int name, FMATRIX *world, int model )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, GM_CurrentMap, 1, world, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
