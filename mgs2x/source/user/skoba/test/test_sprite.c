//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	Ｃ４ランプエフェクト  main.c
	2001/03/23 S.Kobayashi
	$Id: test_sprite.c,v 1.1.1.3 2002/11/19 11:50:25 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "etc.h"

/* ---------------------------------------------------------------- */
/* 回転スプライトの最大頂点数は１６まで！！ */
#define N_PRIMS3	(1)
#define N_VERTS3	(1)

#define N_PRIMS_ICE	(1)
#define N_VERTS_ICE	(1)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define SPRITE_FLAG (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA)
#define ALPHA (128)
#define LAMP_TEX 4059425 // xlit01b_msk

typedef	struct _work {
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;

	FMATRIX   *pWorld;
	FVECTOR       pos;	
	FVECTOR     speed; 
	int		      map;
	int           num;
	float        time;
	int         count;
	int          size;
	void ( *act )(struct _work *);
} Work;

static void Act( Work *pWork )
{
	DG_SwitchBuffPrim2( pWork->prim );
	pWork->act( pWork );
}

static void NormalAct( Work *pWork )
{
	DG_PRIM2_UVRGBWH *pUvrgbwh;
	FVECTOR              *pos;
	int                 clock;

	clock    = pWork->prim->buffer_clock;
	pUvrgbwh = pWork->prim->uvrgb[ clock ];
	pos      = pWork->prim->pos[ clock ];

	if ( pWork->time <= 0.0f ){
		GV_DestroyActor( pWork );
	} else {
		pWork->time -= 1.0f;
	}
}

static void Die( Work *pWork )
{
	if ( pWork->prim != NULL ){
		GM_FreePrim2( pWork->prim );
	}
}

static int GetResources( Work *pWork , FVECTOR *pPos )
{
	DG_TEX	             *tex;
	DG_PRIM2	        *prim;
	DG_PRIM2_UVRGBWH *uvrgbwh;
	FVECTOR			     *pos;
	int				 	 i, k;
	
	/* テクスチャ取得 */
	tex = DG_GetTexture( LAMP_TEX );
	prim = pWork->prim = GM_MakePrim2( SPRITE_FLAG , N_PRIMS3 , N_VERTS3 );
	if ( prim == NULL ){
		return ( -1 );
	}
	GM_GroupPrim2( pWork->prim, GM_CurrentStageMap ) ;
	DG_ConfigPrim2Tex( prim, tex );
	prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ;
	pWork->act = (void *)NormalAct;
	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	for ( i = 0 ; i < N_PRIMS3 ; i++ ){
		for ( k = 0 ; k < N_VERTS3 ; k++ ){
			DG_COPY_VEC( pos , pPos );
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
			uvrgbwh->w = pWork->size;
			uvrgbwh->h = pWork->size;
			uvrgbwh->r = 128;
			uvrgbwh->g = 128;
			uvrgbwh->b = 128;
			uvrgbwh->a = ALPHA;
			pos++;
			uvrgbwh++;
		}
	}
	_MemCopy( prim->pos[ 0 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->pos[ 1 ],   MEM_ADDR1, sizeof(FVECTOR),          N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;

	pWork->prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	pWork->time = 1.0f;

	return ( 0 );
}

void *NewTest_Sprite( FVECTOR *pPos , int size ) // 場所をたしかめるのに便利かも
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( pWork != NULL ) {
			GV_SetActor( &( pWork->actor ), Act, Die ) ;
			GV_ActorEX( &pWork->actor );
			pWork->size = size; 
			if ( GetResources( pWork , pPos ) < 0 ) {
				GV_DestroyActor( &pWork ) ;
				return NULL ;
			}
	}
	return (void *)pWork ;
}

