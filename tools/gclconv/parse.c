/**
   @file
   gclコンバーター
   汎用パースルーチン

   入力は、EUCでかかれたテキスト。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdarg.h>
#include <regex.h>

#include "mymalloc.h"
#include "hash.h"
#include "parse.h"

static PARSE_INFO *current = NULL;

/* -------------- dependモード制御用 ------------------ */

static FILE *depend_log_fp = NULL;
static FILE *dep_log = NULL;
static int depend_line = 0;

/* -------------- #if のネスト ----------- */

#define PP_MAX_IF_NEST	8

static int pp_if_stack_p = 0;
static int pp_if_stack[ PP_MAX_IF_NEST ];

/* -------------- パース時の特殊キャラ一覧 ----------- */

/* 通常スクリプト評価時 */
static char special_char_normal[] = ",(){}[]/#;`\"'\\";

/* 式評価時 */
static char special_char_expr[] = ",(){}[]-+*/%&|~^<>=!#;`\"'\\";

/* 文字列評価時 */
static char special_char_string[] = "{}";

/* パーススキップ時 */

static char special_char_skip[] = "#/";

/* ---------------- ファイル番号処理 ---------------- */

static char *parse_files[ MAX_PARSE_FILES ];
static int max_parse_files = 0;

static int set_parse_filename( char *file )
{
	int i;
	for( i = 0; i < max_parse_files; i++ ){
		if( strcmp( parse_files[ i ], file ) == 0 ){
			return i;
		}
	}
	parse_files[ i ] = my_malloc( strlen( file ) + 1 );
	strcpy( parse_files[ max_parse_files ], file );
	i = max_parse_files ++;
	if( max_parse_files >= MAX_PARSE_FILES ){
		FATAL( "1ファイルから呼び出されるファイル数が多すぎます(%d)\n", MAX_PARSE_FILES ) ;
	}
	return i;
}

static void init_parse_files( void )
{
	max_parse_files = 0;

	set_parse_filename( "CommandLine" );
}

#define get_parse_filename( _no ) ( parse_files[ _no ] )

/* -------------------- 正規表現 ------------------ */
/*
   src は 元の文字列
   patternは、( "/regex/pat/", "/regex/pat/", "..." )
*/

static char *get_regex( char *buf, char *src )
{
	char *d = buf;
	char *p = src;
	while( *p != '/' ){
		if( *p == '\0' ){
			ERROR( "正規表現が異常です\n" );
		}
		if( *p == '\\' ){
			*( d++ ) = *( p++ );
		}
		*( d++ ) = *( p++ );
	}
	*( d++ ) = '\0';
	return p + 1;
}

#define MATCH_BUF_SIZE 1024

typedef struct {
	char *buffer;
	int size;
	int now_p;
} BUFFER;

static void init_buffer( BUFFER *bi )
{
	bi->buffer = NULL;
	bi->size = 0;
	bi->now_p = 0;
}

static void resize_buffer( BUFFER *bi, int size_step )
{
	int size;
	size = bi->size + size_step;
	bi->buffer = my_realloc( bi->buffer, size );
	bi->size = size;
}

static char set_data( BUFFER *bi, void *data, int size )
{
	int offset;

	if( bi->now_p + size + 1 >= bi->size ){
		resize_buffer( bi, MATCH_BUF_SIZE );
	}
	offset = bi->now_p;
	memcpy( bi->buffer + bi->now_p, data, size );

	bi->now_p += size;
	*( bi->buffer + bi->now_p ) = '\0';
	return offset;
}

static char *replace_match( char *src, int *ofs, regmatch_t *pmatch, int matches, char *rep )
{
	BUFFER buf;
	unsigned char *r;
	unsigned char *rp;

	init_buffer( &buf );

	if( *ofs > 0 ){
		set_data( &buf, src, *ofs );
	}
	src = src + *ofs;
	*ofs += pmatch[ 0 ].rm_eo;

	rp = r = ( unsigned char * )rep;

	set_data( &buf, src, pmatch[ 0 ].rm_so );

	for( ;; ){
		if( *r == '\\' ){
			int no;
			/* sub string */
			// そこまでのデータを出力
			if( rp < r ){
				set_data( &buf, rp, r - rp );
			}
			// そのあとは数字１文字
			no = *( r + 1 ) - '0';
			if( no > matches ){
				ERROR( "マッチする数より\\で参照している数のほうが多い\n" );
			}
			// その文を展開
			set_data( &buf, src + pmatch[ no ].rm_so
					 , pmatch[ no ].rm_eo - pmatch[ no ].rm_so );
			// 2文字スキップ

			rp = r = r + 2;
		}
		if( *r == '\0' ){
			// そこまでのデータを出力
			if( rp < r ){
				set_data( &buf, rp, r - rp );
			}
			break;
		}
		r++;
	}
	// ラスト処理
	{
		r = src + pmatch[ 0 ].rm_eo;
		set_data( &buf, r, strlen( r ) + 1 );
	}

	return buf.buffer;
}

#define MAX_REGEX_MATCH 8

// 返り値はmallocしたアドレス。終了後にfreeすること。
static char *change_buffer_regex( char *src_string )
{
	WORDBUF buf;
	char *s;

	char regex_buf[ 256 ];
	char replace_buf[ 256 ];

	regex_t preg;
	regmatch_t pmatch[ MAX_REGEX_MATCH ];

	s = my_malloc( strlen( src_string ) + 1 );
	strcpy( s, src_string );

	while( get_word( buf ) != NULL ){
		if( buf[ 0 ] == '/' ){
			// 正規表現
			char *p = buf + 1;
			char *n;
			int result;
			int pn;

			p = get_regex( regex_buf, p );
			if( strlen( regex_buf ) > 255 ){
				ERROR( "正規表現が長すぎます\n" );
			}
			p = get_regex( replace_buf, p );
			if( strlen( replace_buf ) > 255 ){
				ERROR( "置換部分が長すぎます\n" );
			}
			if( *p != '\0' ){
				ERROR( "正規表現がちゃんと終わっていません\n" );
			}

			if( ( result = regcomp( &preg, regex_buf, REG_EXTENDED ) ) != 0 ){
				ERROR( "正規表現が無効です %d\n", result );
			}
			pn = 0;
			while( regexec( &preg, s + pn, MAX_REGEX_MATCH, pmatch, 0 ) == 0 ){
				n = replace_match( s, &pn, pmatch, preg.re_nsub, replace_buf );
				my_free( s );
				s = n;
			}
			if( pn == 0 ){
				WARNING( "正規表現がマッチしませんでした。\n" );
			}
			regfree( &preg );
		} else if( buf[ 0 ] == ',' ){
			// 次へ
		} else if( buf[ 0 ] == ')' ){
			// 終わり
			break;
		}
	}

	return s;
}

/* -------------------- define 処理 ------------------ */

static int define_check_flag = 0;
static int analyze_ifdef_flag = 0;

// HASH使用版

#define DEFINE_PAGE_SIZE	(256 * 1024)
#define DEFINE_HASH_SIZE	256

typedef struct {
	char *tag;
	char *to;
	int fileno;
	int line;
} DEFINE_TABLE;

static HASH_INFO *def_hash;
static MLTAG *def_mltag;

static void init_define_buf( void )
{
	def_mltag = tMalloc_Create( DEFINE_PAGE_SIZE );
	def_hash = hash_create( DEFINE_HASH_SIZE, HASH_KEY_STRING );
}

static void free_define_buf( void )
{
	tMalloc_Destroy( def_mltag );
	hash_destroy( def_hash );
}

static DEFINE_TABLE *current_define_table = NULL;

static char *check_define_table( char *string, int check_flag )
{
	int flag;
	HASH_RESULT *result;

	flag = 0;
	if( string[ 0 ] == 'd' && string[ 1 ] == ':' ){
		flag = 1;
		string = string + 2;
	}

	current_define_table = NULL;

	if( ( result = hash_search_string( def_hash, string ) ) != NULL ){
		DEFINE_TABLE *tp = HASH_VALUE( result );
		if( analyze_ifdef_flag != 0 ){
			return "1";
		}
		if( flag == 0 && check_flag != 0 ){
			WARNING( "define識別子 %s にd:修飾子がついていません。\n", string );
		}
		current_define_table = tp;
		return tp->to;
	}
	
	if( analyze_ifdef_flag != 0
	   && ( flag == 1 || strchr( special_char_expr, string[ 0 ] ) == NULL ) ){
		return "0";
	}
	if( flag == 1 && depend_log_fp == NULL ){
		ERROR( "d:%s はdefineで登録されていません。\n", string );
	}
	return NULL;
}

/**
	define文字列のセット。

	@param tag : タグ
	@param to  : 変換文字列
*/

void set_define( char *tag, char *to )
{
	HASH_RESULT *result;
	DEFINE_TABLE *dnew;

	dnew = tMalloc( def_mltag, sizeof( *dnew ) );
	memset( dnew, 0, sizeof( *dnew ) );

	dnew->tag = tMalloc( def_mltag, strlen( tag ) + 1 );
	dnew->to = tMalloc( def_mltag, strlen( to ) + 1 );
	strcpy( dnew->tag, tag );
	strcpy( dnew->to, to );
	if( current != NULL ){
		dnew->fileno = current->fileno;
		dnew->line = current->line;
	}

	if( ( result = hash_entry_string( def_hash, dnew->tag, dnew ) ) != NULL ){
		// すでに設定されている
		DEFINE_TABLE *dold = ( DEFINE_TABLE * )HASH_VALUE( result );
		if( strcmp( to, dold->to ) == 0 ){
#if 0
			if( dold->fileno != dnew->fileno || dold->line != dnew->line){
				WARNING( "define %s: 同じものが %s:%d で定義されてます。\n"
						, tag
						, parse_files[ dold->fileno ]
						, dold->line );
			}
#endif
		} else {
			ERROR( "define %s:すでに %s:%d で'%s'と定義されています。\n"
					, tag
					, parse_files[ dold->fileno ]
					, dold->line
					, dold->to );
		}
	}
}

/* ------------------パーサー本体-------------------- */

static int current_mode = 0;
static char *special_char;

static char unget_word_buf[ 256 ] = "";

/**
	ラインバッファに一行取り込む。
	@retval 1 : 正常に取り込めた
	@retval 0 : ファイルが終了している
*/

int get_line( void )
{
	if( current == NULL || current->fp == NULL ){
		return 0;
	}
	if( current->continue_line_flag ){
		current->continue_line_flag = 0;
		return 1;
	}
	if( current->fp == NULL // バッファモード
	   || fgets( current->line_buffer, LINE_BUFFER_SIZE, current->fp ) == NULL ){
		if( is_end_file() ){
			return 0;
		} else {
			return get_line();
		}
	}
	current->back_ptr = current->ptr = current->line_buffer;
	current->line ++;

	return 1;
}

/**
	エラー時に現在のファイルと行数を出力する。
	@param mes : "Error", "Warning"という文字列
*/
void print_file_current_pos( char *mes )
{
	if( current != NULL ){
		{
			PARSE_INFO *p = current;
			int first = 1;
			while( ( p = p->parent ) != NULL ){
				printf( "%s from %s:%d:\n"
					   , ( first ? "In file included" : "                " )
					   , parse_files[ p->fileno ], p->line );
				first = 0;
			}
		}
		printf( "%s: %s: %s: %d: ", program_name, mes, parse_files[ current->fileno ]
			   , current->line );
	} else {
		printf( "%s: ", mes );
	}
}

/**
	エラー時に現在の行の内容と,解釈ポインタの位置を出力する。
*/
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
			if( p != ( char * )current->line_buffer && p[ -1 ] != '\n' ){
				DUMP( "\n" );
			}
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

/**
	文字列を戻す
	バッファにコピーするので,とってきた文字列と違ってもいい。
	@param buf : 文字列
*/
void unget_word( char *buf )
{
PRINTF( "UNGET %s\n", buf );
	strcpy( unget_word_buf, buf );
}

/**
	直前で取得した文字列の先頭まで解釈ポインタを戻す。
*/
void back_word_ptr( void )
{
	current->ptr = current->back_ptr;
}

static int define_check_disable = 0;

static char *_get_word( char *buf )
{
	char *p;

	if( current == NULL ){
		return NULL;
	}

	if( current->continue_line_flag ){
		return NULL;
	}

	if( unget_word_buf[ 0 ] != '\0' ){
		strcpy( buf, unget_word_buf );
PRINTF( "UNGET GET %s\n", buf );
		unget_word_buf[ 0 ] = '\0';
		return buf;
	}

	current->back_ptr = current->ptr;

	for( ;; ){
		if( current->ptr == NULL || *( current->ptr ) == '\0' ){
			/* ラインの終り */
			goto LINEEND;
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
			current->continue_line_flag = 1;
			goto NULLEND;
		}
		if( c == '\\' ){
			/* いきなり次のライン */

			for( ;; ){
				if( *( current->ptr ) == '\0' ){
					/* ラインの終り */
					break;
				}
				if( ! isspace( *( current->ptr ) ) ) break;
				current->ptr++;
			}
			if( *current->ptr == '\\' ){
				WARNING( "\\のあとにさらに\\があります\n" );
			} else if( _get_word( buf ) != NULL ){
				WARNING( "\\のあとにコメントではない文字列があります\n" );
			}
Retry:
			if( get_line() ){
				void *ptr;
				if( ( ptr = _get_word( buf ) ) == NULL ){
					goto Retry;
				}
				return ptr;
			} else {
				goto LINEEND;
			}
		}
		if( c == '\"' ){
			/* 文字列 */
			for( ;; ){
				if( *current->ptr == '\0' ){
					if( depend_log_fp == NULL ){
						ERROR( "文字列が閉じていません %.10s\n", buf );
					} else {
						goto END;
					}
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
					if( depend_log_fp == NULL ){
						ERROR( "文字列が閉じていません %.10s\n", buf );
					} else {
						goto END;
					}
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
				int line;
				line = current->line;

				current->ptr = current->ptr + 1;
				for( ;; ){
					if( *( current->ptr ) == '\0' ){
						if( ! get_line() ){
							ERROR( "コメントが閉じていません(%s:%d)\n"
								  , get_parse_filename( current->fileno ), line );
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
				goto LINEEND;
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
	} while( *current->ptr != '\0' && ! isspace( *( current->ptr ) )
			&& strchr( special_char, *( current->ptr ) ) == NULL );
END:
	if( depend_log_fp == NULL && p - buf > MAX_WORDBUF ){
		ERROR( "文字列が長過ぎます\n" );
	}
	*p = '\0';
	if( define_check_disable ){
		return buf;
	}
#if 0	// 現状ではそれほど高速化しなかったので
	if( define_check_flag == 0 ){
		if( buf[ 0 ] != 'd' || buf[ 1 ] != ':' ){
			return buf;
		}
	}
#endif

	/* define展開 */
	if( ( p = check_define_table( buf, 1 ) ) != NULL ){
		if( analyze_ifdef_flag ){
			// １階層のみ
			strcpy( buf, p );
			return buf;
		} else {
			DEFINE_TABLE *tp = current_define_table;
			if( *current->ptr == '(' ){
				// regexによる式展開を選択
				p = change_buffer_regex( p );
				set_parse_buffer( p );
				my_free( p );
			} else {
				set_parse_buffer( p );
			}
			if( tp != NULL ){
				current->fileno = tp->fileno;
				current->line = tp->line;
			}
			current->def_buffer_flag = 1;
			return _get_word( buf );
		}
	}
//printf( "_get_word '%s'\n", buf );
	return buf;

  LINEEND:
//printf( "LINEEND %d\n", current->def_buffer_flag );
	if( current->def_buffer_flag ){
		reset_parse_buffer();
		return _get_word( buf );
	}
  NULLEND:
//printf( "NULLEND\n" );
	return NULL;
}

/**
	次に取得する文字を返す。
	空白文字の場合もある。
*/
char get_next_char( void )
{
	return *( current->ptr );
}

/**
	テキストファイルをloadする。
	@retval 0 : ファイルがない。
	@retval 1 : 成功。
*/
int load_file( char *filename )
{
	FILE *fp;
	PARSE_INFO *pnew;

	parse_set_depend_file( filename );

	if( ( fp = fopen( filename, "rt" ) ) == NULL ){
PRINTF( "LOAD FILE %s failed\n", filename );
		return 0;
	}

PRINTF( "FILE %s loaded\n", filename );
	pnew = my_malloc( sizeof( PARSE_INFO ) );
	pnew->line_buffer = my_malloc( LINE_BUFFER_SIZE );
	pnew->line_buffer_size = LINE_BUFFER_SIZE;
	pnew->parent = current;
	pnew->back_ptr = pnew->ptr = NULL;
	pnew->fp = fp;

	pnew->fileno = set_parse_filename( filename );
	
	pnew->line = 0;
	pnew->continue_line_flag = 0;
	pnew->def_buffer_flag = 0;

	pnew->pp_if_stack_level = pp_if_stack_p;

	current = pnew;

	return 1;
}

/**
	ファイルが終了しているか判定する。
	基本的には内部で使用。
*/
int is_end_file( void )
{
	PARSE_INFO *cur;

	if( current == NULL ){
		return 1;
	}
	cur = current;

	/*
		ifdefはファイル単位で解決しているはず
	*/
	if( pp_if_stack_p > cur->pp_if_stack_level  ){
		ERROR( "このファイル内で#ifが閉じていません\n" );
	}
	if( pp_if_stack_p < cur->pp_if_stack_level  ){
		ERROR( "このファイル内で#ifと対応してない#endifがあります\n" );
	}

	/*
		前のファイルが開いているようなら閉じる
	*/
	current = current->parent;

	fclose( cur->fp );

	my_free( cur->line_buffer );
	my_free( cur );

	return ( current == NULL ) ? 1 : 0;
}

/**
	標準モードでのパースを設定
*/
void parse_normal_mode( void )
{
	/*PRINTF( "PARSE_NORMAL_MODE\n" );*/
	special_char = special_char_normal;
	current_mode = PARSE_NORMAL_MODE;
}

/**
	式評価モードでのパースを設定
*/
void parse_expr_mode( void )
{
	/*PRINTF( "PARSE_EXPR_MODE\n" );*/
	special_char = special_char_expr;
	current_mode = PARSE_EXPR_MODE;
}

/**
	文字列評価モードでのパースを設定
*/
void parse_string_mode( void )
{
	special_char = special_char_string;
	current_mode = PARSE_STRING_MODE;
}

/**
	任意のパースモードを設定する。
	@param mode : PARSE_NORMAL_MODE, PARSE_EXPR_MODE, PARSE_STRING_MODE
	@return 設定前のモードを返す。
*/

int parse_change_mode( int mode )
{
	int res;
	/*PRINTF( "PARSE_CHANGE_MODE %d\n", mode );*/
	res = current_mode;

	if( mode == PARSE_NORMAL_MODE ){
		parse_normal_mode();
	} else if( mode == PARSE_EXPR_MODE ){
		parse_expr_mode();
	} else if( mode == PARSE_STRING_MODE ){
		parse_string_mode();
	}

	return res;
}

/* ---------------------------------------------------------------------- */
/*
	depend処理関係
*/

#define DEPEND_MAX_LINE 78

/**
	depend logを出力するためのファイルを設定
	@param filename : ファイル名
*/
void parse_set_depend_log_file( char *filename )
{
	FILE *fp;
	if( ( fp = fopen( filename, "at" ) ) == NULL ){
		FATAL( "%s:オープンできません。\n", filename );
	}
	depend_log_fp = fp;
	depend_line = 0;
}

/**
	実際にdependを出力するために現在のファイル名を設定
	@param outfile : 現在処理するファイル名
*/
void parse_set_depend_outfile( char *outfile )
{
	if( depend_log_fp != NULL ){
		fprintf( depend_log_fp, "%s :", outfile );
		depend_line = strlen( outfile ) + 2;
		printf( "output depend %s...\n", outfile );
		dep_log = depend_log_fp;
	}
}

/**
	実際にdependを出力するために依存しているファイル名を設定
	@param depfile : 依存するファイル名
*/
void parse_set_depend_file( char *depfile )
{
	if( dep_log != NULL ){
		int len;
		
		len = strlen( depfile ) + 1;
		if( depend_line + len > DEPEND_MAX_LINE ){
			fprintf( depend_log_fp, " \\\n\t" );
			depend_line = 8;
		}
		fprintf( dep_log, " %s", depfile );
		depend_line += len;
	}
}

/**
	依存ファイルを閉じる。
*/
void parse_close_depend_log_file( void )
{
	if( depend_log_fp != NULL ){
		fprintf( depend_log_fp, "\n" );
		fclose( depend_log_fp );
		depend_log_fp = NULL;
		dep_log = NULL;
	}
}


/* ----------------------------------------------- */
/*
	ユーティリティ
*/

#include "expr.h"

/** get_word で存在チェックあり */
char *get_word_ex( char *buf )
{

	if( get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

/** _get_word で存在チェックあり */
char *_get_word_ex( char *buf )
{

	if( _get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

/** _get_word() でdefine展開をしない */
char *__get_word( char *buf )
{
	char *p;

	define_check_disable = 1;
	p = _get_word( buf );
	define_check_disable = 0;

	return p;
}

/** get_lineで駄目だったら即エラー */
void get_line_ex( void )
{
	if( ! get_line() ){
		ERROR( "途中で終わっています:ex\n" );
	}
}

/** 行が終っていてもそのつぎを読み込む */
char *get_word_cl( char *word )
{
	while( get_word( word ) == NULL ){
		get_line_ex();
	}
	return word;
}

/** __get_word()で行が終っていてもそのつぎを読み込む */
char *__get_word_cl( char *word )
{
	while( __get_word( word ) == NULL ){
		get_line_ex();
	}
	return word;
}

/**
	define展開を行なわないようにする
	@param mode 0 define展開を行なう。
	@param mode 1 define展開を行なわない。
*/
void parse_define_check_disable( int mode )
{
	define_check_disable = mode;
}

/** __get_word で存在チェックあり */
char *__get_word_ex( char *buf )
{
	if( __get_word( buf ) == NULL ){
		ERROR( "パースエラー。途中で終わっています。\n" );
	}
	return buf;
}

/**
	'{'が存在していることをチェック
*/
void open_brance_check( void )
{
	WORDBUF buf;

	if( get_word( buf ) == NULL || buf[ 0 ] != '{' ){
		ERROR( "ブロックが不正です\n" );
	}
}

/**
	defという文字列がその次に書かれていることをチェックする。
	なければ、erromesを出力してエラー終了。
*/
void get_word_with_error_check( char *word, char *def, char *errormes )
{
	if( get_word( word ) == NULL
		|| ( def != NULL && strcmp( word, def ) != 0 ) ){
		ERROR( errormes );
	}
}
	 
/* ---------------------------------------------------------------------- */
/*
	プリプロセッサ処理
*/

static char include_path[ 256 ] = "";

static void pp_include( int arg )
{
	WORDBUF buf;

	get_word_ex( buf );

	if( ! load_file( buf ) ){
		if( include_path[ 0 ] != '\0' ){
			char buf2[ 256 ];
			sprintf( buf2, "%s/%s", include_path, buf );
			if( ! load_file( buf2 ) ){
//				ERROR( "%sが見つかりません\n", buf );
				WARNING( "%sが見つかりません\n", buf );
			}
		} else {
//			ERROR( "%sが見つかりません\n", buf );
			WARNING( "%sが見つかりません\n", buf );
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
	BUFFER buf;
	int first = 1;

	init_buffer( &buf );

	__get_word_ex( buf1 );

	while( get_word( buf2 ) != NULL ){
		if( first == 0 ){
			set_data( &buf, " ", 1 );
		}
		first = 0;
		set_data( &buf, buf2, strlen( buf2 ) );
	}

	set_data( &buf, "", 1 );
	set_define( buf1, buf.buffer );

	my_free( buf.buffer );

	get_line();
}

static void pp_enum( int arg )
{
	WORDBUF tag1, word;
	int num, gf;
	int old_mode;

	old_mode = parse_change_mode( PARSE_EXPR_MODE );

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
		__get_word_cl( word );

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
			parse_change_mode( PARSE_NORMAL_MODE );
			/* -1 を指定できるようにするため,式モードを抜ける */
			get_word_ex( numbuf );
			if( !is_expr_num( numbuf, &value ) ){
				ERROR( "数字が必要です\n" );
			}
			parse_change_mode( PARSE_EXPR_MODE );
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
	parse_change_mode( old_mode );
}

void parse_enum( void )
{
	/* enumタグが読み込まれた状態 */
	pp_enum( 0 );
}

enum {
	PP_SKIP_ELSE,
	PP_SKIP_ENDIF,
	PP_SKIP_ELSEIF,
	PP_SKIP_ELSEIFDEF,
	PP_SKIP_ELSEIFNDEF,
};

static void pp_else( int arg );
static void pp_elseif( int arg );
static void pp_endif( int arg );

static int pp_skip_if_block( void )
{
	/* else か endif が出てくるまで skip */

	/* 途中で if がでてきたら、endifが出てくるまでスキップ */

	static char *key[] = {
		"else", "endif",
		"elseif", "elif",
		"elifdef", "elseifdef",
		"elifndef", "elseifndef"
		, NULL
	};
	static char code[] = {
		PP_SKIP_ELSE, PP_SKIP_ENDIF,
		PP_SKIP_ELSEIF, PP_SKIP_ELSEIF,
		PP_SKIP_ELSEIFDEF, PP_SKIP_ELSEIFDEF,
		PP_SKIP_ELSEIFNDEF, PP_SKIP_ELSEIFNDEF
	};
	int nest;
	char *special_char_bak = special_char;

	nest = 0;
	special_char = special_char_skip;

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
						special_char = special_char_bak;
						return code[ p - key ];
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
	
	special_char = special_char_bak;
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
#if 0
		WORDBUF buf;
		__get_word_ex( buf );
		value = ( check_define_table( buf, 0 ) == NULL ) ? 0 : 1;
		if( arg == 2 ){
			value = !value;
		}
		if( __get_word( buf ) != NULL ){
			ERROR( "#ifdef, #ifndef はd:デファイン名だけが有効です\n" );
		}
#else
		EXPR_INFO info, *ep;
		OPERAND *top;

		ep = &info;

		init_expr( ep, '\0' );

		top = set_expr( ep );
		if( ! calc_expr( ep, top, &value ) ){
			ERROR( "#if 式が正しくありません\n" );
		}
		close_expr( ep );

		if( arg == 2 ){
			value = !value;
		}
#endif
	}
	parse_change_mode( old_mode );

	analyze_ifdef_flag = 0;

	pp_if_stack[ pp_if_stack_p ] = value;
	pp_if_stack_p ++;
	if( pp_if_stack_p >= PP_MAX_IF_NEST ){
		ERROR( "#if のネストが深すぎます。%d段まで\n", PP_MAX_IF_NEST );
	}

	if( ! value ){
		/* else, endif までスキップ */
		int res;

		res = pp_skip_if_block();
		if( res == PP_SKIP_ENDIF ){
			pp_endif( 0 );
		} else if( res == PP_SKIP_ELSE ){
			pp_else( 0 );
		} else {
			pp_elseif( res - PP_SKIP_ELSEIF );
		}
	} else {
		if( get_line() == 0 ){
			ERROR( "途中で終わっています\n" );
		}
	}
}

static void pp_else( int arg )
{
	if( pp_if_stack_p == 0  ){
		ERROR( "#ifが対応していない#elseです。\n" );
	}

	if( pp_if_stack[ pp_if_stack_p - 1 ] ){
		/* すでに if 処理済み */
		while( pp_skip_if_block() != PP_SKIP_ENDIF );
		pp_endif( 0 );
	} else {
		/* まだヒットしていない */
		WORDBUF buf;

		if( _get_word( buf ) == NULL ){
			// ここからスタート
			pp_if_stack[ pp_if_stack_p - 1 ] = 1;
			if( get_line() == 0 ){
				ERROR( "途中で終わっています.\n" );
			}
		} else if( strcmp( buf, "if" ) == 0 ){
			pp_if_stack_p --;
			pp_if( 0 );
		}
	}
}

static void pp_elseif( int arg )
{
	if( pp_if_stack_p == 0  ){
		ERROR( "#ifが対応していない#elseifです。\n" );
	}
	
	if( pp_if_stack[ pp_if_stack_p - 1 ] ){
		/* すでに if 処理済み */
		while( pp_skip_if_block() != PP_SKIP_ENDIF );
		pp_endif( 0 );
	} else {
		/* まだヒットしていない */
		if( pp_if_stack_p == 0  ){
			ERROR( "#ifが対応していない#elseifです。\n" );
		}
		pp_if_stack_p --;
		pp_if( arg );
	}
}

static void pp_endif( int arg )
{
	WORDBUF buf;

	if( _get_word( buf ) != NULL ){
		ERROR( "余分な文字列があります\n" );
	}
	if( pp_if_stack_p == 0  ){
		ERROR( "#ifが対応していない#endifです。\n" );
	}

	pp_if_stack_p --;

	get_line();
}

typedef struct {
	char *key;
	void (*command)( int arg );
	int arg;
} PP_COMMAND;

static PP_COMMAND pp_command[] = {
	{ "include", pp_include, 0 },
	{ "define", pp_define, 0 },
	{ "if", pp_if, 0 },
	{ "ifdef", pp_if, 1 },
	{ "ifndef", pp_if, 2 },
	{ "else", pp_else, 0 },
	{ "elseif", pp_elseif, 0 },
	{ "elseifdef", pp_elseif, 1 },
	{ "elseifndef", pp_elseif, 2 },
	{ "elif", pp_elseif, 0 },
	{ "elifdef", pp_elseif, 1 },
	{ "elifndef", pp_elseif, 2 },
	{ "endif", pp_endif, 0 },
	{ "enum", pp_enum, 0 },
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
	if( depend_log_fp != NULL ){
		return buf;
	}
	ERROR( "# 命令が不正です\n" );

	return buf;
}

char *get_word_pp( char *buf )
{
	if( __get_word( buf ) == NULL ){
		return NULL;
	}
	if( buf[ 0 ] != '#' ) return buf;

	/* プリプロセッサ処理 */
	if( _get_word( buf ) == NULL ){
		// 無視
		return buf;
//		ERROR( "# 命令が不正です\n" );
	}

	if( do_pp_command( buf ) ){
		return get_word( buf );
	}
	// 無視
//	ERROR( "# 命令が不正です\n" );

	return buf;
}

char *get_word_no_pp( char *buf )
{
	return _get_word( buf );
}

/* ----------------------------------------------- */
/*
	初期化、終了
*/


void parse_init( void )
{
	pp_if_stack_p = 0;
	
	current = NULL;
	parse_normal_mode();
	init_parse_files();
}

void parse_end( void )
{
	int i;
	if( pp_if_stack_p > 0 ){
		ERROR( "#if が閉じていません。\n" );
	}
	current = NULL;

	for( i = 0; i < max_parse_files; i++ ){
		my_free( parse_files[ i ] );
	}
}

void parse_init_define( void )
{
	init_define_buf();
}

void parse_free_define( void )
{
	free_define_buf();
}

/**
	deineのチェックモードを設定する。
	現状は未サポート

	@todo 削除する？
*/
void parse_check_define_mode(
	 int mode  ///< 0 ならd:のチェックを行なわない。
)
{
	define_check_flag = mode;
}

#if 0
static char *save_ptr = NULL;
static char save_line_buffer[ LINE_BUFFER_SIZE ];
#endif

void set_parse_buffer( char *ptr )
{
	PARSE_INFO *newb;
	int len;
PRINTF( "SET_PARSE_BUFFER\n" );

	len = strlen( ptr ) + 1;

	newb = my_malloc( sizeof( PARSE_INFO ) + len );
	newb->parent = current;

	newb->line_buffer = ( char * )newb + sizeof( PARSE_INFO );
	newb->line_buffer_size = len;

	strcpy( newb->line_buffer, ptr );

	newb->back_ptr = newb->ptr = newb->line_buffer;
	newb->fp = NULL;
	newb->fileno = current->fileno;
	newb->line = current->line;
	newb->continue_line_flag = 0;
	newb->def_buffer_flag = 0;
	newb->pp_if_stack_level = pp_if_stack_p;
	current = newb;
}

void reset_parse_buffer( void )
{
	PARSE_INFO *cur;
PRINTF( "RESET_PARSE_BUFFER\n" );
	if( current->fp != NULL ){
		ERROR( "reset_parse_bufferが不正です\n" );
	}
	cur = current;
	current = cur->parent;

	my_free( cur );
}

char *parse_get_current_filename( void )
{
	return get_parse_filename( current->fileno );
}

int parse_get_current_linenum( void )
{
	return current->line;
}

char *parse_get_current_linebuf( void )
{
	return current->line_buffer;
}
