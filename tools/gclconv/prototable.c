/*
   gclconv プロトタイプテーブル作成
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "gclconv.h"
#include "expr.h"
#include "mymalloc.h"
#include "hash.h"
#include "parse.h"

#define MAX_LINE_BUF	1024

/* -------------------- テーブル関連ルーチン ------------------- */

#define MAX_COMMAND_INIT	512
#define MAX_COMMAND_ADD		512

static int command_table_p;
static int max_command;
static COMMAND_TABLE *command_table;

static COMMAND_TABLE_TAG current_tag;

static void table_init( void )
{
	command_table_p = 0;
	max_command = MAX_COMMAND_INIT;

	command_table = my_malloc( max_command * sizeof( COMMAND_TABLE ) );
}

static COMMAND_TABLE *new_command( void )
{
	COMMAND_TABLE *new;

	if( command_table_p + 1 >= max_command ){
		max_command = max_command + MAX_COMMAND_ADD;
		command_table = my_realloc( command_table, max_command * sizeof( COMMAND_TABLE ) );
	}
	new = &command_table[ command_table_p ++ ];
	memset( new, 0, sizeof( COMMAND_TABLE ) );
	return new;
}

static void unget_command( void )
{
	/* 直前に確保されたコマンドテーブルを廃棄する */
	command_table_p --;
}

static char *set_string( char *string )
{
	char *p;

	p = Malloc( strlen( string ) + 1 );
	strcpy( p, string );

	return p;
}

/* ---------------------------------------------------------------------- */
/*
	qsort, bsearch
*/

static struct {
	COMMAND_TABLE *top;
	int num;
} search_type_table[ COMMAND_TYPE_MAX ];

static int tbl_compare( const void *ptr1, const void *ptr2 )
{
	const COMMAND_TABLE *p1 = ptr1;
	const COMMAND_TABLE *p2 = ptr2;

	if( p1->type != p2->type ){
		return p1->type - p2->type;
	}
	switch( p1->type ){
	  case COMMAND_TYPE_NORMAL:
		return strcmp( p1->name, p2->name );
	  case COMMAND_TYPE_CHARA:
	  case COMMAND_TYPE_COMMAND:
	  case COMMAND_TYPE_BLOCK:
	  case COMMAND_TYPE_EXTERN:
		return strcmp( p1->class, p2->class );
	  case COMMAND_TYPE_MESG:
		{
			int r;
			if( ( r = strcmp( p1->class, p2->class ) ) == 0 ){
				return strcmp( p1->command, p2->command );
			} else {
				return r;
			}
		}
	}
	return 0;
}

static void sort_table( void )
{
	qsort( command_table, command_table_p, sizeof( COMMAND_TABLE ), tbl_compare );

	{
		int i, n, p, t;
		COMMAND_TABLE *top;
		p = 0; n = 0; t = 0;
		top = command_table;
		for( i = 0; i < command_table_p; i++ ){
			if( t != command_table[ i ].type ){
				while( p < command_table[ i ].type ){
					search_type_table[ p ].top = top;
					search_type_table[ p ].num = n;
					top = &command_table[ i ];
					p++;
					n = 0;
				}
				t = command_table[ i ].type;
				n = 1;
			} else {
				n++;
			}
		}
		search_type_table[ p ].top = top;
		search_type_table[ p ].num = n;
	}
}

static COMMAND_TABLE *search_table( int type, char *name, char *class, char *command )
{
	COMMAND_TABLE key;

	key.type = type;
	key.name = name;
	key.class = class;
	key.command = command;

	return bsearch( &key, search_type_table[ type ].top, search_type_table[ type ].num
					, sizeof( COMMAND_TABLE ), tbl_compare );
}

/* -------------------- PROC からの呼び出しテーブル ------------------ */
/*
   PROC はコード化されない可能性があるので、CALL時にテーブルを作成しておく。
*/

#define MAX_PROC_CALL_BUFFER_SIZE	(16*1024)

typedef struct _command_call_table {
	struct _command_call_table *next;
	int proc_id;
	int mask;
	COMMAND_TABLE *command;
	int call;					// for Check
	int _line;					// for Check
	char *_file;				// for Check
} COMMAND_CALL_TAG;

typedef struct _proc_call_tag {
	struct _proc_call_tag *next;
	int proc_id;
	COMMAND_CALL_TAG *calls;
} PROC_CALL_TAG;

static MLTAG *pt_tag;
static HASH_INFO *pt_hash;

static PROC_CALL_TAG *proc_call_tag;
static PROC_CALL_TAG *current_call_tag;

static void proc_call_table_init( void )
{
	pt_tag = tMalloc_Create( MAX_PROC_CALL_BUFFER_SIZE );
	pt_hash = hash_create( HASH_DEFAULT_SIZE, HASH_KEY_INT );

	proc_call_tag = NULL;
}

static COMMAND_CALL_TAG *new_proc_call( int proc_id )
{
	HASH_RESULT *result;
	PROC_CALL_TAG *cur;
	COMMAND_CALL_TAG *call;

	if( current_call_tag != NULL && current_call_tag->proc_id == proc_id ){
		cur = current_call_tag;
	} else if( ( result = hash_search_id( pt_hash, proc_id ) ) != NULL ){
		// PROC_CALL_TAGは見つかった
		cur = current_call_tag = HASH_VALUE( result );
	} else {
		cur = tMalloc( pt_tag, sizeof( *cur ) );
		memset( cur, 0, sizeof( *cur ) );
		cur->proc_id = proc_id;
		cur->next = proc_call_tag;
		proc_call_tag = cur;
		current_call_tag = cur;
		// HASHに登録
		if( hash_entry_id( pt_hash, proc_id, cur ) != NULL ){
			ERROR( "ct : HASHに登録できません\n" );
		}
	}

	/* 新規コール情報を追加する */

	call = tMalloc( pt_tag, sizeof( *call ) );
	memset( call, 0, sizeof( *call ) );

	call->next = cur->calls;
	cur->calls = call;

	return call;
}

/* ------------------------------ タグがらみ ----------------------------- */

static int table_omit_flag = TABLE_TAG_NORMAL;

static void init_table_tag( void )
{
	current_tag.prev = current_tag.next = &current_tag;
	current_tag.type = GCL_END;
}

static void add_table( COMMAND_TABLE_TAG *tag )
{
	/* 最後につける */
	COMMAND_TABLE_TAG *base;

	base = &current_tag;

	tag->prev = base->prev;
	base->prev->next = tag;
	base->prev = tag;
	tag->next = base;
}

static COMMAND_TABLE_TAG *set_table( int c )
{
	COMMAND_TABLE_TAG *new;

	new = Malloc( sizeof( COMMAND_TABLE_TAG ) );
	new->type = c;
	new->flag = table_omit_flag;
	add_table( new );

	return new;
}

static void set_str_table( int c, char *str )
{
	COMMAND_TABLE_TAG *new;

	new = Malloc( sizeof( COMMAND_TABLE_TAG ) + strlen( str ) + 1 );
	new->type = c;
	new->flag = table_omit_flag;
	strcpy( new->data, str );
	add_table( new );
}

static void set_option( int c, int option, char *string )
{
	COMMAND_TABLE_TAG *new;

	new = Malloc( sizeof( COMMAND_TABLE_TAG ) + sizeof( char ) + strlen( string ) + 1 );
	new->type = c;
	new->flag = table_omit_flag;
	new->data[ 0 ] = option;
	if( strlen( string ) > 1 ){
		char *p, *s;
		p = &( new->data[ 1 ] ); s = string;

		while( ( *( p++ ) = tolower( *( s++ ) ) ) != '\0' ){
			;
		}
	} else {
		new->data[ 1 ] = '\0';
	}
	add_table( new );
}

static COMMAND_TABLE_TAG *get_current_table_tag( void )
{
	COMMAND_TABLE_TAG *base, *top;

	base = &current_tag;

	top = base->next;
	base->prev->next = NULL;
	top->prev = NULL;

	return top;
}

/* ------------ コマンドライン解釈部 ----------- */

static void skip_command_line( void )
{
	WORDBUF buf;
	while( get_word( buf ) != NULL );
}

/* オプション処理 */

static char option_check_buf[ 128 ];

static void init_option_check( void )
{
	int i;
	for( i = 0; i < 128; i++ ){
		option_check_buf[ i ] = 0;
	}
}

static int option_used_check( char c )
{
	int cc;
	cc = ( unsigned char )c;
	if( cc < 0x20 || cc >= 0x80 ){
		ERROR( "不正なオプション文字です\n" );
	}
	if( option_check_buf[ cc ] ){
		ERROR( "オプション文字'%c'が重複しています。\n", cc );
	}
	option_check_buf[ cc ] = 1;
	return 1;
}

static int get_option_letter( char *string )
{
	int option;
	char *p;

	option = string[ 0 ];

	for( p = string; *p != '\0'; p ++ ){
		if( isupper( *p ) ){
			return *p;
		}
	}
	return option;
}

/* 変数型宣言処理 */

static void analyze_variable( char *str )
{
	/* str は、$s:名前 のような文字列 */

	switch( str[ 1 ] ){
	  case 'i':
		/* 32ビット数値 */
		set_table( GCL_INT );
		break;
	  case 'w':
		/* 16ビット数値 */
		set_table( GCL_SHORT );
		break;
	  case 'b':
		/* 8ビット数値 */
		set_table( GCL_BYTE );
		break;
	  case 'c':
		/* 8ビット文字 */
		set_table( GCL_CHAR );
 		break;
	  case 'f':
		/* 1ビットフラグ */
		set_table( GCL_BOOL );
		break;
	  case 's':
		/* 24ビット文字コード */
		/* 名前空間チェック用にプロトタイプ名を記録 */
		set_str_table( GCL_STRID, str + 3 );
		break;
	  case 'S':
		/* 24ビット文字コード + 8 BitID*/
		/* 名前空間チェック用にプロトタイプ名を記録 */
		set_str_table( GCL_STRID2, str + 3 );
		break;
	  case 'v':
		/* 32ビットベクトル */
/*		set_table( GCL_VECTOR );*/
		/* ベクトル型はint 3つとして変換 */
		set_table( GCL_INT );
		set_table( GCL_INT );
		set_table( GCL_INT );
		break;
	  case 'p':
		/* 関数ID(24Bit) */
		set_table( GCL_PROCID );
		break;
	  case 'r':
		set_table( GCL_VAR );
		break;
	  case 'd':
		set_table( GCL_STRRES );
		break;
	  case 't':
		/* tsymbol(int) */
		set_table( GCL_SYMBOL );
		break;
	  case 'T':
		/* tsymbol2(int) */
		set_table( GCL_SYMBOL2 );
		break;
	  case 'C':
		/* tsymbol3(int) */
		set_table( GCL_SYMBOL3 );
		break;
	  case 'E':
		/* EXTCHARA */
		set_table( GCL_EXTCHARA );
		break;
	}
}

static int analyze_command_line( COMMAND_TABLE *cp )
{
	WORDBUF buf;

	init_table_tag();
	init_option_check();

	while( get_word( buf ) != NULL ){
		switch( buf[ 0 ] ){
		  case '[':
			/* 省略可能な宣言 はじまり */
			table_omit_flag |= TABLE_TAG_OMITABLE;
			break;
		  case ']':
			/* 省略可能な宣言 終り */
			table_omit_flag &= ~TABLE_TAG_OMITABLE;
			break;
		  case '$':
			/* 型宣言 */
			analyze_variable( buf );
			break;
		  case '-':
			/* オプション宣言 */
			{
				int option;
				option = get_option_letter( &( buf[ 1 ] ) );
				option_used_check( option );

				set_option( GCL_OPTION, option, &( buf[ 1 ] ) );
			}
			break;
		  case '{':
			{
				/* 実行ブロック */
				int set_arg_flag;
				int imm_flag;
				char *buffer;
				char *p;

				imm_flag = 0;
				set_arg_flag = 0;
				buffer = my_malloc( 1024 );
				buffer[ 0 ] = '\0';
				p = buffer;
				for( ;; ){
					if( get_word( buf ) == NULL ){
						ERROR( "実行ブロックが終わっていません\n" );
						exit( 1 );
					}
					if( buf[ 0 ] == '}' ){
						break;
					}
					if( buf[ 0 ] == '$' ){
						if( imm_flag ){
							ERROR( "即時実行ブロックにはパラメータは渡せません\n" );
						}
						if( buf[ 1 ] == ':' ){
							sprintf( p, "%s ", buf );
							p = p + strlen( buf ) + 1;
							set_arg_flag = 1;
						} else {
							ERROR( "形式は $:変数名 です\n" );
						}
					} else if( strcmp( buf, "imm" ) == 0 ){
						/* 即時実行ブロック */
						if( set_arg_flag ){
							ERROR( "即時実行ブロックにはパラメータは渡せません\n" );
						}
						sprintf( p, "imm" );
						p = p + strlen( p );
						imm_flag = 1;
					} else if( set_arg_flag ){
						ERROR( "変数宣言に無効な文字列があります\n" );
					}
				}
				if( p >= buffer + 1024 ){
					ERROR( "変数宣言が長過ぎます\n" );
				}
PRINTF( "BLOCK ARG LIST = %s\n", buffer );
				set_str_table( GCL_BLOCK, buffer );
				free( buffer );
			}
			break;
		  case '(':
			/* 式 */
			set_table( GCL_EXPR );
			for( ;; ){
				if( get_word( buf ) == NULL ){
					ERROR( "式が終わっていません\n" );
					exit( 1 );
				}
				if( buf[ 0 ] == ')' ){
					break;
				}
			}
			break;
		  case '\'':
			/* 文字列 */
			if( buf[ 1 ] == 'E' ){
				set_table( GCL_EXTCHARA2 );
			} else {
				set_table( GCL_STRING );
			}
#if 0
			for( ;; ){
				if( get_word( buf ) == NULL ){
					ERROR( "文字列が終わっていません\n" );
					exit( 1 );
				}
				if( buf[ 0 ] == '\'' ){
					break;
				}
			}
#endif
			break;
		  case '*':
			/* ワイルドカード */
			set_table( GCL_WILD );
			break;
		  case '.':
			/* 直前の任意個数の繰り返し */
			if( strncmp( buf, "...", 3 ) == 0 ){
				if( buf[ 3 ] != '\0' ){
					int c;
					c = atoi( buf + 3 );
					if( c > 15 || c <= 0 ){
						ERROR( "繰り返し回数が異常です\n" );
					}
					PRINTF( "REPEAT %d\n", c );
					set_table( GCL_REPEAT | c );
				} else {
					set_table( GCL_REPEAT | 1 );
				}
				break;
			}
		  default:
			WARNING( "コマンド定義に無意味な文字列\'%s\'があります\n", buf );
			break;
		}
	}
	set_table( GCL_END );

	if( table_omit_flag != TABLE_TAG_NORMAL ){
		ERROR( "省略ブロック[]が閉じていません\n" );
	}

	cp->tag = get_current_table_tag();

	return 1;
}

/* ------------- コマンド登録部 ------------- */

static int is_new_mesg_command( char *class, char *command, int code )
{
	int i;

	for( i = 0; i < command_table_p; i++ ){
		if( strcmp( command_table[ i ].name, MESG_COMMAND ) == 0
		  && strcmp( command_table[ i ].class, class ) == 0 ){
			if( command_table[ i ].command_id == code ){
				if( strcmp( command_table[ i ].command, command ) == 0 ){
					return 0;
				} else {
					WARNING( "mesgコマンド名のコマンドIDが%sとバッティングしています\n"
							, command_table[ i ].command );
					return 0;
				}
			}
		}
	}
	return 1;
}

static int is_new_chara_command( char *class )
{
	int i;
	int id;

	id = get_strcode( class );

	for( i = 0; i < command_table_p; i++ ){
		if( command_table[ i ].name[ 0 ] != 'c' ) continue;
		if( ( strcmp( command_table[ i ].name, CHARA_COMMAND ) == 0 )
			|| ( strcmp( command_table[ i ].name, CMD_COMMAND ) == 0 ) ){
			if( command_table[ i ].class_id == id ){
				if( strcmp( command_table[ i ].class, class ) == 0 ){
					return 0;
				} else {
					WARNING( "キャラ名の文字列IDが%sとバッティングしています\n"
							, command_table[ i ].class );
					return 0;
				}
			}
		}
	}
	return 1;
}

static int is_new_command( char *command )
{
	int i;

	for( i = 0; i < command_table_p; i++ ){
		if( strcmp( command_table[ i ].name, command ) == 0 ){
			return 0;
		} else if( command_table[ i ].name_id == get_strcode( command ) ){
				WARNING( "コマンド名の文字列IDが%sとバッティングしています\n"
						, command_table[ i ].name );
				return 0;
		}
	}
	return 1;
}

static int is_new_extern_command( int id )	/* 外部コマンドはclass IDで区別 */
{
	int i;

	for( i = 0; i < command_table_p; i++ ){
		if( command_table[ i ].class_id == id ){
			return 0;
		}
	}
	return 1;
}

static int set_command( char *command )
{
	COMMAND_TABLE *cp;
PRINTF( "set %s\n", command );
	if( command[ 0 ] == '-' ){
		ERROR( "%s:オプション文字です\n", command );
	}
	if( strcmp( command, CHARA_COMMAND ) == 0
		|| strcmp( command, CMD_COMMAND ) == 0
		|| strcmp( command, BLOCK_COMMAND ) == 0 ){

		/* 外部コマンド */
		char *class;
		WORDBUF cbuf;

		if( ( class = get_word( cbuf ) ) == NULL || *class == '\0' ){
			ERROR( "charaにクラスの指定がありません\n" );
			exit( 1 );
		}
		if( ! is_new_chara_command( class ) ){
			WARNING( "chara:%s すでに登録されています。無視します。\n", class );
			return 0;
		}
		cp = new_command();

		cp->name = set_string( command );
		cp->name_id = get_strcode( command );
		cp->class = set_string( class );
		cp->class_id = get_strcode( class );

		if( strcmp( command, CHARA_COMMAND ) == 0 ){
			cp->type = COMMAND_TYPE_CHARA;
		} else if( strcmp( command, CMD_COMMAND ) == 0 ){
			cp->type = COMMAND_TYPE_COMMAND;
		} else if( strcmp( command, BLOCK_COMMAND ) == 0 ){
			cp->type = COMMAND_TYPE_BLOCK;
		}

PRINTF( "%s %s %X\n", command, cp->class, cp->class_id );
		/* プログラム名を入れる */
		cp->command = NULL;
		if( get_word( cbuf ) != NULL ){
			if( cbuf[ 0 ] == '[' ){
				get_word( cbuf );
				cp->command = set_string( cbuf );
				get_word( cbuf );
				if( cbuf[ 0 ] != ']' ){
					ERROR( "[]のなかは関数名のみです\n" );
				}
			} else {
				unget_word( cbuf );
			}
		}
	} else if( strcmp( command, MESG_COMMAND ) == 0 ){
		/* mesg コマンド */
		char *class;
		WORDBUF cbuf1, cbuf2, buf;
		int code, codesetf;

		if( ( ( class = get_word( cbuf1 ) ) == NULL ) || *class == '\0' ){
			ERROR( "mesg はキャラクタ名が必要です\n" );
		}
		if( ( get_word( cbuf2 ) == NULL ) || cbuf2[ 1 ] != 's' ){
			ERROR( "mesg は$s:名前で宣言してください\n" );
		}
		if( get_word( cbuf2 ) == NULL ){	
			ERROR( "mesg:コマンド定義が必要です\n" );
		}

		code = get_strcode( cbuf2 );
		codesetf = 0;
		if( get_word( buf ) != NULL ){
			if( buf[ 0 ] == '[' ){
				get_word_ex( buf );
				if( ! is_expr_num( buf, &code ) ){
					ERROR( "コマンドコードは数字のみです\n" );
				}
				get_word_ex( buf );
				if( buf[ 0 ] != ']' ){
					ERROR( "コマンドコードは数字1つのみです\n" );
				}
				codesetf = 1;
			} else {
				unget_word( buf );
			}
		}

		if( codesetf == 0 && ! is_new_mesg_command( cbuf1, cbuf2, code ) ){
			WARNING( "mesg:%s すでに登録されています。無視します。\n", command );
			return 0;
		}
		cp = new_command();
		cp->name = set_string( command );
		cp->name_id = get_strcode( command );
		cp->class = set_string( class );
		cp->class_id = get_strcode( class );

		cp->command = set_string( cbuf2 );
		cp->command_id = code;
		cp->type = COMMAND_TYPE_MESG;

PRINTF( "mesg %s %s[%d]\n", cp->class, cp->command, cp->command_id );
	} else {
		/* 新規コマンド */
		if( ! is_new_command( command ) ){
			WARNING( "%s すでに登録されています。無視します。\n", command );
			return 0;
		}
		cp = new_command();
		cp->name = set_string( command );
		cp->name_id = get_strcode( command );
		cp->type = COMMAND_TYPE_NORMAL;
PRINTF( "command %s %X\n", cp->name, cp->name_id );
	}

	if( ! analyze_command_line( cp ) ){
		unget_command();
		return 0;
	}
	return 1;
}

/* -------------- コマンドテーブルの参照処理 ------------- */

#define CALL_EXEC	1
#define CALL_TARGET	2

void command_proc_call_set( int proc_id )
{
	HASH_RESULT *result;
	if( ( result = hash_search_id( pt_hash, proc_id ) ) != NULL ){
		PROC_CALL_TAG *ptag = HASH_VALUE( result );
		COMMAND_CALL_TAG *cp;

		for( cp = ptag->calls; cp != NULL; cp = cp->next ){
			cp->command->call_flag |= cp->mask;
			cp->call = 1;
		}
	}
}

static void *set_call_flag( COMMAND_TABLE *cmdtbl, int flag, int call_proc )
{
	COMMAND_CALL_TAG *cp;

	cp = new_proc_call( call_proc );
	cp->mask = flag;
	cp->command = cmdtbl;

#if 1	// check用
	// PARSE.Cに問い合わせて現在のfileとラインを取得する
	cp->_file = parse_get_current_filename();
	cp->_line = parse_get_current_linenum();
#endif
	if( call_proc == 0 ){
		// proc外からの呼び出しなので、問答無用でCALL扱い
		cmdtbl->call_flag |= flag;
	}
	return cmdtbl;
}

COMMAND_TABLE *get_extern_command_table( char *command, int call_proc )
{
	COMMAND_TABLE *table;
	table = search_table( COMMAND_TYPE_NORMAL, command, NULL, NULL );
	if( table != NULL ){
		set_call_flag( table, CALL_EXEC, call_proc );
	}
	return table;
}


COMMAND_TABLE *get_chara_command_table( char *class, int call_proc )
{
	COMMAND_TABLE *table;
	table = search_table( COMMAND_TYPE_CHARA, NULL, class, NULL );
	if( table != NULL ){
		set_call_flag( table, CALL_EXEC, call_proc );
	}
	return table;
}

COMMAND_TABLE *get_cmd_command_table( char *class, int call_proc )
{
	COMMAND_TABLE *table;
	table = search_table( COMMAND_TYPE_COMMAND, NULL, class, NULL );
	if( table != NULL ){
		set_call_flag( table, CALL_EXEC, call_proc );
	}
	return table;
}

COMMAND_TABLE *get_block_command_table( char *class )
{
	COMMAND_TABLE *table;
	table = search_table( COMMAND_TYPE_BLOCK, NULL, class, NULL );
	return table;
}

COMMAND_TABLE *get_mesg_command_table( char *class, char *command, int call_proc )
{
	COMMAND_TABLE *table;
	COMMAND_TABLE *chara;

	table = search_table( COMMAND_TYPE_MESG, NULL, class, command );
	/* 相手先のキャラに対して参照フラグをつける */
	chara = search_table( COMMAND_TYPE_CHARA, NULL, class, NULL );
	if( chara != NULL ){
		set_call_flag( chara, CALL_TARGET, call_proc );
	}
	return table;
}

/* -------------- 外部参照キャラ登録処理 -------------- */
/*
	ポリゴンデモなど、ステージのキャラクタリストを外部参照するための処理
*/

#define EXTERNAL_CHARA_LIST_MAX	256

static char *external_chara_list_filename[ EXTERNAL_CHARA_LIST_MAX ];
static int external_chara_list_p = 0;

static char external_chara_list_path[ 256 ] = ".";

static void init_external_chara_list( void )
{
	external_chara_list_p = 0;
}

void set_external_chara_list_path( char *path )
{
	strcpy( external_chara_list_path, path );
}

void set_external_chara_list( char *filename )
{
	if( external_chara_list_p >= EXTERNAL_CHARA_LIST_MAX ){
		ERROR( "EXTERN CHARAが多過ぎます\n" );
	}
	external_chara_list_filename[ external_chara_list_p ] = set_string( filename );
	external_chara_list_p ++;
}

static void set_external_chara_file( void )
{
	int i;
	char *buffer;

	buffer = my_malloc( MAX_LINE_BUF );

	for( i = 0; i < external_chara_list_p; i++ ){
		FILE *fp;
		char filename[ 256 ];

		sprintf( filename, "%s/%s.lst", external_chara_list_path
				 , external_chara_list_filename[ i ] );
		PRINTF( "READ EXTERN FILE %s\n", filename );
		if( ( fp = fopen( filename, "rt" ) ) == NULL ){
			WARNING( "file %s が見つかりません\n", filename );
			continue;
		}

		while( !feof( fp ) ){
			WORDBUF id_str, func_str, name_str;
			int id;
			COMMAND_TABLE *cp;

			if( fgets( buffer, MAX_LINE_BUF, fp ) == NULL ) break;

			if( buffer[ 0 ] == '#' || buffer[ 0 ] == '/' ) continue;
			if( sscanf( buffer, "%s %s %s", id_str, func_str, name_str ) != 3 ){
				continue;
			}

			PRINTF( "%s %s %s\n", id_str, func_str, name_str );

			if( ! is_expr_num( id_str, &id ) ){
				FATAL( "external chara list: WRONG ID %s\n", id_str );
			}

			if( ! is_new_extern_command( id ) ){
				continue;
			}

			cp = new_command();

			cp->name = set_string( EXTERN_COMMAND );
			cp->name_id = 0;
			cp->class_id = id;
			cp->class = set_string( name_str );
			cp->command = set_string( func_str );
			cp->call_flag = 1;
			cp->type = COMMAND_TYPE_EXTERN;
			PRINTF( "SET EXTERN %X %s %s\n", cp->class_id, cp->command, cp->class );
		}

		fclose( fp );
	}

	free( buffer );
}

/* -------------- コマンドテーブルへの登録処理 ------------- */

void setup_table( char *table_file )
{
	table_init();
	proc_call_table_init();
	init_external_chara_list();

	parse_init();

	if( ! load_file( table_file ) ){
		FATAL( "Error:コマンドテーブルファイルが見つかりませんでした。\n" );
	}

	while( get_line() != 0 ){
		/* 行頭 */
		char *command;
		WORDBUF buf;
		if( ( command = get_word( buf ) ) != NULL ){
			if( *command != '\0' ){
				if( ! set_command( command ) ){
					WARNING( "コマンド %s 登録エラー\n", command );
					skip_command_line();
				}
			}
		}
	}
	parse_end();

	sort_table();

	return;
}

/* -------------- stage用 .c 出力 --------------- */

enum {
	OUTPUT_EXTERN,
	OUTPUT_TABLE,
	OUTPUT_LIST,
};

void set_all_chara_output( void )
{
	int i;

	COMMAND_TABLE *tp;

	tp = command_table;
	for( i = 0; i < command_table_p; i++, tp++ ){
		tp->call_flag |= 1;
	}
}

static int sort_cmp( const void *p1, const void *p2 )
{
	COMMAND_TABLE *cp1, *cp2;

	cp1 = ( COMMAND_TABLE * )p1;
	cp2 = ( COMMAND_TABLE * )p2;

	return cp1->class_id - cp2->class_id;
}

static void output_chara_list( FILE *outfp, int phase )
{
	int i;
	COMMAND_TABLE *tp;
	int num;

	qsort( command_table, command_table_p, sizeof( COMMAND_TABLE ), sort_cmp );

	tp = command_table;

	num = 0;
	for( i = 0; i < command_table_p; i++, tp++ ){
		if( ( strcmp( tp->name, CHARA_COMMAND ) == 0 )
			|| ( strcmp( tp->name, CMD_COMMAND ) == 0 )
			|| ( strcmp( tp->name, EXTERN_COMMAND ) == 0 ) ){
if( phase == OUTPUT_TABLE ){
	PRINTF( "CHTB %X %X %s\n", tp->class_id, tp->call_flag, tp->command );
}
			if( tp->call_flag & CALL_EXEC ){
				if( tp->command != NULL ){
					if( phase == OUTPUT_EXTERN ){
						fprintf( outfp, "extern NEWCHARA %s;\n", tp->command );
					} else if( phase == OUTPUT_TABLE ){
						fprintf( outfp, "\t/* %s %s */\n\t\t{ 0x%08X, %s },\n"
								 , tp->name, tp->class, tp->class_id, tp->command );
					} else if( phase == OUTPUT_LIST ){
						fprintf( outfp, "%s\t%s [%s]\n", tp->name, tp->class, tp->command );
					}
				}
				num ++;
			} else if( tp->call_flag & CALL_TARGET ){
				/* mesg はあるが、chara がない */
				WARNING( "起動されていないキャラ %s に送っているメッセージがあります\n"
					   ,tp->class );
			}
		}
	}
	if( phase == OUTPUT_EXTERN ){
		fprintf( outfp, "/* %d functions */\n", num );
	}
}

void print_chara_list( void )
{
	output_chara_list( stdout, OUTPUT_LIST );
}

void dump_chara_function( char *dump_function )
{
	PROC_CALL_TAG *pt;

	for( pt = proc_call_tag; pt != NULL; pt = pt->next ){
		COMMAND_CALL_TAG *cp;
		for( cp = pt->calls; cp != NULL; cp = cp->next ){
			if( cp->call || ( cp->proc_id == 0 ) ){
				COMMAND_TABLE *tp;
				if( ( tp = cp->command ) != NULL ){
					if( ( strcmp( tp->name, CHARA_COMMAND ) == 0
						 || strcmp( tp->name, CMD_COMMAND ) == 0 )
					   && strcmp( tp->command, dump_function ) == 0 ){
						printf( "%s:%d: %s\n", cp->_file, cp->_line, tp->command );
					}
				}
			}
		}
	}
}

#define TMP_FILE_NAME "__stage.###"

#define START_KEY	"/* gcl chara */"
#define END_KEY	"/* gcl chara end */"

void output_stage_file( char *infile, char *outfile )
{
	FILE *infp, *outfp;
	char *buffer;
	int phase = OUTPUT_EXTERN;

	set_external_chara_file();

	if( ( infp = fopen( infile, "rt" ) ) == NULL ){
		/* オープンできない */
		WARNING( "file %s がオープンできません\n", infile );
		return;
	}
	if( ( outfp = fopen( TMP_FILE_NAME, "wb" ) ) == NULL ){
		/* オープンできない */
		FATAL( "file %s がオープンできません\n", TMP_FILE_NAME );
	}

	buffer = my_malloc( MAX_LINE_BUF );

	while( ! feof( infp ) ){
		if( fgets( buffer, MAX_LINE_BUF, infp ) == NULL ) break;
		fputs( buffer, outfp );
		{
			char *p;
			p = strchr( buffer, '/' );
			if( p != NULL ){
				if( strncmp( p, START_KEY, strlen( START_KEY ) ) == 0 ){
					output_chara_list( outfp, phase );
					phase ++;
					/* 終了タグが出てくるまでスキップ */
					for( ;; ){
						char *p;
						fgets( buffer, 1024, infp );
						p = strchr( buffer, '/' );
						if( p != NULL && strncmp( p, END_KEY, strlen( END_KEY ) ) == 0 ){
							break;
						}
						if( feof( infp ) ){
							FATAL( ".c gcl chara 終了タグが検出できません\n" );
						}
					}
					fputs( buffer, outfp );
				}
			}
		}
	}
	fclose( infp );
	fclose( outfp );

	if( phase < OUTPUT_LIST ){
		ERROR( "%sが壊れています。消して作り直して下さい。\n", infile );
	}

	sprintf( buffer, "mv %s %s", TMP_FILE_NAME, outfile );
	system( buffer );
	my_free( buffer );
}
