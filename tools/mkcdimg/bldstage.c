/*
	blsdstage.c
		cdrom.img/stage/ をまとめて、ステージファイルを作成する。
	$Id: bldstage.c,v 1.7 2002/04/09 06:17:07 usr01475 Exp $
*/

/*
	データは基本的には圧縮されて記録される。
	PS2のHDDにインストールする時は,プログラムのはいったBINARYセクション以外は
	展開して保存される。そのため、展開後のサイズの目安になる情報を計算しておく。
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <zlib.h>

#include "print.h"
#include "mymalloc.h"

#define STAGEFILE_VERSION	1

#define TRUE 1
#define FALSE 0

int test_mode = 0;
int compress_mode = 0;

static int install_total_size;	// INSTALLした後のトータルサイズ

char *current_stage = NULL;		// 状況出力用
static int current_stage_id;		// 現在の出力ステージID

#define DATA_SECTION_END	0x7F000000
#define DATA_NOCACHE		0x7F000001
#define DATA_CACHE			0x7F000002
#define DATA_RESIDENT		0x7F000003
#define DATA_BLOCK( no )	( 0x7F000100 | (no) )
#define DATA_BINARY			0x7F010000

#define DATA_SOUND			0x7F000010

#define DATA_COMPRESS_SECTION	0x7E000000

#define FLAG_ALIGN_NORMAL	0	// 16 byte align
#define FLAG_ALIGN_SECTOR	1
#define FLAG_ALIGN_128		2

static int get_align( int flag )
{
	static int aligns[] = { 16, 2048, 128 };
	return aligns[ flag ];
}

/*
 方針:

   全ステージファイルの開始オフセットと終了オフセットをまとめて読み込んでおく。
   読み取りはスタートしたらCDをいっさい止めず、コールバック中で読み込みポインタを
   操作することで全データを一括して読み込む。
   
   nocache, resident は同様に扱い、data.cnf中では1ファイルと見なし、
   ファイル中にID、サイズを含める。

   cacheは、data.cnfの中にオフセット情報を持ち、読み込んだ後にポインタ情報とする。
   読み込みは一括。1ファイルごとは16byte alignment.

   *.binやサウンド用のデータは、システムファイルとして別扱いで扱う。
   1ファイルごとにSECTOR alignment.
*/

/* ---------------------------------------------------------------------- */
/*
	下位関数
*/

/* -------------------- 汎用 ---------------------- */

#define Malloc	my_malloc
#define Free	my_free

static int get_file_size( char *filename )
{
	struct stat st;

	stat( filename, &st );
	return st.st_size;
}

#define BIT_LEN		24

int get_strcode( char *str )
{
	unsigned char c;
	unsigned char *p;
	unsigned int id, mask;

	p = ( unsigned char * )str;
	id = 0;
	mask = ( 1 << BIT_LEN ) - 1;

	if( p[ 0 ] == 0xFF ){
		int code;
		memcpy( &code, &p[ 1 ], sizeof( int ) );
		return code;
	}

	while( ( c = *( p++ ) ) != '\0' ){
		id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
		id += c;
		id &= mask;
	}
	if( id == 0 ) id = 1;

	return ( int )id;
}

/* 暗号化ユーティリティ */

static int encode_flag = 0;

static unsigned int header_seed;
static unsigned int seed;
static unsigned int _seed;

static void set_encode( void )
{
	int s;

	encode_flag = 1;
	s = ( rand() >> 15 ) & 0xFFFF;
	seed = ( s ) | ( ( s ^ 0x6576 ) << 16 );
	_seed = s * 278;
//printf( "encode seed %X %X %X\n", s, seed, _seed );
}

//static void unset_encode( void )
//{
//printf( "unset endcode\n" );
//	encode_flag = 0;
//}

static void encode_buffer( void *buffer, int size )
{
	unsigned int *p = buffer;

	if( compress_mode == 0 ){
		// 圧縮データでない場合は暗号化しない
		return;
	}

	for( ; size > 0; size -= sizeof( int ) ){
		*p = *p ^ seed;
		seed = seed * 48828125 + _seed;
		p++;
	}
}

/* セクタ単位で処理を行なう */

#define SECTOR_SIZE		( 2048 )
#define SECTOR( a )		( ( (a) + SECTOR_SIZE - 1 ) / SECTOR_SIZE )

static void trim_sector_alignment( FILE *fp )
{
	/* sector alignment にあわせる */

	int pos;

	pos = ftell( fp );

	if( pos % SECTOR_SIZE > 0 ){
		int i;
		for( i = SECTOR_SIZE - ( pos % SECTOR_SIZE ); i > 0; i-- ){
			fputc( rand() >> 16, fp );
		}
	}
}

static void trim_align( FILE *fp, int align )
{
	/* 任意のアライメントにあわせる */

	int pos;

	pos = ftell( fp );

	if( pos % align > 0 ){
		int i;
		for( i = align - ( pos % align ); i > 0; i-- ){
			fputc( rand() >> 16, fp );
		}
	}
}

static int check_ext( char *filename, char *ext )
{
	char *p;
	if( ( p = strrchr( filename, '.' ) ) == NULL ){
		return FALSE;
	}
	if( strcmp( p + 1, ext ) != 0 ){
		return FALSE;
	}
	return TRUE;
}

#define NORM_INT( a )		( ( ( (a) + sizeof( int ) - 1 ) / sizeof( int ) ) * sizeof( int ) )
#define NORM_SECTOR( a )	( ( ( (a) + SECTOR_SIZE - 1 ) / SECTOR_SIZE ) * SECTOR_SIZE )
#define NORM_ALIGN( a, align )	( ( ( (a) + (align) - 1 ) / (align) ) * (align) )

/* ---------------- FILE TYPE --------------- */

static int get_nocache_file_type( char *filename )
{
	if( check_ext( filename, "bin" ) ){
		return DATA_BINARY;		/* System_mode */
	}

	return DATA_NOCACHE;		/* nocache_mode */
}

/* ----------------------------------------- */

/* local malloc */

typedef struct {
	void *next;
	int body[0];
} M_TAG;

static void init_local_memory( M_TAG *tag )
{
	tag->next = NULL;
}

static void *malloc_local( M_TAG *tag, int size )
{
	M_TAG *p;

	p = Malloc( size + sizeof( M_TAG ) );
	p->next = tag->next;
	tag->next = p;

	return ( void * )p->body;
}

static void free_local_all( M_TAG *tag )
{
	M_TAG *p, *next;

	for( p = tag->next; p != NULL; p = next ){
		next = p->next;
		free( p );
	}
	tag->next = NULL;
}

/* ---------------------------------------------------------------------- */
/*
	ステージデータ全体のテーブル作成管理
*/

typedef struct {
	unsigned int seed;
	short version;
	short sector;
	short stagenum;
	short padding;
	int install_size;
} STAGE_TABLE_HEADER;

typedef struct {
	char name[ 8 ];
	int offset;
} STAGE_TABLE;

static STAGE_TABLE *stage_table;
static int stage_max_count;
static int stage_table_p = 0;
static int header_sector;

static void init_stage_table( FILE *fp, int count )
{
	STAGE_TABLE_HEADER header;
	int size;

	stage_max_count = count;
	size = sizeof( STAGE_TABLE ) * ( count + 1 );
	stage_table = Malloc( size );

	seed = rand(); _seed = rand();
	encode_buffer( stage_table, size );

	fwrite( &header, sizeof( STAGE_TABLE_HEADER ), 1, fp );
	fwrite( stage_table, sizeof( char ), size, fp );
	trim_sector_alignment( fp );

	stage_table_p = 0;
	memset( stage_table, 0, size );

	header_sector = SECTOR( size + sizeof( STAGE_TABLE_HEADER ) );
	header_seed = time( NULL );

	install_total_size += header_sector;
}

static void set_stage_file_top( char *stagename, FILE *out )
{
	STAGE_TABLE *tp;
	int pos;

	if( strlen( stagename ) > 7 ){
		printf( "\n warning Too int stage name %s\n", stagename );
		exit( 1 );
	}
	tp = stage_table + stage_table_p;
	strcpy( tp->name, stagename );
	pos = ftell( out );
	if( pos % SECTOR_SIZE > 0 ){
		printf( "\nError !! Stage is not on SECTOR Alignment\n" );
		exit( 1 );
	}
	tp->offset = pos / SECTOR_SIZE;

	stage_table_p++;
	if( stage_table_p > stage_max_count ){
		printf( "\nError !! stage count over!!\n" );
		exit( 1 );
	}
}

static void save_stage_table( FILE *fp )
{
	int size;
	STAGE_TABLE_HEADER header;

	printf( "%d stage is packed.\n", stage_table_p );
	if( stage_table_p < stage_max_count ){
		printf( "%d skip stages.\n", stage_max_count - stage_table_p );
	} else if( stage_table_p > stage_max_count ){
		printf( "Wrong Stage Count !!\n" );
		exit( 1 );
	}

	size = sizeof( STAGE_TABLE ) * stage_table_p;

	header.version = STAGEFILE_VERSION;
	header.stagenum = stage_table_p;
	header.seed = header_seed;
	header.sector = header_sector;
	header.install_size = install_total_size;
	
	fseek( fp, 0, SEEK_SET );

	seed = header.seed;
	_seed = header.seed ^ 0xF0F0;

	encode_buffer( ( void * )&header + 4, sizeof( header ) - 4 );
	fwrite( &header, sizeof( header ), 1, fp );
	encode_buffer( stage_table, sizeof( STAGE_TABLE ) * stage_table_p );
	fwrite( stage_table, sizeof( STAGE_TABLE ), stage_table_p, fp );
}

/* --------------------------- 1ステージの作成処理 ------------------------ */

/* ファイル１つあたりの構造体 */

typedef struct _datafile{
	struct _datafile *next;
	char name[ 32 ];		// name[0]が0xFFだった場合、name[1...4]がstrcode
	char dirname[ 256 ];
	int mode;
	int size;
	int flag;
	void *body;
} DATAFILE;

static DATAFILE *datafile_top;
static DATAFILE *datafile_bottom;

/* ---------------------------------------------------------------------- */
/*
	ステージデータ読み込み部
*/

/* ----------- DATA.CNF を parse する */

static char *parse_ptr;

static void set_datacnf_top( char *top )
{
	parse_ptr = top;
}

static int get_datacnf_line( char *linebuf )
{
	char *p;
	char c;

	p = linebuf;
	do{
		if( ( c = *( parse_ptr++ ) ) == '\0' ){
			return FALSE;
		}
	} while( isspace( c ) );
	do{
		*( p++ ) = c;
		if( ( c = *( parse_ptr++ ) ) == '\0' ){
			break;
		}
	} while( ! isspace( c ) );
	*p = '\0';
	if( c == '\0' ){
		parse_ptr --;
	}
	return TRUE;
}

/*
	DATAFILE 構造体をリストにリンクする。
*/

static void add_datafile_list( DATAFILE *df )
{
	df->next = NULL;
	if( datafile_bottom == NULL ){
		datafile_bottom = datafile_top = df;
	} else {
		datafile_bottom->next = df;
		datafile_bottom = df;
	}
}

/*
	ファイルをバッファを確保して読み込む
*/

static void *load_file( char *dirname, char *filename, int *size_p
						, int *align_flag_p, M_TAG *m_tag )
{
	FILE *fp;
	int size;
	int align = FLAG_ALIGN_NORMAL;
	char *buffer;
	char fullpath[ 256 ];

	if( filename[ 0 ] == '*' ){
		/* data.cnfに先頭が*で記述されるファイルがある */
		filename ++;
	}
	if( filename[ 0 ] == '@' ){
		filename ++;
		align = FLAG_ALIGN_128;
	}
	if( align_flag_p != NULL ){
		*align_flag_p = align;
	}
	sprintf( fullpath, "%s/%s", dirname, filename );

	size = get_file_size( fullpath );
	if( ( fp = fopen( fullpath, "rb" ) ) == NULL ){
		printf( "FILE %s Open Error\n", fullpath );
		return NULL;
	}

	buffer = malloc_local( m_tag, size + 1 );

	if( ! test_mode || strcmp( filename, "data.cnf" ) == 0 ){
		fread( buffer, sizeof( char ), size, fp );
		buffer[ size ] = '\0';
	}

	fclose( fp );

	if( size_p != NULL ){
		*size_p = size;
	}

	return buffer;
}

/*
	1 ファイルを読み込んで、登録する。
*/

static int load_data_file( char *dirname, char *filename, int mode, M_TAG *tag )
{
	DATAFILE *df;
	void *ptr;
	int size;
	int flag = FLAG_ALIGN_NORMAL;

	if( ( ptr = load_file( dirname, filename, &size, &flag, tag ) ) == NULL ){
		if( test_mode ) return 0;
		return -1;
//		printf( "\nError:not open data file\n" );
//		exit( 1 );
	}
	if( mode == DATA_BINARY || mode == DATA_SOUND ) flag = FLAG_ALIGN_SECTOR;

	df = malloc_local( tag, sizeof( DATAFILE ) );
	strcpy( df->dirname, dirname );
	strcpy( df->name, filename );
	df->body = ptr;
	df->size = size;
	df->mode = mode;
	df->flag = flag;
PRINTF( "loaded file %s size %d\n", filename, df->size );
	add_datafile_list( df );

	return 0;
}

/*
	サウンドデータ1 ファイルを読み込んで、登録する。
*/

static void load_sound_file( char *dirname, char *filename, M_TAG *tag )
{
	DATAFILE *df;
	void *ptr;
	int size;
	int flag;

	if( ( ptr = load_file( dirname, filename, &size, &flag, tag ) ) == NULL ){
		if( test_mode ) return;
		printf( "\nError:not open data file\n" );
		exit( 1 );
	}
	flag = FLAG_ALIGN_SECTOR;

	df = malloc_local( tag, sizeof( DATAFILE ) );
	strcpy( df->dirname, dirname );
	df->name[ 0 ] = 0xFF;
	{
		int sound_idx;
		sscanf( filename, "pk%06x.sdx", &sound_idx );
		memcpy( &df->name[ 1 ], &sound_idx, sizeof( int ) );
	}

	df->body = ptr;
	df->size = size;
	df->mode = DATA_SOUND;
	df->flag = flag;
PRINTF( "loaded file %s size %d\n", filename, df->size );
	add_datafile_list( df );
}

/*
	dar ファイルを読み込んで解析し、１ファイルづつ登録する。
*/

static void load_dar_file( char *dirname, char *filename, int mode, M_TAG *tag )
{
	char *dartop;
	int darsize;
	int unit;
	int flag = FLAG_ALIGN_NORMAL;
	int now;

	if( ( dartop = load_file( dirname, filename, &darsize, &flag, tag ) ) == NULL ){
		if( test_mode ) return;
		printf( "\nError:not open dar file\n" );
		exit( 1 );
	}

	if( test_mode ){
		return;
	}

	unit = *( ( int * )dartop );
	now = sizeof( int );

	for( ; unit > 0; unit -- ){
		DATAFILE *df;

		df = malloc_local( tag, sizeof( DATAFILE ) );

		strcpy( df->dirname, dirname );
		strcpy( df->name, &dartop[ now ] );
		now += strlen( &dartop[ now ] ) + 1;
		now += ( 3 & ( 4 - ( 3 & ( unsigned int )now ) ) );
		df->size = *( int * )&dartop[ now ];
		now += sizeof( int );
		now += ( 15 & ( 16 - ( 15 & ( unsigned int )now ) ) );

		df->body = &dartop[ now ];
		df->mode = mode;
		df->flag = flag;
if( df->size <= 0 || df->size >= 1024 * 1024 * 12 ){
	printf( "\ndar size error in %s %s ( %d )\n", filename, df->name, df->size );
	exit( 1 );
}
		now += df->size + 1;
		add_datafile_list( df );
PRINTF( "dar %s size %d\n", df->name, df->size );
	}
PRINTF( "LOAD DAR END\n" );
}

/*
	qar ファイルを読み込んで解析し、１ファイルづつ登録する。
*/

static void load_qar_file( char *dirname, char *filename, int mode, M_TAG *tag )
{
	char *qartop;
	int qarsize;
	int flag = FLAG_ALIGN_NORMAL;
	typedef struct {
		int id;
		int size;
	} QAR_TABLE;

PRINTF( "LOAD QAR FILE %s\n", filename );
	if( ( qartop = load_file( dirname, filename, &qarsize, &flag, tag ) ) == NULL ){
		if( test_mode ) return;
		printf( "\nError:not open qar file\n" );
		exit( 1 );
	}

	if( test_mode ){
		return;
	}

	{
		/* 一番最後が情報テーブルへのオフセット */
		char *table;
		int i, num;
		QAR_TABLE *info;
		char *now;

		table = ( qartop + *( int * )( qartop + qarsize - sizeof( int ) ) );
		num = *( short * )( table + 0 );
		info = ( QAR_TABLE * )( table + 4 );
		now = qartop;

		for( i = 0; i < num; i++ ){
			DATAFILE *df;

			df = malloc_local( tag, sizeof( DATAFILE ) );
			strcpy( df->dirname, dirname );
			df->name[ 0 ] = 0xFF;
			memcpy( &df->name[ 1 ], &info->id, sizeof( int ) );
			df->body = now;
			df->size = info->size;
			df->mode = mode;
			df->flag = flag;
PRINTF( "ID %X size %d\n", info->id, info->size );
			now += info->size;
			add_datafile_list( df );
			info ++;
		}
	}
}

/*
	data.cnfを解析して、そこに書かれているファイルをすべて読み込む
*/

static int load_all_datacnf_data( char *dirname, char *datacnf, M_TAG *tag )
{
	char linebuf[ 128 ];
	int mode;

	set_datacnf_top( datacnf );
	datafile_top = datafile_bottom = NULL;
	mode = DATA_CACHE;				/* default is cache mode */

	while( get_datacnf_line( linebuf ) ){
		if( linebuf[ 0 ] == '.' ){
			/* モード切り替え */
			if( strcmp( linebuf, ".nocache" ) == 0 ){
				mode = DATA_NOCACHE;
			} else if( strcmp( linebuf, ".cache" ) == 0 ){
				mode = DATA_CACHE;
			} else if( strcmp( linebuf, ".resident" ) == 0 ){
				mode = DATA_RESIDENT;
			}
		} else {
			/* ファイル読み込み */
			if( check_ext( linebuf, "dar" ) ){
				load_dar_file( dirname, linebuf, mode, tag );
			} else if( check_ext( linebuf, "qar" ) ){
				load_qar_file( dirname, linebuf, mode, tag );
			} else {
				int m;
				if( mode == DATA_NOCACHE ){
					m = get_nocache_file_type( linebuf );
				} else {
					m = mode;
				}
				if( load_data_file( dirname, linebuf, m, tag ) < 0 ){
					return -1;
				}
			}
		}
	}
	return 0;
}

/*
	あるディレクトリに存在するサウンドpakファイルを検索し,
	すべて読み込む
*/

static int sdpak_select( const struct dirent *dp )
{
	/* 拡張子が.sdxのものすべて */
	char *p;

	if( ( p = strrchr( dp->d_name, '.' ) ) == NULL ){
		return 0;
	}
	if( strcmp( p, ".sdx" ) != 0 ){
		return 0;
	}
	return 1;
}

static int sdpak_sort_cmp( const struct dirent **dir1, const struct dirent **dir2 )
{
	return strcmp( ( *dir1 )->d_name, ( *dir2 )->d_name );
}

static void load_sound_data( char *dirname, M_TAG *tag )
{
	struct dirent **entry;
	int i, dir_n;

	if( ( dir_n = scandir( dirname, &entry
						   , ( void * )sdpak_select, ( void * )sdpak_sort_cmp ) ) <= 0 ){
		/* サウンドファイルがなかった */
		return;
	}

	for( i = 0; i < dir_n; i++ ){
		load_sound_file( dirname, entry[ i ]->d_name, tag );
	}
	free( entry );
}

/* ---------------------------------------------------------------------- */
/*
	書き込み部
*/

/* データヘッダ　*/

typedef struct {
	int tagnum;			// タグの総数
} DATACNF_HEADER;

typedef struct {
	int id;			// ( 'ext' << 24 ) | strid; ext=0x7fなら特殊
	int offset;
} DATACNF_TAG;

#define SI_RESIDENT_DATA	0x00000001
#define SI_NOCACHE_DATA		0x00000002
#define SI_SYSTEM_DATA		0x00000004
#define SI_CACHE_DATA		0x00000010

static int stage_file_total_size;

#define MAX_DATA_FILE_NUM 1024		/* data.cnf 中に登録される最大ファイル数 */

static DATACNF_TAG datacnf_work[ MAX_DATA_FILE_NUM ];

static int datacnf_p;

static DATACNF_TAG *set_datacnf( int id, int offset )
{
	DATACNF_TAG *dwork;

	dwork = datacnf_work + datacnf_p;

	dwork->id = id;
	dwork->offset = offset;
PRINTF( "set_datacnf %X %X\n", id, offset );
	datacnf_p ++;

	return dwork;
}

static void get_cache_id( char *filename, int *code, int *type )
{
	char buf[ 64 ];
	char *p;

	strcpy( buf, filename );
	if( ( p = strchr( buf, '.' ) ) == NULL ){
		*code = get_strcode( buf );
		*type = 0;
	} else {
		*p = '\0';
		*type = p[ 1 ] - 'a';
		*code = get_strcode( buf );
	}
}

static void set_file_datacnf( char *filename, int offset )
{
	int code;
	int type;

	if( ( unsigned char )filename[ 0 ] == 0xFF ){
		int id;
		/* 元QARのファイル */

		memcpy( &id, &filename[ 1 ], sizeof( int ) );
		set_datacnf( id, offset );
	} else {
		get_cache_id( filename, &code, &type );

		set_datacnf( code | ( type << 24 ), offset );
	}
}

/* ---------------------------------------------------------------------- */
/*
	ステージデータファイルを作成する。
*/

static void save_datacnf( FILE *out, M_TAG *tag )
{
	DATAFILE *p;
	DATACNF_HEADER head;
	DATACNF_TAG *top_tag;
	int mode;
	int pos, top_pos;

	stage_file_total_size = 0;

	datacnf_p = 0;
	p = datafile_top;

	mode = -1;

	top_pos = pos = 0;
	top_tag = NULL;
	for( ; p != NULL; p = p->next ){
		if( mode != p->mode ){
			if( top_tag != NULL ){
				// 直前のタグにセクションのサイズを入れる
				set_datacnf( DATA_SECTION_END, pos - top_pos );
				top_tag->offset = pos - top_pos;
PRINTF( "SECTION SIZE %d\n", top_tag->offset );
			}
			// セクション毎にセクタアライメント
			pos = NORM_ALIGN( pos, 2048 );
PRINTF( "NEW SECTION %X pos %X\n", p->mode, pos );
			top_tag = set_datacnf( p->mode, 0 );
			top_pos = pos;
			mode = p->mode;
		}
		// ファイルを登録
		set_file_datacnf( p->name, pos - top_pos );
		pos += p->size;
		switch( p->flag ){
		  case FLAG_ALIGN_NORMAL:
			pos = NORM_ALIGN( pos, 16 );
			break;
		  case FLAG_ALIGN_SECTOR:
			pos = NORM_ALIGN( pos, 2048 );
			break;
		  case FLAG_ALIGN_128:
			pos = NORM_ALIGN( pos, 128 );
			break;
		}
	}
	if( top_tag != NULL ){
		set_datacnf( DATA_SECTION_END, pos - top_pos );
		top_tag->offset = pos - top_pos;
PRINTF( "SECTION SIZE %d\n", top_tag->offset );
	}
	set_datacnf( 0, 0 );		/* ターミネーター */

	head.tagnum = datacnf_p;

	fwrite( &head, sizeof( DATACNF_HEADER ), 1, out );
	fwrite( datacnf_work, sizeof( DATACNF_TAG ), datacnf_p, out );
PRINTF( "DATACNF BODY SIZE = %d\n", sizeof( DATACNF_HEADER ) + sizeof( DATACNF_TAG ) * datacnf_p );
	trim_sector_alignment( out );
}

/*
   データ本体部の作成
*/

static void save_stage_file_body( FILE *out )
{
	DATAFILE *p;
	int mode;
	int top;

	p = datafile_top;
	mode = p->mode;
	top = ftell( out );

PRINTF( "TOP POSITON %X\n", top );

	for( ; p != NULL; p = p->next ){
		if( p->mode != mode ){
PRINTF( "TRIM\n" );
			trim_sector_alignment( out );
			mode = p->mode;
		}
if( ( unsigned char )p->name[ 0 ] == 0xFF ){
PRINTF( "%08X %08X\n", ftell( out ) - top, *( int * )&p->name[1] );
} else {
PRINTF( "%08X %s\n", ftell( out ) - top, p->name );
}
		fwrite( p->body, sizeof( char ), p->size, out );
		switch( p->flag ){
		  case FLAG_ALIGN_NORMAL:
			trim_align( out, 16 );
			break;
		  case FLAG_ALIGN_SECTOR:
			trim_align( out, 2048 );
			break;
		  case FLAG_ALIGN_128:
			trim_align( out, 128 );
			break;
		}
	}
	trim_sector_alignment( out );
}

/* ---------------------------------------------------------------------- */
/*
	圧縮バージョン。
	datacnfブロックと本体ブロックを同時に処理する
	同時に簡単な暗号化もかける。
*/

#define Z_BUFFER_SIZE	(64*1024)		// 圧縮バッファサイズ

static char z_out_buffer[ Z_BUFFER_SIZE ];	// 圧縮バッファ
static int _first = 0;

static void compress_init( z_stream *z )
{
	z->zalloc = NULL;
	z->zfree = NULL;
	z->opaque = NULL;

	if( deflateInit( z, Z_DEFAULT_COMPRESSION ) != Z_OK ){
		printf( "%s:%s\n", __FUNCTION__, z->msg );
		exit( 1 );
	}
	_first = 1;

	z->next_out = z_out_buffer;
	z->avail_out = Z_BUFFER_SIZE;
}

static void write_compress_buffer( FILE *out, z_stream *z )
{
	int size;

	size = Z_BUFFER_SIZE - z->avail_out;
	if( size > 0 ){
		if( _first ){
			unsigned char *p = ( unsigned char * )z_out_buffer;
			if( p[ 0 ] != 0x78 || p[ 1 ] != 0x9C ){
				printf( "Error: Comprss Header\n" );
				exit( 1 );
			}
			p[ 0 ] = 0x85;
			p[ 1 ] = 0x93;
			_first = 0;
		}
		if( encode_flag ){
			encode_buffer( z_out_buffer, size );
		}
		if( fwrite( z_out_buffer, sizeof( char ), size, out ) != size ){
			printf( "Output Error\n" );
			exit( 1 );
		}
	}
	z->next_out = z_out_buffer;
	z->avail_out = Z_BUFFER_SIZE;
}

static void compress_write( FILE *out, void *buffer, int size, z_stream *z )
{
	z->avail_in = size;
	z->next_in = buffer;

	while( z->avail_in > 0 ){
		int status;

		status = deflate( z, Z_NO_FLUSH );
		if( status != Z_OK ){
			printf( "%s:%s\n", __FUNCTION__, z->msg );
			exit( 1 );
		}
		if( z->avail_out == 0 ){
			write_compress_buffer( out, z );
		}
	}
}

static void compress_flush( FILE *out, z_stream *z )
{
	for( ;; ){
		int status;
		status = deflate( z, Z_FINISH );
		if( status == Z_STREAM_END ){
			break;
		}
		if( status != Z_OK ){
			printf( "%s:%s\n", __FUNCTION__, z->msg );
			exit( 1 );
		}
		if( z->avail_out == 0 ){
			write_compress_buffer( out, z );
		}
	}
	write_compress_buffer( out, z );

	if( deflateEnd( z ) != Z_OK ){
		printf( "%s:%s\n", __FUNCTION__, z->msg );
		exit( 1 );
	}
}

static void compress_file_output( FILE *out, DATAFILE *p, z_stream *z )
{
	/* あるファイルを圧縮してoutに出力する */
	/* アライメントも合わせる */

	static char align_buf[ 2048 ] = { 0 };
	int size;

	compress_write( out, p->body, p->size, z ) ;

	size = NORM_ALIGN( p->size, get_align( p->flag ) ) - p->size;
	if( size > 0 ){
		compress_write( out, align_buf, size, z );
	}
}

static int is_compress_section( int mode )
{
	switch( mode ){
	  case DATA_SOUND:
		return 0;
	}
	return 1;
}

static void output_body_file( FILE *out, FILE *fp )
{
	int len;

	fseek( fp, 0, SEEK_END );
	len = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	while( len > 0 ){
		int size;
		size = ( len > Z_BUFFER_SIZE ) ? Z_BUFFER_SIZE : len;
		fread( z_out_buffer, sizeof( char ), size, fp );
		if( fwrite( z_out_buffer, sizeof( char ), size, out ) != size ){
			printf( "Write Error\n" );
			exit( 1 );
		}
		len -= size;
	}
}

static void save_compressed_body( FILE *out, M_TAG *tag )
{
	DATAFILE *p;
	DATACNF_HEADER head;
	DATACNF_TAG *top_tag;
	DATACNF_TAG *compress_tag;
	FILE *z_fp;
	int mode;
	int pos, top_pos;
	int compress_top, compress_section;
	z_stream z;
	char tmpfilename[ 256 ];
	int stage_top;

	compress_section = 0;
	compress_top = 0;

	datacnf_p = 0;
	p = datafile_top;

	mode = -1;

	stage_top = ftell( out );
	top_pos = pos = 0;
	top_tag = NULL;
	compress_tag = NULL;
#if 0
	if( tmpnam( tmpfilename ) == NULL ){
		printf( "tmpnam error\n" );
		exit( 1 );
	}
	z_fp = fopen( tmpfilename, "wb" );
#else
	z_fp = tmpfile();
#endif
	if( z_fp == NULL ){
		printf( "tmpfile %s open error\n", tmpfilename );
		exit( 1 );
	}

	for( ; p != NULL; p = p->next ){
		if( mode != p->mode ){
			if( top_tag != NULL ){
				// 直前のタグにセクションのサイズを入れる
				set_datacnf( DATA_SECTION_END, pos - top_pos );
				top_tag->offset = pos - top_pos;

				if( compress_section ){
					int offset;

					compress_flush( z_fp, &z );
					offset = ftell( z_fp ) - compress_top;
					if( offset > 0x00FFFFFF ){
						printf( "TOO LARGE SECTION %d\n", offset );
					}
					compress_tag->id |= offset;
				}

				// INSTALLのためのサイズ計算
				if( mode == DATA_BINARY ){
					int size;
					size = ( compress_tag->id & 0x00FFFFFF );
					install_total_size += SECTOR( size );
				} else {
					install_total_size += SECTOR( top_tag->offset );
				}
				PRINTF( "SECTION SIZE %d\n", top_tag->offset );
			}

			// セクション毎にセクタアライメント
			trim_sector_alignment( z_fp );
			pos = NORM_ALIGN( pos, 2048 );
			PRINTF( "NEW SECTION %X pos %X\n", p->mode, pos );

			top_tag = set_datacnf( p->mode, 0 );
			top_pos = pos;
			mode = p->mode;
			compress_section = is_compress_section( mode );

			if( compress_section ){
				// 圧縮ファイルへのオフセット
				compress_top = ftell( z_fp );
				compress_tag = set_datacnf( DATA_COMPRESS_SECTION, compress_top );

				if( p->mode == DATA_BINARY ){
					/* encode */
					set_encode();
				} else {
//					unset_encode();
					set_encode();
				}
				compress_init( &z );
			}
		}
		// ファイルを登録
		set_file_datacnf( p->name, pos - top_pos );
		pos += p->size;
		pos = NORM_ALIGN( pos, get_align( p->flag ) );

		if( compress_section ){
			// 圧縮してファイルを出力
			compress_file_output( z_fp, p, &z );
		} else {
			// 普通にファイルを出力(サウンドセクションなど)
			fwrite( p->body, sizeof( char ), p->size, z_fp );
			trim_align( z_fp, get_align( p->flag ) );
		}
	}
	if( top_tag != NULL ){
		if( compress_section ){
			int offset;
			compress_flush( z_fp, &z );
			offset = ftell( z_fp ) - compress_top;
			if( offset > 0x00FFFFFF ){
				printf( "TOO LARGE SECTION %d\n", offset );
			}
			compress_tag->id |= offset;
		}
		set_datacnf( DATA_SECTION_END, pos - top_pos );
		top_tag->offset = pos - top_pos;
		pos = NORM_ALIGN( pos, 2048 );

		// INSTALLのためのサイズ計算
		if( mode == DATA_BINARY ){
			int size;
			size = ( compress_tag->id & 0x00FFFFFF );
			install_total_size += SECTOR( size );
		} else {
			install_total_size += SECTOR( top_tag->offset );
		}

		PRINTF( "SECTION SIZE %d\n", top_tag->offset );
	}
	set_datacnf( 0, 0 );		/* ターミネーター */

	head.tagnum = datacnf_p;

	seed = 0xA78925D9 + ( current_stage_id << 7 ) + current_stage_id + header_seed;
	_seed = 0x7A88FB59 + ( current_stage_id << 7 ) + current_stage_id;

	encode_buffer( &head, sizeof( DATACNF_HEADER ) );
	fwrite( &head, sizeof( DATACNF_HEADER ), 1, out );
	encode_buffer( datacnf_work, sizeof( DATACNF_TAG ) * datacnf_p );
	fwrite( datacnf_work, sizeof( DATACNF_TAG ), datacnf_p, out );

	PRINTF( "DATACNF BODY SIZE = %d\n", sizeof( DATACNF_HEADER )
			+ sizeof( DATACNF_TAG ) * datacnf_p );

	// INSTALLのためのサイズ計算
	install_total_size += SECTOR( sizeof( DATACNF_HEADER )
								  + sizeof( DATACNF_TAG ) * datacnf_p );

	trim_sector_alignment( out );
	trim_sector_alignment( z_fp );
	pos += ftell( out ) - stage_top;

	/* tmpfileに作成したステージファイルを出力する */
	output_body_file( out, z_fp );

	fclose( z_fp );

	{
		int bodysize;
		bodysize = ftell( out ) - stage_top;
		fprintf( stderr, "%-8.8s %dK->%dK(%2.2f)\n", current_stage, pos / 1024
				, bodysize / 1024, 100.0 * bodysize / pos );
	}
}

/* ---------------------------------------------------------------------- */
/*
	読み込んだdata.cnf中のすべてのファイルを
	まとめてoutに出力する。
*/

static void save_stage_data( FILE *out, M_TAG *tag )
{
	// 出力
	if( compress_mode == 0 ){
		int top, bottom;

		// 非圧縮出力
		top = ftell( out );
		save_datacnf( out, tag );
		save_stage_file_body( out );
		bottom = ftell( out );

		fprintf( stderr, "%-8.8s stage size = %dK\n", current_stage
				 , ( bottom - top ) / 1024 );
	} else {
		// 圧縮出力
		save_compressed_body( out, tag );
	}
}

/* ---------- ステージデータの作成本体 */
/*
	一つのステージの作成を行なう。
*/

static int append_stage_dir( char *stagepath, char *stagename, FILE *out )
{
	M_TAG m_tag;
	char *datacnf;

	init_local_memory( &m_tag );

	current_stage = stagename;

	if( ( datacnf = load_file( stagepath, "data.cnf", NULL, NULL, &m_tag ) ) != NULL ){
		/* data.cnf があれば解析開始 */

		// datacnf中のすべてのファイルをメモリ上に読み込む
		if( load_all_datacnf_data( stagepath, datacnf, &m_tag ) < 0 ){
			free_local_all( &m_tag );
			return -1;
		}

		// サウンドのpakファイルもメモリ上に読み込む
		load_sound_data( stagepath, &m_tag );

		if( test_mode ){
			free_local_all( &m_tag );
			return TRUE;
		}

		// ステージ先頭をワークに登録
		set_stage_file_top( stagename, out );
		current_stage_id = get_strcode( stagename );

		// 読み込んだデータをステージファイルとして出力
		save_stage_data( out, &m_tag );
	}

	free_local_all( &m_tag );

	return TRUE;
}

/* --------------- ステージファイル作成関数 ----------------- */

typedef struct stagepath {
	struct stagepath *next;
	char fullpath[ 256 ];
	char name[ 64 ];
} STAGEPATH;

/*
	ステージファイル作成本体
*/

static int output_stage_file( char *distfile, STAGEPATH *list, int count )
{
	FILE *stage_fp;
	STAGEPATH *now;
	int size;

	if( ( stage_fp = fopen( distfile, "wb" ) ) == NULL ){
		printf( "can't open file %s\n",distfile );
		return -1;
	}

	install_total_size = 0;

	// ステージ数からステージ情報テーブルの大きさだけ
	// ダミーデータを作成する
	init_stage_table( stage_fp, count );

	for( now = list; now != NULL; now = now->next ){
		PRINTF( "append %s\n", now->name );
		fprintf( stderr, "append %s         \r", now->name );
		// ディレクトリ毎のファイルを作成
		if( append_stage_dir( now->fullpath, now->name, stage_fp ) < 0 ){
			fprintf( stderr, "%-8.8s skipped.\n", now->name );
		}
	}

	if( test_mode ){
		fclose( stage_fp );
		return -1;
	}

	// ステージ情報テーブルをファイル先頭に作成
	save_stage_table( stage_fp );

	fseek( stage_fp, 0, SEEK_END );
	size = ftell( stage_fp );

	fclose( stage_fp );

	printf( "Total size       %8d Kbyte\n", size / 1024 );
	if( compress_mode != 0 ){
		printf( "HDD Install Size %8d Kbyte\n"
				, install_total_size * (SECTOR_SIZE/1024) );
	}

	return size;
}

/*
	新バージョンの方針。
	*.darの中身も、ほかのファイルも、みんなまとめてdata.cnfに記述。
	最初に読み込んで、Mallocする。
	strcodeで変換。データ中にはファイル名は入れない。
	１ステージごとにパースしながら、データファイルを作成していく。
	最後に、このファイルのヘッダを別ファイル or 先頭に持っていって、出力。
*/

/* ---------------------------------------------------------------------- */
/*
	呼出し関数
*/

static char stage_path[ 256 ];

static int dir_select( struct dirent *dp )
{
	// 対象ディレクトリを決める。
	struct stat st;
	int res;
	char path[ 256 ];

	sprintf( path, "%s/%s", stage_path, dp->d_name );
	res = stat( path, &st );
	if( res < 0 ){
		FATAL( "stat error %d\n", res );
	}
	if( S_ISDIR( st.st_mode ) ){
		if( strlen( dp->d_name ) > 7 ){
			printf( "error: %s\n", dp->d_name );
			return 0;
		}
		if( strrchr( dp->d_name, '.' ) == NULL ){
			return 1;
		}
		printf( "skip %s\n", dp->d_name );
	}
	return 0;
}

static int dir_sort_getkey( const char *name )
{
	int value, numf;
	const char *p;

	value = 0;
	numf = 0;
	for( p = name; *p != '\0'; p++ ){
		if( isdigit( *p ) ){
			value = value * 10 + *p - '0';
			numf = 1;
		} else {
			if( numf != 0 ){
				break;
			}
		}
	}
	if( numf != 0 ){
		return value;
	} else {
		return -1;
	}
}

static int dir_sort_cmp( const struct dirent **dir1, const struct dirent **dir2 )
{
	// ディレクトリをソートする。
	// ここでソートした順番にまとめられる

	return dir_sort_getkey( ( *dir1 )->d_name ) - dir_sort_getkey( ( *dir2 )->d_name );
}

int build_stage_file( char *dest_path, char *src_path, char *(*getnext)( char *buf ), char *tsymbol_file )
{
	char destfile[ 256 ];
	STAGEPATH *top, *now;
	M_TAG tag;
	int count;
	int size;

	init_local_memory( &tag );

	strcpy( destfile, dest_path );
	strcpy( stage_path, src_path );

	srand( time( NULL ) );

	top = NULL;
	now = ( STAGEPATH * )&top;
	count = 0;

	if( getnext == NULL ){
		/* ディレクトリをスキャンしてすべてのディレクトリを登録 */
		int i, dir_n;
		struct dirent **entry;

		if( ( dir_n = scandir( stage_path, &entry
							   , ( void * )dir_select, ( void * )dir_sort_cmp ) ) <= 0 ){
			ERROR( "%s: スキャンできませんでした。\n", src_path );
		}

		for( i = 0; i < dir_n; i++ ){
			char datacnf_file[ 256 ];
			printf( "%s\n", entry[ i ]->d_name );

			sprintf( datacnf_file, "%s/%s/data.cnf", src_path, entry[ i ]->d_name );
			if( get_file_size( datacnf_file ) > 0 ){
				/* data.cnf が存在したので、登録 */
				STAGEPATH *p;

				p = malloc_local( &tag, sizeof( STAGEPATH ) );
				p->next = NULL;
				sprintf( p->fullpath, "%s/%s", src_path, entry[ i ]->d_name );
				strcpy( p->name, entry[ i ]->d_name );

				now->next = p;
				now = p;
				count ++;
			}
		}
		free( entry );
	} else {
		/* stagelistにあるディレクトリを登録 */
		for( ;; ){
			char datacnf_file[ 256 ];
			char dirname[ 64 ];
			if( (*getnext)( dirname ) == NULL ){
				break;
			}
			printf( "%s\n", dirname );
			sprintf( datacnf_file, "%s/%s/data.cnf", src_path, dirname );
			if( get_file_size( datacnf_file ) > 0 ){
				/* data.cnf が存在したので、登録 */
				STAGEPATH *p;

				p = malloc_local( &tag, sizeof( STAGEPATH ) );
				p->next = NULL;
				sprintf( p->fullpath, "%s/%s", src_path, dirname );
				strcpy( p->name, dirname );

				now->next = p;
				now = p;
				count ++;
			} else {
				ERROR( "%s:ディレクトリが存在しません\n", dirname );
			}
		}
	}
	printf( "Total %d stages\n", count );

	size = output_stage_file( destfile, top, count );

	if( tsymbol_file[ 0 ] != '\0' ){
		FILE *tfp;
		int i;

		stage_table[ stage_table_p ].offset = ( size + SECTOR_SIZE - 1 )
			/ SECTOR_SIZE;

		if( ( tfp = fopen( tsymbol_file, "wt" ) ) == NULL ){
			ERROR( "%s:オープンできません\n", tsymbol_file );
		}

		for( i = 0; i < stage_table_p; i++ ){
			int ofs;
			ofs = stage_table[ i + 1 ].offset - stage_table[ i ].offset;
			ofs = ( ofs << 20 ) | stage_table[ i ].offset;
			fprintf( tfp, "0x%08X %s\n", ofs, stage_table[ i ].name );
		}

		fclose( tfp );
	}

	free_local_all( &tag );

	return 0;
}
