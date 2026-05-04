/*
	calibur.c
	光残像

	1999/10/29 Y.Korekado
	$Id: calibur.c,v 1.1.1.3 2002/11/19 11:44:01 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/

#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>

#include	"gameheader.h"

/*----------------------------------------------------------------*/
#define PRIO	0x60	/* 敵兵より後に処理 */
#define	N_PACKETS			(128)
#define	N_PRIMS				(1)
#define	DIV				N_PACKETS

#define	GR1			(28)
#define	GG1			(96)
#define	GB1			(78)
#define	GR2			(55)
#define	GG2			(192)
#define	GB2			(155)
#define	GOURAUD		(256/DIV)

typedef	struct	{
	GV_ACT		actor ;
	int			map ;
	
	DG_PRIM		*prim ;
	DG_TEX		*tex ;
	FMATRIX		*world ;
	FVECTOR		shift[2] ;
	int			*sw ;

	FVECTOR		pos[ N_PRIMS * N_PACKETS ][4] ;
	FVECTOR		uvs[ 2 * N_PRIMS * N_PACKETS ] ;
	int			curent ;
	int			count ;
} Work ;

/* 定数定義 --------------------------------------------------------*/

/*----------------------------------------------------------------*/
static	void	SetNewVector ( work ) 
Work	*work ;
{
	int	curent, before ;

	before = work->curent ;
	if ( ++work->curent >= DIV ) work->curent = 0 ;
	curent = work->curent ;

	DG_SetPos( work->world ) ;	/* 刀のマトリックス */
	DG_PutVector( work->shift, work->pos[ curent ], 2 ) ;	/* 絶対座標へ変換 */
	work->pos[curent][2] = work->pos[before][0] ;
	work->pos[curent][3] = work->pos[before][1] ;

//printf( " x=%d z=%d  x=%d z=%d\n",work->vec[ curent ][0].vx,work->vec[ curent ][0].vz
//,work->vec[ curent ][1].vx,work->vec[ curent ][1].vz);

#if 0
	AN_Test_Eye2( &work->pos[curent][2], 3 );
	AN_Test_Eye2( &work->pos[curent][3], 3 );
#endif

}

static	void	SetNewGouraud ( work ) 
Work	*work ;
{
	DG_PRIM 	*prim ;	
	DG_PRIM_PACKET *packet ;
	DG_POLY_GT4	*poly, *packs ;
	int	i, j, t, a, gr, gg, gb ;
	int	r, g, b, flg ;


	flg = *(work->sw) ;
#if 0
		r = 255 ;		g = 0 ;		b = GB2 ;
#else
	if ( flg == 2 ) {
		r = GR2 ;		g = GG2 ;		b = GB2 ;
	} else {
		r = GR1 ;		g = GG1 ;		b = GB1 ;
	}
#endif

	prim = work->prim ;
	for ( j = 0 ; j < prim->n_packet ; j++ ){

		t = work->curent - j ;
		if ( t < 0 ) t += DIV ;

		packet = (DG_PRIM_PACKET*)( (int)prim->packs[ DG_Clock ] + prim->packet_size * t );
//		poly = (DG_POLY_GT4*)packet->prim_top ;
		packs = (DG_POLY_GT4*)packet->prim_top ;

		for ( i = 0 ; i < prim->n_prims ; i++ ){
			a = j ;
			if ( (gr = r - (GOURAUD*a)) < 0) gr = 0 ; 
			if ( (gg = g - (GOURAUD*a)) < 0) gg = 0 ;
			if ( (gb = b - (GOURAUD*a)) < 0) gb = 0 ;

			DG_SET_RGBA1( packs, gr, gg, gb, 128 );
			DG_SET_RGBA2( packs, 0, 0, 0, 128 );

			a++ ;
			if ( (gr = r - (GOURAUD*a)) < 0) gr = 0 ; 
			if ( (gg = g - (GOURAUD*a)) < 0) gg = 0 ;
			if ( (gb = b - (GOURAUD*a)) < 0) gb = 0 ;

			DG_SET_RGBA3( packs, gr, gg, gb, 128 );
			DG_SET_RGBA4( packs, 0, 0, 0, 128 );
			
//			poly++ ;
		}
	}
}

/*----------------------------------------------------------------*/
#define TIME	690
#define LIGHT 	128

static	void	Act( work )
Work		*work ;
{
	OPERATOR() ;

	SetNewVector( work )  ;
	SetNewGouraud( work )  ;
	
//	if ( *(work->sw) > 0 ) {
		DG_VisiblePrim( work->prim );
//	} else {
//		DG_InvisiblePrim( work->prim );
//	}

}

/*----------------------------------------------------------------*/

/* パケットの初期化 */
static void InitPacket( DG_PRIM *prim, DG_TEX *tex )
{
	DG_PRIM_PACKET *packet ;
	DG_POLY_GT4	*poly ;
	int	i, j, k, r,g,b ;

	r=128; g=128; b=128;
	for ( k = 0 ; k < 2 ; k++ ){
		for ( i = 0 ; i < prim->n_packet ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[k] + prim->packet_size * i );

			/* 半透明属性付加 */
			packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;

			/* プリミティブ描画設定初期化処理 */
			*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,2,0,1,0 ) ;/* 加算半透明 */
//			*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 2,0,0,1,0 ) ;/* 減算半透明 */

			poly = (DG_POLY_GT4*)packet->prim_top ;
			for ( j = 0 ; j < prim->n_prims ; j++ ){
				*(u_long*)&packet->prim_init.tex0 = tex->tex0_base ;
				*(u_long*)&packet->prim_init.clamp = tex->clamp ;
#if 1
				DG_SET_RGBA1( poly, r+100, g+100, b+100, 128 );
				DG_SET_RGBA2( poly, r+100, g+100, b+100, 128 );
				DG_SET_RGBA3( poly, r-100, g-100, b-100, 128 );
				DG_SET_RGBA4( poly, r-100, g-100, b-100, 128 );
#else
				DG_SET_RGBA1( poly, 128, 128, 128, 128 );
				DG_SET_RGBA2( poly, 128, 128, 128, 128 );
				DG_SET_RGBA3( poly, 128, 128, 128, 128 );
				DG_SET_RGBA4( poly, 128, 128, 128, 128 );
#endif
				poly ++ ;
			}
		}
	}
}

static	int	GetResources( work, world, shift_s, shift_e, sw )
Work	*work ;
FMATRIX	*world ;
FVECTOR	*shift_s ;
FVECTOR	*shift_e ;
int		*sw ;
{
	DG_PRIM		*prim ;
	DG_TEX		*tex ;
	float		*f ;
	int			i ;

	OPERATOR() ;

	work->world = world ;
	work->shift[0] = *shift_s ;
	work->shift[1] = *shift_e ;
	work->sw = sw ;

	prim = work->prim = GM_MakePrim(DG_PRIM_ON_WORLD|DG_PRIM_POLY_GTQ4, DIV, N_PRIMS, work->pos, work->uvs );
//	tex = DG_GetTexture( 16545701/* gradation_alp */ );
	tex = DG_GetTexture( GV_StrCode( "full100_add" ) );
//	tex = DG_GetTexture( GV_StrCode( "rcm_l" ) );
	if ( tex == NULL ) return -1 ;

	InitPacket( prim, tex ) ;

	f = (float*)work->uvs ;
	for ( i = 0 ; i < N_PACKETS ; i++ ){
		f[i*8+0] = 0.0F * tex->u_scale + tex->u_offset ;
		f[i*8+1] = 0.0F * tex->v_scale + tex->v_offset ;
		f[i*8+2] = 1.0F * tex->u_scale + tex->u_offset ;
		f[i*8+3] = 0.0F * tex->v_scale + tex->v_offset ;
		f[i*8+4] = 0.0F * tex->u_scale + tex->u_offset ;
		f[i*8+5] = 1.0F * tex->v_scale + tex->v_offset ;
		f[i*8+6] = 1.0F * tex->u_scale + tex->u_offset ;
		f[i*8+7] = 1.0F * tex->v_scale + tex->v_offset ;
	}


	work->count = 0 ;
	work->curent = 0 ;

	return 0;
}

static	void	Die( work )
Work		*work ;
{
	OPERATOR() ;
	GM_FreePrim( work->prim ) ;
}


void		*NewCalibur( world, shift_s, shift_e, sw )
FMATRIX	*world ;
FVECTOR	*shift_s ;
FVECTOR	*shift_e ;
int		*sw ;
{
	Work		*work ;

	OPERATOR() ;
//return ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( work, Act, Die ) ;
		if ( GetResources( work, world, shift_s, shift_e, sw ) < 0 ) {
			printf(" Katana Init Err !! \n") ;
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

