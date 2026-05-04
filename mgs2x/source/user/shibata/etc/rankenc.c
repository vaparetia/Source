//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    rankenc.c
    クリアランクエンコード
	2000/11/08 T.Shibata

	$Id: rankenc.c,v 1.1.1.3 2002/11/19 11:48:44 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"


#define	DATA_LENGTH		55
#define SUM_LENGTH		5
#define CRC_LENGTH		6
#define SEED_LENGTH		6
/* 6 Bit の倍数にする */

typedef struct {
	int data;		// 32bitまで
	int len;		// 32まで
} INPUT_DATA;

#define VAL_BITS   (DATA_LENGTH+SUM_LENGTH+CRC_LENGTH)
#define TOTAL_BITS (VAL_BITS+SEED_BITS)

#define ONE_ENC_BITS 6
#define ENC_BYTES	(TOTAL_BITS/ONE_ENC_BITS)
#define SEED_BITS	SEED_LENGTH

#define BITS_MAX	TOTAL_BITS

static int seed_pos[ SEED_BITS + 1 ] = { 6, 16, 21, 24, 46, 59, BITS_MAX };

//static char code[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!?";

static char code[65] = "!$%&*+-:0123456789<=>?@AZCEFGHJKLMNPQRTUVWXY[]^abdefghijkmnqrtyz";

typedef struct {
	char bits[ BITS_MAX ];
	int count;
} BITS_WORK;

static void init_bits_work( BITS_WORK *work )
{
	int i;

	work->count = 0;
	for( i = 0; i < BITS_MAX; i++ ){
		work->bits[ i ] = 0;
	}
}

static void set_bit( BITS_WORK *work, int bit )
{
	work->bits[ work->count ] = bit;
	work->count ++;
}

static void set_bits( BITS_WORK *work, int value, int len )
{
	unsigned int mask;
	int i;

	mask = 1 << ( len - 1 );
	for( i = 0; i < len; i++ ){
		set_bit( work, ( mask & value ) ? 1 : 0 );
		mask = mask >> 1;
	}
}

static int get_bits( BITS_WORK *work, int start, int len )
{
	int i;
	int c;

	c = 0;
	for( i = start; i < start + len; i++ ){
		c = ( c << 1 ) | work->bits[ i ];
	}
	return c;
}

static int get_bits_byte( BITS_WORK *work, int pos )
{
	int i, ep, res;

	if( pos >= work->count ){
		return -1;
	}

	ep = pos + 8;
	if( ep > work->count ){
		ep = work->count;
	}

	res = 0;
	for( i = pos; i < ep; i++ ){
		res = ( res << 1 ) | work->bits[ i ];
	}
	return res;
}

// rand

#define SEED_OFFSET		0x43456345

static unsigned int rand_seed = 1;

int my_rand( void )
{
//	next = next * 1103515245 + 12245;
	rand_seed = 2100005341U * rand_seed + 3321;
	return ( rand_seed >> 16 );
}

// CRC

/* 16 bit CRC 計算 */

//#define CRCPOLY 0xEDB88320
#define CRCPOLY 0x8408

static unsigned int calc_crc_bits( BITS_WORK *work )
{
	unsigned int j;
	unsigned short r;
	int pos;
	
	r = 0xFFFFU;

	pos = 0;

	for( ;; ){
		int c;
		if( ( c = get_bits_byte( work, pos ) ) < 0 ){
			break;
		}
		pos += 8;
		r ^= c;
		for( j = 8; j > 0; j-- ){
			if( r & 1 ){
				r = ( r >> 1 ) ^ CRCPOLY;
			} else {
				r >>= 1;
			}
		}
	}
	r = r ^ 0xFFFFU;
	return ( r >> 8 ) ^ ( r & 0xff );
}

static unsigned int calc_sum_bits( BITS_WORK *work )
{
	int sum;
	int pos;

	sum = 0;
	pos = 0;

	for( ;; ){
		int j, c;

		if( ( c = get_bits_byte( work, pos ) ) < 0 ){
			break;
		}
		pos += 8;

		for( j = 0; j < 2; j++ ){
			sum += ( c & 0x0f );
			sum += ( c >> 4 );
		}
	}
	return ( sum & 0xff );
}

/* ------------------------------------------------------------- */

static void make_table( char *table, int len )
{
	int i;
	int j;

	/* 初期値設定 */
	for( i = 0; i < len; i++ ){
		table[ i ] = i;
	}

	/* シャッフル４回 */
	for( j = 0; j < 4; j++ ){
		for( i = 0; i < len; i++ ){
			int c;
			int t;
			t = my_rand() % len;
			c = table[ t ];
			table[ t ] = table[ i ];
			table[ i ] = c;
		}
	}
}

/* ------------------------------------------------------------- */

static void output_bits( char *result, BITS_WORK *work, int seed, int *pos, int len )
{
	int c, n, j;
	int i, sp, sm;
	char *p;

	p = result;
	c = n = 0;
	j = 0;
	
	sp = 0;
	sm = 1;

	for( i = 0; i < work->count + len; i++ ){
		if( i == pos[ sp ] ){
			sp++;
			c = ( c << 1 ) | ( ( seed & sm ) ? 1 : 0 );
			sm = ( sm << 1 );
		} else {
			c = ( c << 1 ) | work->bits[ j ];
			j++;
		}
		n++;
		if( n == ONE_ENC_BITS ){
			*( p ++ ) = code[ c ];
			n = 0;
			c = 0;
		}
	}
	*p = '\0';
}

/* ------------------------------------------------------------- */

static void swap_bits_enc( BITS_WORK *work )
{
	BITS_WORK dst;
	int i;
	static char table[ BITS_MAX ];

	make_table( table, work->count );

	init_bits_work( &dst );

	for( i = 0; i < work->count; i++ ){
		dst.bits[ (int)table[ i ] ] = work->bits[ i ];
	}
	memcpy( work->bits, dst.bits, work->count * sizeof( char ) );
}

static void swap_bits_dec( BITS_WORK *work )
{
	BITS_WORK dst;
	int i;

	static char table[ BITS_MAX ];

	make_table( table, work->count );

	init_bits_work( &dst );

	for( i = 0; i < work->count; i++ ){
		dst.bits[ i ] = work->bits[ (int)table[ i ] ];
	}
	memcpy( work->bits, dst.bits, work->count * sizeof( char ) );
}

/* ------------------------------------------------------------- */

void encode( char *result, INPUT_DATA *input, int num, int seed )
{
//	unsigned int value;
	int mask;
	int i;
	int crc, sum;
	BITS_WORK Work, *work;

	seed = seed & ( ( 1 << SEED_LENGTH ) - 1 );

	rand_seed = seed + SEED_OFFSET;
	mask = ( my_rand() << 14 ) + my_rand();

	work = &Work;

	init_bits_work( work );

	for( i = 0; i < num; i++ ){
		int data;
		data = input->data ^ mask;
printf( "data = %x %x %x\n", input->data, data, mask );
		set_bits( work, data, input->len );
		input ++;
	}

	sum = calc_sum_bits( work );
	set_bits( work, sum, SUM_LENGTH );
	crc = calc_crc_bits( work );
	set_bits( work, crc, CRC_LENGTH );
printf( "SUM = %x CRC = %x\n", sum, crc );

	swap_bits_enc( work );

	output_bits( result, work, seed, seed_pos, SEED_BITS );
}

/* ------------------------------------------------------------- */

static void get_data( BITS_WORK *work, char *string, int len )
{
	char *p;
	int i, n;
	p = string;
	n = 0;
	for( i = 0; i < len; i++ ){
		char *res;
		if( ( res = strchr( code, *p ) ) != NULL ){
			int c, j, m;

			c = res - code;
			m = ( 1 << ( ONE_ENC_BITS - 1 ) );

			for( j = 0; j < ONE_ENC_BITS; j++ ){
				work->bits[ n ] = ( ( c & m ) ? 1 : 0 );
				m = m >> 1;
				n++;
			}
		} else {
			printf( "ILLEGAL CODE '%c'!!\n", *p );
			exit( 1 );
		}
		p++;
	}
	work->count = n;
}

static int get_enc_bits( BITS_WORK *work, BITS_WORK *org, int *pos, int len )
{
	int i, n, m;
	int seed;
	int sp;
	
	sp = 0;
	seed = 0;
	n = 0;
	m = 1;

	for( i = 0; i < BITS_MAX; i++ ){
		if( i == pos[ sp ] ){
			if( org->bits[ i ] != 0 ){
				seed = seed | m;
			}
			m = m << 1;
			sp ++;
		} else {
			work->bits[ n ] = org->bits[ i ];
			n ++;
		}
	}
	return seed;
}

void decode( INPUT_DATA *input, int num, char *string )
{
	static char data[ ENC_BYTES ];
	int seed;
	int mask;
	BITS_WORK org_bit;
	BITS_WORK Work, *work;

printf( "DECODE:\n" );

	get_data( &org_bit, string, sizeof( data ) );

	work = &Work;

	init_bits_work( work );
	seed = get_enc_bits( work, &org_bit, seed_pos, SEED_BITS );
printf( "SEED %d\n", seed );

	rand_seed = seed + SEED_OFFSET;
	mask = ( my_rand() << 14 ) + my_rand();
printf( "mask = %x\n", mask );
	work->count = VAL_BITS;

	swap_bits_dec( work );
	{
		int crc, sum;
		int _crc, _sum;
		int pos1, pos2;

		int i, pos;

		pos = 0;
		for( i = 0; i < num; i++ ){
			int data;
			data = get_bits( work, pos, input->len );
			input->data = ( data ^ mask ) & ( ( 1 << input->len ) - 1 );
printf( "data = %x len = %d\n", input->data, input->len );
			pos += input->len;
			input ++;
		}

		sum = get_bits( work, pos, SUM_LENGTH );
		pos1 = pos;
		pos += SUM_LENGTH;
		crc = get_bits( work, pos, CRC_LENGTH );
		pos2 = pos;
		pos += CRC_LENGTH;

		printf( "V:CRC %X:SUM %X\n", crc, sum );

		work->count = pos1;
		_sum = calc_sum_bits( work ) & ( ( 1 << SUM_LENGTH ) - 1 );

		work->count = pos2;
		_crc = calc_crc_bits( work ) & ( ( 1 << CRC_LENGTH ) - 1 );

		printf( "C:CRC %X:SUM %X\n", _crc, _sum );
	}
}

/* ------------------------------------------------------------- */
#if 0
static void usage( void )
{
	printf( "Usage:\n" );
	printf( "\trankenc e <score> <flag> <seed>\n" );
	printf( "\trankenc d <code>\n" );
	exit( 1 );
}



static INPUT_DATA input[ 2 ] = {
	{ 0x7BAC0032, 31 },
	{ 0x00164456, 24 },
};

int main( int argc, char *argv[] )
{
	int seed;
	if( argc < 2 ){
		usage();
	}

	seed = 43;
	if( argv[ 1 ][ 0 ] == 'e' ){
		char string[ 64 ];

		encode( string, input, 2, seed );
		printf( "RESULT = %s\n", string );
	} else if( argv[ 1 ][ 0 ] == 'd' ){
		int score;
		int flag;
		decode( input, 2, argv[ 2 ] );
		printf( "1 = %x\n", input[ 0 ].data );
		printf( "2 = %x\n", input[ 1 ].data );
	} else {
		usage();
	}
	return 0;
}
#endif
