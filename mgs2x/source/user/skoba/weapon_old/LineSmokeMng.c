//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	LineSmokeMng.c
	ラインスモークマネージャー 
	2001/03/30 S.Kobayashi
	$Id: LineSmokeMng.c,v 1.1.1.3 2002/11/19 11:50:32 Yoshizawa1 Exp $

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
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include    "../test/etc.h"
#include	"../../okajima/etc/ok_util.h"

#define	N_PRIMS		(32)
#define	N_VERTS		(10)
#define	N_POLYS		(N_VERTS/2)

#define	COL_R		(255)
#define	COL_G		(128)
#define	COL_B		(32)
#define	ALP_MIN		(16.0f)
#define	ALP_RND		(32.0f)
#define	SUB_ALPHA	(0.2f)

#define	INITIAL_LENGTH	(1000.0f)

#define	VEC_MAX		(60.0f)
#define	VEC_RND		(10.0f)
#define	DECAY_RATIO	(0.95f)
#define PRIM_FLAG ( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA )

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;

	FVECTOR		pos[N_VERTS];

	FMATRIX		world;
	FVECTOR     shift;
	int         map;
	int			life;
} Work ;

extern void	*NewLineSmoke( FMATRIX * , FVECTOR * , int , float , float );

static	void Act( Work *pWork )
{
	void *work;
	FVECTOR ftmp;
	float size;
	int life;
	float speed;

	size = rnd() * 20.0f + 1.0f;
	life = (int)(rnd() * 30.0f + 1.0f);
	speed = rnd() * 4.0f + 1.0f;

	ftmp = pWork->shift;
	ftmp.vx += rnd() * 25 - 12.5f;
	ftmp.vz += rnd() * 30 - 15.f;

	work = NewLineSmoke( &pWork->world, &ftmp , life , speed , size );
	//	work = NewLineSmoke( &GM_PlayerBody->objs->objs[12].world, &ftmp , 80 , 4.0f , 10.f );
	if ( work == NULL || pWork->life < 0 ){
		GV_DestroyActor( pWork );
	} else {
		pWork->life--;
	}
}

static	void Die( Work *pWork )
{
	pWork->prim = OK_FreePrim2( pWork->prim );
}

static	int GetResources( Work *pWork )
{
  /*
	DG_TEX		*tex ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	float omega;
	float a;
	float tmp_pos;

	FVECTOR			*pos ;
	int     kick[ 3 ] = { 0x8fff , 0x8fff , 0x0fff };
	int		i, j, k ;

	// テクスチャ取得
	tex = DG_GetTexture( GV_StrCode("bomb1_fl") );
	prim = pWork->prim = GM_MakePrim2( DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	DG_ConfigPrim2Tex( prim, tex );

	// スクラッチパッド上で初期化したデータを実際のワークにコピーする
	pos = MEM_ADDR1;
	uvrgb = MEM_ADDR2;
	j = 0;
	a = 1;
	omega = 0;
	tmp_pos = 0;
	DG_SetPos( &pWork->world ); 
	for ( i = 0 ; i < N_VERTS ; i++ ){
		omega += 0.1;
		pWork->pos[ i ].vx = a * cosf( (float)M_PI * ( omega ) /180.0f );
		tmp_pos = pWork->pos[ i ].vy = tmp_pos + rnd() * 100;
		pWork->pos[ i ].vz = a * sinf( (float)M_PI * ( omega ) /180.0f ) ;
		DG_PutVector( &pWork->pos[ i ] , pos , 1 );
		// ＵＶ値は1.3.12の固定小数点なので注意！
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = kick[ j ];
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		if ( j < 3 ){
			j++;
		}
		pos++;
		uvrgb++;
	}
	_MemCopy( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS );
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS );
	_MemCopy( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS );
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS );
	*/
	return ( 0 );
}

void *NewLineSmokeMng( FMATRIX *pWorld , FVECTOR *pShift , int life )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->world = *pWorld;
		pWork->shift = *pShift;
		pWork->life = life;
		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}
