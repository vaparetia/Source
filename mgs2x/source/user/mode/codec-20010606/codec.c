/*
	codec.c
		無線／表示部
	2000/07/24      Y.Kira
	2000/07/17	K.Uehara
	$Id: codec.c,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec_config.h"
#include "codec.h"

#include "g_define.h"
#include "font.h"
#include "cjimaku.h"
#include "libfs.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"
#include "c_layout.h"
#include "cdc_load.h"
#include "stream.h"
#include "cdc_mind.h"
#include "cdc_hair.h"
#include "codec_signal.h"

#ifdef _DEBUG_
#define DBG(args...) printf(args)
#else
#define DBG(args...)
#endif  /* _DEBUG_ */

/*
	コンフィグレーション
*/

#define MESG_PANEL_W		(512-64+FONT_SIZE_W)
#define MESG_PANEL_H		((FONT_SIZE_H+FONT_SIZE_H/2)*3+FONT_SIZE_H/2)

#define CALL_MIN_WAIT           300
#define NORMAL_CALL_TIME	((60*10) * 5)
#define DEMO_CALL_TIME		((60*2) * 5)
//#define FADE_TIME			(60*1)

#ifdef _CODEC_WITHOUT_FADE_
#define FADE_TIME		(10 * 5)
#else
#define FADE_TIME               (60 * 5)
#endif /* _CODEC_WITHOUT_FADE_ */

/* #define FADE_TIME    30    */
#define CALL_BLINK(a)		((a)%60<20)
#define SELECT_FIRST_REPEAT	30

#define FREQ_MIN		14000
#define FREQ_MAX		14199

#define ITP_STACK_SIZE	(16*1024)

#define RADAR_TEX   7429504
#define CALL_ICON   2133853
#define CALL_WIDTH  82.0F
#define CALL_HEIGHT 26.0F

#define CALL_BLNK   15.0F

#define CALL_POS_X  (256.0F - CALL_WIDTH / 2)
#define CALL_POS_Y  90.0F

#define STRCODE_in  3470
#define STRCODE_out 117524

#ifdef PAL
#define TIMEBASE  6
#else
#define TIMEBASE 5
#endif /* PAL */

extern void * GM_PlayerWork;

static int    last_freq = FREQ_MIN;
static int    last_responsed_freq = -1;
static int    response_count = 0;
static void * bg_work = NULL;
static void * codec_parts;  /* 無線画面素材 */

/*
	解釈部は別スレッドとして起動。
	表示部は解釈部からのリクエストを受け取る。
*/


/* 表示部 */

typedef struct _work {
  GV_ACT_EX	actor;
  int name;
  
  GV_PAD *pad;
  void (*do_func)( struct _work *work );
  
  /* 解釈部スレッド管理 */
  CODEC_ITP_PARAM	param;
  
  /* 起動用 */
  int	status;
  int count;
  
  char type;
  char fade_flag;
  short freq;
  short freq_tmp;
  int code;
  int proc_id;

  int   dummy_bone;
  
  /* 各モードローカルパラメータ */
  union {
    /* セレクトモード用パラメータ */
    struct selectinf {
      int repeat_count;
      int prev_dir;
    } select;
  } inf;
  
  /* 描画用パラメータ */
  struct {
    int call;
  } draw;
  CODEC_MESG_PANEL panel;

  void * layout_work;

  int call_cnt;
  int main_vox_handle;
  int pre_cap_status;
  int finish_cnt;
  int    fade_param;
  void * face_work[2];

  char    finish_id;

  int memcall_status;  /* メモリー呼出結果 */

  /* フェードアウト用マスク */
  SPR_OBJ * fade_mask;

  /* CALL アイコンの UV アニメーション用パラメタ */
  SPR_OBJ * call_sprite;
  SPR_OBJ * call_over;
  SPR_FIX   call_tex_u;
  SPR_FIX   call_tex_v;
  SPR_FIX   call_tex_w;
  SPR_FIX   call_tex_h;

  int     fadein_break:1;
  int     fadeout_break:1;
  int     fade_mode:1;    /* 0: 通常の無線モード開始 / 1: フェードイン開始   */
  int     fade_out:1;     /* 0: 通常の無線モード終了 / 1: フェードアウト終了 */
  int     act_open:1;     /* 0: 開きアクションなし   / 1: 開きアクションあり */
  int     act_close:1;    /* 0: 閉じアクションなし   / 1: 閉じアクションあり */
  int     voice_cancel:1; /* 0: 音声キャンセル前     / 1: 音声キャンセル後   */
  int     vox_playing:1;  /* 0: 音声再生中ではない   / 1: 音声再生中         */
  int     autoskip:1;     /* 0: 要キー入力           / 1: オートスキップ     */
  int     call_block:1;   /* 0: 制限無し             / 1: 呼出/応答不可      */

} Work;

static Work *codec_work = NULL;


enum {
	TYPE_NORMAL = 1,		// 通常選択画面へ
	TYPE_CALL = 2,			// CALLが来たが無視可能
	TYPE_DEMO = 3,			// CALLが来たが強制的に移行
	TYPE_DIRECT = 4,		// CALLなしで強制的に移行
};


int codecGetLastResponsedFreq(void)
{
  return last_responsed_freq;
}

int codecGetResponseCount(void)
{
  return response_count;
}

static inline void SetFunc( Work *work, void (*func)( Work *work ) )
{
	work->do_func = func;
	work->status = 0;
}

void SetFaceWork(int side, void * face_wk)
{
  codec_work->face_work[side] = face_wk;
}

static void setup_fade_mask(Work * work)
{
      /* フェードアウト用のスプライトを用意する */
      work->fade_param = 128;
      if(NULL != work->fade_mask) SPR_Destroy_2D_Object(work->fade_mask);
      work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
      work->fade_mask->head.alpha =
	SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
      SPR_SetPriority(work->fade_mask, 7);
      work->fade_mask->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
      SPR_SetSizeSprite(work->fade_mask, 512.0F, 384.0F);
      SPR_SetPosSprite(work->fade_mask, &(SPR_POS){0.0F, 0.0F});
      work->fade_mask->sprite.col.r = 128;
      work->fade_mask->sprite.col.g = 128;
      work->fade_mask->sprite.col.b = 128;
      work->fade_mask->sprite.col.a = 128;
      SPR_SHOW(work->fade_mask);
      work->finish_cnt = FADE_TIME;
}

static int fade_out_wait(Work * work)
{
  work->fade_param -= ((128 + (FADE_TIME - 1)) / FADE_TIME);
  if(work->fade_param < 0) work->fade_param = 0;
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
  return (work->fade_param) ? 1 : 0;
}

static int fade_in_wait(Work * work)
{
  work->fade_param += ((128 + (FADE_TIME - 1)) / FADE_TIME);
  if(work->fade_param >= 128) work->fade_param = 128;
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
  return (work->fade_param < 128) ? 1 : 0;
}



static void face_erase(Work * work)
{
  int i;

  for(i = 0; i < 2; i++)
    if(NULL != work->face_work[i])
      {
	DBG("Destroy Face [%d]\n", i);
	GV_DestroyActor(work->face_work[i]);
	work->face_work[i] = NULL;
	DBG("Destroy FACE-%d\n", i);
      }
}

static void erase_face_wait(Work * work)
{
  int i, ret;

  for(i = 0; i < 2; i++)
    if(work->face_work[i] != NULL)
      {
	ret = codecFaceIsClosed(work->face_work[i]);
	DBG("codecFaceIsClosed(work->face_work[%d]) = %d\n", i, ret);
	if(ret)
	  {
	    GV_DestroyActor(work->face_work[i]);
	    work->face_work[i] = NULL;
	    DBG("Destroy FACE-%d\n", i);
	  }
      }

}

/* ---------------------------------------------------------------------- */
/*
	描画関数
*/

static void DrawCodecMode( Work *work )
{
  MENU_Locate( 256, 48, MENU_MODE_CENTER );
  MENU_Printf( "%d", work->freq );
#if 0
  MENU_Locate( 256, 48, MENU_MODE_CENTER );
  MENU_Printf( "%d", work->freq );
  
  CODEC_DrawMesgPanel( &work->panel, 256 - ( MESG_PANEL_W * 512 / 640 ) / 2, 120 );
#endif
#if 0
  if( work->draw.call ){
    if( DG_TickCount % 60 > 20 ){
      MENU_Locate( 256, 80, MENU_MODE_CENTER );
      MENU_Printf( "CALL" );
    }
  }
#endif
}

/* 描画設定関数 */

static void SetDrawCall( Work *work, int visible )
{
  static int cnt = 0;

  /* 呼出中は、"PTT" の文字を点滅させる */
  /* work->draw.call = visible; */
  
  if(visible)
    codecSetLight(CODEC_LIGHT_PTT);
  else
    codecSetLight(0);

  cnt ++;
  cnt &= 0x0f;
}



/* ---------------------------------------------------------------------- */
/*
	下請け関数
*/

/* 周波数セレクト動作 */

enum {
	SELECTING,
	SELECT_SEND,
	SELECT_CANCEL,
};

static void InitCodecSelect( Work *work )
{
	struct selectinf *si;
	si = &( work->inf.select );
 	si->repeat_count = 0;
	si->prev_dir = 0;
	codecSetLight(0);
}

/*
 * メモリー選択コール時の処理
 */
static void CodecStartWait( Work *work );
static void CodecMode( Work *work );

static void MemoryCallMode(Work * work)
{
  enum {
    STATE_INIT,
    STATE_OPEN_WAIT,
    STATE_SELECT,
    STATE_CLOSE_WAIT
  };

  switch(work->status)
    {
    case STATE_INIT:                       /* メモリーコールモード初期化 */
      codecSetLight(CODEC_LIGHT_MEMORY);   /* "MEMORY" 点灯              */
      CODEC_HideMesgPanel( &work->panel ); /* メッセージパネルを隠す     */
      MemcallOpen();                       /* リストを開く               */
      GM_SdSet(CODEC_MEM_WNDOPEN);         /* 開くときの SE              */
      work->status = STATE_OPEN_WAIT;
      break;
    case STATE_OPEN_WAIT:  /* 選択リストが開くまで待つ   */
      if(MemcallStatus() == 1)
	{
	  int f;
	  if((f = MemcallFirstFreq()) < 0) f = work->freq;
	  work->freq_tmp = f;
	  work->status = STATE_SELECT;
	}
      break;
    case STATE_SELECT:     /* 選択もしくはキャンセル等の処理 */
      if(work->pad->press & (PAD_U | PAD_D | PAD_R | PAD_L))
	{
	  int f;

	  f = MemcallSelect(work->pad);
	  if(f >= 0)
	    {
	      GM_SdSet(CODEC_SE_SEL_PERSON);
	      work->freq_tmp = f;
	    }
	}
      if(work->pad->press & PAD_OK)
	{
	  work->status = STATE_CLOSE_WAIT;
	  work->memcall_status = 1;
	  work->freq = work->freq_tmp;
	  codecSetFreq(work->freq);
	  MemcallClose();
	}
      if(work->pad->press & PAD_CANCEL)
	{
	  work->status = STATE_CLOSE_WAIT;
	  work->memcall_status = 0;
	  MemcallClose();
	  GM_SdSet(CODEC_MEM_WNDCLOSE);
	}
      break;
    case STATE_CLOSE_WAIT:       /* リストを閉じた後始末、
			      通常選択もしくは通話への移行   */
      if(MemcallStatus() == 0)
	{
	  codecSetLight(0);   /* "MEMORY" 点灯              */
	  CODEC_ShowMesgPanel( &work->panel );
	  SetFunc(work, CodecMode);  /* 通常の無線選択に戻る */
	  if(work->memcall_status)  /* 呼出の選択があった場合 */
	    {
	      work->status = 3;
	      GM_SdSet(CODEC_SE_SEND);
	      work->call_cnt = 0;
	    }
	}
      break;
    }
}


static int DoCodecSelect( Work *work )
{
  GV_PAD *pad = work->pad;
  struct selectinf *si;
  int stat = 0;
  

  si = &( work->inf.select );

  if(pad->press & PAD_D)
    {
      SetFunc(work, MemoryCallMode);
      return SELECTING;
    }
  
  if( pad->press & ( PAD_B | PAD_SEL ) ){
    codecSetLight(0);
    return SELECT_CANCEL;
  } else if( pad->press & ( PAD_A | PAD_U ) ){
    codecSetLight(0);
    return SELECT_SEND;
  } else if( pad->status & ( PAD_L | PAD_R ) ){
    int dir = 0;
    
    stat |= CODEC_LIGHT_TUNING;
    if( pad->status & PAD_L ){
      dir = -1;
      stat |= CODEC_LIGHT_LEFT;
    } else {
      dir = 1;
      stat |= CODEC_LIGHT_RIGHT;
    }
    if( si->prev_dir == dir ){
      if( si->repeat_count < SELECT_FIRST_REPEAT ){
	;
      } else if( dir != 0 ){
	GM_SdSet(CODEC_SE_TUNING);
	work->freq += dir;
      }
      si->repeat_count ++;
    } else {
      GM_SdSet(CODEC_SE_TUNING);
      work->freq += dir;
      si->prev_dir = dir;
      si->repeat_count = 0;
    }
    if( work->freq > FREQ_MAX ){
      work->freq = FREQ_MIN;
    } else if( work->freq < FREQ_MIN ){
      work->freq = FREQ_MAX;
    }
  } else {
    si->prev_dir = 0;
    si->repeat_count = 0;
  }
  codecSetLight(stat);
  
  return SELECTING;
}

/* 無線機データロード開始 */

static void CodecLoad( Work *work, int code )
{
  int size;
  int start;
  void *buffer;
  
  DBG("CodecLoad()  work = %p\n", work);
  last_freq = work->freq;
  
  size = ( ( ( ( u_int )code & 0xFF000000 ) >> 24 ) + 1 ) * FS_SECTOR_SIZE;
  start = ( ( code & 0x00FFFFFF ) << 4 ) / FS_SECTOR_SIZE;
  
  DBG("size = %d\n");
  buffer = codecMalloc( size );		// NEED TO RETOUCH !!!
  ASSERT( buffer != NULL );
  
  work->param.data_buffer = buffer;
  work->param.data = buffer + ( ( code & 0x00FFFFFF ) << 4 ) % FS_SECTOR_SIZE;
  
  FS_LoadFileRequest( FS_FILE_CODEC, start, size, buffer );
}

/* ロード終了チェック */

static int IsCodecLoadEnd( Work *work )
{
  return ( FS_LoadFileSync() > 0 ) ? 0 : 1;
}

/* 解釈タスク起動 */

static void CodecScriptSetup( Work *work )
{
  void *stack;
  stack = codecMalloc( ITP_STACK_SIZE );		// NEED TO RETOUCH !!
  
  work->param.stack = stack;
  work->param.stacksize = ITP_STACK_SIZE;
  
  CODEC_ItpInit( &work->param );
}

/* 解釈タスク終了待ち */

static int IsAliveItp( Work *work )
{
  return ReferThreadStatus( work->param.id, NULL );
}

/* 解釈タスクスタック,読み込んだデータの解放 */

static void CodecScriptEnd( Work *work )
{
  DBG("CodecScriptEnd()\n");
  DBG("[1]\n");
  codecFree( work->param.stack );       // NEED TO RETOUCH !!
  DBG("[2]\n");
  codecFree( work->param.data_buffer ); // NEED TO RETOUCH !!
  DBG("[3]\n");
}

/* "CALL"シグナル描画 */
static void DrawCall( Work *work, int tick )
{
  float height, y;

  if( CALL_BLINK( tick ) )
    {
      /* 表示 */
      work->call_sprite->ex_hd.tex.v = work->call_tex_v;
      work->call_sprite->ex_hd.tex.h = work->call_tex_h;

      work->call_over->ex_hd.tex.v = work->call_tex_v;
      work->call_over->ex_hd.tex.h = work->call_tex_h;
      height = CALL_HEIGHT;
      y = CALL_POS_Y;
    }
  else
    {
      /* 消去 */
      work->call_sprite->ex_hd.tex.h = work->call_tex_h * CALL_BLNK / CALL_HEIGHT;
      work->call_over->ex_hd.tex.h = work->call_tex_h * CALL_BLNK / CALL_HEIGHT;
      height = CALL_BLNK;
      y = CALL_POS_Y;
    }
  {
    SPR_POS pos;
    pos.x = CALL_POS_X;
    pos.y = y;

    SPR_SetPosSprite(work->call_sprite, &pos);
    SPR_SetSizeSprite(work->call_sprite, CALL_WIDTH, height);

    SPR_SetPosSprite(work->call_over, &pos);
    SPR_SetSizeSprite(work->call_over, CALL_WIDTH, height);
  }
  SPR_SHOW(work->call_sprite);
  SPR_SHOW(work->call_over);
}

/* 画面フェードアウトセット */

static void FadeOutSet( Work *work, int count )
{
  work->count = count;
  if(NULL != (bg_work = NewCodecIntoDemo(CODEC_DEMO_IN, work->fade_mode)))
    GV_SetActorChild(work, bg_work);
  DG_UnDrawFrameCount = 0;  /* 必ずリセットする */
  /* NewCodecFadeInOut(1, 0, 0, 0, 1, 0, FADE_TIME, -1, NULL); */
}

/* 画面フェードインセット */

static void FadeInSet( Work *work, int count )
{
  work->count = count;
  /* NewCodecFadeInOut(1, 0, 0, 0, 0, 0, FADE_TIME, -1, NULL); */
  CodecBG_Recover(bg_work, 0);
}

static int IsFadeEnd( Work *work )
{
	if( work->count > 0 ){
		work->count --;
		return 0;
	}
	return 1;
}

/* 無線機モード初期化 */

static void CodecInit( Work *work )
{

  DBG("entering CodecInit()...\n");


 /* 全ての通常ゲームプロセスにポーズがかかる */
#ifndef _CODEC_WITHOUT_PAUSE_
  GV_PauseOnActorSystem( GV_PAUSE_STOP );
#endif /* _CODEC_WITHOUT_PAUSE_ */


#ifdef _DEBUG_
  /* 使用メモリ量カウントのため、カウンタをリセットする */
  codecMemCountReset();
#endif /* _DEBUG_ */
  DBG("CodecInit() exit.\n");
}

static void CodecModeInit( Work *work )
{
  int i;

  DBG("CodecModeInit()\n");
  /* DG_Chanl( DG_CHANL_MAIN )->flag = 0; */
  /* 描画実験用。実際にはチャネル4 は立ててはいけない。
     立てると、描画順の都合上、他のチャネルの画面が表示されなくなる。 */
  /* DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 1; */
  DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 0;  /* 明示的に */

  /* 無線モードの間だけ、チャネル 4 をハイレゾリューション化する */
  DG_Chanl( DG_CHANL_MENU)->high_reso = 1;
}

static void CodecPartsInit(Work * work)
{
  codecSetupScreen();   /* 顔表示画面の初期化 */
  face_erase(work);
  
  CODEC_OpenMesgPanel( &work->panel, MESG_PANEL_W, MESG_PANEL_H );

  /* 新版では、DMAPACK を使用して毎フレームテクスチャとなる
     ビットマップイメージと CLUT, 描画プリミティブを転送する */
  {
    int width, height;

    width  = DG_Chanls[4].width;
    height = DG_Chanls[4].height;

    CODEC_DrawMesgPanel( &work->panel,
			 width / 2 - (MESG_PANEL_W * width / 640) / 2,
			 120 * height / 224 );

  }

  CODEC_DrawMessage( &work->panel, "" );

}

/* 無線機モード終了 */
static void CodecPartsEnd(Work * work)
{
  codecRecoveryScreen();
  face_erase(work);
  /* CDC_ReleaseLogicalDirectory(); */
  if(work->layout_work != NULL)
    {
      GV_DestroyActor(work->layout_work);
      work->layout_work = NULL;
    }
  CODEC_CloseMesgPanel( &work->panel );
  work->type = 0;


}

static void CodecModeEnd( Work *work )
{
  int i;
  /*
  DG_Chanl( DG_CHANL_MAIN  )->flag = 1;
  */
  DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 0;

  DG_Chanl( DG_CHANL_MENU )->high_reso = 0;

  DG_SetPrivilegeMode(0);
  GM_ResetMenuStatus(MENU_WEAPON_OFF |
		     MENU_ITEM_OFF   |
		     MENU_RADAR_OFF  |
		     MENU_GAGE_OFF);


#ifdef _DEBUG_
  {
    long size;

    size = codecMemGetSize();
    DBG("codec mem size = %d(%x)\n", size, size);
  }
#endif /* _DEBUG_ */
}

static void CodecEnd( Work *work )
{
  DBG("CodecEnd()\n");
  GM_LastCodecFreq = last_freq;
  DBG("SAVE: GM_LastCodecFreq = %d\n", GM_LastCodecFreq);
  codecLayoutDisp(0);
#ifndef _CODEC_WITHOUT_PAUSE_
  GV_PauseOffActorSystem( GV_PAUSE_STOP );
#endif /* _CODEC_WITHOUT_PAUSE_ */
}

/* 解釈タスクを起こす */

static void WakeupItp( Work *work, int res )
{
	work->param.result = res;

	// 起こす前にリクエストをクリア
	work->param.request = CODEC_REQ_NOREQ;
	WakeupThread( work->param.id );
}

/* ---------------------------------------------------------------------- */
/*
	無線機モードデーモン
*/

#define IsStartFade(work) ((work)->fade_mode)
#define IsStartOpenAction(work)  ((work)->act_open)
#define IsEndFade(work)  ((work)->fade_out)
#define IsEndCloseAction(work)  ((work)->act_close)

static void CodecEndWait( Work *work )
{
  /* 終了 */
  enum {
    STATE_INIT,           /* 初期化                             */
    STATE_CLOSE_FACE,     /* 顔閉じ                             */
    STATE_CLOSE_FACE_AND_FRAME_WAIT,/* 顔閉じ待ち               */
    STATE_CLOSE_FACE_AND_FRAME_WAIT_2,/* 顔閉じ待ち               */
    STATE_FADEOUT_WAIT,   /* フェードアウト終了待ち             */
    STATE_ERASE_FACE,     /* 顔消去                             */
    STATE_ERASE_FACE2,     /* 顔消去                             */
    STATE_BG_RECOV_START, /* 背景復帰処理開始指示エントリ       */
    STATE_BG_RECOV,       /* 背景サイズ復帰                     */
    STATE_BG_RECOV_END,   /* 背景サイズ復帰終了                 */
    STATE_ERASE_FRAME,    /* 枠、背景消去、ゲーム画面再表示開始 */
    STATE_MEM_RECOV,
    STATE_MEM_RECOV_NEXT,
    STATE_FADEIN_WAIT,    /* フェードイン終了待ち               */
    STATE_EXEC_PROC       /* 無線処理終了、終了プロセス実行     */
  };

  switch(work->status)
    {
    case STATE_INIT:           /* 初期化     */
      DBG("CodecEndWait(): STATE_INIT\n");
      codecDispArrow(0);                 /* 送り矢印消去 */
      last_responsed_freq = last_freq;   /* 最終応答周波数を更新 */

      if(work->type == TYPE_NORMAL && work->finish_id)
	{
	  work->status = STATE_BG_RECOV_START;
	  work->finish_id = 0;
	  break;
	}
      if(IsEndCloseAction(work))
	work->status = STATE_CLOSE_FACE;
      else
	if(IsEndFade(work))
	  {
	    work->status = STATE_FADEOUT_WAIT;
	    setup_fade_mask(work); /* フェードアウト指定 */
	  }
	else
	  work->status = STATE_ERASE_FACE;
      break;
    case STATE_CLOSE_FACE:     /* 顔閉じ待ち */
      DBG("CodecEndWait(): STATE_CLOSE_FACE\n");
      work->status = STATE_CLOSE_FACE_AND_FRAME_WAIT;
      DBG("<1>\n");
      codecCloseFace(work->face_work[0]);
      DBG("<2>\n");
      codecCloseFace(work->face_work[1]);
      DBG("<3>\n");
      codecCloseFrame();     /* 枠閉じを同時に発行 */
      DBG("<4>\n");
      GM_SdSet(CODEC_SE_WNDCLOSE);
      DBG("<5>\n");

      break;
      
    case STATE_CLOSE_FACE_AND_FRAME_WAIT:
      DBG("CodecEndWait(): STATE_CLOSE_FACE_AND_FRAME_WAIT\n");
      erase_face_wait(work);
      /*
	work->status = STATE_CLOSE_FACE_AND_FRAME_WAIT_2;
	break;
	case STATE_CLOSE_FACE_AND_FRAME_WAIT_2:
	DBG("CodecEndWait(): STATE_CLOSE_FACE_AND_FRAME_WAIT_2\n");
      */
      {	
	int ret = codecActionStatus();
	ASSERT(ret >= 0);
	printf("ret = %d  face_work[0] = %p  face_work[1] = %p\n", ret, work->face_work[0], work->face_work[1]);
	if((work->face_work[0] != NULL) ||
	   (work->face_work[1] != NULL) || ret) break;
      }

      DBG("<1>\n");
      if(!IsEndFade(work))
	{
	  work->status = STATE_ERASE_FACE;
	  break;
	}
      work->status = STATE_FADEOUT_WAIT;
      DBG("<2>\n");
      
      /* フェードアウト指定 */
      setup_fade_mask(work);
      DBG("<3>\n");

      break;
    case STATE_FADEOUT_WAIT:   /* フェードアウト終了待ち */
      DBG("CodecEndWait(): STATE_FADEOUT_WAIT\n");
      if(fade_out_wait(work)) break;
      work->status = STATE_ERASE_FACE;

      /* 同フレームに顔消去フェーズに入るため、break が無いのは bug ではない */

    case STATE_ERASE_FACE:     /* 顔消去 */
      DBG("CodecEndWait(): STATE_ERASE_FACE\n");
      face_erase(work);        /* 顔 Act の破棄 */
      work->status = STATE_ERASE_FACE2;  /* フレーム待ち */
      break;
    case STATE_ERASE_FACE2:     /* 顔消去 */
      CDC_ReleaseLogicalDirectory();   /* ロードした顔データを破棄 */
      /* 通常無線ならば、ここまでの工程を終了した段階で
	 再度周波数指定フェーズに復帰 */
      if(work->type == TYPE_NORMAL && !work->finish_id)
	{
	  SetFunc(work, CodecMode);
	  break;
	}
      
      /* フェードありならば、背景サイズ復帰フェーズをスキップし、
	 枠消去フェーズに移行 */
      if(IsEndFade(work))
	{
	  CODEC_CloseMesgPanel( &work->panel );
	  work->status = STATE_ERASE_FRAME;
	  break;
	}
      if(!IsEndCloseAction(work)) codecLayoutDisp(0);


    case STATE_BG_RECOV_START:
      DBG("CodecEndWait(): STATE_BG_RECOV_START\n");
      CODEC_CloseMesgPanel( &work->panel );
      work->status = STATE_BG_RECOV;
      CodecBG_Recover(bg_work, 0);
      /* break が無いのは bug ではない。
	 (設定を行ったのと同フレーム中に以降の処理に入るため) */

    case STATE_BG_RECOV:      /* 背景サイズ復帰 */
      DBG("CodecEndWait(): STATE_BG_RECOV\n");
      if(!IsBGrecovered(bg_work)) break;
      work->status = STATE_BG_RECOV_END;
      /* break が無いのは bug ではない。
	 (終了を検知したのと同フレーム中に、以降の処理に入るため) */

    case STATE_BG_RECOV_END:
      work->status = STATE_ERASE_FRAME;
      codecLayoutDisp(0);
      break;
    case STATE_ERASE_FRAME:
      DBG("CodecEndWait(): STATE_ERASE_FRAME\n");
      codecLayoutDisp(0);

      /* 字幕エリアを消去 */
      // CODEC_CloseMesgPanel( &work->panel );
      /* 枠、背景の Act を殺す */
      /* 背景、枠は子として登録されている Act */
      GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);
      /* シグナルを受け取れるよう 1 フレーム待つ */
      work->status = STATE_MEM_RECOV;
      break;
    case STATE_MEM_RECOV:
      CodecModeEnd(work);

      codecMemRecov();  /* 画面の再表示を指示する前に、
			   パケット領域を復帰させておく */
      work->status = STATE_MEM_RECOV_NEXT;
      break;
    case STATE_MEM_RECOV_NEXT:

      codecRecoveryScreen();
      // SPR_LoadTexture(RADAR_TEX);  /* レーダ等用のテクスチャを再指定 */
      if(!IsEndFade(work))
	{
	  work->status = STATE_EXEC_PROC;
	  break;
	}
      work->status = STATE_FADEIN_WAIT;
      
      /* フェードイン指定 */
      work->fade_param = 0;

    case STATE_FADEIN_WAIT:
      DBG("CodecEndWait(): STATE_FADEIN_WAIT\n");
      if(!work->fadeout_break)
	if(fade_in_wait(work)) break;  /* フェードイン待ち */
      SPR_Destroy_2D_Object(work->fade_mask);
      work->fade_mask = NULL;
      work->status = STATE_EXEC_PROC;
      /* 同フレームに終了フェーズに入るため、break が無いのは bug ではない */
    case STATE_EXEC_PROC:
      DBG("CodecEndWait(): STATE_EXEC_PROC\n");
      CodecEnd(work);
      DBG("*************************\n");
      work->type = 0;
      SetFunc(work, CodecStartWait);
      GM_ResetMenuStatus(MENU_RADIO_ON);
      if(work->proc_id != 0)
	{
	  GCL_ExecProc(work->proc_id, NULL);
	  work->proc_id = 0;
	}
      DBG("CodecEndWait(): finished\n");
      break;
    }
}

      

static void CodecDoScript( Work *work )
{
  /* 無線スクリプトを別タスクで解釈し、その要求から画面を作成する */
  enum {
    STATE_INIT,
    STATE_REQ_WAIT,
    STATE_SKIP_SCRIPT,
    STATE_KEY_WAIT,
    STATE_VOX_PLAYING,
    STATE_FACE_LOADING,
    STATE_FACE_SET,
    STATE_HAIR_SET,
    STATE_DEF_MTN_SET,
    STATE_CALL_WAIT,
    STATE_FRAME_WAIT,
    STATE_DISP_CTRL,
    STATE_FADE_WAIT,
    STATE_FINISH
  };
  DrawCodecMode( work );
  switch( work->status )
    {
    case STATE_INIT:
      DBG("CodecDoScropt(): STATE_INIT\n");
      if( work->param.request != CODEC_REQ_SETUP )
	{
	  /* 心の声ドライバ起動 */
	  GV_SetActorChild(work, NewCodecMindVoice());
	  work->call_block = 1;  /* 心の声ドライバ動作中は、
				    呼出/応答不可とする */
	  
	  work->vox_playing = 0;
	  work->voice_cancel = 0;
	  work->pre_cap_status = 0;
	  work->status = STATE_REQ_WAIT;
	  codecDispArrow(0);
	}
      break;
    case STATE_REQ_WAIT:
      DBG("CodecDoScropt(): STATE_REQ_WAIT\n");
      switch( work->param.request )
	{
	case CODEC_REQ_MESG_WAIT:
	  DBG("\tCODEC_REQ_MESG_WAIT\n");
	  CODEC_DrawMessage( &work->panel, ( char * )work->param.req[ 0 ] );
	  work->status = (work->vox_playing)
	    ? STATE_VOX_PLAYING : STATE_KEY_WAIT;
	  if(work->status == STATE_KEY_WAIT) CDC_KillMindVoice();

	  break;
	case CODEC_REQ_END:
	  /* 終了 */
	  DBG("receive: CODEC_REQ_END\n");
	  if( !IsAliveItp( work ) ){
	    last_responsed_freq = last_freq;   /* 最終応答周波数を更新 */
	    CodecScriptEnd( work );
	    work->status = STATE_FINISH;
	    work->finish_cnt = 0;
	  }
	  break;
	case CODEC_REQ_VOX_START:

	  DBG("\tCODEC_REQ_VOX_START\n");
	  /* 既に音声がキャンセル済であれば、新たな再生は行わない */
	  if(work->voice_cancel)
	    {
	      work->status = STATE_SKIP_SCRIPT;
	      break;
	    }
	  DBG("Vox Start!!\n");
	  work->main_vox_handle =
	    GM_VoxStream(work->param.req[0], GM_STREAM_CHANNEL_0);
	  DBG("Executed GM_VoxStream(): handle = %d\n", work->main_vox_handle);
	  work->status = STATE_VOX_PLAYING;
	  work->pre_cap_status = 0;
	  work->vox_playing = 1;
	  codecDispArrow(0);
	  DBG("Vox Started.\n");
	  break;
	case CODEC_REQ_FACE_LOAD:
	  DBG("\tCODEC_REQ_FACE_LOAD\n");
	  {
	    CDC_LoadLogicalDirectory(work->param.req[0]);
	    work->status = STATE_FACE_LOADING;
	  }
	  break;
	case CODEC_REQ_FACE_SET:
	  DBG("\tCODEC_REQ_FACE_SET\n");
	  work->status = STATE_FACE_SET;
	  break;
	case CODEC_REQ_HAIR_SET:   /* 髪の毛のロードと設定(ライデン用) */
	  DBG("\tCODEC_REQ_HAIR_SET\n");
	  work->status = STATE_HAIR_SET;
	  break;
	case CODEC_REQ_DEF_MTN_SET:
	  DBG("\tCODEC_REQ_DEF_MTN_SET\n");
	  work->status = STATE_DEF_MTN_SET;
	  break;
	case CODEC_REQ_DISP_CTRL:
	  DBG("\tCODEC_REQ_DISP_CTRL\n");
	  work->status = STATE_DISP_CTRL;
	  break;
	}
      break;
    case STATE_SKIP_SCRIPT:
      WakeupItp(work, 0);
      work->status = STATE_REQ_WAIT;
      break;

    case STATE_KEY_WAIT:
      codecDispArrow(1);
      if( (work->pad->press & ( PAD_A | PAD_B )) || work->autoskip )
	{
	  WakeupItp( work, 0 );
	  work->status = STATE_REQ_WAIT;
	  CODEC_DrawMessage( &work->panel, NULL );
	}
      if(work->pad->press & (PAD_X | PAD_Y)) work->autoskip ^= 1;
      break;
    case STATE_FACE_LOADING:   /* 顔モデル読み込み中 */
      SetDrawCall(work, 1);
      work->call_cnt += TIMEBASE;
      if(!CDC_SyncCodecDataLoad())
	{
	  CDC_InitLoadData();
	  work->status = STATE_CALL_WAIT;
	}
      break;
    case STATE_FACE_SET:
      {
	void * face_work;
	DBG("side[%d]  EVM-ID[%d]\n", work->param.req[0], work->param.req[1]);
	face_work = NewCodecFace(0,
				 work->dummy_bone,
				 work->param.req[0],
				 work->param.req[1],
				 work->param.req[2]);
	
	/* 顔モデルのワークを登録する */
	SetFaceWork(work->param.req[0], face_work);
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;

    case STATE_HAIR_SET:
      DBG("CodecDoScropt(): STATE_HAIR_SET\n");
      {
	void * model = NULL;
	void * bound = NULL;
	
	DBG("model ID: 0x%08x\n", work->param.req[1]);
	model = CDC_LoadEvmDef(work->param.req[1]);
	bound = (cdcHair_bound < 0)
	  ? NULL : CDC_GetFileEntry(cdcHair_bound, 'k');
	
	codecSetupHair(work->param.req[0], /* side       */
		       model,              /* model      */
		       work->param.req[2], /* sample_num */
		       bound,              /* bound      */
		       cdcHair_hit_param,  /* hit_param  */
		       cdcHair_ambient);   /* ambient    */
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;
    case STATE_DEF_MTN_SET:
      DBG("CodecDoScropt(): STATE_DEF_MTN_SET\n");
      DBG("-a-\n");
      codecSetDefaultMotion(work->param.req[0], work->param.req[1]);
      WakeupItp(work, 0);
      work->status = STATE_REQ_WAIT;
      break;
    case STATE_CALL_WAIT:
      DBG("CodecDoScropt(): STATE_CALL_WAIT\n");
      SetDrawCall(work, 1);
      work->call_cnt += TIMEBASE;
      if(work->call_cnt >= CALL_MIN_WAIT)
	{
	  /* フェードイン開始指定がある、もしくは開きアクション指定がなければ、
	     開きアクション無しで開始する */
	  codecOpenFrame(!work->act_open);
	  codecLayoutDisp(1);   /* まだ表示されていない場合のため表示を促す */
	  if((work->type == TYPE_NORMAL) || work->act_open)
	    GM_SdSet(CODEC_SE_WNDOPEN);
	  work->status = STATE_FRAME_WAIT;
	}
      break;
    case STATE_FRAME_WAIT:     /* 装飾枠アクションの終了待ち */
      DBG("CodecDoScropt(): STATE_FRAME_WAIT\n");
      if(!codecActionStatus())
	{
	  if((work->type == TYPE_NORMAL) ||
	     (!work->fade_mode && work->act_open))
	    GM_SdSet(CODEC_SE_CONNECT);
	  WakeupItp(work, 0);
	  work->status = STATE_REQ_WAIT;
	}
      break;
    case STATE_VOX_PLAYING:
      DBG("CodecDoScropt(): STATE_VOX_PLAYING\n");
      {
	int status;

	status = GM_GetCodecCapStatus();    /* 字幕のタイミング情報を得る */
	/* キャンセルをチェックする */
	if(work->pad->press & (PAD_A|PAD_B|PAD_X|PAD_Y))
	  {
	    DBG("Voice Cancel\n");
	    GM_StreamStop(work->main_vox_handle);   /* 音声を停止する */
	    CDC_KillMindVoice();
	    codecSetMaterAnimDisable(1);
	    work->vox_playing = 0;
	    work->autoskip = 0;
	    work->voice_cancel = 1;
	    if(!status) WakeupItp(work, 0);
	    /* CODEC_DrawMessage(&work->panel, NULL); */
	    work->status = STATE_REQ_WAIT;
	    break;
	  }
	codecDispArrow(0);
	if(status ^ work->pre_cap_status)
	  {
	    DBG("Switch!!\n");
	    CODEC_DrawMessage(&work->panel, NULL);
	    if(status)
	      {
		WakeupItp(work, 0);
		work->status = STATE_REQ_WAIT;
	      }
	  }
	work->pre_cap_status = status;
	if(GM_StreamStatus(work->main_vox_handle) == GM_STREAM_STATE_END)
	  {
	    DBG("Stream END\n");
	    GM_StreamStop(work->main_vox_handle);
	    WakeupItp(work, 0);
	    CODEC_DrawMessage(&work->panel, NULL);
	    work->vox_playing = 0;
	    work->status = STATE_REQ_WAIT;
	  }
      }
      DBG("END: STATE_VOX_PLAYING\n");
      break;

    case STATE_DISP_CTRL:
      DBG("CodecDoScropt(): STATE_DISP_CTRL\n");
      {
	codecFaceDisp(0, work->param.req[0], !work->act_open);
	codecFaceDisp(1, work->param.req[0], !work->act_open);
	/* フェード開始が指定されていたら、
	   アクションの有無にかかわらずこのタイミングで枠を表示 */
	if(work->fade_mode) codecLayoutDisp(1);
	if(!work->fade_mode)
	  {
	    work->status = STATE_REQ_WAIT;
	    WakeupItp(work, 0);
	  }
	else
	  {
	    work->status = STATE_FADE_WAIT;
	    work->fade_param = 0;
	  }
      }
      break;
    case STATE_FADE_WAIT:
      DBG("CodecDoScropt(): STATE_FADE_WAIT\n");
      work->fade_param += ((128 + (FADE_TIME - 1)) / FADE_TIME);
      if(work->fade_param < 128)
	work->fade_mask->head.alpha =
	  SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
      else
	{
	  SPR_Destroy_2D_Object(work->fade_mask);
	  work->fade_mask = NULL;
	  work->status = STATE_REQ_WAIT;
	  WakeupItp(work, 0);
	}  
      break;
    case STATE_FINISH:
      DBG("CodecDoScropt(): STATE_FINISH\n");
      if(work->finish_cnt <= 0)
	{
	  CDC_KillMindVoice();
	  SetFunc(work, CodecEndWait);
	}
      else
	work->finish_cnt -= TIME_BASE;
      break;
    }
}

static void CodecMode( Work *work )
{
  /* 無線モード画面 */
  enum {
    STATE_INIT,
    STATE_SELECT,
    STATE_CALL,
    STATE_LOAD,
    STATE_LOAD_WAIT,
    STATE_LOAD_WAIT2,
    STATE_DO_SCRIPT,
    STATE_NO_RESPONSE,
    STATE_NO_RESPONSE_DISP,
    STATE_NO_RESPONSE_ERASE
  };
  DrawCodecMode( work );		// 無線機の基本画面の描画

  switch( work->status ){
    
  case STATE_INIT:
    DBG("CodecMode(): STATE_INIT\n");
#if 0
    /* フェードイン待ち */
    if( ! IsBGrecovered() ){
      break;
    }
#endif
    /* 顔を非表示に */
    face_erase(work);
    if(codecActionStatus()) break;
    if( work->type == TYPE_NORMAL ){
      work->status = STATE_SELECT;
      InitCodecSelect( work );
    } else {
      /* デモ */
      work->status = STATE_LOAD;
    }
    codecSetFreq(work->freq);  /* 初期状態の表示周波数を設定する */
    /* codecLayoutDisp(1); */
    break;
  case STATE_SELECT:
    /* 周波数選択 */
    {
      int res;
      if(work->call_block) break;
      res = DoCodecSelect( work );
      codecSetFreq(work->freq);
      switch( res ){
      case SELECT_SEND:
	work->status = STATE_LOAD;
	work->call_cnt = 0;
	break;
      case SELECT_CANCEL:
	work->finish_id = 1;
	SetFunc( work, CodecEndWait );
	break;
      }
    }
    break;
  case STATE_LOAD:
    {
      int code;
      
      if( work->type == TYPE_NORMAL ){
	GM_SdSet(CODEC_SE_SEND);
	if( ( code = CODEC_GetCode( work->freq ) ) < 0 ){
	  /* 該当するコードがなかった */
	  work->status = STATE_NO_RESPONSE;
	  work->call_cnt = 0;
	  break;
	}
      } else {
	code = work->code;
	if(work->type == TYPE_DIRECT && work->act_open)
	  GM_SdSet(CODEC_SE_SEND);
      }
      SetDrawCall(work, 1);
      DBG( "code = %x\n", code );
      response_count++; /* 応答回数を追加 */
      CodecLoad( work, code );
      work->status = STATE_LOAD_WAIT;
      DBG("[[[ 1 ]]]\n");
    }
    break;
  case STATE_LOAD_WAIT:
    if( IsCodecLoadEnd( work ) ){
      /* ロード完了、フレームも完全に開いている状態。 */
      work->status = STATE_DO_SCRIPT;
    }
    break;

  case STATE_DO_SCRIPT:
    SetDrawCall( work, 0 );

    CodecScriptSetup( work );
    SetFunc( work, CodecDoScript );
    break;
  case STATE_NO_RESPONSE:
    work->call_cnt += TIMEBASE;
    SetDrawCall( work, 1 );
    if(work->call_cnt >= CALL_MIN_WAIT)
      work->status = STATE_NO_RESPONSE_DISP;
    break;
  case STATE_NO_RESPONSE_DISP:
    {
      /* "応答ありません" もしくは "No Response" などを表示 */
      char *msg;
      
      SetDrawCall( work, 0 );
      msg = GM_GetResource(3, 0);
      CODEC_DrawMessage( &work->panel, msg );
      work->status = STATE_NO_RESPONSE_ERASE;
    }
    break;
  case STATE_NO_RESPONSE_ERASE:
    if( work->pad->press & ( PAD_A | PAD_B | PAD_X | PAD_Y ) ){
      CODEC_DrawMessage( &work->panel, NULL );
      work->status = STATE_SELECT;
    }
    break;
  }
}

static void codec_erase(Work * work)
{
  if(work->type == TYPE_NORMAL)
    work->fade_mode = work->fade_out = 0;
  /* メニューチャネルに描かれている、
     LIFE表示、武器選択、アイテム選択、レーダー等の表示は、
     無線画面中禁止とする。 */
  GM_SetMenuStatus(MENU_WEAPON_OFF |
		   MENU_ITEM_OFF   |
		   MENU_RADAR_OFF  |
		   MENU_GAGE_OFF);

  CodecInit( work );
  DG_SetPrivilegeMode(1);

  /* メニューやLIFE, レーダーが消えるまで、2 フレーム待つ */
  work->finish_cnt = 2;

  /* フェードインモードで無線モードになる場合は、
     フェードインを開始する */
  work->fade_mask = NULL;
  if(work->fade_mode && (work->type != TYPE_NORMAL)) setup_fade_mask(work);

  DBG("codec_erase() finish.\n");
}

static void codec_setup(Work * work)
{
  DBG("CODEC Setup\n");
  last_responsed_freq = -1;  /* 直前の応答周波数を初期化                   */
  response_count = 0;        /* 無線モードに入ってからの総応答回数を初期化 */
  CodecModeInit( work );
  codecMemInit();  /* 無線用メモリ領域を初期化する */ 
  FadeOutSet( work, FADE_TIME );  /* 背景の準備 */
  /* 画面のゲージ等を用意する */
  /* 中央の周波数表示メータ等表示開始 */
  work->layout_work = NewCodecLayout();
  GV_SetActorChild(work, work->layout_work);

  CodecPartsInit(work);
  if(!work->fade_mode && work->act_open) codecLayoutDisp(1);
  codecSetFreq(work->freq);
}

static void CodecStartWait( Work *work )
{
  /* 通常画面で起動をチェックする */
  enum {
    STATE_WAIT,
    STATE_CALL,
    STATE_ERASE,
    STATE_FADE_OUT,
  };

  /* 意味の無い処理(テストコード) */
  /* get_render_point(NULL, NULL, NULL, 0); */


  switch( work->status ){
  case STATE_CALL:
    /* CALL */
    if(GM_CheckMenuStatus(MENU_RADIO_DISABLE)) break;
    if(GV_PauseLevel ) break;

    if(work->type != TYPE_DIRECT) DrawCall( work, work->count );
    work->count -= TIME_BASE;
    if(work->type == TYPE_CALL && work->count < 0 )
      {
	/* キャンセル */
	work->status = STATE_WAIT;
	work->type = 0;
	break;
      }
    if(GM_StreamIsPlay())
      {
	if(work->count < 0) work->count = 0;
	break;
      }

    if( work->count < 0 || work->pad->press & PAD_SEL )
      {
	work->status = STATE_ERASE;
	codec_erase(work);
      }
    /* 待ち状態へ */
    break;

  case STATE_WAIT:
    if(GM_CheckMenuStatus(MENU_RADIO_DISABLE)) break;
    if(work->call_block) break;
    /* 待ち状態 */
    if( !GV_PauseLevel && !GM_StreamIsPlay() &&
	!GM_CheckGameStatus(STATE_PLAY_DEMO) &&
	(work->pad->status & PAD_SEL ) && (!work->type) &&
	!DG_UnDrawFrameCount)
      {
	work->type = TYPE_NORMAL;
	work->finish_id = 0;
	/* 通常コールの出入りタイプは、
	   出入りいずれもフェードなし、アクションありに固定 */
	work->act_open = 1;
	work->act_close = 1;
	work->fade_mode = 0;
	work->fade_out = 0;
      }
    if( work->type != 0 )
      {
	int type = work->type;
	if(type == TYPE_NORMAL)
	  {
	    work->status = STATE_ERASE;
	    codec_erase(work);
	  }
	else
	  {
	    /*
	     * プレイデモ中は、キャンセル可能コールは発生しない
	     */
	    if(GM_CheckGameStatus(STATE_PLAY_DEMO) &&
	       (work->type == TYPE_CALL))
	      break;

	    work->status = STATE_CALL;
	    switch(type)
	      {
	      case TYPE_CALL:   work->count = NORMAL_CALL_TIME; break;
	      case TYPE_DEMO:   work->count = DEMO_CALL_TIME;   break;
	      case TYPE_DIRECT: work->count = 1;                break;
	      }
	    if(type != TYPE_DIRECT) GM_SdSet(CODEC_SE_CALL);
	  }
      }
    break;
  case STATE_ERASE:
    if(!work->finish_cnt)
      {
	work->status = STATE_FADE_OUT;
	codec_setup(work);
      }
    else
      {
	work->finish_cnt--;
	if(work->fade_mode && (work->type != TYPE_NORMAL))
	  {
	    if(work->fadein_break) work->fade_param = 0;
	    fade_out_wait(work);
	  }
      }
    break;
  case STATE_FADE_OUT:
    if( IsBGsetuped( work ) )
      {
	DBG("Point A\n");
	/* FadeInSet( work, FADE_TIME ); */
	GM_SetMenuStatus(MENU_RADIO_ON);  /* 無線ステータスを ON にする */
	SetFunc( work, CodecMode );
printf( "UNDRAW %d\n", DG_UnDrawFrameCount );
        if( DG_UnDrawFrameCount > 0 ){
          DG_UnDrawFrameCount = 0;
        }
printf( "UNDRAW %d\n", DG_UnDrawFrameCount );
      }
    break;
  }
}

static void Act( Work *work )
{
  codecMemCheck();
  SPR_HIDE(work->call_sprite);  /* 基本的には非表示 */
  SPR_HIDE(work->call_over);  /* 基本的には非表示 */
  /*
    work->pad =
    ((work->type == TYPE_CALL) || (work->type == TYPE_DEMO))
    ? &GV_PadDataDirect[ 0 ] : &GV_PadData[ 0 ];
  */
  work->pad = &GV_PadDataDirect[0];

  ( *work->do_func )( work );
}

static void Die(Work * work)
{
  if(NULL != work->call_sprite) SPR_Destroy_2D_Object(work->call_sprite);
  if(NULL != work->call_over) SPR_Destroy_2D_Object(work->call_over);
  if(work->fade_mask != NULL) SPR_Destroy_2D_Object(work->fade_mask);
  if(codec_work == work)
    {
      DBG("Old Codec Daemon is Die.\n");
      codec_work = NULL;
    }
}

static int RecieveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case CDC_SIGNAL_MIND_DIE:
      DBG("mind voice is die\n");
      /* 心の声 Actor が死んだ */
      work->call_block = 0;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }

  return ret;
}

void *NewCodecDaemon( int name, int map )
{
	Work *work;

	DBG("Entering CODEC Mode!!\n");
	CDC_InitLoadingModule();

	DBG("NewCodecDaemon(): CodecDaemon is registed.\n");
	ASSERT(codec_work == NULL);

	if((GM_LastCodecFreq > FREQ_MAX) ||
	   (GM_LastCodecFreq < FREQ_MIN))
	  GM_LastCodecFreq = FREQ_MIN;
	last_freq = GM_LastCodecFreq;
	DBG("LOAD: GM_LastCodecFreq = %d\n", GM_LastCodecFreq);
	work = codecActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xF0 );
	if( work != NULL ){
	  GV_SetActor( work, Act, Die );
	  GV_ActorEX(&work->actor);
	  ASSERT( name == CodecID );
	  work->name = CodecID;
	  work->freq = last_freq;

	  work->face_work[0] = NULL;
	  work->face_work[1] = NULL;

	  work->fade_mask = NULL;

	  work->dummy_bone = DUMMY_BONE;
	  if(GCL_GetOption('p') != NULL) {
	    work->dummy_bone = GCL_GetNextInt();
	  }

	  SetFunc( work, CodecStartWait );
	  /* ストリーミングドライバの登録 */
	  GM_StreamMtDriverInit();
	  GM_StreamCodecCaptionDriverInit();
	  
	  CODEC_ClearTable();
	  
	  // SPR_LoadTexture( RADAR_TEX );
	  /* CALL 表示用 2D オブジェクトを初期化する */
	  work->call_sprite = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);

	  /* 表示サイズの指定 */
	  SPR_SetSizeSprite(work->call_sprite, CALL_WIDTH, CALL_HEIGHT);
	  
	  /* 表示位置の設定 */
	  SPR_SetPosSprite(work->call_sprite,
			   &(SPR_POS){(float)CALL_POS_X, (float)CALL_POS_Y});
	  /* テクスチャ(全体)の設定 */
	  SPR_ObjSetTexture(work->call_sprite, CALL_ICON, 0);

	  /* テクスチャの UV アニメーションのため、
	     UV 値およびテクスチャのサイズなどを別口に保存する */
	  work->call_tex_u = work->call_sprite->ex_hd.tex.u;
	  work->call_tex_v = work->call_sprite->ex_hd.tex.v;
	  work->call_tex_w = work->call_sprite->ex_hd.tex.w;
	  work->call_tex_h = work->call_sprite->ex_hd.tex.h;
	  
	  /* Alpha を設定 */
	  work->call_sprite->head.alpha = SCE_GS_SET_ALPHA(0, 2, 0, 1, 0);
	  work->call_sprite->head.flags |= SPR_FLAG_ALPHA;
	  work->call_sprite->ex_hd.tex.pabe = 0;
	  
	  work->call_sprite->sprite.col.r = 128;
	  work->call_sprite->sprite.col.g = 128;
	  work->call_sprite->sprite.col.b = 128;
	  work->call_sprite->sprite.col.a = 128;

#if 1
	  work->call_over   = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	  SPR_SetSizeSprite(work->call_over, CALL_WIDTH, CALL_HEIGHT);
	  SPR_SetPosSprite(work->call_over,
			   &(SPR_POS){(float)CALL_POS_X, (float)CALL_POS_Y});
	  SPR_ObjSetTexture(work->call_over, CALL_ICON, 0);

	  /* Alpha を設定 */
	  work->call_over->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
	  work->call_over->head.flags |= SPR_FLAG_ALPHA;
	  work->call_over->ex_hd.tex.pabe = 0;
	  
	  work->call_over->sprite.col.r = 128;
	  work->call_over->sprite.col.g = 128;
	  work->call_over->sprite.col.b = 128;
	  work->call_over->sprite.col.a = 64;
#endif
	  
	  /* オブジェクトは非表示状態にしておく */
	  SPR_HIDE(work->call_sprite);
	  SPR_HIDE(work->call_over);

	  /* シグナル受諾関数を指定する */
	  GV_SetActorSignalFunc(work, RecieveSignal);
	}
	codec_work = work;

	return work;
}

/* ---------------------------------------------------------------------- */
/*
	コール起動
*/

void *CODEC_Call( int type, int freq, int code, int procid )
{
	Work *work = codec_work;

	work->type = type;
	work->freq = freq;
	work->code = code;
	work->proc_id = procid;

	return NULL;
}

/* ---------------------------------------------------------------------- */
/*
	シナリオとのインターフェース
*/

int NewCodec( void )
{
  /*
   * 無線機モード起動関数
   */
  ASSERT(codec_work != NULL);
  
  /* 開始 / 終了時のアクションとして、デフォルト値を設定する */
  codec_work->fade_mode     = 0; /* 通常の無線モード開始 */
  codec_work->fade_out      = 0; /* 通常の無線モード終了 */
  codec_work->act_open      = 1; /* 開きアクションあり   */
  codec_work->act_close     = 1; /* 閉じアクションあり   */
  codec_work->fadein_break  = 0;
  codec_work->fadeout_break = 0;

  /* パラメタ取得 */
  if( GCL_GetOption( 'b' ) != NULL ){
    /* base */
    int freq, code;
    freq = GCL_GetNextInt();
    code = GCL_GetNextInt();
    CODEC_SetBaseCall( freq, code );
  }
  if( GCL_GetOption( 'o' ) != NULL ){
    /* over */
    int ent;
    int freq, code;
    
    ent = GCL_GetNextInt();
    freq = GCL_GetNextInt();
    code = GCL_GetNextInt();
    if( ent == EV_ENTER || ent == 1 ){
      /* in */
    } else if( ent == EV_LEAVE || ent == 0 ){
      /* out */
	  code |= 0x80000000;
    }
    CODEC_SetOverCall( freq, code );
  }
  if( GCL_GetOption( 'c' ) != NULL ){
    /* call */
    int freq, code, type, proc_id;
    // DG_FrameCount = 20;

    freq = GCL_GetNextInt();
    code = GCL_GetNextInt();
    type = GCL_GetNextInt();
	if( GCL_NextStr() != NULL ){
		proc_id = GCL_GetNextInt();
	} else {
		proc_id = 0;
	}

    CODEC_Call( type, freq, code, proc_id );
  }
  if( GCL_GetOption( 'r' ) != NULL ){
    /* reset */
    DBG("!!!!!!!!!!!!! CODEC RESET !!!!!!!!!!!!!!!!!\n");
    codec_work->type = 0;
    SetFunc( codec_work, CodecStartWait );
  }
  if( GCL_GetOption( 'a' ) != NULL ){
    /* allclear */
	CODEC_ClearTable();
  }
  if( GCL_GetOption( 'f' ) != NULL ) {
    int flag;

    flag = GCL_GetNextInt();
    if(flag & 1) 
      {
	DBG("[CODEC] ditect fade in option\n");
	codec_work->fade_mode = 1;
      }
    if(flag & 2)
      {
	DBG("[CODEC] ditect fade out option\n");
	codec_work->fade_out = 1;
      }
    if(!(flag & 4))
      {
	DBG("[CODEC] ditect disable open action\n");
	codec_work->act_open = 0;
      }
    if(!(flag & 8))
      {
	DBG("[CODEC] ditect disable close action\n");
	codec_work->act_close = 0;
      }
    if(!(flag & 16))
      {
	DBG("[CODEC] ditect enable fadein break\n");
	/* フェードインを、完全にフェードアウトした状態から開始する */
	if(codec_work->fade_mode) codec_work->fadein_break = 1;
      }
    if(!(flag & 32))
      {
	DBG("[CODEC] ditect enable fadeout break\n"); 
	/* フェードアウトを、完全にフェードアウトした状態で終了する */
	if(codec_work->fade_out) codec_work->fadeout_break = 1;
      }
  }
  if( GCL_GetOption( 'd' ) != NULL ) {
    DG_UnDrawFrameCount = DG_UNDRAW_MAX;  /* 指定されている場合のみ。*/
  }

  return 0;
}
  
