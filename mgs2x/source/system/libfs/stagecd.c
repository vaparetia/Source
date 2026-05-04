/*
	stagecd.c
		CD ファイルシステム用ステージデータロードルーチン

	# 本来はGMが担当すべきモジュールだが、HD、CDの切り分けを
	# 行うために、FSに置く。

	2000/06/06 K.Uehara
	$Id: stagecd.c,v 1.1.1.3 2002/11/19 11:42:40 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cdbios.h"

#include "libgv.h"
#include "libfs.h"

#include "sd_ee.h"

/* ---------------------------------------------------------------------- */
/*
	ステージファイル管理情報を読み込んで初期化。
*/

typedef struct {
	int version;
	int create_time;
	int stagenum;
	int size;
} STAGE_TABLE_HEADER;

#define STAGE_NAME_LEN	8

typedef struct {
	char name[ STAGE_NAME_LEN ];
	int offset;
} STAGE_TABLE;

static int taskid;
static STAGE_TABLE_HEADER fs_table_header;
static STAGE_TABLE *stage_table;

static void init_callback( CDBIOS_READ_STATUS *rs )
{
	if( rs->read_size == FS_SECTOR_SIZE ){
		STAGE_TABLE_HEADER *table;
		// 1回め
		table = rs->ee_buffer - FS_SECTOR_SIZE;
		memcpy( &fs_table_header, table, sizeof( STAGE_TABLE_HEADER ) );

		rs->total_size = FS_SECTOR_ALIGN( table->size );
	}
	if( rs->total_size == rs->read_size ){
		// 終了
		BP_iWakeupThread( taskid );
	}
}

void FS_CdStageFileInit( void )
{
	void *buffer;

	int maxsize = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY );

	taskid = BP_GetThreadId();

	buffer = GV_Malloc( maxsize );

	SyncDCache( buffer, buffer + maxsize );

	cdbios_callback_read( buffer, FS_GET_FILE_POS( FS_FILE_STAGE ), maxsize
								 , init_callback, FS_SECTOR_SIZE );
	BP_SleepThread();
	printf( "%d STAGES\n", fs_table_header.stagenum );

	stage_table = GV_AllocResidentMemory( fs_table_header.stagenum * sizeof( STAGE_TABLE )
										  , 0 );
	{
		int i;
		int top;
		STAGE_TABLE *s;

		s = buffer + sizeof( STAGE_TABLE_HEADER );
		top = FS_GET_FILE_POS( FS_FILE_STAGE );

		for( i = 0; i < fs_table_header.stagenum; i++ ){
			memcpy( stage_table[ i ].name, s[ i ].name, STAGE_NAME_LEN );
			stage_table[ i ].offset = s[ i ].offset + top;
		}
	}
	GV_Free( buffer );
}

static int get_stage_pos( char *name )
{
	int i;
	for( i = 0; i < fs_table_header.stagenum; i++ ){
		if( strcmp( stage_table[ i ].name, name ) == 0 ){
			return stage_table[ i ].offset;
		}
	}
	ASSERT( FALSE );
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
	IN_STEP_INITIALIZE,			/* セクション初期化中 */
	IN_STEP_INITIALIZE_LAST,	/* 最後のセクションの初期化中 */
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

} FS_STAGE_INFO;

#define ID_SECTION_END	0x7F000000
#define ID_NOCACHE		0x7F000001
#define ID_CACHE		0x7F000002
#define ID_RESIDENT		0x7F000003
#define ID_SOUND		0x7F000010
#define ID_BINARY		0x7F010000
#define ID_BLOCK( no )	( 0x7F000100 | (no) )
#define IS_SECTION_TAG( a )	( ( (a)&0xFF000000 ) == 0x7F000000 )
#define IS_SECTION_END( a )	( (a) == ID_SECTION_END )
#define IS_PRELOAD_TAG( a )	( ( (a) & ~0xFF ) != ID_BLOCK(0) )
#define IS_END_TAG( a )		( (a) == 0 )

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

#if 0
/* ---------------------------------------------------------------------- */
/*
	共通
*/

static DATACNF_TAG *get_next_section( DATACNF_TAG *p )
{
	for( ; ! IS_SECTION_TAG( p->id ) && p->id != 0; p++ ){
		;
	}
	return p;
}

/* ---------------------------------------------------------------------- */
/*
	コールバック
*/

/*
	読み込むべきサイズを確定
*/
static int get_stageload_size( DATACNF *cnf, int *binary_size )
{
	int i;
	int size;
	int binsize;

	size = 0;
	binsize = 0;
	for( i = 0; i < cnf->tagnum; i++ ){
		int id;
		id = cnf->tags[ i ].id;
		if( IS_SECTION_TAG( id ) && IS_PRELOAD_TAG( id ) ){
			if( id == ID_BINARY ){
				binsize += cnf->tags[ i ].offset;
			}
			size += cnf->tags[ i ].offset;
		}
	}
	if( binary_size != NULL ){
		*binary_size = binsize;
	}
	return size;
}

#endif

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

	if( !IS_SECTION_TAG( id ) ){
		scePrintf( "SETUP_SECTION_LOAD_ERROR %X\n", id );
		HANGUP();
	}

	info->read_buf_top = info->next_read_buf_top;
	rs->ee_buffer = info->next_read_buf_top;
	rs->read_size = 0;
	rs->total_size = tag->offset;
	rs->intr_size = -1;

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
			current_cnf = info->datacnf = info->buffertop;
			tagnum = info->tagnum = info->datacnf->tagnum;

//			scePrintf( "tagnum %d\n", tagnum );
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
			// mkcdimg BUG回避
			if( info->cnf_p->id == ID_SECTION_END ){
				info->cnf_p++;
			}
			
			// DATACNF読み込み完了
			// BINARY block なら継続して読み込む
			if( info->cnf_p->id == ID_BINARY ){
				extern int _mgs2_keep_end[];

				rs->ee_buffer = _mgs2_keep_end;
				rs->read_size = 0;
				rs->total_size = info->cnf_p->offset;
				rs->intr_size = -1;

				info->next_read_section_p = NULL;
				info->read_section_p = NULL;
				info->callback_step = CB_STEP_SECTION_LOAD;

				iSyncDCache( rs->ee_buffer, rs->ee_buffer + info->cnf_p->offset );
			} else {
				// 次のトリガーがかかるまでスリープ
				info->callback_step = CB_STEP_SLEEP;
				// 読み込み完了
				read_stop( rs );
			}
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
	printf( "setup_next:MALLOC %d\n", size );
	buf = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size, 128 );
	ASSERT( buf != NULL );
	info->next_read_buf_top = buf;

	SyncDCache( buf, buf + size );

	return buf;
}

static inline int stageload_act( FS_STAGE_INFO *info )
{
	switch( info->init_step ){
	  case IN_STEP_STARTUP:
		{
			int size;

			if( info->callback_step <= CB_STEP_DATACNF_LOAD ){
				break;
			}
			size = info->cnf_size;
			/* 各セクションに対応したテーブルを作成する */
			{
				SECTION_INFO *sinfo;
				int i, n, pos;
				DATACNF_TAG *tag;

				sinfo = ( info->buffertop + info->cnf_size );
				info->section_info = sinfo;
				n = 0;
				tag = info->datacnf->tags;
				pos = FS_SECTOR_ALIGN( size );
				for( i = 0; i < info->tagnum; i++ ){
					if( IS_SECTION_TAG( tag->id ) && tag->id != ID_SECTION_END ){
printf( "SECTION %x\n", tag->id );
						if( is_load_section( tag->id ) ){
							// バイナリタグ以外を設定
							sinfo->section_tag = tag;
							sinfo->section_addr = NULL;
							sinfo->section_top = pos / FS_SECTOR_SIZE;
							pos += FS_SECTOR_ALIGN( tag->offset );
							sinfo++;
							n++;
						}
					}
					tag++;
				}
				sinfo->section_tag = NULL;
				sinfo->section_addr = NULL;
				n++;
				size = size + sizeof( SECTION_INFO ) * n;
			}
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
printf( "section read end\n" );
				info->init_step = IN_STEP_INITIALIZE_LAST;
				info->next_read_section_p = section;
				if( info->callback_step == CB_STEP_SLEEP ){
					info->read_section_p = section;
				}
			} else {
				/* セクション読み込みのセットアップ */
printf( "setup section %x\n", section->section_tag->id );
				section->section_addr = setup_next_section( info, section->section_tag );
				if( info->callback_step == CB_STEP_SLEEP ){
					info->callback_step = CB_STEP_SECTION_LOAD;
					info->read_section_p = section;
					info->next_read_section_p = NULL;
					cdbios_callback_read( info->next_read_buf_top
						, info->stage_top_pos + section->section_top
						, section->section_tag->offset
						, stageload_setup_callback, -1 );
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

			if( section == info->read_section_p ){
				/* このセクションは読み込み中 */
				int read_size;
				read_size = cdbios_get_read_size();
				if( read_size <= ( cnf + 1 )->offset ){
					/* 読み込みが完了していない */
					break;
				}
			}
			// 各データの初期化
			datatop = section->section_addr + cnf->offset;
			if( info->init_type == GV_INIT_RESIDENT ){
				// RESIDENTは、常駐領域にコピーして初期化する
				int size;
				void *p;
				
				size = ( cnf + 1 )->offset - cnf->offset;
				p = GV_AllocResidentMemory( size, cnf->id );
				memcpy( p, datatop, size );
				datatop = p;
			}
			printf( "Loadinit %X %X\n", datatop, cnf->id );
			stat = GV_LoadInit( datatop, cnf->id, info->init_type );
			ASSERT( stat > 0 );
			cnf ++;

			if( IS_SECTION_END( cnf->id ) ) {
				/* 次のセクションの初期化へ移る */
				DATACNF_TAG *isec;
				info->init_section_p++;
				if( ( isec = info->init_section_p->section_tag ) == NULL ){
					return 0;		// init完了
				}
				info->init_type = section_init_type( isec->id );
				cnf = isec + 1;
			}
			info->cnf_p = cnf;
		}
		break;
	}
	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	外部からの呼び出し
*/

void *FS_LoadStageRequest( char *dirname )
{
	int size;
	int pos;
	char *buffer;

	stage_info = GV_Malloc( sizeof( FS_STAGE_INFO ) );
	GV_ZeroMemory( stage_info, sizeof( FS_STAGE_INFO ) );

	stage_info->callback_step = CB_STEP_STARTUP;
	stage_info->init_step = IN_STEP_STARTUP;

	/* ロードバッファとしてとれるだけのメモリを確保 */
	/* 64 バイトアライメントにのるように確保 */
	size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY );
	buffer = stage_info->buffertop = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size - 64, 64 );

	/* 読み込むセクタ位置を取得 */
	pos = get_stage_pos( dirname );
	current_stage_top_pos = stage_info->stage_top_pos = pos;

	/* ロード領域のキャッシュをクリア */
	SyncDCache( buffer, buffer + size );

	/* 念のため */
	while( cdbios_get_status() != 0 ){
		;
	}

printf( "LOAD START size = %d pos %d\n", size, pos );
	cdbios_callback_read( buffer, pos, size, stageload_setup_callback, FS_SECTOR_SIZE );

	FS_CdStageSet( dirname );

	return stage_info;
}

int FS_LoadStageSync( void *info )
{
	return stageload_act( info );
}

void FS_LoadStageComplete( void *info )
{
	FS_STAGE_INFO *sinfo = info;
	GV_Free( sinfo );

	FS_CdStageProgBinFix();
}

void FS_LoadStagePreseek( char *dirname )
{
	cdbios_preseek( get_stage_pos( dirname ) );
}

static int get_sound_section_top( int code )
{
	DATACNF_TAG *p;
	int pos;

	p = current_cnf->tags;
	pos = FS_SECTOR_ALIGN( current_cnf_size );
	for( ; !IS_END_TAG( p->id ); p++){
//printf( "id %x ofs %x\n", p->id, p->offset );
		if( p->id == ID_SOUND ){
			for( ; !IS_SECTION_END( p->id ); p++ ){
				if( code == p->id ){
					return ( pos + p->offset ) / FS_SECTOR_SIZE;
				}
			}
		}
		if( IS_SECTION_END( p->id ) ){
			pos += FS_SECTOR_ALIGN( p->offset );
printf( "add %x = %x\n", p->offset, pos );
		}
	}
	return 0;
}

void FS_LoadSoundPak( int code )
{
	int top;
printf( "FS_LOADSOUNDPAK %d\n", code );
	top = get_sound_section_top( code );
	if( top > 0 ){
printf( "top = %d current = %d\n", top, current_stage_top_pos );
		sd_set_cli( 0xC0000000 | ( top + current_stage_top_pos ) );
	} else {
		printf( "SOUND CODE ERROR %d!!\n", code );
	}
}
