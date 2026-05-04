/*
	m_print.c
	メニュープリント実験

	1999/07/26 K.Takabe
	$Id: m_print.c,v 1.1.1.3 2002/11/19 11:51:29 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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

#define	MAX_MENU_PRIMS	(2048)

#define N_PRIMS	(MAX_MENU_PRIMS)
#define N_PACKETS	(1)

#define TEX_BASE	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_WIDTH	(BUFFER_WIDTH/64)
#define CLUT_BASE	(0x100000/64-1)
#define CLUT_WIDTH	(64/64)

/* 表示関連設定 */
#define	MOJI_X_SPACE	(1)	/* 文字同士の隙間 */
#define	MOJI_Y_SPACE	(1)	/* 文字同士の隙間 */
#define MOJI_WIDTH		(8)	/* 文字幅 */
#define MOJI_HEIGHT		(8)	/* 文字高さ */

/* ------------------------------------------------------------ */
typedef	struct	{
	GV_ACT			actor ;

	DG_PRIM			*prim ;
	DG_MENU_SPRT	*last_sprt ;
	int				max_charas ;
	int				x, y, w, h ;
	DG_PRIM_RGBA	color ;
	u_long			packet_prim ;

	char			print_buffer[ 1024 ];

} Work ALIGN16 ;

static Work *print_work = NULL ;

/* ------------------------------------------------------------ */
void TEST_Color( int r, int g, int b, int a )
{
	if ( print_work == NULL ) return ;

	print_work->color.r = r ;
	print_work->color.g = g ;
	print_work->color.b = b ;
	print_work->color.a = a ;
}

void TEST_Locate( int x, int y, int flag )
{
	if ( print_work == NULL ) return ;

	print_work->x = ( x + 2048 - DRAW_WIDTH/2 ) * 16 ;
	print_work->y = ( y + 2048 - DRAW_HEIGHT/2 ) * 16 ;
	print_work->w = MOJI_WIDTH * 16 ;
	print_work->h = MOJI_HEIGHT * 16 ;
}

void TEST_Printf( char *fmt, ... )
{
	va_list			args ;
	DG_PRIM			*prim ;
	DG_PRIM_PACKET	*packet ;
	DG_MENU_SPRT	*sprt ;
	int				size ;
	int				x, y, w, h, u, v ;
	char			*string, c ;

	if ( print_work == NULL ) return ;
#if 1
	{/* 引数展開 */

		va_start( args, fmt );
		vsprintf( print_work->print_buffer, fmt, args );
		string = print_work->print_buffer ;
	}
#else
	string = "aaa" ;
#endif
	x = print_work->x ;
	y = print_work->y ;
	w = print_work->w ;
	h = print_work->h ;

	/* 文字数分パケットを生成 */
	prim = print_work->prim ;
	packet = prim->packs[ DG_Clock ] ;
	sprt = print_work->last_sprt ;
	while ( *string != '\0' ){
		/* プリミティブ数オーバーチェック */
		if ( print_work->max_charas >= MAX_MENU_PRIMS ){
			printf("menu prim over1\n");
			break ;
		}
		/* テクスチャＵＶ値決定 */
		c = *string ;
		if ( c >= ' ' && c < ( ' ' + 96 ) ){
			c -= ' ' ;
			u = ( c & 31 ) * 8 * 16 ;
			v = ( c / 32 ) * 8 * 16 ;
		} else {
			u = 0 ;
			v = 0 ;
		}
		/* パケットへの書き込み */
		sprt->prim = print_work->packet_prim ;
		sprt->rgba1 = print_work->color ;
		DG_SET_XY1( sprt, x, y );
		DG_SET_XY2( sprt, x + w, y + h );
		DG_SET_UV1( sprt, u, v );
		DG_SET_UV2( sprt, u + w + 15, v + h + 15 );
		/* インクリメント処理 */
		x += w + MOJI_X_SPACE * 16 ;
		sprt++ ;
		string++ ;
		print_work->max_charas++ ;
	}
	print_work->y = y + h + MOJI_X_SPACE * 16 ;
	print_work->last_sprt = sprt ;

	/* ＤＭＡ、ＶＩＦｃｏｄｅ及びＧＩＦタグの転送データ数を変更する */
	size = ( sizeof(DG_PRIM_INIT) + sizeof(sceGifTag) ) / sizeof(u_long128) + prim->prim_size * print_work->max_charas ;
	packet->dma_tag.vifcode[1] = SCE_VIF1_SET_DIRECT( size, 0 ) ;
	packet->dma_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
	packet->gif_tag.NLOOP = print_work->max_charas ;

	va_end( args );
}

/* ------------------------------------------------------------ */

/* パケットの初期化 */
static void InitPacket( DG_PRIM_PACKET *packet, int n, int which )
{
	DG_MENU_SPRT	*sprt ;
	int					data_size, i ;

	sprt = (DG_MENU_SPRT*)packet->prim_top ;
	/* プリミティブ描画設定初期化処理 */
	*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 0,1,0,1,128 ) ;
  	*(u_long*)&packet->prim_init.tex0 = 
	  SCE_GS_SET_TEX0( TEX_BASE, TEX_WIDTH,
					  SCE_GS_PSMT4HL, 8, 8, 1, 0,
					  CLUT_BASE, SCE_GS_PSMCT16, 0, 0, 1 );
	*(u_long*)&packet->prim_init.clamp = SCE_GS_SET_CLAMP( 1, 1, 0, 0, 1024, 1024 ) ;
#if 1 
#if 0
	/* クランプ設定の代わりにバイリニアＯＦＦの設定を行う（不具合でるかも） */
	*(u_long*)&packet->prim_init.clamp = SCE_GS_SET_TEX1_1( 0, 0, 0, 0, 0, 0, 0 ) ;
	packet->prim_init.clamp_addr = SCE_GS_TEX1_1 ;
#else
	/* アルファ設定の代わりにバイリニアＯＦＦの設定を行う（不具合でるかも） */
	*(u_long*)&packet->prim_init.alpha = SCE_GS_SET_TEX1_1( 0, 0, 0, 0, 0, 0, 0 ) ;
	packet->prim_init.alpha_addr = SCE_GS_TEX1_1 ;
#endif
#endif

#if 0 /* 初期化は特に必要ない（文字表示時に全部設定するため） */
	/* プリミティブデータ部の初期化処理 */
	for ( i = n ; i > 0 ; i-- ){
		sprt->prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 );
		DG_SET_RGBA1( sprt, 128, 128, 128, 128 );
		DG_SET_XY1( sprt, 0, 0 );sprt->xy1.pad[0] = 0xffff ;sprt->xy1.pad[1] = 0xffff ;
		DG_SET_XY2( sprt, 1, 1 );sprt->xy2.pad[0] = 0xffff ;sprt->xy2.pad[1] = 0xffff ;
		DG_SET_UV1( sprt, 0, 0 );
		DG_SET_UV2( sprt, 1, 1 );
		sprt++ ;
	}
#endif
}

static void Act( Work *work )
{
	DG_PRIM		*prim ;
	DG_PRIM_PACKET	*packet ;
	int				size ;

	prim = work->prim ;
	packet = prim->packs[ DG_Clock ] ;
	work->last_sprt = (DG_MENU_SPRT*)packet->prim_top ;
	work->max_charas = 0 ;
	size = ( sizeof(DG_PRIM_INIT) + sizeof(sceGifTag) ) / sizeof(u_long128) + prim->prim_size * print_work->max_charas ;
	packet->dma_tag.vifcode[1] = SCE_VIF1_SET_DIRECT( size, 0 ) ;
	packet->dma_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
	packet->gif_tag.NLOOP = print_work->max_charas ;

#if 0
	TEST_Locate( 100, 100, 0 );
	TEST_Color( 96, 96, 96, 128 );
	TEST_Printf( "This is a test !!! %d ", 123  );
	TEST_Color( 96, 48, 48, 128 );
	TEST_Printf( "ABCDEFG %f", 1234.456f  );
#endif
}

static void Die( Work *work )
{
	GM_FreePrim( work->prim );
	print_work = NULL ;
}

static int GetResources( Work *work )
{
	DG_PRIM	*prim ;
	DG_PRIM_PACKET *packet ;
	float	*f ;
	int		i, j ;

	/* メニュー用チャンネルにプリミティブを生成 */
	prim = work->prim = GM_MakePrimChanl( DG_PRIM_SORTONLY|DG_PRIM_MENU_SPRT, N_PACKETS, N_PRIMS, NULL, NULL, 4 );
	prim->near_z = -1 ;

	/* プリミティブの初期化 */
	for ( j = 0 ; j < 2 ; j++ ){
		for ( i = 0 ; i < prim->n_packet ; i++ ){
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[j] + prim->packet_size * i );
			InitPacket( packet, N_PRIMS, j );
			packet->header.sort_z = prim->near_z + 1 ;
		}
	}

	/* その他ワークの初期化 */
	packet = prim->packs[ DG_Clock ] ;
	work->last_sprt = (DG_MENU_SPRT*)packet->prim_top ;
	work->max_charas = 0 ;

	work->packet_prim = SCE_GS_SET_PRIM( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 );
	work->color.r = 96 ;
	work->color.g = 96 ;
	work->color.b = 96 ;
	work->color.a = 128 ;

	{/* テクスチャの読み込み */
		sceGsLoadImage	li ;
		u_long128		*font_data ;

		font_data = (u_long128*)GV_GetCache( GV_CacheID( 3459636/* "font" */, 'r' ) );

		while ( sceGsSyncPath( 0, 0 ) );

		sceGsSetDefLoadImage( &li,
							 TEX_BASE, TEX_WIDTH,
							 SCE_GS_PSMT4HL, 0, 0, 256, 24 );
		FlushCache(0);
		sceGsExecLoadImage( &li, &font_data[2] );
		while ( sceGsSyncPath( 0, 0 ) );

		sceGsSetDefLoadImage( &li,
							 CLUT_BASE, CLUT_WIDTH,
							 SCE_GS_PSMCT16, 0, 0, 16, 1 );
		FlushCache(0);
		sceGsExecLoadImage( &li, &font_data[0] );
		while ( sceGsSyncPath( 0, 0 ) );
		
	}

	/* スタティックワークに登録 */
	print_work = work ;

	return (0);
}


void *NewMenuPrintTest( void )
{
	Work		*work ;

	if ( print_work != NULL ) return (NULL);

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
