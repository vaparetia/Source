/*
  本編エンコーダ
  
  2002/08/20 M.Kobayashi
  $Id: encode.c,v 1.1 2002/08/21 02:09:13 usr03700 Exp $
  
*/

#include	<stdio.h>
#include	"clearcode.h"
#include	<stdlib.h>

#define	CODE_LENGTH	28
#define SRC_BIT_LEN	110

char *program_name = "encode_sub";

void usage( void )
{
	printf( "encode_sub code1 code2 code3 code4\n" );
	exit( 1 );
}

int main ( int argc, char* argv[] )
{	
	u_char src[ 16 ];
	u_char dst[ CODE_LENGTH * 2];
	u_char dstchar[ CODE_LENGTH + 1];
	u_char dstchar2[ CODE_LENGTH + 1 + 4 ];
	int i;

	if( argc != 5 ) usage();

	for( i = 0 ; i < 4; i++ ) {
		u_long code;
		code = strtoul( argv[ i + 1 ], NULL, 16 );
		src[ i * 4 + 0 ] =  code & 0xff;
		src[ i * 4 + 1 ] = ( code >> 8 ) & 0xff;
		src[ i * 4 + 2 ] = ( code >> 16 ) & 0xff;
		src[ i * 4 + 3 ] = ( code >> 24 ) & 0xff;
	}

	if( CODE_LENGTH != ClearCodeGenerate( dst, CODE_LENGTH, 26, 
										  (void*)src, SRC_BIT_LEN, 0 ) ) {
		printf("%d", ClearCodeGenerate( dst, CODE_LENGTH, 26, (void*)src, SRC_BIT_LEN, 0 ));
		printf("encode error\n");
		exit( 1 );
	}
	
	for( i = 0 ; i < CODE_LENGTH ; i++ ) {
		dstchar[ i ] = dst[ i ] + 'A';
		dstchar2[ i * 5 / 4 ] = dst[ i ] + 'A';
		dstchar2[ i * 5 / 4 + 1 ] = ' ';
	}
	dstchar[ i ] = '\0';
	dstchar2[ i * 5 / 4 ] = '\0';
	
	printf("%s|%s\n", dstchar2, dstchar );

	return 0;
}


