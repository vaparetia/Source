//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------

#include <stdio.h>
#include "BP_Misc.h"
#include "BP_FileSupport.h"

#if 0
﻿/*
	demod.c
	デモ管理デーモン

	2000/07/26	K.Takabe
	$Id: demod.c,v 1.6 2002/12/03 13:43:41 takaki Exp $

*/
/*

	＜デモの処理内容について＞
	デモはキャラとして起動し、ＣＤからのストリーム又はメモリに読み込んだイメージファイル
	に記述されているカメラや、オブジェクトなどのシーケンス情報をを管理する。旧方式の
	デモ再生と違い、基本的にデモ管理側ではデータの管理のみを行ない、表示などは他の
	アクターを経由する形式をとっている。

	・フレーム（＝カメラ）の管理
	　デモではそのフレームに存在するフレーム情報を管理するだけで実際にカメラへ反映
	　させるには専用のカメラ設定アクターの起動を行なう。

	・オブジェクト（＝モデル）の管理
	　デモでは専用のオブジェクト表示アクターの起動を行い、表示・非表示などの制御は
	　シグナルを通して行なう。

	・モーションの管理
	　モーションもデモ側では管理のみを行ない、実際のモーションの設定はオブジェクト
	　表示アクター側でデータを取得してもらう事で再生を行なう。

	・エフェクトの管理
	　起動のみ必要な場合は従来通りに各エフェクトごとの起動ルーチンで行い、デモ側で
	　状態保持が必要なものに関しては専用のエフェクト管理アクターを起動する。
	　

	--------------------------------

	void* DM_StartDeamon( void *stream_handle )
	void	*stream_handle ;	ストリーム対応時：取得済みのFS_STREAM*へのポインタ
								ストリーム非対応時：ストリームファイルパスへのポインタ
	デモ用デーモン起動


	DM_WORK* DM_GetDemoWork( void )

	デモ再生が行われていればワークのポインタを返す

*/
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#ifdef DEBUG_MODE
/* デバッグ時のみインクルード */
#include <ctype.h>
#include <libutl.h>
#include <string.h> /*T.Morita Added 2000/08/30 */
#endif

#include "strctrl.h"
#include "libdemo.h"

#ifdef DEBUG_MODE
DM_DEBUG dm_debug;
#endif

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
typedef struct _act {
	GV_ACT_EX	actor ;
	DM_WORK		demo_work ;
	int			name ;					/* メッセージ受け取り用 */
	int			map;
	int			current_time ;
	int			last_time ;
	int			start_time ;			/* デモ再生開始時のシステム絶対時間 */
	int			stream_id;				/* ストリーミングID */
	void		*stream_top ;			/* 処理中のストリームパケットアドレス */
	FS_STREAM	*stream_handle ;		/* オープン済みストリームハンドル */
	GM_STREAM_CONTROL *strctrl;			/* ストリーミングコントロール構造体 */
	int			dg_frame_save;		/* デモ開始前のデモ環境 */
	int			first_flag ;			/* 初期実行フラグ */
	int			begin_proc;				/* 開始時PROC */
	int			undraw_flag;			/* Undraw設定フラグ */

#ifdef DEBUG_MODE
	int			file_flag ;				/* ファイル再生フラグ */
	int			debug_flag;				/* デバッグモード実行フラグ */
	void		*file_ptr ;				/* 読み込んだデモファイルのポインタ */
	int			file_size ;				/* 読み込んだファイルのサイズ（必要無いかも） */
	void		*next_stream_packet ;	/* 次のデモパケットデータ先頭 */
#endif
} Work ;

#ifdef KP_WINDOWS
BOOL			DM_DemoActNow = FALSE ;
#endif
static Work		*DM_DemoActor = NULL ;

#define UNDRAW_BEGIN	0x01
#define UNDRAW_END		0x02

#ifdef DEBUG_MODE
/* デバッグ用のスタティック変数 */
static int loop_count  = 0;
static int blank_count = 0;
static int start_count = 0;
static int de2_size    = 0;

/* デバッグ用のプロトタイプ宣言 */
static void Demo_DebugCursole(Work *work);
static void Demo_DebugView(Work *work);
static int  GetEffectList( Work *work, void *file_path );
#endif

#ifdef DEBUG_MODE

extern int GM_DEMO_SkipFlag;		// in gamed.c

#endif

/* ---------------------------------------------------------------- */
	/*
		各種マクロ定義
	*/


/* ---------------------------------------------------------------- */
#ifdef DEBUG_MODE
/* ファイルの読み込み */
static int _ReadFile(char *filename, unsigned char *data, int *size)
{
#ifdef PSX2
	int fd;
	int readcount;

	printf("file reading...(%s)\n", filename );
	fd = sceOpen(filename, SCE_RDONLY);
	if (fd < 0) {
		printf("Cannot open %s\n", filename);
		return 0;
	}
	*size = sceLseek(fd, 0, SCE_SEEK_END);
	if (*size < 0) {
		printf("sceLseek() fails (%s)\n", filename);
		sceClose(fd);
		return 0;
	}
	sceLseek(fd, 0, SCE_SEEK_SET);

	readcount = sceRead(fd, data, *size);
	if (readcount != *size) {
		printf("Cannot read %s\n", filename);
		sceClose(fd);
		return 0;
	}
	printf("file read end\n" );

	sceClose(fd);
#endif
	return 1;
}
#endif
/* ---------------------------------------------------------------- */
	/*
		次のストリームをセットする
	*/
static void GetNextStream( Work *work )
{
#ifdef DEBUG_MODE
	if(GV_PauseLevel == GV_PAUSE_MENU)
		return;
#endif

	/* 以前に取得したストリームデータがある場合には開放する */
	if ( work->stream_top != NULL ){
		FS_StreamFreeData( work->stream_handle, work->stream_top );
	}
	/* 次のパケットのアドレスを取得 */
	work->stream_top = FS_StreamGetData( work->stream_handle, CHANK_TYPE_DEMO );
//printf( "DATA %X\n",work->stream_top );
}

#ifdef DEBUG_MODE
static void GetNextStreamFile( Work *work )
{
	/* ファイルの場合は連続しているのでポインタをずらすだけ */
	work->stream_top = work->next_stream_packet ;
	work->next_stream_packet = ( char * )work->stream_top + FS_STREAM_GET_SIZE( work->stream_top ) + sizeof(STREAM_TAG) ;
#if 0
	printf("new_stream) 0x%08x time:%d size:%d option:%d\n",
		   work->stream_top,
		   FS_STREAM_GET_TIME( work->stream_top ),
		   FS_STREAM_GET_SIZE( work->stream_top ),
		   FS_STREAM_GET_OPTION( work->stream_top ) );
#endif
}
#endif

#ifdef PSX2
#define DISC_ERROR_MARK_XTOP		472
#define DISC_ERROR_MARK_YTOP		48
#endif
#ifdef KP_XBOX
#define DISC_ERROR_MARK_XTOP		(472*640/512-4)
#define DISC_ERROR_MARK_YTOP		(48)
#endif

static int Demo_ReadErrorCount = 0;
#ifdef PSX2
#define DEMO_READ_ERROR_SHOW		60
#endif
#ifdef KP_XBOX
#define DEMO_READ_ERROR_SHOW		300
#endif

/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	int		current_time, stream_time, exec_flag = 0 ;

   if ( GV_PauseLevel & GV_PAUSE_DEBUG )
   {
      return;
   }

	GM_SetCurrentMap(work->map);

	/* 初期実行チェック */
	if ( work->first_flag ){
		work->start_time = DG_TickCount ;
		work->first_flag = 0 ;
//		work->last_time = DG_TickCount;
		work->current_time = DG_TickCount;
	}

#ifdef DEBUG_MODE
	if( work->debug_flag ){
		/* デバッグモード実処理 */
		Demo_DebugCursole(work);
		/* 現在のフレームカウントを保存 */
		dm_debug.frame = DG_TickCount - work->start_time;
		dm_debug.pre_tick = DG_TickCount;
	}
#endif

	/* 相対時間取得 */
#if 0
	current_time = ( DG_TickCount - work->start_time ) * TIME_BASE ;
#else
	if( work->strctrl->sd_duration > 0 ){
		// 音ありデモ
		int prev;

      current_time = ( int )( work->strctrl->sd_tick );

      prev = work->last_time;
		if( work->last_time < current_time ){
			int dtime;
			dtime = ( DG_TickCount - work->current_time );

			work->last_time += dtime * TIME_BASE;

			while( current_time - work->last_time > TIME_BASE ){
				work->last_time += TIME_BASE;
			}
#if 0
			if( current_time > work->last_time ){
				work->last_time += TIME_BASE;
			}
#endif
			current_time = work->last_time;
		} else {
			// 音がひきつった
		}
		work->current_time = DG_TickCount;
	} else {
		current_time = ( DG_TickCount - work->start_time ) * TIME_BASE;
	}
#endif
	/* エフェクト初期化フラグを毎フレームリセットする */
	DM_EffectInitFlag = 0 ;

	/* ヒキツリフレームを毎フレームリセットする */
	DM_FrameSkip = 0 ;
	/* 現在の時間になるまでデモを処理 */
	while ( exec_flag == 0 && work->demo_work.sequence_end == 0 ){
		/* 新規ストリーム取得 */
#ifndef DEBUG_MODE
		GetNextStream( work );
#else
		if ( !work->file_flag ){
			GetNextStream( work );
		} else {
#ifdef PSX2
			GetNextStreamFile( work );
#endif
		}
#endif
		if( work->stream_top == NULL ){
//			printf( "STREAM NULL !!!\n" );
// リードエラー処理（仮）

GV_PauseOnActorSystem( GV_PAUSE_READERROR );
if( ++Demo_ReadErrorCount > DEMO_READ_ERROR_SHOW ){
	GM_ReadError( ( DG_TickCount % 60 < 30 ) ? 0 : 1 );
}
			return;
		}
Demo_ReadErrorCount = 0;
GV_PauseOffActorSystem( GV_PAUSE_READERROR );
#ifdef KP_XBOX
GM_ReadErrorHide();
#endif
		stream_time = FS_STREAM_GET_TIME( work->stream_top );

		/* 通常処理かスキップ処理かチェック */
		if ( current_time >= ( stream_time + TIME_BASE ) ){
#ifdef DEBUG_MODE
			if ( dm_debug.mode1 != 0 ) {
				exec_flag = 1;
			} else {
			    /* スキップ処理 */
			    DM_FrameSkip++ ;
			    exec_flag = 0 ;
			}
#else
		    /* スキップ処理 */
		    DM_FrameSkip++ ;
		    exec_flag = 0 ;
#endif
//		    printf("frame skip....\n");
		} else {
		    /* スキップなし */
		    exec_flag = 1 ;
		}

#if BP_TGS_DEMO()
      BP_TGS_DEMO_CutscenePlaying();
#endif

		/* デモのパケットストリームを処理 */
		DM_ExecDemoStream( &work->demo_work, work->stream_top, exec_flag );

      //BP_CAMERA - adding extra demo vars for tracking camera cuts
      gBP_Demo_Frame++;
      //BP_CAMERA - adding extra camera name/id for tweak system
	}

#ifdef DEBUG_MODE
	if( work->debug_flag ){
		/* デバッグモード表示処理 */
		Demo_DebugView(work);
	}
#endif

	if ( work->demo_work.sequence_end ){

#ifdef DEBUG_MODE
		if( work->debug_flag ){
			/* ループモード */
			if ( dm_debug.play_mode == 1 ) {
				loop_count++;
				if ( dm_debug.start_time > 0 )
					blank_count = 1;
				else
					blank_count = BLANK_COUNT;

				GM_LoadRequest = GM_REQ_RESET;
				GV_DestroyActor( work );
				return;
			}
		}
#endif
		printf("Destroy Actor\n");
		GV_DestroyActor( work );
	}
}
/* ---------------------------------------------------------------- */
	/*
		デモ起動用関数
	*/

static void StartWait( Work *work )
{
	/* ドライバからの起動要求を受け付けるまでのダミー関数 */

#ifdef DEBUG_MODE
	/* リスタートのブランク */
	if ( blank_count > 0 ) {
		blank_count--;
		GV_PauseOnActorSystem(GV_PAUSE_MENU);

		DEBUG_Locate(12, 12 * VIEW_TYPE, MENU_MODE_NORMAL);
		DEBUG_Printf("Restart Demo. Please wait a minute. %d\n", blank_count);

		if ( blank_count <= 0 ) {
			GV_PauseOffActorSystem(GV_PAUSE_MENU);
			GV_PauseOffActorSystem(GV_PAUSE_STOP);
			GM_LoadRequest = GM_REQ_RESET;
			GV_DestroyActor( work );
		}
		return;
	}

	/* タイミングカウンタ */
	if ( start_count > 0 ) {
		start_count--;
		GV_PauseOnActorSystem(GV_PAUSE_MENU);

		DEBUG_Locate(12, 12, MENU_MODE_NORMAL);
		DEBUG_Printf("Ready............%d\n", start_count/60 + 1);

		if ( start_count <= 0 ) {
			if ( loop_count == 0 )
				loop_count++;
			GV_PauseOffActorSystem(GV_PAUSE_MENU);
			GV_PauseOffActorSystem(GV_PAUSE_STOP);
			GM_LoadRequest = GM_REQ_RESET;
			GV_DestroyActor( work );
		}
		return;
	}
	GM_StreamStart( work->stream_id );
#endif
}

static void *StartDemo( GM_STREAM_CONTROL *ctrl, int type )
{
	if( DM_DemoActor != NULL ){
		GV_ChangeActFunc( DM_DemoActor, Act );
		DM_DemoActor->stream_handle = ctrl->stream_h;
		DM_DemoActor->strctrl = ctrl;

		GV_PauseOffActorSystem( GV_PAUSE_STOP );

		if( DM_DemoActor->undraw_flag & UNDRAW_BEGIN ){
#ifndef KP_WINDOWS
			DG_UnDrawFrameCount = 2;
#else
		    DG_SetUnDrawFrameCount(2);
#endif
		}
		if( DM_DemoActor->begin_proc != 0 ){
			GCL_ExecProc( DM_DemoActor->begin_proc, NULL );
		}
	}
	return GM_STREAM_DRIVER_NO_LINK;
}

#ifdef __GNUC__
static GM_STREAM_DRIVER demo_driver = {
	driver : StartDemo,
};
#else
static GM_STREAM_DRIVER demo_driver = {
	NULL, 0,
	StartDemo,
};
#endif

static void AddDriver( void )
{
	GM_StreamAddDriver( &demo_driver, CHANK_TYPE_DEMO );
}

static void DelDriver( void )
{
	GM_StreamRemoveDriver( &demo_driver );
}

/* ---------------------------------------------------------------- */
	/*
		デーモンの破棄
	*/

// Special case to ensure the undraw frame count (effectivly rendering of the current frame) gets turned on the very frame the demod object gets destroyed.
void BP_SpecialCase_DemoActorDie(void* this)
{
   if( DM_DemoActor && DM_DemoActor == this)
   {
      if( DM_DemoActor->undraw_flag & UNDRAW_END )
         DG_UnDrawFrameCount = DG_UNDRAW_MAX;
   }
}

static void Die( Work *work )
{
#ifndef DEBUG_MODE
	/* 以前に取得したストリームデータがある場合には開放する */
	if ( work->stream_top != NULL ){
		FS_StreamFreeData( work->stream_handle, work->stream_top );
	}
#else
	if ( !work->file_flag ){
		/* 以前に取得したストリームデータがある場合には開放する */
		if ( work->stream_top != NULL ){
			FS_StreamFreeData( work->stream_handle, work->stream_top );
		}
	} else {
		/* ファイルからの場合には何もしない */
	}
#endif

	{/* 圧縮モーション展開用ワーク開放(K.Takabe Added 2000/09/21) */
		int		i ;
		for ( i = 0 ; i < work->demo_work.n_motion ; i++ ){
			GV_DelayedFree( work->demo_work.mtn_list[ i ].motion_ptr );
		}
	}

	/* 子アクターに終了シグナルを送る */
	GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0);
	/* ワークへのポインタを破棄 */
	DM_DemoActor = NULL ;
	//	GM_GameStatus &= ~STATE_DEMO ;
	//GM_ResetGameStatus(STATE_DEMO); /* gamed.c でやる */
	DG_FrameCount = work->dg_frame_save;
	DM_FrameSkip = 0 ;

	DelDriver();

#ifdef DEBUG_MODE
printf( "Demod: Die\n" );
#endif
	if( work->undraw_flag & UNDRAW_END ){
#ifndef KP_WINDOWS
		DG_UnDrawFrameCount = DG_UNDRAW_MAX;
#else
	    DG_SetUnDrawFrameCount(DG_UNDRAW_MAX);
#endif
	}

	/* デモリストをリセットする */
	DM_RefreshRenameList() ;

#ifdef KP_WINDOWS
	/* 稼動フラグOFF */
	DM_DemoActNow = FALSE ;
#endif

#if BP_TGS_DEMO()
   BP_TGS_DEMO_CutsceneEnd();
#endif

   //BP_CAMERA - adding extra demo vars for tracking camera cuts
   gBP_Demo_Active = 0;
   gBP_Demo_Id = 0;
   gBP_Demo_Frame = -1;
   gBP_Demo_StreamName[0] = 0;
   //BP_CAMERA - adding extra camera name/id for tweak system
}

/* ---------------------------------------------------------------- */
	/*
		初期化
	*/
static int GetResources( Work *work, int name, void *stream_handle )
{
	int i ;

	work->name = name ;
	work->demo_work.actor = &work->actor ;
	work->stream_top = NULL ;
	work->first_flag = 1 ;
//	GM_GameStatus |= STATE_DEMO ;
	work->dg_frame_save = DG_FrameCount;
	DG_FrameCount = 1;
	DM_FrameSkip = 0 ;

	/*ローポリ劇場用フラグ   T.Morita Added 2001/09/18 */
	work->demo_work.low_poly_demo = 0 ;
	/* ローポリ劇場フラグをオンにする */
	for( i=DM_n_RenameList ; --i>=0 ; )
	    if ( DM_RenameFrom[i] & 0x80000000 )
	    {
		work->demo_work.low_poly_demo = 1 ;
		break ;
	    }

	/* 圧縮モーション展開用ワーク初期化(K.Takabe Added 2000/09/21) */
	work->demo_work.n_motion = 0 ;

	if( GCL_GetOption( 'S' ) == NULL ){
		GM_SetGameStatus(STATE_DEMO);
	}

	/* T. Morita Added 2001/07/30 */
	/* デモからIDチェンジの設定 */
	if ( GCL_GetOption( 'E' ) != NULL )
	    DM_AddRenameList( 'e', 0 ) ;
	if ( GCL_GetOption( 'K' ) != NULL )
	    DM_AddRenameList( 'k', 0 ) ;
#if DEBUG_MODE
	printf( "DM_n_RenameList == %d\n", DM_n_RenameList ) ;
	for ( i=0 ; i<DM_n_RenameList ; i++ )
	    printf( "DM_RenameFrom[%x]->DM_RenameTo[%x]\n",
		    DM_RenameFrom[i], DM_RenameTo[i] ) ;
#endif

	/* T. Morita Added 2000/08/30 */
	/* デモから起動されるProcの設定 */
	if ( GCL_GetOption( 'p' ) != NULL )
	    for ( i=0 ; GCL_NextStr() && i<DM_MAX_PROC ; i++ ){
			work->demo_work.proc_id[i] = GCL_GetNextInt() ;
		}

	/* T. Morita Added 2001/03/05 */
	DM_EffectInitName = 0 ;
	DM_EffectInitFlag = 0 ;

#ifndef DEBUG_MODE
	/* ストリームハンドルを保存 */
//	work->stream_handle = stream_handle ;
#else
	if ( !work->file_flag ){
		/* ストリームハンドルを保存 */
//		work->stream_handle = stream_handle ;
	} else {
		/* デモストリーミングファイルをメモリに読み込む */
		work->file_ptr = DEMO_FILE_MEMORY ;
		/* ループによる初期化ならばファイルは読み込まない */
		if ( loop_count == 0 ) {
			_ReadFile( stream_handle, work->file_ptr, &work->file_size );
			de2_size = work->file_size;
		} else {
			printf("Skip read de2 file routin. loop count = %d\n", loop_count);
			work->file_size = de2_size;
		}
		work->next_stream_packet = ( char * )work->file_ptr + 16 ;
	}
#endif

	//デモエフェクトコントロールリスト初期化
	DM_EftControlClearList();
	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		デモ用デーモン起動
	*/
void* DM_StartDeamon( int name, void *stream_handle, int file_flag )
{
   int      i;
	Work		*work ;

	OPERATOR() ;

	/* ２重起動防止 */
	if ( DM_DemoActor != NULL ) return ( DM_DemoActor );

    work = (Work *)GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_OBJECT, 
									sizeof( Work ), 0xF4 ) ;
#ifdef DEBUG_MODE
	work->file_flag = 0;
#endif

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), StartWait, Die ) ;

#ifdef KP_WINDOWS
		/* 稼動フラグON */
		DM_DemoActNow = TRUE ;
#endif

		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	DM_DemoActor = work ;

	return ( work );
}
/* ---------------------------------------------------------------- */
	/*
		デモ再生が行われていればワークのポインタを返す
	*/
DM_WORK* DM_GetDemoWork( void )
{
	/* 起動していなければＮＵＬＬを返す */
	if ( DM_DemoActor == NULL ) return ( NULL );
	/* アクターワークのポインタではなく、
	   その中のデモ管理構造体へのポインタを返すので注意！ */
	return ( &DM_DemoActor->demo_work );
}


/* ---------------------------------------------------------------- */

void* NewPolygonDemo( int name, int where )
{
	Work	*work ;
	char *c;
	int   i ;
	int pos;
	int start_proc = 0;
	int end_proc = 0;
	int flag;

	flag = 0;

#ifdef DEBUG_MODE
	{
		/* デバッグ用コードの初期化 */
		memset(&dm_debug, 0x00, sizeof(DM_DEBUG));
		dm_debug.pad = &GV_PadData[1];
		dm_debug.print_buffer[0] = '\0' ; /* T.Morita Added 2000/08/30 */

		/* ループモード */
		if ( GCL_GetOption('l') != NULL ) dm_debug.play_mode = 1;

		/* 文字列読み込み */
		if((c = GCL_GetOption('s')) != NULL){
			dm_debug.string = GCL_GetNextString();
#ifndef KP_WINDOWS
			DG_UnDrawFrameCount = 0;
#else
		    DG_SetUnDrawFrameCount(0);
#endif
		} else {
			dm_debug.string = NULL;
		}

		/* 30フレームカウント表示のベースタイム指定 */
		if((c = GCL_GetOption('f')) != NULL) {
			if ( GCL_NextStr() != NULL )
				dm_debug.half_frame_base = GCL_GetNextInt();
			else
				dm_debug.half_frame_base = 0;
		}

		/* デモ開始までのタイミングカウンタ */
		if ( (c = GCL_GetOption('t')) != NULL ) {
			if ( GCL_NextStr() != NULL ){
				dm_debug.start_time = GCL_GetNextInt() * TIME_BASE * 10;
			} else {
				dm_debug.start_time = START_TIME * TIME_BASE * 10;
			}

			if ( dm_debug.start_time <= 0 ){
				dm_debug.start_time = START_TIME * TIME_BASE * 10;
			}

			if ( loop_count == 0 ){
				start_count = dm_debug.start_time;
			}
			flag |= GM_STREAM_PLAY_WAIT;
		}
		if( blank_count > 0 ){
			flag |= GM_STREAM_PLAY_WAIT;
		}
	}
#endif

#ifdef DEBUG_MODE
printf( "Demo: Start\n" );
#endif
	if( GCL_GetOption( 'i' ) == NULL ){
		printf( "DEMO:need t: symbol\n" );
		return NULL;
	}
	pos = GCL_GetNextInt();


#if BP_TGS_DEMO()
   // Check for ending TGS demo
   {
      // Lookup associated stream name
      int streamTop = FS_GetStreamTop( FS_FILE_DEMO, pos );
      if( streamTop >= 0 )
      {
         const char* streamPath = BP_FindStreamName( streamTop );
         if( streamPath )
         {
            // Search for just filename and extension without drive or folders
            const char* streamFName = strrchr( streamPath, '/' );

            // Setup demo info
            strcpy( gBP_Demo_StreamName, streamFName ? (streamFName+1) : streamPath );

            // End of TGS demo?
            if( BP_TGS_DEMO_CutsceneBegin() )
            {
               return 0;
            }
         }
      }
   }
#endif

	/* 起動時procの設定 */
	start_proc = GCL_GetOptionValue( 'b', 0 );

	/* 終了時procの指定 */
	if( GCL_GetOption( 'n' ) != NULL ){
		end_proc = GCL_GetNextInt();
		flag |= GM_STREAM_FLAG_PROC( end_proc );
	}

#ifdef DEBUG_MODE
	if( ( GM_DEMO_SkipFlag == 1 && pos < 0 ) || GM_DEMO_SkipFlag == 2 ){
		GCL_ARGS arg;
		int argv[ 1 ];
		extern void *NewDebugPauseProg( char *mesg, int proc, int pause );

		arg.argc = 1;
		arg.argv = argv;
		argv[ 0 ] = 1;	// CANCEL

		if( start_proc != 0 ){
			GCL_ExecProc( start_proc, NULL );
		}
#ifndef KP_WINDOWS
		DG_UnDrawFrameCount = 0;
#else
	    DG_SetUnDrawFrameCount(0);
#endif
		NewDebugPauseProg( "ERROR: NO DEMO DATA !!\n", end_proc, GV_PAUSE_STOP );

		return ( void * )1;
	}
	ASSERT( pos >= 0 );
#endif

	/* デモデーモン起動 */
	work = DM_StartDeamon( name, NULL, 1 );
	if( work == NULL ){
		ASSERT( FALSE );
		return NULL;
	}

	work->begin_proc = start_proc;

	/* undraw flagの設定 */
	work->undraw_flag = GCL_GetOptionValue( 'u', 0x03 );

	if( ( work->undraw_flag & UNDRAW_BEGIN ) != 0
#ifdef DEBUG_MODE
		&& ( dm_debug.string == NULL )
#endif
		){
#ifndef KP_WINDOWS
		DG_UnDrawFrameCount = DG_UNDRAW_MAX;
#else
	    DG_SetUnDrawFrameCount(DG_UNDRAW_MAX);
#endif
		GV_PauseOnActorSystem( GV_PAUSE_STOP );
	}

	/* マップ取得 */
	work->map = where;

	/* ストリームコントローラーへの登録 */
	AddDriver();

	/* デモ起動 */
	{
		int id;
#ifdef DEBUG_MODE
printf( "DEMO : %X\n", pos );
#endif
		if( pos & 0x60000000 ){
			// 音つき
			id = GM_DemoStream( pos, flag );
		} else {
			// 音なし
			id = GM_DemoStreamLight( pos, flag );
		}
		work->stream_id = id;
		GV_SetActorChild( GM_GetStreamActor( id ), work );

		GCL_Status = id;
	}

	/* カメラ属性の取得 */
	/* 起動時procの設定 */
	DM_CameraFlag = GCL_GetOptionValue( 'c', 0 );

#ifdef DEBUG_MODE
	/* メッセージが来るまでウェイトを入れる */
	work->debug_flag = 0;
	if ( GCL_GetOption( 'w' ) ){
		GV_WaitMessage( work, name );
	}
	{
		/* バグチェック用処理 */
		char	file_path[128], *filename = "";

		if ( GCL_GetOption('d') != NULL ){
			work->debug_flag = 1;
			filename = GCL_GetNextString();
			/* ファイルパスの生成 */
			sprintf( file_path, "host0:stage/%s/%s.de2", GM_GetArea(), filename );
		}

		/* エフェクトリスト表示 */
		if((c = GCL_GetOption('e')) != NULL) {
			if ( GCL_NextStr() != NULL ){
				dm_debug.view_time = GCL_GetNextInt();
			} else {
				dm_debug.view_time = 30;
			}

			/* ファイルパスの生成 */
			sprintf( file_path, "host0:../list/demolist/%s.lst", filename );
			GetEffectList( work, file_path );
			for ( i = 0; i < dm_debug.n_effect; i++ ) {
				printf("Effect Number = %d : %s : %s\n", i,
					   dm_debug.effect_list[i].id, dm_debug.effect_list[i].name);
			}
		}
		/* デモ表示 初期状態指定 */
		if((c = GCL_GetOption('v')) != NULL) {
			if ( GCL_NextStr() != NULL )
				dm_debug.view_mode = GCL_GetNextInt() - 1;
			else
				dm_debug.view_mode = 0;

			if ( dm_debug.view_mode < 0 ) {
				dm_debug.view_mode = VIEW_MODE_MAX;
			} else if ( dm_debug.view_mode == (VIEW_MODE_MAX - 1) ) {
				if ( dm_debug.n_effect == 0 )
					dm_debug.view_mode = VIEW_EFFECT - 1;
			} else if ( dm_debug.view_mode > VIEW_MODE_MAX )
				dm_debug.view_mode = VIEW_LEVEL0;
		}
	}
#endif

   //BP_CAMERA - adding extra demo vars for tracking camera cuts
   {
      // Lookup associated stream name
      const char* streamPath = GM_GetStreamName( work->stream_id );
      if( streamPath )
      {
         // Search for just filename and extension without drive or folders
         const char* streamFName = strrchr( streamPath, '/' );

         // Setup demo info
         gBP_Demo_Active = 1;
         strcpy( gBP_Demo_StreamName, streamFName ? (streamFName+1) : streamPath );
         gBP_Demo_Id = BP_Camera_GetDemoId();
         gBP_Demo_Frame = -1;
      }
   }
   //BP_CAMERA - adding extra camera name/id for tweak system

   return ( work );
}

void* NewPolygonDemoStart( int name, int where )
{
	return NewPolygonDemo( name, where );
}



#ifdef DEBUG_MODE
/* デバッグ用コード */

/* ---------------------------------------------------------------- */
	/*
		lst ファイルからエフェクトのリストを得る
	*/
static int GetEffectList( Work *work, void *file_path ) {
	int  i, j;
	int  n_effect = 0;
	int  file_size, array_size;
	char *ptr;

	ptr = ( char * )DEMO_FILE_MEMORY + work->file_size;
	if ( _ReadFile( file_path, (void*)ptr, &file_size ) == 0 )
		return 0;
	for ( i = 0; i < file_size; i++ ) {
		/* "0x" をキーワードとして検索 */
		if ( (ptr[i] == '0') && ( ptr[i + 1] == 'x' ) ) {
			/* IDを格納 */
			for ( i += 2, j = 0; ( i < file_size ) && ( j < 8 ) ; i++, j++ )
				dm_debug.effect_list[n_effect].id[j] = ptr[i];
			dm_debug.effect_list[n_effect].id[j] = '\0';
			/* 最初の方の０は空白に置き換える */
			array_size = sizeof(dm_debug.effect_list[0].id);
			for ( j = 0; j < array_size; j++ ) {
				if( dm_debug.effect_list[n_effect].id[j] == '0' )
					dm_debug.effect_list[n_effect].id[j] = ' ';
				else
					break;
			}

			/* 空白を飛ばす */
			while ( isalpha( (int)(ptr[i]) ) == 0 ) i++;

			/* 最初の文字列を飛ばす */
			while ( isspace( (int)(ptr[i]) ) == 0 ) i++;

			/* 空白を飛ばす */
			while ( isalpha( (int)(ptr[i]) ) == 0 ) i++;

			/* 名称を格納 */
			array_size = sizeof(dm_debug.effect_list[0].name);
			for( j = 0; ( i < file_size ) && ( j < array_size ) ; i++, j++ ) {
				if ( isspace( (int)(ptr[i]) ) != 0 ) break;
				dm_debug.effect_list[n_effect].name[j] = ptr[i];
			}
			dm_debug.effect_list[n_effect].name[j] = '\0';
			n_effect++;
		}
	}
	dm_debug.n_effect = n_effect;
	return 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		現在のカメラのパラメータを得る
	*/
static void DemoDebug_GetNowCameraData(void)
{
    GM_CameraSet *now;

    /* 現行カメラから */
    now = GM_GetCurrentCameraSet( 0 ) ;

    fpu_CopyVector(&(dm_debug.fcam_pos),&(now->position));
    fpu_CopyVector(&(dm_debug.ftrg_pos),&(now->target));
	dm_debug.cam_len=now->track;
    dm_debug.zoom=now->angle/2.0f;
}

/* ---------------------------------------------------------------- */
	/*
		カメラの回転移動のPositionとTargetを計算する
	*/
static void Demo_DebugCameraRotate(void)
{
    FMATRIX mat;
	FVECTOR vec;
    SVECTOR rot;
    int speed_rot;
    float speed;

    rot=DG_ZeroSVector;

	/* カメラスピード切り換え */
    if(dm_debug.speed_type){
		speed_rot=48;
		speed=50.0f*2.0f;
    }
    else{
		speed_rot=24;
		speed=10.0f*2.0f;
    }

	/* 十字キー入力処理 */
	switch(dm_debug.pad->status & (PAD_U|PAD_D)){
	case PAD_U:
		rot.vx+=speed_rot;
		break;
	case PAD_D:
		rot.vx-=speed_rot;
		break;
	}
	switch(dm_debug.pad->status & (PAD_L|PAD_R)){
	case PAD_L:
		rot.vy-=speed_rot;
		break;
	case PAD_R:
		rot.vy+=speed_rot;
		break;
	}

	/* 操作対象の切り換え */
	/* 注視点 → 視点 */
	switch(dm_debug.mode2){
	case 0:
		/* 注視点移動 */

		/* カメラ距離変更 */
		/* Move Len */
		switch(dm_debug.pad->status & (PAD_X|PAD_Y)){
		case PAD_X:
			dm_debug.cam_len-=speed;
			if(dm_debug.cam_len<100.0f) dm_debug.cam_len=100.0f;
			break;
		case PAD_Y:
			dm_debug.cam_len+=speed;
			if(dm_debug.cam_len>100000.0f) dm_debug.cam_len=100000.0f;
			break;
		}

		/* Center = Target */
		rot.vx=-rot.vx;
		rot.vy=-rot.vy;

		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&mat);

		vec.vx=vec.vy=0.0f;
		vec.vz=-dm_debug.cam_len;
		vec.vw=0.0f;

		vu0_Ldm1(&mat);
		vu0_Ldm0(&(DG_Chanls->eye));
		vu0_Ldv0(&vec);
		vu0_Ldv1(&(dm_debug.ftrg_pos));
		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		vu0_Addv0v1();

		vu0_Stv0(&(dm_debug.fcam_pos));
		break;

	case 1:
		/* 視点移動 */

		/* Center = Camera */
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&mat);

		vec.vx=vec.vy=0.0f;
		vec.vz=dm_debug.cam_len;
		vec.vw=1.0f;

		vu0_Ldm1(&mat);
		vu0_Ldm0(&(DG_Chanls->eye));
		vu0_Ldv0(&vec);
		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		vu0_Stv0(&(dm_debug.ftrg_pos));
		break;
	}
}

/* ---------------------------------------------------------------- */
	/*
		カメラの平行移動のPositionとTargetを計算する
	*/
static void Demo_DebugCameraMove(void)
{
	/* 平行移動と回転移動がある */
	/* shift → rotate */
	switch(dm_debug.mode2){
	case 0:
		{
			float speed;

			/* スピード切り換え */
			if(dm_debug.speed_type){
				speed=50.0f*5.0f;
			}
			else{
				speed=10.0f*5.0f;
			}

			/* 平行移動 */
			/* shift */
			switch(dm_debug.pad->status & (PAD_U|PAD_D)){
			case PAD_U:
				dm_debug.fcam_pos.vx-=DG_Chanls->eye.m[1][0]*speed;
				dm_debug.fcam_pos.vy-=DG_Chanls->eye.m[1][1]*speed;
				dm_debug.fcam_pos.vz-=DG_Chanls->eye.m[1][2]*speed;

				dm_debug.ftrg_pos.vx-=DG_Chanls->eye.m[1][0]*speed;
				dm_debug.ftrg_pos.vy-=DG_Chanls->eye.m[1][1]*speed;
				dm_debug.ftrg_pos.vz-=DG_Chanls->eye.m[1][2]*speed;
				break;
			case PAD_D:
				dm_debug.fcam_pos.vx+=DG_Chanls->eye.m[1][0]*speed;
				dm_debug.fcam_pos.vy+=DG_Chanls->eye.m[1][1]*speed;
				dm_debug.fcam_pos.vz+=DG_Chanls->eye.m[1][2]*speed;

				dm_debug.ftrg_pos.vx+=DG_Chanls->eye.m[1][0]*speed;
				dm_debug.ftrg_pos.vy+=DG_Chanls->eye.m[1][1]*speed;
				dm_debug.ftrg_pos.vz+=DG_Chanls->eye.m[1][2]*speed;
				break;
			}
			switch(dm_debug.pad->status & (PAD_L|PAD_R)){
			case PAD_L:
				dm_debug.fcam_pos.vx-=DG_Chanls->eye.m[0][0]*speed;
				dm_debug.fcam_pos.vy-=DG_Chanls->eye.m[0][1]*speed;
				dm_debug.fcam_pos.vz-=DG_Chanls->eye.m[0][2]*speed;

				dm_debug.ftrg_pos.vx-=DG_Chanls->eye.m[0][0]*speed;
				dm_debug.ftrg_pos.vy-=DG_Chanls->eye.m[0][1]*speed;
				dm_debug.ftrg_pos.vz-=DG_Chanls->eye.m[0][2]*speed;
				break;
			case PAD_R:
				dm_debug.fcam_pos.vx+=DG_Chanls->eye.m[0][0]*speed;
				dm_debug.fcam_pos.vy+=DG_Chanls->eye.m[0][1]*speed;
				dm_debug.fcam_pos.vz+=DG_Chanls->eye.m[0][2]*speed;

				dm_debug.ftrg_pos.vx+=DG_Chanls->eye.m[0][0]*speed;
				dm_debug.ftrg_pos.vy+=DG_Chanls->eye.m[0][1]*speed;
				dm_debug.ftrg_pos.vz+=DG_Chanls->eye.m[0][2]*speed;
				break;
			}
			switch(dm_debug.pad->status & (PAD_X|PAD_Y)){
			case PAD_X:
				dm_debug.fcam_pos.vx+=DG_Chanls->eye.m[2][0]*speed;
				dm_debug.fcam_pos.vy+=DG_Chanls->eye.m[2][1]*speed;
				dm_debug.fcam_pos.vz+=DG_Chanls->eye.m[2][2]*speed;

				dm_debug.ftrg_pos.vx+=DG_Chanls->eye.m[2][0]*speed;
				dm_debug.ftrg_pos.vy+=DG_Chanls->eye.m[2][1]*speed;
				dm_debug.ftrg_pos.vz+=DG_Chanls->eye.m[2][2]*speed;
				break;
			case PAD_Y:
				dm_debug.fcam_pos.vx-=DG_Chanls->eye.m[2][0]*speed;
				dm_debug.fcam_pos.vy-=DG_Chanls->eye.m[2][1]*speed;
				dm_debug.fcam_pos.vz-=DG_Chanls->eye.m[2][2]*speed;

				dm_debug.ftrg_pos.vx-=DG_Chanls->eye.m[2][0]*speed;
				dm_debug.ftrg_pos.vy-=DG_Chanls->eye.m[2][1]*speed;
				dm_debug.ftrg_pos.vz-=DG_Chanls->eye.m[2][2]*speed;
				break;
			}
		}
		break;

	case 1:
		/* 視点回転移動 */
		/* rotate */
		{
			FMATRIX mat;
			FVECTOR vec;
			SVECTOR rot;
			int speed_rot;

			rot=DG_ZeroSVector;

			if(dm_debug.speed_type){
				speed_rot=48;
			}
			else{
				speed_rot=24;
			}

			switch(dm_debug.pad->status & (PAD_U|PAD_D)){
			case PAD_U:
				rot.vx+=speed_rot;
				break;
			case PAD_D:
				rot.vx-=speed_rot;
				break;
			}
			switch(dm_debug.pad->status & (PAD_L|PAD_R)){
			case PAD_L:
				rot.vy-=speed_rot;
				break;
			case PAD_R:
				rot.vy+=speed_rot;
				break;
			}

			/* Center = Camera */
			DG_SetPos2(&DG_ZeroVector,&rot);
			DG_GetPos(&mat);

			vec.vx=vec.vy=0.0f;
			vec.vz=dm_debug.cam_len;
			vec.vw=1.0f;

			vu0_Ldm1(&mat);
			vu0_Ldm0(&(DG_Chanls->eye));
			vu0_Ldv0(&vec);
			vu0_Mulm2m0m1();
			vu0_Mulv0m2v0();

			vu0_Stv0(&(dm_debug.ftrg_pos));
		}
		break;
	}
}

/* ---------------------------------------------------------------- */
	/*
		環境光のカラーを変更する。
	*/
static void Demo_DebugEditAmbient(void)
{
	int param;
	int ambient_col[3];

	/* 現在の設定値を取得 */
	ambient_col[0] = (int)DG_ColorMatrix.m[3][0];
	ambient_col[1] = (int)DG_ColorMatrix.m[3][1];
	ambient_col[2] = (int)DG_ColorMatrix.m[3][2];

	/* デバッグモードで設定した値でないのなら、デフォルト値として取得 */
	if( (dm_debug.ambient_col[0] != (int)DG_ColorMatrix.m[3][0]) ||
		(dm_debug.ambient_col[1] != (int)DG_ColorMatrix.m[3][1]) ||
		(dm_debug.ambient_col[2] != (int)DG_ColorMatrix.m[3][2]))
	{
		dm_debug.ambient_col_def[0] = (int)DG_ColorMatrix.m[3][0];
		dm_debug.ambient_col_def[1] = (int)DG_ColorMatrix.m[3][1];
		dm_debug.ambient_col_def[2] = (int)DG_ColorMatrix.m[3][2];
	}

	/* 変化率 */
	if(dm_debug.speed_type){
		/* 速い */
		param = 16;
	}
	else{
		/* 通常 */
		param = 1;
	}

	/* 編集対象 */
	switch(dm_debug.pad->press & (PAD_L|PAD_R)){
	case PAD_R:
		dm_debug.ambient_status++;
		if(dm_debug.ambient_status > 4) dm_debug.ambient_status = 0;
		break;
	case PAD_L:
		dm_debug.ambient_status--;
		if(dm_debug.ambient_status < 0) dm_debug.ambient_status = 4;
		break;
	}

	/* 値の変更 */
	switch(dm_debug.ambient_status){
	case 0:	/* R */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			ambient_col[0] -= param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[0] &= 0xfffffff0;
			if(ambient_col[0] < 0) ambient_col[0] = 0;
			break;
		case PAD_U:
			ambient_col[0] += param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[0] &= 0xfffffff0;
			if(ambient_col[0] > 255) ambient_col[0] = 255;
			break;
		}
		break;

	case 1:	/* G */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			ambient_col[1] -= param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[1] &= 0xfffffff0;
			if(ambient_col[1] < 0) ambient_col[1] = 0;
			break;
		case PAD_U:
			ambient_col[1] += param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[1] &= 0xfffffff0;
			if(ambient_col[1] > 255) ambient_col[1] = 255;
			break;
		}
		break;

	case 2:	/* B */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			ambient_col[2] -= param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[2] &= 0xfffffff0;
			if(ambient_col[2] < 0) ambient_col[2] = 0;
			break;
		case PAD_U:
			ambient_col[2] += param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[2] &= 0xfffffff0;
			if(ambient_col[2] > 255) ambient_col[2] = 255;
			break;
		}
		break;

	case 3:	/* RGB */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			ambient_col[0] -= param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[0] &= 0xfffffff0;
			if(ambient_col[0] < 0) ambient_col[0] = 0;
			ambient_col[2] = ambient_col[1] = ambient_col[0];
			break;
		case PAD_U:
			ambient_col[0] += param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) ambient_col[0] &= 0xfffffff0;
			if(ambient_col[0] > 255) ambient_col[0] = 255;
			ambient_col[2] = ambient_col[1] = ambient_col[0];
			break;
		}
		break;

	case 4:	/* RGB+ */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			if( ((ambient_col[0] - param) < 0) ||
				((ambient_col[1] - param) < 0) ||
				((ambient_col[2] - param) < 0))
				break;
		
			ambient_col[0] -= param;
			ambient_col[1] -= param;
			ambient_col[2] -= param;
			if(ambient_col[0] < 0) ambient_col[0] = 0;
			if(ambient_col[1] < 0) ambient_col[1] = 0;
			if(ambient_col[2] < 0) ambient_col[2] = 0;
			break;
		case PAD_U:
			if( ((ambient_col[0] + param) > 255) ||
				((ambient_col[1] + param) > 255) ||
				((ambient_col[2] + param) > 255))
				break;

			ambient_col[0] += param;
			ambient_col[1] += param;
			ambient_col[2] += param;
			if(ambient_col[0] > 255) ambient_col[0] = 255;
			if(ambient_col[1] > 255) ambient_col[1] = 255;
			if(ambient_col[2] > 255) ambient_col[2] = 255;
			break;
		}
		break;
	}

	/* 変更された？ */
	if( (dm_debug.ambient_col[0] != ambient_col[0]) ||
		(dm_debug.ambient_col[1] != ambient_col[1]) ||
		(dm_debug.ambient_col[2] != ambient_col[2]))
		dm_debug.sAmbientFlag |= FLAG_AMBIENT_COLOR;

	if(dm_debug.pad->status & PAD_Y) {
		/* パラメータをリセット */
		ambient_col[0] = dm_debug.ambient_col_def[0];
		ambient_col[1] = dm_debug.ambient_col_def[1];
		ambient_col[2] = dm_debug.ambient_col_def[2];

		/* フラグもリセット */
		dm_debug.sAmbientFlag &= ~FLAG_AMBIENT_COLOR;
	}

	DG_SetAmbient(ambient_col[0], ambient_col[1], ambient_col[2]);
	dm_debug.ambient_col[0] = ambient_col[0];
	dm_debug.ambient_col[1] = ambient_col[1];
	dm_debug.ambient_col[2] = ambient_col[2];
}

/* ---------------------------------------------------------------- */
	/*
		フォグの設定値を入力値に戻して返す。値は四捨五入し、一の位は落とす
	*/
static inline int GetFogNear()
{
	float near;
	near = (255.0f - DG_FogParam2) / DG_FogParam1;
	if(near < 0) return (int)(near - 0.5f);
	else         return (int)(near + 0.5f);
}

static inline int GetFogFar()
{
	float nearf;
	float farf;
	nearf = (255.0f - DG_FogParam2) / DG_FogParam1;
	farf  = nearf - (255.0f / DG_FogParam1);
	if(farf < 0) return (int)(farf - 0.5f);
	else        return (int)(farf + 0.5f);
}

/* ---------------------------------------------------------------- */
	/*
		フォグのカラー設定を変更する
	*/
static void Demo_DebugEditFog(void)
{
	int color_param;
	int fog_col[3];

	/* 現在の設定値を取得 */
	fog_col[0] = DG_FogColor.r;
	fog_col[1] = DG_FogColor.g;
	fog_col[2] = DG_FogColor.b;

	/* デバッグモードで設定した値でないのなら、デフォルト値として取得 */
	if( (dm_debug.fog_col[0] != DG_FogColor.r) ||
		(dm_debug.fog_col[1] != DG_FogColor.g) ||
		(dm_debug.fog_col[2] != DG_FogColor.b))
	{
		dm_debug.fog_col_def[0] = DG_FogColor.r;
		dm_debug.fog_col_def[1] = DG_FogColor.g;
		dm_debug.fog_col_def[2] = DG_FogColor.b;
	}

	/* 変化率 */
	if(dm_debug.speed_type){
		/* 速い */
		color_param = 16;
	}
	else{
		/* 通常 */
		color_param = 1;
	}

	/* 編集対象 */
	switch(dm_debug.pad->press & (PAD_L|PAD_R)){
	case PAD_R:
		dm_debug.fog_status++;
		if(dm_debug.fog_status > 6) dm_debug.fog_status = 0;
		break;
	case PAD_L:
		dm_debug.fog_status--;
		if(dm_debug.fog_status < 0) dm_debug.fog_status = 6;
		break;
	}

	/* 値の変更 */
	switch(dm_debug.fog_status){
	case 0:	/* R */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_col[0] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[0] &= 0xfffffff0;
			if(fog_col[0] < 0) fog_col[0] = 0;
			break;
		case PAD_U:
			fog_col[0] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[0] &= 0xfffffff0;
			if(fog_col[0] > 255) fog_col[0] = 255;
			break;
		}
		break;

	case 1:	/* G */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_col[1] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[1] &= 0xfffffff0;
			if(fog_col[1] < 0) fog_col[1] = 0;
			break;
		case PAD_U:
			fog_col[1] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[1] &= 0xfffffff0;
			if(fog_col[1] > 255) fog_col[1] = 255;
			break;
		}
		break;

	case 2:	/* B */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_col[2] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[2] &= 0xfffffff0;
			if(fog_col[2] < 0) fog_col[2] = 0;
			break;
		case PAD_U:
			fog_col[2] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[2] &= 0xfffffff0;
			if(fog_col[2] > 255) fog_col[2] = 255;
			break;
		}
		break;

	case 3:	/* RGB */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_col[0] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[0] &= 0xfffffff0;
			if(fog_col[0] < 0) fog_col[0] = 0;
			fog_col[2] = fog_col[1] = fog_col[0];
			break;
		case PAD_U:
			fog_col[0] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) fog_col[0] &= 0xfffffff0;
			if(fog_col[0] > 255) fog_col[0] = 255;
			fog_col[2] = fog_col[1] = fog_col[0];
			break;
		}
		break;

	case 4:	/* RGB+ */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			if( ((fog_col[0] - color_param) < 0) ||
				((fog_col[1] - color_param) < 0) ||
				((fog_col[2] - color_param) < 0))
				break;
		
			fog_col[0] -= color_param;
			fog_col[1] -= color_param;
			fog_col[2] -= color_param;
			if(fog_col[0] < 0) fog_col[0] = 0;
			if(fog_col[1] < 0) fog_col[1] = 0;
			if(fog_col[2] < 0) fog_col[2] = 0;
			break;
		case PAD_U:
			if( ((fog_col[0] + color_param) > 255) ||
				((fog_col[1] + color_param) > 255) ||
				((fog_col[2] + color_param) > 255))
				break;

			fog_col[0] += color_param;
			fog_col[1] += color_param;
			fog_col[2] += color_param;
			if(fog_col[0] > 255) fog_col[0] = 255;
			if(fog_col[1] > 255) fog_col[1] = 255;
			if(fog_col[2] > 255) fog_col[2] = 255;
			break;
		}
		break;
	}

	/* 変更された？ */
	if( (dm_debug.fog_col[0] != fog_col[0]) ||
		(dm_debug.fog_col[1] != fog_col[1]) ||
		(dm_debug.fog_col[2] != fog_col[2]))
		dm_debug.sFogFlag |= FLAG_FOG_COLOR;


	if(dm_debug.pad->status & PAD_Y) {
		/* パラメータをリセット */
		fog_col[0] = dm_debug.fog_col_def[0];
		fog_col[1] = dm_debug.fog_col_def[1];
		fog_col[2] = dm_debug.fog_col_def[2];

		/* フラグもリセット */
		dm_debug.sFogFlag &= ~FLAG_FOG_COLOR;
	}

	/* フォグカラーの設定 */
	DG_SetFogColor(fog_col[0], fog_col[1], fog_col[2]);
	dm_debug.fog_col[0] = fog_col[0];
	dm_debug.fog_col[1] = fog_col[1];
	dm_debug.fog_col[2] = fog_col[2];
}

/* ---------------------------------------------------------------- */
	/*
		フォグのパラメータ設定を変更する
	*/
static void Demo_DebugEditFogParam(void)
{
	int near_param, far_param;
	int fog_near, fog_far;
	int fog_near_tmp, fog_far_tmp;

	/* 現在の設定値を取得 */
	fog_near = fog_near_tmp = GetFogNear();
	fog_far  = fog_far_tmp  = GetFogFar();

	/* デバッグモードで設定した値でないのなら、デフォルト値として取得 */
	if( (dm_debug.fog_near   != DG_FogParam1)  ||
		(dm_debug.fog_far    != DG_FogParam2))
	{
		dm_debug.fog_near_def = fog_near;
		dm_debug.fog_far_def  = fog_far;
	}

	/* 変化率 */
	if(dm_debug.speed_type){
		/* 速い */
		near_param  = 200;
		far_param   = 200;
	}
	else{
		/* 通常 */
		near_param  = 10;
		far_param   = 10;
	}

	/* 編集対象 */
	switch(dm_debug.pad->press & (PAD_L|PAD_R)){
	case PAD_R:
		dm_debug.fog_param_status++;
		if(dm_debug.fog_param_status > 1) dm_debug.fog_param_status = 0;
		break;
	case PAD_L:
		dm_debug.fog_param_status--;
		if(dm_debug.fog_param_status < 0) dm_debug.fog_param_status = 1;
		break;
	}

	/* 値の変更 */
	switch(dm_debug.fog_param_status){
	case 0:	/* near */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_near -= near_param;
			break;
		case PAD_U:
			fog_near += near_param;
			break;
		}
		break;

	case 1:	/* far */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			fog_far -= far_param;
			break;
		case PAD_U:
			fog_far += far_param;
			break;
		}
		break;
	}

	/* 変更された？ */
	if( (fog_near != fog_near_tmp) || (fog_far != fog_far_tmp))
		dm_debug.sFogFlag |= FLAG_FOG_LANGE;

	if(dm_debug.pad->status & PAD_Y) {
		/* パラメータをリセット */
		fog_near   = dm_debug.fog_near_def;
		fog_far    = dm_debug.fog_far_def;

		/* フラグもリセット */
		dm_debug.sFogFlag &= ~FLAG_FOG_LANGE;
	}

	/* フォグ係数の設定 */
	DG_SetFogParam(fog_near, fog_far);
	dm_debug.fog_near = DG_FogParam1;
	dm_debug.fog_far  = DG_FogParam2;
}

/* ---------------------------------------------------------------- */
	/*
		平行光源のカラー設定を変更する
	*/
static void Demo_DebugEditParallel(void)
{
	int color_param;
	int parallel_col[3];

	/* 現在の設定値を取得 */
	parallel_col[0] = (int)DG_ColorMatrix.m[0][0];
	parallel_col[1] = (int)DG_ColorMatrix.m[0][1];
	parallel_col[2] = (int)DG_ColorMatrix.m[0][2];

	/* デバッグモードで設定した値でないのなら、デフォルト値として取得 */
	if( (dm_debug.parallel_col[0] != (int)DG_ColorMatrix.m[0][0]) ||
		(dm_debug.parallel_col[1] != (int)DG_ColorMatrix.m[0][1]) ||
		(dm_debug.parallel_col[2] != (int)DG_ColorMatrix.m[0][2]))
	{
		dm_debug.parallel_col_def[0] = (int)DG_ColorMatrix.m[0][0];
		dm_debug.parallel_col_def[1] = (int)DG_ColorMatrix.m[0][1];
		dm_debug.parallel_col_def[2] = (int)DG_ColorMatrix.m[0][2];
	}

	/* 変化率 */
	if(dm_debug.speed_type){
		/* 速い */
		color_param = 16;
	}
	else{
		/* 通常 */
		color_param = 1;
	}

	/* 編集対象 */
	switch(dm_debug.pad->press & (PAD_L|PAD_R)){
	case PAD_R:
		dm_debug.parallel_status++;
		if(dm_debug.parallel_status > 4) dm_debug.parallel_status = 0;
		break;
	case PAD_L:
		dm_debug.parallel_status--;
		if(dm_debug.parallel_status < 0) dm_debug.parallel_status = 4;
		break;
	}

	/* 値の変更 */
	switch(dm_debug.parallel_status){
	case 0:	/* R */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			parallel_col[0] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[0] &= 0xfffffff0;
			if(parallel_col[0] < 0) parallel_col[0] = 0;
			break;
		case PAD_U:
			parallel_col[0] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[0] &= 0xfffffff0;
			if(parallel_col[0] > 255) parallel_col[0] = 255;
			break;
		}
		break;

	case 1:	/* G */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			parallel_col[1] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[1] &= 0xfffffff0;
			if(parallel_col[1] < 0) parallel_col[1] = 0;
			break;
		case PAD_U:
			parallel_col[1] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[1] &= 0xfffffff0;
			if(parallel_col[1] > 255) parallel_col[1] = 255;
			break;
		}
		break;

	case 2:	/* B */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			parallel_col[2] -= color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[2] &= 0xfffffff0;
			if(parallel_col[2] < 0) parallel_col[2] = 0;
			break;
		case PAD_U:
			parallel_col[2] += color_param;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[2] &= 0xfffffff0;
			if(parallel_col[2] > 255) parallel_col[2] = 255;
			break;
		}
		break;

	case 3:	/* RGB */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			parallel_col[0] -= color_param;
			if(parallel_col[0] < 0) parallel_col[0] = 0;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[0] &= 0xfffffff0;
			parallel_col[2] = parallel_col[1] = parallel_col[0];
			break;
		case PAD_U:
			parallel_col[0] += color_param;
			if(parallel_col[0] > 255) parallel_col[0] = 255;
			/* ハイスピードの時は16の倍数にそろえる */
			if(dm_debug.speed_type != 0) parallel_col[0] &= 0xfffffff0;
			parallel_col[2] = parallel_col[1] = parallel_col[0];
			break;
		}
		break;

	case 4:	/* RGB+ */
		switch(dm_debug.pad->status & (PAD_U|PAD_D)){
		case PAD_D:
			if( ((parallel_col[0] - color_param) < 0) ||
				((parallel_col[1] - color_param) < 0) ||
				((parallel_col[2] - color_param) < 0))
				break;
		
			parallel_col[0] -= color_param;
			parallel_col[1] -= color_param;
			parallel_col[2] -= color_param;
			if(parallel_col[0] < 0) parallel_col[0] = 0;
			if(parallel_col[1] < 0) parallel_col[1] = 0;
			if(parallel_col[2] < 0) parallel_col[2] = 0;
			break;
		case PAD_U:
			if( ((parallel_col[0] + color_param) > 255) ||
				((parallel_col[1] + color_param) > 255) ||
				((parallel_col[2] + color_param) > 255))
				break;

			parallel_col[0] += color_param;
			parallel_col[1] += color_param;
			parallel_col[2] += color_param;
			if(parallel_col[0] > 255) parallel_col[0] = 255;
			if(parallel_col[1] > 255) parallel_col[1] = 255;
			if(parallel_col[2] > 255) parallel_col[2] = 255;
			break;
		}
		break;

	}

	/* 修正された？ */
	if( (dm_debug.parallel_col[0] != parallel_col[0]) ||
		(dm_debug.parallel_col[1] != parallel_col[1]) ||
		(dm_debug.parallel_col[2] != parallel_col[2]))
		dm_debug.sParallelFlag |= FLAG_PARALLEL_COLOR;

	if(dm_debug.pad->status & PAD_Y) {
		/* パラメータをリセット */
		parallel_col[0] = dm_debug.parallel_col_def[0];
		parallel_col[1] = dm_debug.parallel_col_def[1];
		parallel_col[2] = dm_debug.parallel_col_def[2];

		/* フラグもリセット */
		dm_debug.sParallelFlag &= ~FLAG_PARALLEL_COLOR;
	}

	/* 平行光源カラーの設定 */
	DG_SetMainLightCol(parallel_col[0], parallel_col[1], parallel_col[2]);
	dm_debug.parallel_col[0] = parallel_col[0];
	dm_debug.parallel_col[1] = parallel_col[1];
	dm_debug.parallel_col[2] = parallel_col[2];
}

/* ---------------------------------------------------------------- */
	/*
		平行光源のベクトル設定を変更する
	*/
static void Demo_DebugEditParallel_Rot(void)
{
	int range_param;
	int parallel_rot[2];
	int parallel_vec[3];

	/* 現在の設定値を取得 */
	parallel_vec[0] = (int)(DG_LightMatrix.m[0][0] * 5000.0f);
	parallel_vec[1] = (int)(DG_LightMatrix.m[1][0] * 5000.0f);
	parallel_vec[2] = (int)(DG_LightMatrix.m[2][0] * 5000.0f);
	parallel_rot[0] = dm_debug.parallel_rot[0];
	parallel_rot[1] = dm_debug.parallel_rot[1];

	/* デバッグモードで設定した値でないのなら、デフォルト値として取得 */
	if( (dm_debug.parallel_vec[0] != DG_LightMatrix.m[0][0]) ||
		(dm_debug.parallel_vec[1] != DG_LightMatrix.m[1][0]) ||
		(dm_debug.parallel_vec[2] != DG_LightMatrix.m[2][0]))
	{
		int   vec;
		float ftemp_r;

		dm_debug.parallel_vec_def[0] = (int)(DG_LightMatrix.m[0][0] * 5000.0f);
		dm_debug.parallel_vec_def[1] = (int)(DG_LightMatrix.m[1][0] * 5000.0f);
		dm_debug.parallel_vec_def[2] = (int)(DG_LightMatrix.m[2][0] * 5000.0f);

		dm_debug.parallel_rot_def[0] = (int)(asinf((float)-dm_debug.parallel_vec_def[1] / 5000.0f) * 2048.0f / M_PI);

		ftemp_r = 5000.0f * cosf(dm_debug.parallel_rot_def[0] * M_PI / 2048.0f);
		if(ftemp_r == 0)
			dm_debug.parallel_rot_def[1] = 0;
		else
			dm_debug.parallel_rot_def[1] = (int)(asinf((float)dm_debug.parallel_vec_def[0] / ftemp_r) * 2048.0f / M_PI);

		/* 180度違いの値になる可能性があるのでそれをチェック */
		vec = ftemp_r * cosf(dm_debug.parallel_rot_def[1] * M_PI / 2048.0f);

		if((vec * dm_debug.parallel_vec_def[2]) < 0) {
			if( dm_debug.parallel_rot_def[1] < 0) dm_debug.parallel_rot_def[1] *= -1;

			if( dm_debug.parallel_rot_def[1] < 2048)
				dm_debug.parallel_rot_def[1] += 2048;
			else
				dm_debug.parallel_rot_def[1] -= 2048;
		}

		dm_debug.parallel_rot[0] = dm_debug.parallel_rot_def[0];
		dm_debug.parallel_rot[1] = dm_debug.parallel_rot_def[1];
	}

	/* 変化率 */
	if(dm_debug.speed_type){
		/* 速い */
		range_param = 100;
	}
	else{
		/* 通常 */
		range_param = 10;
	}

	/* 値の変更 */
	switch(dm_debug.pad->status & (PAD_U|PAD_D|PAD_L|PAD_R)){
	case PAD_D:
		dm_debug.parallel_rot[0] -= range_param;
		if(dm_debug.parallel_rot[0] < 0) dm_debug.parallel_rot[0] = 4095;
		break;
	case PAD_U:
		dm_debug.parallel_rot[0] += range_param;
		if(dm_debug.parallel_rot[0] > 4095) dm_debug.parallel_rot[0] = 0;
		break;
	case PAD_L:
		dm_debug.parallel_rot[1] -= range_param;
		if(dm_debug.parallel_rot[1] < 0) dm_debug.parallel_rot[1] = 4095;
		break;
	case PAD_R:
		( char * )dm_debug.parallel_rot[1] += range_param;
		if(dm_debug.parallel_rot[1] > 4095) dm_debug.parallel_rot[1] = 0;
		break;
	}

	/* 変更された？ */
	if( (dm_debug.parallel_rot[0] != parallel_rot[0]) ||
		(dm_debug.parallel_rot[1] != parallel_rot[1]))
		dm_debug.sParallelFlag |= FLAG_PARALLEL_DIR;

	if(dm_debug.pad->status & PAD_Y) {
		/* パラメータをリセット */
		dm_debug.parallel_rot[0] = dm_debug.parallel_rot_def[0];
		dm_debug.parallel_rot[1] = dm_debug.parallel_rot_def[1];

		/* フラグもリセット */
		dm_debug.sParallelFlag &= ~FLAG_PARALLEL_DIR;
	}

	{
	    float ftemp_radius;
		float ftemp_r;

	    ftemp_radius = 5000.0f;

		parallel_vec[1] = -ftemp_radius * sinf(dm_debug.parallel_rot[0] * M_PI / 2048.0f);
		ftemp_r = ftemp_radius * cosf(dm_debug.parallel_rot[0] * M_PI / 2048.0f);

		parallel_vec[0] = ftemp_r * sinf(dm_debug.parallel_rot[1] * M_PI / 2048.0f);
		parallel_vec[2] = ftemp_r * cosf(dm_debug.parallel_rot[1] * M_PI / 2048.0f);
	}

	/* 平行光源の方向の設定 */
	DG_SetMainLightDir(parallel_vec[0], parallel_vec[1], parallel_vec[2]);
	dm_debug.parallel_vec[0] = DG_LightMatrix.m[0][0];
	dm_debug.parallel_vec[1] = DG_LightMatrix.m[1][0];
	dm_debug.parallel_vec[2] = DG_LightMatrix.m[2][0];
}

/* ---------------------------------------------------------------- */
	/*
		スロー・逆転再生の実処理部分
	*/
static void Demo_DebugPlay( Work *work )
{
	void        *stream_tmp;
	DEMO_PACKET *packet;

	/* デバッグ用コード・スロー再生用 */
	if ( work->stream_top != NULL ) {
		/* デモを停止 */
		if ( dm_debug.pre_tick != 0 )
			work->start_time += DG_TickCount - dm_debug.pre_tick;
		else
			work->start_time++;

		/* DG_TickCountの値を保存 */
		work->next_stream_packet =  work->stream_top;
		dm_debug.play_status = 0;

		switch ( dm_debug.speed_type ) {
		case 0 :
			dm_debug.n_wait++;
			break;
		case 1 :
			dm_debug.n_wait = WAIT_COUNT;
			break;
		}

		if ( dm_debug.n_wait >= WAIT_COUNT ) {
			/* ウェイトを元に戻す */
			dm_debug.n_wait = 0;
			/* プレイ状態を設定 */

			/* ○ボタンと×ボタンが対応 */
//			if ( (dm_debug.pad->status & PAD_B) != 0 ) {
			if ( 0 ) {
				/* 逆転再生はde2圧縮によるデータ形式変更時に使用を停止 */
				/* 逆転再生 */
				if ( (DG_TickCount - work->start_time) > 0 ) {
					work->start_time++;
					stream_tmp = work->next_stream_packet = ( char* )work->file_ptr + 16 ;
					while ( FS_STREAM_GET_TIME( work->next_stream_packet ) != 
						FS_STREAM_GET_TIME( work->stream_top ) ) {
						stream_tmp = work->next_stream_packet;
						work->next_stream_packet = ( char * )work->next_stream_packet +
							FS_STREAM_GET_SIZE( work->next_stream_packet ) + sizeof(STREAM_TAG) ;
					}
					work->next_stream_packet = stream_tmp;

					/* ポーズを一時解除 */
					GV_PauseOffActorSystem(GV_PAUSE_MENU);
					/* 再生状態を設定 */
					dm_debug.play_status = 2;
				}
			} else if ( (dm_debug.pad->status & PAD_A) != 0 ) {
				/* スロー再生 */
				work->start_time--;

				work->next_stream_packet = ( char * )work->stream_top +
					FS_STREAM_GET_SIZE( work->stream_top ) + sizeof(STREAM_TAG) ;
				stream_tmp = work->next_stream_packet;
				while ( 1 ) {
					packet = stream_tmp ;
					stream_tmp = ( char * )stream_tmp + packet->size ;

					if ( (packet->packet_type & 0x7fffffff) == 1)
						break;

					if ( (packet->packet_type & 0x7fffffff) == 0) {
						work->start_time++;
						work->next_stream_packet =  work->stream_top;
						return;
					}
				}

				/* ポーズを一時解除 */
				GV_PauseOffActorSystem(GV_PAUSE_MENU);
				/* 再生状態を設定 */
				dm_debug.play_status = 1;
			}
		}
	}
}

/* ---------------------------------------------------------------- */
	/*
		スロー・逆転再生の実処理部分(ストリーム版)
	*/
static void Demo_DebugPlayStream( Work *work )
{
//	void        *stream_tmp;
	DEMO_PACKET *packet;

	/* デバッグ用コード・スロー再生用 */
	if ( work->stream_top != NULL ) {
		/* デモを停止 */
		if ( dm_debug.pre_tick != 0 )
			work->start_time += DG_TickCount - dm_debug.pre_tick;
		else
			work->start_time++;

		/* DG_TickCountの値を保存 */
		dm_debug.play_status = 0;

		switch ( dm_debug.speed_type ) {
		case 0 :
			dm_debug.n_wait++;
			break;
		case 1 :
			dm_debug.n_wait = WAIT_COUNT;
			break;
		}

		if ( dm_debug.n_wait >= WAIT_COUNT ) {
			/* ウェイトを元に戻す */
			dm_debug.n_wait = 0;
			/* プレイ状態を設定 */

			packet = work->stream_top ;

			/* ○ボタンが対応 */
			if ( (dm_debug.pad->status & PAD_A) != 0 ) {
				/* スロー再生 */
				work->start_time--;

				if ( (packet->packet_type & 0x7fffffff) == 0) {
					work->start_time++;
					return;
				}

				/* ポーズを一時解除 */
				GV_PauseOffActorSystem(GV_PAUSE_MENU);
				/* 再生状態を設定 */
				dm_debug.play_status = 1;
			}
		}
	}
}

/* ---------------------------------------------------------------- */
	/*
		改行インライン
	*/
static int inline ChangeLine(int line)
{
	return line + (9  * VIEW_TYPE);
}

/* ---------------------------------------------------------------- */
	/*
		デバッグ表示処理
	*/
static void Demo_DebugView(Work *work)
{
	int          i;
	int          frame=0, half_frame;
	int          width, height;
	int          line_y, string_line;
	int          change_line;
    GM_CameraSet *now;

	line_y      = VIEW_BASE_Y * VIEW_TYPE;	/* デバッグ文字最上位置 */
	change_line = 9;						/* 改行 */
	string_line = (VIEW_BASE_Y + 135) * VIEW_TYPE;		/* 指定文字列の表示位置 */

	/* 指定文字列の表示 */
	if ( dm_debug.string != NULL ) {
		DEBUG_Locate(12, string_line, MENU_MODE_NORMAL);
		DEBUG_Printf("%s\n", dm_debug.string);
	}

	/* ビューモード */
	if ( dm_debug.pad->press & PAD_L1 ) {
		dm_debug.view_mode++;
		if ( dm_debug.view_mode > VIEW_MODE_MAX )
			dm_debug.view_mode = VIEW_LEVEL0;
	}

	/* デバッグ表示文字列のカラー設定 */
	if ( dm_debug.pad->press & PAD_B ) {
		dm_debug.view_color++;
		if ( dm_debug.view_color >= 2 ) dm_debug.view_color = 0;
	}

	switch ( dm_debug.view_color ) {
	case 0 :
		DEBUG_SetColor(128, 128, 128);
		break;
	case 1 :
		DEBUG_SetColor(0, 0, 0);
		break;
	}

	if ( dm_debug.view_mode >= VIEW_MODE_MAX)
		return;

	if ( dm_debug.view_mode >= VIEW_LEVEL0 ) {
		DEBUG_Locate(12, VIEW_BASE_Y * VIEW_TYPE, MENU_MODE_NORMAL);
		DEBUG_Printf("<Frame>" );

		frame = DG_TickCount - work->start_time;
		half_frame = frame/2 + dm_debug.half_frame_base;
		DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("  - %d(60) : %d(30)", frame, half_frame );

		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);

		if(dm_debug.mode1 == MODE_DEBUG_EDIT)
			DEBUG_Printf("<Mode :L2>" );
		else
			DEBUG_Printf("<Mode>" );

		switch ( dm_debug.mode1 ) {
		case MODE_PLAY :
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("  - Demo" );
			break;

		case MODE_DEBUG_PLAY :
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("  - Debug Play" );
			break;

		case MODE_DEBUG_EDIT :
			switch ( dm_debug.edit_mode ) {
			case EDIT_CAMERA_ROTATE :
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Camera Rotate" );
				break;
			case EDIT_CAMERA_MOVE :
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Camera Move" );
				break;
			case EDIT_AMBIENT :
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Ambient Edit" );
				break;
			case EDIT_FOG:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Fog Edit" );
				break;
			case EDIT_FOG_PARAM:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Fog Parameter Edit" );
				break;
			case EDIT_PARALLEL:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Parallel Edit" );
				break;
			case EDIT_PARALLEL_ROT:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("  - Debug Parallel Vector Edit" );
				break;
			}
			break;
		}

		{/*T.Morita Added 2000/08/30*/
		    char *c = dm_debug.print_buffer-1, *p ;

			if ( dm_debug.view_mode == VIEW_LEVEL0 ) {
				int y;
				y = ChangeLine(line_y);	/* 改行 */
			    DEBUG_Locate(12, y, MENU_MODE_NORMAL);
			    DEBUG_Printf( "<Debug Message>" );
			    for( i=1 ; c ; i++ )
			    {
					y = ChangeLine(y);	/* 改行 */
					c = strchr( p=c+1, '\n' ) ;
					if ( c ) *c = '\0' ;
					DEBUG_Locate(12, y, MENU_MODE_NORMAL);
					DEBUG_Printf( "%s", p );
					if ( c ) *c = '\n' ;
			    }
			}
		}

		// パケット情報
		DEBUG_Locate(12, 356, MENU_MODE_NORMAL);
		DEBUG_Printf("MOTION : %d - %d", dm_debug.lpsPacketList[5][0], dm_debug.lpsPacketList[5][1] );

		DEBUG_Locate(12, 372, MENU_MODE_NORMAL);
		DEBUG_Printf("EFFECT : %d - %d", dm_debug.lpsPacketList[6][0], dm_debug.lpsPacketList[6][1] );

		DEBUG_Locate(12, 388, MENU_MODE_NORMAL);
		DEBUG_Printf("Total Size : %d", dm_debug.lpsPacketList[7][0]);
	}

	/* デバッグの状態表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL1) && (dm_debug.view_mode != VIEW_EFFECT)) {
		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);

		if(dm_debug.mode1 != MODE_PLAY)
			DEBUG_Printf("<Speed:R2>" );
		else
			DEBUG_Printf("<Speed>" );

		switch ( dm_debug.speed_type ) {
		case 0 :
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("  - Normal" );
			break;
		case 1 :
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("  - High" );
			break;
		}
	}

	/* カメラのポジションとターゲット表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL2) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		/* 現行カメラから */
		now = GM_GetCurrentCameraSet( 0 ) ;

		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("<Position>");

		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%5.2f, %5.2f, %5.2f)",now->position.vx ,now->position.vy, now->position.vz );

		/* 操作モード */
		if( (dm_debug.mode1 == MODE_DEBUG_EDIT) &&
			(dm_debug.edit_mode == EDIT_CAMERA_ROTATE)) {
			if(dm_debug.mode2 == 0) {
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("Rot");
			}
		} else if(dm_debug.edit_mode == EDIT_CAMERA_MOVE) {
			if(dm_debug.mode2 == 0) {
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("Move");
			}
		}

		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("<Target>");

		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%5.2f, %5.2f, %5.2f)",now->target.vx ,now->target.vy, now->target.vz );

		/* 操作モード */
		if(dm_debug.edit_mode == EDIT_CAMERA_ROTATE) {
			if(dm_debug.mode2 == 1) {
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("Rot");
			}
		} else if(dm_debug.edit_mode == EDIT_CAMERA_MOVE) {
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("Move");
		}
	}

	/* 環境光の値を表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL3) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("<Ambient>" );

		/* 編集モード */
		if(dm_debug.edit_mode == EDIT_AMBIENT) {
			switch(dm_debug.ambient_status) {
			case 0:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("R" );
				break;

			case 1:
				DEBUG_Locate(98, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("G" );
				break;

			case 2:
				DEBUG_Locate(104, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("B" );
				break;

			case 3:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB" );
				break;

			case 4:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB+" );
				break;
			}
		}

		/* 設定されているカラー値 */
		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%d, %d, %d)",(int)DG_ColorMatrix.m[3][0],
			(int)DG_ColorMatrix.m[3][1], (int)DG_ColorMatrix.m[3][2]);
	}

	/* 平行光源のカラー値を表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL3) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("<Parallel>" );

		if(dm_debug.edit_mode == EDIT_PARALLEL) {
			switch(dm_debug.parallel_status) {
			case 0:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("R" );
				break;

			case 1:
				DEBUG_Locate(98, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("G" );
				break;

			case 2:
				DEBUG_Locate(104, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("B" );
				break;

			case 3:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB" );
				break;

			case 4:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB+" );
				break;
			}
		}
		/* 設定されているカラー値 */
		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%d, %d, %d)",(int)DG_ColorMatrix.m[0][0],
			(int)DG_ColorMatrix.m[0][1], (int)DG_ColorMatrix.m[0][2]);
	}

	/* 平行光源のベクトル値を表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL3) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		line_y = ChangeLine(line_y);	/* 改行 */
		if(dm_debug.edit_mode == EDIT_PARALLEL_ROT) {
			DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
			DEBUG_Printf("Rot" );
		}

		/* 設定されているベクトル値 */
		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%d, %d)=(%d, %d, %d)",
			dm_debug.parallel_rot[0], dm_debug.parallel_rot[1],
			(int)(DG_LightMatrix.m[0][0] * 5000.0f),
			(int)(DG_LightMatrix.m[1][0] * 5000.0f),
			(int)(DG_LightMatrix.m[2][0] * 5000.0f));

	}

	/* フォグの値を表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL3) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		line_y = ChangeLine(line_y);	/* 改行 */
		DEBUG_Locate(12, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("<Fog>" );

		/* 編集モード */
		if(dm_debug.edit_mode == EDIT_FOG) {
			switch(dm_debug.fog_status) {
			case 0:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("R" );
				break;

			case 1:
				DEBUG_Locate(98, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("G" );
				break;

			case 2:
				DEBUG_Locate(104, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("B" );
				break;

			case 3:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB" );
				break;

			case 4:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("RGB+" );
				break;
			}
		}

		/* 設定されているカラー値 */
		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%d, %d, %d)", DG_FogColor.r, DG_FogColor.g, DG_FogColor.b);
	}

	/* フォグの値を表示 */
	if ( (dm_debug.view_mode >= VIEW_LEVEL3) && (dm_debug.view_mode != VIEW_EFFECT) ) {
		int nearf, farf;

		/* near, far */
		line_y = ChangeLine(line_y);	/* 改行 */
		if(dm_debug.edit_mode == EDIT_FOG_PARAM) {
			switch(dm_debug.fog_param_status) {
			case 0:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("near" );
				break;

			case 1:
				DEBUG_Locate(92, line_y, MENU_MODE_NORMAL);
				DEBUG_Printf("far" );
				break;
			}
		}

		/* 設定されているカラー値 */
		nearf = GetFogNear();
		farf  = GetFogFar();
		DEBUG_Locate(122, line_y, MENU_MODE_NORMAL);
		DEBUG_Printf("(%d, %d)", nearf, farf);
	}

	/* エフェクトリスト表示 */
	if ( dm_debug.view_mode == VIEW_EFFECT ) {
		if ( dm_debug.n_effect != 0 ) {
			height = (VIEW_BASE_Y + 18) * VIEW_TYPE;
			DEBUG_Locate(12, height, MENU_MODE_NORMAL);
			DEBUG_Printf("<Effect List>");
			for ( i = 0, width = 12; i < VIEW_LIST_COUNT; i++ ) {
				/* 登録されていなければ終了 */
				if ( dm_debug.effect_view[i].view == 0 ) break;
				/* 12 行以上は表示できないので改ページする */
				if ( i == 12 ) {
					width = 260;
					height = (VIEW_BASE_Y + 18) * VIEW_TYPE;
				}
				/* 登録されているものを表示 */
				if ( (dm_debug.effect_view[i].start <= frame) && 
				     (frame <= (dm_debug.effect_view[i].start+dm_debug.view_time)) ) {
					/* 画面表示 */
					height = ChangeLine(height);
					DEBUG_Locate(width, height, MENU_MODE_NORMAL);
					DEBUG_Printf("%d, %d, %s", dm_debug.effect_view[i].start,
						(dm_debug.effect_view[i].start / 2) + dm_debug.half_frame_base,
						dm_debug.effect_list[dm_debug.effect_view[i].list_index].name );
				} else {
					dm_debug.effect_view[i].view = 0;
				}
			}
			/* 表示しなくなったやつを削除する */
			for ( i = 0; i < VIEW_LIST_COUNT; i++ ) {
				if ( dm_debug.effect_view[i].view != 0 ) break;
			}
			/* 前に詰める */
			if ( (i != 0) && (i != VIEW_LIST_COUNT) ) {
				memcpy(&(dm_debug.effect_view[0]), &(dm_debug.effect_view[i]),
					 (sizeof(DM_EFT_VIEW) * (VIEW_LIST_COUNT - i)) );

				/* 後ろのデータを初期化する */
				memset(&(dm_debug.effect_view[VIEW_LIST_COUNT-i]), 0x00,
					(sizeof(DM_EFT_VIEW) * i) );
			}
		} else {
			/* リストが無ければ表示そのものができないので飛ばす */
			dm_debug.view_mode++;
		}
	}
}

/* ---------------------------------------------------------------- */
	/*
		モード設定関係
	*/
static void Demo_DebugCursole(Work *work)
{
	int i;

	/* モード切り替え */
	if ( dm_debug.pad->press & PAD_AR ) {
		dm_debug.mode1++;
		if ( dm_debug.mode1 >= 3 ) {
			dm_debug.mode1 = 0;
			dm_debug.first = 0;
			dm_debug.edit_mode = 0;
			dm_debug.n_wait = 0;
			dm_debug.play_status = 0;
			dm_debug.ambient_status = 0;
			dm_debug.fog_status = 0;
			dm_debug.fog_param_status = 0;
			dm_debug.parallel_status = 0;
		}
	}

	/* カメラモードでなければ以下の動作はしない */
	if ( dm_debug.mode1 == 0 ) {
		GV_PauseOffActorSystem(GV_PAUSE_MENU);
		return;
	} else {
		GV_PauseOnActorSystem(GV_PAUSE_MENU);
		/* 音声キャンセル */
		GM_StreamSdCancel();
	}

	/* 現在のカメラのパラメータを取得 */
	if ( (dm_debug.mode1 == 2) && (dm_debug.first == 0) ) {
		/* ここは一度しか通らない */
		DemoDebug_GetNowCameraData();
		dm_debug.first = 1;
	}

	/* スピード調整 */
	if ( dm_debug.pad->status & PAD_R2 ) dm_debug.speed_type = 1;
	else                                 dm_debug.speed_type = 0;

	/* 各モードのセカンドモード */
	if ( dm_debug.pad->status & PAD_R1 ) dm_debug.mode2 = 1;
	else                                 dm_debug.mode2 = 0;

	/* デバッグモード実行処理 */

	/* スロー再生・逆転再生モード */
	if ( work->file_flag )
		Demo_DebugPlay( work );
	else
		Demo_DebugPlayStream( work );
	if ( dm_debug.mode1 == 2 ) {
		/* 編集モード */
		if ( dm_debug.pad->press & PAD_L2 ) {
			dm_debug.edit_mode++;
			if ( dm_debug.edit_mode >= 7 ) dm_debug.edit_mode = 0;
		}

		switch ( dm_debug.edit_mode ) {
		case EDIT_CAMERA_ROTATE :
			/* カメラ回転 */
			Demo_DebugCameraRotate();
			break;

		case EDIT_CAMERA_MOVE :
			/* カメラ移動 */
			Demo_DebugCameraMove();
			break;

		case EDIT_AMBIENT :
			/* 環境光編集 */
			Demo_DebugEditAmbient();
			/* 変更を反映させるために、オブジェクトのActを無理やり実行する */
			for(i = 0; i < work->demo_work.n_object; i++) {
				void ( *act )( GV_ACT * );

				act = ((GV_ACT_EX*)(work->demo_work.obj_list[i].work_ptr))->actor.act;
				(*act)(work->demo_work.obj_list[i].work_ptr);
			}
			break;

		case EDIT_PARALLEL :
			/* 平行光源編集 */
			Demo_DebugEditParallel();
			/* 変更を反映させるために、オブジェクトのActを無理やり実行する */
			for(i = 0; i < work->demo_work.n_object; i++) {
				void ( *act )( GV_ACT * );

				act = ((GV_ACT_EX*)(work->demo_work.obj_list[i].work_ptr))->actor.act;
				(*act)(work->demo_work.obj_list[i].work_ptr);
			}
			break;

		case EDIT_PARALLEL_ROT :
			/* 平行光源編集 */
			Demo_DebugEditParallel_Rot();
			/* 変更を反映させるために、オブジェクトのActを無理やり実行する */
			for(i = 0; i < work->demo_work.n_object; i++) {
				void ( *act )( GV_ACT * );

				act = ((GV_ACT_EX*)(work->demo_work.obj_list[i].work_ptr))->actor.act;
				(*act)(work->demo_work.obj_list[i].work_ptr);
			}
			break;

		case EDIT_FOG :
			/* フォグ編集 */
			Demo_DebugEditFog();
			break;

		case EDIT_FOG_PARAM :
			/* フォグパラメータ編集 */
			Demo_DebugEditFogParam();
			break;
		}
	}

	/* セレクトでリセット */
	if ( dm_debug.pad->press & PAD_SEL ) {
		/* ポーズを一時解除 */
		GV_PauseOffActorSystem(GV_PAUSE_MENU);
		loop_count++;
		blank_count = 1;

		if ( (dm_debug.pad->status & PAD_R1) &&
			 (dm_debug.pad->status & PAD_R2) )
			 loop_count = 0;
		GM_LoadRequest = GM_REQ_RESET;
		GV_DestroyActor( work );
	}
}

#endif


/* デモデーモンのマップを取得する（エフェクトで使用するため） */
int DM_GetDemoActorMap()
{
    if ( !DM_DemoActor )
	return GM_CurrentMap ;
    return DM_DemoActor->map ;
}
