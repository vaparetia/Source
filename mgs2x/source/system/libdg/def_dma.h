/*
	def_dma.h
	ＤＭＡ関連定義ヘッダ

	1999/07/07 K.Takabe
	$Id: def_dma.h,v 1.1.1.3 2002/11/19 11:42:05 Yoshizawa1 Exp $

*/

#ifndef __DEF_DMA_H__
#define __DEF_DMA_H__

#include "bp_ps2emu.h"

/*----------------------------------------------------------------*/
	/*
		ＤＭＡ関連定義
	*/

/* ＤＭＡ関連定義 */
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

/* ＶＩＦｃｏｄｅ関連定義 */
#define VIF_VERT_PACK		(0x0d)	/* X(1.15.0),Y(1.15.0),Z(1.15.0),W(1.3.12) */
#define VIF_NORM_PACK		(0x0d)	/* X(1.3.12),Y(1.3.12),Z(1.3.12),W(0,16,0) */
#define VIF_UV_PACK			(0x15)	/* U(1.3.12),V(1.3.12) */
#define VIF_RGBA_PACK		(0x0e)	/* R(0.8.0),G(0.8.0),B(0.8.0),A(0.8.0) */
#define VIF_DATA128			(0x0c)	/*  */

/* サイズアライメントマクロ＆サイズ取得マクロ */
#define ALIGNSIZE1(_n) (_n)							/* そのまま */
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)		/* １６バイト単位にサイズをアラインする */
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)		/* ６４バイト単位にサイズをアラインする */
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)	/* １２８バイト単位にサイズをアラインする */
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))			/* ４バイト単位換算でのサイズを求める */
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))	/* １６バイト単位換算でのサイズを求める */

#ifdef PSX2
/*----------------------------------------------------------------*/
	/*
		ＤＭＡ制御補助関数
	*/

/* サイクルスチールモードＯＮ */
static inline void DG_CycleStealingOn( void )
{
	//BP_PS2 DPUT_D_CTRL( ( DGET_D_CTRL() & ~0x702 ) | 0x002 );
}

/* サイクルスチールモードＯＦＦ（デフォルト） */
static inline void DG_CycleStealingOff( void )
{
	//BP_PS2 DPUT_D_CTRL( ( DGET_D_CTRL() & ~0x702 ) | 0x000 );
}

/* ＤＭＡスタート前のフラグクリア */
static inline void DG_DmaStartCheck( int ch )
{
	//BP_PS2 DPUT_D_STAT( 1 << ch );
}
/* ＤＭＡ転送終了待ち */
static inline void DG_WaitDma( int ch )
{
#if 0 //BP_PS2
	DPUT_D_PCR( ( DGET_D_PCR() & 0xffff0000 ) | ( 1 << ch ) );
	asm volatile ("
		sync.l	# レジスタへの書き込みが終了するまでウェイトを入れる
0:
		nop		# 短いループだと分岐ミスを起こす恐れがあるらしいので
		nop		# ダミーを入れておく
		nop
		nop
		nop
		nop
		bc0f	0b
		nop
	");
#endif
}


/*----------------------------------------------------------------*/
	/*
		スクラッチパッド＜＞メモリ間ＤＭＡ制御補助関数
	*/

/* スクラッチパッドからメインメモリへの転送（システム用） */
static inline void DG_StartSprToMem( void *mem_addr, void *spr_addr, int size )
{
#if 0 //BP_PS2
	DG_DmaStartCheck( 8 );/* fromSPR */
	DPUT_D8_MADR( (int)mem_addr );
	DPUT_D8_SADR( (int)spr_addr );
	DPUT_D8_QWC( size );
	//DPUT_D8_CHCR( 0x0100 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D8_CHCR),"r"(0x100) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */
#else
   memcpy( mem_addr, spr_addr, size * sizeof(u_long128) );
#endif
}
/* スクラッチパッドからメインメモリへの転送終了待ち（システム用） */
static inline void DG_EndSprToMem( void )
{
	//BP_PS2 DG_WaitDma( 8 );/* fromSPR */
}

/* メインメモリへからスクラッチパッドの転送（システム用） */
static inline void DG_StartMemToSpr( void *spr_addr, void *mem_addr, int size )
{
#if 0 //BP_PS2
	DG_DmaStartCheck( 9 );/* toSPR */
	DPUT_D9_MADR( (int)mem_addr );
	DPUT_D9_SADR( (int)spr_addr );
	DPUT_D9_QWC( size );
	//DPUT_D9_CHCR( 0x0101 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D9_CHCR),"r"(0x0101) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:1 */
#else
   memcpy( spr_addr, mem_addr, size * sizeof(u_long128) );
#endif
}
/* メインメモリへからスクラッチパッドの転送終了待ち（システム用） */
static inline void DG_EndMemToSpr( void )
{
	//BP_PS2 DG_WaitDma( 9 );/* toSPR */
}


/* スクラッチパッドからメインメモリへインタリーブモードでの転送（システム用） */
static inline void DG_StartSprToMemInterleave( void *mem_addr, void *spr_addr, int size, int t_size, int s_size )
{
#if 0 //BP_PS2
	DG_DmaStartCheck( 8 );/* fromSPR */
	DPUT_D_SQWC( ( s_size ) | ( t_size << 16 ) );
	DPUT_D8_MADR( (int)mem_addr );
	DPUT_D8_SADR( (int)spr_addr );
	DPUT_D8_QWC( size );
	asm volatile( "sync.l;sw %1, 0(%0)"::"r"(D8_CHCR),"r"(0x108) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:2 DIR:0 */
#else
   char* pCurrentDest = mem_addr;
   char* pCurrentSource = spr_addr;
   int totalCopySize = size * 16;
   int const elementSize = t_size * 16;
   int const totalElementStride = (t_size + s_size) * 16;

   while(totalCopySize > 0)
   {
      memcpy(pCurrentDest, pCurrentSource, elementSize);
      totalCopySize -= elementSize;

      pCurrentSource += elementSize;
      pCurrentDest += totalElementStride;
   }
#endif
}

/* メインメモリからスクラッチパッドへインタリーブモードでの転送（システム用） */
static inline void DG_StartMemToSprInterleave( void *spr_addr, void *mem_addr, int size, int t_size, int s_size )
{
#if 0 //BP_PS2
	DG_DmaStartCheck( 9 );/* toSPR */
	DPUT_D_SQWC( ( s_size ) | ( t_size << 16 ) );
	DPUT_D9_MADR( (int)mem_addr );
	DPUT_D9_SADR( (int)spr_addr );
	DPUT_D9_QWC( size );
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D9_CHCR),"r"(0x0109) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:2 DIR:1 */
#else
   char* pCurrentDest = spr_addr;
   char* pCurrentSource = mem_addr;
   int totalCopySize = size * 16;
   int const elementSize = t_size * 16;
   int const totalElementStride = (t_size + s_size) * 16;

   while(totalCopySize > 0)
   {
      memcpy(pCurrentDest, pCurrentSource, elementSize);
      totalCopySize -= elementSize;

      pCurrentDest += elementSize;
      pCurrentSource += totalElementStride;
   }
#endif
}

/*----------------------------------------------------------------*/
#if 0 /* デバッグ用 */
extern void UTL_StartSprToMem( void *mem_addr, void *spr_addr, int size );	/* CPU転送バージョン */
extern void UTL_StartMemToSpr( void *spr_addr, void *mem_addr, int size );	/* CPU転送バージョン */
#define DG_StartSprToMem UTL_StartSprToMem
#define DG_EndSprToMem UTL_EndSprToMem
#define DG_StartMemToSpr UTL_StartMemToSpr
#define DG_EndMemToSpr UTL_EndMemToSpr
#endif
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
	/*
		スクラッチパッドを使用したデータ書き出しキャッシュ機構（ＤＭＡパケット生成に使用）
	*/
typedef struct {
 	int			clock ;
	int			count ;
	int			max_count ;
	u_long128	*current ;
	u_long128	*mem_addr ;
	u_long128	*end_addr ;
	int			pad[2] ;
	u_long128	dma_buffer[0] ;
} DG_CacheFIFO ;
/* スクラッチパッドの指定アドレス以降をパケットメモリ書き出し用のＦＩＦＯキャッシュとして初期化 */
static inline void DG_InitCacheFIFO( void *addr, void *mem_addr, void *end_addr )
{
	DG_CacheFIFO	*work = addr ;
	work->clock = 0 ;
	work->count = 0 ;
	work->max_count = ( (int)SCRPAD_ADDR + 0x4000 - (int)addr - 32 ) / 16 / 2;
	work->current = work->dma_buffer ;
	work->mem_addr = mem_addr ;
	work->end_addr = end_addr ;
}
/* 現在の仮想メモリ書き出し位置の取得 */
static inline u_long128* DG_GetCacheFIFOAddr( void *addr )
{
	DG_CacheFIFO	*work = addr ;
	return ( work->mem_addr + work->count );
}
/* ＦＩＦＯキャッシュ経由でデータをパケットメモリに書き出す */
static inline void DG_SendCacheFIFO( void *work_addr, void *data_src, int size )
{
	DG_CacheFIFO	*work = work_addr ;

	/* オーバーランチェック */
	if ( ( (int)work->mem_addr + work->count ) > (int)work->end_addr ) return ;

	if ( ( work->count + size ) > work->max_count ){
		DG_EndSprToMem();
		DG_StartSprToMem( work->mem_addr,
						 ( work->clock == 0 ) ? &work->dma_buffer[ 0 ] : &work->dma_buffer[ work->max_count ],
						 work->count );
		work->mem_addr += work->count ;
		work->clock = 1 - work->clock ;
		work->count = 0 ;
		work->current = ( work->clock == 0 ) ? &work->dma_buffer[ 0 ] : &work->dma_buffer[ work->max_count ] ;
	}
	{
		int		i ;
		u_long128	*src, *dst ;
		src = (u_long128*)data_src ;
		dst = (u_long128*)work->current ;
		for ( i = size ; i > 4 ; i-=4 ){
			dst[ 0 ] = src[ 0 ] ;
			dst[ 1 ] = src[ 1 ] ;
			dst[ 2 ] = src[ 2 ] ;
			dst[ 3 ] = src[ 3 ] ;
			dst += 4 ;
			src += 4 ;
		}
		for ( ; i > 0 ; i-- ){
			*dst++ = *src++ ;
		}
		work->current += size ;
		work->count += size ;
	}
}
/* ＦＩＦＯキャッシュに残っているデータをすべてメモリに書き出しスクラッチパッド領域を破棄する */
static inline void* DG_EndCacheFIFO( void *work_addr )
{
	DG_CacheFIFO	*work = work_addr ;

#ifdef PSX2
	if ( work->count > 0 ){
		//BP_PS2 asm volatile( "sync.l" );
		//BP_PS2 asm volatile( "sync.p" );
		DG_EndSprToMem();
		DG_StartSprToMem( work->mem_addr,
						 ( work->clock == 0 ) ? &work->dma_buffer[ 0 ] : &work->dma_buffer[ work->max_count ],
						 work->count );
		work->mem_addr += work->count ;
		work->clock = 1 - work->clock ;
		work->count = 0 ;
		work->current = ( work->clock == 0 ) ? &work->dma_buffer[ 0 ] : &work->dma_buffer[ work->max_count ] ;
	}
	//BP_PS2 asm volatile( "sync.l" );
	//BP_PS2 asm volatile( "sync.p" );
	DG_EndSprToMem();
#endif
	return ( work->mem_addr );
}

#else
/* XBOXの場合 */
extern void UTL_StartSprToMem( void *mem_addr, void *spr_addr, int size );	/* CPU転送バージョン */
extern void UTL_StartMemToSpr( void *spr_addr, void *mem_addr, int size );	/* CPU転送バージョン */
#define DG_StartSprToMem UTL_StartSprToMem
#define DG_EndSprToMem UTL_EndSprToMem
#define DG_StartMemToSpr UTL_StartMemToSpr
#define DG_EndMemToSpr UTL_EndMemToSpr
/* スクラッチパッドからメインメモリへインタリーブモードでの転送（システム用） */
static inline void DG_StartSprToMemInterleave( void *mem_addr, void *spr_addr, int size, int t_size, int s_size )
{
	u_long128	*src, *dst ;
	int		i, j ;

	src = spr_addr ;
	dst = mem_addr ;
	for ( i = 0 ; i < size ; i += t_size ){
		for ( j = 0 ; j < t_size ; j++ ){
			*dst++ = *src++ ;
		}
		dst += s_size ;
	}
}

/* メインメモリからスクラッチパッドへインタリーブモードでの転送（システム用） */
static inline void DG_StartMemToSprInterleave( void *spr_addr, void *mem_addr, int size, int t_size, int s_size )
{
	u_long128	*src, *dst ;
	int		i, j ;

	src = mem_addr ;
	dst = spr_addr ;
	for ( i = 0 ; i < size ; i += t_size ){
		for ( j = 0 ; j < t_size ; j++ ){
			*dst++ = *src++ ;
		}
		src += s_size ;
	}
}
#endif


#endif
