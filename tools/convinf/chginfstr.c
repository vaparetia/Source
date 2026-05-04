/*
	chginfstr.c
		infファイル中の文字列を変換
	2001/06/27	K.Uehara
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "mymalloc.h"
#include "infdata.h"


/* ---------------------------------------------------------------------- */
/*
	スクリプト解釈
*/

#define MAX_REPLACE 16

#define TYPE_NAME	1
#define TYPE_STR	2

typedef struct {
	int type;
	char *old;
	char *new;
} REPLACE_TYPE;

static REPLACE_TYPE reps[ MAX_REPLACE ];
static int reps_p = 0;

static char *get_token( char *dest, char *src )
{
	char *p = src;

	while( isspace( *p ) && *p != '\0' ){
		p++;
	}

	while( !isspace( *p ) && *p != '\0' ){
		*( dest ++ ) = *( p ++ );
	}
	*( dest ++ ) = '\0';

	if( *p == '\0' ){
		return NULL;
	}
	return p;
}

static void read_script( char *fname )
{
	char linebuf[ 1024 ];
	char *p;
	FILE *fp;

	if( ( fp = fopen( fname, "rt" ) ) == NULL ){
		printf( "File %s Open Error\n", fname );
		exit( 1 );
	}

	while( !feof( fp ) ){
		char word[ 256 ];
		int type = 0;
		if( fgets( linebuf, 1024, fp ) == NULL ){
			break;
		}
		if( linebuf[ 0 ] == '#'
			|| linebuf[ 0 ] == '\n'
			|| linebuf[ 0 ] == '\r'
			|| linebuf[ 0 ] == '/' ){
			continue;
		}
		p = linebuf;

		p = get_token( word, p );
		if( p == NULL ){
			printf( "NEED STRING\n" );
			exit( 1 );
		}
		if( strcmp( word, "name" ) == 0 ){
			type = TYPE_NAME;
		} else if( strcmp( word, "str" ) == 0 ){
			type = TYPE_STR;
		} else {
			printf( "Error: %s is not key.\n", word );
		}

		p = get_token( word, p );
		if( p == NULL ){
			printf( "NEED STRING\n" );
			exit( 1 );
		}

		if( reps_p >= MAX_REPLACE ){
			printf( "Too Many Replace( MAX %d )\n", MAX_REPLACE );
			exit( 1 );
		}

		reps[ reps_p ].type = type;
		reps[ reps_p ].old = strdup( word );

		p = get_token( word, p );
		reps[ reps_p ].new = strdup( word );

		reps_p ++;
	}
}

static char target[ 1024 ];

static char *get_replace( int type, char *old )
{
	char *new;
	int old_len;
	char *p;

	old_len = strlen( old ) + 1;
	strcpy( target, "" );

	new = NULL;
	p = old;

	for( ;; ){
		int i;
		int n;
		char *find;

		n = -1;
		find = old + old_len;

		for( i = 0; i < reps_p; i++ ){
			char *pp;
			if( reps[ i ].type == type ){
				if( ( pp = strstr( p, reps[ i ].old ) ) != NULL ){
					if( find > pp ){
						find = pp;
						n = i;
					}
				}
			}
		}
		if( find > p ){
			strncat( target, p, ( find - p ) );
		}
		if( n < 0 ){
			/* HIT しなかった */
			break;
		}
		strcat( target, reps[ n ].new );
		p += strlen( reps[ n ].old );
	}

	return target;
}

/* ---------------------------------------------------------------------- */
/*
	ローカルサブルーチン
*/

static int get_int( void *ptr )
{
	unsigned char *buf = ptr;

	return ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
}

static void put_int( void *ptr, int value )
{
	unsigned char *buf = ptr;
	buf[ 0 ] = ( value >> 24 );
	buf[ 1 ] = ( value >> 16 );
	buf[ 2 ] = ( value >> 8 );
	buf[ 3 ] = ( value >> 0 );
}

#define TICK( a ) a
#define PRINTF	printf

FILE *out_fp;

static void out_block( void *data, int size )
{
	if( out_fp != NULL ){
		if( fwrite( data, sizeof( char ), size, out_fp ) < size ){
			printf( "Write Error\n" );
			exit( 1 );
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	各データの解釈
*/

static int load_inf_file_header( void *data, int size )
{
	INF_FILE_HEADER *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_file_fileinf( void *data, int size )
{
	INF_FILE_FILEINF *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_file_hists( void *data, int size )
{
	INF_FILE_HISTS *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_file_fileanm( void *data, int size )
{
	INF_FILE_FILEANM *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_talk_inf( void *data, int size )
{
	INF_TALK_INF *hp = data;

	char *new_name;

	/* 発言者情報 */

	new_name = get_replace( TYPE_NAME, hp->name );
	if( strcmp( new_name, hp->name ) != 0 ){
		int add_len;
		int newsize;
		INF_TALK_INF *new;
		add_len = strlen( new_name ) - strlen( hp->name );
		newsize = size;
		if( add_len > 0 ){
			add_len = ( add_len + 3 ) & ~3;
			newsize = size + add_len;
		}
		new = malloc( newsize );
		memcpy( new, hp, size );
		put_int( &new->block_size, newsize );
		strcpy( new->name, new_name );
		out_block( new, newsize );
		free( new );
	} else {
		out_block( hp, size );
	}

	return 1;
}

static int load_inf_talk_area( void *data, int size )
{
	INF_TALK_AREA *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_talk_act( void *data, int size )
{
	INF_TALK_ACT *hp = data;

	out_block( hp, size );

	return 1;
}

static int load_inf_talk_dialog( void *data, int size )
{
	INF_TALK_DIALOG *hp = data;

	/* 字幕文字列情報 */

	out_block( hp, size );

	return 1;
}

static int load_inf_talk_block( void *data, int size )
{
	INF_TALK_BLOCK *hp = data;

	out_block( hp, size );

	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	解釈部メイン
*/

typedef int ( *TAGFUNC )( void *data, int size );

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

			res = ( *tp->func )( data, block_size );

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

static void usage( void )
{
	printf( "chginfstr <script> <input_inf> <output_inf>\n" );
	printf( "scriptは、\n"
			"type old_str new_str\n"
			"のShift-JISファイル。\n"
			"typeは、name or str\n" );
}

int main( int argc, char *argv[] )
{
	FILE *fp;

	if( argc < 4 ){
		usage();
		return 1;
	}

	read_script( argv[ 1 ] );

	if( ( fp = fopen( argv[ 2 ], "rb" ) ) == NULL ){
		printf( "Can't open %s\n", argv[ 2 ] );
		return 1;
	}
	if( ( out_fp = fopen( argv[ 3 ], "wb" ) ) == NULL ){
		printf( "Can't open %s\n", argv[ 3 ] );
		return 1;
	}
	
	load_inf_file( fp );

	fclose( fp );

	return 0;
}
