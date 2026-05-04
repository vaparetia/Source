/*
	エンディングのテロップを作成
*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "parse.h"
#include "print.h"
#include "mymalloc.h"

char *program_name = "telopmake";

#define CODE_END	0
#define CODE_BLANK	1
#define CODE_BMP	2
#define CODE_STOP	3
#define CODE_FADE	4
#define CODE_SPEED	5	// ADD M.Kobayashi 2002/08/27

/* ---------------------------------------------------------------------- */
/*
	zlib圧縮関連
*/

#define ZLIB_ENC_BUFFER_SIZE	64*1024

static char zlib_enc_buffer[ ZLIB_ENC_BUFFER_SIZE ];
static z_stream z;

static void zlib_enc_init( void )
{
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	z.next_in = Z_NULL;
	z.avail_in = 0;

	if( deflateInit( &z, Z_DEFAULT_COMPRESSION ) != Z_OK ){
		printf( "%s\n", (z.msg) ? z.msg : "???" );
		return;
	}

	z.next_out = zlib_enc_buffer;
	z.avail_out = ZLIB_ENC_BUFFER_SIZE;
}

static void zlib_write( void *buffer, int size, FILE *fp )
{
	z.next_in = buffer;
	z.avail_in = size;

	for( ;; ){
		int status;
		if( z.avail_in == 0 ){
			break;
		}
		status = deflate( &z, Z_NO_FLUSH );
		if( status != Z_OK ){
			/* ERROR */
			printf( "deflate: %s\n", (z.msg) ? z.msg : "???" );
			return;
		}
		if( z.avail_out == 0 ){
			if( fwrite( zlib_enc_buffer, sizeof( char ), ZLIB_ENC_BUFFER_SIZE, fp )
				!= ZLIB_ENC_BUFFER_SIZE ){
				printf( "Write Error\n" );
				exit( 1 );
			}
			z.next_out = zlib_enc_buffer;
			z.avail_out = ZLIB_ENC_BUFFER_SIZE;
		}
	}
}

static void zlib_flush( FILE *fp )
{
	for( ;; ){
		int status;
		status = deflate( &z, Z_FINISH );
		if( status == Z_STREAM_END ){
			/* 終了 */
			break;
		}
		if( status != Z_OK ){
			/* ERROR */
			printf( "deflate: %s\n", (z.msg) ? z.msg : "???" );
			return;
		}
		if( z.avail_out == 0 ){
			if( fwrite( zlib_enc_buffer, sizeof( char ), ZLIB_ENC_BUFFER_SIZE, fp )
				!= ZLIB_ENC_BUFFER_SIZE ){
				printf( "Write Error\n" );
				exit( 1 );
			}
			z.next_out = zlib_enc_buffer;
			z.avail_out = ZLIB_ENC_BUFFER_SIZE;
		}
	}
	{
		int count;
		count = ZLIB_ENC_BUFFER_SIZE - z.avail_out;
		if( count > 0 ){
			if( fwrite( zlib_enc_buffer, sizeof( char ), count, fp ) != count ){
				printf( "Write Error\n" );
				exit( 1 );
			}
		}
	}
	if( deflateEnd( &z ) != Z_OK ){
		printf( "deflate: %s\n", (z.msg) ? z.msg : "???" );
		exit( 1 );
	}
}

/* ---------------------------------------------------------------------- */
/*
	テロップ作成関連
*/

/*
	ビットマップ
*/

typedef unsigned short	WORD ;
typedef unsigned long	DWORD ;
typedef long	LONG ;

typedef struct tagRGBQUAD {
  unsigned char	b, g, r, alpha ;
} RGBQUAD ;

typedef struct tagRGBTriple {
  unsigned char	b, g, r ;
} RGBTriple ;

typedef struct tagBITMAPINFOHEADER { /* bmih */
   DWORD  biSize;
   LONG   biWidth;
   LONG   biHeight;
   WORD   biPlanes;
   WORD   biBitCount;
   DWORD  biCompression;
   DWORD  biSizeImage;
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
} BITMAPINFOHEADER ;

typedef struct tagBITMAPINFO { /* bmi */
   BITMAPINFOHEADER bmiHeader ;
   RGBQUAD          bmiColors[1];
} BITMAPINFO;

typedef struct tagBITMAPINFO2 { /* bmi2 */
   BITMAPINFOHEADER bmiHeader ;
   RGBTriple        bmiColors[ 1 ] ;
} BITMAPINFO2 ;

typedef struct tagBITMAPFILEHEADER { /* bmfh */
  WORD	  padding ;
  WORD    bfType;
  DWORD   bfSize;
  WORD    bfReserved1;
  WORD    bfReserved2;
  DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPHEADER {
  BITMAPFILEHEADER 	header ;
  BITMAPINFOHEADER	info ;
} BITMAPHEADER ;

static void output_bitmap( FILE *out, char *bitmap )
{
	FILE *fp;
	BITMAPHEADER bmphead;
	static RGBQUAD quat[ 16 ];
	static unsigned int clut[ 16 ];

	if( ( fp = fopen( bitmap, "rb" ) ) == NULL ){
		printf( "not found %s\n", bitmap );
		exit( 1 );
	}
	fread( &( bmphead.header.bfType ), sizeof( BITMAPHEADER ) - 2, 1, fp );
#if 0
printf( "BITMAP HEADER\n" );
printf( "type = %X\n", bmphead.header.bfType );
printf( "size = %X\n", bmphead.header.bfSize );
printf( "ofbits = %X\n", bmphead.header.bfOffBits );

printf( "size %d\n", bmphead.info.biSize ) ;
printf( "width %d\n", bmphead.info.biWidth ) ;
printf( "height %d\n", bmphead.info.biHeight ) ;
printf( "planes %d\n", bmphead.info.biPlanes ) ;
printf( "bitcount %d\n", bmphead.info.biBitCount ) ;
printf( "compression %d\n", bmphead.info.biCompression ) ;
printf( "sizeimage %d\n", bmphead.info.biSizeImage ) ;
printf( "clrused %d\n", bmphead.info.biClrUsed ) ;
#endif
	if( bmphead.info.biBitCount != 4 ){
		printf( "ONLY 4 BIT TEXTURE: %d\n", bmphead.info.biBitCount );
		return;
//		exit( 1 );
	}

	/* パレット読み込み */

    fseek( fp, sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) - 2, SEEK_SET ) ;
	fread( quat, sizeof( RGBQUAD ), 16, fp );

	{
		int i;
	for( i = 0; i < 16; i++ ){
			clut[ i ] =
				( quat[ i ].alpha << 24 )
				| ( quat[ i ].b << 16 )
				| ( quat[ i ].g << 8 )
				| ( quat[ i ].r << 0 );
		}
	}

printf( "FILENAME %s : height %ld\n", bitmap, bmphead.info.biHeight );
	/* ヘッダ情報出力 */
	{
		short code, width, height;
		code = CODE_BMP;
		width = bmphead.info.biWidth;
		height = bmphead.info.biHeight;

		fputc( code, out );
		fwrite( &width, sizeof( short ), 1, out );
		fwrite( &height, sizeof( short ), 1, out );
	}

	/* 32Bitテクスチャへの変換 */
	{
		unsigned int *linebuf;
		unsigned char *readbuf;
		int total;
		int i;
		int width, height;
		unsigned char *base;

		width = bmphead.info.biWidth;
		height = bmphead.info.biHeight;

		linebuf = my_malloc( sizeof( unsigned int ) * bmphead.info.biWidth );
		total = width * height * sizeof( char ) / 2 ;
		readbuf = my_malloc( total );

	    fseek( fp, bmphead.header.bfOffBits, SEEK_SET ) ;
		fread( readbuf, sizeof( char ), total, fp );

		base = readbuf + width * ( height - 1 ) * sizeof( char ) / 2;

		zlib_enc_init();

		for( i = 0; i < bmphead.info.biHeight; i++ ){
			int j;
			unsigned int *p;

			p = linebuf;
			for( j = 0; j < bmphead.info.biWidth / 2; j++ ){
				p[ 0 ] = clut[ ( base[ j ] >> 4 ) & 0x0F ];
				p[ 1 ] = clut[ ( base[ j ] & 0x0F ) ];
				p += 2;
			}
			zlib_write( linebuf, sizeof( int ) * bmphead.info.biWidth, out );
			base -= width * sizeof( char ) / 2;
		}
		zlib_flush( out );
	}
}

static void output_control( FILE *out, int code, int pixel )
{
	fputc( code, out );
	fwrite( &pixel, sizeof( int ), 1, out );
}

static void make_telop( FILE *out )
{
	while( get_line() != 0 ){
		WORDBUF word;

		if( get_word( word ) == NULL ){
			continue;
		}

		if( strcmp( word, "bmp" ) == 0 ){
			// BMP出力
			if( get_word( word ) == NULL ){
				printf( "ファイル名を指定して下さい\n" );
				exit( 1 );
			}
			output_bitmap( out, word );
		} else if( strcmp( word, "blank" ) == 0 ){
			// blank 出力
			int pixel;
			if( get_word( word ) == NULL ){
				printf( "ピクセル数を指定して下さい\n" );
				exit( 1 );
			}
			pixel = atoi( word );
			output_control( out, CODE_BLANK, pixel );
		} else if( strcmp( word, "stop" ) == 0 ){
			int fm;
			if( get_word( word ) == NULL ){
				printf( "フレーム数を指定して下さい\n" );
				exit( 1 );
			}
			fm = atoi( word );
			output_control( out, CODE_STOP, fm );
		} else if( strcmp( word, "fade" ) == 0 ){
			int step;
			if( get_word( word ) == NULL ){
				printf( "ステップ数を指定して下さい\n" );
				exit( 1 );
			}
			step = atoi( word );
			output_control( out, CODE_FADE, step );
		} else if( strcmp( word, "speed" ) == 0 ){
			int speed;
			if( get_word( word ) == NULL ){
				printf( "スクロールスピードを指定して下さい\n" );
				exit( 1 );
			}
			speed = atoi( word );
			output_control( out, CODE_SPEED, speed );
		}
	}
	fputc( CODE_END, out );
}

/* ---------------------------------------------------------------------- */
/*
	main
*/

int main( int argc, char *argv[] )
{
	char *scrfile;
	char *outfile;
	FILE *fp;
	
	if( argc < 3 ){
		printf( "telopmake <scr> <output>\n" );
		exit( 1 );
	}

	scrfile = argv[ 1 ];
	outfile = argv[ 2 ];

	parse_init();

	if( !load_file( scrfile ) ){
		ERROR( "Can't open scrfile\n" );
	}

	if( ( fp = fopen( outfile, "wb" ) ) == NULL ){
		ERROR( "Can't open output file\n" );
	}

	make_telop( fp );

	fclose( fp );

	return 0;
}
