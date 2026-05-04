//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
/*
	endingx.c

	X用MPEG再生
	ドライバは gama/cristream.c でやってる
	ここは表示のみ

	ｐｓｘ２とｘｂｏｘでソースが違うようだがｗｉｎはｘｂｏｘ用ソースで動かす予定

	2002/06/04 M.Kobayashi
	
	$Id: endingx.c,v 1.7 2002/12/24 07:18:29 takaki Exp $
*/

#if 1
//#ifdef KP_XBOX //BP

#include <stdio.h>

#if 1 //BP_PS2
#include "Engine/Mechanics/zlib/zlib.h"
#include "BP_EndianSupport.h"
#else
#define PS2_EE
#include "zlib.h"
#endif

#include "gameheader.h"
#include	"libfs.h"
#include	"strctrl.h"

#ifdef KP_WINDOWS
#include "winstream.h"	
#else
#include "cristream.h"	
#endif

#include	"mpegstr.h"

#undef long
#undef u_long64

#define ZLIB_WORK_SIZE			(45656)

#include "BP_Misc.h"

/* ---------------------------------------------------------------------- */
/*
	進行用データ
*/

#if 0
static MPEG_MV_INFO end_mv_info[] = {
	{ 0, 512, 320, 0, 32 },
	{ 73606, 192, 160, 299, 128+32 },
//	{ 92607, 512, 320, 0, 32 },
	{ 91950, 512, 320, 0, 32 },
	{ -1, 0, 0, 0, 0 }
};
#endif


#define TEX_BUFFER_WIDTH 512
#define TEX_BUFFER_HEIGHT 320

#define TELOP_DECODE_BUF_WIDTH	(512)
#define TELOP_HEIGHT 448
#define TELOP_DECODE_BUF_HEIGHT	(512)
#define	TELOP_DECODE_NET_HEIGHT	(511)	// バイリニアをうまく働かせるため

enum {
	TELOP_TITLE,
	TELOP_WAIT,
	TELOP_DECODE,
	TELOP_BLANK,
	TELOP_STOP,
	TELOP_FADE,
	TELOP_END
};

#define TELOP_CODE_END		0
#define TELOP_CODE_BLANK	1
#define TELOP_CODE_BMP		2
#define TELOP_CODE_STOP		3
#define TELOP_CODE_FADE		4
#define TELOP_CODE_SPEED	5

#ifdef KP_WINDOWS
#define	GET_ENDING_TICK()	(DG_TickCount)
#endif

/* ---------------------------------------------------------------------- */
/*
	work
*/

typedef struct {
	GV_ACT_EX actor;
	
	DG_DMAPACK* pDmapackTelop;
	u_long128	bufferTelop[ 32 ];	// DMAPACK バッファ
	DG_TEX_LIN* pTexTelop;
	void *telop_vram;
	unsigned char *telop_data;
	int display_y;
	int telop_y;
	int telop_fade;
	int telop_fade_step;
	int telop_end_flag;
	int	movie_flag;

	int prev_y;
	
	int decode_y;
	int telop_state;
	int blank_count;

	int end_proc;

	int start_stop;

	int speed;

	float	fOver;
	
#ifdef KP_WINDOWS
	int		pre_tick ;
	BOOL	destroyed ;
#endif
	char zlib_buf[ ZLIB_WORK_SIZE ];
} ENDING_WORK;

/* ---------------------------------------------------------------------- */
/*
	zlib関係
*/

static z_stream z;

static void zlib_init( ENDING_WORK *work )
{
#if 0 //BP_PS2
	z.buffer = work->zlib_buf;
#endif
	if( inflateInit( &z ) != Z_OK ){
		HANGUP();
	}
}

static int zlib_decode( void *decode, int decode_size, ENDING_WORK *work )
{
	z.next_out = decode;
	z.avail_out = decode_size;

	z.next_in = work->telop_data;
	z.avail_in = 0x7FFFFFFF;

	for( ;; ){
		int status;
		status = inflate( &z, Z_NO_FLUSH );
		if( status == Z_STREAM_END || z.avail_out == 0 ){
			/* 終了 */
			work->telop_data = z.next_in;
			return ( status == Z_STREAM_END ? 0 : 1 );
			break;
		}
		if( status != Z_OK ){
			printf( "status %d %s\n", status, z.msg );
			HANGUP();
		}
	}

   // unreachable
   return 0;
}

static void zlib_finish( void )
{
	inflateEnd( &z );
}



/* ---------------------------------------------------------------------- */
/*
	テロップ制御
*/

static void set_telop_sprt( ENDING_WORK *work, int subpix )
{
	int spr1_utop;
	int x, y, w, h;
	unsigned int fade;
	unsigned int col0;

//	subpix *= 2;

	fade = 0xff * work->telop_fade / 256;
	col0 = ( 0xff << 24 ) | ( fade << 16 ) | ( fade << 8 ) | fade;

   if ( BP_IsPAL()==FALSE)
   {
	   if( DG_Clock & 1 ) {	// 30frame/sec スクロールにする
		   spr1_utop = work->prev_y / 16 ;
		   subpix = work->prev_y % 16;
	   } else {
		   spr1_utop = ( work->display_y % TELOP_DECODE_NET_HEIGHT );
		   work->prev_y = spr1_utop * 16 + subpix;
	   }
   }
   else
   {
	   spr1_utop = ( work->display_y % TELOP_DECODE_NET_HEIGHT );
   }

	// 実験
//	memset( (int*)work->pTexTelop->image, 0xff, 256 );
	
	// 上端を下端にコピーする
	memcpy( (int*)work->pTexTelop->image + TELOP_DECODE_NET_HEIGHT * TELOP_DECODE_BUF_WIDTH,
			work->pTexTelop->image,
			(TELOP_DECODE_BUF_HEIGHT - TELOP_DECODE_NET_HEIGHT) * TELOP_DECODE_BUF_WIDTH * sizeof(int));
											 


	x = 0; y = 0;
	w = TELOP_DECODE_BUF_WIDTH;
	h = TELOP_HEIGHT;

#if 0	
	// uv REPEATを設定している。

	sprt = &( draw->sprt1 );
	*( u_long64 * )&sprt->rgba = col0;

	DG_SET_XY1( sprt, DG_POS_X( x ), DG_POS_Y( DIRECT_SCREEN_Y( y ) ) - subpix );
	DG_SET_XY2( sprt, DG_POS_X( x + w ), DG_POS_Y( DIRECT_SCREEN_Y( y + h ) ) - subpix );
	DG_SET_UV1( sprt, 0 + 8, spr1_utop * 16 + 8 );
	DG_SET_UV2( sprt, ( 0 + w ) * 16 + 8, ( spr1_utop + h ) * 16 + 8 );

	sprt = &( draw->sprt1_b );
	*( u_long64 * )&sprt->rgba = col1;

	DG_SET_XY1( sprt, DG_POS_X( x ), DG_POS_Y( DIRECT_SCREEN_Y( y ) + 1 ) - subpix );
	DG_SET_XY2( sprt, DG_POS_X( x + w ), DG_POS_Y( DIRECT_SCREEN_Y( y + h ) + 1 ) - subpix );
	DG_SET_UV1( sprt, 0 + 8, spr1_utop * 16 + 8 );
	DG_SET_UV2( sprt, ( 0 + w ) * 16 + 8, ( spr1_utop + h ) * 16 + 8 );
#else
	{
		u_long128* pBuf;
		int vend;
		float voffset;
		pBuf = work->bufferTelop;
		pBuf = DG_SetDmapackTexLin( pBuf, work->pTexTelop );
		pBuf = DG_SetDmapackAlpha( pBuf, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		vend = spr1_utop + h;

		voffset = subpix / 16.f + 0.5f;
		
		if( vend >= TELOP_DECODE_NET_HEIGHT ) {
			vend %= TELOP_DECODE_NET_HEIGHT;
			
			// 縦方向にバイリニア補間させないよう気をつけたい。。。のだが
#ifndef KP_WINDOWS
			pBuf = DG_SetDmapackSprt( pBuf,
									  0.f, 0.f,
									  0.f, ( spr1_utop + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  (float)TELOP_DECODE_NET_HEIGHT - spr1_utop,
									  1.f, (TELOP_DECODE_NET_HEIGHT + voffset) / TELOP_DECODE_BUF_HEIGHT,
									  DG_MakeDmaPackColorFromInt(col0) );

			pBuf = DG_SetDmapackSprt( pBuf,
									  (float)DIRECT_SCREEN_X( x ),
									  (float)TELOP_DECODE_NET_HEIGHT - spr1_utop,
									  0.f, voffset / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  (float)h,
									  1.f, ( vend + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  DG_MakeDmaPackColorFromInt(col0) );
#else
			// Windows版では、DRAW_HEIGHTに拡大...
			pBuf = DG_SetDmapackSprt( pBuf,
									  0.f, 0.f,
									  0.f, ( spr1_utop + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  ((float)TELOP_DECODE_NET_HEIGHT - spr1_utop)
										*((float)DRAW_HEIGHT/(float)TELOP_HEIGHT),
									  1.f, (TELOP_DECODE_NET_HEIGHT + voffset) / TELOP_DECODE_BUF_HEIGHT,
									  col0 );
			pBuf = DG_SetDmapackSprt( pBuf,
									  (float)DIRECT_SCREEN_X( x ),
									  ((float)TELOP_DECODE_NET_HEIGHT - spr1_utop)
										*((float)DRAW_HEIGHT/(float)TELOP_HEIGHT),
									  0.f, voffset / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  (float)h
										*((float)DRAW_HEIGHT/(float)TELOP_HEIGHT),
									  1.f, ( vend + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  col0 );
#endif
		} else {
#ifndef KP_WINDOWS
			pBuf = DG_SetDmapackSprt( pBuf,
									  (float)DIRECT_SCREEN_X( x ), 0.f, 
									  0.f, ( spr1_utop + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  (float)h,
									  1.f, ( vend + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  DG_MakeDmaPackColorFromInt(col0) );
#else
			// Windows版では、DRAW_HEIGHTに拡大...
			pBuf = DG_SetDmapackSprt( pBuf,
									  (float)DIRECT_SCREEN_X( x ), 0.f, 
									  0.f, ( spr1_utop + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  (float)DIRECT_SCREEN_X( x + w ),
									  (float)h
										*((float)DRAW_HEIGHT/(float)TELOP_HEIGHT),
									  1.f, ( vend + voffset ) / TELOP_DECODE_BUF_HEIGHT,
									  DG_MakeDmaPackColorFromInt(col0) );
#endif
		}
		DG_SetDmapackEnd( pBuf );
	}
#endif	
}

#ifdef KP_WINDOWS
static void _Act( ENDING_WORK *work ) ;

static void Act( ENDING_WORK *work )
{
	int	i ;
	int	tick ;

	/* Windows版は、処理時間が一定とは限らない */
	tick = GET_ENDING_TICK() ;
	for(i=tick-work->pre_tick; i>0; i--)
	{
		_Act(work) ;
		if( work->destroyed ){ break ; }
	}

	work->pre_tick = tick ;
}
#endif

#ifndef KP_WINDOWS
static void Act( ENDING_WORK *work )
#else
static void _Act( ENDING_WORK *work )
#endif
{
	int ty;
	int dec_y, disp_y;

if( GV_PadData[ 0 ].status & PAD_B ){
	printf( "work->telop = %d\n", work->telop_y / 16 );
}
 
	if( work->telop_state == TELOP_TITLE ) {
		if( --work->start_stop < 0 ) {
			work->telop_state = TELOP_WAIT;
			work->telop_fade = 256;
		} else {
			work->telop_fade += 15;
			if( work->telop_fade > 256 ) work->telop_fade = 256;
			set_telop_sprt( work, 0 );
			return;
		}
	}

   if ( BP_IsPAL()==TRUE)
	{
		float fStep = work->speed * 59.94f/ 50.f + work->fOver;
		int step = (int)fStep;
		work->fOver = fStep - (float)step;
		ty = work->telop_y + step;
	}
   else	
	   ty = work->telop_y + work->speed;

	dec_y = work->decode_y;
	disp_y = work->display_y;

	while( ty / 16 > work->telop_y / 16 ){
RETRY:
		switch( work->telop_state ){
		  case TELOP_WAIT:
			switch( *work->telop_data ){
			  case TELOP_CODE_END:
				work->telop_state = TELOP_END;
				break;
			  case TELOP_CODE_BLANK:
printf( "--BLANK %d\n", work->telop_y / 16 );
				memcpy( &work->blank_count, work->telop_data + 1, sizeof( int ) );
            BP_LE_SwapSInt_Inp( &work->blank_count );
				work->telop_data += 1 + sizeof( int );
				work->telop_state = TELOP_BLANK;
				printf( "blank = %d\n", work->blank_count );
				break;
			  case TELOP_CODE_STOP:
				memcpy( &work->blank_count, work->telop_data + 1, sizeof( int ) );
            BP_LE_SwapSInt_Inp( &work->blank_count );
				work->telop_data += 1 + sizeof( int );
				work->telop_state = TELOP_STOP;
				printf( "stop = %d\n", work->blank_count );
				break;
			  case TELOP_CODE_FADE:
				memcpy( &work->telop_fade_step, work->telop_data + 1, sizeof( int ) );
            BP_LE_SwapSInt_Inp( &work->telop_fade_step );
				work->telop_data += 1 + sizeof( int );
				work->telop_state = TELOP_FADE;
				printf( "stop = %d\n", work->telop_fade_step );
				break;
			  case TELOP_CODE_SPEED:
				memcpy( &work->speed, work->telop_data + 1, sizeof( int ) );
            BP_LE_SwapSInt_Inp( &work->speed );
				work->telop_data += 1 + sizeof( int );
				printf( "speed = %d\n", work->speed );
				break;
			  case TELOP_CODE_BMP:
printf( "--TELOP %d\n", work->telop_y / 16 );
				work->telop_state = TELOP_DECODE;
				{
					short width;
					short height;
					memcpy( &width, work->telop_data + 1, sizeof( short ) );
					memcpy( &height, work->telop_data + 3, sizeof( short ) );
               BP_LE_SwapSShort_Inp( &width );
               BP_LE_SwapSShort_Inp( &height );
					printf( "width = %d heigt = %d\n", width, height );
					work->telop_data += 5;
				}
				zlib_init( work );
				break;
			}
			goto RETRY;

		  case TELOP_DECODE:
			{
				u_char *target;
				int result;
				target = (u_char*)work->pTexTelop->image
					+ ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
				result = zlib_decode( target, TELOP_DECODE_BUF_WIDTH * sizeof( int ), work );
				if( result == 0 ){
					// 終了
					zlib_finish();
					work->telop_state = TELOP_WAIT;
					goto RETRY;
				}
#ifdef KP_WINDOWS
				DG_LinerTextureSetImageDirty(work->pTexTelop) ;	// 変更適用
#endif
			}
			break;
		  case TELOP_BLANK:
			{
				u_char* target;
				work->blank_count --;
				target = (u_char*)work->pTexTelop->image
					+ ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
				memset( target, 0, TELOP_DECODE_BUF_WIDTH * sizeof( int ) );

				if( work->blank_count <= 0 ){
					work->telop_state = TELOP_WAIT;
					goto RETRY;
				}
#ifdef KP_WINDOWS
				DG_LinerTextureSetImageDirty(work->pTexTelop) ;	// 変更適用
#endif
			}
		    break;
		  case TELOP_STOP:
			{
				work->blank_count --;
				if( work->blank_count <= 0 ){
					work->telop_state = TELOP_WAIT;
					goto RETRY;
				}
				goto SET;
			}
			break;
		  case TELOP_FADE:
			{
				if( work->telop_fade <= 0 ){
					work->telop_state = TELOP_WAIT;
					goto RETRY;
				}
				work->telop_fade -= work->telop_fade_step;
				goto SET;
			}
			break;
		  case TELOP_END:
			work->telop_end_flag = 1;
//			DG_DequeueDmapack( work->pDmapackTelop );
#ifndef KP_WINDOWS
			if( !work->movie_flag ) GV_DestroyActor( work );
#else
			if( !work->movie_flag ) {
				GV_DestroyActor( work );
				work->destroyed = TRUE ;
			}
#endif
			return;
		}
		work->telop_y += 16;

		dec_y = ( dec_y + 1 ) % TELOP_DECODE_NET_HEIGHT;
		disp_y = ( disp_y + 1 ) % TELOP_DECODE_NET_HEIGHT;
	}
	work->telop_y = ty;

	work->decode_y = dec_y;
	work->display_y = disp_y;

	set_telop_sprt( work, ty % 16 );

	return;
SET:
	set_telop_sprt( work, 0 );
}

/* ---------------------------------------------------------------------- */
/*
	Act, Die
*/

static void Die( ENDING_WORK *work )
{

	DG_DequeueDmapack( work->pDmapackTelop );
	DG_FreeDmapack( work->pDmapackTelop );
	DG_FreeLinerTexture( work->pTexTelop );
	
	GV_SetActorFreeFunc( work, GV_DelayedFree );
	
	if( work->end_proc != 0 ) {
		GCL_ExecProc( work->end_proc, NULL );
	}
}

/* ---------------------------------------------------------------------- */
/*
	ACTOR起動
*/

static void WriteTitleName( ENDING_WORK* work )
{	// スネークテイルズ用・タイトルを描く
	short width;
	short height;
	int dec_y;
	int result;
	
	ASSERT( *work->telop_data == TELOP_CODE_BMP );
	memcpy( &width, work->telop_data + 1, sizeof( short ) );
	memcpy( &height, work->telop_data + 3, sizeof( short ) );
   BP_LE_SwapSShort_Inp( &width );
   BP_LE_SwapSShort_Inp( &height );
	printf( "title width = %d heigt = %d\n", width, height );
	work->telop_data += 5;
	zlib_init( work );

	dec_y = ( TELOP_HEIGHT - height ) / 2;

	// decode
	do {
		u_char* target;
		target = (u_char*)work->pTexTelop->image
				+ ( dec_y * TELOP_DECODE_BUF_WIDTH ) * sizeof( int );
		result = zlib_decode( target, TELOP_DECODE_BUF_WIDTH * sizeof( int ), work );
		++dec_y;
	} while (result != 0);
	
	// 終了
	zlib_finish();
}

static int GetResources( ENDING_WORK *work )
{
	int cache;
	int titlename;

	int end_proc;
	int pos;
	
	/*
		表示関係
	*/
	{
		// telop
		// Dmapack 準備
		work->pDmapackTelop = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_AFTER, 250 );
		DG_QueueDmapack( work->pDmapackTelop );
		work->pDmapackTelop->autopacket = work->bufferTelop;
		DG_SetDmapackEnd( work->bufferTelop );
		// テクスチャ領域準備
		work->pTexTelop = DG_MakeLinerTexture( TELOP_DECODE_BUF_WIDTH, TELOP_DECODE_BUF_HEIGHT, DG_TEXLIN_FORMAT_A8R8G8B8 );
		GV_ZeroMemory( work->pTexTelop->image, TELOP_DECODE_BUF_WIDTH * TELOP_DECODE_BUF_HEIGHT * 4 );
	}
	
	work->decode_y = TELOP_DECODE_BUF_HEIGHT - 4;
	work->display_y = 0;
	work->telop_y = 0;
	work->telop_end_flag = 0;
	
	work->prev_y = 0;
	
	/* データ取得 */

	cache = GCL_GetOptionValue( 't', 0 );
	ASSERT( cache != 0 );

	titlename = GCL_GetOptionValue( 'n', 0 );
	if( titlename != 0 ) {
		work->telop_data = GV_GetCache( GV_CacheID( titlename, 'r' ) );
		ASSERT( work->telop_data != NULL );
		WriteTitleName( work );
		work->start_stop = GCL_GetNextInt();
		work->start_stop = DIRECT_TICK( work->start_stop );
		work->telop_fade = 0;
	} else {
		work->start_stop = 0;
		work->telop_fade = 256;
	}
	work->telop_state = TELOP_TITLE;

	work->speed = GCL_GetOptionValue( 's', 17 );
	
	work->telop_data = GV_GetCache( GV_CacheID( cache, 'r' ) );
	ASSERT( work->telop_data != NULL );

	if( GCL_GetOption( 'p' ) != NULL ){
		end_proc = GCL_GetNextInt();
	} else {
		end_proc = 0;
	}
	if( GCL_GetOption( 'f' ) != NULL ){
		pos = GCL_GetNextInt();
	} else {
		pos = -1;
	}
#if 0
	if( pos != -1 ) {
		GV_SetActorChild( NewMpegPssMovieStrProg( end_mv_info, pos, end_proc, FALSE, 0 ),
						  work );
		work->movie_flag = TRUE;
		work->end_proc = 0;
	} else {
#endif
		work->movie_flag = FALSE;
		work->end_proc = end_proc;
#if 0
	}
#endif
	
	
//	work->tick = 0;

   if ( BP_IsPAL()==TRUE )
   	work->fOver = 0.f;

#ifdef KP_WINDOWS
	work->pre_tick  = GET_ENDING_TICK() - 1 ;
	work->destroyed = FALSE ;
#endif

	return 0;
}

void *NewEnding( int name, int map )
{
	ENDING_WORK *work;

	work = ( ENDING_WORK * )GV_NewActor( GV_ACTOR_PREV2, sizeof( ENDING_WORK ) );

	if( work != NULL ){
		GV_ActorEX( &( work->actor ) );
		GV_SetActor( &( work->actor ), Act, Die );

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}

#endif
