/*
	infload.c
		infファイルをロードする
	2000/08/04	K.Uehara
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mymalloc.h"
#include "infdata.h"

extern int verbose_mode;

int inf_conv_euc_flag = 0;

#define PRINTF( fmt, arg... )	if( verbose_mode ) printf( fmt, ## arg )

void sjis_to_euc( char *to, char *str )
{
	unsigned char *p, *s;

	p = ( unsigned char * )str;
	s = ( unsigned char * )to;
	while( *p != '\0' ){
		if( *p > 0x80 ){
			long h, l;
			h = *p;
			l = *( p + 1 );

			if( h >= 0xa0 + '@' ){
				h -= '@';
			}
			h = ( h - 0x81 ) * 2 + 1 + ' ' + 0x80;

			if( l >= 0x9f ){
				h ++;
				l = l - 0x9f + 0xa1;
			} else {
				if( l > 0x7f ){
					l --;
				}
				l = l - 0x40 + 0xa1;
			}
			*s = h;
			*( s + 1 ) = l;
			p += 2;
			s += 2;
		} else {
			*s = *p;
			p++;
			s++;
		}
	}
	*s = '\0';
}

/* ---------------------------------------------------------------------- */
/*
	内部データ登録／検索
*/

extern long get_strcode( char *str );

#define TICK( a )	( int )( (a) * 300.0 / ( 22050.0 / 1024.0 ) )

/* 共通データタグ構造 */

static COMMON_TAG	area = { NULL };
static COMMON_TAG	dialog = { NULL };
static COMMON_TAG	action = { NULL };

static void *add_tag_last( COMMON_TAG *top, void *add )
{
	COMMON_TAG *p;
	COMMON_TAG *tp = add;

	for( p = top; p->next != NULL; p = p->next ){
		;
	}
	tp->next = NULL;
	p->next = tp;

	return tp;
}

static void *add_tag_pos( COMMON_TAG *top, void *add )
{
	typedef struct {
		void *next;
		int start;
	} TALK_COMMON;
	TALK_COMMON *p;
	TALK_COMMON *tp = add;

	for( p = ( TALK_COMMON * )top; p->next != NULL; p = p->next ){
		TALK_COMMON *next = p->next;
		if( next->start > tp->start ){
			break;
		}
	}
	tp->next = p->next;
	p->next = tp;

	return tp;
}

/* エリアタグ作成 */

static void *new_area( int name, int start, int end, char *string )
{
	TALK_AREA *new;

	new = Malloc( sizeof( TALK_AREA ) );
	new->name = name;
	new->start = start;
	new->end = end;
	new->string = Malloc( strlen( string ) + 1 );
	sjis_to_euc( new->string, string );

	return add_tag_last( &area, new );
}

/* ダイアログタグ生成 */

static void *new_dialog( int name, int start, int end, char *string )
{
	TALK_DIALOG *new;

	new = Malloc( sizeof( TALK_DIALOG ) );
	new->name = name;
	new->start = start;
	new->end = end;
	new->string = Malloc( strlen( string ) + 1 );
	if( inf_conv_euc_flag ){
		sjis_to_euc( new->string, string );
	} else {
		strcpy( new->string, string );
	}
	return add_tag_last( &dialog, new );
}

/* アクション作成 */

static void *new_action( int name, int start, int value )
{
	TALK_ACTION *new;

	new = Malloc( sizeof( TALK_ACTION ) );
	new->start = start;
	new->name = name;
	new->value = value;

	return add_tag_pos( &action, new );
}

/* ダイアログタグの検索用 */

TALK_DIALOG *get_dialog_top( void )
{
	return ( TALK_DIALOG * )dialog.next;
}

TALK_DIALOG *get_next_dialog( TALK_DIALOG *now )
{
	return ( TALK_DIALOG * )( ( ( COMMON_TAG * )now )->next );
}

/* ある時間領域がどのエリアにあるかを検索する */

TALK_AREA *get_talk_area( int start, int end )
{
	TALK_AREA *ap;
	int d1, w1;
//PRINTF( "dlog %d %d\n", start, end );
	d1 = start + end;
	w1 = end - start;

	for( ap = ( TALK_AREA * )area.next; ap != NULL;
		 ap = ( TALK_AREA * )( ( COMMON_TAG * )ap )->next ){
		int d, w;
		d = abs( ap->start + ap->end - d1 );
		w = ap->end - ap->start + w1;
		if( d <= w ){
			return ap;
		}
	}
	return NULL;
}

TALK_ACTION *get_action_top( void )
{
	return ( TALK_ACTION * )action.next;
}

TALK_ACTION *get_next_action( TALK_ACTION *now )
{
	return ( TALK_ACTION * )( ( ( COMMON_TAG * )now )->next );
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

/* ---------------------------------------------------------------------- */
/*
	各データの解釈
*/

static int load_inf_file_header( void *data )
{
	INF_FILE_HEADER *hp = data;
	int version;

	version = get_int( &hp->version );

	PRINTF( "VERSION %X\n", version );

	return 1;
}

static int load_inf_file_fileinf( void *data )
{
	INF_FILE_FILEINF *hp = data;

	PRINTF( "fileinf %s\n", hp->name );

	return 1;
}

static int load_inf_file_hists( void *data )
{
	INF_FILE_HISTS *hp = data;
	int hist_max;
	int change_count;

	hist_max = get_int( &hp->hist_max );
	change_count = get_int( &hp->change_count );

	PRINTF( "hist_max = %d change_count = %d\n", hist_max, change_count );

	return 1;
}

static int load_inf_file_fileanm( void *data )
{
	INF_FILE_FILEANM *hp = data;

	PRINTF( "fileanm %s\n", hp->name );

	return 1;
}

static int current_name_id;
static char current_name[ 128 ];

static int load_inf_talk_inf( void *data )
{
	INF_TALK_INF *hp = data;
	int userref;

	userref = get_int( &hp->userref );
	PRINTF( "TALK INF %d %s\n", userref, hp->name );

	sjis_to_euc( current_name, hp->name );
	current_name_id = get_strcode( current_name );

	return 1;
}

static int load_inf_talk_area( void *data )
{
	INF_TALK_AREA *hp = data;
	int pos, duration;

	pos = TICK( get_int( &hp->pos ) );
	duration = TICK( get_int( &hp->duration ) );

	PRINTF( "area %d %d\n", pos, duration );

	new_area( current_name_id, pos, pos + duration, current_name );

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

	PRINTF( "pos %d duration %d attr %X param1 %x, param2 %x name %s %x\n",
			pos, duration, attr, param1, param2, hp->name, current_name_id );

	new_action( current_name_id, pos, param1 );

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
	PRINTF( "DIALOG pos %d dur %d name %s str %s\n", pos, duration, name, str );

	new_dialog( 0, pos, pos + duration, str );

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

	PRINTF( "BLOCK pos %d, dur %d block_id %d tm %d\n", pos, duration, block_id, tm );

	return 1;
}

static void init_all( void )
{
	memset( &area, 0, sizeof( COMMON_TAG ) );
	memset( &dialog, 0, sizeof( COMMON_TAG ) );
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

	init_all();

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
