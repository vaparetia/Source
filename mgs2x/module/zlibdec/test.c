/*
	zlibによる圧縮・展開テスト
*/

#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"

static void usage( void )
{
	printf( "test d|c input output\n" );
	exit( 1 );
}

#define BLOCKSIZE	1024
static void do_decompress( FILE *in, FILE *out )
{
	int status;
	z_stream z;
	char *inbuf, *outbuf;
	char *buffer;

	if( ( inbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}
	if( ( outbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}
#if 0
	z.zalloc = al_func;
	z.zfree = fr_func;
	z.opaque = Z_NULL;
#endif
printf( "size = %d\n", inflateGetWorkSize() );
	z.buffer = malloc( inflateGetWorkSize() );
	z.next_in = Z_NULL;
	z.avail_in = 0;
	if( inflateInit( &z ) != Z_OK ){
		printf( "%s\n", (z.msg) ? z.msg : "???" );
		return;
	}

	z.next_out = outbuf;
	z.avail_out = BLOCKSIZE;
	status = Z_OK;

	while( status != Z_STREAM_END ){
		if( z.avail_in == 0 ){
			z.next_in = inbuf;
			z.avail_in = fread( inbuf, sizeof( char ), BLOCKSIZE, in );
		}
		status = inflate( &z, Z_NO_FLUSH );
		if( status == Z_STREAM_END ){
			/* 終了 */
			break;
		}
		if( status != Z_OK ){
			/* error */
			printf( "%s\n", (z.msg) ? z.msg : "???" );
			exit( 1 );
		}
		if( z.avail_out == 0 ){
			if( fwrite( outbuf, sizeof( char ), BLOCKSIZE, out ) != BLOCKSIZE ){
				printf( "write error\n" );
				exit( 1 );
			}
			z.next_out = outbuf;
			z.avail_out = BLOCKSIZE;
		}
	}

	/* 残りを吐き出す */
	{
		int count;
		count = BLOCKSIZE - z.avail_out;
		if( count > 0 ){
			if( fwrite( outbuf, sizeof( char ), count, out ) != count ){
				printf( "Write Error\n" );
				exit( 1 );
			}
		}
	}
	/* 後始末 */
	if( inflateEnd( &z ) != Z_OK ){
		printf( "inflateEnd: %s\n", (z.msg) ? z.msg : "???" );
		exit( 1 );
	}
}

int main( int argc, char *argv[] )
{
	FILE *in, *out;
	int level;
	
	if( argc < 4 ){
		usage();
	}

	if( ( in = fopen( argv[ 2 ], "rb" ) ) == NULL ){
		printf( "file %s not found\n", argv[ 2 ] );
	}
	if( ( out = fopen( argv[ 3 ], "wb" ) ) == NULL ){
		printf( "file %s not found\n", argv[ 3 ] );
	}

	switch( argv[ 1 ][ 0 ] ){
	  case 'c':
		break;
	  case 'd':
		do_decompress( in, out );
		break;
	  default:
		usage();
	}

	fclose( in );
	fclose( out );

	return 0;
}
