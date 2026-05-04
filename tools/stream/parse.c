/*
   gclコンバーター
   汎用パースルーチン

   入力は、EUCでかかれたテキスト。
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parse.h"

/* -------------------- define 処理 ------------------ */

typedef struct {
	char *tag;
	char *to;
} DEFINE_TABLE;

#define MAX_DEFINE		1024
#define MAX_DEFINE_BUF	(64*1024)

static char *define_buf = NULL;
static char *define_buf_p;
static DEFINE_TABLE define_table[ MAX_DEFINE ];
static long define_table_p;

static void init_define_buf( void )
{
	if( ( define_buf = malloc( MAX_DEFINE_BUF ) ) == NULL ){
		ERROR( "define用のメモリが足りません\n" );
	}
	define_buf_p = define_buf;
	define_table_p = 0;
}

static void free_define_buf( void )
{
	free( define_buf );
}

static char *set_define_buf( char *string )
{
	char *p;
	p = define_buf_p;
	strcpy( p, string );
	define_buf_p += strlen( string ) + 1;
	return p;
}

static char *check_define_table( char *string )
{
	long i;
	DEFINE_TABLE *tp;

	tp = define_table;
	for( i = 0; i < define_table_p; i++, tp++ ){
		if( strcmp( tp->tag, string ) == 0 ){
			return tp->to;
		}
	}
	return NULL;
}

void set_define( char *tag, char *to )
{
	DEFINE_TABLE *new;

	/* 重複チェック */
	if( check_define_table( tag ) != NULL || strcmp( tag, to ) == 0 ){
		WARNING( "define %s:重複してます\n", tag );
		return;
	}

	new = &( define_table[ define_table_p ++ ] );
	if( define_table_p >= MAX_DEFINE ){
		ERROR( "defineの数が多すぎます\n" );
	}
	new->tag = set_define_buf( tag );
	new->to = set_define_buf( to );
/*printf( "set_define %s %s\n", new->tag, new->to );*/
/*printf( "end\n" );*/
}

static char *check_define( char *string )
{
	long i;
	char *p;

	if( ( p = check_define_table( string ) ) != NULL ){
		strcpy( string, p );
	}
	return string;
}

/* ------------------パーサー本体-------------------- */

static PARSE_INFO *current = NULL;

static char word_buffer[ 256 ];

static char special_char_expr[] = ",(){}[]-+*/%&|~^=!#;\"'\\";
static char special_char_normal[] = ",(){}[]#;\"'\\";

static char *special_char;

static long continue_line_flag = 0;


long get_line( void )
{
	if( continue_line_flag ){
		continue_line_flag = 0;
		return 1;
	}
	if( fgets( current->line_buffer, LINE_BUFFER_SIZE, current->fp ) == NULL ){
		if( is_end_file() ){
			return 0;
		} else {
			return get_line();
		}
	}
	current->ptr = current->line_buffer;
	current->line ++;

	return 1;
}

void print_file_current_pos( char *mes )
{
	if( current != NULL ){
		printf( "%s: %s: %s: %d: ", program_name, mes, current->filename, current->line );
	} else {
		printf( "%s: ", mes );
	}
}

void print_current_line( void )
{
	if( current != NULL ){
		long i;
		char *p;
		for( p = current->line_buffer; *p != '\0'; p++ ){
			long code;
			code = *p;
			if( code == '\t' ){
				code = ' ';
			}
			printf( "%c", code );
		}
		for( i = current->ptr - current->line_buffer; i > 1; i-- ){
			printf( " " );
		}
		printf( "^\n" );
	}
}

static char unget_word_buf[ 256 ] = "";

void unget_word( char *buf )
{
	strcpy( unget_word_buf, buf );
}

char *get_word( char *buf )
{
	char *p;

	if( continue_line_flag ){
		return NULL;
	}

	if( unget_word_buf[ 0 ] != '\0' ){
		strcpy( buf, unget_word_buf );
		unget_word_buf[ 0 ] = '\0';
		return buf;
	}

	for( ;; ){
		if( current->ptr == NULL || *( current->ptr ) == '\0' ){
			/* ラインの終り */
			return NULL;
		}
		if( ! isspace( *( current->ptr ) ) ) break;
		current->ptr++;
	}
	/* ここで空白文字列以外 */

	p = buf;

	if( strchr( special_char, *( current->ptr ) ) != NULL ){
		/* 特殊文字ならその文字だけを返す */
		char c;
		c = *( current->ptr ++ );
		if( c == '#' ){
			/* いきなりラインの終り */
			current->ptr = NULL;
			return NULL;
		}
		if( c == ';' ){
			/* 終わったと見せかけて、次のラインに行かせる */
			continue_line_flag = 1;
			return NULL;
		}
		if( c == '\\' ){
			/* いきなり次のライン */
			if( get_line() ){
				return get_word( buf );
			} else {
				return NULL;
			}
		}
		if( c == '\"' ){
			/* 文字列 */
			for( ;; ){
				if( current->ptr == '\0' ){
					ERROR( "文字列が閉じていません\n" );
				}
				if( ( *( p ++ ) = *( current->ptr ++ ) ) == '\"' ){
					p--;
					goto END;
				}
			}
		} else if( c == '\'' ){
			/* 文字列 */
			*( p ++ ) = '\'';
			for( ;; ){
				if( current->ptr == '\0' ){
					ERROR( "文字列が閉じていません\n" );
				}
				if( ( *( p ++ ) = *( current->ptr ++ ) ) == '\'' ){
					goto END;
				}
			}
		} else {
			*( p++ ) = c;
			goto END;
		}
	}

	do {
		if( *current->ptr == '/' && *( current->ptr + 1 ) == '*' ){
			/* コメント */
			for( ;; ){
				if( *( current->ptr ) == '\0' ){
					if( ! get_line() ){
						ERROR( "コメントが閉じていません\n" );
					}
				}
				if( *( current->ptr ) == '*' && *( current->ptr + 1 ) == '/' ){
					current->ptr = current->ptr + 2;
					break;
				}
				current->ptr ++;
			}
			return get_word( buf );
		}
		*( p ++ ) = *( current->ptr ++ );
	} while( current->ptr != '\0' && ! isspace( *( current->ptr ) )
			&& strchr( special_char, *( current->ptr ) ) == NULL );
END:
	*p = '\0';
/*printf( "[%s:%s]", buf, check_define( buf ) );*/
	return check_define( buf );
}

char get_next_char( void )
{
	return *( current->ptr );
}

long load_file( char *filename )
{
	FILE *fp;
	PARSE_INFO *new;

	if( ( fp = fopen( filename, "rt" ) ) == NULL ){
		return 0;
	}

	new = malloc( sizeof( PARSE_INFO ) );
	new->parent = current;
	new->ptr = NULL;
	new->fp = fp;

	new->filename = filename;
	new->line = 0;

	current = new;

	return 1;
}

long is_end_file( void )
{
	current = current->parent;

	return ( current == NULL ) ? 1 : 0;
}

void parse_normal_mode( void )
{
	special_char = special_char_normal;
}

void parse_expr_mode( void )
{
	special_char = special_char_expr;
}

void parse_init( void )
{
	current = NULL;
	parse_normal_mode();
	init_define_buf();
}

void parse_end( void )
{
	current = NULL;
	free_define_buf();
}

#if 0

int main( int argc, char *argv[] )
{
	if( argc < 2 ) return;

	parse_init();

	load_file( argv[ 1 ] );

	do {
		while( get_line() != NULL ){
			char *p;
			char buf[ 128 ];
			while( ( p = get_word( buf ) ) != NULL ){
				printf( "%s:",p );
				if( strcmp( p, "include" ) == 0 ){
					load_file( get_word( buf ) );
				}
			}
			printf( "\n" );
		}
	} while( ! is_end_file() );
	return 0;
}

#endif
