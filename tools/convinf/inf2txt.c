/*
	inf2txt.c
		infファイルをtxt形式で出力
	2001/06/27	K.Uehara
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mymalloc.h"
#include "infdata.h"

/* ---------------------------------------------------------------------- */
/*
	ローカルサブルーチン
*/

static int get_int( void *ptr )
{
	unsigned char *buf = ptr;

	return ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
}

#define TICK( a ) a
#define PRINTF	printf

/* ---------------------------------------------------------------------- */
/*
	各データの解釈
*/

static int load_inf_file_header( void *data )
{
	INF_FILE_HEADER *hp = data;
	int version;

	version = get_int( &hp->version );

	printf( "%s:%d\n", INF_FILE_HEADER_ID, version );

	return 1;
}

static int load_inf_file_fileinf( void *data )
{
	INF_FILE_FILEINF *hp = data;

	printf( "%s:{\n", INF_FILE_FILEINF_ID );
	printf( "%s", hp->name );
	printf( "\n}\n" );

	return 1;
}

static int load_inf_file_hists( void *data )
{
	INF_FILE_HISTS *hp = data;
	int hist_max;
	int change_count;
	int i, cr = 0;

	hist_max = get_int( &hp->hist_max );
	change_count = get_int( &hp->change_count );

	printf( "%s:%d,%d,{\n", INF_FILE_HISTS_ID, hist_max, change_count );
	for( i = 0; i < change_count; i++ ){
		printf( "%d,", get_int( &( hp->changes[ i ] ) ) );
		cr = 1;
		if( i % 16 == 15 ){
			printf( "\n" );
			cr = 0;
		}
	}
	if( cr ){
		printf( "\n" );
	}
	printf( "}\n" );

	return 1;
}

static int load_inf_file_fileanm( void *data )
{
	INF_FILE_FILEANM *hp = data;

	printf( "%s:{\n", INF_FILE_FILEANM_ID );
	printf( "%s", hp->name );
	printf( "\n}\n" );

	return 1;
}

static int load_inf_talk_inf( void *data )
{
	INF_TALK_INF *hp = data;
	int userref;

	userref = get_int( &hp->userref );

	printf( "%s:%d,{\n", INF_TALK_INF_ID, userref );
	printf( "%s", hp->name );
	printf( "\n}\n" );

	return 1;
}

static int load_inf_talk_area( void *data )
{
	INF_TALK_AREA *hp = data;
	int pos, duration;

	pos = TICK( get_int( &hp->pos ) );
	duration = TICK( get_int( &hp->duration ) );

	printf( "%s: %d, %d\n", INF_TALK_AREA_ID, pos, duration );

	return 1;
}

static int load_inf_talk_act( void *data )
{
	INF_TALK_ACT *hp = data;
	int pos, duration, attr, param1, param2;

	pos = TICK( get_int( &hp->pos ) );
	duration = TICK( get_int( &hp->duration ) );
	attr = get_int( &hp->attr );
	param1 = get_int( &hp->param1 );
	param2 = get_int( &hp->param2 );


	printf( "%s: %d, %d, %d, %d, %d, {\n", INF_TALK_ACT_ID
			, pos, duration, attr, param1, param2 );
	printf( "%s", hp->name );
	printf( "\n}\n" );

	return 1;
}

static int load_inf_talk_dialog( void *data )
{
	INF_TALK_DIALOG *hp = data;

	int pos;
	int duration;
	char *name;
	char *str;

	pos = TICK( get_int( &hp->pos ) );
	duration = TICK( get_int( &hp->duration ) );

	name = hp->name;
	str = hp->name + strlen( hp->name ) + 1;

	printf( "%s: %d, %d, {\n", INF_TALK_DIALOG_ID, pos, duration );
	printf( "%s", name );
	printf( "\n},{\n" );
	printf( "%s", str );
	printf( "\n}\n" );

	return 1;
}

static int load_inf_talk_block( void *data )
{
	INF_TALK_BLOCK *hp = data;

	int pos;
	int duration;
	int block_id;
	int tm;

	pos = TICK( get_int( &hp->pos ) );
	duration = TICK( get_int( &hp->duration ) );
	block_id = get_int( &hp->block_id );
	tm = get_int( &hp->tm );

	printf( "%s: %d %d %d %d\n", pos, duration, block_id, tm );

	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	解釈部メイン
*/

typedef int ( *TAGFUNC )( void *data );

typedef struct {
	char *idstr;
	TAGFUNC func;
} LOAD_TAG_TABLE;

static LOAD_TAG_TABLE *search_tag( char *tag, LOAD_TAG_TABLE *table )
{
	LOAD_TAG_TABLE *tp;

	for( tp = table; tp->idstr != NULL; tp++ ){
		if( strncmp( tag, tp->idstr, IDTAG_SIZE ) == 0 ){
			return tp;
		}
	}
	return NULL;
}

int load_inf_file( FILE *fp )
{
	static LOAD_TAG_TABLE table[] = {
		{ INF_FILE_HEADER_ID, load_inf_file_header },
		{ INF_FILE_FILEINF_ID, load_inf_file_fileinf },
		{ INF_FILE_HISTS_ID, load_inf_file_hists },
		{ INF_FILE_FILEANM_ID, load_inf_file_fileanm },
		{ INF_TALK_INF_ID, load_inf_talk_inf },
		{ INF_TALK_AREA_ID, load_inf_talk_area },
		{ INF_TALK_ACT_ID, load_inf_talk_act },
		{ INF_TALK_DIALOG_ID, load_inf_talk_dialog },
		{ INF_TALK_BLOCK_ID, load_inf_talk_block },
		{ NULL, NULL }
	};

	while( !feof( fp ) ){
		INF_BLOCK tag;
		int block_size;
		LOAD_TAG_TABLE *tp;

		if( fread( &tag, sizeof( tag ), 1, fp ) < 1 ){
			break;
		}

		block_size = get_int( &tag.block_size );

		if( ( tp = search_tag( ( char * )&tag.id, table ) ) != NULL ){
			int res;
			void *data;

			if( ( data = malloc( block_size ) ) == NULL ){
				PRINTF( "NO MEMORY\n" );
				exit( 1 );
			}
			memcpy( data, &tag, sizeof( tag ) );
			fread( data + sizeof( tag ), sizeof( char ), block_size - sizeof( tag ), fp );

			res = ( *tp->func )( data );

			free( data );

			if( !res ){
				PRINTF( "ERROR : %s\n", tp->idstr );
				return 0;
			}
		} else {
			PRINTF( "ERROR : %s\n", ( char * )&tag.id );
			return 0;
		}
	}
	return 1;
}

int main( int argc, char *argv[] )
{
	FILE *fp;

	if( argc < 2 ){
		printf( "inf2txt <inffile>\n" );
		return 1;
	}

	if( ( fp = fopen( argv[ 1 ], "rb" ) ) == NULL ){
		printf( "Can't open %s\n", argv[ 1 ] );
		return 1;
	}
	load_inf_file( fp );

	fclose( fp );

	return 0;
}
