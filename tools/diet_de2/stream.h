/*
	stream.h
		ストリーミング処理ルーチンヘッダ

	2000/06/26	K.Uehara
	$Id: stream.h,v 1.1 2000/09/21 06:53:34 usr02774 Exp $
*/

typedef void FS_STREAM;

typedef struct {
	int type;
	int size;
	int time;
	int option;
} STREAM_TAG;

enum {
	FS_STR_START = -1,
	FS_STR_STOP = 0,
	FS_STR_RUNNING = 1,
};

enum {
	CHANK_TYPE_FREE = 0,		/* 使用終了 */
	CHANK_TYPE_PCM = 1,				/* 音声データ */
	CHANK_TYPE_PCM_HEADER = 2,		/* 音声データヘッダ */
	CHANK_TYPE_JIMAKU = 3,			/* 字幕、口パクデータ */
	CHANK_TYPE_JIMAKU_HEADER = 4,	/* 字幕ヘッダ */
	CHANK_TYPE_DEMO = 5,			/* デモ用バイナリデータ */
	CHANK_TYPE_SYSTEM = 0x10,	/* システムデータ */
	CHANK_TYPE_USE = 0x80,		/* ストリーミングデータ転送中 */
	CHANK_TYPE_END = 0xf0,		/* このストリームデータの終端 */
	CHANK_TYPE_BUFEND = 0xff	/* バッファ終端 */
};

FS_STREAM *FS_Stream( int pos, void *buffer, int size, int flag );
void FS_StreamClose( FS_STREAM *h_info );
int FS_StreamSync( FS_STREAM *h_info );
void *FS_StreamGetData( FS_STREAM *h_info, int req_type );
int FS_StreamGetNextType( FS_STREAM *h_info );
void FS_StreamFreeData( FS_STREAM *h_info, void *data );
int FS_StreamGetSystemData( FS_STREAM *h_info );
int FS_StreamGetSize( FS_STREAM *h_info, void *ptr );
int FS_StreamGetStatus( FS_STREAM *h_info );
int FS_StreamSystemStatus( void );	// CDが読み取り中かどうか

#ifdef DEBUG_MODE
void FS_StreamDump( FS_STREAM *h_info );
#endif //DEBUG_MODE

#define FS_STREAM_GET_TIME( ptr )	( ( ( STREAM_TAG * )(ptr) - 1 )->time )
#define FS_STREAM_GET_SIZE( ptr ) \
	( ( ( STREAM_TAG * )(ptr) - 1 )->size - sizeof( STREAM_TAG ) )
#define FS_STREAM_GET_OPTION( ptr )	( ( ( STREAM_TAG * )(ptr) - 1 )->option )


