//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mpegstrx.c

	X用MPEG再生
	ドライバは gama/cristream.c でやってる
	ここは表示のみ

	ｐｓｘ２とｘｂｏｘでソースが違うようだがｗｉｎはｘｂｏｘ用ソースで動かす予定

	2002/06/04 M.Kobayashi
	
	$Id: mpegstrx.c,v 1.5 2002/11/23 12:09:27 Yoshizawa1 Exp $
*/

#include "gameheader.h"
#include	"libfs.h"
#include	"strctrl.h"

#ifdef KP_WINDOWS
#include "winstream.h"	
#else
#include "cristream.h"	
#endif

#include	"mpegstr.h"

/* ---------------------------------------------------------------------- */
/*
	work
*/

typedef struct {
	GV_ACT_EX actor;
	
	DG_DMAPACK*	pDmapack;
	u_long128	buffer[ 16 ];	// DMAPACK バッファ
#if 0 //BP_XBOX
	DG_TEX_LIN* pTex[2];
#endif
	int		disp_which;

	int		width;
	int		height;
	int 	buf_width;
	int		buf_height;
	int		disp_width;
	int		disp_height;
	int		x;
	int		y;
	int		end_proc;
	int		repeat_flag;
	int		end_flag;
	int		pad_cancel;
	int		framenum;
	int		top_pos;
	int		tick;
	int		priority;
	
	int		movie_no;
#if 0 //BP_XBOX
   MPEG_MV_INFO*	pMvInfo;
#endif

	int		status;

	BOOL  	bCheckConnect;
	u_int	prevconnectstate;
	
	void*	stream_h;
	int		stream_id;

	int bReqCancel;
	int file;	// 0 : movie.dat, 1 : movievr.dat
	
} MPEG_WORK;

enum {
	STATE_INIT,
	STATE_WAIT,
	STATE_PLAY,
	STATE_END,
};

static MPEG_WORK* pCurrentActor;

typedef struct _REQUEST_INFO {
	int top_pos;
	int file;
	int width;
	int height;
	int x;
	int y;
	int repeat;
	int pad_cancel;
	int	end_proc;
	int priority;
	int disp_width;	
	int disp_height;	
} REQUEST_INFO;

static REQUEST_INFO request_info = { -1, FS_FILE_MOVIE };
static void *NewMpegPssMovieStrProg2( REQUEST_INFO* pReqinfo );


/* ---------------------------------------------------------------------- */
/*
	Act, Die
*/

static BOOL CheckCancel( MPEG_WORK* work )
{	// パッドキャンセル処理
	BOOL bRet = FALSE;
	if( work->bReqCancel ) {
		bRet = TRUE;
		work->bReqCancel = FALSE;
	} else if( work->bCheckConnect ) {
		// 接続状態の監視
		int i;
		u_int state = 0;
		u_int press = 0;
		for( i = 0 ; i < GV_PAD_MAX; i++ ) {
			state |= (( GV_PadDataDirect[ i ].flag & GV_PAD_DISCONNECT ) ? 0 : 1 ) << i;
			press |= GV_PadDataDirect[ i ].press;
		} 

		if( ( (state ^ work->prevconnectstate) & state) ||
			( press & work->pad_cancel ) ) {
			bRet = TRUE;
		}
		work->prevconnectstate = state;
	} else {
		if( GV_PadDataDirect[ 0 ].release & work->pad_cancel ){
			bRet = TRUE;
		}
	}
	if( bRet ) {
		GM_StreamStop( work->stream_id );
		work->end_flag = 1;
	}
	return bRet;
}

static void Act( MPEG_WORK *work )
{
#if 0 //BP_XBOX
   switch( work->status ){
	case STATE_INIT:
		/*
		  バッファ管理情報の初期化と初期読み込み
		*/
		work->stream_h = NULL;
		{
			int flag;

			if( work->end_proc > 0 && work->repeat_flag == 0 ){
				flag = GM_STREAM_FLAG_PROC( work->end_proc );
			} else {
				flag = 0;
			}
			work->stream_id = GM_MovieStream2( work->file, work->top_pos, flag );
		}
		work->status = STATE_WAIT;
		break;
	case STATE_PLAY:
	{
		if( CheckCancel( work ) ) break;
//		work->tick ^= 1;
//		if( work->tick ) break;
		
		if ( GM_StreamGetCri() ) {
			int		w, h, f ;
			work->disp_which ^= 1;
			
			if( work->framenum == 0 ){
				DG_UnDrawFrameCount = 1;
			}
			work->framenum++;

			GM_StreamGetCriInfo( &w, &h, &f );
			GM_StreamCopyCriImage( work->pTex[ work->disp_which ]->image, work->buf_width, work->buf_height, 0, 0, 0 );
			
			DG_LinerTextureSetImageDirty( work->pTex[ work->disp_which ] );
			/* ＩＰＵ保持イメージを開放 */
			GM_StreamFreeCri();

			// 表示パケット準備
			{	
				u_long128* pBuf;
				pBuf = work->buffer;
				pBuf = DG_SetDmapackTexLin( pBuf, work->pTex[ work->disp_which ] );
				pBuf = DG_SetDmapackAlpha( pBuf, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
				pBuf = DG_SetDmapackSprt( pBuf,
										  (float)DIRECT_SCREEN_X( work->x ) , (float)DIRECT_SCREEN_Y( work->y ), 0.f, 0.f,
										  (float)DIRECT_SCREEN_X( work->x + work->disp_width ),
										  (float)DIRECT_SCREEN_Y( work->y + work->disp_height ),
										  (float)work->width / work->buf_width, (float)work->height/work->buf_height,
										  0x80808080);
				DG_SetDmapackEnd( pBuf );
			}
			
		} else if( GM_StreamIsEndCri() ) {
			/* 終了 */
			printf( "DECODE END\n" );
			work->status = STATE_END;
		}
	}
	break;
	case STATE_WAIT:
		if( CheckCancel( work ) ) break;
	case STATE_END:
		/* 終了処理 */
	{	
		if( !GM_StreamIsEndCri() ) {
			// MPEGが始まった
			work->status = STATE_PLAY;
			if( work->pMvInfo != NULL ) {
				work->disp_width = work->width = work->pMvInfo[ work->movie_no ].width;
				work->disp_height = work->height = work->pMvInfo[ work->movie_no ].height;
				work->x = work->pMvInfo[ work->movie_no ].x;
				work->y = work->pMvInfo[ work->movie_no ].y;
				++work->movie_no;
			}
			break;
		}
		if( GM_StreamStatus( work->stream_id ) != GM_STREAM_STATE_END ){
			return;
		}
		printf( "REPEAT = %d\n", work->repeat_flag );
		if( work->repeat_flag == 0 ){
			work->end_flag = 1;
		}
		
		if( work->end_flag ){
			printf( "END\n" );
			GV_DestroyActor( work );
		} else {
			printf( "REPEAT\n" );
			work->status = STATE_INIT;
			work->framenum = 0;
		}
	}
	break;
	}
#endif
}

static void Die( MPEG_WORK *work )
{
#if 0 //BP_XBOX
   DG_DequeueDmapack( work->pDmapack );
	DG_FreeDmapack( work->pDmapack );
	DG_FreeLinerTexture( work->pTex[0] );
	DG_FreeLinerTexture( work->pTex[1] );

	GM_StreamCriDriverEnd();

	if( work->repeat_flag != 0 && work->end_proc != 0 && !GV_IsStageDestroy( work ) ){
		GCL_ExecProc( work->end_proc, NULL );
	}
	GV_SetActorFreeFunc( work, GV_DelayedFree );
printf( "mpegstr dead\n" );
 
	ASSERT( pCurrentActor == work );
	pCurrentActor = NULL;

	if( !GV_IsStageDestroy( work ) ){
		if( request_info.top_pos != -1 ) {
			NewMpegPssMovieStrProg2( &request_info );
			request_info.top_pos = -1;
		}
	}
#endif
}

/* ---------------------------------------------------------------------- */
/*
	ACTOR起動
*/

static void GetGclOption( MPEG_WORK* work )
{
#if 0 //BP_XBOX
   if( GCL_GetOption( 's' ) == NULL ){
		ASSERT( FALSE );
	}
	work->disp_width = work->width = work->buf_width = GCL_GetNextInt();
	work->disp_height = work->height = work->buf_height = GCL_GetNextInt();
	/*
		コールバック登録
	*/

	if( GCL_GetOption( 'p' ) != NULL ){
		work->end_proc = GCL_GetNextInt();
	} else {
		work->end_proc = 0;
	}
	
	/*
		PSSの名前,場所を取得
	*/
	if( GCL_GetOption( 'f' ) != NULL ){
		work->top_pos = GCL_GetNextInt();
	} else {
		ASSERT( FALSE );
	}

	if( GCL_GetOption( 't' ) != NULL ){
		work->x = GCL_GetNextInt();
		work->y = GCL_GetNextInt();
	}
#if 1
	if( GCL_GetOption( 'r' ) != NULL ){
		work->repeat_flag = 1;
	} else {
		work->repeat_flag = 0;
	}
#endif
	if( GCL_GetOption( 'x' ) != NULL ) {
		// オープニングのキャンセル方法は違う
		work->bCheckConnect = TRUE;
		work->prevconnectstate = ~0;
		work->pad_cancel = PAD_ABXY | PAD_LR | PAD_STA | PAD_SEL | PAD_AL | PAD_AR;
			
		DG_UnDrawFrameCount = DG_UNDRAW_MAX;
		GM_SetGameStatus(STATE_DEMO);
		
	} else {
		work->bCheckConnect = FALSE;
		if( work->repeat_flag == 0 ){
			work->pad_cancel = PAD_DEMO_CANCEL;
			
			DG_UnDrawFrameCount = DG_UNDRAW_MAX;
			GM_SetGameStatus(STATE_DEMO);
		} else {
			work->pad_cancel = PAD_CANCEL;
		}
	}

	work->pMvInfo = NULL;
	work->priority = GCL_GetOptionValue( 'R', 249 );
	request_info.file = work->file = FS_FILE_MOVIE;
#endif
}


static int GetResources( MPEG_WORK *work )
{
#if 0 //BP_XBOX
   printf( "MPEG %x\n", work->top_pos );
	/*
		表示関係
	*/
	{
		// Dmapack 準備
		work->pDmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, work->priority );
		DG_QueueDmapack( work->pDmapack );
		work->pDmapack->autopacket = work->buffer;
		DG_SetDmapackEnd( work->buffer );
		
		// テクスチャ領域準備
		work->pTex[0] = DG_MakeLinerTexture( work->buf_width, work->buf_height, DG_TEXLIN_FORMAT_A8R8G8B8 );
		ZeroMemory( work->pTex[0]->image, work->buf_width * work->buf_height * 4 );
		work->pTex[1] = DG_MakeLinerTexture( work->buf_width, work->buf_height, DG_TEXLIN_FORMAT_A8R8G8B8 );
		ZeroMemory( work->pTex[1]->image, work->buf_width * work->buf_height * 4 );

		work->disp_which = 0;
	}
	work->movie_no = 0;
	work->framenum = 0;
	work->end_flag = 0;
	work->status = STATE_INIT;
//	work->tick = 0;

	/*
		ストリーム起動
	*/

	GM_StreamCriDriverInit( 0 );
	
	if( work->top_pos < 0 ) {
		GV_DestroyActor( work );
		if( work->end_proc != 0 ) {
			GCL_ExecProc( work->end_proc, NULL );
		}
	}
	
	ASSERT( pCurrentActor == NULL );
	pCurrentActor = work;
	work->bReqCancel = FALSE;
#endif	
	return 0;
}

void *NewMpegPssMovieStr( int name, int map )
{
	MPEG_WORK *work;

	work = ( MPEG_WORK * )GV_NewActor( GV_ACTOR_PREV2, sizeof( MPEG_WORK ) );

	if( work != NULL ){
		GV_ActorEX( &( work->actor ) );
		GV_SetActor( &( work->actor ), Act, Die );
		GetGclOption( work );
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}

#if 0 //BP_XBOX

// プログラム入り口
void *NewMpegPssMovieStrProg( MPEG_MV_INFO *pMvInfo,
							  int pos, int proc, int repeat, u_short cancel )
{	// Ending 用
	// １ストリーム中のムービーが変わるたびに位置などを変えることができる
	// pMvInfo は参照しつづける
   MPEG_WORK *work;

	work = ( MPEG_WORK * )GV_NewActor( GV_ACTOR_PREV2, sizeof( MPEG_WORK ) );

	if( work != NULL ){
		GV_ActorEX( &( work->actor ) );
		GV_SetActor( &( work->actor ), Act, Die );

		work->disp_width = work->width = work->buf_width = pMvInfo->width;
		work->disp_height = work->height = work->buf_height = pMvInfo->height;
		work->end_proc = proc;
		work->top_pos = pos;
		work->x = pMvInfo->x;
		work->y = pMvInfo->y;
		work->repeat_flag = repeat;
		work->bCheckConnect = FALSE;
		work->pMvInfo = pMvInfo;
		work->pad_cancel = cancel;
		work->file = FS_FILE_MOVIE;
		
		if( work->repeat_flag == 0 ){
			DG_UnDrawFrameCount = DG_UNDRAW_MAX;
			GM_SetGameStatus(STATE_DEMO);
		}
		
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}
#endif

static void *NewMpegPssMovieStrProg2( REQUEST_INFO* pReqinfo )
{	// Request 用
#if 0 //BP_XBOX
   MPEG_WORK *work;

	work = ( MPEG_WORK * )GV_NewActor( GV_ACTOR_PREV2, sizeof( MPEG_WORK ) );

	if( work != NULL ){
		GV_ActorEX( &( work->actor ) );
		GV_SetActor( &( work->actor ), Act, Die );
		work->width = work->buf_width = pReqinfo->width;
		work->height = work->buf_height = pReqinfo->height;
		
		work->disp_width = pReqinfo->disp_width;
		work->disp_height = pReqinfo->disp_height;
		
		work->end_proc = pReqinfo->end_proc;
		work->top_pos = pReqinfo->top_pos;
		work->x = pReqinfo->x;
		work->y = pReqinfo->y;
		work->repeat_flag = pReqinfo->repeat;
		work->bCheckConnect = FALSE;
		work->pMvInfo = NULL;
		work->pad_cancel = pReqinfo->pad_cancel;
		work->priority = pReqinfo->priority;
		work->file = pReqinfo->file;
		
		if( work->repeat_flag == 0 ){
			DG_UnDrawFrameCount = DG_UNDRAW_MAX;
			GM_SetGameStatus(STATE_DEMO);
		}
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
#else
   return NULL;
#endif
}


// Movie をリクエストする 最後にリクエストしたものが有効
// 		top_pos        : ストリームデータの位置
//		width,height   : ムービーの幅、高さ
//		x,y	           : ムービーの表示位置	
//		repeat         : 真なら繰返し再生する			  
//		pad_cancel     : パッドでキャンセル可能ならそのボタンを入れる
//		disp_width
//		disp_height    : 表示幅・高さ

void RequestMovieStream( int top_pos, int width, int height,
						 int x, int y, 
						 int repeat, int pad_cancel,
						 int disp_width, int disp_height )
{
#if 0 //BP_XBOX
   REQUEST_INFO info;

	info.file = FS_FILE_MOVIEVR;

	if( FS_GetStreamTop( info.file, top_pos ) == -1 ) {
		// ムービーがない
		return;
	}
	info.top_pos = top_pos;
	info.width = width;
	info.height = height;
	info.x = x;
	info.y = y;
	info.repeat = repeat;
	info.pad_cancel = pad_cancel;
	info.end_proc = 0;
	info.priority = 249;

	info.disp_width = disp_width;
	info.disp_height = disp_height;

	if( pCurrentActor == NULL ) {
		NewMpegPssMovieStrProg2( &info );
	} else {
		request_info = info;
	}
#endif
}

// Request 中のものを含めてムービーをキャンセルする
void CancelMovieStream( void )
{
#if 0 //BP_XBOX
   if( pCurrentActor != NULL ) {
		pCurrentActor->bReqCancel = TRUE;
	}
	request_info.top_pos = -1;
#endif
}
