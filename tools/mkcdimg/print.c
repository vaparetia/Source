#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

/*
   バーポーズモード用
*/

int verbose_mode = 0;

#ifdef _DOS_

static void euc_to_sjis( char *buf )
{
	unsigned char *p;
	p = ( unsigned char * )buf;
	while( *p != '\0' ){
		if( *p & 0x80 ){
			int high, low;
			high = p[ 0 ] - 0x80;
			low = p[ 1 ] - 0x80;
			if( high & 1 ){
				low += 0x1f;
				if( low >= 0x7f ) low++;
			} else {
				low += 0x7e;
			}
			high = ( high - ' ' - 1 ) / 2 + 0x81;
			if( high >= 0xa0 ) high += '@';
			p[ 0 ] = high;
			p[ 1 ] = low;
			p ++;
		} else if( p[ 0 ] == '\\' && p[ 1 ] == 'n' ){
			/* 変換され損なった改行文字 */
			p[ 0 ] = ' ';
			p[ 1 ] = '\n';
			p++;
		}
		p++;
	}
}

static void my_vprintf( char *format, va_list ap )
{
	char buf[ 256 ];

	vsprintf( buf, format, ap );
	euc_to_sjis( buf );
	printf( buf );
}

#define vprintf my_vprintf

#endif

void PRINTF( char *format, ... )
{
	va_list ap;
	va_start( ap, format );
	if( verbose_mode ){
		vprintf( format, ap );
	}
	va_end( ap );
}

void DUMP( char *format, ... )
{
	va_list ap;
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );
}

void ERROR( char *format, ... )
{
	va_list ap;

	print_file_current_pos( "Error" );
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );
	print_current_line();

	exit( 1 );
}

void WARNING( char *format, ... )
{
	va_list ap;

	print_file_current_pos( "Warning" );
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	print_current_line();
}

void FATAL( char *format, ... )
{
	va_list ap;

	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	exit( 1 );
}
