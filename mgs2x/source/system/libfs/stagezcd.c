/*
	stagezcd.c
		CD ファイルシステム用圧縮ステージデータロードルーチン

	# 本来はGMが担当すべきモジュールだが、HD、CDの切り分けを
	# 行うために、FSに置く。

	2000/06/06 K.Uehara
	$Id: stagezcd.c,v 1.1.1.3 2002/11/19 11:42:41 Yoshizawa1 Exp $
*/

#ifdef PSX2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sifdev.h>

#endif

#ifdef KP_XBOX
#include <xtl.h>
#define SyncDCache( p1, p2 )
#define FlushCache( f )
#define scePrintf	printf
#endif

#include "cdbios.h"

#include "libgv.h"
#include "libfs.h"

#include "sd_ee.h"

#define PS2_EE
#include "zlib.h"

#include "mts.h"

#define TIMECHECK

#ifdef TIMECHECK
extern int DG_TickCount;
#endif

#define BINARY_ENCODE	1		// BINARY sectionのみEncode


/* ---------------------------------------------------------------------- */
/*
	CDからの圧縮データを展開するタスク
*/

#define ZLIB_READBUFFER			(128*1024)
#define ZLIB_WORK_SIZE			(45656)
#define ZLIB_TASK_STACK_SIZE	(16*1024)

#ifdef KP_XBOX
#define ZLIB_TASK_PRI			(THREAD_PRIORITY_BELOW_NORMAL)
#endif
#ifdef PSX2
#define ZLIB_TASK_PRI			(14)
#endif

enum {
	ZLIBDEC_IDLE = 0,
	ZLIBDEC_COMMAND_DECOMPRESS_READ = 1,
	ZLIBDEC_COMMAND_NORMAL_READ,
	ZLIBDEC_COMMAND_END
};

typedef struct {
	u_long128 read_buffer[ 2 ][ ZLIB_READBUFFER / sizeof( u_long128 ) ];
	CDBIOS_READ_STATUS read_status;		// 内部ステータス
	int task_id;
	int sem_id;
	int command;
	int which;
	int org_size;
	int decoded_size;
	void (*callback_func )( CDBIOS_READ_STATUS *rs );
	char zlib_buf[ ZLIB_WORK_SIZE ];
	u_long128 stack[ ZLIB_TASK_STACK_SIZE / sizeof( u_long128 ) ];
} ZLIBDEC_WORK;

static ZLIBDEC_WORK *zlibdec_work;

static inline void SLEEP( ZLIBDEC_WORK *work )
{
	WaitSema( work->sem_id );
}

static inline void Wakeup( ZLIBDEC_WORK *work )
{
	SignalSema( work->sem_id );
}

static inline void iWakeup( ZLIBDEC_WORK *work )
{
	iSignalSema( work->sem_id );
}

#ifdef BINARY_ENCODE

/*
	バイナリデータを復号化するためのルーチン
*/

static unsigned int seed;
static unsigned int _seed;

static void setup_decrypt( void *buffer )
{
	unsigned short *p = buffer;
	unsigned int s;

	s = *p ^ 0x9385;
	seed = ( s ) | ( ( s ^ 0x6576 ) << 16 );
	_seed = s * 278;
}

static void decrypt_buffer( void *buffer, int size )
{
	unsigned int *p = buffer;
	unsigned int s, _s;

	s = seed;
	_s = _seed;

	for( ; size > 0; size -= sizeof( int ) ){
		*p = *p ^ s;
		s = s * 48828125 + _s;
		p++;
	}
	seed = s;
}

#endif

static inline int NEED_ALIGN( unsigned int id )
{
	int type = ( id >> 24 );

	switch( type ){
#ifdef PSX2
	  case 't'-'a':
		return 128;
#endif
#ifdef KP_XBOX
	  case 'x'-'a':
		return 128;
#endif
	}
	return 16;
}


/*
	CDから圧縮されたデータをWORK中のバッファに読み込み,
	順次展開して目的のバッファに転送する。
	ステージロードではバッファは折り返さないため,途中停止などはサポートしない
*/

static void zlibdec_read_callback( CDBIOS_READ_STATUS *rs )
{
	iWakeup( zlibdec_work );
}

static void zlibdec_read_start( void *buffer, int pos, int size )
{
	while( cdbios_get_status() != 0 ){
		;	// 読み込み待ち
	}
	SyncDCache( buffer, buffer + size );
	cdbios_callback_read( buffer, pos, size, zlibdec_read_callback, -1 );
}

static void zlibdec_read( ZLIBDEC_WORK *work )
{
	CDBIOS_READ_STATUS *rs;
	int read, rest;
	z_stream z;
	int first = 1;

	rs = &( work->read_status );
	rs->intr_size = -1;

	work->which = 0;
	read = 0;
	rest = rs->total_size;

	// 初期読み込み開始
	read = ( rest > ZLIB_READBUFFER ) ? ZLIB_READBUFFER : rest;

	zlibdec_read_start( work->read_buffer[ work->which ], rs->pos, read );
	rs->pos += ( FS_SECTOR_ALIGN( read ) / FS_SECTOR_SIZE );

	// 展開ライブラリセットアップ
	ASSERT( inflateGetWorkSize() == ZLIB_WORK_SIZE );
	z.buffer = work->zlib_buf;
	if( inflateInit( &z ) != Z_OK ){
printf( "Inflate\n" );
		HANGUP();
	}
	z.next_out = rs->ee_buffer;
	z.avail_out = work->org_size;

	while( rs->read_size < rs->total_size ){
		SLEEP( work );	// 読み込み終了待ち

		rest = rest - read;
		rs->read_size += read;

		// 展開設定
		z.next_in = ( void * )work->read_buffer[ work->which ];
		z.avail_in = read;
#ifdef BINARY_ENCODE
		if( first ){
			first = 0;
			setup_decrypt( z.next_in );
			decrypt_buffer( z.next_in, read );
			*( short * )z.next_in += ( 0x9C78 - 0x9385 );
		} else {
			decrypt_buffer( z.next_in, read );
		}
#endif
		// 次の読み込みをセットアップ
		if( rest > 0 ){
			int r;
			work->which = 1 - work->which;
			read = ( rest > ZLIB_READBUFFER ) ? ZLIB_READBUFFER : rest;
			zlibdec_read_start( work->read_buffer[ work->which ], rs->pos, read );
			r = FS_SECTOR_ALIGN( read );
			rs->pos += ( r / FS_SECTOR_SIZE );
		}

		// 展開メイン
		{
			int status;

			for( ;; ){
				status = inflate( &z, Z_NO_FLUSH );
				work->decoded_size = work->org_size - z.avail_out;
				if( status == Z_STREAM_END || z.avail_in == 0 ){
					/* 終了 */
					break;
				}
				if( status != Z_OK ){
					printf( "pos = %d status = %d %s\n", work->decoded_size, status, z.msg );
					HANGUP();
				}
			}
		}
	}
	/* 終り */
	inflateEnd( &z );
}

/*
	非圧縮部のロード
*/

static void normal_read( ZLIBDEC_WORK *work )
{
	CDBIOS_READ_STATUS *rs = &( work->read_status );
	int size;

	size = rs->total_size - rs->read_size;
	if( rs->intr_size > 0 && rs->intr_size < size ){
		size = rs->intr_size;
	}

	zlibdec_read_start( rs->ee_buffer, rs->pos, size );
	SLEEP( work );

	rs->read_size += size;
	( char * )rs->ee_buffer += size;

	rs->pos += FS_SECTOR_ALIGN( size ) / FS_SECTOR_SIZE;
}

/*
	進行状況取得
*/

int zlibdec_get_read_size( void )
{
	ZLIBDEC_WORK *work = zlibdec_work;

	if( work->command == ZLIBDEC_COMMAND_DECOMPRESS_READ ){
		return work->decoded_size;
	} else if( work->command == ZLIBDEC_COMMAND_NORMAL_READ ){
		return cdbios_get_read_size();
	}
	return work->read_status.read_size;
}

/*
	デコードタスクメイン
*/

static void zlibdec_main( int id, void *arg )
{
	ZLIBDEC_WORK *work = arg;

	for( ;; ){
		SLEEP( work );

		while( work->command != ZLIBDEC_IDLE ){
			switch( work->command ){
			  case ZLIBDEC_COMMAND_DECOMPRESS_READ:
				zlibdec_read( work );
				break;
			  case ZLIBDEC_COMMAND_NORMAL_READ:
				normal_read( work );
				break;
			  case ZLIBDEC_COMMAND_END:
				work->task_id = -1;
				return;
			}
			if( work->callback_func != NULL ){
				( *work->callback_func )( &work->read_status );
			}
			if( work->read_status.read_size == work->read_status.total_size ){
				/* 終了 */
				work->command = ZLIBDEC_IDLE;
				break;
			}
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	メインタスクからデコードタスクへ
*/

/*
	リード要求
*/

static void zdec_callback_read( void *buffer, int pos, int size, int org_size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size )
{
	ZLIBDEC_WORK *work = zlibdec_work;
	CDBIOS_READ_STATUS *rs;

	rs = &work->read_status;

	if( pos > 0 ){
		rs->pos = pos;
	}

	rs->ee_buffer = buffer;
	rs->intr_size = intr_size;
	rs->total_size = size;
	rs->read_size = 0;
	work->decoded_size = 0;
	work->org_size = org_size;
	work->callback_func = callback;

	work->command = ZLIBDEC_COMMAND_DECOMPRESS_READ;

	Wakeup( work );
}

static void normal_callback_read( void *buffer, int pos, int size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size )
{
	ZLIBDEC_WORK *work = zlibdec_work;
	CDBIOS_READ_STATUS *rs;

	rs = &work->read_status;

	if( pos > 0 ){
		rs->pos = pos;
	}

	rs->ee_buffer = buffer;
	rs->intr_size = intr_size;
	rs->total_size = size;
	rs->read_size = 0;
	rs->read_size = 0;
	work->callback_func = callback;

	work->command = ZLIBDEC_COMMAND_NORMAL_READ;

	Wakeup( work );
}

/*
	デコードタスク起動
*/

static void start_decode_task( ZLIBDEC_WORK *work )
{
	work->which = 0;

	zlibdec_work = work;
	work->task_id = MTS_NewThread( "STGDEC", zlibdec_main, ZLIB_TASK_PRI
								   , work->stack, ZLIB_TASK_STACK_SIZE, work );
}

/*
	デコードタスク終了
*/

static void kill_decode_task( ZLIBDEC_WORK *work )
{
	work->command = ZLIBDEC_COMMAND_END;
	Wakeup( work );
}

/* ---------------------------------------------------------------------- */
/*
	ステージファイル管理情報を読み込んで初期化。
*/

typedef struct {
	unsigned int seed;
	short version;
	short sector;
	short stagenum;
	short padding;
	int install_size;
} STAGE_TABLE_HEADER;

#define STAGE_NAME_LEN	8

typedef struct {
	char name[ STAGE_NAME_LEN ];
	int offset;
} STAGE_TABLE;

typedef struct {
	STAGE_TABLE_HEADER table_header;
	STAGE_TABLE *stage_table;
} STAGE_FILE;

static int taskid;

//static STAGE_TABLE_HEADER fs_table_header;
//static STAGE_TABLE *stage_table;

#define MAX_LAYER 2

extern int FS_current_layer;
static STAGE_FILE fs_stage[ MAX_LAYER ];

static void init_callback( CDBIOS_READ_STATUS *rs )
{
	if( rs->read_size == FS_SECTOR_SIZE ){
		STAGE_TABLE_HEADER *table;
		STAGE_TABLE_HEADER *target;
		
		// 1回め
		table = ( STAGE_TABLE_HEADER * )( ( char * )rs->ee_buffer - FS_SECTOR_SIZE );
		target = &( fs_stage[ FS_current_layer ].table_header );
		memcpy( target, table, sizeof( STAGE_TABLE_HEADER ) );

		seed = target->seed;
		_seed = seed ^ 0xF0F0;
		decrypt_buffer( ( char * )target + 4, sizeof( STAGE_TABLE_HEADER ) - 4 );

		rs->total_size = target->sector * FS_SECTOR_SIZE;
	}
	if( rs->total_size == rs->read_size ){
		// 終了
		BP_iWakeupThread( taskid );
	}
}

void FS_CdStageFileInit( int alt )
{
	void *buffer;
	STAGE_TABLE *stage_table;
	int stagenum;
	int pos;
	int maxsize = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY );

	taskid = BP_GetThreadId();

	buffer = GV_Malloc( maxsize );

#ifdef PSX2
	if( alt ){
		pos = FS_GET_FILE_ALT_POS( FS_FILE_STAGE );
		FS_current_layer = FS_GET_FILE_ALT_LAYER( FS_FILE_STAGE );
	} else {
		pos = FS_GET_FILE_POS( FS_FILE_STAGE );
		FS_current_layer = FS_GET_FILE_LAYER( FS_FILE_STAGE );
	}
#else
	pos = FS_GET_FILE_POS( FS_FILE_STAGE );
	FS_current_layer = 0;
#endif

	while( cdbios_get_status() != 0 ){
		;	// 読み込み待ち
	}
	SyncDCache( buffer, buffer + maxsize );
	cdbios_callback_read( buffer, pos, maxsize
								 , init_callback, FS_SECTOR_SIZE );
	BP_SleepThread();

	stagenum = fs_stage[ FS_current_layer ].table_header.stagenum;

	stage_table = GV_AllocResidentMemory( stagenum * sizeof( STAGE_TABLE )
										  , 0 );
	{
		int i;
		int top;
		STAGE_TABLE *s;

		s = ( STAGE_TABLE * )( ( char * )buffer + sizeof( STAGE_TABLE_HEADER ) );
		top = pos;

		decrypt_buffer( s, stagenum * sizeof( STAGE_TABLE ) );

		for( i = 0; i < stagenum; i++ ){
			memcpy( stage_table[ i ].name, s[ i ].name, STAGE_NAME_LEN );
			stage_table[ i ].offset = s[ i ].offset + top;
printf( "%08X %s\n", stage_table[ i ].offset, stage_table[ i ].name );
		}
	}
	fs_stage[ FS_current_layer ].stage_table = stage_table;

	GV_Free( buffer );
}

static int get_stage_pos( char *name )
{
	int i;
	STAGE_FILE *sf;

	sf = &fs_stage[ FS_current_layer ];
	for( i = 0; i < sf->table_header.stagenum; i++ ){
		if( strcmp( sf->stage_table[ i ].name, name ) == 0 ){
			return sf->stage_table[ i ].offset;
		}
	}
	sf = &fs_stage[ 1 - FS_current_layer ];
	if( sf->stage_table == NULL ) return 0;
	for( i = 0; i < sf->table_header.stagenum; i++ ){
		if( strcmp( sf->stage_table[ i ].name, name ) == 0 ){
			FS_current_layer = 1 - FS_current_layer;
			return sf->stage_table[ i ].offset;
		}
	}
//	ASSERT( FALSE );
	return -1;
}

/* ---------------------------------------------------------------------- */
/*
	ステージデータの読み込み
*/


/*
	割り込みルーチンのSTEP
*/
enum {
	CB_STEP_STARTUP,
	CB_STEP_DATACNF_LOAD,
	CB_STEP_SECTION_LOAD,
	CB_STEP_SLEEP,
};

/*
	初期化ルーチンのSTEP
*/

enum {
	IN_STEP_STARTUP,			/* datacnf読み終わり待ち */
	IN_STEP_BINLOAD,			/* bin block 読み込み中 */
	IN_STEP_INITIALIZE,			/* セクション初期化中 */
	IN_STEP_INITIALIZE_LAST,	/* 最後のセクションの初期化中 */
	IN_STEP_END					/* 終了 */
};

/*
	構造体
*/

typedef struct {
	int id;
	int offset;
} DATACNF_TAG;

typedef struct {
	int tagnum;
	DATACNF_TAG tags[0];
} DATACNF;

typedef struct {
	DATACNF_TAG *section_tag;
	void *section_addr;
	int	section_top;
} SECTION_INFO;

typedef struct {
	int stage_top_pos;
	void *buffertop;
	int	callback_step;
	int init_step;
	int cnf_size;
	int tagnum;
	DATACNF	*datacnf;
	SECTION_INFO *section_info;

	// 割り込みルーチン（読み込み側）のステータス
	SECTION_INFO *read_section_p;	// 読み込むセクションへのポインタ
	void *read_buf_top;

	SECTION_INFO *next_read_section_p;
	void *next_read_buf_top;

	// メインタスク（初期化側）のステータス
	SECTION_INFO *init_section_p;	// 初期化するセクションへのポインタ
	int init_type;
	DATACNF_TAG *cnf_p;
	int current_stage_id;

} FS_STAGE_INFO;

#define ID_SECTION_END	0x7F000000
#define ID_NOCACHE		0x7F000001
#define ID_CACHE		0x7F000002
#define ID_RESIDENT		0x7F000003
#define ID_SOUND		0x7F000010
#define ID_BINARY		0x7F010000
#define ID_BLOCK( no )	( 0x7F000100 | (no) )

#define ID_COMPRESS		0x7E000000
#define ID_NOP			0x70000000

#define IS_SECTION_TAG( a )	( ( (a)&0xFF000000 ) == 0x7F000000 )
#define IS_SECTION_END( a )	( (a) == ID_SECTION_END )
#define IS_PRELOAD_TAG( a )	( ( (a) & ~0xFF ) != ID_BLOCK(0) )
#define IS_END_TAG( a )		( (a) == 0 )

#define IS_COMPRESSION_TAG( a )		( ( (a)&0xFF000000 ) == 0x7E000000 )
#define COMPRESSION_SIZE( a )		( (a)&0x00FFFFFF )

#define IS_COMMAND_TAG( a )	( ( (a)&0xF0000000 ) == 0x70000000 )

static FS_STAGE_INFO *stage_info;
int fs_loaded_file_size;

/* loadの後も残しておくパラメータ */

static DATACNF *current_cnf;
static int current_cnf_size;
static int current_stage_top_pos;

static int is_load_section( int id )
{
	switch( id ){
	  case ID_BINARY:
	  case ID_NOCACHE:
	  case ID_CACHE:
	  case ID_RESIDENT:
		return 1;
	}
	return 0;
}

static int section_init_type( int id )
{
	switch( id ){
	  case ID_NOCACHE:
		return GV_INIT_NOCACHE;
	  case ID_CACHE:
		return GV_INIT_CACHE;
	  case ID_RESIDENT:
		return GV_INIT_RESIDENT;
	}
	return 0;
}

/*
	次に読み込むセクションを決定し、セットアップを行なう。
*/

static int setup_section_load( FS_STAGE_INFO *info, CDBIOS_READ_STATUS *rs )
{
	int id;
	DATACNF_TAG *tag;

	if( info->next_read_section_p == NULL ) return 0;

	tag = info->next_read_section_p->section_tag;
	if( tag == NULL || IS_END_TAG( tag->id ) ){
		// 終了
		info->read_section_p = info->next_read_section_p;
		info->next_read_section_p = NULL;

		return 0;
	}

	id = tag->id;

#ifdef DEBUG_MODE
	if( !IS_SECTION_TAG( id ) ){
printf( "SETUP_SECTION_LOAD_ERROR %x\n", id );
		scePrintf( "SETUP_SECTION_LOAD_ERROR %x\n", id );
		HANGUP();
	}
#endif

	info->read_buf_top = info->next_read_buf_top;
	rs->ee_buffer = info->next_read_buf_top;
	rs->read_size = 0;
	rs->intr_size = -1;

	// 圧縮版
	if( IS_COMPRESSION_TAG( ( tag + 1 )->id ) ){
		rs->total_size = COMPRESSION_SIZE( ( tag + 1 )->id );
		zlibdec_work->org_size = tag->offset;
		zlibdec_work->decoded_size = 0;
		zlibdec_work->command = ZLIBDEC_COMMAND_DECOMPRESS_READ;
	} else {
		rs->total_size = tag->offset;
		zlibdec_work->command = ZLIBDEC_COMMAND_NORMAL_READ;
	}

	/*	info->read_section_p = tag;*/
	info->read_section_p = info->next_read_section_p;
	info->next_read_section_p = NULL;
	info->callback_step = CB_STEP_SECTION_LOAD;

	return 1;
}

static inline void read_stop( CDBIOS_READ_STATUS *rs )
{
	rs->read_size = rs->total_size;
}

static void stageload_setup_callback( CDBIOS_READ_STATUS *rs )
{
	FS_STAGE_INFO *info = stage_info;

	switch( stage_info->callback_step ){
	  case CB_STEP_STARTUP:
		{
			int size;
			int tagnum;

			seed = 0xA78925D9 + ( info->current_stage_id << 7 )
				+ info->current_stage_id + fs_stage[ FS_current_layer ].table_header.seed;
			_seed = 0x7A88FB59 + ( info->current_stage_id << 7 ) + info->current_stage_id;
			decrypt_buffer( info->buffertop, FS_SECTOR_SIZE );

			current_cnf = info->datacnf = info->buffertop;
			tagnum = info->tagnum = info->datacnf->tagnum;
			size = sizeof( DATACNF ) + sizeof( DATACNF_TAG ) * tagnum;
			current_cnf_size = info->cnf_size = size;

			info->cnf_p = info->datacnf->tags;

			size = FS_SECTOR_ALIGN( size );
			if( rs->read_size < size ){
				rs->intr_size = size - rs->read_size;
				info->callback_step = CB_STEP_DATACNF_LOAD;
				// 読み込み継続
				break;
			}
			// DATACNFの読み込みが終っているのでそのまま次へ
			info->callback_step = CB_STEP_DATACNF_LOAD;
		}
	  case CB_STEP_DATACNF_LOAD:
		{
			if( rs->read_size > FS_SECTOR_SIZE ){
				decrypt_buffer( ( char * )info->buffertop + FS_SECTOR_SIZE
								, rs->read_size - FS_SECTOR_SIZE );
			}
			
			// mkcdimg BUG回避
			if( info->cnf_p->id == ID_SECTION_END ){
				info->cnf_p++;
			}
			// 次のトリガーがかかるまでスリープ
			info->next_read_section_p = NULL;
			info->read_section_p = NULL;
			info->callback_step = CB_STEP_SLEEP;
			// 読み込み完了
			read_stop( rs );
		}
		break;
	  case CB_STEP_SECTION_LOAD:
		/* セクション読み込み完了時に呼ばれる */
		{
			if( setup_section_load( info, rs ) == 0 ){
				// 次のトリガーがかかるまでスリープ
				info->callback_step = CB_STEP_SLEEP;
				// 読み込み完了
				read_stop( rs );
			}
		}
		break;
	}
}

/* ---------------------------------------------------------------------- */
/*
	メインコンテキスト
	1/60ごとに呼び出される。
*/

/*
	データを読み込んでいるうちに、次のセクションの読み込みの設定をやっておく。
*/

static void *setup_next_section( FS_STAGE_INFO *info, DATACNF_TAG *next )
{
	int size;
	void *buf;

	size = next->offset;
	/*
		そのセクションのサイズのメモリを確保
	*/
	buf = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size, 128 );
	ASSERT( buf != NULL );
	info->next_read_buf_top = buf;

	SyncDCache( buf, buf + size );

	return buf;
}

static int section_read_set( int pos, void *buffer, DATACNF_TAG *tag )
{
	if( IS_COMPRESSION_TAG( ( tag + 1 )->id ) ){
		// 圧縮情報
		DATACNF_TAG *cpr_tag;
		int offset;
		cpr_tag = tag + 1;
		offset = COMPRESSION_SIZE( cpr_tag->id );
//printf( "compression_size = %d %d\n", offset, FS_SECTOR_ALIGN( offset ) );
//printf( "uncompression_size = %d\n", tag->offset );
		zdec_callback_read( buffer, pos, FS_SECTOR_ALIGN( offset ), tag->offset
							, stageload_setup_callback, -1 );
		return 1;
	} else {
		// 非圧縮
		normal_callback_read( buffer, pos, tag->offset
							, stageload_setup_callback, -1 );
		return 0;
	}
}

static inline int stageload_act( FS_STAGE_INFO *info )
{
	switch( info->init_step ){
	  case IN_STEP_STARTUP:
		{
			int size;
			int read_start = 0;

			if( info->callback_step <= CB_STEP_DATACNF_LOAD ){
				break;
			}
			size = info->cnf_size;
			/* 各セクションに対応したテーブルを作成する */
			{
				SECTION_INFO *sinfo;
				int i, n, pos;
				DATACNF_TAG *tag;

				sinfo = ( SECTION_INFO * )( ( char * )info->buffertop + info->cnf_size );
				info->section_info = sinfo;
				n = 0;
				tag = info->datacnf->tags;
				pos = FS_SECTOR_ALIGN( size );

				for( i = 0; i < info->tagnum; i++ ){
					if( IS_SECTION_TAG( tag->id ) && tag->id != ID_SECTION_END ){
printf( "SECTION %X %X %X\n", sinfo, tag->id, tag->offset );
#ifdef PSX2
						if( tag->id == ID_BINARY ){
							// バイナリタグなので読み込み開始
							extern int _mgs2_keep_end[];
							int r;
							info->callback_step = CB_STEP_SECTION_LOAD;
							r = section_read_set( -1, _mgs2_keep_end, tag );
							if( r == 0 ) *( int * )1 = 0;	// address error
							read_start = 1;
						}
#endif
						if( is_load_section( tag->id ) ){
							sinfo->section_tag = tag;
							sinfo->section_addr = NULL;
							sinfo->section_top = pos / FS_SECTOR_SIZE;
							if( IS_COMPRESSION_TAG( ( tag + 1 )->id ) ){
								pos += FS_SECTOR_ALIGN( COMPRESSION_SIZE( ( tag + 1 )->id ) );
							} else {
								pos += FS_SECTOR_ALIGN( tag->offset );
							}
							sinfo++;
							n++;
						}
					} else {
//printf( "FILE %08X %08X\n", tag->id, tag->offset );
					}
					tag++;
				}
				sinfo->section_tag = NULL;
				sinfo->section_addr = NULL;
				n++;
				size = size + sizeof( SECTION_INFO ) * n;
			}
			/*
				セクションタグのメモリサイズにあわせる
			*/
printf( "TAG SIZE %d : %d\n", size, info->cnf_size );
			GV_Resize( info->buffertop, size );

			info->init_step = IN_STEP_INITIALIZE;
			{
				SECTION_INFO *top;
				top = info->section_info;
				ASSERT( top->section_tag != NULL );
				info->read_section_p = top;
				if( top->section_tag->id == ID_BINARY ){
					top++;
				}
				info->init_section_p = top;
				info->init_type = section_init_type( top->section_tag->id );
				info->cnf_p = top->section_tag + 1;
				if( IS_COMMAND_TAG( info->cnf_p->id ) ){
					info->cnf_p++;
				}
			}
			// バイナリがない場合は、先頭のセクションの読み込みスタート
			if( read_start == 0 ){
				SECTION_INFO *section;

				section = info->read_section_p;
				section->section_addr = setup_next_section( info, section->section_tag );
				info->callback_step = CB_STEP_SECTION_LOAD;
				info->next_read_section_p = NULL;

				section_read_set( info->stage_top_pos + section->section_top
								  , info->next_read_buf_top
								  , section->section_tag );
			}
		}
		/* そのまま次へ */
	  case IN_STEP_INITIALIZE:
		if( info->read_section_p == NULL ){
			break;
		}
		if( info->next_read_section_p == NULL ){
			/* 次に読み込むべきモジュールの設定 */
			SECTION_INFO *section;

			section = info->read_section_p + 1;
			if( section->section_tag == NULL ){
				/* 読み込み終了 */
				info->init_step = IN_STEP_INITIALIZE_LAST;
				info->next_read_section_p = section;
				if( info->callback_step == CB_STEP_SLEEP ){
					info->read_section_p = section;
				}
			} else {
				/* セクション読み込みのセットアップ */
				section->section_addr = setup_next_section( info, section->section_tag );
				if( info->callback_step == CB_STEP_SLEEP ){
					info->callback_step = CB_STEP_SECTION_LOAD;
					info->read_section_p = section;
					info->next_read_section_p = NULL;
					section_read_set( info->stage_top_pos + section->section_top
									  , info->next_read_buf_top
									  , section->section_tag );
					
				} else {
					info->next_read_section_p = section;
				}
			}
		}
	  case IN_STEP_INITIALIZE_LAST:
		/* 初期化部は共通 */
		for( ;; ){
			SECTION_INFO *section;
			DATACNF_TAG *cnf;
			void *datatop;
			int stat;

			section = info->init_section_p;

			if( section > info->read_section_p ){
				break;
			}

			cnf = info->cnf_p;
			if( IS_COMMAND_TAG( cnf->id ) ){
				// 圧縮タグなどであればスキップ
				cnf++;
			}

			if( section == info->read_section_p ){
				/* このセクションは読み込み中 */
				int read_size;

				read_size = zlibdec_get_read_size();
				if( read_size <= ( cnf + 1 )->offset ){
					/* 読み込みが完了していない */
					break;
				}
			}
			// 各データの初期化
			datatop = ( char * )section->section_addr + cnf->offset;
			if( info->init_type == GV_INIT_RESIDENT ){
				// RESIDENTは、常駐領域にコピーして初期化する
				int size;
				void *p;
				
				size = ( cnf + 1 )->offset - cnf->offset;
				p = GV_AllocResidentMemoryAligned( size, cnf->id, NEED_ALIGN( cnf->id ) );
				memcpy( p, datatop, size );
				datatop = p;
			}
			/* 初期化 */
printf( "Loadinit %X %X %X\n", datatop, cnf->id, (cnf+1)->offset );
			stat = GV_LoadInit( datatop, cnf->id, info->init_type );
			ASSERT( stat > 0 );
			cnf ++;

			if( IS_SECTION_END( cnf->id ) ) {
				/* 次のセクションの初期化へ移る */
				DATACNF_TAG *isec;

				info->init_section_p++;
				if( ( isec = info->init_section_p->section_tag ) == NULL ){
					info->init_step = IN_STEP_END;
					kill_decode_task( zlibdec_work );
					return 0;		// init完了
				}
				info->init_type = section_init_type( isec->id );
				cnf = isec + 1;
			}
			info->cnf_p = cnf;
		}
		break;
	  case IN_STEP_END:
	    return 0;
	}
	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	外部からの呼び出し
*/

#ifdef TIMECHECK
static int start_tick_time;
#endif

void *FS_LoadStageRequest( char *dirname )
{
	int size;
	int pos;
	char *buffer;
	void *zlibwork;

#ifdef TIMECHECK
printf( "LOAD START TICK = %d\n", ( start_tick_time = DG_TickCount ) );
#endif
	stage_info = GV_Malloc( sizeof( FS_STAGE_INFO ) );
	GV_ZeroMemory( stage_info, sizeof( FS_STAGE_INFO ) );

	stage_info->callback_step = CB_STEP_STARTUP;
	stage_info->init_step = IN_STEP_STARTUP;

	/* ロードバッファとしてとれるだけのメモリを確保 */
	/* 64 バイトアライメントにのるように確保 */
	size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY );
printf( "LOAD MEMORY %d\n", size );
	size -= sizeof( ZLIBDEC_WORK ) + 128;

	buffer = stage_info->buffertop = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size - 64, 64 );

	zlibwork = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, sizeof( ZLIBDEC_WORK ), 64 );
	GV_ZeroMemory( zlibwork, sizeof( ZLIBDEC_WORK ) );
	{
		struct SemaParam sema;
		sema.initCount = 0;
		sema.maxCount = 1;
		sema.option = 0;
		( ( ZLIBDEC_WORK * )zlibwork )->sem_id = CreateSema( &sema );
	}
	start_decode_task( zlibwork );

	/* 読み込むセクタ位置を取得 */
	pos = get_stage_pos( dirname );
	ASSERT( pos >= 0 );
	current_stage_top_pos = stage_info->stage_top_pos = pos;
	stage_info->current_stage_id = GV_StrCode( dirname );

	/* 念のため */
	while( cdbios_get_status() != 0 ){
//Sleep( 10 );
		;
	}

	FlushCache( 0 );

	/* ロード領域のキャッシュをクリア */
//	SyncDCache( buffer, buffer + size );

	normal_callback_read( buffer, pos, size, stageload_setup_callback, FS_SECTOR_SIZE );

	FS_CdStageSet( dirname );

	return stage_info;
}

int FS_LoadStageSync( void *info )
{
	int res;
	res = stageload_act( info );
	if( res == 0 && zlibdec_work->task_id >= 0 ){
		return 1;
	}
	return res;
}

void FS_LoadStageComplete( void *info )
{
	FS_STAGE_INFO *sinfo = info;

	DeleteSema( zlibdec_work->sem_id );

	GV_Free( sinfo );
	ASSERT( zlibdec_work->task_id == -1 );
	GV_Free( zlibdec_work );

#ifdef TIMECHECK
printf( "LOAD END TICK %d %d\n", DG_TickCount, DG_TickCount - start_tick_time );
#endif
	FS_CdStageProgBinFix();
}

void FS_LoadStagePreseek( char *dirname )
{
	if( cdbios_get_status() == 0 && FS_StreamSystemStatus() == 0 ){
#if 0
		int pos;
		pos = get_stage_pos( dirname );
		if( pos >= 0 ){
			cdbios_preseek( pos );
		}
#else
		int i;
		STAGE_FILE *sf;
		sf = &fs_stage[ FS_current_layer ];
		for( i = 0; i < sf->table_header.stagenum; i++ ){
			if( strcmp( sf->stage_table[ i ].name, dirname ) == 0 ){
				cdbios_preseek( sf->stage_table[ i ].offset );
			}
		}
#endif
	}
}

static int get_sound_section_top( int code )
{
	DATACNF_TAG *p;
	int pos;

	p = current_cnf->tags;
	pos = 0;
	for( ; !IS_END_TAG( p->id ); p++ ){
		if( p->id == ID_SOUND ){
			// 非圧縮セクション
			for( ; !IS_SECTION_END( p->id ); p++ ){
				if( code == p->id ){
					return ( pos + FS_SECTOR_ALIGN( current_cnf_size ) + p->offset )
						/ FS_SECTOR_SIZE;
				}
			}
		} else if( IS_SECTION_TAG( p->id ) && !IS_SECTION_END( p->id ) ){
			if( IS_COMPRESSION_TAG( ( p + 1 )->id ) ){
				p = p + 1;
				pos = p->offset + FS_SECTOR_ALIGN( COMPRESSION_SIZE( p->id ) );
			} else {
				pos += FS_SECTOR_ALIGN( p->offset );
			}
		}
	}
	return 0;
}

void FS_LoadSoundPak( int code )
{
	int top;

	top = get_sound_section_top( code );
	if( top > 0 ){
		sd_set_cli( 0xC0000000 | ( top + current_stage_top_pos ) );
	} else {
		printf( "SOUND CODE ERROR %d!!\n", code );
	}
}
