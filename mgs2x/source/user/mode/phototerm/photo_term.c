//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  photo_term.c
  タンカー編で、オタコンにデジカメ画像を転送する端末。

  2001/02/06  Y.Kira
  $Id: photo_term.c,v 1.2 2002/12/20 06:00:37 takaki Exp $
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
#endif

#include "gameheader.h"
#include "g_struct.h"
#include "g_define.h"
#include "libdg.h"
#include "font.h"
#include "libfs.h"
#include "bp_threading.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#define _photo_term_c_
#include "photo_config.h"
#include "photo_term.h"
#include "photo_pic.h"
#include "photo_itp.h"
#include "photo_panel.h"
#include "photo_puppet.h"
#include "photo_disp.h"
#include "console.h"
#include "../codec/codecmem.h"
#include "../codec/cdc_load.h"
#include "../codec/codeccap.h"


#include "photo_codes.h"

extern void PHOTO_ShowMesgPanel(PHOTO_MESG_PANEL * panel);
extern void PHOTO_HideMesgPanel(PHOTO_MESG_PANEL * panel);

#define PHOTO_TERM_TEXT  TEXT_photo_term

#ifdef DEBUG
#define DBG printf
#else
#define DBG
#endif  /* DEBUG */

#ifdef PSX2
#define MESG_PANEL_W		(512-64+FONT_SIZE_W)
#endif
#ifdef KP_XBOX
#define MESG_PANEL_W		(512)
#endif
#define MESG_PANEL_H		((FONT_SIZE_H+FONT_SIZE_H/2)*3+FONT_SIZE_H/2)

#define MAX_IDLE_CNT   3


typedef struct Work {
  GV_ACT_EX    actor;

  int          name;     /* chara 名                 */
  int          skin;     /* 使用するレイアウト名     */
  int          handle;   /* レイアウトハンドル       */

  GV_PAD     * pad;

  /*
   * 画面切替え関連
   */
  SPR_OBJ      * fade_mask;       /* フェードアウト用マスク           */
  int            fade_cnt;        /* フェードアウト用カウンタ         */
  int            chanls_flag[4];  /* 各描画チャネルの表示フラグ保存用 */

  int            sel_pict;
  void         * pict_work;
  

  /* Act() 関数制御用 */
  int            step;
  void        (* act_func)(struct Work * work);

  /*
   * 文字表示パネル
   */
  PHOTO_MESG_PANEL   panel;        /* パネル領域                      */

  /* ストりーミング再生ハンドル */
  int               main_vox_handle;

  /*
   * 評価スクリプト解釈部スレッド管理
   */
  PHOTO_ITP_PARAM    param;

  int                code;        /* 転送処理スクリプトコード */
  int                proc_id;     /* 終了 proc_id */

  int                pre_cap_status;  /* 直前の字幕ステータス */

  int                time_wait_cnt;   /* 時間待ちコマンド用カウンタ */

  int                file_entry;      /* ロードデータのエントリ */

  int                idle_cnt;     /* タイトルでの待ちアクションループ回数 */

  int                progress_time;
  int                progress_cnt;
  int                progress_target;
  int                progress_pre;
  int                progress_rate;

  int                progress_se_cnt;

  int                boot_wait;
  int                boot_status;


  int                evoke:1;         /* 起動フラグ             */
  int                cold_start:1;     /* ホットスタートフラグ   */
  
  int                vox_playing:1;   /* 音声再生中フラグ       */
  int                voice_cancel:1;  /* キャンセル操作済フラグ */
  int                autoskip:1;      /* 自動スキップフラグ     */
  int                progress:1;      /* プログレスバー成長フラグ */

  int                puppet_enable:1; /* パペットは起動済 */
  int                boot_finish:1;

#ifdef DEBUG_MODE
  int f_cnt;
#endif

} Work;

static Work * now_work = NULL;


/* ------------------------------------------------------------------------ */

#define FADE_TIME    30   /* フェードアウト時間 (1/300秒) */
#define CHANGE_TIME  100  /* 画像の入れ換え時間           */

static void act_browse(Work * work);
static void act_sending(Work * work);
static void act_waiting(Work * work);


/* ------------------------------------------------------------------------ */

/*
 * 解釈タスク側にスレッドを切替える
 */
static void WakeupItp(Work * work, int res)
{
  work->param.result = res;
  work->param.request = PHOTO_REQ_NOREQ;
  BP_WakeupThread(work->param.id);
}

/*
 * 転送端末スクリプト解釈タスク起動
 */
static void photo_script_setup(Work * work)
{
  void * stack;

  stack = codecMalloc(ITP_STACK_SIZE);
  ASSERT(NULL != stack);

  work->param.stack     = stack;
  work->param.stacksize = ITP_STACK_SIZE;

  PHOTO_ItpInit(&work->param);
}

/* 解釈タスクスタックおよび、データの開放 */
static void photo_script_finish(Work * work)
{
  codecFree( work->param.stack );
#if 0 //BP_PS2
  codecFree( work->param.data_buffer );
#else
  free( work->param.data_buffer );
#endif
}

static void photo_load(Work * work, int code)
{
  int size;
  int start;
  void * buffer;
  int codecTop;
  int codecOffset;

  printf("SCRIPT Code = %x\n", code);

  size = ((((unsigned int)code & 0xff000000) >> 24) + 1) * FS_SECTOR_SIZE * 4;

  start = ((code & 0x00ffffff) << 4) / FS_SECTOR_SIZE;
#if 0 //BP_PS2
  buffer = codecMalloc(size);
  ASSERT(NULL != buffer);
#else
  size *= 2;
  buffer = malloc( size );
#endif
  
  work->param.data_buffer = buffer;
  codecTop = ( ( code & 0x00FFFFFF ) << 4 );
  codecOffset = codecTop % FS_SECTOR_SIZE;
  work->param.data = (void*)((int)buffer + codecOffset );

  /* 判定スクリプトは,無線スクリプトと同じ扱い */
  FS_LoadFileRequest(FS_FILE_CODEC, start, size, buffer, codecTop );
}

/*
 * ロードの終了待ち
 */
static int is_photo_load_end(Work * work)
{
  return (FS_LoadFileSync() > 0) ? 0 : 1;
}

/* ------------------------------------------------------------------------ */

/*
 * Act 処理の切替え
 */
static void SetupAct(Work * work, void (*func)(Work * work))
{
  work->step = 0;
  work->act_func = func;
}

/* ------------------------------------------------------------------------ */


/*
 * 画像転送モードから通常画面への復帰
 */
static void recov_photo_term_mode(Work * work)
{
  PictFinishDisp(work->pict_work);
  
}

/*
 * 画像転送モード初期化
 */
static void init_photo_term_mode(Work * work)
{
  int i;

  /*
   * チャネル 0 ～ チャネル 3 を非表示にする
   */
  for(i = 0; i < 4; i++)
    {
      work->chanls_flag[i] = DG_Chanls[i].flag;
      DG_Chanls[i].flag = 0;
    }

  /*
   * パケット用メモリを別 malloc 空間として割り当てる(無線の処理を流用)
   */
  codecMemInit();

  /*
   * 2D オブジェクト領域を, codecMalloc() で確保するようにする。
   */
  SPR_SetMemoryManager(codecMalloc, codecDelayedFree);
}

/*
 * 閉じ状態の Act()
 */
static void act_finish(Work * work)
{
  enum {
    STEP_INIT,
    STEP_CLOSING,
    STEP_CLOSE,
    STEP_PRE_RECOV,
    STEP_RECOV,
    STEP_FDIN,
    STEP_FINISH
  };
  switch(work->step)
    {
    case STEP_INIT:
      DBG("act_finish.c: STEP_INIT\n");
      // GM_SdSet(SD_A_HA_DISCN);
      GM_SdSet(SNG_FOUTS_S);
      L2D_EvokeAction(work->handle, ACT_closeViewer);
      PHOTO_CloseMesgPanel(&(work->panel));   /* パネルを開放 */ 
      
      photo_script_finish(work);
      work->step = STEP_CLOSING;
      break;
    case STEP_CLOSING:
      DBG("act_finish.c: STEP_CLOSING\n");
#ifdef KP_WINDOWS
      if( DG_CurrentFrameDrawSkip() ){ break ; }
#endif
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;

      /*
       * 画像転送端末終了アクションが終ったら、各リソースを破棄する
       */
      
      /* パペットアニメーションキャラを殺す */
      GV_CallChildSignalFunc(work, PHOTO_SIGNAL_KILL_PUPPET, 0);

      /* 画面リソースを破棄 */
      L2D_ReleaseLayout(work->handle);
      work->handle = -1;
      
      work->step = STEP_CLOSE;
      break;
    case STEP_CLOSE:
//      DBG("act_finish.c: STEP_CLOSE[%d]\n", DG_FrameSkipRemainCount);
      work->step = STEP_PRE_RECOV;
      work->fade_cnt = 0;
      recov_photo_term_mode(work);

      /* ロードされているデータを開放する */
      CDC_ReleaseLogicalDirectory();
      /* 2D オブジェクトマネージャの使用メモリ確保関数をリセットする */
      SPR_ResetMemoryManager();
      break;

    case STEP_PRE_RECOV:
//      DBG("act_finish.c: STEP_PRE_RECOV[%d]\n", DG_FrameSkipRemainCount);
	  {
		  static SPR_POS zero = {0.0F, 0.0F} ;
		  static SPR_COLOR black = SPR_COLOR_CTOR(0, 0, 0, 0);
		  work->fade_cnt = 0;
		  work->fade_mask->sprite.col.a = 128;
		  SPR_SetSizeSprite(work->fade_mask, 512.0F, 384.0F);
		  SPR_SetPosSprite(work->fade_mask, &zero );
		  work->fade_mask->sprite.col = black;
		  work->fade_mask->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
		  SPR_SHOW(work->fade_mask);
		  work->step = STEP_RECOV;
	  }
      break;
    case STEP_RECOV:
//      DBG("act_finish.c: STEP_RECOV[%d]\n", DG_FrameSkipRemainCount);
      if(codecMemRecov()) break;  /* 遅延開放が終るまで待つ */


      /* この時点で,全リソースの開放が終了する */

      {
	int i;

	work->step = STEP_FDIN;

	GV_PauseOffActorSystem( GV_PAUSE_STOP ); /* ポーズを解除 */  
	/* チャネル 0～3 の表示状態を復帰 */
	for(i = 0; i < 4; i++)
	  DG_Chanls[i].flag = work->chanls_flag[i];
	DBG("recov_photo_term_mode(): [4]\n");
      }

    case STEP_FDIN:
//      DBG("act_finish.c: STEP_FDIN\n[%d]\n", DG_FrameSkipRemainCount);
      {
	int alpha;

	work->fade_cnt += TIME_BASE;
	alpha = work->fade_cnt * 128 / FADE_TIME;
	if(alpha > 128)
	  {
	    alpha = 128;
	    SPR_HIDE(work->fade_mask);
	    work->step = STEP_FINISH;
	  }
	work->fade_mask->sprite.col.a = 128 - alpha;
      }
      break;
    case STEP_FINISH:
      DBG("act_finish.c: STEP_FINISH\n");
      SetupAct(work, act_waiting);
      if(work->proc_id != 0)
	{
	  /* 終了 proc を呼び出す */
	  GCL_ExecProc(work->proc_id, NULL);
	  work->proc_id = 0;
	}
      break;
    }
}


/*
 * 閲覧,判定中(判定そのものは撮影時に終っているので,その結果通知のみ)
 *
 * このインタプリタ解釈部は、無線スクリプトのサブセットとなっている。
 * 使用できるのは、メッセージの出力,音声の出力のみ。
 */
static void act_browse(Work * work)
{
  enum {
    STEP_INIT,
    STEP_REQ_WAIT,
    STEP_MESG_DRAWING,
    STEP_SKIP_SCRIPT,
    STEP_KEY_WAIT,
    STEP_VOX_PLAYING,
    STEP_SET_FRAME,
    STEP_GET_FRAME,
    STEP_LOAD_PUPPET,
    STEP_LOADING_PUPPET,
    STEP_SET_DEFAULT_ACTION,
    STEP_SET_PUPPET_ACTION,
    STEP_KEY_WAIT2,
    STEP_TIME_WAIT,
    STEP_TIME_WAITING,
    STEP_PHOTO_SAVE,
    STEP_PHOTO_SOUND,
    STEP_FINISH
  };

#ifdef DEBUG_MODE
  work->f_cnt ++;
#endif /* DEBUG_MODE */

  switch(work->step)
    {
    case STEP_INIT:

      {
	float x, y, w, h;
	SPR_OBJ * obj = L2D_GetObject(work->handle, OBJ_TextArea);

	x = 64;
	y = 320;
	w = MESG_PANEL_W * 512 / 640;
	h = MESG_PANEL_H * 384 / 480;
#ifdef DEBUG_MODE
	work->f_cnt = 0;
#endif /* DEBUG_MODE */
	if(NULL != obj)
	  {
	    x = obj->box.rect.begin.x;
	    y = obj->box.rect.begin.y;
	    w = obj->box.rect.end.x - x;
	    h = obj->box.rect.end.y - y;
	  }

	printf("PANEL: x=%f  y= %f  w = %f  h = %f\n", x, y, w, h);

	/* 文字表示パネルの初期化 */
	PHOTO_OpenMesgPanel(&(work->panel), MESG_PANEL_W, MESG_PANEL_H);
	PHOTO_DrawMesgPanel(&(work->panel), (int)x, (int)y, (int)w, (int)h);
      }
      // PHOTO_DrawMessage(&(work->panel), "");
      DBG("Message-0\n");
      PHOTO_DrawMessage(&(work->panel), NULL);

      work->vox_playing = 0;    /* 音声は停止中                         */
      work->voice_cancel = 0;   /* 音声はまだキャンセル操作されていない */
      work->pre_cap_status = 0; /* 直前字幕のステータスは, 直前字幕なし */

      work->autoskip = 0;


      work->step = STEP_REQ_WAIT; /* リクエスト受付状態に移行 */
      break;
    case STEP_REQ_WAIT:   /* インタプリタからのリクエスト受け取り */
      switch(work->param.request)
	{
	case PHOTO_REQ_MESG_WAIT:  /* テキスト出力   */
	  DBG("PHOTO_REQ_MESG_WAIT:\n");
	  DBG("Message-1\n");
	  PHOTO_HideMesgPanel(&(work->panel));
	  work->step = STEP_MESG_DRAWING;
	  break;
	case PHOTO_REQ_END:        /* スクリプト終了 */
	  DBG("PHOTO_REQ_END:\n");
	  work->step = STEP_FINISH;
	  break;
	case PHOTO_REQ_VOX_START:  /* 音声開始       */
	  {
	    DBG("PHOTO_REQ_VOX_START:\n");
	    if(work->voice_cancel)
	      {
		work->step = STEP_SKIP_SCRIPT;
		break;
	      }
	    /* 直前の音声が終了しているかどうかを確認し,
	       終っていなければ終了するまで処理をブロック */
	    if(GM_CheckMenuStatus(PHOTO_MENU_CH)) break;

	    work->main_vox_handle =
	      GM_VoxStream(work->param.req[0], PHOTO_VOICE_CH);
	    DBG("vox handle = %d\n", work->main_vox_handle);
	    ASSERT(work->main_vox_handle >= 0);
	    work->step = STEP_VOX_PLAYING;
	    work->pre_cap_status = 0;
	    work->vox_playing = 1;
	  }
	  break;

	case PHOTO_REQ_SET_FRAME:  /* コマの設定 */
	  DBG("PHOTO_REQ_SET_FRAME:\n");
	  work->step = STEP_SET_FRAME;
	  break;

	case PHOTO_REQ_GET_FRAME:  /* コマ番号の取得 */
	  DBG("PHOTO_REQ_GET_FRAME:\n");
	  work->step = STEP_GET_FRAME;
	  break;
	case PHOTO_REQ_LOAD_PUPPET:
	  DBG("PHOTO_REQ_SET_PUPPET:\n");
	  work->step = STEP_LOAD_PUPPET;
	  break;
	case PHOTO_REQ_SET_DEFACT:
	  DBG("PHOTO_REQ_SET_DEFACT:\n");
	  work->step = STEP_SET_DEFAULT_ACTION;
	  break;
	case PHOTO_REQ_SET_ACT:
	  DBG("PHOTO_REQ_SET_ACT:\n");
	  work->step = STEP_SET_PUPPET_ACTION;
	  break;
	case PHOTO_REQ_KEY_WAIT:
	  DBG("PHOTO_REQ_KEY_WAIT:\n");
	  work->step = STEP_KEY_WAIT2;
	  break;

	case PHOTO_REQ_TIME_WAIT:
	  DBG("PHOTO_REQ_TIME_WAIT\n");
	  work->step = STEP_TIME_WAIT;
	  break;
	case PHOTO_REQ_PHOTO_SAVE:
	  DBG("PHOTO_REQ_PHOTO_SAVE\n");
	  work->step = STEP_PHOTO_SAVE;
	  break;
	case PHOTO_REQ_SOUND:
	  DBG("PHOTO_REQ_SOUND\n");
	  GM_SdSet(work->param.req[0]);
	  work->step = STEP_PHOTO_SOUND;
	  break;
	}
      break;

    case STEP_SKIP_SCRIPT:   /* スクリプト終了                */
      WakeupItp(work, 0);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_MESG_DRAWING:  /* メッセージ描画 */

      PHOTO_DrawMessage(&(work->panel), BP_GCL_LOOKUP_NEW_FONT_STRING((char *)(work->param.req[0])));
      PHOTO_ShowMesgPanel(&(work->panel));
      work->step = (work->vox_playing)
	? STEP_VOX_PLAYING : STEP_KEY_WAIT;
      break;

    case STEP_KEY_WAIT:      /* ページ送りキー入力待ち        */
      // DBG("STEP_KEY_WAIT:\n");
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->handle))
	L2D_EvokeAction(work->handle, ACT_KeyWait);
      if((work->pad->press & (PAD_A | PAD_B)) || work->autoskip)
	{
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
 	    L2D_BreakAction(work->handle);
	  {
	    SPR_OBJ * dot = L2D_GetObject(work->handle, OBJ_dot);
	    dot->sprite.col.a = 0;
	  }

	  WakeupItp(work, 0);
	  work->step = STEP_REQ_WAIT;
	  DBG("Message-2\n");
	  // PHOTO_DrawMessage(&work->panel, NULL);
	}
      if(work->pad->press & (PAD_X | PAD_Y)) work->autoskip ^= 1;
      
      break;
    case STEP_VOX_PLAYING:   /* 音声再生中,ページ送り信号待ち */
      {
	int status;
	
	status = GM_GetCodecCapStatus();   /* 字幕のタイミング情報を得る */

	/* キャンセルのチェック */
	if((work->pad->press & (PAD_A | PAD_B | PAD_X | PAD_Y)))
	  {
	    GM_StreamStop(work->main_vox_handle);
	    work->vox_playing = 0;
	    work->autoskip = 0;
	    work->voice_cancel = 1;
	    work->main_vox_handle = -1;
	    if(!status) WakeupItp(work, 0);
	    work->step = STEP_REQ_WAIT;
	    break;
	  }

	if(!work->pre_cap_status && status)
	  {
	    DBG("PHOTO CAPTION CHANGE\n");
	    DBG("Message-3\n");
	    PHOTO_DrawMessage(&work->panel, NULL);
	    if(status)
	      {
		WakeupItp(work, 0);
		work->step = STEP_REQ_WAIT;
	      }
	  }
	work->pre_cap_status = status;
	if(GM_StreamStatus(work->main_vox_handle) == GM_STREAM_STATE_END)
	  {
	    DBG("PHOTO: STREAM END\n");
	    GM_StreamStop(work->main_vox_handle);
	    WakeupItp(work, 0);
	    // PHOTO_DrawMessage(&work->panel, NULL);
	    work->vox_playing = 0;
	    work->autoskip = 0;
	    // work->voice_cancel = 0;
	    // if(!status) WakeupItp(work, 0);
	    work->step = STEP_REQ_WAIT;
	  }
      }
      break;

    case STEP_SET_FRAME:      /* 表示画像設定 */
#ifdef DEBUG_MODE
      // if(work->f_cnt <=20) break;
#endif /* DEBUG_MODE */
      work->sel_pict = work->param.req[0];

      /* これまで表示されていた側をフェードアウト消去する */
      if(NULL != work->pict_work)
	GV_CallChildSignalFunc(work, KILL_PICT, -1);

      /* ここから表示する画像のキャラを起動する */
      if(work->param.req[0] >= 0)
	{
	  work->pict_work =
	    NewPhotoTermPicture(work->param.req[0], work->handle);
	  if(NULL != work->pict_work) GV_SetActorChild(work, work->pict_work);
	}
      else
	work->pict_work = NULL;  /* 消去の場合 */
#ifdef DEBUG_MODE
      work->f_cnt = 0;
#endif /* DEBUG_MODE */
      WakeupItp(work, 0);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_GET_FRAME:      /* 表示画像取得 */
      work->param.res[0] = work->sel_pict; /* 現在指定されているコマ番号 */
      WakeupItp(work, PHOTO_ACK);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_LOAD_PUPPET:
      printf("STEP_LOAD_PUPPET\n");
      /* 起動はスクリプト外で行うようになった。
	GV_SetActorChild(work, NewPhotoTermPuppet(work->param.req[0],
	work->param.req[1]));
      */
      work->step = STEP_LOADING_PUPPET;
      break;

    case STEP_LOADING_PUPPET:
      if(PHOTO_PuppetAck()) break;  /* 初期アクションが終了するまで待つ */
      WakeupItp(work, 0);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_SET_DEFAULT_ACTION:
      PHOTO_SetDefaultAction(work->param.req[0],   /* デフォルトアクション */
			     work->param.req[1],   /* 疲れアクション       */
			     work->param.req[2]);  /* 疲れるまでの時間     */

      WakeupItp(work, 0);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_SET_PUPPET_ACTION:
      PHOTO_PuppetAction(work->param.req[0], work->param.req[1]);
      /* SE がある場合は,同時に発行する */
      if(work->param.req[2] >= 0) GM_SdSet(work->param.req[2]);
      WakeupItp(work, 0);
      work->step = STEP_REQ_WAIT;
      break;

    case STEP_KEY_WAIT2:
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->handle))
 	L2D_EvokeAction(work->handle, ACT_KeyWait);

      if((work->pad->press & (PAD_A | PAD_B | PAD_X | PAD_Y)))
	{
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	    L2D_BreakAction(work->handle);
	  {
	    SPR_OBJ * dot = L2D_GetObject(work->handle, OBJ_dot);
	    dot->sprite.col.a = 0;
	  }
	  WakeupItp(work, 0);
	  work->step = STEP_REQ_WAIT;
	}
      break;

    case STEP_TIME_WAIT:
      /* 既に音声がキャンセルされて進行中であれば、
	 時間待ちの処理は無視する */
      if(work->voice_cancel)
	{
	  work->step = STEP_REQ_WAIT;
	  WakeupItp(work, 0);
	  break;
	}
      /* 音声がキャンセルされていない場合は,時間待ちに入る */
      work->time_wait_cnt = work->param.req[0];
      work->step = STEP_TIME_WAITING;
      break;
    case STEP_TIME_WAITING:
      
      /* 時間が経過したら処理を続行する */
      if((work->time_wait_cnt -= TIME_BASE) <= 0)
	{
	  WakeupItp(work, 0);
	  work->step = STEP_REQ_WAIT;
	  break;
	}

      /* 時間がまだ経過していないが,キーでキャンセルされたら処理を続行する */
      if((work->pad->press & (PAD_A | PAD_B | PAD_X | PAD_Y)))
	{
	  work->vox_playing = 0;
	  work->autoskip = 0;
	  work->voice_cancel = 1;
	  WakeupItp(work, 0);
	  work->step = STEP_REQ_WAIT;
	  break;
	}

      break;

    case STEP_PHOTO_SAVE:
      /*
       * 指定された写真をセーブする
       */
      {
	void * pic;

	pic = PIC_GetPicture(work->param.req[0]);
	memcpy(GM_TankerPicture, pic, GM_TANKER_PICTURE_SIZE);
	WakeupItp(work, 0);
	work->step = STEP_REQ_WAIT;
      }
      break;
    case STEP_PHOTO_SOUND:
      /*
       * SEを発生した後の処理
       */
      {
	WakeupItp(work, 0);
	work->step = STEP_REQ_WAIT;
      }
      break;
    case STEP_FINISH:
      GV_CallChildSignalFunc(work, KILL_FORCE_PICT, 0);
      SetupAct(work, act_finish);
      break;
   }
}

/*
 * 判定演出用スクリプトロード中
 */
static void act_loading(Work * work)
{
  enum {
    STEP_INIT,
    STEP_LOAD,
    STEP_EXEC_SCRIPT
  };

  /* 閲覧開始アクション実行と平行して判定スクリプトをロードする */
  switch(work->step)
    {
    case STEP_INIT:   /* レイアウト終了待ち */
      {
	photo_load(work, work->code);
	work->step = STEP_LOAD;
      }
      break;

    case STEP_LOAD:   /* 評価スクリプトロード */
      if(is_photo_load_end(work))
	work->step = STEP_EXEC_SCRIPT;
      break;

    case STEP_EXEC_SCRIPT:  /* ロードが終ったのでスクリプト実行の準備をする */

      /* ロードは終了しているので、アクションの終了を待つ。 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	{
	  DBG("\twaiting layout animation\n");
	  break;
	}

      /* ロード,アクションともに終了すれば閲覧開始 */
      photo_script_setup(work);
      SetupAct(work, act_browse);
      break;
    }
}

static int get_sound(int strcode)
{
  static struct {
    int strcode;
    int sound;
  } snd_list[] = {
    { 0x00de6e5a  /* "SD_A_HA_CMPLT" */, SD_A_HA_CMPLT },
    { 0x00d95fb0  /* "SD_A_HA_SCLWI" */, SD_A_HA_SCLWI },
    { 0x007f8650  /* "SD_A_HA_MOVLI" */, SD_A_HA_MOVLI },
    { 0x00ec7934  /* "SD_A_HA_DISCN" */, SD_A_HA_DISCN },

    { 0x009ca51d  /* "SD_A_OTA_IN_1" */, SD_A_OTA_IN_1 },
    { 0x009fbfbd  /* "SD_A_OTA_OUT1" */, SD_A_OTA_OUT1 },
    { 0x009f991d  /* "SD_A_OTA_OK_1" */, SD_A_OTA_OK_1 },
    { 0x009f091d  /* "SD_A_OTA_NG_1" */, SD_A_OTA_NG_1 },
    { 0x009defdd  /* "SD_A_OTA_LAU1" */, SD_A_OTA_LAU1 },
    { 0x00a18c5d  /* "SD_A_OTA_SHY1" */, SD_A_OTA_SHY1 },
    { 0x00a1ab7d  /* "SD_A_OTA_SPR1" */, SD_A_OTA_SPR1 },
    { -1, -1}
  };

  int i;

  for(i = 0; snd_list[i].strcode > 0; i++)
    if(snd_list[i].strcode == strcode) return snd_list[i].sound;

  printf("Unknown SE by layout key.\n");
  return -1;
}


/*
 * レイアウトから受け取るシグナルのハンドラ
 */
static void MainSignalHandle(void * workp, int signal, int value)
{
  Work * work = workp;

  switch(signal)
    {
    case SIG_progress_start:   /* プログレスバースタート */
      work->progress = 1;    /* プログレスバーの成長を許可する */
      break;
    case SIG_showPuppet:       /* パペット表示開始(本当は puppet なんだけど) */
      GV_SetActorChild(work, NewPhotoTermPuppet(NAME_puppet, ACT_enter));
      break;
    case SIG_hidePuppet:       /* パペット表示終了       */
      if(!work->puppet_enable)
	{
	  work->puppet_enable = 1;
	  GV_CallChildSignalFunc(work, PHOTO_SIGNAL_EXIT_PUPPET, 0);
	}
    case SIG_showPhoto:       /* 画像表示開始 */
      
      break;

    case SIG_SE:              /* SE */
      {
	int snd = get_sound(value);
	if(snd >= 0) GM_SdSet(snd);
      }
      break;
    }
}

static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret;

  switch(signal)
    {
    case CON_SIGNAL_FINISH:   /* テキストのデモンストレーションが終了した */
      ret = 0;
      work->boot_finish = 1;
      work->boot_status = value;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}


static void act_sending(Work * work)
{
   float safeZoneOffsetY = 0;

   enum {
    STEP_TITLE,
    STEP_MAIN,
    STEP_UPLOAD,
    STEP_OPENING,
    STEP_PROGRESS,
    STEP_SENDING,
    STEP_COMPLETE,
    STEP_CLOSING,
    STEP_FINISH,
    STEP_NEXT
  };

  switch(work->step)
    {
    case STEP_TITLE:    /* ホットスタート用 l2d の再生を開始する */
      work->step = STEP_MAIN;
      GM_SdSet(SD_AUTO_FADER2);
      break;

    case STEP_MAIN:
      {
	void * l2d_ptr;

	if(NULL == (l2d_ptr = GV_GetCache(GV_CacheID(NAME_densou_main, 'o'))))
	  l2d_ptr = CDC_GetFileEntry(NAME_densou_main, 'o');
	if(NULL == l2d_ptr)
	  {
	    work->step = STEP_FINISH;
	    break;
	  }

	work->handle = L2D_SetupLayout2(l2d_ptr, 4, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	ASSERT(work->handle >= 0);
	work->puppet_enable = 0;
	L2D_SetSignalHandle(work->handle, work, MainSignalHandle);
	work->progress = 0;
	work->step = STEP_UPLOAD;
	L2D_EvokeAction(work->handle, ACT_DefaultAction);
      }
      break;

    case STEP_UPLOAD:   /* アップロード中プログレスバー表示 */
      L2D_EvokeAction(work->handle, ACT_openUpload);
      
      work->step = STEP_OPENING;
      break;
    case STEP_OPENING:
      if(work->progress)
	{
	  work->step = STEP_PROGRESS;
	  break;
	}
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      /* アクションが終了してしまったら、自動的にプログレスバーの成長 */
      work->step = STEP_PROGRESS;
      break;

    case STEP_PROGRESS: /* プログレスバー成長開始 */
      /* プログレスバーオブジェクトを,0% の状態に設定 */
      {
	void * parts;

	parts = L2D_GetParts(work->handle, OBJ_progress);
	L2D_MorfObject(parts, KEY_progress0, KEY_progress0, 1.0F);
	work->progress_time = (BP_PS2_rand() >> 16) % 100 + 1;
	work->progress_target = (BP_PS2_rand() >> 16) % 15 + 1;
	work->progress_pre  = 0;
	work->progress_rate = 0;
	work->progress_cnt = 0;
	work->step = STEP_SENDING;
	work->progress_se_cnt = 0;
      }
      break;
    case STEP_SENDING:  /* アップロード中プログレスバー成長 */
      {
	work->progress_se_cnt += TIME_BASE;
	if(work->progress_se_cnt >= 30)
	  {
	    GM_SdSet(SD_A_HA_LOAD1);
	    work->progress_se_cnt = 0;
	  }

	work->progress_rate =
	  (work->progress_target - work->progress_pre) *
	  work->progress_cnt / work->progress_time + work->progress_pre;
	
	work->progress_cnt += TIME_BASE;

	if(work->progress_cnt >= work->progress_time)
	  {
	    work->progress_cnt -= work->progress_time;
	    work->progress_pre = work->progress_rate;
	    work->progress_time = (BP_PS2_rand() >> 16) % 100 + 1;
	    work->progress_target = work->progress_pre + (BP_PS2_rand() >> 16) % 15 + 1;
	  }
	if(work->progress_rate >= 100)
	  {
	    work->progress_rate = 100;
	    work->step = STEP_COMPLETE;
	  }
	
	{
	  float rate;
	  void * parts;
	  
	  rate = (float)work->progress_rate / 100.0F;
	  
	  parts = L2D_GetParts(work->handle, OBJ_progress);
	  L2D_MorfObject(parts, KEY_progress0, KEY_progress100, rate);
	}
      }
      break;

    case STEP_COMPLETE: /* 転送完了                         */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      L2D_EvokeAction(work->handle, ACT_closeUpload);
      work->step = STEP_CLOSING;
      // GM_SdSet(SNG_FOUTS_S);
      // GM_SdSet(SD_A_HA_CMPLT);

      break;
    case STEP_CLOSING:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;

      // GM_SdSet(SD_A_HA_SCLWI);
      L2D_EvokeAction(work->handle, ACT_openViewer);

      work->step = STEP_FINISH;
      break;

    case STEP_FINISH:
      DBG("\tSTEP_FINISH\n");
      /*
       * 転送完了のアクションは終了している。
       */ 
      SetupAct(work, act_loading);  /* 閲覧モード用 Act() に切替え */
      break;
    }
}


static void act_title(Work * work)
{
   float safeZoneOffsetY = 0;
  enum {
    STEP_TITLE,
    STEP_OPENTITLE,
    STEP_IDLETITLE,
    STEP_SETTITLE,
    STEP_FIXTITLE,
    STEP_CLOSETITLE,
    STEP_EXEC
  };

  switch(work->step)
    {
    case STEP_TITLE:
      {
	void * l2d_ptr;

	if(NULL == (l2d_ptr = GV_GetCache(GV_CacheID(NAME_densou_title, 'o'))))
	  l2d_ptr = CDC_GetFileEntry(NAME_densou_title, 'o');
	if(NULL == l2d_ptr)
	  {
	    work->step = STEP_EXEC;
	    break;
	  }
	work->handle = L2D_SetupLayout2(l2d_ptr, 4, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	L2D_EvokeAction(work->handle, ACT_DefaultAction);
	work->step = STEP_OPENTITLE;
	break;
      }

    case STEP_OPENTITLE:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      L2D_EvokeAction(work->handle, ACT_openTitle);
      GM_SdSet(SNG_PLAY_06);

      work->step = STEP_IDLETITLE;
      work->idle_cnt = 0;
      break;
    case STEP_IDLETITLE:
      /* 何か押されたら中断 */
      if(work->pad->press & (PAD_OK | PAD_CANCEL))
	{
	  work->step = STEP_SETTITLE;
	  break;
	}
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      if(work->idle_cnt >= MAX_IDLE_CNT)
	{
	  work->step = STEP_CLOSETITLE;
	  break;
	}
      L2D_EvokeAction(work->handle, ACT_idleTitle);
      work->idle_cnt++;
      break;
    case STEP_SETTITLE:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	L2D_BreakAction(work->handle);
      L2D_EvokeAction(work->handle, ACT_setTitle);
      work->step = STEP_FIXTITLE;
      break;
    case STEP_FIXTITLE:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      work->step = STEP_CLOSETITLE;
      break;
    case STEP_CLOSETITLE:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	L2D_BreakAction(work->handle);
      L2D_ReleaseLayout(work->handle);
      work->step = STEP_EXEC;
      break;

    case STEP_EXEC:
      DG_UnDrawFrameCount = 2;
      SetupAct(work, act_sending);
      break;
    }
}

static void act_bootcancel(Work * work)
{
  enum {
    STEP_CANCEL,
    STEP_EXEC
  };
  switch(work->step)
    {
    case STEP_CANCEL:
      L2D_EvokeAction(work->handle, ACT_setTitle);
      GM_SdSet(SNG_PLAY_06);
      work->step = STEP_EXEC;
      break;
    case STEP_EXEC:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      SetupAct(work, act_sending);
      break;
    }
}


/*
 * 起動時の流れ
 */
static void act_booting(Work * work)
{
  enum {
    STEP_BOOT,
    STEP_BOOT2,
    STEP_BOOT3,
    STEP_EXEC,
    STEP_EXEC2
  };
  switch(work->step)
    {
    case STEP_BOOT:

      /* 入る際に BGM フェードアウト */
      // GM_SdSet(SNG_FOUTS_S);

      work->boot_finish = 0;
      GV_SetActorChild(work, NewConsoleDemo(SCRN_densou_log,
					    PHOTO_TERM_TEXT, 0));
      work->step = STEP_BOOT2;
      break;
    case STEP_BOOT2:
      if(!work->boot_finish) break;
      if(!work->boot_status)
	{
	  /* 自然終了 */
	  work->step = STEP_BOOT3;
	  work->boot_wait = 600;
	}
      else
	{
	  /* キャンセル終了 */
	  work->step = STEP_EXEC2;
	}
      break;
    case STEP_BOOT3:
      work->boot_wait -= TIME_BASE;
      if(work->boot_wait < 0) work->step = STEP_EXEC;
      break;
    case STEP_EXEC:
      SetupAct(work, act_title);
      break;
    case STEP_EXEC2:  /* キャンセル終了なので、起動ロゴをスキップする */
      SetupAct(work, act_bootcancel);
      break;
    }
}


static void msg_wait(Work * work)
{
  int      n_msg, i;
  GV_MSG * msg;

  if(!(n_msg = GV_ReceiveMessage(work->name, &msg))) return;

  for(i = 0; i < n_msg; i++, msg++)
    switch(msg->message[0])
      {
      case 0:  /* 端末画面起動 */
	work->evoke = 1;
	break;
      }
}

/*
 * 画像転送端末起動
 */
void AccessPhotoTerm(void)
{
  Work * work = now_work;
  int code, proc_id;

  ASSERT(NULL != work);
  proc_id = 0;

  printf("<1>\n");
  code = 0;
  if(NULL != GCL_GetOption('c'))
    code = GCL_GetNextInt();
  else
    HANGUP();


  work->cold_start = 0;
  if(NULL != GCL_GetOption('C')) work->cold_start = 1;
  printf("<2>\n");
  
  if(NULL != GCL_GetOption('p')) proc_id = GCL_GetNextInt();
  printf("<3>\n");


  work->code = code;
  work->proc_id = proc_id;

  work->evoke = 1;
  DBG("Evoke Photo Term!!\n");
}

static void act_waiting(Work * work)
{
  enum {
    STEP_INIT,
    STEP_WAIT,
    STEP_FADE,
    STEP_LOAD,
    STEP_LOADING,
    STEP_EXEC,
  };

  switch(work->step)
    {
    case STEP_INIT:    /* 初期化 */
      work->step = STEP_WAIT;
      work->fade_mask->sprite.col.a = 0;
      SPR_HIDE(work->fade_mask);
      break;
    case STEP_WAIT:    /* 転送端末起動待機   */
      if(!work->evoke)
	{
	  msg_wait(work); /* メッセージ待ち処理 */
	  break;
	}

      /* ここに処理が移るのは、前のフレームで
	 起動メッセージを受け取っている場合のみ */
      {
	void * tmp;
	if(NULL == (tmp = GV_Malloc(64 * 1024))) break;

	/* 全 Actor にポーズをかける */  
	GV_PauseOnActorSystem( GV_PAUSE_STOP );

	GV_Free(tmp);
	
	work->step = STEP_FADE;  /* フェードアウトに入る */
	work->fade_mask->sprite.col.a = 0;
	work->fade_cnt = 0;
	SPR_SHOW(work->fade_mask);
	work->evoke = 0;
      }
      break;
    case STEP_FADE:  /* フェードアウト中 */
      {
	int alpha;

	work->fade_cnt += TIME_BASE;
	alpha = work->fade_cnt * 128 / FADE_TIME;
	if(alpha > 128) alpha = 128;
	work->fade_mask->sprite.col.a = alpha;
	if(work->fade_cnt >= FADE_TIME)
	  {
	    /* 画面が完全に見えなくなる時点で、
	       チャネル 0 を非表示にし、パケット用エリアを別の malloc 空間
	       として割り当てる */
	    init_photo_term_mode(work);
	    work->step = STEP_LOAD;
	    SPR_HIDE(work->fade_mask);
	  }
      }
      break;
    case STEP_LOAD:
      if(cdbios_get_status()) break;
      CDC_LoadLogicalDirectory(work->file_entry);
      work->step = STEP_LOADING;
      break;
    case STEP_LOADING:
      if(!CDC_SyncCodecDataLoad())
	{
	  CDC_InitLoadData();
	  work->step = STEP_EXEC;
	}
      break;
    case STEP_EXEC:
      /* 起動レイアウトアクションを再生し、Act() を切替える */
      {
	if(work->cold_start)
	  SetupAct(work, act_booting);
	else
	  SetupAct(work, act_title);
      }
      break;
    }
}


/* ------------------------------------------------------------------------ */
static void Act(Work * work)
{
  work->pad = &GV_PadDataDirect[0];

  (work->act_func)(work);
}


static void Die(Work * work)
{
  /* 確保している全ての資源を開放 */
  DBG("[0]\n");
  if(work->handle >= 0) L2D_ReleaseLayout(work->handle);

  if(work->pict_work)
    GV_DestroyOtherActor(work->pict_work);

  codecMemRecov();

  /* フェードマスク用オブジェクトは、起動時から終了時まで存在し続ける。 */
  SPR_Destroy_2D_Object(work->fade_mask);

  if(work == now_work) now_work = NULL;
  DBG("Photo Terminal is die: success.\n");
}

static int GetResources(Work * work, int name, int where)
{
	static SPR_POS zero = {0.0F, 0.0F} ;
	static SPR_COLOR black = SPR_COLOR_CTOR(0, 0, 0, 0);
  DBG("Photo Transfer Terminar\n");

  if(NULL != now_work) return -1;
  now_work = work;

  work->proc_id = 0;

  /* DG_DMAPACK のポインタは、使用されていないときは NULL が入る */
  work->pict_work = NULL;
  work->sel_pict = -1;

  /* レイアウト名の取得 */
  work->skin = -1;
  work->evoke = 0;
  if(GCL_GetOption('s'))  /* 'skin' */
    work->skin = GCL_GetNextInt();

  if(GCL_GetOption('f'))  /* 'file' */
    work->file_entry = GCL_GetNextInt();

  if(work->skin < 0) return -1;
  work->handle = -1;
  work->fade_mask = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
  SPR_SetSizeSprite(work->fade_mask, 512.0F, 384.0F);
  SPR_SetPosSprite(work->fade_mask, &zero );
  work->fade_mask->sprite.col = black ;
  work->fade_mask->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
  work->fade_mask->head.flags |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
  SPR_HIDE(work->fade_mask);

  SetupAct(work, act_waiting);
  DBG("Photo Trans Term setup success\n");

  GV_SetActorSignalFunc(work, ReceiveSignal);

  return 0;
}

void * NewPhotoTerminal(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActorPrio(PHOTO_ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;
  
  GV_SetActor(work, Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResources(work, name, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}

