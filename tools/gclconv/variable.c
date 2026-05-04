/*
   変数登録、解析ルーチン for gclk
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include "gclconv.h"
#include "gclk.h"
#include "expr.h"
#include "mymalloc.h"
#include "gcoform.h"

int header_output_flag = FALSE;

static char *current_file = NULL;

#define MAX_VARIABLE	(16*1024)
#define MAX_VAR_NAME	(64*1024)

#define TYPE_NORMAL		0x00
#define TYPE_LINKVAR	0x01
#define TYPE_LOCALVAR	0x02

static int current_type = TYPE_NORMAL;
static int linkvar_load_flag = FALSE;

static int linkvar_max;
static int variable_table_date;

static char type_str[] = "nlb";

extern char variable_table_name[ 128 ];

#define TYPE( a, b )		( ( (b) << 8 ) | ((a) & 0xff) )

#define VAR_TYPE( a )		( ( (a)->type ) >> 8 )

#define IS_LINKVAR( a )		( VAR_TYPE( a ) == TYPE_LINKVAR )

#define VAR_TYPE_STR( a )	( type_str[ VAR_TYPE( a ) ] )

#define IS_ARRAY( a )		( ( (a) & 0xff ) == GCL_ARRAY )

#define SECTOR_SIZE 2048

#define LOCAL_VARIABLE_FLAG 0x10000	// gcoの変数バッファ中のタイプ

#if 0
static int set_var_type( char c )
{
	int i;
	for( i = 0; type_str[ i ] != '\0'; i++ ){
		if( type_str[ i ] == c ){
			return i;
		}
	}
	return -1;
}
#endif

typedef struct {
	char *name;
	short type;
	short flag;
	int block;
	int max;
	int code;
} VAR;

typedef struct {
	VAR *table;
	int table_p;
	int max;
} VAR_ALLOC_TABLE;

static VAR_ALLOC_TABLE normal_var;
static VAR_ALLOC_TABLE local_var;

static char *var_str_buf;
static char *var_str_buf_p;

typedef struct {
	char *buf;
	int size;
} VAR_MAP_BUF;

static VAR_MAP_BUF normal_map = {
	NULL, MAX_VAR_BUF
};
static VAR_MAP_BUF local_map = {
	NULL, MAX_LOCAL_VAR_BUF
};

static int max_local_var_buf = 0;
static char max_local_var_filename[ 256 ] = "";

#define VAR_FLAG_NONE	0x00
#define VAR_FLAG_REF	0x01		/* 参照:（元データは０） */
#define VAR_FLAG_SET	0x02		/* 代入:（元データは１） */

#define VAR_FLAG( a )	( 0x01 << ( a ) )

/* -------------------------------------------------------- */
/*
	変数情報管理
*/

void set_variable_buffer_size( int size )
{
	normal_map.size = size;
}

void set_local_variable_buffer_size( int size )
{
	local_map.size = size;
}

static void reset_var_map_buf( VAR_MAP_BUF *map )
{
	memset( map->buf, 0, map->size );
}

static void init_var_map_buf( VAR_MAP_BUF *map )
{
	if( ( map->buf = malloc( map->size ) ) == NULL ){
		ERROR( "メモリが足りません\n" );
	}
	reset_var_map_buf( map );
}

static void init_var_alloc_table( VAR_ALLOC_TABLE *alloc_table, int max )
{
	/* 変数ワーク確保 */
	if( ( alloc_table->table = malloc( sizeof( VAR ) * max ) ) == NULL ){
		ERROR( "メモリが足りません\n" );
	}
	alloc_table->table_p = 0;
	alloc_table->max = max;
}

static void reset_var_alloc_table( VAR_ALLOC_TABLE *alloc_table )
{
	// すでにメモリが確保されていることが前提
	alloc_table->table_p = 0;
}

static VAR *alloc_new_var( VAR_ALLOC_TABLE *alloc_table )
{
	VAR *new;

	if( alloc_table->table_p + 1 >= alloc_table->max ){
		alloc_table->max += MAX_VARIABLE;
		alloc_table->table = my_realloc( alloc_table->table
										, alloc_table->max * sizeof( VAR ) );
	}
	new = alloc_table->table + alloc_table->table_p;
	alloc_table->table_p++;

	return new;
}

static VAR *new_var( VAR_ALLOC_TABLE *alloc_table, char *name, int type, int flag, int max )
{
	VAR *new;
	int i;

	/* すでに登録されているかどうかの検索 */
	for( i = 0; i < alloc_table->table_p; i++ ){
		if( strcmp( alloc_table->table[ i ].name, name ) == 0 ){
			VAR *var;
			var = &( alloc_table->table[ i ] );
			if( ( var->type & 0xff ) != ( type & 0xff ) ){
				if( current_file != NULL ){
					printf( "%s:", current_file );
				}
				if( type & LOCAL_VARIABLE_FLAG ){
					ERROR( "ローカル変数'%s'が違った型でつかわれています。(%X %X)\n"
						  , name, var->type & 0xFF, type & 0xFF );
				} else {
					ERROR( "変数'%s'が違った型でつかわれています。(%X %X)\ngclを修正するか%sを消して作り直してください\n", name, var->type, type, variable_table_name );
				}
			}
			if( var->max != max ){
				if( current_file != NULL ){
					printf( "%s:", current_file );
				}
				if( type & LOCAL_VARIABLE_FLAG ){
					ERROR( "ローカル配列'%s'の最大値が異なります。\n", name );
				} else {
					ERROR( "配列'%s'の最大値が異なります。\ngclを修正するか%sを消して作り直してください\n", name, variable_table_name );
				}
			}
			var->flag |= flag;
			return var;
		}
	}

	/* 新規登録 */

	new = alloc_new_var( alloc_table );
	new->code = -1;
	new->type = type;
	new->flag = flag;
	new->block = 0;
	new->max = max;

	if( var_str_buf_p + strlen( name ) + 1 > var_str_buf + MAX_VAR_NAME ){
		// 溢れたので次のバッファを確保
		var_str_buf = my_malloc( MAX_VAR_NAME );
		var_str_buf_p = var_str_buf;
	}

	new->name = var_str_buf_p;
	strcpy( new->name, name );
	var_str_buf_p += strlen( name ) + 1;
PRINTF( "NEW VAR %s %X\n", name, type );
	return new;
}

void init_variable_table( void )
{
	linkvar_max = 0;
	variable_table_date = -1;

	init_var_alloc_table( &normal_var, MAX_VARIABLE );
	init_var_alloc_table( &local_var, MAX_VARIABLE );

	/* 変数名ワーク確保 */
	if( ( var_str_buf = malloc( MAX_VAR_NAME ) ) == NULL ){
		ERROR( "メモリが足りません\n" );
	}
	var_str_buf_p = var_str_buf;

	/* 変数マッピングテーブル確保 */
	init_var_map_buf( &normal_map );
	init_var_map_buf( &local_map );
}

static int get_type( char *varname )
{
	int type;
	switch( varname[ 1 ] ){
	  case 'i':
		type = GCL_INT | GCL_VAR;
		break;
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
		/* 24ビット文字コード */
		type = GCL_STRID | GCL_VAR;
		break;
	  default:
		ERROR( "%s: syntax error\n", varname );
	}
	return type;
}

static VAR *search_var( VAR_ALLOC_TABLE *alloc_table, char *varname )
{
	int i;
	char *name;

	if( ( name = strchr( varname, ':' ) ) == NULL ){
		name = varname;
	}

	for( i = 0; i < alloc_table->table_p; i++ ){
		if( strcmp( name, alloc_table->table[ i ].name ) == 0 ){
			return &( alloc_table->table[ i ] );
		}
	}
	return NULL;
}

/* -------------------------------------------------------- */
/*
	プログラムとのリンク変数を読み込む。
	( linkvar.def )
*/

static int linkvar_end_pos;

void load_linkvar_table( char *filename )
{
	int pos;
	FILE *fp = NULL;

	parse_init();
	if( !load_file( filename ) ){
		ERROR( "Can't open %s\n", filename );
	}

	/* 登録のみ行う */

	/* ヘッダファイル出力オプションがついていたら、ヘッダファイルを出力 */

	if( header_output_flag ){
		if( ( fp = fopen( "linkvar.h", "wt" ) ) != NULL ){
			/* 共通部分出力 */
			fprintf( fp, "/*\n" );
			fprintf( fp, "\t\theader file for system link variable\n" );
			fprintf( fp, "*/\n\n" );
		}
	}

	pos = 0;
	while( get_line() ){
		WORDBUF buf;
		if( get_word( buf ) != NULL ){
			int type;
			int num;
			char *name;
			type = get_type( buf );
			if( ( name = strchr( buf, ':' ) ) != NULL ){
				VAR *new;
				WORDBUF prog_name;

				new = new_var( &normal_var, name + 1
							  , TYPE( type, TYPE_LINKVAR ), VAR_FLAG_NONE, 1 );
				new->code = VAR_TYPE_LINK | pos;
				PRINTF( "linkvar %s %X\n", new->name, new->code );

				num = 1;
				{
					WORDBUF buf;
					
					get_word( buf );
					if( buf[ 0 ] == '[' ){
						/* array */
						int value;
						get_word( buf );
						if( ! is_expr_num( buf, &value ) ){
							ERROR( "linkvar: 数字が必要です\n" );
						}
						new->max = value;
						new->type = ( new->type & 0xff0f ) | GCL_ARRAY;
						num = value;
						get_word( buf );
						if( buf[ 0 ] != ']' ){
							ERROR( "linkvar: []が閉じていません\n" );
						}
					} else {
						unget_word( buf );
					}
				}

				get_word( prog_name );
				type = ( type & 0x0f );
				switch( type ){
				  case GCL_INT:
#ifndef STRCODE16
				  case GCL_STRID:
#endif
					if( ( pos & 3 ) != 0 ){
						/* int align に載っていない */
						pos += sizeof( short );
						if( fp != NULL ){
							WARNING( "linkvar: %s : int align に載っていません。補正します。\n", prog_name );
						}
					}
					if( fp != NULL ){
						fprintf( fp, "#define %s\t\t(%s( int *)( linkvarbuf + %d ))\n"
								, prog_name, ( num > 1 ) ? "" : "*", pos );
					}
					pos += sizeof( int ) * num;
					break;
				  case GCL_SHORT:
#ifdef STRCODE16
				  case GCL_STRID:
#endif
					if( fp != NULL ){
						fprintf( fp, "#define %s\t\t(%s( short * )( linkvarbuf + %d ))\n"
								, prog_name, ( num > 1 ) ? "" : "*", pos );
					}
					pos += sizeof( short ) * num;
					break;
				  default:
					ERROR( "linkvar: %s:サポートしていません\n", new->name );
				}
			} else {
				ERROR( "libkvar: %s : フォーマットが不正です\n", buf );
			}
		}
	}
	linkvar_end_pos = pos;
	linkvar_max = pos;

	if( fp != NULL ){
		fprintf( fp, "\n#define MAX_LINKVARBUF %d\n\n", pos );
#if 0
		fprintf( fp, "#ifdef __VARIABLE_C__\n" );
		fprintf( fp, "char linkvarbuf[ MAX_LINKVARBUF ];\n" );
		fprintf( fp, "char linkvarbuf[ MAX_LINKVARBUF ];\n" );
		fprintf( fp, "#else\n" );
		fprintf( fp, "extern char linkvarbuf[ MAX_LINKVARBUF ];\n" );
		fprintf( fp, "#endif\n\n" );
#else
		fprintf( fp, "extern char *linkvarbuf;\n" );
#endif
		fclose( fp );
	}
	linkvar_load_flag = TRUE;
}

/* -------------------------------------------------------- */
/*
	変数情報管理とマッピング
*/

static int get_int( void *ptr )
{
	unsigned char *p = ptr;
	int v;

	v = ( p[ 3 ] << 24 ) | ( p[ 2 ] << 16 ) | ( p[ 1 ] << 8 ) | p[ 0 ];

	return v;
}

/* gco フォーマット中の管理情報 */
typedef struct {
	int which;
	int flag;
	int offset;
	int out_offset;
	int type;
	short name_len;
	short array_max;
} VARIABLE;

static void set_global_variables( char *top )
{
	VARIABLE *vp, vr;
	char *end;

	vp = ( VARIABLE * )( top + sizeof( int ) );
	end = top + sizeof( int ) + get_int( top );

	while( ( void * )vp < ( void * )end ){
		memcpy( &vr, vp, sizeof( VARIABLE ) );

		if( ! ( vr.type & LOCAL_VARIABLE_FLAG ) ){
			// ローカルはこの関数では登録しない。
			new_var( &normal_var, ( char * )( vp + 1 ), vr.type
					, VAR_FLAG( vr.flag ), vr.array_max );
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vr.name_len );
	}
}

static int array_search_mark( VAR_MAP_BUF *map, int align, int size, int len )
{
	int i, l;
	unsigned char *p;
	unsigned char *top;

	p = ( unsigned char * )map->buf;
	top = NULL;
	l = 0;
	for( i = map->size; i > 0; i -= align ){
		int j;
		for( j = 0; j < size; j++ ){
			if( *p != 0x00 || ( l == 0 && j != 0 ) ){
				l = 0;
			} else {
				if( l == 0 ){
					top = p;
				}
				l++;
				if( l == len * size ){
					goto MARK;
				}
			}
			p++;
		}
		p += align - size;
	}
	ERROR( "変数データが領域(%dbyte)をオーバーしました\n", map->size );
MARK:
	for( i = len, p = top; i > 0; i-- ){
		int j;
		for( j = 0; j < size; j++ ){
			*p = 0xff;
			p++;
		}
		p += align - size;
	}
	
	return ( ( char * )top - ( char * )map->buf );
}

static int array_search_mark_bits( VAR_MAP_BUF *map, int len )
{
	unsigned char *p;
	int i, l;
	unsigned char *top;
	int top_bit = 0;

	p = ( unsigned char * )map->buf;
	top = NULL;
	l = 0;
	for( i = map->size; i > 0; i-- ){
		int mask, j;
		mask = 1;
		for( j = 0; j < 8; j++ ){
			if( *p & mask ){
				l = 0;
			} else {
				if( l == 0 ){
					top = p;
					top_bit = j;
				}
				l++;
				if( l == len ){
					goto MARK;
				}
			}
			mask = mask << 1;
		}
		p++;
	}
	ERROR( "変数データが領域(%dbyte)をオーバーしました\n", map->size );
MARK:
	for( i = len, l = top_bit, p = top; i > 0; i-- ){
		*p |= ( 1 << l );
		l++;
		if( l == 8 ){
			p++;
			l = 0;
		}
	}
	return ( ( char * )top - ( char * )map->buf ) | ( top_bit << 16 );
}

static int search_var_map( VAR_MAP_BUF *map, int type, int max )
{
	switch( type & 0x0f ){
	  case GCL_SHORT:
		return array_search_mark( map, sizeof( short ), sizeof( short ), max );

	  case GCL_STRID:
#ifdef STRCODE16
		return array_search_mark( map, sizeof( short ), sizeof( short ), max );
#else
		return array_search_mark( map, sizeof( int ), sizeof( int ), max );
#endif

	  case GCL_INT:
		return array_search_mark( map, sizeof( int ), sizeof( int ), max );

	  case GCL_BYTE:
	  case GCL_CHAR:
		return array_search_mark( map, 1, 1, max );

	  case GCL_BOOL:
		return array_search_mark_bits( map, max );

	  default:
		ERROR( "type err\n" );
		return -1;
	}
	ERROR( "変数バッファのサイズが足りません\n" );
}

static void set_alloc_map( VAR_MAP_BUF *map, int type, int code, int max )
{
	char *p;
	int i;

	switch( type & 0x0f ){
	  case GCL_INT:
		/* 32 Bit */
		if( code + 4 * max >= map->size ){
			goto ERR;
		}
		p = map->buf + code;
		for( i = 0; i < max; i++ ){
			p[ 0 ] = p[ 1 ] = p[ 2 ] = p[ 3 ] = 0xff;
			p += 4;
		}
		break;
	  case GCL_STRID:
#ifndef STRCODE16
		/* 32 Bit */
		if( code + 4 * max >= map->size ){
			goto ERR;
		}
		p = map->buf + code;
		for( i = 0; i < max; i++ ){
			p[ 0 ] = p[ 1 ] = p[ 2 ] = p[ 3 ] = 0xff;
			p += 4;	/* int アライメントにあわせる */
		}
		break;
#endif // STRCODE16Bit時はSHORTと同じ扱い。
	  case GCL_SHORT:
		/* 16 bit */
		if( code + 2 * max  >= map->size ){
			goto ERR;
		}
		p = map->buf + code;
		for( i = 0; i < max; i++ ){
			p[ 0 ] = p[ 1 ] = 0xff;
			p += 2;
		}
		break;
	  case GCL_BYTE:
	  case GCL_CHAR:
		if( code + 1 * max >= map->size ){
			goto ERR;
		}
		p = map->buf + code;
		for( i = 0; i < max; i++ ){
			*p = 0xff;
			p++;
		}
		break;
	  case GCL_BOOL:
		{
			int bit;
			p = map->buf + ( code & 0xffff );
			bit = ( ( code >> 16 ) & 0x07 );

			if( ( code & 0xFFFF ) + ( bit + max + 7 ) / 8 >= map->size ){
				goto ERR;
			}
			for( i = 0; i < max; i++ ){
				*p |= ( 1 << bit );
				bit++;
				if( bit == 8 ){
					p++; bit = 0;
				}
			}
			break;
		}
	  default:
		ERROR( "var type error %X\n", type );
		break;
	}
	return;
ERR:
	ERROR( "%sの変数データが領域(%dbyte)をオーバーしました\n", variable_table_name, map->size );
}

static void alloc_var_buf( VAR_ALLOC_TABLE *alloc_table )
{
	int i;
	VAR *var;

	var = alloc_table->table;

	for( i = 0; i < alloc_table->table_p; i++, var++ ){
		if( ! IS_LINKVAR( var ) ){
			if( var->code == -1 ){
				/* 未割り付け */
				int code;
				if( ( code = search_var_map( &normal_map, var->type, var->max ) ) < 0 ){
					ERROR( "割り付けできませんでした:%s\n", var->name );
				}
#if 0
				/* 通常変数はリンク変数の後から割り付け */
				code += linkvar_end_pos;
#else
				/* リンク変数と通常変数は別扱い */
#endif
				var->code = code;
PRINTF( "ALLOC %s : %X\n", var->name, var->code );
			}
		}
	}
}

static void allocate_variables( void )
{
	alloc_var_buf( &normal_var );
}

static int get_mapped_size( VAR_MAP_BUF *map )
{
	char *p;
	int size;

	size = map->size - 1;
	p = map->buf + size;

	for( ; size > 0; size --, p-- ){
		if( *p != 0 ) break;
	}
	return size;
}

/* ---------------------------------------------------------------- */
/*
	変数情報テーブル(variable.sym)の操作
*/

/* 変数情報テーブルからの読み込み */

int load_variable_table( char *filename )
{
	FILE *fp;
	char linebuf[ 256 ];

PRINTF( "LOAD VAR TABLE %s\n", filename );
	if( ( fp = fopen( filename, "rt" ) ) == NULL ){
		return FALSE;
	}
	/* バージョンチェック */
	fgets( linebuf, 256, fp );
	if( strncmp( linebuf, "version", 4 ) == 0 ){
		char buf[ 256 ];
		sscanf( linebuf, "version %s", buf );
		if( strcmp( buf, varsym_version ) != 0 ){
			FATAL( "gclk がバージョンアップしました。\n%sを消して作り直してください\n", variable_table_name );
		}
	} else {
		FATAL( "version フィールドがありません\n%sを消して作り直してください\n", variable_table_name );
	}
	/* 作成日付チェック */
	fgets( linebuf, 256, fp );
	if( strncmp( linebuf, "date", 4 ) == 0 ){
		sscanf( linebuf, "date %08X", &variable_table_date );
	} else {
		FATAL( "date フィールドがありません\n" );
	}
	/* リンク変数のサイズチェック */
	fgets( linebuf, 256, fp );
	if( strncmp( linebuf, "link", 4 ) == 0 ){
		int max;
		sscanf( linebuf, "link %08X", &max );
		if( max != linkvar_max ){
			FATAL( "リンク変数のサイズが違っています\n%sを消して作り直してください\n", variable_table_name );
		}
	} else {
		FATAL( "link フィールドがありません\n" );
	}

	/* 読み取り開始 */
	while( fgets( linebuf, 256, fp ) != NULL ){
		int code, type, block, flag, max;
		char name[ 128 ], typechar;
		VAR *var;

		sscanf( linebuf, "%08X %c %04X %d %d %d\t%s\n", &code, &typechar
			   , &type, &block, &flag, &max, name );

		if( linkvar_load_flag != FALSE && typechar == 'l' ){
			continue;
		}

		var = new_var( &normal_var, name, type, 0, max );
		var->block = block;
		if( var->code == -1 ){
			var->code = code;
			if( VAR_TYPE( var ) != TYPE_LINKVAR ){
				set_alloc_map( &normal_map, var->type, var->code, var->max );
			}
		} else {
			ERROR( "Duplicate symbols %s in %s\n", name, filename );
		}
#if 0
		if( VAR_TYPE( var ) != current_type ){
			/* このモジュールで使用しているもの以外の
			   参照情報を読み込む */
			var->flag = flag;
		}
#else
		if( block_output_mode == 0 ){
			var->flag = block;
		}
#endif
	}
PRINTF( "LOAD VAR TABLE END\n" );

	fclose( fp );
	return TRUE;
}

/* 変数情報テーブルへの書き出し */

static char typestr( int type )
{
	switch( type & 0x0F ){
	  case GCL_STRID:
		return 's';
	  case GCL_INT:
		return 'i';
	  case GCL_SHORT:
		return 'w';
	  case GCL_BYTE:
		return 'b';
	  case GCL_BOOL:
		return 'f';
	  default:
		return '?';
	}
}

void output_variable_table( char *filename, int dump_flag )
{
	FILE *fp;
	int i;
	int n_unuse = 0;
	int n_unrefs = 0;
	int n_undefs = 0;
	VAR_ALLOC_TABLE *alloc_table = &normal_var;
	VAR *var;
	VAR *vartop;
	int varnum;

	if( ( fp = fopen( filename, "wt" ) ) == NULL ){
		ERROR( "file %s がwriteオープンできません\n", filename );
	}

	vartop = alloc_table->table;
	varnum = alloc_table->table_p;

	if( dump_flag ){
		/* 定義,参照チェック */

		var = vartop;
		for( i = varnum; i > 0; i--, var++ ){
			if( VAR_TYPE( var ) == current_type ){
				if( var->flag == VAR_FLAG_NONE ){
					DUMP( "Warning:変数 '$%c:%s' は使用されていません。\n"
						  , typestr( var->type & 0xFF )
						  , var->name );
					n_unuse ++;
				}
			}
		}
		var = vartop;
		for( i = varnum; i > 0; i--, var++ ){
			if( VAR_TYPE( var ) == current_type ){
				if( var->flag == VAR_FLAG_SET ){
					DUMP( "Warning:変数 '$%c:%s' は代入されていますが、参照されていません。\n"
						  , typestr( var->type & 0xFF )
						  , var->name );
					n_unrefs ++;
				}
			}
		}
		var = vartop;
		for( i = varnum; i > 0; i--, var++ ){
			if( VAR_TYPE( var ) == current_type ){
				if( var->flag == VAR_FLAG_REF ){
					DUMP( "WARNING:変数 '$%c:%s' は参照されていますが、代入されていません。\n"
						  , typestr( var->type & 0xFF )
						  , var->name );
					n_undefs ++;
				}
			}
		}
	} else {
		/* 定義,参照チェック */

		var = vartop;
		for( i = varnum; i > 0; i--, var++ ){
			if( VAR_TYPE( var ) == current_type ){
				if( var->flag == VAR_FLAG_NONE ){
					n_unuse ++;
				} else if( var->flag == VAR_FLAG_SET ){
					n_unrefs ++;
				} else if( var->flag == VAR_FLAG_REF ){
					n_undefs ++;
				}
			}
		}
	}
	if( ( n_unuse + n_unrefs + n_undefs ) > 0 ){
		DUMP( "未使用変数 %d\n", n_unuse );
		DUMP( "未参照変数 %d\n", n_unrefs );
		DUMP( "未定義で参照されている変数 %d\n", n_undefs );
	}

	/* 出力 */
	if( variable_table_date < 0 ){
		time_t t;
		variable_table_date = time( &t );
	}
	fprintf( fp, "version %s\n", varsym_version );
	fprintf( fp, "date %08X\n", variable_table_date );
	fprintf( fp, "link %08X\n", linkvar_max );

	var = vartop;
	if( block_output_mode == 0 ){
		// normal mode
		for( i = varnum; i > 0; i--, var++ ){
			fprintf( fp, "%08X %c %04X %d %d %3d\t%s\n"
					, var->code, VAR_TYPE_STR( var )
					, var->type, var->block, var->flag, var->max
					, var->name );
		}
	} else {
		// block mode
		for( i = varnum; i > 0; i--, var++ ){
			fprintf( fp, "%08X %c %04X %d %d %3d\t%s\n"
					, var->code, VAR_TYPE_STR( var )
					, var->type, var->flag, var->flag, var->max
					, var->name );
		}
	}
	fclose( fp );
}

/* ---------------------------------------------------------------- */
/*
	GCOファイルから変数情報を読み込む。
*/

/* GCOファイル内のBODYとPROCの先頭 */

static char *data_body[ 2 ];

/* GCOファイルの構造を読み込む */

#define IS_GCO_TAG( _ptr, _tag )	( strncmp( _ptr, _tag, 4 ) == 0 )

typedef struct {
	char tag[ 4 ];
	char size[ 4 ];
} GCO_CHANK;

static char *set_gco_data_body_top( char *data_top )
{
	char *top = data_top;
	char *res = NULL;
	int dataflag;

	/* 読み込みを行う */

	dataflag = 0;
	while( dataflag != 7 ){
		GCO_CHANK *cp;
		int size;
		cp = ( GCO_CHANK * )top;
		if( IS_GCO_TAG( cp->tag, GCO_PROC ) ){
			/* proc */
			data_body[ 1 ] = ( char * )( cp + 1 );
			dataflag |= ( 1 << 1 );
		} else if( IS_GCO_TAG( cp->tag, GCO_BODY ) ){
			/* body */
			data_body[ 0 ] = ( char * )( cp + 1 );
			dataflag |= ( 1 << 0 );
		} else if( IS_GCO_TAG( cp->tag, GCO_VARS ) ){
			/* 変数情報 */
			dataflag |= ( 1 << 2 );
			res = ( char * )( &cp->size );
		} else if( IS_GCO_TAG( cp->tag, GCO_END ) ){
			/* 終端 */
			FATAL( "gcoに変数情報がありません\n" );
		} else {
			// SKIP;
		}
		size = get_int( cp->size );
		top = ( char * )( cp + 1 ) + size;
	}
	return res;
}

static void write_gcx_file( FILE *fp, char *gco_data_top )
{
	/* GCX構造(block mode共通)の出力関数 */
	/*
	   アライメント構造も考慮に入れ、
	   proc_table
	   string_data
	   proc_body
	   data_body
	   で出力。
	*/
	char *top = gco_data_top;

	GCO_CHANK *proc = NULL;
	GCO_CHANK *prtable = NULL;
	GCO_CHANK *body = NULL;
	GCO_CHANK *rsrc = NULL;
	GCO_CHANK *strg = NULL;
	GCO_CHANK *font = NULL;

	int chanks = 0;

	/* 書き換え後のデータチャンクを再取得 */

	for( ;; ){
		GCO_CHANK *cp;
		int size;
		cp = ( GCO_CHANK * )top;
		if( IS_GCO_TAG( cp->tag, GCO_PROC ) ){
			proc = cp;
			chanks |= ( 1 << 0 );
		} else if( IS_GCO_TAG( cp->tag, GCO_PROCTABLE ) ){
			prtable = cp;
			chanks |= ( 1 << 1 );
		} else if( IS_GCO_TAG( cp->tag, GCO_BODY ) ){
			body = cp;
			chanks |= ( 1 << 2 );
		} else if( IS_GCO_TAG( cp->tag, GCO_RESOURCETBL ) ){
			rsrc = cp;
			chanks |= ( 1 << 3 );
		} else if( IS_GCO_TAG( cp->tag, GCO_STRINGTBL ) ){
			strg = cp;
			chanks |= ( 1 << 4 );
		} else if( IS_GCO_TAG( cp->tag, GCO_FONT ) ){
			font = cp;
			chanks |= ( 1 << 5 );
		} else if( IS_GCO_TAG( cp->tag, GCO_END ) ){
			/* 終端 */
			break;
		} else {
			// SKIP;
		}
		size = get_int( cp->size );
		top = ( char * )( cp + 1 ) + size;
	}

	if( chanks != 0x3F ){
		FATAL( "BLOKEN GCO FILE\n" );
	}

	/* 出力開始 */

	fwrite( &variable_table_date, sizeof( int ), 1, fp );

	/* PROCテーブル */
	fwrite( prtable + 1, sizeof( char ), get_int( prtable->size ), fp );
	/* 文字列テーブル */
	/* 先頭にヘッダ出力 */
	{
		struct {
			int blocksize;
			int resourcep;
			int stringp;
			int fontp;
		} header;
		int r_size, s_size, f_size;
		r_size = get_int( rsrc->size );
		s_size = get_int( strg->size );
		f_size = get_int( font->size );

		r_size = ( r_size + 3 ) / 4 * 4;	// 4 byte align
		s_size = ( s_size + 3 ) / 4 * 4;	// 4 byte align

		header.blocksize = sizeof( header ) + r_size + s_size + f_size;
		header.resourcep = sizeof( header );
		header.stringp = header.resourcep + r_size;
		header.fontp = header.stringp + s_size;

		fwrite( &header, sizeof( char ), sizeof( header ), fp );
		fwrite( rsrc + 1, sizeof( char ), r_size, fp );
		fwrite( strg + 1, sizeof( char ), s_size, fp );
		fwrite( font + 1, sizeof( char ), f_size, fp );
	}
	/* PROC本体 : 頭にサイズ情報を付加 */
   //printf("BP: PROC TOP: %d\n", ftell( fp ) );
	fwrite( proc->size, sizeof( char ), sizeof( int ), fp );
	fwrite( proc + 1, sizeof( char ), get_int( proc->size ), fp );

	/* BODY本体 : 頭にサイズ情報を付加 */
   //printf("BP: BODY TOP: %d\n", ftell( fp ) );
	fwrite( body->size, sizeof( char ), sizeof( int ), fp );
	fwrite( body + 1, sizeof( char ), get_int( body->size ), fp );
   //printf("BP: BODY END: %d\n", ftell( fp ) );
}

/*
   変数操作関数
*/

static void dump_variables( char *top )
{
	VARIABLE *vp, vr;
	char *end;
PRINTF( "dump variable %X\n", top );
	vp = ( VARIABLE * )( top + sizeof( int ) );
	end = top + sizeof( int ) + get_int( top );
	while( ( void * )vp < ( void * )end ){
		memcpy( &vr, vp, sizeof( VARIABLE ) );
		PRINTF( "%s which %d flag %d offset %d namelen %d type %X ", ( char * )( vp + 1 )
			   , vr.which, vr.flag, vr.out_offset, vr.name_len, vr.type );
		if( vr.which < 0 ){
			PRINTF( "SKIP VAR %s\n", ( char * )( vp + 1 ) );
		} else {
			unsigned char *p;
			p = data_body[ vr.which ] + vr.out_offset;
			PRINTF( "[%02X %02X  %02X  %02X %02X]\n", p[-2], p[-1], p[0], p[1], p[2] );
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vr.name_len );
	}
}

static void read_variable( char *buffer )
{
	char *top;
	extern int verbose_mode;

	top = set_gco_data_body_top( buffer );

	if( verbose_mode ){
		dump_variables( top );
PRINTF( "DUMP END\n" );
	}
	set_global_variables( top );
PRINTF( "SET END\n" );

	/* 登録処理 */
	allocate_variables();
PRINTF( "ALLOC END\n" );
}

/* 通常GCOファイルからの読み込み */
void read_variable_from_file( char *filename )
{
	FILE *fp;
	int size, version;
	void *buffer;

	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		ERROR( "ファイル %sがオープンできません\n", filename );
	}
	PRINTF( "reading veriable table in %s..\n", filename );
	current_file = filename;

	/* 全データメモリ読み込み */
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	if( ( buffer = malloc( size ) ) == NULL ){
		ERROR( "メモリが足りません。\n" );
	}

	/* GCO version check */
	fread( &version, sizeof( int ), 1, fp );
	if( version != gco_version ){
		ERROR( "GCO フォーマットが変更されました。make clean して下さい\n" );
	}

	fread( buffer, sizeof( char ), size, fp );

	fclose( fp );

	read_variable( buffer );

	free( buffer );
}

/* ブロックモードのデータからの読み込み */
void read_variable_block_mode( char *filename )
{
	FILE *fp;
	int size, version;
	void *buffer;
PRINTF( "BLOCK MODE !!\n" );
	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		ERROR( "ファイル %sがオープンできません\n", filename );
	}
	PRINTF( "reading veriable table in %s..\n", filename );
	current_file = filename;

	while( !feof( fp ) ){
		// 先頭にサイズのデータ
		int len;
		fread( &size, sizeof( int ), 1, fp );

		if( feof( fp ) ) break;

		len = fgetc( fp );
		fseek( fp, len, SEEK_CUR );

		if( ( buffer = malloc( size ) ) == NULL ){
			ERROR( "メモリが足りません。\n" );
		}
		
		/* GCO version check */
		fread( &version, sizeof( int ), 1, fp );
		if( version != gco_version ){
			ERROR( "GCO フォーマットが変更されました。make clean して下さい\n" );
		}

		fread( buffer, sizeof( char ), size - ( sizeof( int ) * 2 + len + 1 ), fp );
		PRINTF( "BUFFER END = %X\n", (char*)buffer + size - ( sizeof( int ) * 2 + len + 1 ) );
		read_variable( buffer );

		free( buffer );
	}
	current_file = NULL;

	fclose( fp );
}

/* ---------------------------------------------------------------- */
/*
	出力系
*/

/* ---- データ本体の出力 ---- */

/* 変数情報をテーブルから検索してファイル中に埋め込む */

static void restore_variable_offset( char *top )
{
	VARIABLE *vp, vr;
	char *end;

	vp = ( VARIABLE * )( top + sizeof( int ) );
	end = top + sizeof( int ) + get_int( top );

	reset_var_alloc_table( &local_var );
	reset_var_map_buf( &local_map );

	while( ( void * )vp < ( void * )end ){
		VAR *var;

		memcpy( &vr, vp, sizeof( VARIABLE ) );

		if( vr.which >= 0 ){
			/* which == -1 の場合はスキップ */

			if( vr.type & LOCAL_VARIABLE_FLAG ){
				// ローカルの場合
				var = new_var( &local_var, ( char * )( vp + 1 ), vr.type
							  , VAR_FLAG( vr.flag ), vr.array_max );
				if( var->code == -1 ){
					if( ( var->code = search_var_map( &local_map
													 , var->type, var->max ) ) < 0 ){
						ERROR( "割り付けできませんでした %s:%s\n", current_file, var->name );
					}
					var->code |= VAR_TYPE_LOCAL;
				}
			} else {
				// グローバルの場合
				if( ( var = search_var( &normal_var, ( char * )( vp + 1 ) ) ) == NULL ){
					ERROR( "???? %s\n", ( char * )( vp + 1 ) );
				}
			}
			{
				unsigned char *p;
				p = data_body[ vr.which ] + vr.out_offset;
				/* p[ 0 ]が変数タイプ、p[ 1 .. 3 ]がオフセット情報 */
				p[ 1 ] = ( var->code >> 16 ) & 0xff;
				p[ 2 ] = ( var->code >> 8 ) & 0xff;
				p[ 3 ] = ( var->code >> 0 ) & 0xff;
				PRINTF( "CHANGE %s %02X %06X [%d:%06X]\n", var->name
						, var->type, var->code, vr.which, vr.out_offset );
			}
		}
		vp = ( VARIABLE * )( ( char * )( vp + 1 ) + vr.name_len );
	}
}

/* 通常GCOからGCXへの変換 */
void write_binarys( char *infile, char *outfile )
{
	FILE *fp;
	int size;
	int version;
	void *buffer;
	char *top;

	if( ( fp = fopen( infile, "rb" ) ) == NULL ){
		ERROR( "ファイル %sがオープンできません\n",	infile );
	}
	current_file = infile;
	PRINTF( "reading %s..\n", infile );
	/* ファイルサイズの取得 */
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	if( ( buffer = malloc( size ) ) == NULL ){
		ERROR( "メモリが足りません。\n" );
	}

	/* バージョンチェック */
	fread( &version, sizeof( int ), 1, fp );
	if( version != gco_version ){
		ERROR( "GCO フォーマットが変更されました。make clean して下さい\n" );
	}

	fread( buffer, sizeof( char ), size, fp );
	fclose( fp );

	top = set_gco_data_body_top( buffer );

	restore_variable_offset( top );

	{
		// ローカル変数使用量のチェック
		int max;

		max = get_mapped_size( &local_map );
		if( max_local_var_buf < max ){
			max_local_var_buf = max;
			strcpy( max_local_var_filename, infile );
		}
	}

#if 1 //BP
   {
      char outputFolder[FILENAME_MAX];
      char * lastSlash;
      strcpy( outputFolder, outfile );
      if( lastSlash = strrchr( outputFolder, '/' ) )
      {
         *lastSlash = 0;
         _mkdir( outputFolder );
      }
   }
#endif

	/* ファイルへの出力 */
	if( ( fp = fopen( outfile, "wb" ) ) == NULL ){
		WARNING( "ファイル %sがオープンできません\n", outfile );
		free( buffer );
		return;
	}
	PRINTF( "writing %s ...\n", outfile );

	write_gcx_file( fp, buffer );

	current_file = NULL;
	fclose( fp );

	free( buffer );
}

/* ブロックモードの変換 */
int write_binarys_block_mode( char *infile, FILE *out, FILE *tp )
{
	FILE *fp;
	int num = 0;

	if( ( fp = fopen( infile, "rb" ) ) == NULL ){
		ERROR( "ファイル %sがオープンできません\n",	infile );
	}
	PRINTF( "reading %s..\n", infile );

	current_file = infile;

	while( ! feof( fp ) ){
		void *buffer;
		char *top;
		int size;
		int version;
		int len;
		int pos, startpos;
		static char block_name[ 128 ];

		fread( &size, sizeof( int ), 1, fp );
		if( feof( fp ) ) break;
		len = fgetc( fp );
		fread( block_name, sizeof( char ), len, fp );

		if( ( buffer = malloc( size ) ) == NULL ){
			FATAL( "メモリが足りません。\n" );
		}
		/* バージョンチェック */
		fread( &version, sizeof( int ), 1, fp );
		if( version != gco_version ){
			FATAL( "GCO フォーマットが変更されました。make clean して下さい\n" );
		}
		fread( buffer, sizeof( char ), size - ( sizeof( int ) * 2 + len + 1 ), fp );
		top = set_gco_data_body_top( buffer );

		restore_variable_offset( top );

		pos = ftell( out );
		PRINTF( "writing %s %08x ...\n", block_name, pos );
		startpos = pos;

PRINTF( "WRITING %X-%X\n", buffer, top );

		write_gcx_file( out, buffer );

		pos = ftell( out );
		// 16バイトアライメント
		if( pos % 16 > 0 ){
			static char buf[ 16 ] = { 0 };
			fwrite( buf, sizeof( char ), 16 - pos % 16, out );
		}
		free( buffer );
		{
			unsigned int endpos, sec;
			endpos = ftell( out );

			if( block_output_mode == 2 ){
				// LARGE MODE
				int sec_size = SECTOR_SIZE * 4;
				sec = ( endpos - startpos + sec_size - 1 ) / sec_size;
			} else {
				sec = ( endpos - startpos + SECTOR_SIZE - 1 ) / SECTOR_SIZE;
			}
			

			if( sec > 0x7F ){
				FATAL( "ブロックが大きすぎます %s %X\n", block_name, sec );
			}
			if( ( startpos >> 4 ) > 0x00FFFFFF ){
				FATAL( "TOO LARGE SIZE !!\n" );
			}
			fprintf( tp, "0x%08X\t%s\n", ( startpos >> 4 ) | ( sec << 24 )
					 , block_name );
		}
		num ++;
	}
	current_file = NULL;
	fclose( fp );
	return num;
}

void dump_var_report( void )
{
	/* 変数使用領域サイズ出力 */
	int size;

	DUMP( "Link   variable size %d bytes\n", linkvar_max );
	size = get_mapped_size( &normal_map );
	DUMP( "Normal variable size %d/%d bytes\n", size + 1, normal_map.size );
	size = max_local_var_buf;
	DUMP( "Local  variable size %d/%d bytes", size + 1, local_map.size );
	if( max_local_var_filename[ 0 ] != '\0' ){
		DUMP( " (%s)", max_local_var_filename );
	}
	DUMP( "\n" );
}
