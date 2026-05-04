/*
	KCEJ SIF MANAGER
		sifcmd, sifrpc のサービス関数群
		ソースコードそのものはEE,IOP共通

	1999/11/17	K.Uehara
	$Id: libmain.c,v 1.16 2001/03/01 09:12:49 usr01475 Exp $
*/


#include <stdio.h>
#ifdef IOP
#include <kernel.h>
#endif
#ifdef EE
#include <eekernel.h>
#endif
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>

#include "common.h"
#include "jsifman.h"

/* 識別用タグ */
#ifdef IOP
ModuleInfo Module = { "KCEJ_SIF_MANAGER", 0x0101 };
#endif

sceSifCmdData sifcmd_buffer[ SIF_CALLBACK_FUNC_MAX ];

int callback_set_flag = 0;

/* ------------------------------------------------------------------- */
/*
	インターフェース関数
*/

// 割り込みコールバック関数の登録
int sif_set_callback_func( int id, void *func, void *data )
{
	INT_CTRL;
	FUNC_INIT();

	DI();
	sceSifAddCmdHandler( id, func, data );
	callback_set_flag |= ( 1 << id );
	EI();
	{
		sceSifCmdSRData d;
		int id;

		/* どのコールバックが設定されたかを向うのシステムレジスタに設定する */
		d.rno = SIF_SYSREG_IDSET;
		d.value = callback_set_flag;
		id = sceSifSendCmd( SIF_CMDC_SET_SREG, &d, sizeof( d ), 0, 0, 0 );

		while( sceSifDmaStat( id ) >= 0 ){
			;
		}
	}
	FUNC_EXIT();
	return 0;
}

/* ------------------------------------------------------------------- */
/*
	インターフェース関数
*/

/*
	パケット中で送れる最大サイズ。これを越えるものは
	自前でヘッダつきパケットを用意する。
*/

#ifdef IOP

/*
	IOPは、ライブラリでバッファリングされないので、
	システムでバッファリングを行なう。
*/

#define MAX_PACKET	4

typedef struct {
	sceSifCmdHdr head;	/* 16 byte */
	int data[ MAX_PACKET_DATA_SIZE / sizeof( int ) ];
	int id;
} SIFCMD_PACKET;

static SIFCMD_PACKET cmdpacket[ MAX_PACKET ] __attribute__((aligned(16)));

/*
	head.fcodeが-1(0xFFFFFFFF)になることはないので,	それを空きフラグにする
*/

#define PACK_SET_FREE( _sp )	((_sp)->head.fcode = 0xFFFFFFFF)
#define PACK_SET_USED( _sp )	((_sp)->head.fcode = 0)
#define IS_PACK_FREE( _sp )		((_sp)->head.fcode == 0xFFFFFFFF)

static inline SIFCMD_PACKET *get_packet( void )
{
	SIFCMD_PACKET *res;

	// 割り込み禁止領域で呼ぶ。
	// DMA使用中フラグをクリアするため,すべてのパケットを検索する

	res = NULL;

	while( res == NULL ){
		int i;
		SIFCMD_PACKET *pack;

		pack = cmdpacket;
		for( i = 0; i < MAX_PACKET; i++ ){
			if( !IS_PACK_FREE( pack ) ){
				// DMA使用中
				if( pack->id != 0 && sceSifDmaStat( pack->id ) < 0 ){
					pack->id = 0;
					PACK_SET_FREE( pack );
					res = pack;
				}
			} else {
				res = pack;
			}
			pack ++;
		}
	}
	PACK_SET_USED( res );
	return res;
}
#define i_get_packet()	get_packet()

#endif // IOP

#ifdef EE
/*
	EEはライブラリでバッファリングを行なうので、
	システムではバッファリングしない。

	-> stack上に持つことにする。(00/12/25)
*/

typedef struct {
	sceSifCmdHdr head;	/* 16 byte */
	int data[ MAX_PACKET_DATA_SIZE / sizeof( int ) ];	/* 80 byte */
	int id;
} SIFCMD_PACKET;

#if 0
static SIFCMD_PACKET cmdpacket __attribute__((aligned(64)));
static SIFCMD_PACKET icmdpacket __attribute__((aligned(64)));

static inline SIFCMD_PACKET *get_packet( void )
{
	return &cmdpacket;
}

static inline SIFCMD_PACKET *i_get_packet( void )
{
	return &icmdpacket;
}
#endif
#endif	// EE

int sif_send_packet_and_data( int id, int fno, void *packet, int psize
								, void *dest, void *src, int size )
{
	int i;
	int dma_id;
#ifdef IOP
	SIFCMD_PACKET *pack;
	INT_CTRL;

	FUNC_INIT();

	IOP_DI();
	pack = get_packet();
	IOP_EI();
#endif
#ifdef EE
	SIFCMD_PACKET packetbuffer __attribute__((aligned(16)));
	SIFCMD_PACKET *pack = &packetbuffer;
#endif

	pack->head.opt = fno;
	if( psize > MAX_PACKET_DATA_SIZE ){
		printf( "jsifman:PACKET SIZE OVER %d\n", psize );
	}

	if( psize > 0 ){
		int *d = ( void * )pack->data, *s = packet;
		for( i = psize / 4; i > 0; i-- ){
			*( d++ ) = *( s++ );
		}
	}
	do {
		dma_id = sceSifSendCmd( id, pack, psize + sizeof( sceSifCmdHdr ), src, dest, size );
	} while( dma_id == 0 );

#ifdef IOP
	pack->id = dma_id;
#endif
	FUNC_EXIT();

	return dma_id;
}

int i_sif_send_packet_and_data( int id, int fno, void *packet, int psize
								, void *dest, void *src, int size )
{
	int i;
	int dma_id;
#ifdef IOP
	SIFCMD_PACKET *pack;

	FUNC_INIT();

	pack = i_get_packet();
#endif
#ifdef EE
	SIFCMD_PACKET packetbuffer __attribute__((aligned(16)));
	SIFCMD_PACKET *pack = &packetbuffer;
#endif

	pack->head.opt = fno;

	if( psize > MAX_PACKET_DATA_SIZE ){
		iPrintf( "jsifman:PACKET SIZE OVER %d\n", psize );
	}
	if( psize > 0 ){
		int *d = ( void * )pack->data, *s = packet;

		for( i = psize / 4; i > 0; i-- ){
			*( d++ ) = *( s++ );
		}
	}
	do {
		dma_id = isceSifSendCmd( id, pack, psize + sizeof( sceSifCmdHdr )
								 , src, dest, size );
	} while( dma_id == 0 );

#ifdef IOP
	pack->id = dma_id;
#endif
	FUNC_EXIT();
	return dma_id;
}

/* ---------------------------------------------------------------------- */
/*
	ダイレクト発送型
*/

int sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *src, int size )
{
	SIFCMD_PACKET *pack;
	pack = packet;
	pack->head.opt = fno;
	return sceSifSendCmd( id, packet, psize, src, dst, size );
}

int i_sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *src, int size )
{
	SIFCMD_PACKET *pack;
	pack = packet;
	pack->head.opt = fno;
	return isceSifSendCmd( id, packet, psize, src, dst, size );
}

/* ---------------------------------------------------------------------- */
/*
	割り込みなしDMA転送 を発生
*/

#ifdef EE

int i_sif_send_mem( void *dst, void *src, int size )
{
	sceSifDmaData dma;
	int id;

	dma.data = ( unsigned int )src;
	dma.addr = ( unsigned int )dst;
	dma.size = size;
	dma.mode = 0;

	do {
		id = isceSifSetDma( &dma, 1 );
	} while( id == 0 );

	return id;
}

int sif_send_mem( void *dst, void *src, int size )
{
	int id;
	sceSifDmaData dma;

	dma.data = ( unsigned int )src;
	dma.addr = ( unsigned int )dst;
	dma.size = size;
	dma.mode = 0;

	do {
		id = sceSifSetDma( &dma, 1 );
	} while( id == 0 );
	
	return id;
}

#endif

#ifdef IOP

#define MAX_SEND_PACKET 4
typedef struct {
	sceSifDmaData dma[ MAX_SEND_PACKET ];
	int id[ MAX_SEND_PACKET ];
	int flag[ MAX_SEND_PACKET ];
} SIFDMA_PACKET;

static SIFDMA_PACKET dmapacket __attribute__((aligned(16)));

static inline int get_packet_no( void )
{
	// 割り込み禁止領域で呼ぶこと
	int r = -1;

	while( r < 0 ){
		int i;
		for( i = 0; i < MAX_SEND_PACKET; i++ ){
			int id = dmapacket.id[ i ];
			if( dmapacket.flag[ i ] != 0 ){
				if( id != 0 && sceSifDmaStat( id ) < 0 ){
					dmapacket.id[ i ] = 0;
					dmapacket.flag[ i ] = 0;
					r = i;
					break;
				}
			} else {
				r = i;
				break;
			}
		}
	}
	dmapacket.flag[ r ] = 1;
	return r;
}

int i_sif_send_mem( void *dst, void *src, int size )
{
	int r;
	sceSifDmaData *dma;
	int id;

	r = get_packet_no();

	dma = &dmapacket.dma[ r ];

	dma->data = ( unsigned int )src;
	dma->addr = ( unsigned int )dst;
	dma->size = size;
	dma->mode = 0;

	do {
		id = sceSifSetDma( dma, 1 );
	} while( id == 0 );
	dmapacket.id[ r ] = id;

	return id;
}

int sif_send_mem( void *dst, void *src, int size )
{
	int id, r;
	sceSifDmaData *dma;
	INT_CTRL;

	IOP_DI();

	r = get_packet_no();
	dma = &dmapacket.dma[ r ];

	dma->data = ( unsigned int )src;
	dma->addr = ( unsigned int )dst;
	dma->size = size;
	dma->mode = 0;

	do {
		id = sceSifSetDma( dma, 1 );
	} while( id == 0 );
	dmapacket.id[ r ] = id;
	
	IOP_EI();

	return id;
}

#endif

/* ------------------------------------------------------------------- */
/*
	チェック用
*/

int sif_check_status( int id )
{
	int stat;
	INT_CTRL;

	IOP_DI();
	stat = sceSifDmaStat( id );
	IOP_EI();

	return stat;
}

/* ------------------------------------------------------------------- */
/*
	初期化エントリ
*/

static int init_flag = 0;

int sif_init( void )
{
	INT_CTRL;

	if( init_flag ) return 0;
	init_flag = 1;

#ifdef IOP
	sceSifInitCmd();
#endif
#ifdef EE
	sceSifInitRpc( 0 );
#endif

	DI();
	sceSifSetCmdBuffer( sifcmd_buffer, SIF_CALLBACK_FUNC_MAX * sizeof( sceSifCmdData ) );
	callback_set_flag = 0;
	EI();

#ifdef IOP
	{
		int i;
		for( i = 0; i < MAX_PACKET; i++ ){
			PACK_SET_FREE( &cmdpacket[ i ] );
		}
	}
#endif

	return 0;
}

#ifdef IOP

int KCEJ_sifman_init( void )
{
    extern libhead jsifman_entry; /* ライブラリ名_entry を参照 */
	FUNC_INIT();

    if( RegisterLibraryEntries(&jsifman_entry) != 0 ) {
		/* 既に同名の常駐ライブラリがいるので登録に失敗 */
		return NO_RESIDENT_END; /* 終了してメモリから退去 */
    }

//printf( "KCEJ SIF MANAGER ver0.01\n" );

	sif_init();
	sif_init_rv_man();

	FUNC_EXIT();

    return RESIDENT_END; /* 終了して常駐する */
}

#endif
