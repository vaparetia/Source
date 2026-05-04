/*
	ストリーミング用ドライバ共通ヘッダ

	2000/07/14 K.Uehara
	$Id: strctrl.h,v 1.1.1.3 2002/11/19 11:41:56 Yoshizawa1 Exp $
*/

#include "libgv.cnf"

/* ---------------------------------------------------------------------- */
/*
	コンフィグレーション
*/

#define VOX_BUFFER_SIZE		(STREAM_BUFFER_SIZE/4)
#define DEMO_BUFFER_SIZE	(STREAM_BUFFER_SIZE)
#define MOVIE_BUFFER_SIZE	(STREAM_BUFFER_SIZE)

/* ---------------------------------------------------------------------- */
/*
	内部使用フラグ
*/

#define TAGFLAG_SOUND_VAG	0x20000000
#define TAGFLAG_SOUND_8BIT	0x40000000
#define TAGFLAG_SOUND_AC3	0x10000000

#define TAGFLAG_SOUND_EXIST	( TAGFLAG_SOUND_VAG | TAGFLAG_SOUND_8BIT | TAGFLAG_SOUND_AC3 )

#define STREAM_TYPE_MSF 0x00030001
#define STREAM_TYPE_XAUDIO 0x00040001
#define STREAM_TYPE_VITA_AT9 0x00050001

/* ---------------------------------------------------------------------- */
/*
	ストリーミングコントローラがドライバに公開する情報
*/

struct GM_STREAM_CONTROL_s {
	int id;
	int state;
	int tick;
	void *stream_h;

	short caption_prio;
	short sd_channel;
	int sd_duration;
	int sd_tick;
	unsigned int flag;
   unsigned int tagflag;//BP added
#ifdef KP_XBOX
	u_char	hnsd3d;		// 音声３Ｄ化のためのハンドル
#endif
   int   top_pos;       //BP added
};

typedef struct GM_STREAM_CONTROL_s GM_STREAM_CONTROL;

#define STR_ACTION_PROC( ctrl )		( ctrl->flag >> 8 )
#define STR_FLAG( ctrl )			( ctrl->flag )

/* ---------------------------------------------------------------------- */
/*
	各ストリーミングドライバが登録する管理構造体
*/

/* 各ドライバは、strctrlの子として起動される */
/* workは、GV_ACTOR_EXとすること */

typedef struct _str_control{
	struct _str_control *next;			// 管理用
	int type;							// ドライバが管理するタイプ
	void *(*driver)( GM_STREAM_CONTROL *ctrl, int type );	// ドライバの初期化関数
} GM_STREAM_DRIVER;


EXTERN_INLINE int GM_IS_STREAM_END( GM_STREAM_CONTROL *ctrl )
{
	return ( ctrl->state == GM_STREAM_STATE_READ_END );
}


/* ストリーミングドライバを登録/削除 */
void GM_StreamAddDriver( GM_STREAM_DRIVER *driver, int type );
void GM_StreamRemoveDriver( GM_STREAM_DRIVER *driver );

/* ダミー用ドライバ */

void *NewStreamDummyDriver( GM_STREAM_CONTROL *ctrl, int type );
void GM_StreamDummyAct( void *work );

/* ------------------------------------ */

/* ドライバ初期化関数が返すマクロ／正常終了はworkへのポインタを返す */

#define GM_STREAM_DRIVER_NO_USE		NULL			// 使用しない/dummyを登録
#define GM_STREAM_DRIVER_NO_LINK	( (void *)1 )	// childにリンクしない

/* ---------------------------------------------------------------------- */
/*
	標準のドライバ
*/

/*
	 サウンド
*/
int GM_StreamSdDriverInit( void );
void GM_StreamSdSetPan( int channel, int volpan, float bp_angle );

#ifdef DEBUG_MODE
void GM_StreamSdCancel( void );
#endif

/* サウンドはこれを内部的に使って空きチャンネルを得る */

int GM_StreamSdGetChannel( void );
int GM_StreamSdGetChannelNo( int no );
void GM_StreamSdFreeChannel( int channel );
void GM_StreamSdPauseOn( GM_STREAM_CONTROL *ctrl );
void GM_StreamSdPauseOff( GM_STREAM_CONTROL *ctrl );

void GM_StreamSdPauseControl( int channel, int on_off );

/*
	字幕
*/

int GM_StreamCaptionDriverInit( void );
void GM_StreamSetCaptionPrio( int handler, int prio );

/* 今誰の字幕を出しているか */

extern int GM_StreamCaptionCurrentName;
extern int GM_StreamCaptionCurrentHandler;

#define GM_DEFAULT_CAPTION_PRIO		128

/* ---------------------------------------------------------------------- */
/*
	ストリーミング開始
*/

int NewStreamControl( int pos, int size, int flag, int tagflag );

//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)
const char* GM_GetStreamName( int id );
//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)

// AS MCampbell - Adding a function to get the sound channel of stream.
int GM_GetSoundPlayBackChannel( int id );

/*
	ユーティリティ関数
*/

EXTERN_INLINE int GM_DemoStream( int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( FS_FILE_DEMO, pos );
	if( ofs < 0 ) return -1;

	return NewStreamControl( ofs, DEMO_BUFFER_SIZE, flag, pos );
}

EXTERN_INLINE int GM_DemoStreamLight( int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( FS_FILE_DEMO, pos );
	if( ofs < 0 ) return -1;

	return NewStreamControl( ofs, STREAM_BUFFER_SIZE / 2, flag, pos );
}

EXTERN_INLINE int GM_MovieStream( int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( FS_FILE_MOVIE, pos );
	if( ofs < 0 ) return -1;

	return NewStreamControl( ofs, MOVIE_BUFFER_SIZE, flag, pos );
}

EXTERN_INLINE int GM_MovieStream2( int file, int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( file, pos );
	if( ofs < 0 ) return -1;

	return NewStreamControl( ofs, MOVIE_BUFFER_SIZE, flag, pos );
}

EXTERN_INLINE int GM_IpuStream( int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( FS_FILE_MOVIE, pos );
	if( ofs < 0 ) return -1;

	return NewStreamControl( ofs, STREAM_BUFFER_SIZE/4, flag, pos );
}

