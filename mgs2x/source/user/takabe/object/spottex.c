//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	spottex.c
	スポットライト背景イメージ指定

	2000/05/18 K.Takabe
	$Id: spottex.c,v 1.7 2002/12/04 05:24:52 takaki Exp $

*/
/*

chara スポットライト背景イメージ指定[NewSpotLightImageSet] $s:name \
	-image $s:テクスチャ名（raw形式イメージなので注意！（RGBARGBA・・・））\
	-file $s:ＩＰＵデータ名（stm形式をraw形式にリネームしたもの）\
	-movie $t:ムービー指定（ＩＰＵ形式ストリーム） \
	-proc $p:終了コールバック（ムービー指定を行ったときのみ）
mesg スポットライト背景イメージ指定 $s:名前 停止[0]
mesg スポットライト背景イメージ指定 $s:名前 イメージ変更[1]
mesg スポットライト背景イメージ指定 $s:名前 ムービー再生[2] $t:ムービー指定（ＩＰＵ形式ストリーム）
mesg スポットライト背景イメージ指定 $s:名前 テクスチャ許可[3] $w:オン・オフ指定
// スポットライト投影のプリミティブ描画のスポットライト模様の変わりに
// 使用する背景イメージを指定する
// IPUストリームによるムービーも再生可能
// IPUストリーム再生時に再生開始直後は$1=0、再生終了後直後は$1=1が設定され
// コールバックが呼ばれる
// オプションに-image,-file,-movieのいずれの指定も行わない場合はミックスストリーム
// モードになり、他のストリーム内に存在するＩＰＵストリームデータを参照しに行く
// （＝このキャラ自身はストリームの管理を行わずに、データのみを奪う。デモなどに使用）


*/

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
#include <libipu.h>
#endif

#include "BP_MovieSupport.h"
#include "../../../game/ipu.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"gameheader.h"
#include	"libfs.h"
#include	"strctrl.h"
#include	"ipu.h"

#ifdef KP_XBOX
#ifdef KP_WINDOWS
#include "winstream.h"	
#else
#include "cristream.h"	
#endif
#endif

#define CLAMP( v, max, min ) {(v)=((v)>min)?(v):min;(v)=((v)<max)?(v):max;}

DG_TEX *DG_SpotLightBaseTexture = NULL;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	int			where ;
	int			data_id ;		/* データＩＤ */
	void		*data_ptr ;
	int			mode ;			/* 動作モード */
	int			tex_enable ;	/* テクスチャを本当に設定するかどうかのフラグ */

	/* メモリストリーム関連ワーク */
	void		*mem_ipu ;		/* メモリデータアドレス */
	void		*mem_ipu_work ;	/* ＩＰＵ再生アクター用ワーク */

#ifndef KP_WINDOWS
	int			active_buffer ;	/*  */
#endif
	int			handle ;		/* ストリームハンドル */
	int			proc ;			/* 再生終了proc */
	int			count ;

#ifndef KP_WINDOWS
	u_int		*ImageBuffer[2] ;	/* スポットライト設定テクスチャバッファ */
#else
	u_int		*ImageBuffer ;		/* スポットライト設定テクスチャバッファ */
	DG_TEX_LIN	texlin ;			/* スポットライト設定テクスチャ */
#endif

   BP_Movie_Handle movie_handle;

} SpotTexWork ;

/* ---------------------------------------------------------------- */
enum {
	MODE_STREAM_IPU		= 0,		/* 通常ストリームＩＰＵ */
	MODE_MEMORY_IPU		= 1,		/* オンメモリストリームＩＰＵ */
	MODE_FIX_TEX		= 2,		/* 固定テクスチャ */
	MODE_MIXED_IPU		= 3,		/* ミックスストリームＩＰＵ */
};

/* ---------------------------------------------------------------- */
static void ExecProc( SpotTexWork *work, int mode );


/* ---------------------------------------------------------------- */
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
static void *MallocBuffer( int size )
{
	void	*mem ;
#ifdef PSX2
	mem = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, ALIGNSIZE128(size), 128 );
#else
	mem = GV_AllocMemory( GV_NORMAL_MEMORY, NULL, ALIGNSIZE128(size), 128 );
	//mem = DG_AllocLocalVideoMemoryAlign( size, 256 );
#endif
	return mem ;
}
static void FreeBuffer( void *addr )
{
#ifdef PSX2
	if ( addr != NULL ) GV_DelayedFree( addr );
#else
	if ( addr != NULL ) GV_DelayedFree( addr );
	//if ( addr != NULL ) DG_DelayedFreeLocalVideoMemory( addr );
#endif
}

/* ---------------------------------------------------------------- */
/* 各種再生開始処理 */
static void Start( SpotTexWork *work, int data_id )
{
	work->data_id = data_id;

	switch ( work->mode )
   {
	  case MODE_STREAM_IPU:	/* ストリーミングＩＰＵ */
		if ( work->handle != -1 )
      {
			if ( GM_StreamStatus( work->handle ) != GM_STREAM_STATE_END )
         {
				GM_StreamStop( work->handle );
			}
		}
		work->handle = GM_IpuStream( work->data_id, GM_STREAM_PLAY_WAIT );
		break;

	  case MODE_MEMORY_IPU:	/* メモリＩＰＵ */
		if ( work->mem_ipu_work != NULL )
      {
			GV_DestroyActorQuick( work->mem_ipu_work );
		}
		
      /* データ取得 */
		work->mem_ipu = GV_GetCache( GV_CacheID( work->data_id, 'r' ) );

		/* ＩＰＵメモリストリーム再生開始 */
		if ( work->mem_ipu != NULL )
      {
			work->mem_ipu_work = NewMemStreamIpu( work->mem_ipu, GM_IPU_MODE_RGB16, &work->movie_handle );
		}
		break;

	  case MODE_FIX_TEX:	/* 固定テクスチャ */
		/* データ取得 */
		work->data_ptr = GV_GetCache( GV_CacheID( work->data_id, 'r' ) );
		break;

	  case MODE_MIXED_IPU:
		break ;
	}
}

/* 各種再生停止処理 */
static void Stop( SpotTexWork *work )
{
	switch ( work->mode ){
	  case MODE_STREAM_IPU:	/* ストリーミングＩＰＵ */
		if ( work->handle != -1 )
      {
			if ( GM_StreamStatus( work->handle ) != GM_STREAM_STATE_END )
         {
				GM_StreamStop( work->handle );
			}
		}
		break;

	  case MODE_MEMORY_IPU:	/* メモリＩＰＵ */
		if ( work->mem_ipu_work != NULL )
      {
			GV_DestroyActor(work->mem_ipu_work);
		}
		break ;
	  case MODE_FIX_TEX:	/* 固定テクスチャ */
		DG_SpotLightBaseTexture = NULL ;
		break ;
	  case MODE_MIXED_IPU:
		break ;
	}
}
/* ---------------------------------------------------------------- */
/* ストリームＩＰＵ再生処理 */
static void ActStreamIpu( SpotTexWork *work )
{
	int		state ;

	if ( work->handle != -1 ){
		state = GM_StreamStatus( work->handle );
		switch ( state ){
		  case GM_STREAM_STATE_INIT_WAIT:
		  case GM_STREAM_STATE_INIT:
			break ;
		  case GM_STREAM_STATE_WAIT:
			ExecProc( work, 0 );
			GM_StreamStart( work->handle );
			break ;
		  case GM_STREAM_STATE_PLAY:
		  case GM_STREAM_STATE_READ_END:
		  case GM_STREAM_STATE_END:
			//if ( ( work->count++ ) & 3 ) return ;	/* ４フレームに１回のみ実行 */
			if ( GM_StreamGetIpu() ){/* ＩＰＵ展開バッファからテクスチャイメージバッファに変換コピー */
				u_int	*buffer ;
				/* テクスチャ保持バッファの切り替え */
#ifndef KP_WINDOWS
				work->active_buffer = 1 - work->active_buffer ;
				buffer = work->ImageBuffer[ work->active_buffer ];
#else
				buffer = work->ImageBuffer;
#endif
				/* ＩＰＵ展開後マクロブロック形式からラスタ形式に変換してテクスチャバッファにコピー */
				//GM_StreamCopyIpuImage( buffer, 256, 256, 0, 0, 0 );
				{
					int		w, h, f ;
#ifndef KP_WINDOWS
					GM_StreamGetIpuInfo( &w, &h, &f ) ;
					GM_StreamCopyIpuImage( buffer, w, h, (256-w)/2, (256-h)/2, 0 );
#else
					if ( GM_StreamGetIpuInfo( &w, &h, &f ) != -1 ) {
						GM_StreamCopyIpuImage( buffer, w, h, (256-w)/2, (256-h)/2, 0 );

						if ( work->tex_enable == 0 ) {
							DG_SpotLightBaseTexture = NULL ;
						} else {
							DG_LinerTextureSetImageAddr(&work->texlin, buffer) ;
							DG_SpotLightBaseTexture = &work->texlin ;
						}
					}
#endif
				}
				/* ＩＰＵ保持イメージを開放 */
				GM_StreamFreeIpu();
				/* テクスチャの設定 */
#ifndef KP_WINDOWS
				//MC DG_SpotLightBaseImage = buffer ;
				//MC if ( work->tex_enable == 0 ) DG_SpotLightBaseImage = NULL ;
#endif
			}
			if ( state == GM_STREAM_STATE_END ){
				/* 終了proc呼び出し */
				ExecProc( work, 1 );
				work->handle = -1 ;
			}
			break ;
		}
	}
}
/* ---------------------------------------------------------------- */
/* オンメモリＩＰＵ再生処理 */
static void ActMemIpu( SpotTexWork *work )
{
   if ( GM_StreamGetIpu() )
   {	
      if ( work->tex_enable == 0 ) 
      {
         DG_SpotLightBaseTexture = NULL ;
      } 
      else 
      {
         DG_SpotLightBaseTexture = BP_Movie_Get_Texture(work->movie_handle);
      }

      GM_StreamFreeIpu();
   }
}
/* ---------------------------------------------------------------- */
/* ミックスＩＰＵ再生処理 */
static void ActMixedIpu( SpotTexWork *work )
{
	if ( GM_StreamGetIpu() )
   {	
		if ( work->tex_enable == 0 ) 
      {
			DG_SpotLightBaseTexture = NULL ;
		} 
      else 
      {
			DG_SpotLightBaseTexture = BP_Movie_Get_Texture(work->movie_handle);
		}
		
		GM_StreamFreeIpu();
	}
}
/* ---------------------------------------------------------------- */
static void ExecProc( SpotTexWork *work, int mode )
{
	GCL_ARGS	arg ;
	int			data[4] ;

	if ( work->proc == 0 ) return ;
	arg.argc = 1 ;
	arg.argv = data ;
	data[0] = mode ;
	GCL_ExecProc( work->proc, &arg );
}
/* ---------------------------------------------------------------- */
static void Act( SpotTexWork *work )
{
	//extern int DG_ActiveShadowFlag ;
    //int		state ;

	//DG_ActiveShadowFlag = 2 ;
	{/* メッセージチェック */
		GV_MSG *msg;
		int n;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for ( ; n > 0 ; n--, msg++ ){
				switch ( msg->message[0] ){
				  case 1:/* Change */
					/* 固定テクスチャの入れ替え */
					work->data_id = msg->message[1] ;
					work->data_ptr = GV_GetCache( GV_CacheID( work->data_id, 'r' ) );
#ifndef KP_WINDOWS
					//MC DG_SpotLightBaseImage = work->data_ptr ;
#else
					if ( !work->data_ptr ) {
						DG_SpotLightBaseTexture = NULL ;
					} else {
						DG_LinerTextureSetImageAddr(&work->texlin, work->data_ptr) ;
						DG_SpotLightBaseTexture = &work->texlin ;
					}
#endif
					break ;
				  case 0:/* Stop */
					Stop( work );
					break ;
				  case 2:/* movie change */
					Start( work, msg->message[1] );
					break ;
				  case 3:/* tex enable */
					work->tex_enable = msg->message[ 1 ];
					return ;
				}
			}
		}
	}

	switch ( work->mode ){
	  case MODE_STREAM_IPU:
		ActStreamIpu( work );
		break ;
	  case MODE_MEMORY_IPU:
		ActMemIpu( work );
		break ;
	  case MODE_FIX_TEX:
		break ;
	  case MODE_MIXED_IPU:
		ActMixedIpu( work );
		break ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( SpotTexWork *work )
{
	Stop( work );
	DG_SpotLightBaseTexture = NULL ;
	GM_StreamIpuDriverEnd();
}

/* ---------------------------------------------------------------- */
static int GetResources( SpotTexWork *work, int name, int where )
{
	work->name = name ;
	work->where = where ;
	work->handle = -1 ;
	work->tex_enable = 1 ;
	work->mode = MODE_MIXED_IPU ;	/* デフォルトはミックスＩＰＵストリームモード */

	/* テクスチャ指定 */
    if ( GCL_GetOption( 'i' ) != NULL ) {
		work->mode = MODE_FIX_TEX ;
		work->data_id = GCL_GetNextInt() ;
		/* 開始 */
		Start( work, work->data_id );
	}
	/* ＩＰＵファイル指定 */
    if ( GCL_GetOption( 'f' ) != NULL ) {
		work->mode = MODE_MEMORY_IPU ;
		work->data_id = GCL_GetNextInt() ;
		/* 開始 */
		Start( work, work->data_id );
	}
	/* ＩＰＵストリーム再生 */
    if ( GCL_GetOption( 'm' ) != NULL ) {
		work->mode = MODE_STREAM_IPU ;
		work->data_id = GCL_GetNextInt() ;
		printf("movie id = %d\n", work->data_id );
		/* ＩＰＵストリームドライバ起動 */
		work->movie_handle = GM_StreamIpuDriverInit(256, 256);
		/* 開始 */
		Start( work, work->data_id );
	}
    if ( GCL_GetOption( 'p' ) != NULL ) {
		work->proc = GCL_GetNextInt() ;
	}
	/* ミックスストリーム時はドライバを起動するだけ */
	if ( work->mode == MODE_MIXED_IPU ){
		/* ＩＰＵストリームドライバ起動 */
		work->movie_handle = GM_StreamIpuDriverInit(256, 256);
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewSpotLightImageSet( int name, int where )
{
	SpotTexWork		*work ;
	OPERATOR() ;
	work = (SpotTexWork *)GV_NewActor( GV_ACTOR_AFTER, sizeof( SpotTexWork ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_SetActorMessageKill( work, name );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
