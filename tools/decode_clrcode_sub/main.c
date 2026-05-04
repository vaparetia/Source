/*
  
  クリアコードデコーダ Substance 用
  2002/08/06 M.Kobayashi
  $Id: main.c,v 1.4 2002/10/02 06:47:24 usr03700 Exp $
  
*/

#include	<stdio.h>
#include	<string.h>
#include	"clearcode.h"
#include	"format.h"

char *program_name = "decode_clrcode_sub";

void usage( void )
{
	printf( "decode_clrcode_sub code...\n" );
	exit( 1 );
}

#define result_print( name )	printf("%s : %d\n", #name, data[ name ] )
#define common_result_print( name, max ) printf("%s : %d\n", #name, data[ name + max ] )

extern void DisplayGame( u_int* data );
extern void DisplayBoss( u_int* data );
extern void DisplayBossEx( u_int* data );

static const int common_format[ COMMON_DATA_MAX ] = COMMON_CODE_FORMAT;

static void DecodeGame( CODE_BIT_DATA* pCrypt )
{
	static const int format[ GM_DATA_MAX ] = GM_CODE_FORMAT;
	int i;
	int bitlen = 0;
	CODE_BIT_DATA	org;

	// 長さ計算
	for( i = 0 ; i < GM_DATA_MAX ; i++ ) {
		bitlen += format[ i ];
	}
	for( i = 0 ; i < COMMON_DATA_MAX ; i++ ) {
		bitlen += common_format[ i ];
	}

	pCrypt->bitlen = bitlen + CODE_ADD_LEN;

	if ( ClearCodeDecodeEx( &org, pCrypt ) ) {
		// データ読み込み
		u_int data[ GM_DATA_MAX + COMMON_DATA_MAX ];
		int pos = 0;
		for( i = 0; i < GM_DATA_MAX; i++ ) {
			data[ i ] = ClearCodeGetData( &org, pos, format[ i ] );
			pos += format[ i ];
		}
		for( i = 0; i < COMMON_DATA_MAX; i++ ) {
			data[ GM_DATA_MAX + i ] = ClearCodeGetData( &org, pos, common_format[ i ] );
			pos += common_format[ i ];
		}
		DisplayGame( data );

	} else {
		printf("decode error\n");
		exit( 1 );
	}
}

static void DecodeBoss( CODE_BIT_DATA* pCrypt )
{
	static const int format[ BS_DATA_MAX ] = BS_CODE_FORMAT;
	int i;
	int bitlen = 0;
	CODE_BIT_DATA	org;

	// 長さ計算
	for( i = 0 ; i < BS_DATA_MAX ; i++ ) {
		bitlen += format[ i ];
	}
	for( i = 0 ; i < COMMON_DATA_MAX ; i++ ) {
		bitlen += common_format[ i ];
	}

	pCrypt->bitlen = bitlen + CODE_ADD_LEN;

	if ( ClearCodeDecodeEx( &org, pCrypt ) ) {
		// データ読み込み
		u_int data[ BS_DATA_MAX ];
		int pos = 0;
		for( i = 0; i < BS_DATA_MAX; i++ ) {
			data[ i ] = ClearCodeGetData( &org, pos, format[ i ] );
			pos += format[ i ];
		}
		for( i = 0; i < COMMON_DATA_MAX; i++ ) {
			data[ BS_DATA_MAX + i ] = ClearCodeGetData( &org, pos, common_format[ i ] );
			pos += common_format[ i ];
		}
		DisplayBoss( data );
	} else {
		printf("decode error\n");
		exit( 1 );
	}
}

static void DecodeBossEx( CODE_BIT_DATA* pCrypt )
{
	static const int format[ BE_DATA_MAX ] = BE_CODE_FORMAT;
	int i;
	int bitlen = 0;
	CODE_BIT_DATA	org;

	// 長さ計算
	for( i = 0 ; i < BE_DATA_MAX ; i++ ) {
		bitlen += format[ i ];
	}
	for( i = 0 ; i < COMMON_DATA_MAX ; i++ ) {
		bitlen += common_format[ i ];
	}

	pCrypt->bitlen = bitlen + CODE_ADD_LEN;

	if ( ClearCodeDecodeEx( &org, pCrypt ) ) {
		// データ読み込み
		u_int data[ BE_DATA_MAX ];
		int pos = 0;
		for( i = 0; i < BE_DATA_MAX; i++ ) {
			data[ i ] = ClearCodeGetData( &org, pos, format[ i ] );
			pos += format[ i ];
		}
		for( i = 0; i < COMMON_DATA_MAX; i++ ) {
			data[ BE_DATA_MAX + i ] = ClearCodeGetData( &org, pos, common_format[ i ] );
			pos += common_format[ i ];
		}
		DisplayBossEx( data );
	} else {
		printf("decode error\n");
		exit( 1 );
	}
}

static void DecodeVR( CODE_BIT_DATA* pCrypt )
{
	static const int format[ VR_DATA_MAX ] = VR_CODE_FORMAT;
	int i;
	int bitlen = 0;
	CODE_BIT_DATA	org;

	// 長さ計算
	for( i = 0 ; i < VR_DATA_MAX ; i++ ) {
		bitlen += format[ i ];
	}
	for( i = 0 ; i < COMMON_DATA_MAX ; i++ ) {
		bitlen += common_format[ i ];
	}

	pCrypt->bitlen = bitlen + CODE_ADD_LEN;

	if ( ClearCodeDecodeEx( &org, pCrypt ) ) {
		// データ読み込み
		u_int data[ VR_DATA_MAX ];
		int pos = 0;
		for( i = 0; i < VR_DATA_MAX; i++ ) {
			data[ i ] = ClearCodeGetData( &org, pos, format[ i ] );
			pos += format[ i ];
		}
		for( i = 0; i < COMMON_DATA_MAX; i++ ) {
			data[ VR_DATA_MAX + i ] = ClearCodeGetData( &org, pos, common_format[ i ] );
			pos += common_format[ i ];
		}

		DisplayVR( data );

	} else {
		printf("decode error\n");
		exit( 1 );
	}
}

int main ( int argc, char* argv[] )
{	
	u_char	src26[ CODE_MAX_BYTE ];
	CODE_BIT_DATA crypt;
	CODE_BIT_DATA dst;
	int i, j, n;

	if( argc < 2 ) usage();

	{ // 26進数を設定
		n = 0;
		for( j = 1 ; j < argc ; j++ ) {
			for( i = 0 ; i < strlen( argv[ j ] ) ; i++ ) {
				src26[ n ] = toupper(argv[ j ][ i ]) - 'A';
				if( src26[ n ] >= 26 ) {
					printf("invalid code\n");
					exit( 1 );
				}
				++n;
				if( n >= CODE_MAX_BYTE ) {
					printf("too long code\n");
					exit( 1 );
				}
			}
		}
		src26[ n ] = '\0';
	}

	// デコード
	ClearCodeChangeRadix( crypt.data, CODE_MAX_BYTE, 256,
						  src26, n, 26 );
	switch( n ) {
	case GM_CODE_LEN:
		DecodeGame(&crypt);
		break;
	case BS_CODE_LEN:
		DecodeBoss(&crypt);
		break;
	case BE_CODE_LEN:
		DecodeBossEx(&crypt);
		break;
	case VR_CODE_LEN:
		DecodeVR(&crypt);
		break;
	default:
		printf("invalid length code\n");
		exit( 1 );
	}
	return 0;
}
