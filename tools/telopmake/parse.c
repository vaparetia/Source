/*
   gclコンバーター
   汎用パースルーチン

   入力は、EUCでかかれたテキスト。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdarg.h>

#include "mymalloc.h"
#include "parse.h"

/* -------------- パース時の特殊キャラ一覧 ----------- */

/* 通常スクリプト評価時 */
static char special_char_normal[] = ",(){}[]/#;`\"'\\";

/* 式評価時 */
static char special_char_expr[] = ",(){}[]-+*/%&|~^<>=!#;`\"'\\";

/* パス名評価時 */
static char special_char_path[] = ",;#`\"'\\";

/* -------------------- define 処理 ------------------ */

typedef struct {
	int tag;
	int to;
} DEFINE_TABLE;

#define MAX_DEFINE			512
#define REALLOC_DEFINE		512
#define MAX_DEFINE_BUF		(32*1024)
#define REALLOC_DEFINE_BUF	(32*1024)

static char *define_buf = NULL;
static int define_buf_p;
static int max_define_buf;
static DEFINE_TABLE *define_table = NULL;
static int define_table_p;
static int max_define_table;

static int analyze_ifdef_flag = 0;

static void init_define_buf( void )
{
	if( ( define_buf = my_malloc( MAX_DEFINE_BUF ) ) == NULL ){
		ERROR( "define用のメモリが足りません\n" );
	}
	max_define_buf = MAX_DEFINE_BUF;
	if( ( define_table = my_malloc( sizeof( DEFINE_TABLE ) * MAX_DEFINE ) ) == NULL ){
		ERROR( "define用のメモリが足りません\n" );
	}
	max_define_table = MAX_DEFINE;
	define_buf_p = 0;
	define_table_p = 0;
}

static void free_define_buf( void )
{
	my_free( define_buf );
	my_free( define_table );
}

static int set_define_buf( char *string )
{
	char *p;
	int pos;
	int len;

	len = strlen( string ) + 1;
	if( define_buf_p + len >= max_define_buf ){
		/* realloc */
		max_define_buf += REALLOC_DEFINE_BUF;
		define_buf = my_realloc( define_buf, max_define_buf );
	}

	p = define_buf + define_buf_p;
	pos = define_buf_p;
	strcpy( p, string );
	define_buf_p += len;
	return pos;
}

static inline char *get_str( int ofs )
{
	return define_buf + ofs;
}

static char *check_define_table( char *string, int check_flag )
{
	int i;
	int flag;
	DEFINE_TABLE *tp;

	tp = define_table;
	flag = 0;
	if( string[ 0 ] == 'd' && string[ 1 ] == ':' ){
		flag = 1;
		string = string + 2;
	}
	for( i = 0; i < define_table_p; i++, tp++ ){
		if( strcmp( get_str( tp->tag ), string ) == 0 ){
PRINTF( "define change %s -> %s\n", get_str( tp->tag ), get_str( tp->to ) );
			if( flag == 0 && check_flag != 0 ){
				WARNING( "define識別子 %s にd:修飾子がついていません。\n", string );
			}
			return get_str( tp->to );
		}
	}
	if( flag == 1 ){
		if( analyze_ifdef_flag != 0 ){
			return NULL;
		}
		ERROR( "d:%s はdefineで登録されていません。\n", string );
	}
	return NULL;
}

void set_define( char *tag, char *to )
{
	DEFINE_TABLE *new;
	char *p;

	/* 重複チェック */
	if( ( p = check_define_table( tag, 0 ) ) != NULL ){
		if( strcmp( to, p ) == 0 ){
//			WARNING( "define %s:重複してます\n", tag );
		} else {
			ERROR( "define %s:すでに'%s'と定義されています。\n", tag, p );
		}
		return;
	}

	if( define_table_p + 1 >= max_define_table ){
		/* 拡張 */
		max_define_table += REALLOC_DEFINE;
		define_table = my_realloc( define_table, max_define_table * sizeof( DEFINE_TABLE ) );
	}
	new = &( define_table[ define_table_p ++ ] );
	new->tag = set_define_buf( tag );
	new->to = set_define_buf( to );
/*printf( "set_define %s %s\n", new->tag, new->to );*/
/*printf( "end\n" );*/
}

static char *check_define( char *string )
{
	char *p;

	if( ( p = check_define_table( string, 1 ) ) != NULL ){
		strcpy( string, p );
	}
	return string;
}

/* ------------------パーサー本体-------------------- */

static PARSE_INFO *current = NULL;

static int current_mode = 0;
static char *special_char;

static int continue_line_flag = 0;
static int no_get_line_flag = 0;
static char unget_word_buf[ 256 ] = "";

int get_line( void )
{
	if( current == NULL || no_get_line_flag ){
		return 0;
	}

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
		printf( "%s: %s: %s: %ld: ", program_name, mes, current->filename, current->line );
	} else {
		printf( "%s: ", mes );
	}
}

void print_current_line( void )
{
	if( current != NULL ){
		int i;
		char *p;
		for( p = current->line_buffer; *p != '\0'; p++ ){
			int code;
			code = *p;
			if( code == '\t' ){
				*p = ' ';
			}
		}
		{
			DUMP( current->line_buffer );
			{
				int pos;
				int len;
				len = strlen( unget_word_buf );
				pos = current->ptr - current->line_buffer - len;
				
				if( pos < 80 ){
					for( i = pos; i > 1; i-- ){
						printf( " " );
					}
					printf( "^\n" );
				}
			}
		}
	}
}

void unget_word( char *buf )
{
PRINTF( "UNGET %s\n", buf );
	strcpy( unget_word_buf, buf );
}

static int define_check_disable = 0;

static char *_get_word( char *buf )
{
	char *p;

	if( current == NULL ){
		return NULL;
	}

	if( continue_line_flag ){
		return NULL;
	}

	if( unget_word_buf[ 0 ] != '\0' ){
		strcpy( buf, unget_word_buf );
PRINTF( "UNGET GET %s\n", buf );
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
		if( c == ';' ){
			/* 終わったと見せかけて、次のラインに行かせる */
			continue_line_flag = 1;
			return NULL;
		}
		if( c == '\\' ){
			/* いきなり次のライン */
Retry:
			if( get_line() ){
				void *ptr;
				if( ( ptr = _get_word( buf ) ) == NULL ){
					goto Retry;
				}
				return ptr;
			} else {
				return NULL;
			}
		}
		if( c == '\"' ){
			/* 文字列 */
			for( ;; ){
				if( *current->ptr == '\0' ){
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
				char c;
				if( *current->ptr == '\0' ){
					ERROR( "文字列が閉じていません\n" );
				}
				c = *( current->ptr ++ );
				if( c == '\\' ){
					c = *current->ptr;
					current->ptr ++;
					switch( c ){
					  case '\r':
						if( *current->ptr == '\n' ){
							current->ptr ++;
						}
						break;
					  case '\n':
						break;
					  case 'n':
						*( p++ ) = '\n';
						break;
					  case 't':
						*( p++ ) = '\t';
						break;
					  default:
						*( p++ ) = c;
						break;
					}
				} else {
					*( p++ ) = c;
					if( c == '\'' ){
						goto END;
					}
				}
			}
		} else if( c == '/' ){
			/* コメントの可能性と割り算記号の可能性がある */
			if( *current->ptr == '*' ){
				/* normal コメント */
				int nest = 1;
				current->ptr = current->ptr + 1;
				for( ;; ){
					if( *( current->ptr ) == '\0' ){
						if( ! get_line() ){
							ERROR( "コメントが閉じていません\n" );
						}
					}
					if( *( current->ptr ) == '/' && *( current->ptr + 1 ) == '*' ){
						nest ++;
						current->ptr = current->ptr + 2;
					}
					if( *( current->ptr ) == '*' && *( current->ptr + 1 ) == '/' ){
						current->ptr = current->ptr + 2;
						if( --nest <= 0 ){
							break;
						}
					}
					current->ptr ++;
				}
				return _get_word( buf );
			} else if( *current->ptr == '/' ){
				/* C++ コメント */
				/* ラインの終り扱いにする */
				current->ptr = NULL;
				return NULL;
			}
			*( p++ ) = c;
			goto END;
		} else {
			*( p++ ) = c;
			goto END;
		}
	}

	do {
		*( p ++ ) = *( current->ptr ++ );
	} while( current->ptr != '\0' && ! isspace( *( current->ptr ) )
			&& strchr( special_char, *( current->ptr ) ) == NULL );
END:
	if( p - buf > MAX_WORDBUF ){
		ERROR( "文字列が長過ぎます\n" );
	}
	*p = '\0';
/*printf( "[%s:%s]", buf, check_define( buf ) );*/
	if( define_check_disable ){
		return buf;
	}
	return check_define( buf );
}

char get_next_char( void )
{
	return *( current->ptr );
}

int load_file( char *filename )
{
	FILE *fp;
	PARSE_INFO *new;

	if( ( fp = fopen( filename, "rt" ) ) == NULL ){
		return 0;
	}

	new = my_malloc( sizeof( PARSE_INFO ) );
	new->parent = current;
	new->ptr = NULL;
	new->fp = fp;

	new->filename = my_malloc( strlen( filename ) + 1 );
	strcpy( new->filename, filename );
	new->line = 0;

	current = new;

	return 1;
}

int is_end_file( void )
{
	PARSE_INFO *cur;

	if( current == NULL ){
		return 1;
	}

	/*
		前のファイルが開いているようなら閉じる
	*/

	cur = current;
	current = current->parent;

	fclose( cur->fp );
	my_free( cur->filename );
	my_free( cur );

	return ( current == NULL ) ? 1 : 0;
}

void parse_normal_mode( void )
{
	/*PRINTF( "PARSE_NORMAL_MODE\n" );*/
	special_char = special_char_normal;
	current_mode = PARSE_NORMAL_MODE;
}

void parse_expr_mode( void )
{
	/*PRINTF( "PARSE_EXPR_MODE\n" );*/
	special_char = special_char_expr;
	current_mode = PARSE_EXPR_MODE;
}

void parse_path_mode( void )
{
	special_char = special_char_path;
	current_mode = PARSE_PATH_MODE;
}

int parse_change_mode( int mode )
{
	int res;
	/*PRINTF( "PARSE_CHANGE_MODE %d\n", mode );*/
	res = current_mode;

	if( mode == PARSE_NORMAL_MODE ){
		parse_normal_mode();
	} else if( mode == PARSE_EXPR_MODE ){
		parse_expr_mode();
	} else if( mode == PARSE_PATH_MODE ){
		parse_path_mode();
	}

	return res;
}

/* ----------------------------------------------- */

/*
	プリプロセッサ処理
*/

char *get_word_ex( char *buf )
{
	/* get_word で存在チェックあり */

	if( get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

char *_get_word_ex( char *buf )
{
	/* _get_word で存在チェックあり */

	if( _get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

char *__get_word( char *buf )
{
	char *p;

	define_check_disable = 1;
	p = _get_word( buf );
	define_check_disable = 0;

	return p;
}

char *__get_word_ex( char *buf )
{
	/* __get_word で存在チェックあり */

	if( __get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

static char include_path[ 256 ] = "";

static void pp_include( int arg )
{
	WORDBUF buf;

	get_word_ex( buf );

	if( ! load_file( buf ) && include_path[ 0 ] != '\0' ){
		char buf2[ 256 ];
		sprintf( buf2, "%s/%s", include_path, buf );
		if( ! load_file( buf2 ) ){
			ERROR( "%sが見つかりません\n" );
		}
	}
	get_line();
}

void set_include_path( char *path )
{
	strcpy( include_path, path );
}

static void pp_define( int arg )
{
	WORDBUF buf1, buf2;

	__get_word_ex( buf1 );
	get_word_ex( buf2 );
	set_define( buf1, buf2 );

	if( _get_word( buf1 ) != NULL ){
		ERROR( "defineは１つの置換のみ行います\n" );
	}
	get_line();
}

#if 0

static void pp_enum( int arg )
{
	WORDBUF tag1, word;
	int num, gf;

	/* すでに enum タグは読み込まれている */

	get_word_ex( tag1 );
	if( tag1[ 0 ] == '{' ){
		tag1[ 0 ] = '\0';
	} else {
		get_word_cl( word );
		if( word[ 0 ] != '{' ){
			ERROR( "{ がありません\n" );
		}
	}

	num = 0;
	gf = 0;
PRINTF( "PP_ENUM\n" );
	for( ;; ){
		WORDBUF tag2;
		get_word_cl( word );

		if( word[ 0 ] == '}' ){
			if( gf == 0 ){
				break;
			}
			gf = 3;
		} else if( word[ 0 ] == ',' ){
			if( gf == 0 ){
				ERROR( ",がおかしなところにあります\n" );
			}
			gf = 2;
		} else if( word[ 0 ] == '=' ){
			WORDBUF numbuf;
			int value;
			if( gf == 0 ){
				ERROR( "=がおかしなところにあります\n" );
			}
			get_word_ex( numbuf );
			if( !is_expr_num( numbuf, &value ) ){
				ERROR( "数字が必要です\n" );
			}
			num = value;
		} else {
			if( gf != 0 ){
				ERROR( "直前に,が必要です\n" );
			}
			gf = 1;
			strcpy( tag2, word );
		}
		if( gf >= 2 ){
			WORDBUF buf, buf2;
			if( tag1[ 0 ] != '\0' ){
				sprintf( buf, "%s:%s", tag1, tag2 );
			} else {
				strcpy( buf, tag2 );
			}
			sprintf( buf2, "%d", num );
			set_define( buf, buf2 );
PRINTF( "enum %s %s\n", buf, buf2 );
			num ++;
			if( gf == 3 ){
				break;
			}
			gf = 0;
		}
	}
}

#define PP_MAX_IF_NEST	8

static int pp_if_stack_p = 0;
static int pp_if_stack[ PP_MAX_IF_NEST ];

enum {
	PP_SKIP_ELSE,
	PP_SKIP_ENDIF,
};

static void pp_else( int arg );
static void pp_endif( int arg );

static int pp_skip_if_block( void )
{
	/* else か endif が出てくるまで skip */

	/* 途中で if がでてきたら、endifが出てくるまでスキップ */

	static char *key[] = {
		"else", "endif", NULL
	};
	int nest;

	nest = 0;

	for( ;; ){
		WORDBUF buf;

		if( __get_word( buf ) == NULL ){
			if( get_line() ){
				continue;
			}
			ERROR( "#ifが閉じていません\n" );
		}
		if( buf[ 0 ] == '#' ){
			char **p;
			__get_word_ex( buf );
			if( strcmp( buf, "if" ) == 0
				|| strcmp( buf, "ifdef" ) == 0
				|| strcmp( buf, "ifndef" ) == 0
				) {
				nest ++;
			}
			if( nest == 0 ){
				for( p = key; *p != NULL; p++ ){
					if( strcmp( *p, buf ) == 0 ){
						return p - key;
					}
				}
			} else {
				if( strcmp( buf, "endif" ) == 0 ){
					nest --;
				}
				if( nest < 0 ){
					ERROR( "#ifのネストが不正です\n" );
				}
			}
		}
	}
	return 0;	/* not reached */
}

static void pp_if( int arg )
{
	int value;
	int old_mode;

	analyze_ifdef_flag = arg;
	/*define されていなかったら0 になるようにする */

	old_mode = parse_change_mode( PARSE_EXPR_MODE );
	if( arg == 0 ){
		EXPR_INFO info, *ep;
		OPERAND *top;

		ep = &info;

		init_expr( ep, '\0' );

		top = set_expr( ep );
		if( ! calc_expr( ep, top, &value ) ){
			ERROR( "#if 式が正しくありません\n" );
		}

		close_expr( ep );
	} else {
		WORDBUF buf;
		__get_word_ex( buf );
		value = ( check_define_table( buf, 0 ) == NULL ) ? 0 : 1;
		if( arg == 2 ){
			value = !value;
		}
		if( __get_word( buf ) != NULL ){
			ERROR( "#ifdef, #ifndef はd:デファイン名だけが有効です\n" );
		}
	}
	parse_change_mode( old_mode );

	analyze_ifdef_flag = 0;

	pp_if_stack_p ++;
	if( pp_if_stack_p >= PP_MAX_IF_NEST ){
		ERROR( "#if のネストが深すぎます。%d段まで\n", PP_MAX_IF_NEST );
	}
	pp_if_stack[ pp_if_stack_p ] = value;

	if( ! value ){
		/* else, endif までスキップ */
		int res;

		res = pp_skip_if_block();
		if( res == PP_SKIP_ELSE ){
			pp_else( 0 );
		} else if( res == PP_SKIP_ENDIF ){
			pp_endif( 0 );
		}
	} else {
		if( get_line() == 0 ){
			ERROR( "途中で終わっています\n" );
		}
	}
}

static void pp_else( int arg )
{
	if( pp_if_stack[ pp_if_stack_p ] ){
		/* すでに if 処理済み */
		while( pp_skip_if_block() != PP_SKIP_ENDIF );
		pp_endif( 0 );
	} else {
		/* まだヒットしていない */
		WORDBUF buf;

		if( _get_word( buf ) == NULL ){
			// ここからスタート
			pp_if_stack[ pp_if_stack_p ] = 1;
			if( get_line() == 0 ){
				ERROR( "途中で終わっています\n" );
			}
		} else if( strcmp( buf, "if" ) == 0 ){
			pp_if_stack_p --;
			pp_if( 0 );
		}
	}
}

static void pp_endif( int arg )
{
	WORDBUF buf;
	if( _get_word( buf ) != NULL ){
		ERROR( "余分な文字列があります\n" );
	}

	pp_if_stack_p --;

	get_line();
}

#endif

typedef struct {
	char *key;
	void (*command)( int arg );
	int arg;
} PP_COMMAND;

static PP_COMMAND pp_command[] = {
	{ "include", pp_include, 0 },
	{ "define", pp_define, 0 },
#if 0
	{ "if", pp_if, 0 },
	{ "ifdef", pp_if, 1 },
	{ "ifndef", pp_if, 2 },
	{ "else", pp_else, 0 },
	{ "endif", pp_endif, 0 },
	{ "enum", pp_enum, 0 },
#endif
	{ NULL, NULL }
};

static int do_pp_command( char *buf )
{
	PP_COMMAND *p;

	for( p = pp_command; p->key != NULL; p++ ){
		if( strcmp( buf, p->key ) == 0 ){
			( *p->command )( p->arg );
			return 1;
		}
	}
	return 0;
}

char *get_word( char *buf )
{
	if( _get_word( buf ) == NULL ){
		return NULL;
	}
	if( buf[ 0 ] != '#' ) return buf;

	/* プリプロセッサ処理 */
	if( _get_word( buf ) == NULL ){
		ERROR( "# 命令が不正です\n" );
	}

	if( do_pp_command( buf ) ){
		return get_word( buf );
	}
	ERROR( "# 命令が不正です\n" );

	return buf;
}

/* ----------------------------------------------- */
/*
	初期化、終了
*/


void parse_init( void )
{
#if 0
//	pp_if_stack_p = 0;
#endif
	
	current = NULL;
	parse_normal_mode();
}

void parse_end( void )
{
#if 0
	if( pp_if_stack_p > 0 ){
		ERROR( "#if が閉じていません。\n" );
	}
#endif
	current = NULL;
}

void parse_init_define( void )
{
	init_define_buf();
}

void parse_free_define( void )
{
	free_define_buf();
}

/* ----------------------------------------------- */
/*
	ユーティリティ
*/

void get_line_ex( void )
{
	if( ! get_line() ){
		ERROR( "途中で終わっています\n" );
	}
}

void open_brance_check( void )
{
	WORDBUF buf;

	if( get_word( buf ) == NULL || buf[ 0 ] != '{' ){
		ERROR( "ブロックが不正です\n" );
	}
}

void get_word_with_error_check( char *word, char *def, char *errormes )
{
	if( get_word( word ) == NULL
		|| ( def != NULL && strcmp( word, def ) != 0 ) ){
		ERROR( errormes );
	}
}
	 
char *get_word_cl( char *word )
{
	while( get_word( word ) == NULL ){
		get_line_ex();
	}
	return word;
}

static char *save_ptr = NULL;
static char save_line_buffer[ LINE_BUFFER_SIZE ];

void set_parse_buffer( char *ptr )
{
	save_ptr = current->ptr;
	strcpy( save_line_buffer, current->line_buffer );

	strcpy( current->line_buffer, ptr );
	current->ptr = current->line_buffer;
	no_get_line_flag = 1;
}

void reset_parse_buffer( void )
{
	current->ptr = save_ptr;
	strcpy( current->line_buffer, save_line_buffer );
	save_ptr = NULL;
	no_get_line_flag = 0;
}
