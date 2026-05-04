/*
	zlibによる圧縮・展開テスト
*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

static int total = 0;

static void *al_func( void *op, unsigned int item, unsigned int size )
{
	total = total + item * size;
//	printf( "Alloc %d %d = %d %d\n", item, size, item * size, total );
	return malloc( item * size );
}

static void fr_func( void *op, void *adr )
{
	free( adr );
}

static void usage( void )
{
	printf( "test d|c input output\n" );
	exit( 1 );
}

#define BLOCKSIZE	1024*1024

static void do_compress( FILE *in, FILE *out, int level )
{
	int flush, status;
	z_stream z;
	char *inbuf, *outbuf;
	int size;
	int total;

	fseek( in, 0, SEEK_END );
	total = ftell( in );
	fseek( in, 0, SEEK_SET );

	fwrite( &total, sizeof( int ), 1, out );

	if( ( inbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}
	if( ( outbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}

	z.zalloc = al_func;
	z.zfree = fr_func;
	z.opaque = Z_NULL;

	if( deflateInit( &z, level ) != Z_OK ){
		printf( "deflateInit: %s\n", (z.msg) ? z.msg : "???" );
		return;
	}

	z.avail_in = 0;

	z.next_out = outbuf;
	z.avail_out = BLOCKSIZE;

	flush = Z_NO_FLUSH;
	size = 0;

	for( ;; ){
		if( z.avail_in == 0 ){
			z.next_in = inbuf;
			z.avail_in = fread( inbuf, sizeof( char ), BLOCKSIZE, in );
			size += z.avail_in;
fprintf( stderr, "%d\r", size );
			if( z.avail_in < BLOCKSIZE ){
				flush = Z_FINISH;
			}
		}
		status = deflate( &z, flush );
//printf( "In %d Out %d\n", z.avail_in, z.avail_out );
		if( status == Z_STREAM_END ){
			/* 終了 */
			break;
		}
		if( status != Z_OK ){
			/* error */
			printf( "deflate: %s\n", (z.msg) ? z.msg : "???" );
			return;
		}
		if( z.avail_out == 0 ){
			if( fwrite( outbuf, sizeof( char ), BLOCKSIZE, out ) != BLOCKSIZE ){
				printf( "Write Error\n" );
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
	if( deflateEnd( &z ) != Z_OK ){
		printf( "deflate: %s\n", (z.msg) ? z.msg : "???" );
		exit( 1 );
	}
fprintf( stderr, "              \r" );

	free( inbuf );
	free( outbuf );
}

static void do_decompress( FILE *in, FILE *out )
{
	int status;
	z_stream z;
	char *inbuf, *outbuf;
	int total;

	fread( &total, sizeof( int ), 1, in );
printf( "org size = %d\n", total );

	if( ( inbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}
	if( ( outbuf = malloc( BLOCKSIZE ) ) == NULL ){
		printf( "NO MEMORY\n" );
		return;
	}

	z.zalloc = al_func;
	z.zfree = fr_func;
	z.opaque = Z_NULL;

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
//printf( "In %d Out %d\n", z.avail_in, z.avail_out );
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
	free( inbuf );
	free( outbuf );
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

	if( argc < 5 ){
		level = Z_DEFAULT_COMPRESSION;
	} else {
		level = atoi( argv[ 4 ] );
	}
printf( "%s -> %s\n", argv[ 2 ], argv[ 3 ] );
	switch( argv[ 1 ][ 0 ] ){
	  case 'c':
		do_compress( in, out, level );
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
