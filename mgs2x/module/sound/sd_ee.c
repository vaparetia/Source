/*
	sd_ee.c
		サウンドプログラムEE側インターフェース

	1999/11/30 K.Uehara
	$Id: sd_ee.c,v 1.15 2000/10/31 01:28:18 usr00339 Exp $
*/

#include <stdio.h>

#include "mgs_type.h"

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>

#if 1 //BP_PS2
//direct calls to iop functions work fine for us!
int bp_iop_sd_set_cli (int sound_code);
void bp_iop_sd_init (void);
void bp_iop_main();
#else
#include "../jsifman/jsifman.h"
#endif

static int status[ 16 ] //BP_GCC __attribute__((aligned (64)))
;

int sd_init( void )
{
#if 1 //BP_PS2
   bp_iop_sd_init();
   bp_iop_main( status );
#else
	/* IOP 側との通信を確保 ( timeout つき ) */

	sif_init();
	sif_init_rv_man();
#if 0	// TIMEOUT
	{
		int timeout;
		for( timeout = 60; timeout > 0; timeout -- ){
			if( sif_check_setup_module( IOP_SIF_SD_SET ) ){
				goto OK;
			}
			sceGsSyncV( 0 );
		}
	}
	return 0;
OK:
#else
	while( !sif_check_setup_module( IOP_SIF_SD_SET ) ){
		;
	}
#endif
	sceGsSyncV( 0 );
	{
		status[ 0 ] = ( int )status;
		sif_send_packet( IOP_SIF_SD_SET, 0, status, 16 );
	}
#endif
	return 1;
}

void sd_set_cli( int code )
{
volatile int *p;

/* 2000/01/06 ロード時、ステータス変更bitをonする */
#if 0
	if ((code & 0xFF000000) == 0x01000000) {	/* ＢＧＭロード */
		if ((code >= 0x01000010) && (code < 0x01FFFF00)) (( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] = 0x80000000;
	}
	if ((code & 0xFF000000) == 0x02000000) {	/* 効果音ロード */
		(( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] = 0x80000000;
	}
	if ((code & 0xFF000000) == 0xFE000000) {	/* 波形ロード */
		(( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] = 0x80000000;
	}
#endif
#if 1 //BP_PS2
   status[ 1 ] |= 0x80000000;
   bp_iop_sd_set_cli(code);
#else
   (( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] |= 0x80000000;
	sif_send( IOP_SIF_SD_SET, code );
#endif

	if ((code & 0xFF000000) == 0xFE000000) {	/* パック・ロード */
#if 1 //BP_PS2
      BP_TED_BREAK;
#else
		while (1) {
		//ロード動作の開始を待つ2000/10/31
			sceGsSyncV( 0 );
//			if ( (( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] & 7) break;
			p = (( int * )( ( unsigned int )status | 0x20000000 ) );
			if (p[1] & 7) break;
		}
#endif
	}

	if ((code & 0xFF000000) == 0xF1000000) {	/* ストリーミング1 */
#if 1 //BP_PS2
      status[ 1 ] |= 0x1000;
#else
		(( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] |= 0x1000;
#endif
	}
	if ((code & 0xFF000000) == 0xF3000000) {	/* ストリーミング2 */
#if 1 //BP_PS2
      status[ 1 ] |= 0x2000;
#else
		(( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ] |= 0x2000;
#endif
	}
}

int *sd_status( void )
{
	/* キャッシュスルーで読み込む */

#if 1 //BP_PS2
   return status;
#else
	return ( ( int * )( ( unsigned int )status | 0x20000000 ) );
#endif
//	return ( ( int * )( ( unsigned int )status | 0x20000000 ) )[ 1 ];
}
