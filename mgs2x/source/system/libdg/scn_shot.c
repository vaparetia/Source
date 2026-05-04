/*
	scn_shot.c
	スクリーンショット撮影

	2000/03/21 K.Takabe
	$Id: scn_shot.c,v 1.1.1.3 2002/11/19 11:42:23 Yoshizawa1 Exp $

*/
/*

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

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libfs.h"

/* ---------------------------------------------------------------- */

#ifdef DEBUG_MODE

/* セーブファイル名記録 */
static char	DG_SaveFileName[256] ;

static int		DG_VramSaveCount = 0 ;

extern int		DG_OffsetControlFlag ;

/* ---------------------------------------------------------------- */




/* ---------------------------------------------------------------- */
	/*
		スクリーンショット撮影（４フレーム後）
	*/
void DG_SaveScreen( char *filename )
{
	char		*str ;

	if ( DG_VramSaveCount != 0 ) return ;
	str = DG_SaveFileName ;
	while ( ( *str++ = *filename++ ) != '\0' );

	DG_VramSaveCount = 5 ;
	DG_OffsetControlFlag = 1 ;
}

void DG_CheckSaveScreen( void )
//void _DG_CheckSaveScreen( void )
{
	static ALIGN16_PRE sceGsStoreImage store_image ALIGN16_POST ;
	int		fd, y ;
	void	*mem_addr0, *mem_addr1 ;

	//if ( GV_PadData[1].press & PAD_A ) DG_SaveScreen( "host0:shottest.raw" ) ;
	if ( DG_VramSaveCount == 0 ) return ;

	if ( --DG_VramSaveCount == 0 ){

		/* 描画が終了するまでウェイトを入れる */
		while ( DG_DmaCheckEnd() == 0 );
		sceGsSyncPath( 0, 0 );			/* 念のため */

		if ( DG_Clock == 0 )	mem_addr0 = PACK_ADDR1 ;
		else					mem_addr0 = PACK_ADDR0 ;
		mem_addr1 = mem_addr0 + 512 * 1024 ;

#ifndef HIGHRESO_FFI
		/* 通常ハイレゾモード用ルーチン */
		sceGsSetDefStoreImage( &store_image,
							  BUFFER_PAGE(0)/64, DRAW_WIDTH / 64,
							  FRAME_BUFFER_COLOR_MODE(),
							  0, 0, DRAW_WIDTH, DRAW_HEIGHT );
		FlushCache(0);
		sceGsExecStoreImage( &store_image, mem_addr0 );
		sceGsSyncPath( 0, 0 );

		sceGsSetDefStoreImage( &store_image,
							  BUFFER_PAGE(1)/64, DRAW_WIDTH / 64,
							  FRAME_BUFFER_COLOR_MODE(),
							  0, 0, DRAW_WIDTH, DRAW_HEIGHT );
		FlushCache(0);
		sceGsExecStoreImage( &store_image, mem_addr1 );
		sceGsSyncPath( 0, 0 );

		FlushCache(0);

		/* ファイルへのセーブ処理 */
		fd = pcOpen( DG_SaveFileName , SCE_WRONLY | SCE_TRUNC | SCE_CREAT);

		for ( y = 0 ; y < DRAW_HEIGHT ; y++ ){
			pcWrite( fd, (int*)mem_addr0 + y * DRAW_WIDTH, DRAW_WIDTH * sizeof(int) );
			pcWrite( fd, (int*)mem_addr1 + y * DRAW_WIDTH, DRAW_WIDTH * sizeof(int) );
		}

		pcClose(fd);

#else
#ifndef PAL
		/* フリッカーフリーモード用ルーチン */
		sceGsSetDefStoreImage( &store_image,
							  BUFFER_PAGE(0), DRAW_WIDTH / 64,
							  FRAME_BUFFER_COLOR_MODE(),
							  0, 0, DRAW_WIDTH, DRAW_HEIGHT );
		FlushCache(0);
		sceGsExecStoreImage( &store_image, mem_addr0 );
		sceGsSyncPath( 0, 0 );

		FlushCache(0);
#else
		/* ＰＡＬでは２回に分けて転送 */
		/* フリッカーフリーモード用ルーチン */
		sceGsSetDefStoreImage( &store_image,
							  BUFFER_PAGE(0), DRAW_WIDTH / 64,
							  FRAME_BUFFER_COLOR_MODE(),
							  0, 0, DRAW_WIDTH, DRAW_HEIGHT/2 );
		FlushCache(0);
		sceGsExecStoreImage( &store_image, mem_addr0 );
		sceGsSyncPath( 0, 0 );

		FlushCache(0);
		/* フリッカーフリーモード用ルーチン */
		sceGsSetDefStoreImage( &store_image,
							  BUFFER_PAGE(0), DRAW_WIDTH / 64,
							  FRAME_BUFFER_COLOR_MODE(),
							  0, DRAW_HEIGHT/2, DRAW_WIDTH, DRAW_HEIGHT/2 );
		FlushCache(0);
		sceGsExecStoreImage( &store_image, (void*)((int)mem_addr0+DRAW_WIDTH*DRAW_HEIGHT*COLOR_DEPTH/8/2) );
		sceGsSyncPath( 0, 0 );

		FlushCache(0);
#endif

		/* ファイルへのセーブ処理 */
		fd = pcOpen( DG_SaveFileName , SCE_WRONLY | SCE_TRUNC | SCE_CREAT);

		for ( y = 0 ; y < DRAW_HEIGHT ; y++ ){
			pcWrite( fd, (int*)mem_addr0 + y * DRAW_WIDTH, DRAW_WIDTH * sizeof(int) );
		}

		pcClose(fd);
#endif
		printf("finish!!\n");

		DG_OffsetControlFlag = 0 ;
	}
}



void _DG_CheckSaveScreen( void )
//void DG_CheckSaveScreen( void )
{
	static ALIGN16_PRE sceGsStoreImage store_image ALIGN16_POST ;
	int		fd, x, y, i, j ;
	void	*mem_addr0, *mem_addr1 ;

	if ( GV_PadData[1].press & PAD_AL ) DG_SaveScreen( "host0:shottest.raw" ) ;
	if ( DG_VramSaveCount == 0 ) return ;

	if ( --DG_VramSaveCount == 0 ){

		/* 描画が終了するまでウェイトを入れる */
		while ( DG_DmaCheckEnd() == 0 );
		sceGsSyncPath( 0, 0 );			/* 念のため */
		FlushCache(0);

		mem_addr0 = (void*)0x02000000 ;


#if 0
		/* １６サンプルキャプチャリング */
		for ( i = 0 ; i < 16 ; i++ ){

			for ( j = 0 ; j < 4 ; j++ ){
				static int	offset_x[16] = {0,4,8,12,0,4,8,12,0,4,8,12,0,4,8,12};
				static int	offset_y[16] = {0,0,0,0,4,4,4,4,8,8,8,8,12,12,12,12};
				int		f ;
				DG_CHANL	*cp ;
				cp = DG_Chanl(j) ;
				if ( cp->flag ){
					*(u_long64*)&cp->draw_offset[DG_Clock].datas.xyoffset1 = 
					  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - cp->offset_x ) << 4 ) + offset_x[i],
										  ( ( 2048 - DRAW_HEIGHT / 2 - cp->offset_y ) << 4 ) + offset_y[i] ) ;
				}
			}

			printf("start draw %d\n", i);
			FlushCache(0);
			DG_DmaStart( DG_Clock );
			while ( DG_DmaCheckEnd() == 0 );
			printf("end draw %d\n", i);

			/* フリッカーフリーモード用ルーチン */
			sceGsSetDefStoreImage( &store_image,
								  BUFFER_PAGE(DG_Clock)/64,
								  DRAW_WIDTH / 64,
								  FRAME_BUFFER_COLOR_MODE(),
								  0, 0, DRAW_WIDTH, DRAW_HEIGHT );
			FlushCache(0);
			sceGsExecStoreImage( &store_image, (void*)( (int)mem_addr0 + 1024*1024*i ) );
			sceGsSyncPath( 0, 0 );

			FlushCache(0);
		}

		{
			CVECTOR	*col ;
			int		r, g, b, a ;
			col = (CVECTOR*)mem_addr0 ;
			for ( y = 0 ; y < DRAW_HEIGHT ; y++ ){
				for ( x = 0 ; x < DRAW_WIDTH ; x++ ){
					r = g = b = a = 0 ;
					for ( i = 0 ; i < 16 ; i++ ){
						r += col[512*512*i].r ;
						g += col[512*512*i].g ;
						b += col[512*512*i].b ;
						a += col[512*512*i].cd ;
					}
					r /= 16 ;
					g /= 16 ;
					b /= 16 ;
					a /= 16 ;
					col[0].r = r ;
					col[0].g = g ;
					col[0].b = b ;
					col[0].cd = a ;
					col++ ;
				}
			}
			FlushCache(0);
		}

		/* ファイルへのセーブ処理 */
		fd = pcOpen( DG_SaveFileName , SCE_WRONLY | SCE_TRUNC | SCE_CREAT);

		for ( y = 0 ; y < DRAW_HEIGHT ; y++ ){
			pcWrite( fd, (int*)mem_addr0 + y * DRAW_WIDTH, DRAW_WIDTH * sizeof(int) );
		}
#else
		/* 最大解像度（２５６倍サンプル）キャプチャリング */

		for ( i = 0 ; i < 64 ; i++ ){
			int	offset_x ;
			int	offset_y ;

			offset_x = ( i & 7 ) * 2 ;
			offset_y = ( i / 8 ) * 2 ;
			for ( j = 0 ; j < 4 ; j++ ){
				DG_CHANL	*cp ;
				cp = DG_Chanl(j) ;
				if ( cp->flag ){
					*(u_long64*)&cp->draw_offset[DG_Clock].datas.xyoffset1 = 
					  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - cp->offset_x ) << 4 ) + offset_x,
										  ( ( 2048 - DRAW_HEIGHT / 2 - cp->offset_y ) << 4 ) + offset_y ) ;
				}
			}

			printf("start draw %d\n", i);
			FlushCache(0);
			DG_DmaStart( DG_Clock );
			while ( DG_DmaCheckEnd() == 0 );
			printf("end draw %d\n", i);

			/* フリッカーフリーモード用ルーチン */
			sceGsSetDefStoreImage( &store_image,
								  BUFFER_PAGE(DG_Clock)/64,
								  DRAW_WIDTH / 64,
								  FRAME_BUFFER_COLOR_MODE(),
								  0, 0, DRAW_WIDTH, DRAW_HEIGHT );
			FlushCache(0);
			sceGsExecStoreImage( &store_image, (void*)( (int)mem_addr0 + 1024*1024*i ) );
			sceGsSyncPath( 0, 0 );

			FlushCache(0);

		}

		/* ファイルへのセーブ処理 */
		fd = pcOpen( DG_SaveFileName , SCE_WRONLY | SCE_TRUNC | SCE_CREAT);

		/* １６分割で取り込む */
		for ( y = 0 ; y < DRAW_HEIGHT*8 ; y++ ){
			int		*mem_addr ;
			char	*src, *dst ;
			mem_addr = (int*)( (int)mem_addr0 + 1024*1024*64 );
			for ( x = 0 ; x < DRAW_WIDTH*8 ; x++ ){
				mem_addr[x] = ((int*)mem_addr0)[ x / 8 + ( y / 8 ) * DRAW_WIDTH + 512*512*(x&7) + 512*512*8*(y&7) ] ;
			}
			src = mem_addr ;
			dst = src ;
			for ( x = 0 ; x < DRAW_WIDTH*8 ; x++ ){
				*dst++ = *src++ ;
				*dst++ = *src++ ;
				*dst++ = *src++ ;
				src++ ;
			}
			FlushCache(0);
			//pcWrite( fd, (int*)mem_addr, DRAW_WIDTH * sizeof(int) * 8 );
			pcWrite( fd, (int*)mem_addr, DRAW_WIDTH * 3 * 8 );
		}

#endif

		pcClose(fd);
		printf("finish!!\n");

		DG_OffsetControlFlag = 0 ;

		for ( j = 0 ; j < 4 ; j++ ){
			DG_CHANL	*cp ;
			cp = DG_Chanl(j) ;
			if ( cp->flag ){
				*(u_long64*)&cp->draw_offset[DG_Clock].datas.xyoffset1 = 
				  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - cp->offset_x ) << 4 ),
									  ( 2048 - DRAW_HEIGHT / 2 - cp->offset_y ) << 4 ) ;
			}
		}
	}
}


#else
void DG_SaveScreen( char *filename )
{
}
#endif
