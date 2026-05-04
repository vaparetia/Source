//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	stream.c
		ストリーミング処理ルーチン
	2000/06/26	K.Uehara
	$Id: stream.c,v 1.18 2002/07/22 12:43:45 usr01475 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifrpc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sifdev.h>
#endif
#ifdef KP_XBOX
#include <xtl.h>
#endif

#if defined(BP_PS3)
#  include <sys/synchronization.h>
static sys_mutex_attribute_t s_mutexStreamAttr;
static sys_mutex_t s_mutexStreamId;
#elif defined(BP_360)
#  include <xtl.h>
static CRITICAL_SECTION stream_critical_section;
#elif defined(BP_VITA)
#  include <kernel/threadmgr.h>
static SceKernelLwMutexWork stream_critical_section;
#elif defined(BP_WIN32)
#else
#  error Unsupported platform.
#endif


#include "cdbios.h"
#include "libgv.h"
#include "libfs.h"
#include "stream.h"

#include "BP_TrophyLogicMGS2.h"
#include "BP_FileSupport.h"
#include "BP_RendererDebug.h"

#if BP_VITA
#  include "scebase_common.h"
#endif

void FS_StreamDump( FS_STREAM *h_info );

//BP
static void DI() {}
static void EI() {}

#define READ_TRIGGER_SIZE	( FS_SECTOR_SIZE * 32 )

/* ---------------------------------------------------------------------- */
/*
	管理構造体
*/

#define TAG_SIZE( _p )	FS_STREAM_TAG_SIZE(_p)
#define TAG_TYPE( _p )	FS_STREAM_TAG_TYPE(_p)
#define TAG_TICK( _p )	FS_STREAM_TAG_TIME(_p)
#define TAG_OPTION( _p ) FS_STREAM_TAG_OPTION(_p)

#define SET_TAG_TYPE( _p, _t ) FS_STREAM_TAG_TYPE_SET(_p,_t)
//#define SET_TAG_TYPE( _p, _t )	((_p)->type = (_t))
//#define NEXT_TAG( _p )	( typeof( _p ) )( ( char * )(_p) + TAG_SIZE(_p) )
#define NEXT_TAG( _p )	( STREAM_TAG * )( ( char * )(_p) + TAG_SIZE(_p) )

typedef struct {
	int status;
   int top_pos;      // BP added
	int read_pos;		// 読み取り開始セクタ
	void *buffer;
	int buffer_size;
	void *buffer_end;

	void *data_top;		// 有効データの先頭
	void *empty_top;	// 空き領域の先頭

	void *bottom_chank_address;	// 次のターンでバッファ先頭に転送する必要のある
								// chankの先頭アドレス
	
	void *read_end;				// 読み込みポインタ
	STREAM_TAG *loaded_tag;		// 読み込みを完了した最後のタグ
   int loaded_tag_swapped;

	int start_time;		// 読み込みを開始した時間

	int reading_flag;
	int read_end_flag;
	int force_stop_request;
	int getting_flag;
	int lock_count;

} STREAM_INFO;

static STREAM_INFO *current_info = NULL;

#ifdef PSX2

int gBP_Stream_ShowStats = 0;

void FS_StreamSystemInit( void )
{
#if 0//BP_TODO
	struct SemaParam sema;

	sema.initCount = 1;
	sema.maxCount = 1;

	stream_sema_id = CreateSema( &sema );
	ASSERT( stream_sema_id >= 0 );
#endif

#if defined(BP_PS3)
   sys_mutex_attribute_initialize( s_mutexStreamAttr );
   sys_mutex_create( &s_mutexStreamId, &s_mutexStreamAttr );
#elif defined(BP_360)
   InitializeCriticalSection(&stream_critical_section);
#elif defined(BP_VITA)
   int res = sceKernelCreateLwMutex(&stream_critical_section, "stream_mutex", 
      SCE_KERNEL_LW_MUTEX_ATTR_TH_PRIO | SCE_KERNEL_LW_MUTEX_ATTR_RECURSIVE, 0, NULL);
   ASSERT(res == SCE_OK);
#elif defined(BP_WIN32)

#else
#error unsupported platform.
#endif

}

#if defined(BP_PS3) //BP_TODO (threading support)
#define LOCK() sys_mutex_lock( s_mutexStreamId, 0 );
#define UNLOCK() sys_mutex_unlock( s_mutexStreamId );
#elif defined(BP_360)//BP_TODO (threading support)
#define LOCK() EnterCriticalSection(&stream_critical_section);
#define UNLOCK() LeaveCriticalSection(&stream_critical_section);
#elif defined(BP_VITA)
#define LOCK() sceKernelLockLwMutex(&stream_critical_section, 2, NULL);
#define UNLOCK() sceKernelUnlockLwMutex(&stream_critical_section, 2);
#else
#define LOCK()
#define UNLOCK()
#endif

#endif

#ifdef KP_XBOX
// TODO XBOX : 将来的にセマフォを導入する必要あるかも
#define LOCK()
#define UNLOCK()
#define iSyncDCache( a, b )
#define SyncDCache( a, b )
#define DI()
#define EI()
#endif

/* ---------------------------------------------------------------------- */
/*
	コールバック
*/

static inline int get_point_distance( STREAM_INFO *info, void *p1, void *p2 )
{
	int d;
	d = ( char * )p2 - ( char * )p1;
	if( d < 0 ){
		d += info->buffer_size;
	}
	return d;
}

static inline void read_stop( CDBIOS_READ_STATUS *rs )
{
	rs->total_size = rs->read_size;
}

static inline int copy_chank_buffer_top( STREAM_INFO *info, STREAM_TAG *p )
{
	int dist;
	dist = get_point_distance( info, p, info->data_top );
	if( dist == 0 ){
		dist = info->buffer_size;
	}
	if( get_point_distance( info, p, ( char * )info->buffer + TAG_SIZE( p ) )
		> dist ){
		/* 使用中 */

//printf( "ccbt: buffer %X buffer_end %X data_top %X empty_top %X bottom %X read_end %X loaded %X\n", info->buffer, info->buffer_end, info->data_top, info->empty_top, info->bottom_chank_address, info->read_end, info->loaded_tag );

		return -1;
	}
	/* チャンクをリングバッファ先頭に転送する */
	{
		u_long128 *s, *d;
		d = info->buffer;
		s = ( u_long128 * )p;
		while( ( void * )s < info->read_end ){
			*( d++ ) = *( s++ );
		}
		info->read_end = d;
	}
	/* 現在のチャンクのタイプを変更 */
	SET_TAG_TYPE( p, CHANK_TYPE_BUFEND );
	/* バッファの終端を再調整 */
	{
		int sz;
		sz = info->buffer_size
			- ( ( char * )info->read_end - ( char * )info->buffer );
		sz = ( sz / FS_SECTOR_SIZE ) * FS_SECTOR_SIZE;	// 切り捨て
		info->buffer_end = ( char * )info->read_end + sz;
		info->loaded_tag = info->buffer;
	}
	return 0;
}

static void stream_callback( CDBIOS_READ_STATUS *rs )
{
	STREAM_INFO *info = current_info;

	ASSERT( info != NULL );

	// バッファの末尾またはデータ終端に来たら止まる。

	// そこからバッファを埋めることもある。
	info->read_end = rs->ee_buffer;
	info->read_pos = rs->pos;
	if( info->force_stop_request || info->read_end_flag ){
		/* 強制停止 */
		read_stop( rs );
		goto READ_END;
	}

	/* 終端をチェックする */
	{
		STREAM_TAG *p;

		p = info->loaded_tag;
		for( ;; ){
			STREAM_TAG *next;
			next = NEXT_TAG( p );

			if( ( char * )next <= ( char * )info->read_end - sizeof( STREAM_TAG ) ){
				/* 現在読み込んだデータ内に次のチャンクのトップがある */
				p = NEXT_TAG( p );

				if( TAG_TYPE( p ) == CHANK_TYPE_END ){
					/* 終端タグを読み込んだ */
					info->read_end_flag = 1;
					info->loaded_tag = p;
					read_stop( rs );
					goto READ_END;
				}
			} else {
				/* 次のチャンクはこのデータ内にはない */
				if( ( void * )next > info->buffer_end
					|| ( char * )info->read_end + FS_SECTOR_SIZE
						   > ( char * )info->buffer_end ){
					/*
						このチャンクがリングバッファの終端からはみ出しているか
						次のリードがバッファ終端を越えそうだったら
						このチャンクをバッファ先頭に転送する
					*/
					if( copy_chank_buffer_top( info, p ) < 0 ){
						/* 使用中なので、次のリードスタート時に転送する */
						info->bottom_chank_address = p;
						read_stop( rs );
						goto READ_END;
					} else {
						/* 次の読み込みバッファアドレスを更新 */

						/* CPU転送を行なったので書き戻す */
						iSyncDCache( info->buffer, info->read_end );

						rs->ee_buffer = info->read_end;
						/*
							バッファ先頭から転送したタグの分だけ
							読み込みサイズを減らす
						*/
						rs->total_size -= FS_SECTOR_ALIGN( ( char * )info->read_end
														  - ( char * )info->buffer );
						if( rs->total_size < rs->read_size ){
							rs->total_size = rs->read_size;
						}
					}
				} else if( info->read_end >= info->buffer_end ){
					/* チャンクがちょうどバッファ終端で終った */
					void *buf_end;

					buf_end = ( char * )info->buffer + info->buffer_size;
					if( ( char * )info->read_end < ( char * )buf_end - sizeof( STREAM_TAG ) ){
						STREAM_TAG *tag = info->read_end;
						SET_TAG_TYPE( tag, CHANK_TYPE_BUFEND );
					}
					info->read_end = info->loaded_tag = rs->ee_buffer = info->buffer;
					info->buffer_end = buf_end;
				}
				break;
			}
			info->loaded_tag = p;
		}
	}
	if( rs->read_size < rs->total_size ){
		/* 継続する */
		iSyncDCache( rs->ee_buffer, rs->ee_buffer + 64 );
		return;
	}
#if 0
	if( get_point_distance( info, info->read_end, info->data_top ) > READ_TRIGGER_SIZE ){
		/* まだよみこむ？ */
//		set_callback_read( rs );
	}
#endif
	/* 読み込み終了 */
READ_END:
	info->reading_flag = 0;
	current_info = NULL;
}

static int read_start( STREAM_INFO *info )
{
	/* CDが停止状態であること */

	if( cdbios_get_status() != 0 ){
		return 0;
	}

	if( info->bottom_chank_address != NULL ){
		if( copy_chank_buffer_top( info, ( STREAM_TAG * )info->bottom_chank_address ) < 0 ){
			/* まだ先頭バッファが使用中 */
			return -1;
		}

		SyncDCache( info->buffer, info->read_end );
		
		info->bottom_chank_address = NULL;
	}

	{
		int dist, intr;

		dist = ( char * )info->data_top - ( char * )info->read_end;
		if( dist < 0 ){
			dist += ( char * )info->buffer_end - ( char * )info->buffer;
		}
		dist = ( dist / FS_SECTOR_SIZE ) * FS_SECTOR_SIZE;

		if( ( char * )info->read_end + dist >= ( char * )info->buffer_end ){
			intr = ( char * )info->buffer_end - ( char * )info->read_end;
			SyncDCache( info->read_end, info->read_end + intr );
		} else {
			intr = -1;
			SyncDCache( info->read_end, info->read_end + dist );
		}
		current_info = info;
//printf( "read %x %x %x %x %x\n", info->read_end, info->read_pos, dist, stream_callback, intr );
		info->reading_flag = 1;
		cdbios_callback_read( info->read_end, info->read_pos, dist, stream_callback, intr, ( info->top_pos & 0xffffff ) * FS_SECTOR_SIZE );
	}
	
	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	メインタスクのルーチン
*/

static void StreamSetBuffer( STREAM_INFO *info, void *buftop, int bufsize )
{
	info->buffer = buftop;
	info->buffer_size = bufsize;
}

static void StreamStart( STREAM_INFO *info, int pos )
{
	// ストリーミング開始
//	ASSERT( info->status == FS_STR_STOP );
	info->read_end_flag = 0;
	info->force_stop_request = 0;
	info->buffer_end = ( char * )info->buffer + info->buffer_size;
	info->bottom_chank_address = NULL;
	info->read_end = info->loaded_tag = info->data_top = info->buffer;

   info->top_pos = pos;
	info->read_pos = pos;
	info->getting_flag = 0;
	info->lock_count = 0;

	info->status = FS_STR_START;

	current_info = info;
	info->reading_flag = 1;
//printf( "Read %x %x %x %x %x\n", info->read_end, info->read_pos, info->buffer_size, stream_callback, -1 );

	/* 初期リード開始 */
	SyncDCache( info->read_end, info->read_end + info->buffer_size );
	cdbios_callback_read( info->read_end, info->read_pos, info->buffer_size, stream_callback, -1, ( info->top_pos & 0xffffff ) * FS_SECTOR_SIZE );
   bp_trophy_begin_stream( pos );
}

static int StreamSync( STREAM_INFO *info )
{
	/* バッファの整理, data_topの更新、ロードトリガー */

	STREAM_TAG *p;
	int read_status, end_flag;

	if( info->getting_flag ){
		/*
			他タスクでGetDataしている途中
			データ読み込みトリガーがかかるとまずいので
			すぐに終了する
		*/
		return 1;
	}

	/* 割り込みで不具合が起こらないようこの時点の状態を保存 */
	read_status = cdbios_get_status();
	end_flag = info->read_end_flag;

	if( end_flag || info->force_stop_request ){
		info->status = FS_STR_STOP;
		return 0;
	}

	if( info->status == FS_STR_START ){
		/* 初期読み込み状態 */
		if( read_status != 0 ) return 1;
		info->empty_top = info->buffer;
		info->status = FS_STR_RUNNING;
	}

	p = ( STREAM_TAG * )info->data_top;

	while( p != info->loaded_tag ){
		int type;
		type = TAG_TYPE( p );

		if( type == CHANK_TYPE_BUFEND ){
			p = info->buffer;
			continue;
		} else if( type != CHANK_TYPE_FREE ){
			break;
		}
		p = NEXT_TAG( p );
		if( ( char * )p >= ( char * )info->buffer + info->buffer_size ){
			p = info->buffer;
		}
	}

	info->data_top = p;

	if( read_status == 0 && end_flag == 0 ){
		if( info->buffer_size - get_point_distance( info, info->data_top, info->read_end ) > info->buffer_size * 1 / 3 ){
			read_start( info );
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	外から呼ばれるルーチン
	void *でハンドラを渡す。
*/

#define MAX_STREAM 2

static STREAM_INFO stream_info[ MAX_STREAM ];

static STREAM_INFO *get_stream_info( void )
{
	int i;

	for( i = 0; i < MAX_STREAM; i++ ){
		if( stream_info[ i ].buffer == NULL ){
			return &stream_info[ i ];
		}
	}
	ASSERT( FALSE );
	return NULL;
}

FS_STREAM *FS_Stream( int pos, void *buffer, int size, int flag )
{
	STREAM_INFO *info;

	info = get_stream_info();
	StreamSetBuffer( info, buffer, size );
	StreamStart( info, pos );

	return ( FS_STREAM * )info;
}

void FS_StreamClose( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	info->buffer = NULL;

   bp_trophy_end_stream( info->top_pos );
}

int FS_StreamSync( FS_STREAM *h_info )
{
	int res;

	LOCK();
	res = StreamSync( h_info );
	UNLOCK();

	return res;
}

void *FS_StreamGetData( FS_STREAM *h_info, int req_type )
{
	STREAM_INFO *info = h_info;
	STREAM_TAG *p;
	void *res;
	int getf;

	LOCK();

	if( info->force_stop_request ){
		res = NULL;
		goto END;
	}

	/*
		複数のタスクから呼び出される可能性があるため、
		SYNCでデータが書き換えられないようLOCKをかける
	*/
	
	getf = info->getting_flag;
	info->getting_flag = 1;

	res = NULL;

	p = info->data_top;

	while( p != info->loaded_tag ){
		int type;
		type = TAG_TYPE( p );

		if( type == CHANK_TYPE_BUFEND ){
			p = info->buffer;
			continue;
		} else if( type == req_type ){
			SET_TAG_TYPE( p, type | CHANK_TYPE_USE );

			res = ( void * )( p + 1 );
			break;
		}
		p = NEXT_TAG( p );
		if( ( char * )p >= ( char * )info->buffer + info->buffer_size ){
			p = info->buffer;
		}
	}

	info->getting_flag = getf;
END:
	UNLOCK();

	return res;
}

void FS_StreamUngetData( FS_STREAM *h_info, void *data )
{
	STREAM_TAG *p;
	int type;

	LOCK();
	p = ( STREAM_TAG * )data - 1;
	type = TAG_TYPE( p );
	SET_TAG_TYPE( p, type & ~CHANK_TYPE_USE );
	UNLOCK();
}

#if 0
int FS_StreamGetNextType( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;
	STREAM_TAG *p;
	int type;

	if( info->force_stop_request ){
		return CHANK_TYPE_END;
	}
	p = info->data_top;

	type = TAG_TYPE( p );

	if( type == CHANK_TYPE_BUFEND ){
		p = info->buffer;
		type = TAG_TYPE( p );
	}
	return type;
}
#endif

void FS_StreamFreeData( FS_STREAM *h_info, void *data )
{
	/* データ領域の解放 */
	STREAM_TAG *p;

	LOCK();
	p = ( STREAM_TAG * )data - 1;
	SET_TAG_TYPE( p, CHANK_TYPE_FREE );
	UNLOCK();
}

int FS_StreamGetSystemData( FS_STREAM *h_info )
{
	int type;
	STREAM_TAG *p;


	if( ( p = FS_StreamGetData( h_info, CHANK_TYPE_SYSTEM ) ) == NULL ){
		return CHANK_TYPE_FREE;
	}
   type = FS_STREAM_TAG_OPTION( p - 1 );
	FS_StreamFreeData( h_info, p );

	return type;
}

int FS_StreamGetSize( FS_STREAM *h_info, void *ptr )
{
	STREAM_TAG *p = ( STREAM_TAG * )ptr - 1;

	return TAG_SIZE( p ) - sizeof( STREAM_TAG );
}

void FS_StreamStop( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	if( info == NULL ) return;

	LOCK();

	info->force_stop_request = 1;
	info->read_end_flag = 1;
	info->status = FS_STR_STOP;

	if( info->reading_flag && cdbios_get_status() != 0 ){
		cdbios_stop();
	}

	UNLOCK();
}

int FS_StreamGetStatus( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	if( info == NULL ) return FS_STR_STOP;

	return info->status;
}

int FS_StreamSystemStatus( void )
{
	return ( cdbios_get_status() || ( current_info != NULL ) ) ? 1 : 0;
}

int FS_StreamIsReading( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	if( cdbios_get_status() == 0 ){
		info->reading_flag = 0;
		current_info = NULL;
	}

	return info->reading_flag;
}

void FS_StreamLock( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	DI();

	info->lock_count ++;

	EI();
}

void FS_StreamUnlock( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;

	DI();

	info->lock_count --;

	EI();
}

int FS_StreamIsLocked( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;
	return info->lock_count;
}

#ifndef GOLD_VERSION //BP def DEBUG_MODE
void FS_StreamDump( FS_STREAM *h_info )
{
	STREAM_INFO *info = h_info;
	STREAM_TAG *p;

	p = info->data_top;
printf( "StreamDump = %X %X\n", p, info->loaded_tag );
	while( p != info->loaded_tag ){
		int type;
		type = TAG_TYPE( p );

		if( type == CHANK_TYPE_BUFEND ){
			p = info->buffer;
			continue;
		}
		printf( "%08X TYPE %02X tick %04X size %04X %d\n", p, TAG_TYPE( p )
         , TAG_TICK( p ), TAG_SIZE( p ), FS_STREAM_TAG_OPTION(p) );
		p = NEXT_TAG( p );
		if( ( char * )p >= ( char * )info->buffer + info->buffer_size ){
			p = info->buffer;
		}
	}
}

void BP_InitStreamDebugMenu()
{
   BP_DebugMenu_AddBool( -1, "Show Stream Stats", &gBP_Stream_ShowStats );
}

void FS_StreamDebugPrint( FS_STREAM *h_info )
{
   if( gBP_Stream_ShowStats )
   {
      int packetCount[0xff+1] = { 0 };
      int packetSize[0xff+1] = { 0 };
      int totalPacketCount = 0;
      int totalPacketSize = 0;
      int totalNonFreePacketSize = 0;
      STREAM_INFO *info = h_info;
      STREAM_TAG *p;
      const char * stream_name;

      p = info->data_top;
      while( p != info->loaded_tag ){
         int type;
         type = TAG_TYPE( p );

         if( type == CHANK_TYPE_BUFEND ){
            p = info->buffer;
            continue;
         }

         packetCount[type & 0xff]++;
         packetSize[type & 0xff] += TAG_SIZE(p);
         totalPacketCount++;
         totalPacketSize+= TAG_SIZE(p);
         if( type != CHANK_TYPE_FREE )
         {
            totalNonFreePacketSize += TAG_SIZE(p);
         }

         p = NEXT_TAG( p );
         if( ( char * )p >= ( char * )info->buffer + info->buffer_size ){
            p = info->buffer;
         }
      }

      if(stream_name = BP_FindStreamName(info->top_pos))
      {
         BP_DebugText_Print("Stream Name: %s", stream_name );
      }

      {
         float packageSizePercentage;
         packageSizePercentage = (float)packetSize[CHANK_TYPE_PCM] / totalNonFreePacketSize;
         BP_DebugText_Print("  PCM: %7d (%.6f) ", packetSize[CHANK_TYPE_PCM], packageSizePercentage );
         packageSizePercentage = (float)packetSize[CHANK_TYPE_DEMO] / totalNonFreePacketSize;
         BP_DebugText_Print(" DEMO: %7d (%.6f) ", packetSize[CHANK_TYPE_DEMO], packageSizePercentage );
         packageSizePercentage = (float)packetSize[CHANK_TYPE_CAPTION] / totalNonFreePacketSize;
         BP_DebugText_Print("  CAP: %7d (%.6f) ", packetSize[CHANK_TYPE_CAPTION], packageSizePercentage );
         packageSizePercentage = (float)packetSize[CHANK_TYPE_MOTION] / totalNonFreePacketSize;
         BP_DebugText_Print("MOT'N: %7d (%.6f) ", packetSize[CHANK_TYPE_MOTION], packageSizePercentage );
         packageSizePercentage = (float)packetSize[CHUNK_TYPE_MOVIE] / totalNonFreePacketSize;
         BP_DebugText_Print("MOVIE: %7d (%.6f) ", packetSize[CHUNK_TYPE_MOVIE], packageSizePercentage );
      }
   }
}

#endif

//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)
const char* FS_StreamGetName( FS_STREAM* h_info )
{
   STREAM_INFO* info = h_info;
   return BP_FindStreamName( info->top_pos );
}
//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)
