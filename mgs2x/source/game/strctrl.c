//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	ストリーミングコントローラー
		ダブルストリーミング対応

	2000/07/04 K.Uehara
	$Id: strctrl.c,v 1.5 2002/11/23 11:01:04 Yoshizawa1 Exp $
*/
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#include	<libdma.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"memlist.h"
#include	"libfs.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libgcl.h"
#include	"stream.h"

#include	"gameheader.h"
/*
#include	"g_sound.h"
#include	"g_define.h"
#include	"g_macro.h"
*/
#include	"strctrl.h"

#include "BP_FileSupport.h"

#ifdef KP_XBOX
#include	"sd_ee.h"
#endif


#undef KP_XBOX      //BP_SOUND

extern void FS_StreamDump( FS_STREAM *h_info );
extern void FS_StreamDebugPrint( FS_STREAM *h_info );

extern int sd_set_cli ( int );

extern int DG_TickCount;

/* ---------------------------------------------------------------------- */
/*
	コントロール構造体
*/

// ストリーミングシステム全体のワーク

#define MAX_STREAM 2		// このモジュールが管理するストリーミングの最大数

// 各ストリーム1つずつのワーク 
typedef struct _strctrl_Work {
	GV_ACT_EX actor;

	GM_STREAM_CONTROL ctrl;

	int channel;
	int flag;
	int end_flag;
	int start;
	void *str_buffer;
	int str_size;
} Work;

/* ---------------------------------------------------------------------- */
/*
	ストリーミングコントローラ全体の管理用ワーク
*/

typedef struct {
	MEMLIST list;
	int id_seed;
	int ids[ MAX_STREAM ];
	Work *works[ MAX_STREAM ];
	GM_STREAM_DRIVER *drivers;
} STREAM_CONTROL;

static STREAM_CONTROL control;

#define FLAG_START_WAIT		GM_STREAM_PLAY_WAIT
#define FLAG_EXEC_PROC		GM_STREAM_EXEC_PROC
#define FLAG_PAUSE_MENU		GM_STREAM_PAUSE_MENU
#define FLAG_PAUSE			0x00000008
#define FLAG_NO_PAUSE		GM_STREAM_NO_PAUSE

//BP - Added to get at the currently playing audio streams.
GM_STREAM_CONTROL* BP_GetStreamControl( unsigned int stream )
{
   // simply return null if no stream is currently available.
   if( control.works[ stream ] == 0 )
      return 0;
 
   // return the control structure of the requested stream.
   return &control.works[ stream ]->ctrl;
}


/* ---------------------------------------------------------------------- */
/*
	ID取得と返却と検索
*/

static int get_channel_id( Work *work, int ch )
{
	if( control.works[ ch ] == NULL ){
		control.ids[ ch ] = control.id_seed;
		control.works[ ch ] = work;
		work->channel = ch;
		control.id_seed ++;
		if( control.id_seed < 0 ) control.id_seed = 1;
		return control.ids[ ch ];
	}
	work->channel = -1;
	return -1;
}

static int get_empty_id( Work *work )
{
	int i;
	for( i = 0; i < MAX_STREAM; i++ ){
		int id;
		id = get_channel_id( work, i );
		if( id >= 0 ){
			return id;
		}
	}
	return -1;
}

static Work *get_id_work( int id )
{
	int i;
	if( id < 0 ){
		printf( "STR:WrongID %X\n", id );
		return NULL;
	}
	for( i = 0; i < MAX_STREAM; i++ ){
		if( control.ids[ i ] == id ){
			return control.works[ i ];
		}
	}
	return NULL;
}

static void release_id( int id )
{
	int i;
	for( i = 0; i < MAX_STREAM; i++ ){
		if( control.ids[ i ] == id ){
			control.ids[ i ] = -1;
			control.works[ i ] = NULL;
			return;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	ドライバ検索と起動
*/

static void StartStreamDriver( Work *work, int type )
{
	GM_STREAM_DRIVER *cp;

	for( cp = control.drivers; cp != NULL; cp = cp->next ){
		if( cp->type == ( type & 0xFFFF ) ){
			void *child;
			child = ( *cp->driver )( &( work->ctrl ), type );
			if( child == NULL ) break;
			if( child == GM_STREAM_DRIVER_NO_LINK ){
				return;
			} else {
				GV_SetActorChild( work, child );
				return;
			}
		}
	}

	/* ダミーを起動 */
	GV_SetActorChild( work, NewStreamDummyDriver( &( work->ctrl ), type ) );
}

/* ---------------------------------------------------------------------- */
/*
	コントローラー処理
*/

static void update_tick( Work *work )
{
	if( work->ctrl.sd_duration == 0 ){
		// サウンドがないストリーミング
		int now = DG_TickCount;
		if( work->ctrl.tick < 0 ){
			work->ctrl.tick = 0;
		}
		if( work->ctrl.sd_tick < 0 ){
			work->ctrl.sd_tick = now;
		}
		if( ( work->flag & FLAG_NO_PAUSE ) ||
		   ! ( ( work->flag & FLAG_PAUSE ) || ( GV_PauseLevel & (GV_PAUSE_PAUSE|GV_PAUSE_DEBUG) ) ) ){//BP_PAUSE - added GV_PAUSE_DEBUG
			/* PAUSE中はとまる */
#if 1 
			work->ctrl.tick += ( now - work->ctrl.sd_tick ) * BASE_TICK;
#else /// 実験
			if( now - work->ctrl.sd_tick > 0 ) work->ctrl.tick += BASE_TICK;
#endif			
//printf( "tick %d\n", work->ctrl.tick );
		}
		work->ctrl.sd_tick = now;
	}
}

static void pause_check( Work *work )
{
	if( work->flag & FLAG_PAUSE_MENU ){
		if( GV_PauseLevel & GV_PAUSE_MENU ){
			if( ( work->flag & FLAG_PAUSE ) == 0 ){
				GM_StreamSdPauseOn( &work->ctrl );
				work->flag |= FLAG_PAUSE;
			}
		} else if( work->flag & FLAG_PAUSE ){
			GM_StreamSdPauseOff( &work->ctrl );
			work->flag &= ~FLAG_PAUSE;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	ACT処理本体。
	3つの関数を切替えていく
*/

static void Die( Work *work )
{
	GM_STREAM_CONTROL *ctrl = &( work->ctrl );

	if( work->flag & FLAG_PAUSE ){
		GM_StreamSdPauseOff( &work->ctrl );
	}
	release_id( ctrl->id );

	if( ctrl->state >= GM_STREAM_STATE_INIT && ctrl->stream_h != NULL ){
		FS_StreamClose( ctrl->stream_h );
	}

	if( work->str_buffer != NULL ){
		GV_MlFree( &control.list, work->str_buffer, work->str_size );
		work->str_buffer = NULL;
	}
	if( ctrl->sd_channel >= 0 ){
		GM_StreamSdFreeChannel( ctrl->sd_channel );
	}
	if( work->channel >= 0 ){
		GM_ResetMenuStatus( ( MENU_STREAM_CH_0 << work->channel ) );
	}
	printf( "STREAM END lq = %d FLAG = %d\n", GM_LoadRequest, work->flag );

#ifdef KP_XBOX
	// 3D処理・ハンドルを確保していた場合は返還
	if( ctrl->hnsd3d != 0 ) {
		sd_3d_release_handle( ctrl->hnsd3d, 0 );
	}
#endif

	/* 本当は良くないが,ここで終了procを呼ぶ */
	/* ロード要求があった場合は実行しない */
	if( GM_LoadRequest == 0 ){
		/* 終了procを呼ぶ */
		if( work->flag & GM_STREAM_EXEC_PROC ){
			GCL_ARGS arg;
			int proc;
			int param[ 1 ];

			arg.argc = 1;
			arg.argv = param;
			param[ 0 ] = ( work->end_flag ) ? 1 : 0;

			proc = ( ( unsigned int )work->flag ) >> 8;
			printf( "stream exec proc %x arg %d\n", proc, param[ 0 ] );
			GCL_ExecProc( proc, &arg );
		}
	}
}

static void DieWait( Work *work )
{
	update_tick( work );
	pause_check( work );
	if( work->actor.child == NULL
		&& !FS_StreamIsReading( work->ctrl.stream_h )
		&& ( FS_StreamIsLocked( work->ctrl.stream_h ) == 0 )
//		&& ( ( work->flag & FLAG_PAUSE ) == 0 )
		){
		/* 子が全部終了した */
		GV_DestroyActor( work );
	}

#ifndef GOLD_VERSION
   FS_StreamDebugPrint( work->ctrl.stream_h );
#endif
}

static void Act( Work *work )
{
	GM_STREAM_CONTROL *ctrl;

	ctrl = &( work->ctrl );

	if( work->end_flag ){
		ctrl->state = GM_STREAM_STATE_READ_END;
		GV_ChangeActFunc( work, DieWait );
		return;
	}
	switch( ctrl->state ){
	  case GM_STREAM_STATE_INIT:
		{
			int state;

			FS_StreamSync( ctrl->stream_h );
			state = FS_StreamGetStatus( ctrl->stream_h );
			if( state == FS_STR_START ){
				/* 初期読み込み中 */
				break;
			}
			ctrl->state = GM_STREAM_STATE_WAIT;
		}
	  case GM_STREAM_STATE_WAIT:
		pause_check( work );
		if( !( work->flag & FLAG_START_WAIT ) ){
			/* 再生開始 */
			int type;

			work->start = DG_TickCount;
			/* そのデータ中にあるタイプのデータの登録ドライバを起動する */
			while( ( type = FS_StreamGetSystemData( ctrl->stream_h ) ) != CHANK_TYPE_FREE ){
				StartStreamDriver( work, type );
			}
			ctrl->state = GM_STREAM_STATE_PLAY;
		}
		break;
	  case GM_STREAM_STATE_PLAY:
		pause_check( work );
		update_tick( work );
		if( FS_StreamSync( ctrl->stream_h ) == 0 ){
			/* 読み込み完了 */
printf( "STREAM READ END\n" );
			ctrl->state = GM_STREAM_STATE_READ_END;
			GV_ChangeActFunc( work, DieWait );
		}
#ifndef GOLD_VERSION //BP
		if( GV_PadDataDirect[ 0 ].press & PAD_Y && GV_PadDataDirect[ 0 ].status & PAD_R1 ){
			FS_StreamDump( ctrl->stream_h );
		}
      FS_StreamDebugPrint( ctrl->stream_h );
#endif
		break;
	}
}

static void Act_InitWait( Work *work )
{
	GM_STREAM_CONTROL *ctrl = &( work->ctrl );

	/*
		CDの空き待ち
	*/

	if( work->end_flag ){
		ctrl->stream_h = NULL;
		/*
			CDは動いていないのでそのまま終了
		*/
		GV_DestroyActor( work );
		return;
	}
	if( FS_StreamSystemStatus() == 0 ){
		int pos;

		/*
			CDスタート
		*/
		pos = ( int )ctrl->stream_h;
		ctrl->stream_h = FS_Stream( pos, work->str_buffer, work->str_size, 0 );
		ctrl->state = GM_STREAM_STATE_INIT;
		GV_ChangeActFunc( work, Act );
	}
}

static int bp_apply_tagflag_hacks( const int tagflag )
{
   if( BP_Area_JP() )
   {
      switch(tagflag)
      {
      case 0x20006034:  // tag/flag for demo/t04a2d (in jp/substance)
      case 0x20052ad4:  // tag/flag for demo/p036_07_p06 (in jp/substance)
      case 0x20089D1B:  // tag/flag for demo/p079_03_p02 (in jp/substance)
         {
            // replace TAGFLAG_SOUND_VAG with TAGFLAG_SOUND_8BIT
            // because that's the format of the sound in the jp/sol data
            // we have merged in
            return (tagflag & 0x0FFFFFFF) | TAGFLAG_SOUND_8BIT;
         }
         break;
      }
   }

   return tagflag;
}

#ifdef KP_WINDOWS
static int		stream_ch = -1;
#endif
int NewStreamControl( int pos, int size, int flag, int tagflag )
{
	Work *work;
	void *ptr;
   const char * stream_name;

	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM
								 , sizeof( Work ), 0xf1 ) ) != NULL ){
		GM_STREAM_CONTROL *ctrl = &( work->ctrl );
		int id = -1;
		int ch = -1;

		GV_ActorEX( &work->actor );
		GV_SetActor( &( work->actor ), Act_InitWait, Die );
		/* ステージロード時には暗黙にdestroyされない */
		GV_SetActorKillLevel( &( work->actor ), GV_KILL_LEVEL_SYSTEM );
#ifdef KP_XBOX
		ctrl->hnsd3d = 0;
#endif		
      ctrl->top_pos = pos;

      if(stream_name = BP_FindStreamName(ctrl->top_pos))
      {
         printf("strctrl: init: %s\n", stream_name );
      }

#ifdef KP_WINDOWS
		if((flag & GM_STREAM_CHANNEL_MASK) == GM_STREAM_CHANNEL_MASK ){
			ch = stream_ch;
		}else{
#endif
			if( flag & GM_STREAM_CHANNEL_0 ){
				ch = 0;
			} else if( flag & GM_STREAM_CHANNEL_1 ){
				ch = 1;
			}
#ifdef KP_WINDOWS
		}
#endif
      tagflag = bp_apply_tagflag_hacks( tagflag );
		if( tagflag & TAGFLAG_SOUND_EXIST ){
			// サウンドがあるのでチャンネルを確保

			if( tagflag & ( TAGFLAG_SOUND_8BIT | TAGFLAG_SOUND_AC3 ) ){
				// 8bitSoundはch1
#ifdef PSX2
				sd_set_cli( 0xFF00000E );   /** コンパイル通し用にコメントアウト T.Morita 2002.02.01  **/
#endif
				ch = 1;
			}

			ctrl->sd_channel = GM_StreamSdGetChannelNo( ch );
			if( ctrl->sd_channel < 0 ){
				printf( "STR:SOUND USED\n" );
				GV_DestroyActor( work );
				return -1;
			}
		}
		if( ch < 0 ){
			id = get_empty_id( work );
		} else {
			id = get_channel_id( work, ch );
		}

		if( id < 0 ){
			printf( "STR:STREAM_USED\n" );
			GV_DestroyActor( work );
			return -1;
		}
		
		ctrl->id = id;

		// バッファ確保

		ptr = GV_MlMalloc( &control.list, size );

		work->str_buffer = ptr;
		work->str_size = size;
		if( ptr == NULL ){
			printf( "STR:NO MEMORY!!\n" );
			GV_DestroyActor( work );
			return -1;
		}

		GM_SetMenuStatus( MENU_STREAM_CH_0 << work->channel );

		ctrl->state = GM_STREAM_STATE_INIT;

		if( FS_StreamSystemStatus() == 0 ){
			ctrl->stream_h = FS_Stream( pos, work->str_buffer, work->str_size, 0 );
			GV_ChangeActFunc( work, Act );
		} else {
			ctrl->stream_h = ( void * )pos;
			ctrl->state = GM_STREAM_STATE_INIT_WAIT;
		}

		work->flag = flag;
		work->end_flag = 0;

		ctrl->tick = 0;
		ctrl->sd_tick = -1;
		ctrl->sd_duration = 0;
		ctrl->flag = ( flag & 0xFF );
      ctrl->tagflag = tagflag;   //BP added
		ctrl->caption_prio = 128;

printf( "STREAM %d BUF %X SIZE %d: ID = %d FLAG = %X\n"
		, work->channel, work->str_buffer, work->str_size, id, flag );
		return id;
	}
	printf( "STR:NO MEM\n" );
	return -1;
}

//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)
const char* GM_GetStreamName( int id )
{
   // Search for associated work vars
   Work* work = get_id_work( id );
   if( work )
   {
      // Playing a stream?
      GM_STREAM_CONTROL *ctrl = &work->ctrl;
      if( ( ctrl->stream_h != NULL ) && ( ctrl->state >= GM_STREAM_STATE_INIT ) )
      {
         return FS_StreamGetName( ctrl->stream_h );
      }
   }

   // No active stream
   return NULL;
}
//BP_CAMERA - adding function to get stream file name (used by cinema camera settings)

// AS MCampbell - Adding a function to get the sound channel of stream.
int GM_GetSoundPlayBackChannel( int id )
{
   Work* work = get_id_work( id );
   if (work)
   {
      // Playing a stream?
      GM_STREAM_CONTROL *ctrl = &work->ctrl;
      if( ( ctrl->stream_h != NULL ) && ( ctrl->state >= GM_STREAM_STATE_INIT ) )
      {
         return ctrl->sd_channel;
      }
   }

   return -1;
}

/* ---------------------------------------------------------------------- */
/*
	外部関数
*/

void *GM_GetStreamActor( int handler )
{
	return get_id_work( handler );
}

void GM_StreamReset( void )
{
	STREAM_CONTROL *ctrl;
	
	ctrl = &control;

	/* ストリーミング用メモリ領域を初期化 */

	GV_MlBufferInit( &ctrl->list, "STREAM", STREAM_BUFFER
					 , STREAM_BUFFER + STREAM_BUFFER_SIZE, FS_SECTOR_SIZE );

	ctrl->id_seed = 1;
	ctrl->drivers = NULL;

	GM_StreamSdDriverInit();
	GM_StreamCaptionDriverInit();
}

#ifdef KP_WINDOWS
/*---------------------------------------------------------------------------
|
|		BGMをストリームにするためGM_VoxStreamにチャンネル数を拡張した関数
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
int GM_VoxStreamEx( int pos, int flag, int ch )
{
	stream_ch = ch;
	return(GM_VoxStream( pos, flag | GM_STREAM_CHANNEL_0 | GM_STREAM_CHANNEL_1 ));
}
#endif
int GM_VoxStream( int pos, int flag )
{
	int ofs;
	ofs = FS_GetStreamTop( FS_FILE_VOX, pos );
	if( ofs < 0 ) return -1;
#if 0
	if( ( flag & GM_STREAM_CHANNEL_MASK ) == 0 ){
		int channel;
		channel = GM_StreamSdGetChannel();
		GM_StreamSdSetPan( channel, 0x0000203F );
		if( channel < 0 ){
			return -1;
		}
		flag |= GM_STREAM_FLAG_CHANNEL( channel );
	}
#endif
printf( "VOX %x %x %x\n", pos, ofs, flag );
#ifdef KP_XBOX
 	{	// 3Dストリーム処理
		int hnstrm = NewStreamControl( ofs, VOX_BUFFER_SIZE, flag & ~GM_STREAM_FLAG_3D, pos );
		if( hnstrm >= 0 && (flag & GM_STREAM_FLAG_3D) ) {
			Work* pw = get_id_work( hnstrm );
			pw->ctrl.hnsd3d = sd_3d_get_handle();
			// デフォルトでは３Ｄ処理を切っておく
#if 0			
			sd_3dsrc_setmode( pw->ctrl.hnsd3d, DS3DMODE_DISABLE );
#else
			sd_3dsrc_setmode( pw->ctrl.hnsd3d, DS3DMODE_HEADRELATIVE );
#endif			
			sd_3dsrc_setvol( pw->ctrl.hnsd3d, DSBVOLUME_MAX );
		}
		return hnstrm;
	}
#else 
	return NewStreamControl( ofs, VOX_BUFFER_SIZE, flag, pos );
#endif	
}

int GM_StreamStatus( int handler )
{
	Work *work;

	work = get_id_work( handler );
	if( work == NULL ){
		return GM_STREAM_STATE_END;
	}

	return work->ctrl.state;
}

// 再生制御

void GM_StreamSetCaptionPrio( int handler, int prio )
{
	Work *work;
	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}
	work->ctrl.caption_prio = prio;
}

void GM_StreamStart( int handler )
{
	Work *work;
	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}
	work->flag &= ~FLAG_START_WAIT;
}

void GM_StreamStop( int handler )
{
	Work *work;

	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}

	if( work->ctrl.state >= GM_STREAM_STATE_INIT ){
		FS_StreamStop( work->ctrl.stream_h );
	}
	work->end_flag = 1;
	GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0 );
}

void GM_StreamStopAll( void )
{
	/* 現在再生中のすべてのストリームを停止 */
	int i;
	for( i = 0; i < MAX_STREAM; i++ ){
		if( control.works[ i ] != NULL ){
			GM_StreamStop( control.ids[ i ] );
		}
	}
}

int GM_StreamIsPlay( void )
{
	int i, n;
	n = 0;
	for( i = 0; i < MAX_STREAM; i++ ){
		if( control.works[ i ] != NULL ){
			n++;
		}
	}
	return n;
}

int GM_StreamGetChannelHandler( int channel )
{
	int i;

	for( i = 0; i < MAX_STREAM; i++ ){
		if( control.works[ i ] == NULL ) continue;
		if( control.works[ i ]->ctrl.sd_channel == channel ){
			return control.ids[ i ];
		}
	}
	return -1;
}

// 属性変更

void GM_VoxStreamSetPan( int handler, int vol, int pan, float bp_angle )
{
	Work *work;

	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}
#ifdef KP_XBOX
	if( work->ctrl.hnsd3d != 0 ) {
		sd_3dsrc_setmode( work->ctrl.hnsd3d, DS3DMODE_HEADRELATIVE );
		sd_3dsrc_setpos( work->ctrl.hnsd3d, (D3DXVECTOR3*)&DG_ZeroVector );
	}
#endif	
	GM_StreamSdSetPan( work->ctrl.sd_channel, ( vol ) | ( pan << 8 ), bp_angle );
}

#ifdef KP_XBOX
#if 0
void GM_VoxStreamSetPos( int handler, FVECTOR* pv, int mode, int* pvol, int* ppan )
{
	Work *work;
	int tmpvol = 0, tmppan = GM_PAN_CENTER;

	if( pvol == NULL ) pvol = &tmpvol;
	if( ppan == NULL ) ppan = &tmppan;
	if( pv != NULL ) {
		GM_SeGetVolPan( pv, mode, pvol, ppan );
	}

	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}
	
	if( work->ctrl.hnsd3d != 0 ) {
		if( pv != NULL ) {
			sd_3dsrc_setmode( work->ctrl.hnsd3d, DS3DMODE_NORMAL );
			sd_3dsrc_setpos( work->ctrl.hnsd3d, (D3DXVECTOR3*)pv );
			sd_3dsrc_setvol( work->ctrl.hnsd3d, GM_ConvertVol( *pvol ) );
		} else {
			sd_3dsrc_setmode( work->ctrl.hnsd3d, DS3DMODE_HEADRELATIVE );
			sd_3dsrc_setvol( work->ctrl.hnsd3d, GM_ConvertVol( *pvol ) );
			sd_3dsrc_setpos( work->ctrl.hnsd3d, (D3DXVECTOR3*)&DG_ZeroVector );
		}
	} else {
		GM_StreamSdSetPan( work->ctrl.sd_channel, ( *pvol ) | ( (*ppan) << 8 ) );
	}
}
#else

void GM_VoxStreamSetParam( int handler, FVECTOR* pv, HZX_ZONE_ADD addr, int vol, int pan )
{
	Work *work;
	
	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}
	
	if( work->ctrl.hnsd3d != 0 ) {
		if( pv != NULL ) {
			sd_3dsrc_setmode( work->ctrl.hnsd3d, DS3DMODE_NORMAL );
			sd_3dsrc_setpos( work->ctrl.hnsd3d, (D3DXVECTOR3*)pv );
			sd_3dsrc_setvol( work->ctrl.hnsd3d, GM_ConvertVol( vol ) );
		} else {
			sd_3dsrc_setmode( work->ctrl.hnsd3d, DS3DMODE_HEADRELATIVE );
			sd_3dsrc_setvol( work->ctrl.hnsd3d, GM_ConvertVol( vol ) );
			sd_3dsrc_setpos( work->ctrl.hnsd3d, (D3DXVECTOR3*)&DG_ZeroVector );
		}
	} else {
		GM_StreamSdSetPan( work->ctrl.sd_channel, vol | ( (pan) << 8 ) );
	}
}
#endif // 0
#endif // XBOX

void GM_StreamSetActionProc( int handler, int proc )
{
	Work *work;

	work = get_id_work( handler );
	if( work == NULL ){
		return;
	}

	work->ctrl.flag = ( work->ctrl.flag & 0xFF ) | ( ( unsigned int )proc << 8 );
}

/* ---------------------------------------------------------------------- */
/*
	初期化関数
*/

void GM_StreamAddDriver( GM_STREAM_DRIVER *driver, int type )
{
	STREAM_CONTROL *ctrl;
	
	ctrl = &control;

	{
		GM_STREAM_DRIVER *next;

		for( next = ctrl->drivers; next != NULL; next = next->next ){
			if( next == driver ){
				return;
			}
		}
	}
printf( "ADD DRIVER %X\n", type );
	driver->type = type;
	driver->next = ctrl->drivers;
	ctrl->drivers = driver;
}

void GM_StreamRemoveDriver( GM_STREAM_DRIVER *driver )
{
	GM_STREAM_DRIVER *cp, *pre;

	pre = NULL;
	for( cp = control.drivers; cp != NULL; cp = cp->next ){
		if( cp == driver ){
			if( pre == NULL ){
				control.drivers = cp->next;
			} else {
				pre->next = cp->next;
			}
			return;
		}
	}
printf( "RmDrv: no DRV\n" );
}
