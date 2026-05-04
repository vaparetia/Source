/*
   gclconv
   文字列エンコードルーチン
   本体のプログラムと同じアルゴリズム。

   MGS2用に24Bit Strcode
*/

#include <stdio.h>
#include <stdlib.h>

#include "gclconv.h"

long dump_strid_flag = 0;
long dump_strid_code;

#ifdef STRCODE16
#define BIT_LEN		16
#else
#define BIT_LEN		24
#endif

long get_strcode( char *str )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )str;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	while( ( c = *( p++ ) ) != '\0' ){
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	if( dump_strid_flag ){
		if( !( dump_strid_flag == 2 && dump_strid_code != id ) ){
			print_file_current_pos( "strcheck" );
			printf( "%08X \"%s\"\n", id, str );
		}
	}

	return ( long )id;
}
