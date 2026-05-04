//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fit_doll.c
	アクションコピー人形

	1999/11/10 K.Takabe
	$Id: fit_doll.c,v 1.1.1.3 2002/11/19 11:51:10 Yoshizawa1 Exp $

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
	int			model_id ;
	DG_TEX		*tex ;
	DG_OBJS		*org_objs ;
	DG_OBJS		*objs ;
	int			flag ;
	void		*envmap_work ;
} Work ;


/* ---------------------------------------------------------------- */
enum {
	FLAG_LIGHT_COPY =		0x0001,
	FLAG_MODEL_TRANS =		0x0002,
	FLAG_MODEL_ALPHA =		0x0004,
	FLAG_ORG_INVISIBLE =	0x0010,
	FLAG_ORG_DRAWLAST =		0x0020,
	FLAG_ENVIRONMENT_MAP =	0x0100,
	FLAG_SCROLLANIME_MAP =	0x0200,
	FLAG_CAMOUFLAGE_MAP =	0x0400,
};

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i, flag, num ;
	/* オリジナルと同じポーズを取る */
	work->objs->world = work->org_objs->world ;
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		work->objs->objs[ i ].world = work->org_objs->objs[ i ].world ;
	}

	/* 可視・不可視をオリジナルに合わせる */
	work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	if ( ( flag = work->org_objs->flag & DG_FLAG_INVISIBLE ) ){
		work->objs->flag |= flag ;
	}
#if 1
	/* 各関節の可視・不可視もチェックし、オリジナルに合わせる */
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		work->objs->objs[ i ].flag &= ~DG_FLAG_INVISIBLE ;
		/* 参照オブジェクト番号のチェック */
		if ( work->objs->objs[ i ].model->type & DG_TYPE_EXTEND ){
			/* 拡張モデルであった場合 */
			num = work->objs->objs[ i ].model->parent ;
		} else {
			/* 通常の場合 */
			num = i ;
		}
		if ( ( flag = work->org_objs->objs[ num ].flag & DG_FLAG_INVISIBLE ) ){
			work->objs->objs[ i ].flag |= flag ;
		}
		
	}
#endif
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i ;

	if ( work->envmap_work ) GV_DestroyActor( work->envmap_work );
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );
	if ( work->flag & FLAG_ORG_INVISIBLE ){
		for ( i = 0 ; i < work->org_objs->n_models ; i++ ){
			work->org_objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
		}
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *org_objs, int model_id, int flag, int param )
{
	DG_DEF		*def ;
	DG_OBJS		*objs ;

	work->model_id = model_id ;
	work->org_objs = org_objs ;
	work->flag = flag ;

	def = GV_GetCache( GV_CacheID( model_id, 'k' ) ) ;
	work->objs = objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 );
	DG_QueueObjs( objs );
	DG_SetLightMatrix( objs, work->lights );

	/* ノーシェードに設定 */
	work->lights[1].m[3][0] = 128.0f ;
	work->lights[1].m[3][1] = 128.0f ;
	work->lights[1].m[3][2] = 128.0f ;

	printf("fit action doll init...\n");
	if ( flag & FLAG_LIGHT_COPY ){/* ライト複製フラグ */
		printf("fit_doll: light copy mode\n");
		DG_SetLightMatrix( objs, org_objs->light );
	}

	if ( flag & FLAG_MODEL_TRANS ){/* モデルの半透明化 */
		if ( flag & FLAG_MODEL_ALPHA ) ;/* これは予めモデルにテクスチャをはってもらうことで解決する */
	}

	if ( flag & FLAG_ENVIRONMENT_MAP ){/* 環境マッピングフラグ */
		extern void *NewEnvmap( DG_OBJS *objs, int model_id, int tex_id );
		printf("fit_doll: environment mapping on\n");
		ASSERT( work->envmap_work == NULL );
		work->envmap_work = NewEnvmap( objs, model_id, param );
	}

	if ( flag & FLAG_SCROLLANIME_MAP ){/* スクロールアニメテクスチャマッピング */
		extern void *NewScrollAnimeMapping( DG_OBJS *objs, int model_id, int tex_id );
		printf("fit_doll: scroll anime mapping on\n");
		ASSERT( work->envmap_work == NULL );
		work->envmap_work = NewScrollAnimeMapping( objs, model_id, param );
	}

	if ( flag & FLAG_CAMOUFLAGE_MAP ){/* 迷彩マッピング */
		extern void *NewCamouflageMapping( DG_OBJS *objs, int model_id, int param );
		printf("fit_doll: scroll anime mapping on\n");
		ASSERT( work->envmap_work == NULL );
		work->envmap_work = NewCamouflageMapping( objs, model_id, param );
	}

	if ( flag & FLAG_ORG_INVISIBLE ){
		int		i ;
		for ( i = 0 ; i < org_objs->n_models ; i++ ){
			org_objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
		}
	}

	if ( flag & FLAG_ORG_DRAWLAST ){
		DG_DequeueObjs( org_objs );
		DG_QueueObjs( org_objs );
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewFitActionDoll( DG_OBJS *objs, int model_id, int flag, int param )
{
	Work		*work ;

	return NULL ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, objs, model_id, flag, param ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewFitActionDoll_Video( DG_OBJS *objs, int model_id, int flag, int param, int visible_num )
{
	return NewFitActionDoll( objs, model_id, flag, param );
}


#if 0
	/*
		使用例
	*/

#if 0 /* 環境マッピング関連実験 */
#if 1
	/*  */
	switch ( 2 ){
	  case 0:
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test01" ),
						 0x0002, 0  );
		//NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test01" ),
		//				   0x0103, GV_StrCode("envmap_alp_ovl_mod1120") );
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test01" ),
						 0x0203, GV_StrCode("tnr_stes03_ovl_mod1120_alp") );
		break ;
	  case 1:
		NewFitActionDoll( work->body.objs, GV_StrCode( "crg_spe_test" ),
						 0x0002, 0  );
		NewFitActionDoll( work->body.objs, GV_StrCode( "crg_spe_test" ),
						 0x0103, GV_StrCode("crg_stes00_ovl_mod1120_alp") );
		break ;
	  case 2:
		NewFitActionDoll( work->body.objs, GV_StrCode( "crg_spe_test" ),
						   0x0103, GV_StrCode("envmap_alp_ovl_mod1120") );
		break ;
	}
#else
	/* 光学迷彩実験 */
	switch (0){
	  case 0:
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test00" ),
						 0x0212, GV_StrCode("scroll_alp_ovl_mod1011") );
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test00" ),
						 0x0402, SCE_GS_SET_ALPHA(0,1,1,1,0) );
		break ;
	  case 1:
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test00" ),
						 0x0212, GV_StrCode("scroll_alp_ovl_mod1011") );
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_def" ),
						 0x0007, SCE_GS_SET_ALPHA(0,1,1,1,0) );
		break ;
	  case 2:
		NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test00" ),
						 0x0410, 0 );
		break ;
	  case 3:
		NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
						 0x0006, SCE_GS_SET_ALPHA(1,1,2,1,128) );
		NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
						 0x0212, GV_StrCode("scroll_alp_ovl_mod1011") );
		NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
						 0x0007, SCE_GS_SET_ALPHA(0,1,0,1,0) );
		break ;
	  case 4:
#if 1
	NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
					 0x0006, SCE_GS_SET_ALPHA(1,1,0,1,0) );
#endif
	NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
					 0x0210, GV_StrCode("scroll_alp_ovl_mod1011") );
	NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ),
					 0x0007, SCE_GS_SET_ALPHA(0,1,1,1,0) );
		break ;
	}
	//NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ), 0x0007, GV_StrCode("envmap_alp_ovl_mod1120") );
	//NewFitActionDoll( work->body.objs, GV_StrCode( "sna_def" ), 0x000b, GV_StrCode("tnr_stes02_ovl_mod1120_alp") );
	//NewFitActionDoll( work->body.objs, GV_StrCode( "tnr_spe_test00" ), 0x0007, GV_StrCode("envmap_alp_ovl_mod1120") );
#endif
#endif



#endif
