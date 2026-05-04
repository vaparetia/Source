/*
   シンボル情報定義テーブル
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gclconv.h"
#include "mymalloc.h"

typedef struct _symtag {
	int code;
	int name;
} SYMTAG;

#define MAX_SYM_ADD		1024
#define MAX_SYMBUF_ADD	(64*1024)

int tsymbol_no_error_flag = 0;
int tsymbol_dump_flag = 0;

static SYMTAG *sym_table;
static int sym_table_max;
static int sym_table_num;

static char *sym_str_buf;
static int sym_str_max;
static int now_ptr;

void init_sym_table( void )
{
	sym_table = NULL;
	sym_table_max = 0;
	sym_table_num = 0;

	sym_str_buf = NULL;
	sym_str_max = 0;
	now_ptr = 0;
}

static void set_sym_table( int code, char *name )
{
	SYMTAG *p;

	if( sym_table_num >= sym_table_max ){
		sym_table_max += MAX_SYM_ADD;
		sym_table = my_realloc( sym_table, sym_table_max * sizeof( SYMTAG ) );
	}
	p = sym_table + sym_table_num;
	sym_table_num ++;

	p->code = code;

	if( now_ptr + strlen( name ) + 1 >= sym_str_max ){
		sym_str_max += MAX_SYMBUF_ADD;
		sym_str_buf = my_realloc( sym_str_buf, sym_str_max );
	}
	p->name = now_ptr;
	strcpy( sym_str_buf + now_ptr, name );
	now_ptr += strlen( name ) + 1;
}

static int comp_name( SYMTAG *key, SYMTAG *d )
{
	int ret;

	if( ( ret = strcmp( sym_str_buf + key->name, sym_str_buf + d->name ) ) == 0 ){
#if 0 // TODO
		if( key->code != d->code ){
			extern int tsymbol_test_mode;
			if( tsymbol_test_mode ){
				/* SKIP */
			} else {
				ERROR( "同じ文字列が違うコードに割り当てられています。%s\n", key->name );
			}
		}
#endif
	}
	return ret;
}

static char line_buffer[ 256 * 2 ];
static char name_buffer[ 256 ];

int load_symbol_file( char *filename )
{
	FILE *fp;

	if( ( fp = fopen( filename, "rt" ) ) == NULL ){
		WARNING( "シンボルファイル %s が見つかりません。\n", filename );
		return 0;
	}

	while( fgets( line_buffer, 256, fp ) != NULL ){
		int code;
		if( line_buffer[ 0 ] == ';' || line_buffer[ 0 ] == '#' ){
			continue;
		}
		if( sscanf( line_buffer, "%x %s", &code, name_buffer ) < 2 ){
			continue;
		}
/*		PRINTF( "0x%X %s\n", code, name_buffer );*/
		set_sym_table( code, name_buffer );
	}

	fclose( fp );

	/* bsearch用に並べ替え */

	qsort( sym_table, sym_table_num, sizeof( SYMTAG ), ( void * )comp_name );

	return 1;
}

void check_duplicate_symbol( void )
{
	// 重複チェック
	// すでにソートされているのが前提
	int i;

	for( i = 1; i < sym_table_num; i++ ){
		char *p1 = sym_str_buf + sym_table[ i - 1 ].name;
		char *p2 = sym_str_buf + sym_table[ i ].name;
		if( strcmp( p1, p2 ) == 0 ){
			if( tsymbol_no_error_flag == 0 ){
				if( sym_table[ i - 1 ].code != sym_table[ i ].code ){
					WARNING( "symbol : %s : 二重に定義されています\n", p2 );
				}
			} else {
				if( sym_table[ i - 1 ].code > sym_table[ i ].code ){
					// 小さい方にあわせる。
					sym_table[ i - 1 ].code = sym_table[ i ].code;
				} else {
					sym_table[ i ].code = sym_table[ i - 1 ].code;
				}
			}
		}
	}
}

static int search_name( char *key, SYMTAG *d )
{
	return strcmp( key, sym_str_buf + d->name );
}

int get_symbol_code( char *name, int flag )
{
	SYMTAG *p;

	p = bsearch( name, sym_table, sym_table_num
				, sizeof( SYMTAG ), ( void * )search_name );

	if( p == NULL ){
		if( flag == 0 ){
			WARNING( "シンボル %s は見つかりませんでした。0で置換します。\n"
					, name );
			return 0;
		} else if( flag == 1 ){
			WARNING( "シンボル %s は見つかりませんでした。-1で置換します。\n"
					, name );
			return -1;
		} else if( flag == 2 ){
			if( tsymbol_no_error_flag ){
				WARNING( "シンボル %s は見つかりませんでした。0で置換します。\n"
						, name );
				return 0;
			}
			ERROR( "シンボル %sが見つかりません\n", name );
		} else {
			return 0;
		}
	}
	if( tsymbol_dump_flag ){
		printf( "%s\n", name );
	}
	return p->code;
}
