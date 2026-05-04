/*
	stream.h
		ストリーミング処理ルーチンヘッダ

	2000/06/26	K.Uehara
	$Id: stream.h,v 1.1.1.3 2002/11/19 11:42:41 Yoshizawa1 Exp $
*/

#include "BP_EndianSupport.h"

typedef void FS_STREAM;

typedef struct {
	int type_le;
	int size_le;
	int time_le;
	int option_le;
} STREAM_TAG;

#define FS_STREAM_TAG_TYPE(tag) BP_LE_SwapSInt( (tag)->type_le )
#define FS_STREAM_TAG_TYPE_SET(tag,type) (tag)->type_le = BP_LE_SwapSInt( (type) )
#define FS_STREAM_TAG_SIZE(tag) BP_LE_SwapSInt( (tag)->size_le )
#define FS_STREAM_TAG_TIME(tag) BP_LE_SwapSInt( (tag)->time_le )
#define FS_STREAM_TAG_OPTION(tag) BP_LE_SwapSInt( (tag)->option_le )

enum {
	FS_STR_START = -1,
	FS_STR_STOP = 0,
	FS_STR_RUNNING = 1,
};

enum {
	CHANK_TYPE_FREE = 0,		/* 使用終了 */

	CHANK_TYPE_PCM = 1,				/* 音声データ */
	CHANK_TYPE_DEMO = 2,			/* デモデータ */
	CHANK_TYPE_LIP	= 3,			/* 口パクデータ */
	CHANK_TYPE_CAPTION = 4,			/* 字幕データ */
	CHANK_TYPE_MOTION = 5,			/* モーションデータ */
	CHANK_TYPE_CODEC_CAP = 6,		/* 無線用字幕データ */

	CHANK_TYPE_CRI_MPEG	= 0x0C,			/* XBOX CRI MPEG Stream */
	CHANK_TYPE_CRI_IPIC	= 0x0D,			/* XBOX CRI Ipicture Stream */

	CHANK_TYPE_MPEG2_VIDEO = 0x0E,	/* MPEG2 VIDEO Stream */
	CHANK_TYPE_IPU = 0x0F,			/* IPU Stream */

   // BP custom chunks
   CHUNK_TYPE_MOVIE = 0x20,

	CHANK_TYPE_SYSTEM = 0x10,		/* システムデータ */

	CHANK_TYPE_USE = 0x80,		/* ストリーミングデータ転送中 */
	CHANK_TYPE_END = 0xf0,		/* このストリームデータの終端 */
	CHANK_TYPE_BUFEND = 0xff	/* バッファ終端 */
};

FS_STREAM *FS_Stream( int pos, void *buffer, int size, int flag );
void FS_StreamClose( FS_STREAM *h_info );
int FS_StreamSync( FS_STREAM *h_info );
void FS_StreamStop( FS_STREAM *h_info );
void *FS_StreamGetData( FS_STREAM *h_info, int req_type );
void FS_StreamUngetData( FS_STREAM *h_info, void *data );
int FS_StreamGetNextType( FS_STREAM *h_info );
void FS_StreamFreeData( FS_STREAM *h_info, void *data );
int FS_StreamGetSystemData( FS_STREAM *h_info );
int FS_StreamGetSize( FS_STREAM *h_info, void *ptr );
int FS_StreamGetStatus( FS_STREAM *h_info );
int FS_StreamSystemStatus( void );	// CDが読み取り中かどうか
int FS_StreamIsReading( FS_STREAM *h_info );// このstreamが読み取り中かどうか

void FS_StreamLock( FS_STREAM *h_info );
void FS_StreamUnlock( FS_STREAM *h_info );
int FS_StreamIsLocked( FS_STREAM *h_info );

#ifdef DEBUG_MODE
void FS_StreamDump( FS_STREAM *h_info );
#endif //DEBUG_MODE

//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)
const char* FS_StreamGetName( FS_STREAM* h_info );
//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)

#define FS_STREAM_GET_TIME( ptr )	( FS_STREAM_TAG_TIME( ( STREAM_TAG * )(ptr) - 1 ) )
#define FS_STREAM_GET_SIZE( ptr ) \
   ( FS_STREAM_TAG_SIZE( ( STREAM_TAG * )(ptr) - 1 ) - sizeof( STREAM_TAG ) )
#define FS_STREAM_GET_OPTION( ptr )	( FS_STREAM_TAG_OPTION( ( STREAM_TAG * )(ptr) - 1 ) )


