#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


static void usage( void )
{
	printf( "tolower <file> ...\n" );
	printf( "  filename lower case.\n" );
	exit( 1 );
}

static void ToLower( char *buf, char *in )
{
	char *p, *d;

	d = buf;
	for( p = in; *p != '\0'; p++, d++ ){
		*d = tolower( *p );
	}
	*d = '\0';
}

int main( int argc, char *argv[] )
{
	long i;

	if( argc == 1 ){
		usage();
	}

	for( i = 1; i < argc; i++ ){
		char buf[ 256 ];

		if( argv[ i ][ 0 ] == '-' ){
			usage();
			break;
		}

		ToLower( buf, argv[ i ] );
		if( strcmp( buf, argv[ i ] ) != 0 ){
			char com[ 256 ];
			printf( "convert %s -> %s\n", argv[ i ], buf );
			sprintf( com, "mv %s %s", argv[ i ], buf );
			system( com );
		}
	}
	return 0;
}
