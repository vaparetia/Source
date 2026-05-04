/*
	utl_dma.h
	ＤＭＡ関連ユーティリティ定義ヘッダ

	1999/09/03 K.Takabe
	$Id: utl_dma.h,v 1.1.1.3 2002/11/19 11:42:59 Yoshizawa1 Exp $

*/


	/*
		ＤＭＡ関連定義
	*/
#ifdef PSX2

#define DMATAG_PCE_0		(2<<26)
#define DMATAG_PCE_1		(3<<26)
#define DMATAG_ID_REFE		(0<<28)
#define DMATAG_ID_CNT		(1<<28)
#define DMATAG_ID_NEXT		(2<<28)
#define DMATAG_ID_REF		(3<<28)
#define DMATAG_ID_REFS		(4<<28)
#define DMATAG_ID_CALL		(5<<28)
#define DMATAG_ID_RET		(6<<28)
#define DMATAG_ID_END		(7<<28)
#define DMATAG_IRQ			(1<<31)
#define DMATAG_SET_QWC( flag, qwc ) ( (flag) | (qwc) )
#define DMATAG_SET_ADDR( addr ) ( (int)(addr) )
#define DMA_SIZE_OF( n )	( ( sizeof( n ) + 3 ) / 4 )

/* ＶＩＦｃｏｄｅ関連定義 */
#define VIF_VERT_PACK		(0x0d)	/* X(1.15.0),Y(1.15.0),Z(1.15.0),W(1.3.12) */
#define VIF_NORM_PACK		(0x0d)	/* X(1.3.12),Y(1.3.12),Z(1.3.12),W(0,16,0) */
#define VIF_UV_PACK			(0x15)	/* U(1.3.12),V(1.3.12) */
#define VIF_DATA128			(0x0c)	/*  */

#endif


/* ＤＭＡスタート前のフラグクリア */
static inline void UTL_DmaStartCheck( int ch )
{
#ifdef PSX2	
	DPUT_D_STAT( 1 << ch );
#endif	
}

#if 0 //BP_PS2

/* ＤＭＡ転送終了待ち */
static inline void UTL_WaitDma( int ch )
{
#ifdef PSX2	
	/*
		ＤＭＡ転送開始前に上のUTL_DmaStartCheck()を予め呼んでおいた
		場合のみ、この関数を使ってＤＭＡが終了するまでブロックする。
		ステータスレジスタによるポーリングに比べてＤＭＡに対する負荷が低い。
	*/
	DPUT_D_PCR( ( DGET_D_PCR() & 0xffff0000 ) | ( 1 << ch ) );
	asm("
0:
		nop		# 短いループだと分岐ミスを起こす恐れがあるらしいので
		nop		# ダミーを入れておく
		nop
		nop
		nop
		bc0f	0b
		nop
	");
#endif	
}

#endif

extern void UTL_StartSprToMem( void *dst, void *src, int size );
extern void UTL_EndSprToMem( void );
extern void UTL_StartMemToSpr( void *dst, void *src, int size );
extern void UTL_EndMemToSpr( void );
