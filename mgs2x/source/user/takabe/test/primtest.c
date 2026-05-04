/*
	primtest.c
	プリミティブ表示実験プログラム

	1999/07/07 K.Takabe
	$Id: primtest.c,v 1.1.1.3 2002/11/19 11:51:29 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"


#define N_PRIMS	(20)
#define N_PACKETS	(1)

typedef	struct	{
	GV_ACT		actor ;

	DG_PRIM		*prim ;
	DG_PRIM		*prim_sprt ;
	FMATRIX		world ;
	FVECTOR		pos[128] ;
	FVECTOR		uvs[128] ;
	FVECTOR		pos2[64] ;
	FVECTOR		uvs2[64] ;

} Work ;

/* パケットの初期化 */
static void InitPacket( DG_PRIM_PACKET *packet, DG_TEX *tex, int n, int which )
{
	DG_POLY_GT4	*poly ;
	int					i ;

	poly = (DG_POLY_GT4*)packet->prim_top ;
	/* プリミティブ描画設定初期化処理 */
	*(u_long*)&packet->prim_init.tex0 = tex->tex_trans.tex0.data ;
	*(u_long*)&packet->prim_init.clamp = tex->tex_trans.clamp.data ;
	/* 半透明属性付加 */
	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	packet->gif_tag.PRE = 1 ;
	packet->gif_tag.REGS0 = GS_REGS_PRIM ;
	/* プリミティブデータ部の初期化処理 */
	for ( i = n ; i > 0 ; i-- ){
		DG_SET_POLYGT4( poly, 1 );
		DG_SET_RGBA1( poly, 128, 128, 128, 64 );
		DG_SET_RGBA2( poly, 128, 128, 128, 64 );
		DG_SET_RGBA3( poly, 128, 128, 128, 64 );
		DG_SET_RGBA4( poly, 128, 128, 128, 64 );
		DG_SET_STQ1( poly, 0.0F * tex->u_scale + tex->u_offset, 0.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_STQ2( poly, 1.0F * tex->u_scale + tex->u_offset, 0.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_STQ3( poly, 0.0F * tex->u_scale + tex->u_offset, 1.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_STQ4( poly, 1.0F * tex->u_scale + tex->u_offset, 1.0F * tex->v_scale + tex->v_offset, 1.0F );
		poly++ ;
	}
}

static void InitPacket2( DG_PRIM_PACKET *packet, DG_TEX *tex, int n, int which )
{
	DG_SPRT	*sprt ;
	int					i ;

	sprt = (DG_SPRT*)packet->prim_top ;
	/* プリミティブ描画設定初期化処理 */
	*(u_long*)&packet->prim_init.tex0 = tex->tex_trans.tex0.data ;
	*(u_long*)&packet->prim_init.clamp = tex->tex_trans.clamp.data ;
	*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;/* 加算半透明 */
	/* 半透明属性付加 */
	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	/* プリミティブデータ部の初期化処理 */
	for ( i = n ; i > 0 ; i-- ){
		DG_SET_RGBA1( sprt, 128, 128, 128, 128 );
		DG_SET_STQ1( sprt, 0.0F * tex->u_scale + tex->u_offset, 0.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_STQ2( sprt, 1.0F * tex->u_scale + tex->u_offset, 1.0F * tex->v_scale + tex->v_offset, 1.0F );
		sprt++ ;
	}
}

static void Act( Work *work )
{
#if 0
	{/* カメラの設定 */
		FVECTOR	cam = {1000.0F,2000.0F,12000.0F,0.0F}, trg = {0.0F,0.0F,0.0F,0.0F} ;
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( DG_Chanls, &cam, &trg, 2.0F );
	}
	{/* カメラの設定 */
		FVECTOR	cam = {-1000.0F,2000.0F,-1500.0F,0.0F}, trg = {0.0F,1000.0F,0.0F,0.0F} ;
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( DG_Chanls + 1, &cam, &trg, 2.0F );
	}
#endif
	{/* カメラの設定 */
		FVECTOR	cam = {-1000.0F,2000.0F,-1500.0F,0.0F}, trg = {0.0F,1000.0F,0.0F,0.0F} ;
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2( DG_Chanls + 4, &cam, &trg, 2.0F );
	}
}

static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	DG_TEX	*tex ;
	DG_PRIM	*prim ;
	DG_PRIM_PACKET *packet ;
	float	*f ;
	int		i, j ;

	prim = work->prim = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_POLY_GTQ4, N_PACKETS, N_PRIMS, work->pos, work->uvs );

	tex = DG_GetTexture( 10691 );
	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < prim->n_packet ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[j] + prim->packet_size * i );
			InitPacket( packet, tex, N_PRIMS, j );
		}
	}

	f = (float*)work->uvs ;
	for ( i = 0 ; i < N_PACKETS * N_PRIMS ; i++ ){
		work->pos[0+i*4].vx = 1000.0F + i * 1000.0F;
		work->pos[0+i*4].vy = 3000.0F ;
		work->pos[0+i*4].vz = 1000.0F + 20000 ;
		work->pos[1+i*4].vx = -1000.0F + i * 1000.0F;
		work->pos[1+i*4].vy = 2000.0F ;
		work->pos[1+i*4].vz = 1000.0F + 20000 ;
		work->pos[2+i*4].vx = 1000.0F + i * 1000.0F;
		work->pos[2+i*4].vy = 2000.0F ;
		work->pos[2+i*4].vz = -1000.0F + 20000 ;
		work->pos[3+i*4].vx = -1000.0F + i * 1000.0F;
		work->pos[3+i*4].vy = 1000.0F ;
		work->pos[3+i*4].vz = -1000.0F + 20000 ;
		f[i*8+0] = 0.0F * tex->u_scale + tex->u_offset ;
		f[i*8+1] = 0.0F * tex->v_scale + tex->v_offset ;
		f[i*8+2] = 1.0F * tex->u_scale + tex->u_offset ;
		f[i*8+3] = 0.0F * tex->v_scale + tex->v_offset ;
		f[i*8+4] = 0.0F * tex->u_scale + tex->u_offset ;
		f[i*8+5] = 1.0F * tex->v_scale + tex->v_offset ;
		f[i*8+6] = 1.0F * tex->u_scale + tex->u_offset ;
		f[i*8+7] = 1.0F * tex->v_scale + tex->v_offset ;
	}

#if 0
	prim = work->prim_sprt = GM_MakePrimChanl( DG_PRIM_ON_CAMERA|DG_PRIM_SPRT3D1, 50, 1, work->pos2, NULL, 4 );
#else
	prim = work->prim_sprt = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_SPRT3D1, 50, 1, work->pos2, NULL );
	//prim = work->prim_sprt = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_SPRT3D2, 50, 1, work->pos2, work->uvs2 );
#endif

	tex = DG_GetTexture( 10691 );
	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < prim->n_packet ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[j] + prim->packet_size * i );
			InitPacket2( packet, tex, 1, j );
		}
	}

	/* DG_PRIM_SPRT3D2で使用するサイズ指定配列はqwordでアラインされていなければならない */
	f = (float*)work->uvs2 ;
	for ( i = 0 ; i < 50 ; i++ ){
		work->pos2[i].vx = ( rand()%20000 - 10000 ) ;
		work->pos2[i].vy = ( rand()%5000 ) ;
		work->pos2[i].vz = ( rand()%20000 - 10000 ) ;
		/* スプライトサイズ指定（DG_PRIM_SPRT3D1使用時に必要） */
		work->pos2[i].vw = 1000.0F ;
		/* スプライトサイズ指定（DG_PRIM_SPRT3D2使用時に必要） */
		f[i*2+0] = rand()%1000 ;
		f[i*2+1] = rand()%1000 ;
	}

	//NewBlurTest();

	return (0);
}


void *NewPrimTestProgram( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
