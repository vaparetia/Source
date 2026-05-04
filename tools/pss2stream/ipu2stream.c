/*
	ipu2stream.c
	$Id: ipu2stream.c,v 1.2 2002/05/27 06:51:34 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

static float tick_count = 300.0F / 30.0F;	// 2/60

/* ---------------------------------------------------------------------- */
/*
	パケット出力
*/

typedef struct {
	int type;
	int size;
	int time;
	int option;
} STREAM_HEADER;

#define NORM16( a )	( ( (a) + 15 ) & ~15 )

#define ONE_READ_SIZE	1024*512

static void output_chank( FILE *in, int start, int end, int tick, FILE *out )
{
	STREAM_HEADER head;
	int size, pad;
	
	static char buffer[ ONE_READ_SIZE ];

	size = end - start;

	head.type = 0;
	head.size = NORM16( size ) + sizeof( STREAM_HEADER );
	head.time = tick;
	head.option = size;

	pad = NORM16( size ) - size;

	fwrite( &head, sizeof( STREAM_HEADER ), 1, out );
	fseek( in, start, SEEK_SET );

	while( size > 0 ){
		int l;
		l = ( size > ONE_READ_SIZE ) ? ONE_READ_SIZE : size;

		fread( buffer, sizeof( char ), l, in );
		if( fwrite( buffer, sizeof( char ), l, out ) < l ){
			printf( "Write Error\n" );
			exit( 1 );
		}
		size -= l;
	}
	if( pad > 0 ){
		memset( buffer, 0, pad );
		fwrite( buffer, sizeof( char ), pad, out );
	}
}

static void output_head( int tick, void *ptr, int size, FILE *out )
{
	STREAM_HEADER head;
	int pad;
	
	head.type = 0;
	head.size = NORM16( size ) + sizeof( STREAM_HEADER );
	head.time = tick;
	head.option = 0;

	pad = NORM16( size ) - size;

	fwrite( &head, sizeof( STREAM_HEADER ), 1, out );
	if( fwrite( ptr, sizeof( char ), size, out ) < size ){
		printf( "Write Error\n" );
		exit( 1 );
	}
	if( pad > 0 ){
		static char buffer[ 16 ];
		memset( buffer, 0, pad );
		fwrite( buffer, sizeof( char ), pad, out );
	}
}

/* ---------------------------------------------------------------------- */
/*
	IOPストリーム出力
*/

typedef struct {
	int id;
	int size;
	short width;
	short height;
	int nframes;
} IPU_HEADER;

static void mkstream( FILE *in, FILE *out )
{
	/* ヘッダ出力 */
	{
		IPU_HEADER head;
		fread( &head, sizeof( IPU_HEADER ), 1, in );

printf( "FRAMENUM %d WIDTH %d HEIGHT %d SIZE %d\n"
		, head.nframes, head.width, head.height, head.size );

		output_head( 0, &head, sizeof( head ), out );
	}
	/* データ本体出力 */
	{
		int top, now;
		float tick;
		static unsigned char endtag1[] = { 0x00, 0x00, 0x01, 0xb0 };
		static unsigned char endtag2[] = { 0x00, 0x00, 0x01, 0xb1 };
		unsigned char buf[ 4 ] = { 0, 0, 0, 0 };

		top = ftell( in );
		tick = 0.0F;

		for( ;; ){
			int c;
			buf[ 3 ] = c = fgetc( in );
			
			if( c == EOF ){
				printf( "Error: Wrong Format\n" );
				break;
			}
			if( memcmp( buf, endtag1, 4 ) == 0 ){
				now = ftell( in );
				output_chank( in, top, now, ( int )tick, out );
				top = now;
				tick += tick_count;
				fseek( in, now, SEEK_SET );
			} else if( memcmp( buf, endtag2, 4 ) == 0 ){
				break;
			}
			memcpy( buf, buf + 1, 3 );
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	メインルーチン
*/

static char program[] = "ipu2stream";

static void usage( void )
{
	printf( "%s <input ipu file> <output stream> [tick/sec]\n", program );
}

int main( int argc, char *argv[] )
{
	FILE *fp, *out;
	char *filename;

	if( argc < 3 ){
		usage();
		return 1;
	}

	filename = argv[ 1 ];

	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf( "FILE %s not found.\n", filename );
		return 1;
	}

	filename = argv[ 2 ];

	if( ( out = fopen( filename, "wb" ) ) == NULL ){
		printf( "FILE %s can not open.\n", filename );
		return 1;
	}

	if( argc > 3 ){
		float tick;

		tick = atof( argv[ 3 ] );
		tick_count = 300.0 / tick;
	}

	mkstream( fp, out );

	fclose( fp );
	fclose( out );

	return 0;
}

