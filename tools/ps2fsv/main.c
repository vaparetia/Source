/*
	ps2fsv main
		2000/05/17 K.Uehara
		ORIGINAL VERSION	Yoshihito Kira
		$Id: main.c,v 1.6 2000/08/04 07:39:42 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _main_c_
#include "linQio.h"

int verbose_mode = 0;

static char *program_name = "ps2fsv";

static void usage( void )
{
	printf( "usage: %s [options] [root path]\n", program_name );
	printf( "       -p<port>    HUB port No.(0..n)\n" );
	printf( "       -v          verbose mode\n" );
#if 0
	printf( "       -nA			do not output all message\n" );
	printf( "       -nS         do not output server message\n" );
	printf( "       -nC         do not output client message\n" );
#endif
	exit( 1 );
}

static int analyze_option( int argc, char *argv[], int *port )
{
	int i;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			switch( argv[ i ][ 1 ] ){
			  case 'h':
				usage();
				break;
			  case 'p':
				{
					int c;
					c = argv[ i ][ 2 ];
					if( c < '0' || c > '9' ){
						usage();
						break;
					}
					*port = c - '0';
				}
				break;
			  case 'v':
				verbose_mode = 1;
				break;
			}
		} else {
			svSetRootPath( argv[ i ] );
		}
	}
}

int main(int argc, char **argv)
{
	int ret;
	int portno;
	char device_name[ 64 ];

	ret = EXIT_SUCCESS;

	portno = 0;
	svSetRootPath( "." );

	analyze_option( argc, argv, &portno );

	sprintf( device_name, "/dev/usblinq%d", portno );

	if( svMain( device_name ) ){
		ret = EXIT_FAILURE;
	}
	svRelease();   /* サーバ処理で確保したメモリの開放 */

	fprintf(stderr, "server termination.\n");
	return ret;
}

