//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  mobile.c
  携帯メール画面。
  どうでもいいけど、日本以外に携帯電話でメールを受け取る習慣がある国ってあるの?

  2001/05/24  Y.Kira
  $Id: mobile.c,v 1.4 2003/01/13 08:41:40 takaki Exp $
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

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "../codec/codecmem.h"
#include "../codec/cdc_load.h"
#include "mob_panel.h"
#include "../phototerm/photo_itp.h"

#include "bp_threading.h"

#define _mobile_c_
#include "mobile.h"

extern int SK_CheckMapStatus( void );

#define DISP
#ifdef DISP
#ifdef DEBUG
#define DBG(args) printf(args)
#else
#define DBG(args)
#endif  /* DEBUG */
#endif  /* DISP */

#define PANEL_WIDTH    384.0F
#define PANEL_HEIGHT   150.0F

#define PANEL_X   64.0F
#define PANEL_Y   320.0F

#define DEFAULT_MAX_TIME 4500  /* 非装備であれば、着信から15秒以内に装備 */
#define DEFAULT_HOOK_TIME 300  /* 装備中であれば、着信から1秒後に受信 */

typedef struct Work {
  GV_ACT_EX    actor;

  GV_PAD     * pad;   /* パッドステータスへのポインタ */

  void (*act_func)(struct Work * work);  /* Act() 処理用関数ポインタ */
  MOBILE_MESG_PANEL panel;  /* メッセージパネル               */
  PHOTO_ITP_PARAM  param;  /* スクリプト解釈部インタフェース */

  /*
   * 基本情報
   */
  int          layout;  /* レイアウト名称 */


  /*
   * 稼働情報
   */
  int          handle;  /* レイアウト稼働中のハンドル(普段は負の値) */
  int          step;


  int          proc_id; /* 終了プロシージャの ID */
  int          code;    /* 実行コード */

  int          file_entry;  /* ファイル名(t-symbol の値)  */

  int          call_cnt;       /* コールされてからの経過時間         */
  int          call_max_time;  /* コール開始から切れるまでの最長時間 */
  int          hook_time;      /* 装備している状態で着信してから、
				  受け取るまでの時間 */

  int          vib_cnt;        /* 振動パターン生成のためのカウンタ */

  int          old_chanl_disp[4];


  int          call_stop_cnt;  /* 起動が決定してから、起動音を消すまでのカウンタ */

  int          call_se_on:1;   /* 1 のとき、CALL 用 SE が鳴っている */
  int          autoskip:1; /* メッセージ高速スキップフラグ */
  int          evoke:1;    /* 起動スイッチ                 */
  int          vib_sw:1;   /* 振動スイッチ                 */
  int          vib_mask:1; /* 振動マスク                   */
  int          pre_stat:1; /* 直前のフレームが無線/ノード中であったかどうか */

} Work;


/* ========================================================================= */

static void act_finish(Work * work);
static void act_reading(Work * work);
static void act_mobile(Work * work);
static void act_waiting(Work * work);

/* ------------------------------------------------------------------------- */

static Work * now_work = NULL;


/* ------------------------------------------------------------------------- */

static void SetupAct(Work * work, void (*act)(Work *))
{
  work->act_func = act;
  work->step = 0;
}


/* ------------------------------------------------------------------------- */

/*
 * GCL から command 定義されているものを使って呼び出す、
 * もしくはプログラムから以下の関数を呼び出せば起動。
 */
void AccessMobile(void)
{
  Work * work = now_work;
  int   code, proc_id;

  if(NULL != GCL_GetOption('r'))  /* 'reset' */
    {
      work->evoke = 0;   /* 呼び出しはクリア */
      work->vib_sw = 0;  /* 振動は停止 */
#ifdef CALL_SOUND_ON
      GM_SdSet(PHONE_MUTE);
      work->call_se_on = 0;
#endif /* CALL_SOUND_ON */
      SetupAct(work, act_waiting);
      return;  /* 他のオプションは全て無視 */
    }

  code = -1, proc_id = 0;
  if(NULL != GCL_GetOption('c'))  /* 'call' */
    code = GCL_GetNextInt();
#ifdef DEBUG_MODE
  else
    HANGUP();
#endif

  if(NULL != GCL_GetOption('p')) /* 'p' */
    proc_id = GCL_GetNextInt();

  work->code    = code;
  work->proc_id = proc_id;

  work->evoke = 1;
}

/* ------------------------------------------------------------------------- */


static void Act(Work * work)
{
#if defined(BP_VITA)
   extern void MENU_Item_Panel_PulseAnim();
   extern void MENU_Item_Panel_Swap_Texture(int);
#endif

  /* 無線中,ノード中は、振動と着信音を停止する */
  if(GM_CheckMenuStatus(MENU_RADIO_ON) || GM_CheckMenuStatus(MENU_NODE_ON))
    {
      if(!work->pre_stat)
	{
	  work->pre_stat = 1;
	  if(work->vib_sw) GM_SdSet(PHONE_MUTE);
	}
    }
  else
    if(work->pre_stat)
    {
	   work->pre_stat = 0;
	   work->vib_mask = 1;
	   if(work->vib_sw) 
      {
#if defined(BP_VITA)
         MENU_Item_Panel_PulseAnim();
#endif
         GM_SdSet(PHONE_CALL);  /* 着信音を再開する */
      }
    }

  if(work->vib_sw && work->vib_mask)
    {
      /* 0.5秒振動させて 0.5秒止めるを繰り返す。 */
#ifndef KP_WINDOWS
      if((work->vib_cnt += TIME_BASE) < 150) GM_PadVibration1 = 1;
#else
      if((work->vib_cnt += TIME_BASE) < 150)
        {
          GM_PadVibration1  = 1;
          GM_DispVibration2 = 60;	// Windows版は画面振動(代替処理)
		}
#endif

#if defined(BP_VITA)
      if(work->vib_cnt >= 100) 
      {
         MENU_Item_Panel_Swap_Texture(IT_PHS);
         work->vib_cnt -= 100;
      }
#else
      if(work->vib_cnt >= 300) 
      {
         work->vib_cnt -= 300;
      }
#endif
    }
#ifdef CALL_SOUND_ON
  if(work->call_se_on)
    work->call_stop_cnt += TIME_BASE;
#endif /* CALL_SOUND_ON */

  work->pad = &GV_PadDataDirect[0];
  (work->act_func)(work);
}

static void Die(Work * work)
{
  /* 終了時に呼び出し中であれば、呼び出し音を消す */
  if(work->vib_sw) GM_SdSet(PHONE_MUTE);

  if(work->handle >= 0)
    {
      L2D_ReleaseLayout(work->handle);
      work->handle = -1;
    }

  if(now_work == work) now_work = NULL;
}

static void draw_channel_recov(Work * work)
{
  int i;

  for(i = 0; i < 4; i++)
    DG_Chanls[i].flag = work->old_chanl_disp[i];
}

static void draw_channel_setup(Work * work)
{
  int i;

  /* 現在の描画状態を保存し,通常の描画チャネルの描画を禁止する */
  for(i = 0; i < 4; i++)
    {
      work->old_chanl_disp[i] = DG_Chanls[i].flag;
      DG_Chanls[i].flag = 0;
    }
}


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
static void mobile_script_setup(Work * work)
{
  void * stack;

  stack = codecMalloc(ITP_STACK_SIZE);
  ASSERT(NULL != stack);

  work->param.stack     = stack;
  work->param.stacksize = ITP_STACK_SIZE;

  PHOTO_ItpInit(&work->param);
}

/* 解釈タスクスタックおよび、データの開放 */
static void mobile_script_finish(Work * work)
{
  codecFree( work->param.stack );
#if 0 //BP_PS2
  codecFree( work->param.data_buffer );
#else
  free( work->param.data_buffer );
#endif
}

static void mobile_load(Work * work, int code)
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
  work->param.data = (void *)((int)buffer + codecOffset);

  /* 判定スクリプトは,無線スクリプトと同じ扱い */
  FS_LoadFileRequest(FS_FILE_CODEC, start, size, buffer, codecTop);
}

/*
 * ロードの終了待ち
 */
static int is_mobile_load_end(Work * work)
{
  return (FS_LoadFileSync() > 0) ? 0 : 1;
}

/* ========================================================================= */

/*
 * メールを読み終り,携帯モードを閉じる
 */
static void act_finish(Work * work)
{
  enum {
    STEP_INIT,
    STEP_CLOSING,
    STEP_EXIT,
    STEP_PRE_RECOV,
    STEP_RECOV,
    STEP_FINISH
  };

#ifdef KP_WINDOWS
  if( DG_CurrentFrameDrawSkip() ){ return ; }
#endif

  switch(work->step)
    {
    case STEP_INIT:     /* 終了段階初期化 */
      work->step = STEP_EXIT;
      break;

    case STEP_EXIT:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;

      /* レイアウトは用済なので開放する */
      L2D_ReleaseLayout(work->handle);
      SPR_ResetMemoryManager(); /* 2D オブジェクト確保用の関数をリセットする */

      /* ロードデータも不要なので開放 */
      if(work->file_entry >= 0) CDC_ReleaseLogicalDirectory();

      work->handle = -1;
      work->step = STEP_PRE_RECOV;
      break;
    case STEP_PRE_RECOV:
      work->step = STEP_RECOV;
      break;
    case STEP_RECOV:
      if(codecMemRecov()) break; /* メモリ領域の復帰       */
      work->step = STEP_FINISH;
      break;
    case STEP_FINISH:
      DG_SetPrivilegeMode(0);    /* 特権モード解除         */
      draw_channel_recov(work);  /* 描画チャネル表示の復帰 */
      GV_PauseOffActorSystem( GV_PAUSE_STOP );  /* ポーズからの復帰 */ 
      GM_ResetMenuStatus(MENU_NODE_ON); /* メニューフラグをリセット */
      if(work->proc_id != 0)
	{
	  GCL_ExecProc(work->proc_id, NULL);
	  work->proc_id = 0;
	}
      SetupAct(work, act_waiting);
      break;
    }
}

/*
 * メール読み中 Act() 処理
 */
static void act_reading(Work * work)
{
  enum {
    STEP_INIT,      /* 初期化                                   */
    STEP_SWITCH,    /* スイッチを押すまで待つ                   */
    STEP_REQ_WAIT,  /* 解釈部からのリクエスト待ち               */
    STEP_SKIP,      /* 命令を実行せずに、次の処理へ             */
    STEP_SKIP_2,    /* 命令を実行せずに次の処理へ。コマ設定など */
    STEP_KEY_WAIT,  /* キー入力待ち                             */
    STEP_FINISH,    /* スクリプト終了                           */
  };
  
  switch(work->step)
    {
    case STEP_INIT:
      {
	SPR_OBJ * obj;
	float x, y, w, h, tw, th;

	x = PANEL_X;
	y = PANEL_Y;
	w = PANEL_WIDTH;
	h = PANEL_HEIGHT;

#if 0 //BP
	/* レイアウトから,LCD パネルの位置を取得する */
	obj = L2D_GetObject(work->handle, OBJ_LCD);
	if(NULL != obj)
	  {
	    /* 文字表示領域を得る */
	    x = obj->box.rect.begin.x;
	    y = obj->box.rect.begin.y;
	    w = obj->box.rect.end.x - x;
	    h = obj->box.rect.end.y - y;
	    
	    printf("         w = %d\n", w);
	    /* 実寸に直す */
	    x = SPR_SCALE_X(MOBILE_CHANL, x);
	    y = SPR_SCALE_Y(MOBILE_CHANL, y);
	    w = SPR_SCALE_X(MOBILE_CHANL, w);
	    h = SPR_SCALE_Y(MOBILE_CHANL, h);
	    printf("         w = %d\n", w);
	  }
   tw = w;
   th = h;
#else
   x = 200;
   y = 132;

   w = 112;
   h = 175;

   tw = 126;
   th = h;
#endif

	/* パネルを初期化、表示 */
	MOBILE_OpenMesgPanel(&(work->panel), tw, th);
	MOBILE_DrawMesgPanel(&(work->panel), x, y, w, h);
	MOBILE_DrawMessage(&(work->panel), NULL);
	
	work->autoskip = 0;
	
	work->step = STEP_SWITCH;
      }
      break;
    case STEP_SWITCH:
	/* 着信が決定してから、NTSC で云う 40 フレームが経過するまで待つ */
      if(work->call_stop_cnt < 200) break;
#ifdef CALL_SOUND_ON
      GM_SdSet(PHONE_HANGUP);   /* 着信音 */
      work->call_se_on = 0;
#endif
      work->step = STEP_REQ_WAIT;
      break;
    case STEP_REQ_WAIT:
      /* 命令が少ないので、写真転送端末のリクエスト構造を流用する。*/
      switch(work->param.request)
	{
	case PHOTO_REQ_MESG_WAIT:
	  MOBILE_DrawMessage(&(work->panel), BP_GCL_LOOKUP_NEW_FONT_STRING((char *)(work->param.req[0])));
	  work->step = STEP_KEY_WAIT; /* 音声は無いので, 常にキー送り */
	  break;
	case PHOTO_REQ_END:
	  work->step = STEP_FINISH;
	  break;
	case PHOTO_REQ_VOX_START: /* 音声命令は無視する     */
	case PHOTO_REQ_SET_FRAME: /* コマ設定は無視する     */
	  work->step = STEP_SKIP;
	  break;
	case PHOTO_REQ_GET_FRAME: /* コマ番号取得は無視する */
	  work->step = STEP_SKIP_2;
	  break;
	}
      break;
    case STEP_SKIP:
      WakeupItp(work, 0);
      break;
    case STEP_SKIP_2:
      WakeupItp(work, PHOTO_ERR);
      break;
    case STEP_KEY_WAIT:  /* キー入力待ち */
      if((work->pad->press &(PAD_A | PAD_B)) || work->autoskip)
	{
	  if(work->pad->press & (PAD_A | PAD_B))  GM_SdSet(PHONE_HANGUP);
	  WakeupItp(work, 0);
	  work->step = STEP_REQ_WAIT;
	  MOBILE_DrawMessage(&work->panel, NULL);
	}
      if(work->pad->press & (PAD_X | PAD_Y)) work->autoskip ^= 1;
      break;
    case STEP_FINISH:    /* 終了         */
#ifdef KP_WINDOWS
      if( DG_CurrentFrameDrawSkip() ){ break ; }
#endif

      MOBILE_CloseMesgPanel(&(work->panel));  /* パネルを閉じる */
      mobile_script_finish(work);
      SetupAct(work, act_finish);            /* 終了 Act() へ移行 */
      break;
    }
}

/*
 * モバイル起動 Act() 処理
 */
static void act_mobile(Work * work)
{
  enum {
    STEP_INIT,    /* 携帯モードに入った直後の初期化   */
    STEP_OPEN,    /* 開き中                           */
    STEP_READING, /* 閲覧状態                         */
    STEP_LOADEND  /* ロード終了待ち                   */
  };
  switch(work->step)
    {
    case STEP_INIT:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      L2D_EvokeAction(work->handle, MOBILE_OpenPhone);
      work->step = STEP_OPEN;
      break;
    case STEP_OPEN:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      L2D_EvokeAction(work->handle, MOBILE_MailBlink);
      mobile_load(work, work->code);
      work->step = STEP_READING;
      break;
    case STEP_READING:
      if(is_mobile_load_end(work)) work->step = STEP_LOADEND;
      break;
    case STEP_LOADEND:


      /* MOBILE_Read を再生中であれば何もしない */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle)) break;
      mobile_script_setup(work);   /* スクリプト実行準備           */
      SetupAct(work, act_reading); /* ロードが終れば次のステップへ */
      break;
    }
}

static int init_mobile_mode(Work * work)
{
  void * tmp;
  if(NULL == (tmp = GV_Malloc(1024 * 64))) return -1;
  GV_PauseOnActorSystem( GV_PAUSE_STOP ); /* ポーズをかける       */
  GV_Free(tmp);
  draw_channel_setup(work);               /* 描画チャネルの設定   */
  codecMemInit();                         /* 専用メモリ領域を用意 */
  
  GM_SetMenuStatus(MENU_NODE_ON);  /* メニューフラグ上ではノードと同じ扱い */
  SPR_SetMemoryManager(codecMalloc, codecDelayedFree);
  return 0;
}


static void vibration_start(Work * work)
{
#if defined(BP_VITA)
  extern void MENU_Item_Panel_PulseAnim();
  extern void MENU_Item_Panel_ResetAnim(int);

  if (GM_Item == IT_PHS)
  {
     MENU_Item_Panel_ResetAnim(1);
     MENU_Item_Panel_PulseAnim();
  }
  else
  {
     MENU_Item_Panel_Swap_Texture(IT_PHS);
     MENU_Item_Panel_ResetAnim(0);
  }
#endif

  work->vib_sw = 1;
  work->vib_cnt = 0;

}

static void vibration_stop(Work * work)
{
#if defined(BP_VITA)
   extern void MENU_Item_Panel_ResetAnim(int);
   MENU_Item_Panel_ResetAnim(0);
#endif

  work->vib_sw = 0;
}

/*
 * 待機中 Act() 処理
 */
static void act_waiting(Work * work)
{
   float safeZoneOffsetY=0;


  enum {
    STEP_INIT,
    STEP_WAITING,
    STEP_HAVE_MOBILE,
    STEP_HAVE_OTHER,
    STEP_EVOKE,
    STEP_LOAD_START,
    STEP_LOAD_WAIT,
    STEP_START
  };
  
  work->vib_mask = 1;
  switch(work->step)
    {
    case STEP_INIT:     /* 初期化 */
      work->evoke = 0;            /* 起動信号を切る */
      work->step = STEP_WAITING;
      work->call_se_on = 0;      /* SE は鳴っていない */
      break;
    case STEP_WAITING:  /* 待機中 */
      if(!work->evoke) break;   /* 起動信号がなければ何もしない */

      vibration_start(work);
#ifdef CALL_SOUND_ON
      GM_SdSet(PHONE_CALL);   /* 着信開始(着信音発生) */
#endif /* CALL_SOUND_ON */

      work->step = (IT_PHS == PL_GetPlayerItem())
	? STEP_HAVE_MOBILE : STEP_HAVE_OTHER;
      work->call_cnt = 0;
      break;

    case STEP_HAVE_MOBILE:   /* コールの段階で携帯電話を装備していた場合     */
      /* メニュー選択中は何もしない */
      if(GV_PauseLevel & (GV_PAUSE_MENU | GV_PAUSE_STOP | GV_PAUSE_PAUSE))
	{
	  work->vib_mask = 0;
	  return;
	}
      /* コール中に他の装備に切替えられたら、
	 携帯電話を装備していない場合の扱いに切替える */
      if(IT_PHS != PL_GetPlayerItem())
	{
	  work->step = STEP_HAVE_OTHER;
	  break;
	}
      work->call_cnt += TIME_BASE;
      if(work->call_cnt >= work->hook_time) work->step = STEP_EVOKE;
      break;
    case STEP_HAVE_OTHER:   /* コールの段階では携帯電話を装備していない場合 */
      /* メニュー選択中は何もしない */
      if(GV_PauseLevel & (GV_PAUSE_MENU | GV_PAUSE_STOP | GV_PAUSE_PAUSE))
	{
	  work->vib_mask = 0;
	  return;
	}

      /*
       * 装備が携帯電話になったら受信
       */
      if(IT_PHS == PL_GetPlayerItem())
	{
	  work->step = STEP_EVOKE;
	  break;
	}
      work->call_cnt += TIME_BASE;
      if(work->call_cnt >= work->call_max_time)
	{
	  vibration_stop(work);
#ifdef CALL_SOUND_ON
	  GM_SdSet(PHONE_MUTE);   /* 着信音停止 */
	  work->call_se_on = 0;
#endif /* CALL_SOUND_ON */
	  work->evoke = 0;
	  work->step = STEP_WAITING;
	}
      break;
    case STEP_EVOKE:    /* 起動開始 */
      /* ポーズ画面がまだ処理中である場合は何もしない */
      if(SK_CheckMapStatus()) break;

      /* 携帯モードに入れない場合は何もしない */
      if(init_mobile_mode(work)) break;
#ifdef CALL_SOUND_ON
      work->call_se_on = 1;
      work->call_stop_cnt = 0;   /* 起動開始 */
#endif
      vibration_stop(work);
      work->step = STEP_START;
      DG_SetPrivilegeMode(1);   /* 特権描画モードへ */
      if(work->file_entry >= 0)
	{
	  work->step = STEP_LOAD_START;
	}
      break;

    case STEP_LOAD_START:
      if(cdbios_get_status()) break;
      CDC_LoadLogicalDirectory(work->file_entry);
      work->step = STEP_LOAD_WAIT;
      break;
    case STEP_LOAD_WAIT:
      /* ロードが終るまで何もしない */
      if(CDC_SyncCodecDataLoad()) break;
      CDC_InitLoadData();
      work->step = STEP_START;
      break;
    case STEP_START:
      {
	void * l2d_ptr;

	l2d_ptr = GV_GetCache(GV_CacheID(work->layout, 'o'));
	if(NULL == l2d_ptr)
	  l2d_ptr = CDC_GetFileEntry(work->layout, 'o');
	if(NULL == l2d_ptr)
	  printf( "No LayoutData(%d)\n", work->layout ) ;

	work->handle = L2D_SetupLayout2(l2d_ptr,
					MOBILE_CHANL, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	L2D_EvokeAction(work->handle, MOBILE_DefaultAction);
	/* 携帯モード起動アクション実行 */
	SetupAct(work, act_mobile);
      }
      break;

    }
}

static int GetResources(Work * work, int name, int where)
{
  ASSERT(NULL == now_work);

  work->pre_stat = 0;  /* 直前は無線ではない(起動直後なので) */
  work->handle = -1;   /* ロード前なので、ハンドルは負の値にしておく */
  work->vib_sw = 0;
  if(NULL != GCL_GetOption('s'))  /* 'skin' */
    work->layout = GCL_GetNextInt();   /* 使用するレイアウトの名称 */

  work->file_entry = -1;
  if(NULL != GCL_GetOption('f'))  /* 'file' */
    work->file_entry = GCL_GetNextInt();

  work->call_max_time = DEFAULT_MAX_TIME;
  work->hook_time     = DEFAULT_HOOK_TIME;
  if(NULL != GCL_GetOption('t')) /* 'time' */
    {
      work->call_max_time = GCL_GetNextInt();
      work->hook_time = GCL_GetNextInt();
    }
#ifdef DEBUG_MODE
  if(work->call_max_time < work->hook_time)
    {
      printf("<MOBILE> Illegal time limit.\n");
      HANGUP();
    }
#endif /* DEBUG_MODE */


  work->evoke = 0;
  work->code = -1;
  work->proc_id = 0;

  now_work = work;

  SetupAct(work, act_waiting);

  return 0;
}

void * NewMobileDaemon(int name, int where)
{
  Work * work;

  DBG("Mobile Terminal Initialize!!\n");
  if(NULL == (work = GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, name, where))
    {
      DBG("Mobile Failed.\n");
      GV_DestroyActor(work);
      return NULL;
    }
  DBG("Mobile Success.\n");
  return work;
}
