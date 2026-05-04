/*
	convhd¥á¥¤¥ó
*/

#include <stdio.h>

extern void convert_hd( char *infile, char *outfile );

int main( int argc, char *argv[] )
{
	if( argc < 3 ){
		printf( "need 2 args\n" );
		return 1;
	}

	convert_hd( argv[ 1 ], argv[ 2 ] );
	return 0;
}
