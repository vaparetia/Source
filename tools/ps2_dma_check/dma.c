/*
	dma.c
	DMA解析＆エミュレーションルーチン

	2000/06/29 K.Takabe
	$Id: dma.c,v 1.2 2001/10/22 06:00:49 usr02774 Exp $
*/

#include <stdio.h>
#include "app.h"
#include "dmacheck.h"

/* ---------------------------------------------------------------- */
/* 指定したアドレスの内容を1QWORD読み込む */
void ReadQword( unsigned int addr, unsigned int *buffer )
{
	if ( addr < 0x00100000 || addr > 0x02000000 ){
		printf("address error!! (%08x)\n", addr );
		exit( 0 );
		return ;
	}
	/* deci library function */
	DbgpReadMemory( addr, buffer, 16 );
}


/* ---------------------------------------------------------------- */
/* DMAタグを解析 */
void CheckDmaTag( unsigned int *tag, char *output )
{
	static char *dmatag_ID[8] = { "REFE","CNT ","NEXT","REF ","REFS","CALL","RET ","END " };
	int		qwc, pce, id, irq ;

	/* DMATAGを要素に分解 */
	irq = ( tag[0] >> 31 ) & 0x0001 ;
	id  = ( tag[0] >> 28 ) & 0x0007 ;
	pce = ( tag[0] >> 26 ) & 0x0003 ;
	qwc = ( tag[0] >>  0 ) & 0xffff ;

	/* 解析文字列作成 */
	sprintf( output, "IRQ:%d ID:%s PCE:%d QWC:%04x ADDR:%08x",
			 irq,
			 dmatag_ID[id],
			 pce,
			 qwc,
			 tag[1] );
}


/* ---------------------------------------------------------------- */
/* DMA初期化 */
void InitDma( DmaWork *dma, int mode )
{
	dma->flag = mode ;
	InitVif( &dma->vif_work, mode );
	InitGif( &dma->gif_work, mode );
}

/* DMA実行開始 */
void SendDma( DmaWork *dma, unsigned int addr )
{
	dma->flag &= ~DMA_FLAG_TERMINATED ;
	dma->stack_count = 0 ;
	dma->qwc = 0 ;
	dma->tadr = addr ;
}

/* DMAタグを実行 */
void ExecDma( DmaWork *dma )
{
	int		pce, irq ;
	char	out_buffer[256] ;

	/* 動作チェック */
	if ( dma->flag & DMA_FLAG_TERMINATED ) return ;

	if ( dma->qwc == 0 ){

		/* データ読み込み＆DMATAG解析 */
		ReadQword( dma->tadr, dma->buffer );
		if ( dma->flag & DMA_FLAG_DISP_TAG ){
			/* デバッグ用表示 */
			CheckDmaTag( dma->buffer, out_buffer );
			printf("%08x: %s\n", dma->tadr, out_buffer );
		}
		irq      = ( dma->buffer[0] >> 31 ) & 0x0001 ;
		dma->id  = ( dma->buffer[0] >> 28 ) & 0x0007 ;
		pce      = ( dma->buffer[0] >> 26 ) & 0x0003 ;
		dma->qwc = ( dma->buffer[0] >>  0 ) & 0xffff ;

		/* VIF実行が有効でかつ、タグ転送が有効であればVIFを実行する */
		if ( ( dma->flag & DMA_FLAG_EXEC_VIF ) && ( dma->flag & DMA_FLAG_TTE ) ){
			if ( dma->flag & DMA_FLAG_SKIP_DATA ) dma->vif_work.skip_count = 0 ;/* デバッグ用 */
			ExecVif( &dma->vif_work, dma->buffer[ 2 ] );
			ExecVif( &dma->vif_work, dma->buffer[ 3 ] );
		}

		/* タグID毎に次のタグアドレス及びデータ転送アドレスの再設定を行なう */
		switch ( dma->id ){
		case DMATAG_CNT:
			dma->madr = dma->tadr + 16 ;
			dma->tadr = dma->tadr + 16 + dma->qwc * 16 ;
			break ;
		case DMATAG_REF:
		case DMATAG_REFE:
		case DMATAG_REFS:
			dma->madr = dma->buffer[1] ;
			dma->tadr = dma->tadr + 16 ;
			break ;
		case DMATAG_NEXT:
			dma->madr = dma->tadr + 16 ;
			dma->tadr = dma->buffer[1] ;
			break ;
		case DMATAG_CALL:
			if ( dma->stack_count >= 2 ){
				printf("DMATAG_CALL: stack over error!!\n");
				dma->flag |= DMA_FLAG_TERMINATED ;
				break ;
			}
			dma->madr = dma->tadr + 16 ;
			dma->dma_stack[ dma->stack_count++ ] = dma->tadr + 16 + dma->qwc * 16 ;
			dma->tadr = dma->buffer[1] ;
			break ;
		case DMATAG_RET:
			if ( dma->stack_count <= 0 ){
				printf("DMATAG_RET: stack over error!!\n");
				dma->flag |= DMA_FLAG_TERMINATED ;
				break ;
			}
			dma->madr = dma->tadr + 16 ;
			dma->tadr = dma->dma_stack[ --(dma->stack_count) ] ;
			break ;
		case DMATAG_END:
			dma->madr = dma->tadr + 16 ;
			break ;
		}

		if ( dma->flag & DMA_FLAG_DISP_TAG ){
			if ( dma->qwc != 0 ){
				printf(" [dma data] %08x - %08x\n", dma->madr, dma->madr + dma->qwc * 16 );
			}
		}

	} else {
		/* データ転送 */
		if ( dma->flag & DMA_FLAG_SKIP_DATA ){
			/* 内容は完全にスキップ（デバッグ用） */
			dma->madr += dma->qwc * 16 ;
			dma->qwc = 0 ;

		} else {
			/* DMAが実際に何らかの実行を行なう必要がある場合はメモリの内容を読み出す */
			if ( dma->flag & ( DMA_FLAG_EXEC_VIF | DMA_FLAG_EXEC_GIF ) ){
				ReadQword( dma->madr, dma->buffer );
			}
			dma->madr += 16 ;
			dma->qwc-- ;

			/* VIF実行が有効であればVIFを実行する */
			if ( dma->flag & DMA_FLAG_EXEC_VIF ){
				ExecVif( &dma->vif_work, dma->buffer[ 0 ] );
				ExecVif( &dma->vif_work, dma->buffer[ 1 ] );
				ExecVif( &dma->vif_work, dma->buffer[ 2 ] );
				ExecVif( &dma->vif_work, dma->buffer[ 3 ] );
			} else {
				ExecGif( &dma->vif_work.gif_work, dma->buffer );
			}

		}

	}

	if ( dma->qwc == 0 ){
		/* 転送が終了した時に直前のTAGIDが終了コードであればDMAを停止 */
		if ( dma->id == DMATAG_END || dma->id == DMATAG_REFE ) dma->flag |= DMA_FLAG_TERMINATED ;
	}

	return ;
}



