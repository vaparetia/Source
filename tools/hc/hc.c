/*
	HC main
	$Id: hc.c,v 1.7 2001/04/10 01:55:22 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "parse.h"
#include "expr.h"

char *program_name = "hc";
extern int set_line_buffer( char *line_buf );

static void usage( void )
{
	printf( "usage: %s <calc string>\n", program_name );
	printf( " calculate commandline.( numerical expression or MGS2 strcode )\n" );
	printf( " hc - is read strings from stdin\n" );
	printf( " ver. 0.01 %s by K.Uehara\n", __DATE__ );
	exit( 1 );
}

static char *int2bin( int value )
{
	static char buf[ 256 ];

	char *p = buf;
	int i;
	unsigned int mask = 0x80000000;

	for( i = 0; i < 32; i++ ){
		if( value & mask ){
			*p = '1';
		} else {
			*p = '0';
		}
		p++;
		if( i == 31 ){
			*p = '\0';
		} else if( i % 8 == 7 ){
			*p = '_';
			p++;
		}
		mask = mask >> 1;
	}
	return buf;
}

static void calc( void )
{
	EXPR_INFO info, *ep;
	OPERAND *op;
	int value;
	int type;

	ep = &info;
	init_expr( ep, ')' );
	op = set_expr( ep );

	type = calc_expr( ep, op, &value );
	if( type == 1 ){
		/* 定数になった */
		printf( "ans. %d ( hex 0x%08X : float %g )\n0b%s\n", value, value, *(float*)&value, int2bin( value ) );
	} else if( type == 2 ){
		printf( "float %f ( 0x%08X )\n", *( float * )&value, value );
	} else {
		ERROR( "式がおかしい\n" );
	}
	close_expr( ep );
}

static char comline_buf[ 1024 ];

int main( int argc, char *argv[] )
{
	int i;
#if 0
	extern int verbose_mode;
	verbose_mode = 1;
#endif
	if( argc < 2 ){
		usage();
	}

	if( strcmp( argv[ 1 ], "-" ) == 0 ){
		fgets( comline_buf, 1023, stdin );
	} else {
		comline_buf[ 0 ] = '\0';
		for( i = 1; i < argc; i++ ){
			if( strlen( comline_buf ) + strlen( argv[ i ] ) + 1 >= 1000 ){
				printf( "式が長過ぎます\n" );
				return 1;
			}
			strcat( comline_buf, argv[ i ] );
			strcat( comline_buf, " " );
		}
		strcat( comline_buf, " )\n" );
	}
PRINTF( "CALC %s\n", comline_buf );
	parse_init();
	parse_init_define();

	set_line_buffer( comline_buf );
	parse_change_mode( PARSE_EXPR_MODE );

	calc();

	parse_free_define();
	parse_end();

	return 0;
}
