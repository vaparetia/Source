/*
	gclconv
		convert.c 変換メインルーチン

	1999/06/15 K.Uehara
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gclconv.h"
#include "mymalloc.h"
#include "hash.h"
#include "expr.h"
#include "fontconv.h"
#include "gcoform.h"


extern int block_output_mode;

static char current_block_name[ 128 ];	/* ブロックモード用 */

#define PROC_UNDEFINE	0
#define PROC_DEFINE		1
#define PROC_REFER		2
#define PROC_OK			3
#define PROC_REFDONE	4
#define PROC_EXTEND		8

typedef struct _command_proc {
	struct _command_proc *next;		// 次へのポインタ
	struct _command_proc *sub;		// 番号つきのものへのポインタ	
	int prefix_no;					// 番号つきの場合はその番号
	int id;
	int flag;
	int size;
	int argnum;
	int local_argnum;
	int body;
	int refer_num;
	int refer_max;
	struct _command_proc **refer_p;
	char name[ 0 ];
} COMMAND_PROC;

/* ----------------------------------------------------- */
/*
	バッファリング関数

	realloc されることを前提に
	先頭からのオフセット値でアクセス。
*/

#define BUF_REALLOC_SIZE	(16*1024)

#define BODY_BUFFER_SIZE	(64*1024)
#define VALUE_BUFFER_SIZE	(64*1024)
#define PROC_BUFFER_SIZE	(64*1024)
#define STRING_BUFFER_SIZE	(64*1024)

typedef struct {
	char *buffer;
	int which;	// body_buf なら0, proc_buf なら-1 そのほかなら -2
	int now_p;
	int size;
} BUFFER_INFO;

static BUFFER_INFO body_buf;	/* データ本体 */
static BUFFER_INFO proc_buf;	/* proc定義バッファ */
static BUFFER_INFO value_buf;	/* 変数バッファ */
static BUFFER_INFO string_buf;	/* 文字列リソースバッファ */

#define BUF_WHICH_BODY  0
#define BUF_WHICH_PROC	-1
#define BUF_WHICH_NONE	-2

static void resize_buffer( BUFFER_INFO *bi, int size_step )
{
	int size;
	size = bi->size + size_step;
	bi->buffer = my_realloc( bi->buffer, size );
	bi->size = size;
}

static void init_buffer( BUFFER_INFO *bi, int size, int which )
{
	bi->buffer = NULL;
	bi->size = 0;
	bi->which = which;
	resize_buffer( bi, BUF_REALLOC_SIZE );

	bi->now_p = 0;
}

static void free_buffer( BUFFER_INFO *bi )
{
	my_free( bi->buffer );
}

static void reset_buffer( BUFFER_INFO *bi )
{
	free_buffer( bi );
	init_buffer( bi, BODY_BUFFER_SIZE, bi->which );
}

static void store_data( BUFFER_INFO *bi, int p, void *data, int size )
{
	int i;

	char *d, *s;
	d = bi->buffer + p;
	s = ( char * )data;
	for( i = 0; i < size; i++ ){
		*( d ++ ) = *( s ++ );
	}
}

static void store_value( BUFFER_INFO *bi, int p, int data, int size )
{
	// LITTLE ENDIAN 前提

	store_data( bi, p, &data, size );
}

static int set_data( BUFFER_INFO *bi, void *data, int size )
{
	int offset;

	while( bi->now_p + size >= bi->size ){
		resize_buffer( bi, BUF_REALLOC_SIZE );
	}
	offset = bi->now_p;
	store_data( bi, bi->now_p, data, size );
	bi->now_p += size;
	return offset;
}

static int set_value( BUFFER_INFO *bi, int data, int size )
{
	// LITTLE ENDIAN 前提

	return set_data( bi, &data, size );
}

static int set_byte( BUFFER_INFO *bi, int data )
{
	char buf;
	buf = data;
	return set_data( bi, &buf, 1 );
}

static int set_short( BUFFER_INFO *bi, int data )
{
	short buf;
	buf = data;

	return set_data( bi, &buf, 2 );
}

static int set_strcode( BUFFER_INFO *bi, int strcode )
{
	int pos;
#ifdef STRCODE16
	
	/* 16Bit */

	pos = set_byte( bi, strcode & 0xff );
	strcode >>= 8;
	pos = set_byte( bi, strcode & 0xff );
#else
	
	/* 24Bit */

	pos = set_byte( bi, strcode & 0xff );
	strcode >>= 8;
	pos = set_byte( bi, strcode & 0xff );
	strcode >>= 8;
	pos = set_byte( bi, strcode & 0xff );
#endif

	return pos;
}

static int set_int( BUFFER_INFO *bi, int data )
{
	int buf;
	buf = data;

	return set_data( bi, &buf, 4 );
}

#if 0	// 使わなくなった
static void store_byte( BUFFER_INFO *bi, int pos, int data )
{
	char buf;
	buf = data;
	store_data( bi, pos, &buf, 1 );
}

static void store_short( BUFFER_INFO *bi, int pos, int data )
{
	unsigned short buf;

	if( data > 0xFFFF ){
		ERROR( "サイズが１６ビットをこえました。\n" );
	}

	buf = data;

	return store_data( bi, pos, &buf, sizeof( short ) );
}

static void store_int( BUFFER_INFO *bi, int pos, int data )
{
	return store_data( bi, pos, &data, sizeof( int ) );
}
#endif


static int get_now_pos( BUFFER_INFO *bi )
{
	return bi->now_p;
}

static int get_block_size( BUFFER_INFO *bi, int top )
{
	return bi->now_p - top;
}

static void set_align( BUFFER_INFO *bi, int align )	// 最大１６
{
	if( get_now_pos( bi ) % align > 0 ){
		static int tmp[ 4 ] = { 0 };

		set_data( bi, tmp, align - get_now_pos( bi ) % align );
	}
}

static int get_buffer_total_size( BUFFER_INFO *bi )
{
	return bi->now_p;
}

static void write_buffer( FILE *fp, BUFFER_INFO *bi );

static void set_number( BUFFER_INFO *bi, int value, int p_type );
static void set_number_minimum( BUFFER_INFO *bi, int value, int maxbyte );

/* -------------------------------------------------------------- */
/*
	変数処理
*/

static long analyze_expr( BUFFER_INFO *bi, char term_char );

/*
	配列定義用
		dim コマンドで定義された配列のリスト
*/

typedef struct _dim_list {
	struct _dim_list *next;
	char *name;
	int max;
} DIMLIST;

static DIMLIST *dim_top = NULL;

static void add_array_dim( char *name, int max )
{
	DIMLIST *dl;

	dl = Malloc( sizeof( DIMLIST ) );
	dl->name = Malloc( strlen( name ) + 1 );
	strcpy( dl->name, name );
	dl->max = max;

	dl->next = dim_top;
	dim_top = dl;
}

static int get_array_max( char *name )
{
	DIMLIST *dl;

	for( dl = dim_top; dl != NULL; dl = dl->next ){
		if( strcmp( dl->name, name ) == 0 ){
			return dl->max;
		}
	}
	return -1;
}

/* -------------------------------------------------------------- */
/*
	PROC の引数リスト対応

	proc 解釈時に毎回初期化される。
*/

#define ARG_NEST		8
#define MAX_PROC_ARG	16
#define MAX_LOCAL_ARG	15
#define MAX_PROC_NAMEBUF	2048*2

#define ARG_TYPE_PROC	0
#define ARG_TYPE_BLOCK	1

static int analyze_proc_id = 0;				// 現在解析中のprocID
static COMMAND_PROC *analyze_proc_p;		// 現在解析中のPROC

typedef struct {
	int type;
	int num;
	int local_num;
	int max_ref;
	char *ptr[ MAX_PROC_ARG ];
	char *local_ptr[ MAX_LOCAL_ARG ];
	char *buf_p;
	char buf[ MAX_PROC_NAMEBUF ];
} PROC_ARG_INFO ;

static int arg_nest = -1;
static PROC_ARG_INFO proc_arg[ ARG_NEST ];
static PROC_ARG_INFO *current_arg_info;

static void call_proc_arg( int num )
{
	if( current_arg_info == NULL ){
		ERROR( "引数はprocのコンテキスト内でしか使えません\n" );
	}
	if( current_arg_info->max_ref < num ){
		current_arg_info->max_ref = num;
	}
}

static int get_max_arg_num( void )
{
	int max, num;

	max = current_arg_info->max_ref;
	num = current_arg_info->num + 1;

	if( num > 0 && max > num ){
		ERROR( "プロトタイプ引数の数が少なすぎます。\n" );
	} else {
		if( num > max ){
			max = num;
		}
	}
	return max;
}

static void init_proc_arg_table( int type )
{
	long i;
	PROC_ARG_INFO *ap;

	arg_nest ++;

	ap = &( proc_arg[ arg_nest ] );
	current_arg_info = ap;

	ap->type = type;
	ap->buf_p = ap->buf;

	for( i = 0; i < MAX_PROC_ARG; i++ ){
		ap->ptr[ i ] = NULL;
	}
	for( i = 0; i < MAX_LOCAL_ARG; i++ ){
		ap->local_ptr[ i ] = NULL;
	}
	
	ap->local_num = -1;
	ap->num = -1;
	ap->max_ref = -1;
}

static void close_proc_arg_table( void )
{
	arg_nest --;
	if( arg_nest >= 0 ){
		current_arg_info = &( proc_arg[ arg_nest ] );
	} else {
		current_arg_info = NULL;
	}
}

static void set_proc_arg( char *arg )
{
	PROC_ARG_INFO *ap;
	ap = current_arg_info;
	ap->num ++;
	if( ap->num >= MAX_PROC_ARG ){
		ERROR( "PROC 引数は %d 個までです。\n", MAX_PROC_ARG );
	}
	if( ( ap->buf_p - ap->buf ) + strlen( arg ) + 1 >= MAX_PROC_NAMEBUF ){
		ERROR( "PROC 引数の名前の総量が長すぎます。\n" );
	}

	strcpy( ap->buf_p, arg );
	ap->ptr[ ap->num ] = ap->buf_p;
	ap->buf_p += strlen( arg ) + 1;
}

static void set_proc_local_arg( char *arg )
{
	PROC_ARG_INFO *ap;

	ap = current_arg_info;

	if( ap->type != ARG_TYPE_PROC ){
		ERROR( "このブロック中ではlocal変数は使えません\n" );
	}
	
	ap->local_num ++;
	if( ap->local_num >= MAX_LOCAL_ARG ){
		ERROR( "PROC 引数は %d 個までです。\n", MAX_LOCAL_ARG );
	}
	if( ( ap->buf_p - ap->buf ) + strlen( arg ) + 1 >= MAX_PROC_NAMEBUF ){
		ERROR( "PROC 引数の名前の総量が長すぎます。\n" );
	}

	strcpy( ap->buf_p, arg );
	ap->local_ptr[ ap->local_num ] = ap->buf_p;
	ap->buf_p += strlen( arg ) + 1;
}

static long search_proc_arg( char *arg )
{
	PROC_ARG_INFO *ap;
	long i;

	ap = current_arg_info;
	if( ap == NULL ) return -1;

	if( ap->num < 0 ) return -1;

	for( i = 0; i < arg_nest; i++ ){
		int j;
		PROC_ARG_INFO *a;
		a = &( proc_arg[ i ] );
		for( j = 0; j < a->num; j++ ){
			if( strcmp( arg, a->ptr[ j ] ) == 0 ){
				WARNING( "引数 %s は上のレベルの引数とは別に扱われます\n", arg );
			}
		}
	}

	for( i = 0; i <= ap->num; i++ ){
		if( strcmp( ap->ptr[ i ], arg ) == 0 ){
PRINTF( "ARG %s %s %d\n", arg, ap->ptr[ i ], i + 1 );
			return i + 1;
		}
	}
	return 0;
}

static long search_proc_local_arg( char *arg )
{
	PROC_ARG_INFO *ap;
	long i;

	ap = current_arg_info;
	if( ap == NULL ) return -1;

	if( ap->local_num < 0 ) return -1;

	for( i = 0; i < arg_nest; i++ ){
		int j;
		PROC_ARG_INFO *a;
		a = &( proc_arg[ i ] );
		for( j = 0; j < a->local_num; j++ ){
			if( strcmp( arg, a->local_ptr[ j ] ) == 0 ){
				WARNING( "引数 %s は上のレベルの引数とは別に扱われます\n", arg );
			}
		}
	}

	for( i = 0; i <= ap->local_num; i++ ){
		if( strcmp( ap->local_ptr[ i ], arg ) == 0 ){
PRINTF( "LOCAL %s %s %d\n", arg, ap->local_ptr[ i ], i + 1 );
			return i + 1;
		}
	}
	return 0;
}

static int get_proc_local_arg_num( void )
{
	return current_arg_info->local_num + 1;
}


/* -------------------------------------------------------------- */
/*
	変数形式の解析
*/

#define LOCAL_VARIABLE_FLAG 0x10000

static long variable_type( char *varstr )
{
	long type;
	int local_flag = 0;

	if( varstr[ 1 ] == '$' ){
		local_flag = 1;
		if( varstr[ 2 ] != ':' ){
			if( search_proc_local_arg( varstr + 4 ) > 0 ){
				WARNING( "%s : PROC local変数と同じ名前の変数があります。\n", varstr );
			}
		}
		varstr ++;
	} else {
		if( varstr[ 1 ] != ':' ){
			if( search_proc_arg( varstr + 3 ) > 0 ){
				WARNING( "%s : PROC引数と同じ名前の変数があります。\n", varstr );
			}
		}
	}
	switch( varstr[ 1 ] ){
	  case 'w':
		/* 16ビット数値 */
		type = GCL_SHORT | GCL_VAR;
		break;
	  case 'b':
		/* 8ビット数値 */
		type = GCL_BYTE | GCL_VAR;
		break;
	  case 'c':
		/* 8ビット文字 */
		type = GCL_CHAR | GCL_VAR;
		break;
	  case 'f':
		/* 1ビットフラグ */
		type = GCL_BOOL | GCL_VAR;
		break;
	  case 's':
		if( strcmp( varstr + 1, "status" ) == 0 ){
			type = GCL_STATUS;
		} else {
			/* 24ビット文字コード */
			type = GCL_STRID | GCL_VAR;
		}
		break;
	  case 'v':
		/* 32ビットベクトル */
		type = GCL_VECTOR | GCL_VAR;
		ERROR( "%s: $v:変数は未サポートです\n", varstr );
		break;
	  case 'i':
		/* 32 ビット数値 */
		type = GCL_INT | GCL_VAR;
		break;
	  case '?':
		type = GCL_STATUS;
		break;
	  default:
		if( local_flag == 0 ){
			if( varstr[ 1 ] >= '1' && varstr[ 1 ] <= '9' ){
				/* PROC 用引数 */
				int c;
				
				//			c = varstr[ 1 ] - '0';
				c = atoi( varstr + 1 );
				if( c >= 10 && c < MAX_PROC_ARG ){
					varstr[ 1 ] = '0' + c;
				}
				if( c <= 0 || c >= MAX_PROC_ARG ){
					ERROR( "引数の番号が不正です\n" );
				}
				call_proc_arg( c );
				return GCL_ARG;
			} else {
				int code;

				if( ( code = search_proc_arg( varstr + 2 ) ) > 0 ){
					/* 引数プロトタイプで宣言されているかどうか */
					varstr[ 1 ] = code + '0';
					return GCL_ARG;
				} else if( code == 0 ){
					ERROR( "引数 %s が引数リストで見つかりません。\n", varstr );
				}
				ERROR( "%s 変数名が不正です\n", varstr );
				return GCL_INT | GCL_VAR;
			}
		} else {
			// ローカル引数バッファ
			int code;

			if( ( code = search_proc_local_arg( varstr + 2 ) ) > 0 ){
				/* 引数プロトタイプで宣言されているかどうか */
				return GCL_LOCAL | ( code );
			} else if( code == 0 ){
				ERROR( "ローカル引数 %s が引数リストで見つかりません。\n", varstr - 1 );
			}
			ERROR( "%s ローカル変数は定義されていません\n", varstr - 1 );
		}
	}
	return type | ( ( local_flag ) ? LOCAL_VARIABLE_FLAG : 0 );
}

/* -------------------------------------------------------------- */
/*
	文字列リソース管理
	バイナリリソースも同列に扱う
*/

#define STRRES_TYPE_STRING	0x0001	// 文字列変換を行なう
#define STRRES_TYPE_BINARY	0x0002	// 文字列変換を行なわない
#define STRRES_TYPE_ALIGN4	0x0100
#define STRRES_TYPE_ALIGN16	0x0200

#define STRRES_OPTION_NO_STRCHECK		0x00000001
#define STRRES_OPTION_STRCHECK_PARAM	0x00000002

typedef struct _string_tag {
	struct _string_tag *next;
	int tagname;				// string_buf 上のオフセット
	int max;
	int string_table_top;		// STRING_TABLE の先頭位置
} STRING_TAG;

typedef struct {
	int no;				// 通し番号
	int out_pos;		// 出力したファイル上の場所（出力する前は-1)
	int tag_offset;
	int label_offset;	// label のstring_buf 上のオフセット
	int offset;			// string_buf 上のオフセット
	int type;			// データのタイプ
	int length;			// データの長さ
	int options;			// オプションフラグ
	union {
		struct {
			short w;
			short h;
			unsigned char c_skip;
			unsigned char l_skip;
			short flag;
		} font;
	} u;
} STRING_TABLE;

typedef struct {
	int no;			// STRING_TABLE の通し番号
	int which;		// 0:body 1:proc(called) -1:proc(not called)
	int checked;	// 0: already checked string
	int offset;		// 呼び出し元のbuf上のオフセット
	int out_offset;	// 出力OFFSET
	int no_caller;	// 呼び出し元の書き換えを行なわない
} STRING_REFER_TABLE;

static STRING_TAG tag_top;

static BUFFER_INFO string_table;
static BUFFER_INFO string_refer_table;

static int string_table_no = 0;

#define STRING_TABLE_SIZE			(1024)
#define STRING_TABLE_REALLOC_SIZE	(512)

enum {
	STRING_WRITEBACK_CALLER = 0,
	STRING_NO_WRITEBACK = 1
};

static void init_string_tag( void )
{
	tag_top.next = NULL;

	string_table_no = 0;
	init_buffer( &string_table, STRING_TABLE_SIZE, BUF_WHICH_NONE );
	init_buffer( &string_refer_table, STRING_TABLE_SIZE, BUF_WHICH_NONE );
}

static char *get_string_buf( int offset )
{
	return string_buf.buffer + offset;
}

static int set_string_buf( char *string )
{
	return set_data( &string_buf, string, strlen( string ) + 1 );
}

static STRING_TABLE *get_new_string_table( STRING_TAG *tag, char *label, char *string )
{
	int pos, offset;
	STRING_TABLE new, *p;

	/* 文字列リソースとしての確保 */
	pos = string_table_no ++;
	new.no = pos;
	new.offset = set_string_buf( string );
	new.length = strlen( string ) + 1;
	new.type = STRRES_TYPE_STRING;
	new.tag_offset = tag->tagname;
	new.label_offset = set_string_buf( label );
	new.out_pos = -1;
	new.options = 0;

	offset = set_data( &string_table, &new, sizeof( new ) );

	p = ( STRING_TABLE * )( string_table.buffer + offset );

	return p;
}

static STRING_TABLE *get_new_binary_table( char *label, void *binary, int len, int align )
{
	int pos, offset;
	STRING_TABLE new, *p;

	/* 文字列リソースとしての確保 */
	pos = string_table_no ++;
	new.no = pos;
	new.offset = set_data( &string_buf, binary, len );
	new.length = len;
	new.type = STRRES_TYPE_BINARY | align;
	new.label_offset = set_string_buf( label );
	new.out_pos = -1;

	offset = set_data( &string_table, &new, sizeof( new ) );

	p = ( STRING_TABLE * )( string_table.buffer + offset );

	return p;
}

static STRING_TAG *new_string_tag( char *tagname )
{
	// 文字列ブロックの登録／タグ名を参照
	STRING_TAG *p, *prev, *new;

	prev = &tag_top;
	for( p = prev->next; p != NULL; prev = p, p = p->next ){
		if( strcmp( get_string_buf( p->tagname ), tagname ) == 0 ){
			ERROR( "文字列リソース %s はすでに定義されています。\n", tagname );
		}
	}

	new = Malloc( sizeof( STRING_TAG ) );
	new->next = prev->next;
	prev->next = new;

	new->max = 0;
	new->tagname = set_string_buf( tagname );
	new->string_table_top = string_table_no;

	return new;
}

static STRING_TAG *search_string_tag( char *tagname )
{
	// 文字列ブロックの登録／タグ名を参照
	STRING_TAG *p;

	for( p = tag_top.next; p != NULL; p = p->next ){
		if( strcmp( get_string_buf( p->tagname ), tagname ) == 0 ){
			return p;
		}
	}

	ERROR( "文字列リソース %sは定義されていません。\n", tagname );
	return NULL;
}

static STRING_TABLE *set_string_table( STRING_TAG *tag, char *label, char *string )
{
	// タグに文字列データの登録

	tag->max ++;
	return get_new_string_table( tag, label, string );
}

static void set_binary_table( STRING_TAG *tag, char *label, void *binary, int len, int align )
{
	// タグにバイナリデータの登録
	tag->max ++;
	get_new_binary_table( label, binary, len, align );
}

static STRING_TABLE *search_string_table( STRING_TABLE *tag, int num, char *label )
{
	int i;

	for( i = 0; i < num; i++, tag++ ){
		if( strcmp( get_string_buf( tag->label_offset ), label ) == 0 ){
			return tag;
		}
	}
	return NULL;
}

static void set_string_refer( BUFFER_INFO *bi, STRING_TABLE *tp, int caller_flag )
{
	STRING_REFER_TABLE ref;
	int which;

	which = bi->which;
	if( which == BUF_WHICH_NONE ){
		ERROR( "ここではリソースは使用できません\n" );
	}

	ref.no = tp->no;
	ref.which = which;
	ref.out_offset = ref.offset = bi->now_p;
	ref.no_caller = caller_flag;
	ref.checked = 0;

	set_data( &string_refer_table, &ref, sizeof( ref ) );
}

static void refer_string_all_tag( BUFFER_INFO *bi, STRING_TAG *tag, STRING_TABLE *tp )
{
	int i;

	for( i = 0; i < tag->max; i++ , tp++ ){
		int caller;
		caller = ( ( i == 0 ) ? STRING_WRITEBACK_CALLER : STRING_NO_WRITEBACK );
		set_string_refer( bi, tp, caller );
	}
}

static void refer_string_table( BUFFER_INFO *bi, char *tagname, char *label )
{
	STRING_TAG *tag;
	STRING_TABLE *tp;

	PRINTF( "refer_string_table\n" );

	tag = search_string_tag( tagname );

	/* このタグのリソースすべてのテーブルの先頭アドレス */
	tp = ( STRING_TABLE * )string_table.buffer + tag->string_table_top;

	if( strcmp( label, "*" ) == 0 ){
		/* そのリソースすべてを使用可能状態にする */
		refer_string_all_tag( bi, tag, tp );
	} else if( ( tp = search_string_table( tp, tag->max, label ) ) != NULL ){
		set_string_refer( bi, tp, STRING_WRITEBACK_CALLER );
		PRINTF( "refer %s:%s = %s\n", tagname, label, get_string_buf( tp->offset ) );
	} else {
		ERROR( "label %s は %s の中に見つかりません。\n", label, tagname );
	}
}

static int change_proc_string_table_offset( int index, long top, long len, long offset )
{
	STRING_REFER_TABLE *table;
	int i, max;

	/*
		proc_buf の top から len バイトの範囲内を指している文字列テーブルの
		オフセットをoffset(負)だけずらす

		これもtable->offsetの値は整列しているはずなので、それを使って高速化する。
	*/
	table = ( STRING_REFER_TABLE * )( string_refer_table.buffer );
	max = string_refer_table.now_p / sizeof( STRING_REFER_TABLE );
	table = table + index;
	for( i = index; i < max; i++, table ++ ){
		if( table->which < 0 ){
			if( table->out_offset >= top ){
				if( table->out_offset < top + len ){
					table->out_offset += offset;
					table->which = 1;
					index = i;
				} else {
					break;
				}
			}
		}
	}
	return index;
}

static void shift_string_ref_offset( int string_p, int which, int offset )
{
	STRING_REFER_TABLE *table;
	STRING_REFER_TABLE *end;
	
	table = ( STRING_REFER_TABLE * )( string_refer_table.buffer + string_p );
	end = ( STRING_REFER_TABLE * )( string_refer_table.buffer + string_refer_table.now_p );

	while( table < end ){
		if( table->which == which ){
			table->offset += offset;
			table->out_offset += offset;
		}
		table++;
	}
}

static void reset_string_call( void )
{
	int i;
	STRING_TABLE *tp;

	tp = ( STRING_TABLE * )string_table.buffer;

	for( i = 0; i < string_table_no; i++ ){
		tp->out_pos = -1;
		tp ++;
	}

	{
		int max;
		STRING_REFER_TABLE *rp, *dp;

		rp = dp = ( STRING_REFER_TABLE * )( string_refer_table.buffer );
		max = string_refer_table.now_p / sizeof( STRING_REFER_TABLE );
		for( i = 0; i < max; i++, rp++ ){
			if( rp->which != 0 ){
				*dp = *rp;
				dp->which = -1;
				dp->out_offset = dp->offset;
				dp++;
			}
		}
		string_refer_table.now_p = ( char * )dp - string_refer_table.buffer;
	}
	
//	reset_buffer( &string_refer_table );
}

/* -------------------------------------------------------------- */
/*
	文字列ID作成処理
*/

enum {
	STR_ANY = 0,
	STR_STRID = 1,
	STR_STRID2 = 2,
};

static char *set_strid( BUFFER_INFO *bi, char *string, int type_flag )
{
	static char base_str[ 256 ];
	char *p;
PRINTF( "set_strid %s\n", string );
	strcpy( base_str, string );
	if( ( p = strchr( base_str, ':' ) ) == NULL ){
		/* 24BitSTRID */

PRINTF( "24bit\n" );
		if( type_flag == STR_STRID2 ){
			ERROR( "名前:番号 形式が必要です。\n" );
		}
		set_byte( bi, GCL_STRID );
		set_strcode( bi, get_strcode( base_str ) );
	} else {
		/* 24BitSTRID + 8Bit Number */
		unsigned int id;

#ifdef STRCODE16
		ERROR( "名前:番号 形式はつかえません。\n" );
#endif

PRINTF( "32bit %s\n", string );
		if( type_flag == STR_STRID ){
			ERROR( "名前:番号 形式はここではつかえません。\n" );
		}
		id = get_expr_strcode( base_str );
PRINTF( "ID = %X CODE = %X\n", id, get_strcode( base_str ) );
		if( ( id & 0xff000000 ) > 0 ){
			set_byte( bi, GCL_STRID2 );
			set_strcode( bi, get_strcode( base_str ) );
			set_byte( bi, ( id >> 24 ) );
		} else {
			set_byte( bi, GCL_STRID );
			set_strcode( bi, get_strcode( base_str ) );
		}
	}

	return base_str;
}

/* ------------------------- 変数宣言 -------------------------- */

#define VAR_REFERD		0		/* 参照 */
#define VAR_DEFINE		1		/* 代入 */

typedef struct {
	int which;
	int flag;
	int offset;
	int out_offset;
	int type;
	short name_len;
	short array_max;
} VARIABLE;

static long set_variable( BUFFER_INFO *bi, char *varstr, long flag )
{
	VARIABLE var;
	char *name;
	long dummy = 0;
	long array_flag, offset;
	int array_max;
	int type;

	var.which = bi->which;
	if( bi->which == BUF_WHICH_NONE ){
		ERROR( "ここでは変数は使用できません\n" );
	}

	var.flag = flag;
PRINTF( "set_variable(%d) %s %d\n", var.which, varstr, flag );

	type = variable_type( varstr );

	var.type = ( type & 0xFF );

	if( var.type == GCL_ARG ){
		int no;
		no = ( varstr[ 1 ] - '0' );
		if( no < 0x0F ){
			set_byte( bi, GCL_ARG | no );
		} else {
			set_byte( bi, GCL_ARG | 0x0F );
			set_byte( bi, ( no - 0x0F ) );
		}
		return GCL_ARG;
	}
	if( ( var.type & 0xF0 ) == GCL_LOCAL ){
		set_byte( bi, var.type );
		return GCL_ARG;
	}
	if( var.type == GCL_STATUS ){
		set_byte( bi, GCL_ARG | 0 );
		return GCL_ARG;
	}
	array_flag = 0;
	array_max = get_array_max( varstr );
	{
		WORDBUF word;
		if( get_word( word ) != NULL ){
			if( word[ 0 ] == '[' ){
				/* 配列 */
				var.type = GCL_TYPE( var.type ) | GCL_ARRAY;
				array_flag = 1;
			} else {
				if( array_max >= 0 ){
					/* 配列として宣言されている */
					var.type = GCL_TYPE( var.type ) | GCL_ARRAY;
					array_flag = 2;
				}
				unget_word( word );
			}
		}
	}

	offset = set_byte( bi, var.type );
	var.offset = offset;
	var.out_offset = offset;
	set_data( bi, &dummy, 3 );					/* データは3バイト */

	if( type & LOCAL_VARIABLE_FLAG ){
		var.type |= LOCAL_VARIABLE_FLAG;
	}
	if( ( name = strchr( varstr, ':' ) ) == NULL ){
		name = varstr + 1;
	} else {
		name ++;
	}
	var.name_len = ( ( ( strlen( name ) + 1 ) + 3 ) & ( ~3 ) );
	var.array_max = 1;	/* 配列以外はながさ１ */

	if( array_flag == 1 ){
		/* 配列である */

		if( array_max <= 0 ){
			ERROR( "配列 %s は dim宣言されていません。\n", varstr );
		}
		var.array_max = array_max;
		set_number_minimum( bi, array_max, 2 );
		if( ! analyze_expr( bi, ']' ) ){
			ERROR( "[]が閉じていません。\n" );
		}
	} else if( array_flag == 2 ){
		/* 配列だが添字がない */
		WARNING( "%s 配列なので添字が必要です。０として扱います。\n", varstr );
		var.array_max = array_max;
		set_number_minimum( bi, array_max, 2 );
		set_byte( bi, GCL_BYTE );
		set_byte( bi, 0 );
	}
PRINTF( "set_variable %08X %s %X\n", value_buf.now_p, name, var.type );
	set_data( &value_buf, &var, sizeof( VARIABLE ) );
	set_data( &value_buf, name, var.name_len );
	return var.type;
}

static void dump_variable( void )
{
	VARIABLE *vp;
	void *end;

	vp = ( VARIABLE * )value_buf.buffer;
	end = ( void * )(value_buf.buffer + value_buf.now_p);

	while( ( void * )vp < end ){
		PRINTF( "%s which %d flag %d offset %d out %d type %X\n", ( char * )( vp + 1 )
			   , vp->which, vp->flag, vp->offset, vp->out_offset, vp->type );
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vp->name_len );
	}
}

static void reset_variable_call( void )
{
	VARIABLE *vp, *dp, *vp_next;
	void *end;

	dp = vp = ( VARIABLE * )value_buf.buffer;
	end = ( void * )(value_buf.buffer + value_buf.now_p);
	while( ( void * )vp < end ){
		vp_next = ( VARIABLE * )( ( char * )( vp + 1 ) + vp->name_len );
		if( vp->which != 0 ){
			// PROC内への参照データは残す
			memcpy( dp, vp, sizeof( VARIABLE ) + vp->name_len );
			// 書き換えられた参照情報はリセット
			dp->which = -1;
			dp->out_offset = dp->offset;
			// dpを次に進める
			dp = ( VARIABLE * )( ( char * )( dp + 1 ) + dp->name_len );
		}
		vp = vp_next;
	}
	value_buf.now_p = ( char * )dp - ( char * )value_buf.buffer;
}

static void *change_proc_variable_offset( void *ptr, long top, long len, long offset )
{
	VARIABLE *vp;
	void *end;

	/*
		proc_buf の top から len バイトの範囲内を指している変数テーブルの
		オフセットをoffset(負)だけずらす

		value_buf中のproc_bufへの参照情報は、offsetの順に並んでいるはずなので、
		最後に書き換えたvpを覚えておいて高速化する。
	*/

	if( ptr == NULL ){
		vp = ( VARIABLE * )value_buf.buffer;
	} else {
		vp = ( VARIABLE * )ptr;
	}
	end = ( void * )(value_buf.buffer + value_buf.now_p);
PRINTF( "CHANGE TOP = %d len = %d OFFSET = %d\n", top, len, offset );
	while( ( void * )vp < end ){
		if( vp->which < 0 ){
			if( vp->out_offset >= top ){
				if( vp->out_offset < top + len ){
					vp->out_offset = vp->offset + offset;
					vp->which = 1;
					PRINTF( "change %s which %d flag %d offset %X type %X\n"
						   , ( char * )( vp + 1 )
						   , vp->which, vp->flag, vp->out_offset - top, vp->type );
					ptr = vp;
				} else {
					break;
				}
			}
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vp->name_len );
	}
	return ptr;
}

static void shift_variable_offset( int value_p, int which, int offset )
{
	VARIABLE *vp;
	void *end;

	vp = ( VARIABLE * )( value_buf.buffer + value_p );
	end = ( void * )( value_buf.buffer + value_buf.now_p );
	while( ( void * )vp < end ){
		if( vp->which == which ){
			vp->offset += offset;
			vp->out_offset += offset;
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vp->name_len );
	}
}

/* ---------------------------------------------------------------------- */
/*
	可変長サイズ記述用
*/

typedef struct {
	int type;
	int value_p;
	int string_p;
	int top_p;
	BUFFER_INFO *bi;
} BLOCK_TOP;

#define BLOCK_LEN_DONE			0	// すでに完了している
#define BLOCK_LEN_TYPE_TAG		1	// TYPETAGに含むタイプ
#define BLOCK_LEN_TYPE_SHORT	2	// 最大shortで記録するタイプ

#define GCL_NO_TAG				0

static void set_block_top( BUFFER_INFO *bi, int tag, BLOCK_TOP *btop )
{
	if( tag != GCL_NO_TAG ){
		set_byte( bi, tag );
		btop->type = BLOCK_LEN_TYPE_TAG;
	} else {
		btop->type = BLOCK_LEN_TYPE_SHORT;
	}

	/* この直前のバイトにGCL_OPTIONなりのタグが入っているのが前提 */
	btop->value_p = value_buf.now_p;
	btop->string_p = string_refer_table.now_p;
	btop->top_p = get_now_pos( bi );
	btop->bi = bi;

	set_int( bi, 0 );	// 最大４バイトであるため、その分確保しておく。
}

static void set_block_end( BLOCK_TOP *btop )
{
	int size;
	int offset;
	BUFFER_INFO *bi;
	unsigned char *p;
	const int max = 4;

	bi = btop->bi;

	size = get_block_size( bi, btop->top_p ) - 4;	// 確保した分を引く

	p = ( unsigned char * )( bi->buffer + btop->top_p );

	switch( btop->type ){
	  case BLOCK_LEN_TYPE_TAG:
		{
			// *( p - 1 )に入っているGCL_*タグの下位を含めたエンコード
			if( size <= 0x0C ){
				/* OPTIONにエンコード */
				*( p - 1 ) |= size;
				offset = 0;
			} else if( size < 256 ){
				/* 1byteにエンコード */
				*( p - 1 ) |= 0x0D;
				store_value( bi, btop->top_p, size, 1 );
				offset = 1;
			} else if( size < 0x10000 ){
				/* 2bytesにエンコード */
				*( p - 1 ) |= 0x0E;
				store_value( bi, btop->top_p, size, 2 );
				offset = 2;
			} else if( size < 0x1000000 ){
				/* 3bytesにエンコード */
				*( p - 1 ) |= 0x0F;
				store_value( bi, btop->top_p, size, 3 );
				offset = 3;
			} else {
				offset = 4;
				ERROR( "TOO LARGE BLOCK %s\n", size );
			}
		}
		break;
	  case BLOCK_LEN_TYPE_SHORT:
		{
			// byteかshortにエンコード

			if( size <= 0x7F ){
				*p = size;
				offset = 1;
			} else if( size <= 0x7FFF ){
				*p = ( size >> 8 ) | 0x80;
				*( p + 1 ) = ( size & 0xFF );
				offset = 2;
			} else {
				ERROR( "ブロックサイズは0x7FFF(32K)までです。(%d)\n", size );
			}
		}
		break;
	  default:
		ERROR( "Internal Error: block type %d\n", btop->type );
		break;
	}

	if( offset != max ){
		int n;
		n = bi->now_p - btop->top_p - max;
		// この場合、offset - maxは必ず負になる。
		memmove( p + offset, p + max, n );
		shift_variable_offset( btop->value_p, bi->which, offset - max );
		shift_string_ref_offset( btop->string_p, bi->which, offset - max );
		bi->now_p -= max - offset;
	}
	btop->type = BLOCK_LEN_DONE;
}

/* -------------------------------------------------------------- */
/*
	変換処理メイン 
*/

#define PARSE_GLOBAL 1
#define PARSE_LOCAL  0

static int convert_block( BUFFER_INFO *bi, int global_flag );
static COMMAND_PROC *set_call_proc( char *command );

/* -------------------------------------------------------------- */
/*
	式の解釈
*/

#define MAX_EXPR_STEP	8	// 解釈系でのスタックの最大値

static int max_expr_step;
static int current_expr_step;

static void output_expr( BUFFER_INFO *bi, OPERAND *top );

static void output_node( BUFFER_INFO *bi, NODE *node, long flag )
{
	switch( node->type ){
	  case TYPE_UNSET:
		PRINTF( "START " );
		break;
	  case TYPE_VALUE:
		set_number_minimum( bi, node->value, sizeof( int ) );
		PRINTF( "%d ", node->value );
		current_expr_step ++;
		break;
	  case TYPE_STRID:
		if( ( node->value & 0xff000000 ) == 0 ){
			set_byte( bi, GCL_STRID );
			set_strcode( bi, node->value );
		} else {
			set_byte( bi, GCL_STRID2 );
			set_int( bi, node->value );
		}
		current_expr_step ++;
		break;
	  case TYPE_VARIABLE:
		set_parse_buffer( "" );
		set_variable( bi, ( char * )node->value, flag );
		reset_parse_buffer();
		PRINTF( "%s ", ( char * )node->value );
		current_expr_step ++;
		break;
	  case TYPE_ARRAY:
		{
			char *base, *index;

			base = ( char * )node->value;
			index = base + strlen( base ) + 1;
			set_parse_buffer( index );
			set_variable( bi, base, flag );
			reset_parse_buffer();
		}
		current_expr_step ++;
		break;
	  case TYPE_BLOCK:
		PRINTF( "BLOCK " );
	  case TYPE_OPERAND:
		output_expr( bi, ( OPERAND * )node->value );
		break;
	  case TYPE_COMMAND:
		/* GCL のコマンド呼出 */
		{
			int mode;

			set_parse_buffer( ( char * )node->value );
			mode = parse_change_mode( PARSE_NORMAL_MODE );
PRINTF( "EXPR_CONVERT_BLOCK\n" );
			convert_block( bi, PARSE_GLOBAL );
PRINTF( "EXPR_CONVERT_BLOCK end\n" );
			reset_parse_buffer();
			parse_change_mode( mode );
		}
		current_expr_step ++;
		break;
	}
}

static int is_str_variable( char *name )
{
	if( strncmp( name, "$s:", 3 ) == 0
		|| strncmp( name, "$$s:", 4 ) == 0 ){
		return 1;
	}
	return 0;
}

static void output_expr( BUFFER_INFO *bi, OPERAND *top )
{
	static char *operands[] = {
		"OP_END",						/* 終端記号	*/
		"OP_MNS", "OP_NOT",	"OP_NEG",					/* 単項演算 */
		"OP_ADD", "OP_SUB", "OP_MUL", "OP_DIV", "OP_MOD",		/* 四則演算	*/
		"OP_LSL", "OP_LSR",						/* 論理シフト */
		"OP_EQ", "OP_NE", "OP_LT", "OP_LE", "OP_GT", "OP_GE",	/* 比較演算	*/
		"OP_OR", "OP_AND",  "OP_XOR",				/* 論理演算	*/
		"OP_OROR", "OP_ANDAND",					/* 論理演算	*/
		"OP_SET",								/* 代入 */
		"OP_COMMA",								/* コンマ */
	};

	long flag;
	flag = ( top->operand == OP_SET ) ? VAR_DEFINE : VAR_REFERD;
	if( flag == VAR_DEFINE ){
		/* 代入先が変数かどうかをチェック */
		if( top->prev.type != TYPE_VARIABLE && top->prev.type != TYPE_ARRAY ){
			ERROR( "変数以外に代入しようとしています。\n" );
		}
		if( ( ( ( char * )top->prev.value )[ 1 ] >= '1'
			&& ( ( char * )top->prev.value )[ 1 ] <= '9' )
			|| strncmp( ( char * )top->prev.value, "$:", 2 ) == 0 ){
			ERROR( "引数に代入しようとしています。\n" );
		}
		if( strncmp( ( char * )top->prev.value, "$$:", 3 ) == 0 ){
			; // OK
		} else if( top->next.type == TYPE_VALUE ){
			/* 代入元と代入先の型チェック */
			if( is_str_variable( ( char * )top->prev.value ) ){
				ERROR( "数値定数を文字列型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		} else if( top->next.type == TYPE_STRID ){
			if( ! is_str_variable( ( char * )top->prev.value ) ){
				ERROR( "文字列定数を数値型 %s に代入しようとしています。\n"
					  , ( char * )top->prev.value );
			}
		}
	}
	if( top->operand != OP_END ){
		/* 値セット */
		output_node( bi, &( top->prev ), flag );
		output_node( bi, &( top->next ), VAR_REFERD );
		/* オペランドセット */

		set_byte( bi, GCL_OP | top->operand );

		if( current_expr_step > max_expr_step ){
			max_expr_step = current_expr_step;
		}
		current_expr_step --;
		PRINTF( "%s ", operands[ top->operand ] );
	} else {
		/* 値セット */
		output_node( bi, &( top->next ), VAR_REFERD );
	}
}

static long analyze_expr( BUFFER_INFO *bi, char term_char )
{
	/* その行の終端で終わったら 0、
	   最後がマッチしない')'なら、1 */
	EXPR_INFO info, *ep;
	OPERAND *op;
	int value, old_mode;

	old_mode = parse_change_mode( PARSE_EXPR_MODE );

	ep = &info;

	init_expr( ep, term_char );
	op = set_expr( ep );

	/* 一回計算してみる */
	if( calc_expr( ep, op, &value ) ){
		/* 定数になった */
		set_number( bi, value, GCL_WILD );
	} else {
		BLOCK_TOP top;
		/* 式の出力 */

		set_block_top( bi, GCL_EXPR, &top );

		max_expr_step = current_expr_step = 0;

		output_expr( bi, op );
		set_byte( bi, GCL_OP | GCL_END );

		if( max_expr_step > MAX_EXPR_STEP ){
			ERROR( "式が複雑過ぎます\n" );
		}

		set_block_end( &top );
	}
	close_expr( ep );

	parse_change_mode( old_mode );

	return ep->term_flag;
}

/* -------------------------------------------------------------- */
/*
	if の解析だけ、別
	if 文解析
*/

static void _open_brace_check( void )
{
	WORDBUF buf;

	if( get_word_cl( buf ) == NULL || buf[ 0 ] != '{' ){
		ERROR( "ブロックが不正です。\n" );
	}
/*
	if( get_word( buf ) != NULL ){
		ERROR( "'{'の後に不正な文字があります\n" );
	}
*/
}

static void _set_expr( BUFFER_INFO *bi )
{
	WORDBUF buf;

	if( get_word( buf ) == NULL || buf[ 0 ] != '(' ){
		ERROR( "式がありません。\n" );
	}
	if( ! analyze_expr( bi, ')' ) ){
		ERROR( "式が閉じていません。\n" );
	}
}

/*
	特殊扱い関数
*/

static int convert_if( BUFFER_INFO *bi, BLOCK_TOP *argtop )
{
	BLOCK_TOP top;
	WORDBUF buf;
	int else_end_flag;

	else_end_flag = 0;

PRINTF( "block if\n" );
	top = *argtop;

ELSEIF:
	_set_expr( bi );

	_open_brace_check();
ELSE:
	convert_block( bi, PARSE_LOCAL );

	/* この時点では、block の終了の '}'のみ読み込み済み */

	set_block_end( &top );
	if( get_word( buf ) == NULL ){
		/* 終了 */
//		set_byte( bi, GCL_END );
		return 0;
	}

	if( strcmp( buf, "else" ) == 0 ){
		if( else_end_flag ){
			ERROR( "else の後に、まだ elseがあります。\n" );
		}

		get_word_cl( buf );
PRINTF( "%s\n", buf );
		if( strcmp( buf, "if" ) == 0 ){
PRINTF( "else if\n" );
			set_block_top( bi, GCL_OPTION, &top );
			set_byte( bi, 'i' );
			goto ELSEIF;
		} else if( strcmp( buf, "{" ) == 0 ){
PRINTF( "else\n" );
			set_block_top( bi, GCL_OPTION, &top );
			set_byte( bi, 'e' );
			else_end_flag = 1;
			goto ELSE;
		}
	} else if( strcmp( buf, "}" ) == 0 ){
		/* このブロックの終了 */
		return 1;
	}
	ERROR( "BLOCKの最後におかしな文字 %s があります。\n", buf );

	return 0;
}

static int convert_switch( BUFFER_INFO *bi, BLOCK_TOP *argtop )
{
	WORDBUF buf;
	BLOCK_TOP top;
	int set_default = 0;

	_set_expr( bi );
	_open_brace_check();		// swicth( .. ) { まで解釈

	set_block_end( argtop );

	while( get_word_cl( buf ) != NULL ){
		if( strcmp( buf, "case" ) == 0 ){
			if( set_default ){
				WARNING( "defaultのあとにcaseがあるのは無意味です。\n" );
			}
			set_block_top( bi, GCL_OPTION, &top );
			set_byte( bi, 'c' );
			_set_expr( bi );
		} else if( strcmp( buf, "default" ) == 0 ){
			set_block_top( bi, GCL_OPTION, &top );
			set_byte( bi, 'd' );
			set_default = 1;
		} else if( buf[ 0 ] == '}' ){
			// このブロックの終了
			return 0;
		} else {
			ERROR( "switch文の途中におかしな文字 %s があります。\n", buf );
		}
		_open_brace_check();
		convert_block( bi, PARSE_LOCAL );
		set_block_end( &top );
	}
	ERROR( "switch文が途中で終わっています\n" );
	return 0;
}

/* -------------------------------------------- */
/*
	外部関数解析
*/

static int repeat_flag = 0;

static COMMAND_TABLE_TAG *current_tag;

/* 引数リスト取得関連 */

static COMMAND_TABLE_TAG *get_table_arg( COMMAND_TABLE *ct, char *code )
{
	repeat_flag = 0;

	if( code == NULL ){
		if( ct->tag->type == GCL_OPTION || ct->tag->type == GCL_END ){
			return NULL;
		}
		if( ct->tag->type == GCL_WILD ){
			repeat_flag = 1;
		}
		current_tag = ct->tag;
		return ct->tag;
	} else {
		COMMAND_TABLE_TAG *p;
		int flag;

		flag = ( strlen( code ) > 1 ) ? 0 : 1;
		for( p = ct->tag; p->type != GCL_END; p = p->next ){
			if( p->type == GCL_OPTION ){
				if( flag ){
					if( p->data[ 0 ] == *code ){
						current_tag = p;
						return p->next;
					}
				} else {
					if( strcmp( &( p->data[ 1 ] ), code ) == 0 ){
						current_tag = p;
						return p->next;
					}
				}
			}
		}
	}
	return NULL;
}

static int get_table_type( COMMAND_TABLE_TAG *tp )
{
	current_tag = tp;
	if( tp == NULL ){
		return GCL_END;
	}
	return ( int )tp->type;
}

static COMMAND_TABLE_TAG *get_next_table( COMMAND_TABLE_TAG *tp )
{
	int type;

	tp = tp->next;

	if( tp->type == GCL_END || tp->type == GCL_OPTION ){
		return NULL;
	}
	type = ( tp->type & 0xf0 );
	if( type == GCL_REPEAT ){
		int i, n;
		repeat_flag = 1;
		n = ( tp->type & 0x0f );
PRINTF( "REPEAT %d\n", n );
		for( i = 0; i < n; i++ ){
			tp = tp->prev;
		}
	} else {
		repeat_flag = 0;
	}
	current_tag = tp;
	return tp;
}

/* リピート処理中か、どうか */

static int is_repeat_top( void )
{
	return repeat_flag;
}

/* 省略可能なタグかどうか */

static int is_omitable_tag( COMMAND_TABLE_TAG *tp )
{
	return ( tp->flag & TABLE_TAG_OMITABLE ) ? 1 : 0;
}

/* コマンドライン解釈時用 */

#if 0
static char *get_cml_word( char *buf )
{
	if( get_word( buf ) == NULL ){
		ERROR( "コマンドラインが途中で終わっています。\n" );
	}
	return buf;
}
#endif

static void set_number_minimum( BUFFER_INFO *bi, int value, int maxbyte )
{
	/* maxbyteを越えない大きさで最小のものにする */
	if( -1 <= value && value <= 62 ){
		set_byte( bi, GCL_VALUE | ( value + 1 ) );
	} else if( value == ( unsigned char )value ){
		set_byte( bi, GCL_BYTE );
		set_byte( bi, value );
	} else if( value == ( short )value ){
		if( maxbyte < 2 ){
			ERROR( "値はunsigned charです。\n", maxbyte );
		}
		set_byte( bi, GCL_SHORT );
		set_short( bi, value );
	} else {
		if( maxbyte < 4 ){
			ERROR( "値はshortです。\n", maxbyte );
		}
		set_byte( bi, GCL_INT );
		set_int( bi, value );
	}
}

static void set_number( BUFFER_INFO *bi, int value, int p_type )
{
	/* 数値定数 */
	int type;
	if( p_type != GCL_WILD ){
		type = p_type & 0xf;
	} else {
		type = p_type;
	}
	switch( type ){
	  case GCL_WILD:
		{
			/* 何でもOKなので、収まる大きさにする */
			set_number_minimum( bi, value, 4 );
		}
		break;
#if 0
		// 現在はVECTORはINT*3と展開されるので、使用していない。
	  case GCL_VECTOR:
		{
			/* ベクトル定数 */
			WORDBUF buf;
			int i;
			set_byte( bi, GCL_VECTOR );
			set_int( bi, value );
			for( i = 0; i < 2; i++ ){
				get_cml_word( buf );
				if( buf[ 0 ] == ',' ){
					get_cml_word( buf );
				}
				if( is_expr_num( buf, &value ) ){
					set_int( bi, value );
				} else {
					ERROR( "異常なベクトル型です。\n" );
				}
			}
		}
		break;
#endif
	  case GCL_SHORT:
		set_number_minimum( bi, value, 2 );
		break;
	  case GCL_BYTE:
		set_number_minimum( bi, value, 1 );
		break;
	  case GCL_INT:
		set_number_minimum( bi, value, 4 );
		break;
	  case GCL_BOOL:
		set_number_minimum( bi, ( value != 0 ) ? 1 : 0, 1 );
		break;
	  case GCL_CHAR:
		if( value < 0 || value > 10 ){
			ERROR( "CHAR 型に数値が指定されています。\n" );
		}
		set_byte( bi, GCL_CHAR );
		set_byte( bi, value + '0' );
		break;
	  case GCL_STRID:
	  case GCL_STRID2:
	  case GCL_STRING:
	  case GCL_PROCID:
		ERROR( "引数が間違っています。%d\n", type );
		break;
	}
}

static void set_string( BUFFER_INFO *bi, char *buf, int p_type )
{
	switch( p_type ){
	  case GCL_CHAR:
		set_byte( bi, GCL_CHAR );
		set_byte( bi, buf[ 0 ] );
		if( strlen( buf ) > 1 ){
			ERROR( "1文字定数ではありません。\n" );
		}
		break;
	  case GCL_STRID:
		{
			/* 必要なら文字列groupチェックをかける */
			if( current_tag != NULL ){
				check_group( current_tag->data, buf );
			}
			/* 後処理はGCL_WILDと同じ */
		}
	  case GCL_WILD:
		set_strid( bi, buf, STR_ANY );
		break;
	  case GCL_STRID2:
		{
			char *p;
			p = set_strid( bi, buf, STR_STRID2 );
			if( current_tag != NULL ){
				check_group( current_tag->data, p );
			}
		}
		break;
	  case GCL_STRING:
	  case GCL_STRRES:
		if( buf[ 0 ] == '[' ){
			/* 文字列リソース参照 */
			char *p;
			get_word( buf );
			if( ( p = strchr( buf, ':' ) ) == NULL ){
				ERROR( "リソース番号が必要です。\n" );
			}
			*p = '\0';
			set_byte( bi, GCL_STRRES );
			refer_string_table( bi, buf, p + 1 );
			set_short( bi, 0 );		// dummy
			get_word( buf );
			if( buf[ 0 ] != ']' ){
				ERROR( "文字列リソース参照が閉じていません。\n" );
			}
		} else {
			if( p_type == GCL_STRRES ){
				ERROR( "リソースを指定して下さい\n" );
			}
			set_byte( bi, GCL_STRING );
			set_byte( bi, strlen( buf ) + 1 );
			set_data( bi, buf, strlen( buf ) + 1 );
		}
		break;
	  case GCL_PROCID:
		set_byte( bi, GCL_PROCID );
		if( strcmp( buf, "NULL" ) == 0 ){
			set_strcode( bi, 0 );
		} else {
			set_call_proc( buf );
			set_strcode( bi, get_strcode( buf ) );
PRINTF( "GCL_PROCID %s %X\n", buf, get_strcode( buf ) );
		}
		break;
	  default:
		ERROR( "string引数の型が違います %s %X\n", buf, p_type );
	}
}

static void set_tsymbol( BUFFER_INFO *bi, char *buf, int p_type )
{
	int code;
	int flag;

	flag = ( p_type & GCL_TYPEMASK ) - GCL_SYMBOL;

	{
		extern int tsymbol_no_check_flag;
		if( tsymbol_no_check_flag != 0 ){
			flag = -1;
		}
	}

	code = get_symbol_code( buf + 2, flag );
	PRINTF( "t:%s %08X\n", buf + 2, code );
	set_byte( bi, GCL_SYMBOL );
	set_int( bi, code );
}

static int is_strtype( int type )
{
	switch( type ){
	  case GCL_STRID:
	  case GCL_STRID2:
	  case GCL_STRING:
	  case GCL_PROCID:
	  case GCL_CHAR:
		return 1;
	}
	return 0;
}

static int type_extend_check( int type )
{
	int t = 0;
	switch( type ){
	  case GCL_INT:
		t++;
	  case GCL_SHORT:
		t++;
	  case GCL_BYTE:
		t++;
	  case GCL_BOOL:
		t++;
		break;
	}
	return t;
}

static int analyze_type_and_store( BUFFER_INFO *bi, char *buf, int p_type )
{
	int value;

	if( p_type == GCL_VAR && buf[ 0 ] != '$' ){
		ERROR( "変数が必要です。\n" );
	}
	if( buf[ 0 ] == '$' ){
		/* 変数 */
		int type;
		int ref;
		ref = ( p_type == GCL_VAR ) ? VAR_DEFINE : VAR_REFERD;
		type = set_variable( bi, buf, ref );

		if( p_type != GCL_VAR && p_type != GCL_WILD && type != GCL_ARG ){
			if( type_extend_check( GCL_TYPE( type ) ) == 0 ){
				if( GCL_TYPE( type ) != p_type ){
					ERROR( "引数の型が違います。 %s\n", buf );
				}
			}
			if( type_extend_check( p_type ) < type_extend_check( GCL_TYPE( type ) ) ){
				ERROR( "引数の型を拡張できません。 %s %X %X\n", buf, p_type, type );
			}
		}
	} else if( buf[ 0 ] == '(' ){
		/* 式 */
		if( ( p_type == GCL_WILD || p_type == GCL_EXPR
			  || p_type == GCL_SHORT || p_type == GCL_BYTE
			  || p_type == GCL_BOOL || p_type == GCL_INT ) ){
			if( ! analyze_expr( bi, ')' ) ){
				ERROR( "')'が見つかりません。\n" );
			}
		} else {
			ERROR( "式を設定することはできません。\n" );
		}
	} else if( buf[ 0 ] == '{' ){
PRINTF( "BLOCK ARG %X\n", current_tag );
		if( p_type == GCL_BLOCK ){
			/* 実行ブロック */
			int rt;

			if( current_tag != NULL && strcmp( current_tag->data, "imm" ) != 0 ){
				WORDBUF buf;

				init_proc_arg_table( ARG_TYPE_BLOCK );
				set_parse_buffer( current_tag->data );

				for( ;; ){
					if( get_word( buf ) == NULL ){
						break;
					}
					if( buf[ 0 ] == '$' && buf[ 1 ] == ':' ){
						set_proc_arg( buf + 2 );
					}
				}
				reset_parse_buffer();

				rt = convert_block( bi, PARSE_LOCAL );

				close_proc_arg_table();
			} else {
				rt = convert_block( bi, PARSE_LOCAL );
			}
/*PRINTF( "BLOCK ARG END\n" );*/
			return rt;
		} else {
			/* 数字の羅列ブロックとして解釈する */
			return -1;
		}
	} else if( buf[ 0 ] == '\'' ){
		char *p;
		// 文字列
		p = buf + strlen( buf ) - 1;
		if( *p == '\'' ) *p = '\0';
		if( p_type == GCL_WILD || p_type == GCL_STRING ){
			set_string( bi, buf + 1, GCL_STRING );
		} else if( p_type == GCL_EXTCHARA2 ){
			set_external_chara_list( buf + 1 );
			set_string( bi, buf + 1, GCL_STRING );
		}
	} else if( buf[ 0 ] == '[' ){
		/* 文字列リソース参照 */
		if( p_type == GCL_WILD ){
			p_type = GCL_STRING;
		}
		if( p_type != GCL_STRING && p_type != GCL_STRRES ){
			ERROR( "[]は、リソース参照用です\n" );
		}
		set_string( bi, buf, p_type );
	} else if( ( ! is_strtype( p_type ) ) && ( is_expr_num( buf, &value ) ) ){
		/* 数値定数 */
		set_number( bi, value, p_type );
	} else if( buf[ 0 ] == 't' && buf[ 1 ] == ':' ){
		/* t_symbol */
		if( p_type == GCL_SYMBOL || p_type == GCL_SYMBOL2
		  || p_type == GCL_SYMBOL3 ){
			set_tsymbol( bi, buf, p_type );
		} else if( p_type == GCL_WILD ){
			set_tsymbol( bi, buf, GCL_SYMBOL3 );
		} else if( p_type == GCL_EXTCHARA ){
			set_external_chara_list( buf + 2 );
			set_tsymbol( bi, buf, GCL_SYMBOL2 );
		} else {
			ERROR( "t:シンボルが必要です。\n" );
		}
	} else if( buf[ 0 ] == 'p' && buf[ 1 ] == ':' ){
		if( p_type == GCL_PROCID || p_type == GCL_WILD ){
			set_string( bi, buf + 2, GCL_PROCID );
		} else {
			ERROR( "p:PROC名はここでは使えません。\n" );
		}
	} else if( buf[ 0 ] == '`' ){
		ERROR( "コマンドブロックは式の中で使用して下さい。\n" );
	} else {
		/* 文字定数 */
		set_string( bi, buf, p_type );
	}
	return 0;
}

static int analyze_args( BUFFER_INFO *bi, COMMAND_TABLE_TAG *tp )
{
	WORDBUF buf;
	int rf;

	rf = 0;
/*PRINTF( "analyze_args\n" );*/

	while( tp != NULL ){
		int p_type;
		if( get_table_type( tp ) == GCL_OPTION ) break;
		if( get_word( buf ) == NULL ){
			if( ! is_repeat_top() && !is_omitable_tag( tp ) ){
				ERROR( "引数が足りません(EOL) %X\n", tp->next->type );
			} else {
				break;
			}
		}
		if( buf[ 0 ] == '-' && buf[ 1 ] > '9' ){
			/* オプション？ */
			if( ! is_repeat_top() && !is_omitable_tag( tp ) ){
				ERROR( "引数が足りません %X\n", tp->next->type );
			} else {
PRINTF( "BREAK\n" );
				unget_word( buf );
				break;
			}
		}

		if( buf[ 0 ] == '}' ){
			rf = 1;
			break;
		}
		if( buf[ 0 ] == ',' ){
			/* 区切り記号のコンマは無視する */
			continue;
		}
		p_type = get_table_type( tp );
PRINTF( "TYPE %X\n", p_type );
		if(	analyze_type_and_store( bi, buf, p_type ) < 0 ){
			/* 数字の羅列ブロックが検出された */
			/* ベクトルか、数字のみのものとして検出 */
			int nest = 1;
PRINTF( "RARETU BLOCK\n" );
			while( tp != NULL ){
				if( get_word( buf ) == NULL ){
					get_line();
					continue;
				}
				if( buf[ 0 ] == '{' ){
					nest ++;
					continue;
				}
				if( buf[ 0 ] == '}' ){
					nest --;
					if( nest == 0 ){
						goto END;
					}
					continue;
				}
				if( buf[ 0 ] == ',' ){
					continue;
				}
				analyze_type_and_store( bi, buf, p_type );
#if 0
				if( ( tp = get_next_table( tp ) ) != NULL ){
					int type;
					if( ( type = get_table_type( tp ) ) != p_type ){
						ERROR( "不正な羅列ブロックです %d %d\n", type, p_type );
					}
				}
#else
				if( ( tp = get_next_table( tp ) ) != NULL ){
					p_type = get_table_type( tp );
				}
#endif
			}
			get_word( buf );
			if( buf[ 0 ] != '}' ){
				ERROR( "{}の対応が不正です。\n" );
			}
			goto END;
		}
		tp = get_next_table( tp );
	};
END:
	if( tp != NULL ){
		if( ! is_repeat_top() && !( is_omitable_tag( tp ) ) ){
			ERROR( "引数が足りません(EOL) %X\n", tp->next->type );
		}
	}
	current_tag = NULL;
	return rf;
}

static int store_option_args( BUFFER_INFO *bi, COMMAND_TABLE *ct )
{
	WORDBUF buf;
	COMMAND_TABLE_TAG *tp;
	int rf = 0;
	BLOCK_TOP top;

	/* '}'でおわったら 1, eolで終わったら0 */

	/* オプション処理 */
	
	for( ;; ){
		char *opt;

		if( get_word( buf ) == NULL ){
			break;
		}
		if( buf[ 0 ] == '}' ){
			rf = 1;
			break;
		}
		if( buf[ 0 ] != '-' ){
			opt = &( buf[ 0 ] );
		} else {
			opt = &( buf[ 1 ] );

			if( *opt == '\0' ){
				ERROR( "オプション指定が '-' のみです。\n" );
			}
		}
		PRINTF( "OPTION %s\n", opt );

		if( ( tp = get_table_arg( ct, opt ) ) == NULL ){
			ERROR( "オプション %s が引数リストにありません。\n", opt );
		}

		set_block_top( bi, GCL_OPTION, &top );

		set_byte( bi, current_tag->data[ 0 ] );
		PRINTF( "OPTION LETTER %c\n", current_tag->data[ 0 ] );

		if( tp->type == GCL_OPTION || tp->type == GCL_END ) tp = NULL;

		rf = analyze_args( bi, tp );

		set_block_end( &top );

		if( rf != 0 ){
			break;
		}
	}

	return rf;
}

static int store_main_args( BUFFER_INFO *bi, COMMAND_TABLE *ct )
{
	COMMAND_TABLE_TAG *tp;
	tp = get_table_arg( ct, NULL );
	return analyze_args( bi, tp );
}

static int extern_functions( char *command, BUFFER_INFO *bi )
{
	COMMAND_TABLE *ct;
	BLOCK_TOP top, argtop;
	int res;

	current_tag = NULL;

	set_block_top( bi, GCL_COMMAND, &top );

	if( command[ 0 ] == '%' ){
		// 特殊
		set_strcode( bi, get_strcode( CMD_COMMAND ) );
	} else {
		set_strcode( bi, get_strcode( command ) );
	}

	set_block_top( bi, GCL_NO_TAG, &argtop );

	if( strcmp( command, CHARA_COMMAND ) == 0 ){
		/* chara convert */
		WORDBUF buf;

		get_word( buf );
		if( ( ct = get_chara_command_table( buf, analyze_proc_id ) ) == NULL ){
			ERROR( "%s:未定義のキャラクラスです。\n", buf );
		}
		set_byte( bi, GCL_STRID );
		set_strcode( bi, get_strcode( buf ) );
	} else if( ( strcmp( command, CMD_COMMAND ) == 0 )
			   || command[ 0 ] == '%' ){
		/* chara convert */
		WORDBUF buf;

		if( command[ 0 ] == '%' ){
			strcpy( buf, command + 1 );
		} else {
			get_word( buf );
		}
		if( ( ct = get_cmd_command_table( buf, analyze_proc_id ) ) == NULL ){
			ERROR( "%s:未定義のキャラクラスです。\n", buf );
		}
		set_byte( bi, GCL_STRID );
		set_strcode( bi, get_strcode( buf ) );
	} else if( ( strcmp( command, BLOCK_COMMAND ) == 0 ) ){
		/* block convert */
		WORDBUF buf;

		if( block_output_mode == 0 ){
			ERROR( "ブロックモードではありません。\n" );
		}
		if( analyze_proc_id != 0 ){
			ERROR( "PROCの中でblock定義はできません。\n" );
		}
		get_word( buf );
		if( ( ct = get_block_command_table( buf ) ) == NULL ){
			ERROR( "%s:未定義のキャラクラスです。\n", buf );
		}
		set_byte( bi, GCL_STRID );
		set_strcode( bi, get_strcode( buf ) );

		get_word( current_block_name );
		unget_word( current_block_name );
	} else if( strcmp( command, MESG_COMMAND ) == 0 ){
		/* mesg convert */
		WORDBUF class, name, com;

		get_word_ex( class );
		get_word_ex( name );
		get_word_ex( com );

		if( ( ct = get_mesg_command_table( class, com, analyze_proc_id ) ) == NULL ){
			ERROR( "未定義の mesg コマンドです。\n" );
		}
		analyze_type_and_store( bi, name, GCL_STRID );

		set_byte( bi, GCL_STRID );
		set_strcode( bi, ct->command_id );
	} else if( strcmp( command, "if" ) == 0 ){
		/* if は特別扱い */
		int res;

		res = convert_if( bi, &argtop );
		set_byte( bi, GCL_END );
		set_block_end( &top );

		return res;
	} else if( strcmp( command, "switch" ) == 0 ){
		/* switchも特別扱い */
		int res;

		res = convert_switch( bi, &argtop );
		set_byte( bi, GCL_END );
		set_block_end( &top );

		return res;
	} else if( ( ct = get_extern_command_table( command, analyze_proc_id ) ) == NULL ){
		ERROR( "%s:未定義のコマンドです。\n", command );
	}
/*PRINTF( "COMMAND %s\n", command );*/

	/* オプション部以外の引数を解釈 */
	res = store_main_args( bi, ct );
	set_block_end( &argtop );

	if( res == 0 ){
		/* オプションが存在する */
		res = store_option_args( bi, ct );
	}

	set_byte( bi, GCL_END );
	set_block_end( &top );
	return res;
}

/* ---------------------------------------------------------- */
/*
	埋め込み関数定義
*/

/* ----- 内部関数 ---- */

static int analyze_dim( void )
{
	/* 配列の最大要素数宣言 */
	WORDBUF body, tmp;

	for( ;; ){
		int value;

		if( get_word( body ) == NULL ){
			break;
		}

		value = variable_type( body );
		if( value == GCL_ARG || value == GCL_STATUS ){
			ERROR( "dim の型が正しくありません。\n" );
		}

		if( get_word( tmp ) == NULL || tmp[ 0 ] != '[' ){
			ERROR( "dim:要素数が必要です。\n" );
		}
		get_word_ex( tmp );
		if( ! is_expr_num( tmp, &value ) ){
			ERROR( "数字が必要です。\n" );
		}

		if( value <= 0 ){
			ERROR( "添字は１以上の整数です。\n" );
		}

		add_array_dim( body, value );

		get_word_ex( tmp );
		if( tmp[ 0 ] != ']' ){
			ERROR( "dim:[]が閉じていません。\n" );
		}

		if( get_word( tmp ) != NULL ){
			if( tmp[ 0 ] != ',' ){
				ERROR( "パースエラー。\n" );
			}
		} else {
			break;
		}
	}
	return 0;
}

static int analyze_group_sub( void )
{
	analyze_group();
	return 0;
}

static int analyze_enum( void )
{
	parse_enum();
	return 0;
}

static int get_binary_block( char *type, void **bufp, int *len, int *align )
{
	/* バイナリブロックを読み込んでバッファを作成する */
	int al, sz;
	BUFFER_INFO bin;
	int nest;
	WORDBUF word;
	int s_flag = 0;
	int t_flag = 0;

	switch( type[ 1 ] ){
	  case 'b':
		al = 0; sz = 1;
		break;
	  case 'w':
		al = STRRES_TYPE_ALIGN4; sz = 2;
		break;
	  case 'i':
		al = STRRES_TYPE_ALIGN4; sz = 4;
		break;
	  case 't':
		t_flag = 1;
		al = STRRES_TYPE_ALIGN4; sz = 4;
		break;
	  case 's':
		s_flag = 1;
		al = STRRES_TYPE_ALIGN4; sz = 4;
		break;
	  case 'v':
		al = STRRES_TYPE_ALIGN16; sz = 4;
		break;
	  default:
		ERROR( "type %cはバイナリの形式として不適切です\n", type[ 1 ] );
	}
	init_buffer( &bin, 2 * 1024, BUF_WHICH_NONE );
PRINTF( "RES:al = %x sz = %d\n", al, sz );

	get_word_cl( word );
	if( word[ 0 ] != '{' ){
		ERROR( "バイナリブロックは{}で囲んで下さい\n" );
	}

	nest = 1;
	while( nest > 0 ){
		int value;

		get_word_cl( word );
		switch( word[ 0 ] ){
		  case '{':
			nest ++;
			continue;
		  case '}':
			nest --;
			continue;
		  case ',':
			continue;
		  case '(':
			/* 式 */
			{
				EXPR_INFO info, *ep;
				OPERAND *top;
				int old_mode;
				old_mode = parse_change_mode( PARSE_EXPR_MODE );
				ep = &info;
				init_expr( ep, ')' );
				top = set_expr( ep );
				if( ! calc_expr( ep, top, &value ) ){
					ERROR( "変数は使用できません\n" );
				}
				close_expr( ep );
				parse_change_mode( old_mode );
			}
			break;
		  case 't':
			if( word[ 1 ] == ':' ){
				/* tsymbol */
				if( t_flag == 0 ){
					ERROR( "tsymbolは許されません\n" );
				}
				value = get_symbol_code( &word[ 2 ], 0 );
				break;
			}
		  default:
			/* 数字 */
			if( !is_expr_num( word, &value ) ){
				/* 数字ではない */
				if( s_flag == 0 ){
					ERROR( "文字列IDは許されません\n" );
				}
				value = get_strcode( word );
			}
			break;
		}
PRINTF( "RES:VALUE = %d\n", value );
		set_value( &bin, value, sz );
	}
	*align = al;

	*bufp = malloc( get_now_pos( &bin ) );
	*len = bin.now_p;
	memcpy( *bufp, bin.buffer, *len );

	free_buffer( &bin );
PRINTF( "BINARY BLOCK SIZE = %d\n", *len );

	return 1;
}

static int get_gclcode_block( char *type, void **bufp, int *len )
{
	/* GCLコードを読み込んでバッファを作成する */
	BUFFER_INFO bin;
	int nest;
	WORDBUF word;

	init_buffer( &bin, 2 * 1024, BUF_WHICH_NONE );

	get_word_cl( word );
	if( word[ 0 ] != '{' ){
		ERROR( "ブロックは{}で囲んで下さい\n" );
	}

	nest = 1;
	while( nest > 0 ){
		get_word_cl( word );
		switch( word[ 0 ] ){
		  case '{':
			nest ++;
			continue;
		  case '}':
			nest --;
			continue;
		  case ',':
			continue;
		}
		analyze_type_and_store( &bin, word, GCL_WILD );
	}
	set_byte( &bin, GCL_END );

	*bufp = malloc( get_now_pos( &bin ) );
	*len = bin.now_p;
	memcpy( *bufp, bin.buffer, *len );

	free_buffer( &bin );
PRINTF( "BINARY BLOCK SIZE = %d\n", *len );

	return 1;
}

static void get_numeric_params( int params[], int max )
{
	int np = 0;
	WORDBUF buf;
	int valid = 0;

	while( get_word_cl( buf ) != NULL ){
		if( buf[ 0 ] == '{' || buf[ 0 ] == '<' ){
			unget_word( buf );
			break;
		}
		if( buf[ 0 ] == ',' ){
			if( valid == 0 ){
				ERROR( "値が必要です\n" );
			} else {
				valid = 0;
			}
			continue;
		}
		if( np < 5 ){
			int value;
			if( is_expr_num( buf, &value ) ){
				params[ np ] = value;
				np++;
			} else {
				ERROR( "数字が必要です\n" );
			}
		} else {
			ERROR( "パラメータが多すぎます\n" );
		}
	}
	if( np < max ){
		ERROR( "パラメータが足りません\n" );
	}
}

static int get_resource_option( char *word )
{
	int flag;
	if( strcmp( word + 1, "no_fontcheck" ) == 0 ){
		// 文字列描画チェックを行わない
		flag = STRRES_OPTION_NO_STRCHECK;
	} else if( strcmp( word + 1, "fontcheck" ) == 0 ){
		// パラメータ取得
		flag = STRRES_OPTION_STRCHECK_PARAM;
	} else {
		ERROR( "%s : 未定義のオプションです\n", word );
	}
	return flag;
}

#define MAX_STRING_RES	4096

static int analyze_resource( void )
{
	/* すでに resource タグは読み込まれている */

	WORDBUF tagname, word;
	BUFFER_INFO mesg;
	STRING_TAG *tag;
	int font_top_flag = 0;
	int top_params[ 5 ];
	int gf;

	get_word_ex( tagname );
	get_word_cl( word );

	if( word[ 0 ] == '-' ){
		font_top_flag = get_resource_option( word );
		if( font_top_flag & STRRES_OPTION_STRCHECK_PARAM ){
			get_numeric_params( top_params, 5 );
		}
		get_word_cl( word );
	}

	if( word[ 0 ] != '{' ){
		ERROR( "{ がありません。\n" );
	}

	tag = new_string_tag( tagname );

	init_buffer( &mesg, MAX_STRING_RES, BUF_WHICH_NONE );

	gf = 0;
	for( ;; ){
		static char label[ 256 ];
		int font_flag = font_top_flag;
		int *params = top_params;

		get_word_cl( word );
		if( word[ 0 ] == '\'' ){
			/* 文字列 */
			char *p;
			word[ strlen( word ) - 1 ] = '\0';
			p = word + 1;
			switch( gf ){
			  case 0:
				ERROR( "ラベルが必要です。\n" );
				break;
			  case 1:
				set_data( &mesg, p, strlen( p ) );
				gf = 2;
				break;
			  case 2:
				set_byte( &mesg, '\n' );
				set_data( &mesg, p, strlen( p ) );
				break;
			}
		} else if( strcmp( word, "$*:" ) == 0 ){
			/* GCLコードブロック */
			void *buf;
			int len;
			if( gf == 0 ){
				ERROR( "ラベルが必要です。\n" );
			}
			if( gf != 1 ){
				ERROR( "ブロックはラベルにつき1つのみ登録可能です\n" );
			}
			if( !get_gclcode_block( word, &buf, &len ) ){
				ERROR( "GCLブロックが閉じていません\n" );
			}
			set_binary_table( tag, label, buf, len, 0 );
			gf = 0;
			free( buf );
		} else if( word[ 0 ] == '$' ){
			/* バイナリブロック */
			void *buf;
			int len, align;
			if( gf == 0 ){
				ERROR( "ラベルが必要です。\n" );
			}
			if( gf != 1 ){
				ERROR( "バイナリブロックはラベルにつき1つのみ登録可能です\n" );
			}
			if( !get_binary_block( word, &buf, &len, &align ) ){
				ERROR( "バイナリブロックが閉じていません\n" );
			}
			set_binary_table( tag, label, buf, len, align );
			gf = 0;
			free( buf );
		} else if( word[ 0 ] == '{' ){
			/* 文字列ブロック */
			WORDBUF tmp;
			int mode;
			int sp_flag;

			parse_define_check_disable( 1 );
			mode = parse_change_mode( PARSE_STRING_MODE );

			if( gf == 0 ){
				ERROR( "ラベルが必要です。\n" );
			}
			while( get_word_no_pp( tmp ) == NULL ){
				if( !get_line() ){
					ERROR( "{}ブロックが途中で終っています\n" );
				}
			}
			if( tmp[ 0 ] == '}' ){
				// 空文字列
				goto STRING_NEXT;
			}
			set_data( &mesg, tmp, strlen( tmp ) );
			sp_flag = 1;

			for( ;; ){
				if( get_word_no_pp( tmp ) == NULL ){
					if( !get_line() ){
						ERROR( "{}ブロックが途中で終っています\n" );
					}
					set_byte( &mesg, '\n' );
					sp_flag = 0;
					continue;
				}
				if( strlen( tmp ) >= 256 ){
					ERROR( "1ワードは256文字までです\n" );
				}
				if( tmp[ 0 ] == '{' ){
					ERROR( "文字列ブロックが終っていません\n" );
				}
				if( tmp[ 0 ] == '}' ){
					break;
				}
				if( sp_flag == 1 ){
					set_byte( &mesg, ' ' );
				}
				set_data( &mesg, tmp, strlen( tmp ) );
				sp_flag = 1;
			}
STRING_NEXT:
			gf = 2;

			parse_change_mode( mode );
			parse_define_check_disable( 0 );
		} else if( strncmp( word, "<<", 2 ) == 0 ){
			// Here Document;
			WORDBUF tagbuf;
			int line_sep_flag;
			char *p;

			if( gf == 0 ){
				ERROR( "ラベルが必要です。\n" );
			}
			p = &word[ 2 ];
			if( *p == '-' ){
				line_sep_flag = 0;
				p++;
			} else {
				line_sep_flag = 1;
			}
			if( *p == '\0' ){
				if( get_word( word ) == NULL ){
					ERROR( "終了識別子が必要です\n" );
				}
				p = word;
			}
			strcpy( tagbuf, p );

			if( get_word( word ) != NULL ){
				ERROR( "おかしな文字があります\n", word );
			}

			while( get_line() ){
				char *line;
				int n;
				line = parse_get_current_linebuf();
				n = strlen( line ) - 1;
				while( isspace( line[ n ] ) ){
					line[ n ] = '\0';
				}
				if( strcmp( line, tagbuf ) == 0 ){
					get_line_ex();
					break;
				}
				set_data( &mesg, line, strlen( line ) );
				if( line_sep_flag != 0 ){
					set_byte( &mesg, '\n' );
				}
			}
			gf = 2;
		} else if( word[ strlen( word ) - 1 ] == ':' ){
			/* ラベル */
			word[ strlen( word ) - 1 ] = '\0';
			if( gf == 0 ){
				gf = 3;
			} else {
				gf = 4;
			}
		} else if( word[ 0 ] == '}' ){
			/* 終了 */
			if( gf == 0 ){
				gf = 6;
			} else {
				gf = 5;
			}
		} else if( word[ 0 ] == ',' ){
			/* 無視 */
			continue;
		} else if( word[ 0 ] == '-' ){
			/* option */
			if( gf != 1 ){
				ERROR( "オプションの位置が不正です\n" );
			}
			font_flag = get_resource_option( word );
			if( font_flag & STRRES_OPTION_STRCHECK_PARAM ){
				get_numeric_params( params, 5 );
			}
		} else {
			ERROR( "無意味な文字列 %sがあります。\n", word );
		}

		if( gf == 4 || gf == 5 ){
			/* バッファを登録する */
			STRING_TABLE *tp;

			set_byte( &mesg, '\0' );
			tp = set_string_table( tag, label, mesg.buffer );
			mesg.now_p = 0;	// バッファをリセット

			/* フォントオプションの設定 */
			if( ( tp->options = font_flag ) & STRRES_OPTION_STRCHECK_PARAM ){
				tp->u.font.w = params[ 0 ];
				tp->u.font.h = params[ 1 ];
				tp->u.font.c_skip = params[ 2 ];
				tp->u.font.l_skip = params[ 3 ];
				tp->u.font.flag = params[ 4 ];
#if 0
				printf( "fontcheck %d %d %d %d %d\n", 
					tp->u.font.w,
					tp->u.font.h,
					tp->u.font.c_skip,
					tp->u.font.l_skip,
					tp->u.font.flag );
#endif
			}
		}
		if( gf == 5 || gf == 6 ){
			break;
		}
		if( gf == 3 || gf == 4 ){
			/* ラベルを登録する */
			strcpy( label, word );
			gf = 1;

			/* ラベルを登録し終わったので、フラグ類を戻す */
			font_flag = font_top_flag;
			params = top_params;
		}
	}
	free_buffer( &mesg );
	
	return 0;
}

static int analyze_extern( void )
{
	WORDBUF word, buf;

	get_word_ex( word );
	get_word_ex( buf );
	if( strcmp( word, CHARA_COMMAND ) == 0 ){
		if( get_chara_command_table( buf, analyze_proc_id ) == NULL ){
			ERROR( "%s:未定義のキャラクラスです。\n", buf );
		}
	} else if( strcmp( word, CMD_COMMAND ) == 0 ){
		if( get_cmd_command_table( buf, analyze_proc_id ) == NULL ){
			ERROR( "%s:未定義のキャラクラスです。\n", buf );
		}
	} else {
		ERROR( "extern は charaもしくはcommandを設定して下さい\n" );
	}
	if( get_word( word ) != NULL ){
		ERROR( "無意味な文字列があります\n" );
	}
	return 0;
}

static int analyze_local( void )
{
	WORDBUF word;

	while( get_word( word ) != NULL ){
		if( strncmp( word, "$$:", 3 ) != 0 ){
			ERROR( "$$:型の変数名が必要です\n" );
		}
		if( current_arg_info == NULL || analyze_proc_p == NULL ){
			ERROR( "localはPROCの中でしか使えません\n" );
		}
		set_proc_local_arg( word + 3 );
	}
	return 0;
}

static int analyze_proc( void );

typedef struct {
	char *name;
	int ( *analyze_func )( void );	/* その行の最後が '}' だったら、1を返す */
} FUNCTION_INFO;

// 実際にはバイトコードに変換されない特殊命令

static FUNCTION_INFO builtin_func[] = {
	{ "proc",	analyze_proc },
	{ "group",  analyze_group_sub },
	{ "resource", analyze_resource },
	{ "enum", analyze_enum },
	{ "dim", analyze_dim },
	{ "extern", analyze_extern },
	{ "local", analyze_local },
};

#define BUILTIN_NUM	( sizeof( builtin_func ) / sizeof( FUNCTION_INFO ) )

static int is_builtin_functions( char *name )
{
	int i;

	for( i = 0; i < BUILTIN_NUM; i++ ){
		if( strcmp( name, builtin_func[ i ].name ) == 0 ){
PRINTF( "BUILTIN %s\n", name );
			return i;
		}
	}
	return -1;
}

static int builtin_functions( int no )
{
	return ( builtin_func[ no ].analyze_func )();
}

/* ---------------------------------------------------- */
/*
	proc 変換
*/

/* 実行ブロックは一度登録してから、使用されているものだけを出力する */

static int now_command_proc_num = 0;
static COMMAND_PROC *command_proc_top;
static COMMAND_PROC *command_proc_bottom;

int proc_prefix_check_mask = -1;

static HASH_INFO *proc_hash;

static void init_command_proc_work( void )
{
	now_command_proc_num = 0;
	command_proc_top = NULL;
	command_proc_bottom = NULL;

	proc_hash = hash_create( HASH_DEFAULT_SIZE, HASH_KEY_INT );
}

/* 必ずリストの最後につける */
static void add_command_proc( COMMAND_PROC *pr )
{
	pr->next = NULL;
	if( command_proc_bottom == NULL ){
		command_proc_top = pr;
	} else {
		command_proc_bottom->next = pr;
	}
	command_proc_bottom = pr;
}

#define PROC_REFER_MAX_STEP	16

static void set_proc_refer( COMMAND_PROC *proc )
{
	if( analyze_proc_id == 0 ){
		/* メインコンテキスト */
		proc->flag |= PROC_REFER;
	} else {
		/* proc定義中 */
		COMMAND_PROC *caller;
		caller = analyze_proc_p;
		if( caller->refer_num + 1 >= caller->refer_max ){
			caller->refer_max += PROC_REFER_MAX_STEP;
			caller->refer_p = my_realloc( caller->refer_p
										   , caller->refer_max * sizeof( COMMAND_PROC * ) );
		}
		caller->refer_p[ caller->refer_num ] = proc;
		caller->refer_num ++;
	}
}

static COMMAND_PROC *new_command_proc( char *name, int id, int pflag )
{
	COMMAND_PROC *n;

	n = Malloc( sizeof( COMMAND_PROC ) + strlen( name ) + 1 );

	memset( n, 0, sizeof( *n ) );

	strcpy( n->name, name );

	n->id = id;
	n->flag = pflag;
	n->refer_p = NULL;
	n->refer_num = 0;
	n->refer_max = 0;
	n->argnum = -1;
	n->local_argnum= 0;

	now_command_proc_num ++;

	return n;
}

static COMMAND_PROC *new_proc( char *org_name, int flag )
{
	COMMAND_PROC *n;
	unsigned int id;
	int pflag;
	int no;
	char * name = (char*)_alloca( strlen( org_name ) + 1 );

	{
		char *p;
		// ':番号'を取り除く
		strcpy( name, org_name );
		if( ( p = strrchr( name, ':' ) ) != NULL ){
			*p = '\0';
		}
	}

	pflag = PROC_UNDEFINE;

	id = get_expr_strcode( org_name );

	no = ( id >> 24 );
	id = ( id & 0x00FFFFFF );

PRINTF( "NEW_PROC %s, %d\n", name, flag );

	if( flag == PROC_DEFINE ){
		HASH_RESULT *result;
		if( ( result = hash_search_id( proc_hash, id ) ) != NULL ){
			COMMAND_PROC *proc = HASH_VALUE( result );

			if( proc->flag & PROC_DEFINE ){
				ERROR( "%s : 同じIDのPROCがあります。\n", name );
			} else {
				if( strcmp( proc->name, name ) != 0 ){
					ERROR( "%s : %s とPROC IDが衝突しています。\n"
						   , name, proc->name );
				}
				if( no == 0 ){
					if( proc->flag & PROC_EXTEND ){
						ERROR( "%s : 番号つきPROCが定義されています。\n", name );
					}
					// このprocを返す
					return proc;
				} else {
					COMMAND_PROC *p;

					if( !( proc->flag & PROC_EXTEND ) ){
						ERROR( "%s : 番号なしのprocがあります\n" );
					}
					proc->flag |= PROC_EXTEND;

					// 番号つきのprocを登録
					for( p = proc->sub; p != NULL; p = p->next ){
						if( p->prefix_no == no ){
							ERROR( "%s : すでに同じ番号のprocがあります\n", name );
						}
					}
					p = new_command_proc( name, id, pflag );

					p->prefix_no = no;

					p->next = proc->sub;
					proc->sub = p;

					return p;
				}
			}
		}
	} else if( flag == PROC_REFER ){
		if( no != 0 ){
			WARNING( "呼び出し側の番号は無視されます\n" );
			no = 0;
		}
	}

	n = new_command_proc( name, id, pflag );

	add_command_proc( n );
	if( hash_entry_id( proc_hash, id, n ) != NULL ){
		ERROR( "hashに登録できませんでした。\n" );
	}

	if( no != 0 ){
		// 番号つき
		COMMAND_PROC *np;
		n->flag |= PROC_EXTEND;

		np = new_command_proc( name, id, pflag );
		np->prefix_no = no;

		n->sub = np;

		return np;
	}

	return n;
}

/*
	proc 定義
*/

static int analyze_proc( void )
{
	/* 'proc' は読み込まれ済み */
	/* この時点では書き込みに行かないので、biは参照しない */
	WORDBUF buf;
	COMMAND_PROC *proc;
	int top;

	if( analyze_proc_id != 0 ){
		ERROR( "PROC定義の中でPROC定義はできません。\n" );
	}

	/* IDの読み込み */
	if( get_word( buf ) == NULL ){
		ERROR( "proc の名前がありません。\n" );
	}

	proc = new_proc( buf, PROC_DEFINE );
	PRINTF( "proc %s id %d\n", buf, proc->id );

	analyze_proc_id = proc->id;
	analyze_proc_p = proc;

	init_proc_arg_table( ARG_TYPE_PROC );

	for( ;; ){
		if( get_word_cl( buf ) == NULL ){
			ERROR( "proc %s : ブロックが始まっていません。\n", proc->name );
		}
		if( strcmp( buf, "{" ) == 0 ){
			break;
		}
		if( buf[ 0 ] == '$' && buf[ 1 ] == ':' ){
			set_proc_arg( buf + 2 );
		} else {
			ERROR( "proc定義に無意味な文字列'%s'があります。\n", buf );
		}
	}

	if( strcmp( buf, "{" ) != 0 ){
		ERROR( "proc %sの実体が見つかりません。\n", buf );
	}

	// procの実態はproc_bufに生成

	top = get_now_pos( &proc_buf );
	proc->flag |= PROC_DEFINE;

	proc->body = top;

	convert_block( &proc_buf, PARSE_LOCAL );

	proc->size = get_block_size( &proc_buf, top );

	if( proc->argnum == -1 ){
		proc->argnum = get_max_arg_num();
	} else {
		if( proc->argnum < get_max_arg_num() ){
			ERROR( "proc %s:参照している引数の個数(%d)が呼び出し側(%d)と一致していません。\n", proc->name, get_max_arg_num(), proc->argnum );
		}
	}
	proc->local_argnum = get_proc_local_arg_num();

	close_proc_arg_table();

	analyze_proc_id = 0;
	analyze_proc_p = NULL;

	return 0;
}

static COMMAND_PROC *set_call_proc( char *command )
{
	unsigned int id;
	HASH_RESULT *result;
	COMMAND_PROC *proc;

	id = get_expr_strcode( command );
	if( id & 0xFF000000 ){
		WARNING( "呼び出し側の番号は無視されます\n" );
		id = id & 0x00FFFFFF;
	}

	if( ( result = hash_search_id( proc_hash, id ) ) != NULL ){
		// 定義されているので、それを取得
		proc = HASH_VALUE( result );
	} else {
		/* まだ定義されていないので新規登録 */
		proc = new_proc( command, PROC_REFER );
	}
	set_proc_refer( proc );

	return proc;
}

static void reset_proc_call( void )
{
	COMMAND_PROC *p;

	for( p = command_proc_top; p != NULL; p = p->next ){
		p->flag &=~ ( PROC_REFER | PROC_REFDONE );
		if( p->flag & PROC_EXTEND ){
			COMMAND_PROC *pp;
			for( pp = p->sub; pp != NULL; pp = pp->next ){
				pp->flag &=~ ( PROC_REFDONE );
			}
		}
	}
}

/*
	proc 参照
*/

static COMMAND_PROC *is_user_proc_command( char *command )
{
	/* 呼び出し時には先頭に＠をつける */

	if( *command != '@' ){
		return NULL;
	}
	command ++;
	return set_call_proc( command );
}

static int user_functions( COMMAND_PROC *ptr, BUFFER_INFO *bi )
{
	BLOCK_TOP top;
	int rf = 0;
	int argnum;

PRINTF( "user_functions name = %s id = %d\n", ptr->name, ptr->id );
	set_block_top( bi, GCL_PROC, &top );

	set_strcode( bi, ptr->id );

	set_proc_refer( ptr );			/* 使用宣言 */

	for( argnum = 0;; argnum++){
		WORDBUF buf;
		if( get_word( buf ) == NULL ){
			break;
		}
		if( buf[ 0 ] == ',' ){
			/* 区切り記号のコンマは無視する */
			argnum --;
			continue;
		}
		if( buf[ 0 ] == '}' ){
			rf = 1;
			break;
		}
PRINTF( "ARG %s\n", buf );
		analyze_type_and_store( bi, buf, GCL_WILD );
	}
	set_byte( bi, GCL_END );

	if( ptr->flag & PROC_DEFINE ){
		/* すでにdefineされているので、引数の数のチェックを行う */
		if( ptr->argnum > argnum ){
			ERROR( "PROC %s の引数が足りません。\n", ptr->name );
		}
	} else {
		/* まだdefineされていないが,参照チェックを行なう */
		if( ptr->argnum == -1 ){
			ptr->argnum = argnum;
		} else {
			if( ptr->argnum != argnum ){
				WARNING( "proc '%s' の引数の数(%d)が前の呼び出し(%d)と異なります。\n", ptr->name, argnum, ptr->argnum );
				if( ptr->argnum > argnum ){
					ptr->argnum = argnum;
				}
			}
		}
	}
	set_block_end( &top );

	return rf;
}

/* ---------------------------------------------------------------------- */
/*
	式変換
*/

static int is_expr_command( char *command )
{
	if( strcmp( command, EVAL_COMMAND ) == 0 ){
		// 次のワードをセット
		if( get_word( command ) == NULL || command[ 0 ] != '(' ){
			ERROR( "式が必要です\n" );
		}
		return 1;
	} else if( command[ 0 ] == '$' || command[ 0 ] == '(' ){
		return 1;
	}
	return 0;
}

static int block_expr( BUFFER_INFO *bi, char *command )
{
	int end_char;

	if( command[ 0 ] == '(' ){
		end_char = ')';
	} else {
		back_word_ptr();
		end_char = '\0';
	}
	analyze_expr( bi, end_char );

	{
		WORDBUF word;
		if( get_word( word ) != NULL ){
			if( word[ 0 ] == '}' ){
				return 1;
			}
		}
	}

	return 0;
}

/* ----------------------------------------------------- */
/*
	ブロック変換
*/

static int block_nest;

static int convert_block( BUFFER_INFO *bi, int global_flag )
{
	/* global_flagがPARSE_LOCALの時は, '{'が読み込まれている */
	BLOCK_TOP top;
	int end_block_char = 0;
	int nest;
	char *file;
	int line;

	file = parse_get_current_filename();
	line = parse_get_current_linenum();

	set_block_top( bi, GCL_BLOCK, &top );
	nest = 0;

PRINTF( "BLOCK IN %d\n", block_nest ++ );
	do {
		char *command;
		WORDBUF buf;
NEXT:
		if( ( command = get_word( buf ) ) != NULL ){
			if( *command != '\0' && *command != '}' ){
				/* コマンド処理 */
				/* 処理の最後に '}'を検出したら、1を返す */
				int no;
				void *ptr;
PRINTF( "COMMAND %s\n", command );
				if( ( no = is_builtin_functions( command ) ) >= 0 ){
					end_block_char = builtin_functions( no );
				} else if( ( ptr = is_user_proc_command( command ) ) != NULL ){
					end_block_char = user_functions( ptr, bi );
				} else if( is_expr_command( command ) ){
					/* 式として解釈してみる */
					end_block_char = block_expr( bi, command );
				} else if( command[ 0 ] == '{' ){
					/* サブブロックの開始 */
					nest ++;
					goto NEXT;
				} else {
					end_block_char = extern_functions( command, bi );
/*PRINTF( "end_block_char %d\n", end_block_char );*/
				}
			}
			if( end_block_char || *command == '}' ){
				if( nest > 0 ){
					nest --;
				} else {
					if( global_flag == PARSE_GLOBAL ){
						/* proc外部 */
						ERROR( "無意味な '}'があります。\n" );
					} else {
						/* proc内部 */
						end_block_char = 1;
						break;
					}
				}
			}
		}
		if( get_word( buf ) != NULL ){
			ERROR( "文の後ろに無意味な文字列'%s'があります\n", buf );
		}
	} while( get_line() != 0 );

	if( global_flag == PARSE_LOCAL && end_block_char == 0 ){
		ERROR( "'}'が閉じていません。[%s:%d]\n", file, line );
	}

	if( global_flag == PARSE_GLOBAL && get_block_size( bi, top.top_p ) == sizeof( int ) ){
		printf( "NULL GCL\n" );
		return 0;
	}

	set_byte( bi, GCL_END );

	set_block_end( &top );
/*PRINTF( "end\n" );*/
PRINTF( "BLOCK OUT %d\n", -- block_nest );
	return 1;
}

/* ----------------------------------------------------- */
/*
	変換メイン／入出力

	以下のチャンク構造をもつGCOファイルとして出力

	+0 gco version
	'PRTB' size.l PROC TABLE
	'PROC' size.l PROC本体データ
	'BODY' size.l 本体パケット
	'VARS' size.l 変数情報
	'RSRC' size.l フォントデータ
	'STRG' size.l フォントデータ
	'FONT' size.l フォントデータ
	'END_' 終端
*/

static int write_tag( FILE *fp, char *tag )
{
	// タグは必ず４バイト

	return fwrite( tag, sizeof( char ), GCO_TAG_SIZE, fp );
}

static void init_all_buffer( void )
{
	init_buffer( &body_buf, BODY_BUFFER_SIZE, BUF_WHICH_BODY );
	init_buffer( &proc_buf, PROC_BUFFER_SIZE, BUF_WHICH_PROC );
	init_buffer( &value_buf, VALUE_BUFFER_SIZE, BUF_WHICH_NONE );
	init_buffer( &string_buf, STRING_BUFFER_SIZE, BUF_WHICH_NONE );

	init_command_proc_work();
	init_string_tag();
}

/*
	ファイル出力関数
*/

static void put_int( FILE *fp, int value )
{
	fwrite( &value, sizeof( int ), 1, fp );
}

static void put_short( FILE *fp, int value )
{
	short v;
	v = value;

	fwrite( &v, sizeof( short ), 1, fp );
}

static void write_buffer( FILE *fp, BUFFER_INFO *bi )
{
	int size;

	size = get_buffer_total_size( bi );

	put_int( fp, size );
	fwrite( bi->buffer, sizeof( char ), size, fp );
}

/*
	参照情報更新
*/

static void proc_refer_mark( COMMAND_PROC *proc )
{
	int i;

	if( proc->flag & PROC_REFDONE ){
		return;
	}

	proc->flag |= ( PROC_REFDONE | PROC_REFER );

	if( proc->flag & PROC_EXTEND ){
		COMMAND_PROC *pp;

		for( pp = proc->sub; pp != NULL; pp = pp->next ){
			for( i = 0; i < pp->refer_num; i++ ){
				proc_refer_mark( pp->refer_p[ i ] );
			}
		}
	} else {
		for( i = 0; i < proc->refer_num; i++ ){
			proc_refer_mark( proc->refer_p[ i ] );
		}
	}
}

static void proc_refer_check( void )
{
	/*
		メインコンテキストからの呼び出しは
		flagにPROC_REFERが立っているので
		そこから呼び出されるPROCをたどる
	*/

	COMMAND_PROC *pp;

	for( pp = command_proc_top; pp != NULL; pp = pp->next ){
		if( ( pp->flag & ( PROC_REFER | PROC_REFDONE ) ) == PROC_REFER ){
PRINTF( "REFER %s\n", pp->name );
			proc_refer_mark( pp );
		}
	}
}

static void proc_set_call_command_refer( void )
{
	COMMAND_PROC *pp;

	/* PROC中で使用されているコマンドに使用フラグをつける */
	proc_refer_check();

	for( pp = command_proc_top; pp != NULL; pp = pp->next ){
		if( ( pp->flag & PROC_OK ) == PROC_OK ){
			command_proc_call_set( pp->id );
		} else if( ( pp->flag & PROC_OK ) == PROC_REFER ){
			/* 参照だけされているが、定義されていない */
			if( !( pp->flag & PROC_EXTEND ) ){
				FATAL( "PROC %s が定義されていません。\n", pp->name );
				exit( 1 );
			}
			command_proc_call_set( pp->id );
		} else {
			PRINTF( "PROC %s SKIP\n", pp->name );
		}
	}
}

typedef struct {
	COMMAND_PROC *proc;
	unsigned int offset;
} PROC_OFFSET_TABLE;

static int proc_offset_table_comp( const void *p1, const void *p2 )
{
	const PROC_OFFSET_TABLE *t1 = p1;
	const PROC_OFFSET_TABLE *t2 = p2;

	return t1->offset - t2->offset;
}

static PROC_OFFSET_TABLE *sort_command_proc( void )
{
	PROC_OFFSET_TABLE *table;
	COMMAND_PROC *p;
	int i;
	int err_flag = 0;

	table = my_malloc( sizeof( PROC_OFFSET_TABLE ) * ( now_command_proc_num + 1 ) );

	i = 0;
	for( p = command_proc_top; p != NULL; p = p->next ){
		if( ( p->flag & PROC_EXTEND ) && ( p->flag & PROC_REFER ) ){
			COMMAND_PROC *pp;
			int prefix_mask = 0;

			for( pp = p->sub; pp != NULL; pp = pp->next ){
				if( proc_prefix_check_mask != -1 ){
					int prefix;
					if( ( prefix = pp->prefix_no ) > 32 ){
						WARNING( "[%s:%02X]32以上のprefixです\n", pp->name, prefix );
					} else {
						if( ( proc_prefix_check_mask & ( 1 << ( prefix - 1 ) ) ) == 0 ){
							PRINTF( "SKIP %s:%02X\n", pp->name, prefix );
							continue;
						}
						prefix_mask |= ( 1 << ( prefix - 1 ) );
					}
				}
				table[ i ].proc = pp;
				table[ i ].offset = pp->body;

				i++;
			}
			if( prefix_mask != 0 && proc_prefix_check_mask != -1 ){
				int mask;
				mask = ~prefix_mask & proc_prefix_check_mask;
				if( mask != 0 ){
					int i;
					for( i = 0; i < 32; i++ ){
						if( mask & ( 1 << i ) ){
							ERROR( "PROC %s:%02X が定義されていません\n", p->name, i + 1 );
						}
					}
				}
			}
		} else if( ( p->flag & PROC_OK ) == PROC_OK ){
			table[ i ].proc = p;
			table[ i ].offset = p->body;
			i++;
		} else if( ( p->flag & PROC_OK ) == PROC_REFER ){
			/* 参照だけされているが、定義されていない */
			DUMP( "ERROR:PROC %s が定義されていません。\n", p->name );
			err_flag = 1;
		} else {
			PRINTF( "PROC %s SKIP\n", p->name );
		}
	}
	if( err_flag != 0 ){
		DUMP( "未定義PROCが検出されました。\n" );
		my_free( table );
		return NULL;
	}
	qsort( table, i, sizeof( PROC_OFFSET_TABLE ), proc_offset_table_comp );

	table[ i ].proc = NULL;
	table[ i ].offset = 0;

	return table;
}

/*
	proc 情報書き出し
		整合性チェックと変数テーブルの書き換えを行う。
		宣言されていても呼び出されないprocは書き出さない。
*/

typedef struct {
	unsigned int id;
	unsigned int offset;
} PROC_ID_TABLE;

static int proc_table_comp( const void *p1, const void *p2 )
{
	const PROC_ID_TABLE *t1 = p1;
	const PROC_ID_TABLE *t2 = p2;

	return ( t1->id - t2->id );
}

static int write_proc( FILE *fp )
{
	char *buffer;
	char *now_p;
	int proc_top;
	BUFFER_INFO pb;
	int num;
	void *var_next;
	int str_next;
	PROC_OFFSET_TABLE *table;
	PROC_OFFSET_TABLE *ppt;

	COMMAND_PROC *pp;

	proc_refer_check();

	// この時点でOFFSETで整列されたデータがくる。
	// ここでreferチェックも行う。
	if( ( table = sort_command_proc() ) == NULL ){
		return -1;
	}

	/* 初期サイズは適当 */
	init_buffer( &pb, 4 * 1024, BUF_WHICH_NONE );

	/* procが定義されている */
	if( ( buffer = malloc( proc_buf.now_p ) ) == NULL ){
		DUMP( "write_proc:メモリオーバー\n" );
		return -1;
	}
	now_p = buffer;
	num = 0;
	var_next = NULL;
	str_next = 0;

	for( ppt = table; ppt->proc != NULL; ppt++ ){
		pp = ppt->proc;
		{
			/*
			   参照、定義両方されている。
			*/
			int size;
			PRINTF( "PROC %s PUT %X %X \n", pp->name, pp->id, pp->body );

			if( block_output_mode == 0 ){
				// ブロックモードでは必要ない
				command_proc_call_set( pp->id );
			}
			/*
			   ヘッダ部出力
			*/
			{
				int id;
				int ofs;

				id = pp->id | ( pp->prefix_no << 24 );
				ofs = now_p - buffer;
//printf( "output %X %X\n", id, ofs );
				if( pp->local_argnum > 0 ){
					ofs = ( pp->local_argnum << 24 ) | ofs;
				}
				set_int( &pb, id );
				set_int( &pb, ofs );
			}
			num ++;
			
			size = pp->size;
			memcpy( now_p, pp->body + proc_buf.buffer, size );
			{
				int top, offset;
				top = pp->body;	
				offset = ( now_p - buffer ) - top;
#if 1
				PRINTF( "top %d size %d offset %d\n", top, size, offset );
#endif
				var_next = change_proc_variable_offset( var_next, top, size, offset );
				str_next = change_proc_string_table_offset( str_next, top, size, offset );
			}
			/*PRINTF( "END\n" );*/
			now_p += size;
		}
	}

	free( table );

	/* ID順にソート */
	qsort( pb.buffer, num, sizeof( PROC_ID_TABLE ), proc_table_comp );

	/* 終端情報 */
	set_int( &pb, 0 );
	set_int( &pb, 0 );

	/* 実際に出力開始 */

	/* テーブル書き込み */
	write_tag( fp, GCO_PROCTABLE );
	put_int( fp, pb.now_p );
	fwrite( pb.buffer, sizeof( char ), pb.now_p, fp );
	free_buffer( &pb );

	write_tag( fp, GCO_PROC );
	put_int( fp, now_p - buffer );
	proc_top = ftell( fp );
	if( now_p > buffer ){
		fwrite( buffer, sizeof( char ), now_p - buffer, fp );
	}
	free( buffer );

	return proc_top;
}

/*
	文字列テーブル出力
	構造:
		+0 この文字列バッファそのもののサイズ
		+4 offset tableへのoffset
		+8 string dataへのoffset
		+C font dataへのoffset

		offset table
			+0 0へのoffset
			+4 1へのoffset
			:
			+4*n nへのoffset
*/

static void write_string_table( int proc_top, int body_top, FILE *fp )
{
	int top, size;
	BUFFER_INFO offset, string;
	STRING_REFER_TABLE *refp;
	STRING_TABLE *tablep;
	char *conv_buffer;
	int i, num, no;

	// もうすでに出力された、proc, bodyのバッファに関して、
	// 直接書き換えを行う。

	top = ftell( fp );

	init_buffer( &offset, 2 * 1024, BUF_WHICH_NONE );
	init_buffer( &string, 2 * 1024, BUF_WHICH_NONE );

	refp = ( STRING_REFER_TABLE * )string_refer_table.buffer;
	tablep = ( STRING_TABLE * )string_table.buffer;

	if( ( conv_buffer = my_malloc( 1024*1024 ) ) == NULL ){
		ERROR( "メモリが足りません。\n" );
	}

	no = 0;
	num = string_refer_table.now_p / sizeof( STRING_REFER_TABLE );

	for( i = 0; i < num; i++, refp++ ){
		if( refp->which >= 0 ){
			/* callされている */
			int ofs;
			STRING_TABLE *tp;

			tp = tablep + refp->no;

			if( tp->out_pos < 0 ){
				// まだ登録していないので、登録を行なう。
				char *mesg;

				mesg = get_string_buf( tp->offset );
PRINTF( "NO = %d\n", no );
				if( tp->type & STRRES_TYPE_STRING ){
					int res;
PRINTF( "OUTPUT %s\n", mesg );

				    res = font_conv_buffer( conv_buffer, mesg );

					if( ( tp->options & STRRES_OPTION_NO_STRCHECK ) == 0 
					   && refp->checked == 0 ){

						FONT_AREA_CHECK font_area;
						FONT_AREA_CHECK *area = NULL;

						if( tp->options & STRRES_OPTION_STRCHECK_PARAM ){
							// パラメータを再設定する。

							font_area.width = tp->u.font.w;
							font_area.height = tp->u.font.h;
							font_area.c_skip = tp->u.font.c_skip;
							font_area.l_skip = tp->u.font.l_skip;
							font_area.kinsoku = tp->u.font.flag;

							area = &font_area;
						}
						res |= fontconv_area_check( conv_buffer, area );
					    
						if( res & FONTCONV_RESULT_OVER ){
							WARNING( "文字列 '%s:%s'\n%s\n--は表示領域をはみ出しそうです。\n"
									, get_string_buf( tp->tag_offset )
									, get_string_buf( tp->label_offset )
									, mesg );
						}
						if( res & FONTCONV_RESULT_ORIKAESHI ){
							WARNING( "文字列 '%s:%s'\n%s\n--は折り返しています。\n"
									, get_string_buf( tp->tag_offset )
									, get_string_buf( tp->label_offset )
									, mesg );
						}
#if 0
						if( res & FONTCONV_NO_ZENKAKU ){
							WARNING( "文字列 '%s:%s'\n%s\n--全角文字がありません。\n"
									, get_string_buf( tp->tag_offset )
									, get_string_buf( tp->label_offset )
									, mesg );
						}
#endif
						refp->checked = 1;
					}

					ofs = set_data( &string, conv_buffer, strlen( conv_buffer ) + 1 );
					ofs = ofs | 0x80000000;
				} else if( tp->type & STRRES_TYPE_BINARY ){
PRINTF( "OUTPUT binary\n" );
					/* アライメントが必要な場合はダミーでデータを入れる */
					if( tp->type & STRRES_TYPE_ALIGN4 ){
						set_align( &string, 4 );
					} else if( tp->type & STRRES_TYPE_ALIGN16 ){
						set_align( &string, 16 );
					}
					ofs = set_data( &string, mesg, tp->length );
				} else {
					ERROR( "STRRESのtypeが異常です%08X\n", tp->type );
				}

				tp->out_pos = no;
				set_int( &offset, ofs );
				no++;
			}

			if( refp->no_caller == STRING_WRITEBACK_CALLER ){
				/* 相手先バッファの書き換え */
				if( refp->which == 0 ){
					// 本体情報
					fseek( fp, body_top + refp->out_offset, SEEK_SET );
PRINTF( "body pos %X\n", body_top + refp->out_offset );
				} else if( refp->which == 1 ){
					// PROC情報
					fseek( fp, proc_top + refp->out_offset, SEEK_SET );
PRINTF( "proc pos %X\n", proc_top + refp->out_offset );
				}
				put_short( fp, tp->out_pos );
			}
		}
	}

	my_free( conv_buffer );

	set_align( &offset, 4 );

	/* このSTRINGブロックの出力開始 */

	fseek( fp, top, SEEK_SET );

	/* 文字列テーブル */
	write_tag( fp, GCO_RESOURCETBL );
	put_int( fp, offset.now_p );
	fwrite( offset.buffer, offset.now_p, sizeof( char ), fp );

	/* 文字列本体 */
	write_tag( fp, GCO_STRINGTBL );
	put_int( fp, string.now_p );
	fwrite( string.buffer, string.now_p, sizeof( char ), fp );

	/* フォント */
	write_tag( fp, GCO_FONT );
	top = ftell( fp );
	put_int( fp, 0 );	// DUMMY
	font_output_font_data( fp );
	size = ftell( fp ) - top - sizeof( int );
	fseek( fp, top, SEEK_SET );
	put_int( fp, size );
	fseek( fp, 0, SEEK_END );

	/* テンポラリバッファの開放 */
	free_buffer( &offset );
	free_buffer( &string );
}

static void write_variable( FILE *fp )
{
	/* いらないものは出力しない */
	int top, size;
	VARIABLE *vp;
	void *end;

	top = ftell( fp );
	put_int( fp, 0 );	// dummy

	vp = ( VARIABLE * )value_buf.buffer;
	end = ( void * )(value_buf.buffer + value_buf.now_p);

	while( ( void * )vp < end ){
		if( vp->which >= 0 ){
			fwrite( vp, sizeof( VARIABLE ), 1, fp );
			fwrite( ( char * )( vp + 1 ), sizeof( char ), vp->name_len, fp );
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vp->name_len );
	}

	size = ftell( fp ) - top - sizeof( int );

	fseek( fp, top, SEEK_SET );
	put_int( fp, size );
	fseek( fp, 0, SEEK_END );
}

/*
   出力処理メイン
*/

static int output_file( FILE *fp )
{
	int proc_top;
	int body_top;

	extern int gco_version;	/* in gclconv.c */

	put_int( fp, gco_version );

	if( ( proc_top = write_proc( fp ) ) < 0 ){
		return -1;
	}

	write_tag( fp, GCO_BODY );
	body_top = ftell( fp ) + sizeof( int );	// 先頭のサイズを飛ばす
	write_buffer( fp, &body_buf );

	write_tag( fp, GCO_VARS );
	write_variable( fp );

	write_string_table( proc_top, body_top, fp );

	write_tag( fp, GCO_END );

	return 0;
}

void convert_script( char *infile, char *outfile )
{
	FILE *fp;
	int err;

	/* 全バッファのイニシャライズ */
	init_all_buffer();

	parse_init();
	if( ! load_file( infile ) ){
		FATAL( "Error:ファイル %s がオープンできません\n", infile );
	}
	printf( "converting %s ...\n", infile );

	block_nest = 0;

	convert_block( &body_buf, PARSE_GLOBAL );

	parse_end();
	/*
	   ここまでで、body_bufには本体のバイナリ、
	   value_bufには、使用される変数情報のバイナリ
	   proc_bufには、定義されたprocの本体のバイナリ
	   が入っている。
	*/
	if( verbose_mode ){
PRINTF( "DUMP VARIABLE\n" );
		dump_variable();
//		return;
	}

	if( outfile == NULL ){
		/* proc中からのコマンド参照情報のみを設定 */
		proc_set_call_command_refer();
		return;
	}

	if( ( fp = fopen( outfile, "wb" ) ) == NULL ){
		ERROR( "ファイル %sがオープンできません。\n", outfile );
	}
	err = output_file( fp );

	fclose( fp );

	if( err < 0 ){
		remove( outfile );
		exit( 1 );
	}
}

/* ----------------------------------------------------------------- */

static int convert_block_mode( BUFFER_INFO *bi, FILE *fp )
{
	int pos, end;
	BLOCK_TOP top;

	// 呼出し情報のクリア
	reset_buffer( bi );

	reset_proc_call();
	reset_variable_call();
	reset_string_call();
	font_reset_font_table();

	set_block_top( bi, GCL_BLOCK, &top );

	extern_functions( BLOCK_COMMAND, bi );

	set_byte( bi, GCL_END );

	set_block_end( &top );

	// 通常のGCLと同じ様に出力する。
	// ただし,先頭にブロック全体のサイズとブロック名を記録
	
	pos = ftell( fp );
	put_int( fp, 0 );
PRINTF( "BLOCK TOP = %X %s\n", pos, current_block_name );
	{
		int len;
		len = strlen( current_block_name ) + 1;
		fputc( len, fp );
		fwrite( current_block_name, sizeof( char ), len, fp );
	}
	if( output_file( fp ) < 0 ){
		return -1;
	}
	end = ftell( fp );
	fseek( fp, pos, SEEK_SET );
	put_int( fp, end - pos );
	fseek( fp, end, SEEK_SET );

	return 0;
}

void convert_script_block_mode( char *infile, char *outfile )
{
	FILE *outfp;

	if( ( outfp = fopen( outfile, "wb" ) ) == NULL ){
		FATAL( "Error:ファイル %s がオープンできません\n", outfile );
	}

	/* 全バッファのイニシャライズ */
	init_all_buffer();

	parse_init();

	if( ! load_file( infile ) ){
		FATAL( "Error:ファイル %s がオープンできません\n", infile );
	}
	printf( "converting %s ...\n", infile );

	block_nest = 0;

	do {
		char *command;
		WORDBUF buf;

		if( ( command = get_word( buf ) ) != NULL ){
			if( *command != '\0' ){
				int no;

				if( ( no = is_builtin_functions( command ) ) >= 0 ){
					builtin_functions( no );
				} else if( strcmp( command, BLOCK_COMMAND ) == 0 ){
					if( convert_block_mode( &body_buf, outfp ) < 0 ){
						fclose( outfp );
						remove( outfile );
						FATAL( "Errorが発生しました\n" );
					}
				} else {
					ERROR( "block, proc 以外のコマンドがあります\n" );
				}
			}
		}
	} while( get_line() != 0 );

	parse_end();
	fclose( outfp );
}
