/*
	capcheck

		capデータを解析しその中の文字列が表示領域からはみ出さないかどうかをチェックする
*/

#include <stdio.h>
#include <stdlib.h>

#include "font.h"

typedef struct {
	int type;
	int size;
	int time;
	int option;
} STREAM_TAG;

typedef struct {
	int start;
	int end;
	int name;
	int len;
	char caption[ 0 ];	// 4 byte align
} CAPTION;

typedef struct {
	int start;
	int name;
	int value;
	int option;
} ACTION;

static int verbose_mode;
static int fontcheck_mode = 0;

static int font_width;
static int font_height;
static int font_c_skip;
static int font_l_skip;
static int font_kinsoku;

#define PRINTF( a, b... )	if( verbose_mode ) { printf( a, ##b ); }

static void usage( void )
{
	printf( "usage: capcheck [-fc <keepfont> <width> <height> <c_skip> <l_skip>] <capfile>...\n" );
	printf( "       only <capfile> : dump cap structure\n" );
	exit( 1 );
}

/* ---------------------------------------------------------------------- */
/*
	caption dump
*/

static int caption_no = 0;

static int dump_caption_structure( void *buffer, int offset )
{
	int size;
	CAPTION *caption, *caption_end;
	ACTION *action;
	void *font_top;
	int over = 0;

	size = *( int * )buffer;
printf( "DATA SIZE = %X\n", size );
	caption = ( CAPTION * )( buffer + sizeof( int ) );
	font_top = ( void * )caption + size;

	if( offset > 0 ){
		action = ( void * )font_top - offset;
		caption_end = ( void * )action;
	} else {
		action = NULL;
		caption_end = font_top;
	}

	while( caption < caption_end ){
		int han_count = 0;
		int zen_count = 0;
		PRINTF( "CAPTION : start %X end %X name %X len %X\n"
				, caption->start, caption->end, caption->name, caption->len );
		{
			int i;
			unsigned char *p = caption->caption;
			for( i = 0; i < caption->len; ){
				if( *p == 0x00 ) break;
				if( *p & 0x80 ){
					int code;
					code = ( *p << 8 ) | *( p + 1 );
					PRINTF( "%04X ", code );
					p += 2;
					i += 2;
					zen_count ++;
				} else {
					PRINTF( "%02X ", *p );
					p++;
					i++;
					han_count ++;
				}
			}
			PRINTF( "%02X\n", *p );
		}
		if( fontcheck_mode ){
			int flag;
			if( ( flag = font_draw_area_check( caption->caption
									  , font_width, font_height
									  , font_c_skip, font_l_skip, font_kinsoku ) ) != 0 ){
				if( flag & FONT_RESULT_OVER ){
					printf( "%d:", caption_no );
					over = 1;
				}
				if( flag & FONT_RESULT_ORIKAESHI ){
					printf( "[%d]:", caption_no );
					over = 1;
				}
			}
//			if( han_count > 0 && han_count / 2 >= zen_count ){
			if( han_count > 0 && zen_count == 0 ){
				printf( "<%d>:", caption_no );
				over = 1;
			}
		}
		caption = ( CAPTION * )( ( void * )caption + caption->len );
		caption_no ++;
	}

	if( action != NULL ){
		while( ( void * )action < font_top ){
			PRINTF( "ACTION : start %X name %X value %X option %X\n"
					, action->start, action->name, action->value, action->option );
			action ++;
		}
	}

	PRINTF( "FONT : size %08X\n", *( int * )font_top );

	return over;
}

static void dump_caption( char *name )
{
	FILE *fp;

	if( ( fp = fopen( name, "rb" ) ) == NULL ){
		printf( "cant open %s\n", name );
		return;
	}

	caption_no = 0;
	for( ;; ){
		STREAM_TAG tag;
		void *buffer;

		if( fread( &tag, sizeof( tag ), 1, fp ) < 1 ){
			break;
		}
		PRINTF( "TAG : TYPE %X SIZE %X TIME %X OPTION %X\n"
				, tag.type, tag.size, tag.time, tag.option );

		buffer = malloc( tag.size - 16 );
		if( buffer == NULL ){
			printf( "no memory\n" );
			exit( 1 );
		}
		fread( buffer, sizeof( char ), tag.size - 16, fp );

		if( dump_caption_structure( buffer, tag.option ) != 0 ){
			printf( " %s\n", name );
		}

		free( buffer );
	}
	fclose( fp );
}

/* ---------------------------------------------------------------------- */
/*
	メインルーチン
*/

int main( int argc, char *argv[] )
{
	int i;
	
	if( argc < 2 ){
		usage();
	}

	i = 1;
	verbose_mode = 1;
	if( argv[ i ][ 0 ] == '-' ){
		if( argv[ i ][ 1 ] == 'f' ){
			char *buffer;
			int size;
			FILE *fp;
			verbose_mode = 0;
			fontcheck_mode = 1;
			
			if( argc < i + 5 + 1){
				usage();
			}
			if( ( fp = fopen( argv[ i + 1 ], "rb" ) ) == NULL ){
				printf( "CANT OPEN %s\n", argv[ i + 1 ] );
				exit( 1 );
			}
			fseek( fp, 0, SEEK_END );
			size = ftell( fp );
			fseek( fp, 0, SEEK_SET );
			buffer = malloc( size );
			if( buffer == NULL ){
				printf( "NO MEMORY\n" );
				exit( 1 );
			}
			fread( buffer, sizeof( char ), size, fp );
			fclose( fp );

			font_resident_load_set( buffer );

			font_width = atoi( argv[ i + 2 ] );
			font_height = atoi( argv[ i + 3 ] );
			font_c_skip = atoi( argv[ i + 4 ] );
			font_l_skip = atoi( argv[ i + 5 ] );
			font_kinsoku = ( argv[ i ][ 2 ] == 'c' ) ? 0 : FONT_NO_KINSOKU;

			i += 6;
		}
	}
	for( ; i < argc; i++ ){
		dump_caption( argv[ i ] );
	}
	return 0;
}

