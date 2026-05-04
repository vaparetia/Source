/*
	main.c
	$Id: main.c,v 1.2 2001/05/01 08:44:04 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "pss2stream.h"

static char program[] = "pss2stream";

int verbose_mode = 0;

static void usage( void )
{
	printf( "%s [-v] <input pss file> <output stream>\n", program );
	exit( 1 );
}

static char in_filename[ 256 ];
static char out_filename[ 256 ];

#if 0
static int get_arg_filename( char *filename, char **argv )
{
	if( argv[ 0 ][ 2 ] != '\0' ){
		strcpy( filename, &( argv[ 0 ][ 2 ] ) );
		return 0;
	} else {
		strcpy( filename, argv[ 0 + 1 ] );
		argv[ 0 ][ 0 ] = '\0';
		return 1;
	}
}
#endif

int analyze_option( int argc, char *argv[] )
{
	int i;
	int flag = 0;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			/* option */
			switch( argv[ i ][ 1 ] ){
			  case 'h':
				usage();
				break;
			  case 'v':
				verbose_mode = 1;
				break;
			}
		} else {
			if( flag == 0 ){
				strcpy( in_filename, argv[ i ] );
			} else if( flag == 1 ){
				strcpy( out_filename, argv[ i ] );
			} else {
				usage();
			}
			flag ++;
		}
	}
	return flag;
}

int main( int argc, char *argv[] )
{
	FILE *fp, *out;
	char *filename;


	if( analyze_option( argc, argv ) != 2 ){
		usage();
		return 1;
	}

	filename = in_filename;

	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf( "FILE %s not found.\n", filename );
		return 1;
	}

	filename = out_filename;

	if( ( out = fopen( filename, "wb" ) ) == NULL ){
		printf( "FILE %s can not open.\n", filename );
		return 1;
	}

	mkstream( fp, out );

	fclose( fp );
	fclose( out );

	return 0;
}
