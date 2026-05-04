/*
	main.c
	PS2 DMA tag checker

	2000/06/29 K.Takabe
	$Id: main.c,v 1.2 2001/10/22 06:00:49 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include "app.h"
#include "dmacheck.h"


typedef struct _exec_param {
	int		start_addr ;		/* DMA開始アドレス */
	int		break_addr ;		/* DMA停止アドレス */
	int		type ;				/* DMAタイプ */
	int		level ;				/* GIF解析レベル */
} ExecParam ;


int DmaTrace( ExecParam *param );

int main( int argc, char *argv[] )
{
	ExecParam	param ;
	int			i ;

	if( t_connect( argv[ 0 ] ) != 0 ){
		exit( 1 );
	}

	memset( &param, 0, sizeof(ExecParam) );

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 's' ){
				sscanf( argv[1], "%x", &param.start_addr );
				argv++ ;
				argc-- ;
			}
			if ( argv[0][1] == 'b' ){
				sscanf( argv[1], "%x", &param.break_addr );
				argv++ ;
				argc-- ;
			}
			if ( argv[0][1] == 'v' ){
				param.type = 0 ;
			}
			if ( argv[0][1] == 'g' ){
				param.type = 1 ;
			}
			if ( argv[0][1] == 'l' ){
				sscanf( argv[1], "%d", &param.level );
				argv++ ;
				argc-- ;
			}
		} else {
#if 0
			switch ( mode ){
			  default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
#endif
		}
	}

	if ( param.start_addr == 0 ){
		printf("PS2 DMA tag checker\n");
		printf("Usage: ps2_dma_check [options] -start <start address>\n");
		printf("Options:  -break <address> ... trace break address\n");
		printf("          -vif             ... dma type vif(default)\n");
		printf("          -gif             ... dma type gif\n");
		printf("          -level <n>       ... gif disp level (0~2)\n");
		return ( 0 );
	}

	//param.start_addr = 0x01d00000 ;
	//param.break_addr = 0x01b1c7d0 ;
	DmaTrace( &param );

	return 0;
}

int DmaTrace( ExecParam *param )
{
	DmaWork	dma_work ;
	int		i, flag ;

	/* DMA初期化 */

	flag = 0 ;
	switch ( param->type ){
	case 0:
		/* DMA動作モード設定 */
		flag |= DMA_FLAG_EXEC_VIF ;
		flag |= DMA_FLAG_EXEC_GIF ;
		flag |= DMA_FLAG_TTE ;
		flag |= DMA_FLAG_DISP_TAG ;
		/* VIF動作モード設定 */
		flag |= DMA_FLAG_DISP_VIF ;
		/* GIF動作モード設定 */
		if ( param->level > 0 ) flag |= DMA_FLAG_DISP_GIF ;
		if ( param->level > 1 ) flag |= DMA_FLAG_DISP_GS ;
		break ;
	case 1:
		/* DMA動作モード設定 */
		flag |= DMA_FLAG_EXEC_GIF ;
		flag |= DMA_FLAG_DISP_TAG ;
		/* GIF動作モード設定 */
		if ( param->level > 0 ) flag |= DMA_FLAG_DISP_GIF ;
		if ( param->level > 1 ) flag |= DMA_FLAG_DISP_GS ;
		break ;
	}

	InitDma( &dma_work, flag );

	/* DMA転送開始 */
	SendDma( &dma_work, param->start_addr );

	for ( i = 0 ; i < 0x2000000 ; i++ ){
		ExecDma( &dma_work );
		if ( dma_work.flag & DMA_FLAG_TERMINATED ) break ;
		if ( dma_work.tadr == param->break_addr && dma_work.qwc == 0 ) break ;
		if ( dma_work.madr == param->break_addr && dma_work.qwc != 0 ) break ;
	}
}
