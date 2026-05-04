#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
	int i;

	if( argc < 2 ){
		printf( "   +0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F\n" );
		for( i = 0x20; i < 0x80; i++ ){
			if( i % 16 == 0 ){
				printf( "%02X:", i );
			}
			printf( " %c", i );
			if( i % 16 == 15 ){
				printf( "\n" );
			}
		}
	} else {
		int code, max;

		code = strtol( argv[ 1 ], NULL, 16 );
		code = code & ~0xF;

		if( code < 0xa100 ){
			printf( "EUC is 0xA1A1 - 0xFEFE\n" );
			return 0;
		}

		if( argc == 2 ){
			max = 0x100;
		} else {
			int code2;
			code2 = strtol( argv[ 2 ], NULL, 16 );
			code2 = ( code2 + 15 ) & ~0xF;
			max = code2 - code;
		}

		printf( "     +0+1+2+3+4+5+6+7+8+9+A+B+C+D+E+F\n" );
		for( i = 0x00; i < max; i++ ){
			unsigned char buf[ 3 ] = "  ";
			if( i % 16 == 0 ){
				if( ( ( code + i ) & 0xF0 ) < 0xA0 ){
					i += 0xA0 - ( ( code + i ) & 0xF0 );
				}
				printf( "%04X:", code + i );
			}
			buf[ 0 ] = ( code + i ) >> 8;
			buf[ 1 ] = ( code + i ) & 0xFF;
			if( buf[ 0 ] < 0xA1 || buf[ 0 ] > 0xFE
				|| buf[ 1 ] < 0xA1 || buf[ 1 ] > 0xFE ){
				buf[ 0 ] = buf[ 1 ] = ' ';
			}
			printf( buf );
			if( i % 16 == 15 ){
				printf( "\n" );
			}
		}
	}
	return 0;
}
