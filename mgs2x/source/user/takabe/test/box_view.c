//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	box_view.c
	バウンディング表示デバッグキャラ

	1999/11/02 K.Takabe
	$Id: box_view.c,v 1.1.1.3 2002/11/19 11:51:28 Yoshizawa1 Exp $

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

#include	"def_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define N_PRIMS	(20)
#define N_PACKETS	(1)

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/

typedef	struct	{
	GV_ACT		actor ;

	DG_PRIM2	*prim ;
	FMATRIX		world ;
	FVECTOR		pos[12*2] ;

	int			count ;

} Work ;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

#if 0
/* パケットの初期化 */
static void InitPacket( DG_PRIM_PACKET *packet, int n, int which, int color )
{
	DG_LINE_R2	*line ;
	int					data_size, i ;

	line = (DG_LINE_R2*)packet->prim_top ;
	/* 半透明属性付加 */
	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	/* プリミティブデータ部の初期化処理 */
	for ( i = n ; i > 0 ; i-- ){
		DG_SET_RGBA1( line, color & 255, ( color >> 8 ) & 255, ( color >> 16 ) & 255, 64 );
		line++ ;
	}
}
#endif

/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	GV_DestroyActor( work );
}

static void Die( Work *work )
{
	if ( work->prim != NULL ){
		DG_DequeuePrim2( work->prim );
		DG_FreePrim2( work->prim );
	}
}

static int GetResources_1( Work *work, FMATRIX *world, float *max, float *min, int color )
{
#if 0
	DG_PRIM	*prim ;
	DG_PRIM_PACKET *packet ;
	FVECTOR			vec[8] ;
	int		i, j ;

	prim = work->prim = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_LINE_R2, 1, 12, work->pos, NULL );
	if ( prim == NULL ) return ( -1 );
	prim->world = *world ;

	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < prim->n_packet ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[j] + prim->packet_size * i );
			InitPacket( packet, 12, j, color );
		}
	}

	for ( i = 0 ; i < 8 ; i++ ){
		vec[ i ].vx = ( i & 1 ) ? max[0] : min[0] ;
		vec[ i ].vy = ( i & 2 ) ? max[1] : min[1] ;
		vec[ i ].vz = ( i & 4 ) ? max[2] : min[2] ;
	}
	work->pos[ ( 0 ) * 2 + 0 ] = vec[0] ;
	work->pos[ ( 0 ) * 2 + 1 ] = vec[1] ;
	work->pos[ ( 1 ) * 2 + 0 ] = vec[1] ;
	work->pos[ ( 1 ) * 2 + 1 ] = vec[3] ;
	work->pos[ ( 2 ) * 2 + 0 ] = vec[3] ;
	work->pos[ ( 2 ) * 2 + 1 ] = vec[2] ;
	work->pos[ ( 3 ) * 2 + 0 ] = vec[2] ;
	work->pos[ ( 3 ) * 2 + 1 ] = vec[0] ;
	work->pos[ ( 4 ) * 2 + 0 ] = vec[0+4] ;
	work->pos[ ( 4 ) * 2 + 1 ] = vec[1+4] ;
	work->pos[ ( 5 ) * 2 + 0 ] = vec[1+4] ;
	work->pos[ ( 5 ) * 2 + 1 ] = vec[3+4] ;
	work->pos[ ( 6 ) * 2 + 0 ] = vec[3+4] ;
	work->pos[ ( 6 ) * 2 + 1 ] = vec[2+4] ;
	work->pos[ ( 7 ) * 2 + 0 ] = vec[2+4] ;
	work->pos[ ( 7 ) * 2 + 1 ] = vec[0+4] ;
	work->pos[ ( 8 ) * 2 + 0 ] = vec[0] ;
	work->pos[ ( 8 ) * 2 + 1 ] = vec[0+4] ;
	work->pos[ ( 9 ) * 2 + 0 ] = vec[1] ;
	work->pos[ ( 9 ) * 2 + 1 ] = vec[1+4] ;
	work->pos[ ( 10 ) * 2 + 0 ] = vec[2] ;
	work->pos[ ( 10 ) * 2 + 1 ] = vec[2+4] ;
	work->pos[ ( 11 ) * 2 + 0 ] = vec[3] ;
	work->pos[ ( 11 ) * 2 + 1 ] = vec[3+4] ;

#else
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR			vec[8] ;
	int		i, k ;

	for ( i = 0 ; i < 8 ; i++ ){
		vec[ i ].vx = ( i & 1 ) ? max[0] : min[0] ;
		vec[ i ].vy = ( i & 2 ) ? max[1] : min[1] ;
		vec[ i ].vz = ( i & 4 ) ? max[2] : min[2] ;
	}
	work->pos[ ( 0 ) * 2 + 0 ] = vec[0] ;
	work->pos[ ( 0 ) * 2 + 1 ] = vec[1] ;
	work->pos[ ( 1 ) * 2 + 0 ] = vec[1] ;
	work->pos[ ( 1 ) * 2 + 1 ] = vec[3] ;
	work->pos[ ( 2 ) * 2 + 0 ] = vec[3] ;
	work->pos[ ( 2 ) * 2 + 1 ] = vec[2] ;
	work->pos[ ( 3 ) * 2 + 0 ] = vec[2] ;
	work->pos[ ( 3 ) * 2 + 1 ] = vec[0] ;
	work->pos[ ( 4 ) * 2 + 0 ] = vec[0+4] ;
	work->pos[ ( 4 ) * 2 + 1 ] = vec[1+4] ;
	work->pos[ ( 5 ) * 2 + 0 ] = vec[1+4] ;
	work->pos[ ( 5 ) * 2 + 1 ] = vec[3+4] ;
	work->pos[ ( 6 ) * 2 + 0 ] = vec[3+4] ;
	work->pos[ ( 6 ) * 2 + 1 ] = vec[2+4] ;
	work->pos[ ( 7 ) * 2 + 0 ] = vec[2+4] ;
	work->pos[ ( 7 ) * 2 + 1 ] = vec[0+4] ;
	work->pos[ ( 8 ) * 2 + 0 ] = vec[0] ;
	work->pos[ ( 8 ) * 2 + 1 ] = vec[0+4] ;
	work->pos[ ( 9 ) * 2 + 0 ] = vec[1] ;
	work->pos[ ( 9 ) * 2 + 1 ] = vec[1+4] ;
	work->pos[ ( 10 ) * 2 + 0 ] = vec[2] ;
	work->pos[ ( 10 ) * 2 + 1 ] = vec[2+4] ;
	work->pos[ ( 11 ) * 2 + 0 ] = vec[3] ;
	work->pos[ ( 11 ) * 2 + 1 ] = vec[3+4] ;


	prim = work->prim = 
	  GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING|DG_PRIM2_SINGLEBUFF, 1, 12*2 );
	if ( prim == NULL ) return ( 1 );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	//prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
   prim->as_world = *world ;
	prim->group_id = 0x7fffffff ;

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	uvrgb = prim->uvrgb[ 0 ] ;
	for ( k = 0 ; k < 12*2 ; k++ ){
		prim->pos[0][k] = work->pos[k] ;
		uvrgb[k].q = 4096 ;
		if ( k & 1 ){
			uvrgb[k].f = 0x0fff ;
		} else {
			uvrgb[k].f = 0x8fff ;
		}
		uvrgb[k].r = ( color ) & 255 ;
		uvrgb[k].g = ( color >> 8 ) & 255 ;
		uvrgb[k].b = ( color >> 16 ) & 255 ;
		uvrgb[k].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
	}

#endif

	return (0);
}


void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources_1( work, world, max, min, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
