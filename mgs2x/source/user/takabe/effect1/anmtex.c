//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	anmtex.c
	アニメーションテクスチャ展開実験プログラム

	2001/06/04 K.Takabe
	$Id: anmtex.c,v 1.1.1.3 2002/11/19 11:51:08 Yoshizawa1 Exp $

*/
/*

chara アニメテクスチャセット[NewAnmtexSet] $s:name \
	-tex $s:ＴＲＩＩＤ $s:テクスチャＩＤ \
	-anmtex $s:アニメテクスチャＩＤ \
	-interval $w:再生間隔（デフォルト＝１） \
	-flag $w:各種フラグ
mesg アニメテクスチャセット $s:名前 start[0]	// 再生処理を再開
mesg アニメテクスチャセット $s:名前 stop[1]		// 再生処理を停止
mesg アニメテクスチャセット $s:名前 step[2]		// １フレームのみアニメーションを進ませる
// アニメテクスチャデータはdpackのanmtexコマンドで
// FIXの.BMPファイルから.rat形式データを生成する必要がある
// フラグ説明
//  0x0001:ループ再生
//  0x0002:停止状態で起動

//  0x0004:パレットを転送しない（テクセルデータのみアニメーション）


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../../mode/demo/libdemo.h"

#include	"utl_anmtex.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_EndianSupport.h"

/* ------------------------------------------------------------ */
/* プログラムワーク */
typedef	struct anmtex_Work	{
	GV_ACT_EX	actor ;
	int			name ;
	int			map ;
	int			tri_id ;				/* ＴＲＩファイルＩＤ */
	int			tex_id ;				/* テクスチャＩＤ */
	int			anmtex_id ;				/* アニメテクスチャデータＩＤ */

	int			interval_frame ;		/* 再生間隔（フレーム単位） */
	int			flag ;					/* 実行フラグ */
	int			count ;					/* カウンタ */

	int			mode ;					/* 再生モード */

	/* アニメテクスチャ展開ワーク */
	ANMTEX_WORK	anmtex_work ;			/* アニメテクスチャ再生ワーク */
   unsigned int   bp_tex;

   DG_DMAPACK *pDmapack;
   int updateTextureClock;
}
SAnmTexWork;

enum {
	FLAG_LOOP			= 0x0001,		/* ループフラグ */
	FLAG_IDLESTART		= 0x0002,		/* 停止状態スタート */
	FLAG_NOPALETTE		= 0x0004,		/* パレット転送禁止 */
};

/* ---------------------------------------------------------------- */
/* アクターメイン処理 */
static void Act( SAnmTexWork *work )
{
	int		n_msg, i ;
	GV_MSG	*msg ;

	if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) != 0 ){
		/* メッセージチェック */
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* 再生開始 */
				printf("%s:play start\n", __FILE__);
				work->mode = 0 ;
				break ;
			  case 1:/* 再生中止 */
				printf("%s:play stop\n", __FILE__);
				work->mode = 1 ;
				break ;
			  case 2:/* １フレーム再生 */
				work->mode = 2 ;
				break ;
			}
		}
	}

   // Don't set updateTextureClock to -1 here, because if two Act calls
   // happen, we don't want update to be set to yes and then no
   // So we wait until the buffer call

   switch ( work->mode )
   {
   case 0:
      {
         // Arm fix: removed old_clock nonsense
         // if the animated texture referenced the previous texture (XOR compression)
         // the old_clock stuff would break it as it wouldn't swap buffers between two updates in the same frame

         // Arm fix: don't rely on DM_FrameSkip as it's different on Vita
         // part of the fix for MGSTWO-2783, MGSTWO-3202
         for ( i = 0 ; i < ( 1 /* + DM_FrameSkip */ ) ; i++ )
         {
            /* 再生処理 */
            work->count++ ;
            if ( work->count >= work->interval_frame )
            {
               /* アニメテクスチャ展開（１回の呼び出しで１フレーム進む） */
               int bp_needsNewUpdate = UTL_ActAnmtex( &work->anmtex_work );
               work->count = 0 ;

               if (bp_needsNewUpdate)
               {
                  work->updateTextureClock = work->anmtex_work.buffer_clock;
               }
            }
         }
      }
	   break ;
   case 1:
	   GV_WaitMessage( work, work->name );
	   break ;
   case 2:
      {
	      int bp_needsNewUpdate = UTL_ActAnmtex( &work->anmtex_work );
         work->mode = 1 ;

         if (bp_needsNewUpdate)
         {
            work->updateTextureClock = work->anmtex_work.buffer_clock;
         }
      }
	   break ;
   }
}

/* ---------------------------------------------------------------- */
/* アクター終了処理 */
static void Die( SAnmTexWork *work )
{
   if (work->pDmapack != NULL)
   {
      DG_DequeueDmapack(work->pDmapack);
      DG_FreeDmapack(work->pDmapack);
   }
   if( work->bp_tex )
   {
      BP_FreeDynamicTexture(work->bp_tex);
      work->anmtex_work.tex->BP_TextureHandle = 0;
   }

   /* アニメテクスチャ展開ワークの開放 */
	UTL_FreeAnmtexWork( &work->anmtex_work );
}

/* ---------------------------------------------------------------- */
// Callback system so that the render buffer commands are executed in the same thread

static void *_anmtex_buffer_callback(void *pParam, DG_DMAPACK *pDstDmaPack)
{
   SAnmTexWork *work = (SAnmTexWork *)pParam;
   void *dst_work = DG_AS_SceneBufferAllocCopy( work, sizeof( *work ) );

   // We only need to update the texture for the renderer once
   // so wait until the flush to actually update it
   
   // Only has an effect in buffered situations.
   // This callback may not be called!
   work->updateTextureClock = -1;

   return dst_work;
}

static void _anmtex_callback(void *pWork)
{
   SAnmTexWork *work = (SAnmTexWork *)pWork;
   if (work->updateTextureClock != -1)
   {
      SBP_UpdateDynamicTexture* pPacket = (SBP_UpdateDynamicTexture*)BP_RB_Alloc(sizeof(SBP_UpdateDynamicTexture));

      int buffer = work->updateTextureClock;

      // make this the same as MGS3
      if (!strcmp(GM_GetArea(), "n_title"))
      {
         // If we're in the title, there's no need to copy the animated textures
         // to another buffer.
         pPacket->vram = work->anmtex_work.tex_image[buffer];
         pPacket->clut = work->anmtex_work.tex_clut[buffer];
      }
      else
      {
         // Otherwise "triple buffer" by copying the images into the render buffer.

         pPacket->vram = BP_RB_Alloc( work->anmtex_work.image_size );
         pPacket->clut = BP_RB_Alloc( work->anmtex_work.clut_size );

         memcpy( pPacket->vram, work->anmtex_work.tex_image[buffer], work->anmtex_work.image_size );
         memcpy( pPacket->clut, work->anmtex_work.tex_clut[buffer], work->anmtex_work.clut_size );
      }

      pPacket->bp_tex = work->bp_tex;
      pPacket->bitsPerPixel = ((BP_LE_SwapSInt(work->anmtex_work.anmtex_header->flag_le) & ~ANMTEX_HEADER_FLAG_ENDIANSWAPPED) == 1) ? 8 : 4;

      BP_RB_AddCommand(kCmd_UpdateDynamicTexture, (char*)pPacket);

      // Only has an effect in non-buffered situations
      work->updateTextureClock = -1;
   }
}

static void SetupDmapack(SAnmTexWork *work)
{
   work->pDmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL, DG_DMAPACK_PHASE_NORMAL );

   if ( work->pDmapack == NULL )
   {
      printf("Null DmaPackPrim -> /user/takabe/effect1/anmtex.c\n");
      return;
   }

   DG_AS_DMAPackSetCallbacks(work->pDmapack, _anmtex_callback, _anmtex_buffer_callback);
   work->pDmapack->BP_callbackParam = work;

   // Arm fix:
   // Queue the DMA pack!
   // fix for MGSTWO-2783, MGSTWO-3202
   DG_QueueDmapack(work->pDmapack);

   work->updateTextureClock = -1;
}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */
static int GetResources( SAnmTexWork *work, int name, int where )
{
	int		anmtex_flag = 0 ;

	work->name = name ;
	work->map = where ;

	/* シナリオ起動時のオプションを解析 */

	/* 差し替えテクスチャ設定を取得 */
	if ( GCL_GetOption( 't' ) != NULL ){
		work->tri_id = GCL_GetNextInt();
		work->tex_id = GCL_GetNextInt();
	}

	/* アニメテクスチャファイルの設定 */
	if ( GCL_GetOption( 'a' ) != NULL ){
		work->anmtex_id = GCL_GetNextInt();
	}

	/* フレームレートの設定 */
	if ( GCL_GetOption( 'i' ) != NULL ){
		work->interval_frame = GCL_GetNextInt();
	}
	if ( work->interval_frame < 1 ) work->interval_frame = 1 ;

	/* 各種フラグの設定 */
	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt();
	}
	if ( work->flag & FLAG_LOOP ){
		anmtex_flag |= ANMTEX_FLAG_LOOP ;
		printf("%s:loop mode\n", __FILE__);
	}
	if ( work->flag & FLAG_IDLESTART ){
		work->mode = 1 ;
		printf("%s:idle start mode\n", __FILE__);
	printf("%s:start mode_a %d\n", __FILE__, work->mode );
	}
	printf("%s:start mode_b %d\n", __FILE__, work->mode );

	/* アニメテクスチャ展開ワーク初期化 */
	if ( UTL_InitAnmtexWork( &work->anmtex_work, work->tri_id, work->tex_id, work->anmtex_id, anmtex_flag ) < 0 ){
		/* 初期化失敗 */
		return ( -1 );
	}

   {
      int w, h, offX, offY;
      DG_GetTexelInfo(&w, &h, &offX, &offY, work->anmtex_work.tex);
      work->bp_tex = BP_AllocDynamicTexture(w, h, 1);

      work->anmtex_work.tex->BP_TextureHandle = work->bp_tex;
   }

	printf("%s:start mode %d\n", __FILE__, work->mode );

   SetupDmapack(work);
	return (0);
}

/* シナリオ起動関数 */
void *NewAnmtexSet( int name, int where )
{
	SAnmTexWork		*work ;

	OPERATOR() ;
	//work = (SAnmTexWork *)GV_NewActor( GV_ACTOR_USER, sizeof( SAnmTexWork ) ) ;	/* 旧形式 */
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(SAnmTexWork), 0 );	/* 新形式 */
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
/* リソース確保関数 */

#if 0 //BP_UNUSED

static int GetResourcesP( SAnmTexWork *work, int tri_id, int tex_id, int anmtex_id, int interval, int flag )
{
	int		anmtex_flag = 0 ;

	work->name = 6923105 ;/* "NewAnmtex" */
	work->map = GM_CurrentMap ;

	work->tri_id = tri_id ;
	work->tex_id = tex_id ;
	work->anmtex_id = anmtex_id ;
	work->interval_frame = interval ;
	if ( work->interval_frame < 1 ) work->interval_frame = 1 ;
	work->flag = flag ;

	if ( work->flag & FLAG_LOOP ) anmtex_flag |= ANMTEX_FLAG_LOOP ;
	if ( work->flag & FLAG_IDLESTART ) work->mode = 1 ;

	/* アニメテクスチャ展開ワーク初期化 */
	if ( UTL_InitAnmtexWork( &work->anmtex_work, work->tri_id, work->tex_id, work->anmtex_id, anmtex_flag ) < 0 ){
		/* 初期化失敗 */
		return ( -1 );
	}

	return (0);
}

/* プログラム起動関数 */
void *NewAnmtex( int tri_id, int tex_id, int anmtex_id, int interval, int flag )
{
	SAnmTexWork		*work ;

	OPERATOR() ;
	//work = (SAnmTexWork *)GV_NewActor( GV_ACTOR_USER, sizeof( SAnmTexWork ) ) ;	/* 旧形式 */
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(SAnmTexWork), 0 );	/* 新形式 */
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResourcesP( work, tri_id, tex_id, anmtex_id, interval, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#endif