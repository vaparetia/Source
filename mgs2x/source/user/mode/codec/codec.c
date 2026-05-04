//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	codec.c
		無線／表示部
	2000/07/24      Y.Kira
	2000/07/17	K.Uehara
	$Id: codec.c,v 1.8 2003/01/05 04:44:19 takaki Exp $
*/
#ifdef PSX2
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
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "debugmenu.h"
#include "codec_config.h"
#include "codec.h"

#include "g_define.h"
#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "strctrl.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"
#include "c_layout.h"
#include "cdc_load.h"
#include "stream.h"
#include "cdc_mind.h"
#include "cdc_hair.h"
#include "cdc_movie.h"
#include "codec_signal.h"
#include "codec_act.h"
#include "codeccap.h"
#include "memcall.h"
#include "cdc_noise.h"
#include "cdc_vib.h"
#include "face_bug.h"
#include "interval_noise.h"
#include "bp_threading.h"

#include "../../kira/2D_action/layout_2d.h"
#include "../../kira/radar/rdr_movie.h"
#include "../../kano/mcman/mcman.h"
#ifdef KP_XBOX
#include "../../kano/xmcman/xmcman.h"
#endif

#include "BP_BuildDefines.h"
#include "BP_Misc.h"
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

int GM_StreamMtDriverInit( void );

#include "BP_AudioStream.h"
#include "BP_SoundSupport.h"


#ifdef KP_XBOX
#define DBG
#else
#undef _DEBUG_
#ifdef _DEBUG_
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif  /* _DEBUG_ */
#endif
/*
	コンフィグレーション
*/

#ifdef PSX2
#define MESG_PANEL_W		(640)/*(512-64+FONT_SIZE_W)*/
#endif
#ifdef KP_XBOX
#define MESG_PANEL_W		(512)
#endif

//Added FONT_SIZE_H to allow the 4th line of text to show up (MGSTWO-3376)
#define MESG_PANEL_H	 (6 * FONT_SIZE_H)//((FONT_SIZE_H+FONT_SIZE_H/2)*3+FONT_SIZE_H/2)

#define CALL_MIN_WAIT           300
#define NORMAL_CALL_TIME	((60*5) * 10)  /* 10秒間 */
#define DEMO_CALL_TIME		(60 * 5)
#define CALLING_TIME            360   /* CALL 音の間隔 */
//#define FADE_TIME			(60*1)

#define FADE_TIME		(10 * 5)
#if 0
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

#define TIMEBASE  (BP_BASE_TICK())

extern void * GM_PlayerWork;

static int    last_freq = FREQ_MIN;
static int    last_responsed_freq = -1;
static int    response_count = 0;
static void * bg_work = NULL;

#if defined(BP_VITA)
//Difference between current delta and NEUTRAL_ANALOG_STICK_POS required in order to
//make a call or use the mem function through the analog stick.
#define DESIRED_ANALOG_DELTA_FOR_CALL 110
#endif

/*
	解釈部は別スレッドとして起動。
	表示部は解釈部からのリクエストを受け取る。
*/

/* 表示部 */

typedef struct _codec__work {
  GV_ACT_EX	actor;
  int name;
  
  GV_PAD *pad;
  void (*do_func)( struct _work *work );
  
  /* 解釈部スレッド管理 */
  CODEC_ITP_PARAM	param;
  
  /* 起動用 */
  int	status;
  int   count;
  
  char type;
  char fade_flag;
  short freq;
  short freq_tmp;
  int code;
  int proc_id;

  int   dummy_bone;
  int   player_name;   /* プレイヤ名(プレイヤ発言時のうなづきを禁止するため) */
  
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

  void *  movie_work;
  int     movie_start;  /* ムービー表示状態 */

  int call_cnt;
  int main_vox_handle;
  int main_ipu_handle;
  int pre_cap_status;
  int finish_cnt;
  int    fade_param;
  void * face_work[2];

  char    finish_id;

  int memcall_select;  /* メモリー呼出項目 */
  int memcall_status;  /* メモリー呼出結果 */
  int mem_cnt;         /* メモリー呼び出し項目数 */

  /* 継続コール予約 */
  
  void  * reserve_list;   /* 終了後コール予約リストのポインタ */
  int     reserve_mode;   /* 0: 切ってから Send / 1: 切ってから Call */
  int     reserve_freq;   /* 予約周波数                       */
  int     reserve_code;   /* 予約コード                       */

  /* メニュー選択用 */
  int     sel_total;  /* 選択肢総数     */
  char ** sel_list;   /* 選択項目リスト */
  int     sel_num;    /* 選択番号       */
  int     sel_pre;    /* 直前の選択項目 */

  /* 待ち時間カウンタ */
  int     wait_cnt;

  /* フェードアウト用マスク */
  SPR_OBJ * fade_mask;


  /* デフォルトモーション ID */
  int     defmtn_id[2];

  int     l2d_handle;     /* CALLサイン用 .l2d ハンドル */
  int     call_step;      /* CALL サイン表示進行        */
  int     call_act_idle;  /* CALL サイン表示中アクション名   */
  int     calling_time;   /* コールを開始してからの経過時間 */

#ifdef AUX_CALL_ENABLE
  int     call_mode;   /* 0: 強制コール / 1: 任意コール */
  int     call_act_open[2];
  int     call_act_close[2];
#else
  int     call_act_open;  /* CALL サイン表示開始アクション名 */
  int     call_act_close; /* CALL サイン表示終了アクション名 */
#endif /* AUX_CALL_ENABLE */

  int     disp_enable;    /* 0: 顔表示禁止           / 1: 顔表示許可         */
  int     pre_enable;

  int     save_return;    /* セーブモードの返り値 */
  int     save_step;      /* セーブモードへの移行進捗 */

  int     alarm_cnt;      /* 呼び出しアラーム回数 */

  CVECTOR old_fog;        /* 旧 fog カラー */
  float   old_fog_param1; /* 旧 fog パラメータ 1 */
  float   old_fog_param2; /* 旧 fog パラメータ 2 */
#ifndef PSX2
  FVECTOR	old_fog_param ;
#endif
  
  void  * tmp_area;       /* 容量予約用ポインタ */

  /* 無線前のメニューフラグ保持領域 */
  int     old_menu_flags;

  /* 現在表示されている顔が, face か、bugface か(0:face / 1:bugface) */
  char    face_bug_status[2];

  u_int     waiting_icon:1;
  u_int     fadein_break:1;
  u_int     fadeout_break:1;
  u_int   fade_mode:1;    /* 0: 通常の無線モード開始 / 1: フェードイン開始   */
  u_int     fade_out:1;     /* 0: 通常の無線モード終了 / 1: フェードアウト終了 */
  u_int     act_open:1;     /* 0: 開きアクションなし   / 1: 開きアクションあり */
  u_int     act_close:1;    /* 0: 閉じアクションなし   / 1: 閉じアクションあり */
  u_int     voice_cancel:1; /* 0: 音声キャンセル前     / 1: 音声キャンセル後   */
  u_int     vox_playing:1;  /* 0: 音声再生中ではない   / 1: 音声再生中         */
  u_int     mov_playing:1;  /* 0: 動画再生中ではない   / 1: 動画再生中         */
  u_int     autoskip:1;     /* 0: 要キー入力           / 1: オートスキップ     */
  u_int     call_block:1;   /* 0: 制限無し             / 1: 呼出/応答不可      */
  u_int     mind_mask:1;    /* 0: うなづき許可         / 1: うなづき禁止       */
  u_int     save_mode:1;    /* 0: 非セーブモード       / 1: セーブモード中     */
  u_int     cont_call:1;    /* 0: 継続コール予約無し   / 1: 継続コール予約あり */
  u_int     speech_mode:1;  /* 0: 通常モード           / 1: 演説モード         */
  u_int     speach_ch1:1;   /* 0: Ch.1 PAUSE無し       / 1: Ch.1 PAUSE あり    */
  u_int     evoke_set:1;    /* 0: 起動は確定していない / 1: 起動が確定した     */
  u_int     alarm_disable:1;/* 0: アラームを鳴らす     / 1: アラーム不可       */
  u_int     icon_disable:1; /* 0: CALLアイコンを表示   / 1: CALL アイコン不可  */
  u_int     icon_show:1;    /* 0: CALLアイコン表示禁止 /1: CALLアイコン表示許可*/
  u_int     start_mtn:1;    /* フェードスタート時に最初のモーションを設定したか*/

  int     codec_must_block:1;  /* 無条件で無線処理をブロックする */

} Work;

static Work *codec_work = NULL;


#ifdef DEBUG_MODE

static int _DEBUG_codec = 0;
static int _DEBUG_codec_into = 0;
static int _DEBUG_codec_call = 0;
static int _DEBUG_aging = 0;

#ifndef KP_XBOX

static GM_DEBUG_MENU debug_menu = {
  class:   "CODEC",
  menu:    "EVOKE STATUS",
  max:     2,
  items: (char *[]){"HIDE", "DISP"},
  values: (int[]){0, 1},
  target: &_DEBUG_codec,
  mask: 0x00000001
};

static GM_DEBUG_MENU debug_menu2 = {
  class:   "CODEC",
  menu:    "FAILE REASON",
  max:     2,
  items: (char *[]){"OFF", "ON"},
  values: (int[]){0, 1},
  target: &_DEBUG_codec_into,
  mask: 0x00000001
};

static GM_DEBUG_MENU debug_menu3 = {
  class:   "CODEC",
  menu:    "CALL SEQUENCE",
  max:     2,
  items:  (char *[]){"OFF", "ON"},
  values: (int[]){0, 1},
  target: &_DEBUG_codec_call,
  mask: 0x00000001
};

static GM_DEBUG_MENU debug_menu4 = {
  class:   "CODEC",
  menu:    "AGING TEST",
  max:     2,
  items: (char *[]){"OFF", "ON"},
  values: (int[]){0, 1},
  target: &_DEBUG_aging,
  mask: 0x00000001
};

#endif


#endif

enum {
	TYPE_NORMAL = 1,		// 通常選択画面へ
	TYPE_CALL = 2,			// CALLが来たが無視可能
	TYPE_DEMO = 3,			// CALLが来たが強制的に移行
	TYPE_DIRECT = 4,		// CALLなしで強制的に移行
};

enum {
  CALL_STEP_WAIT,     /* 待機中 */
  CALL_STEP_INIT,     /* CALL 表示準備 */
  CALL_STEP_START,    /* CALL 開始アクション */
  CALL_STEP_CALLING,  /* CALL 表示中ループアクション */
  CALL_STEP_TIMEOUT,  /* CALL 時間切れアクション */
  CALL_STEP_FINISH,   /* 時間切れアクション終了,レイアウト開放 */
  CALL_STEP_BREAK,    /* CALL 表示中断 */
  CALL_STEP_SOUNDONLY,/* CALLの SE のみリピート */
};


static void setup_fade_mask(Work * work);
static int fade_out_wait(Work * work);
static int fade_in_wait(Work * work);

/*
 * セーブモード起動処理
 */
static  void evoke_save_mode(Work * work)
{
  void * ch_work;
#ifdef PSX2  
  if(GM_Configuration & GM_CONFIG_STORY_TANKER){
	  ch_work = CODEC_NewMCSave(CODEC_SAVE_ICON_TNK,SAVEGAME_L2D_STRCODE);
  }
  else{
	  ch_work = CODEC_NewMCSave(CODEC_SAVE_ICON_PLT,SAVEGAME_L2D_STRCODE);
  }
#else
  ch_work = CODEC_XNewMCSave( SAVEGAME_L2D_STRCODE );
#endif  
  ASSERT(NULL != ch_work);
  GV_SetActorChild(work, ch_work);
}

static int save_mode_waiting(Work * work)
{
  enum {
    STEP_INIT,
    STEP_FADEOUT,
    STEP_SAVE_START_WAIT,
    STEP_SAVE_START,
    STEP_SAVING,
    STEP_FADEIN,
    STEP_FINISH
  };
  int ret = 0;

  switch(work->save_step)
    {
    case STEP_INIT:
      setup_fade_mask(work);
      CODEC_HideMesgPanel( &work->panel );
      work->save_step = STEP_FADEOUT;
      break;
    case STEP_FADEOUT:
      if(fade_out_wait(work)) break;
      /* 表示を一旦消去して、セーブモードを起動する */
      codecLayoutDisp(0);  /* レイアウト消去 */

      /* 顔表示も一時中断 */
      DG_Chanls[2].flag = 0;
      DG_Chanls[3].flag = 0;
      work->save_step = STEP_SAVE_START_WAIT;
      evoke_save_mode(work);
      work->wait_cnt = 5;
      break;
    case STEP_SAVE_START_WAIT:
      if(--work->wait_cnt) break;
      work->save_step = STEP_SAVE_START;
      break;
    case STEP_SAVE_START:
      SPR_Destroy_2D_Object(work->fade_mask);
      work->fade_mask = NULL;
      work->save_step = STEP_SAVING;
      break;
    case STEP_SAVING:
      if(work->save_mode) break;

      setup_fade_mask(work);
      work->fade_param = 0;
      fade_in_wait(work);
      
      /* レイアウト,顔の表示状態を復帰 */
      DG_Chanls[2].flag = 1;
      DG_Chanls[3].flag = 1;
      codecLayoutDisp(1);

      work->save_step = STEP_FADEIN;
      break;
    case STEP_FADEIN:
      if(fade_in_wait(work)) break;
      work->save_step = STEP_FINISH;
      SPR_Destroy_2D_Object(work->fade_mask);
      work->fade_mask = NULL;
      CODEC_DrawMessage( &work->panel, NULL );
//      CODEC_ShowMesgPanel( &work->panel );
      CODEC_HideMesgPanel( &work->panel );  // Add 02/07/01 K.Uehara
      break;
    case STEP_FINISH:
      ret = 1;
      break;
    }
  return ret;
}

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
#ifndef KP_WINDOWS
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
  /* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+1);
#endif
  SPR_SetPriority(work->fade_mask, 7);
  work->fade_mask->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
  SPR_SetSizeSprite(work->fade_mask, 512.0F, 384.0F);
#ifdef BP_PSX2_GCC
  SPR_SetPosSprite(work->fade_mask, &(SPR_POS){0.0F, 0.0F});
#else //BP
//#endif
//#ifdef KP_XBOX
  {
      SPR_POS pos = { 0.0F, 0.0F };
      SPR_SetPosSprite(work->fade_mask, &pos );
  }
#endif
  
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
#ifndef KP_WINDOWS
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
  /* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+1);
#endif
  return (work->fade_param > 0) ? 1 : 0;
}

static int fade_in_wait(Work * work)
{
  work->fade_param += ((128 + (FADE_TIME - 1)) / FADE_TIME);
  if(work->fade_param >= 128) work->fade_param = 128;
#ifndef KP_WINDOWS
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
  /* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
  work->fade_mask->head.alpha =
    SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+1);
#endif
  return (work->fade_param < 128) ? 1 : 0;
}



static void face_erase(Work * work)
{
  int i;

  GV_CallChildSignalFunc(work, SIGNAL_BUGFACE_KILL, 0);
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
#if 0
  MENU_Locate( 256, 48, MENU_MODE_CENTER );
  MENU_Printf( "%d", work->freq );
  
  CODEC_DrawMesgPanel( &work->panel, 256 - ( MESG_PANEL_W * 512 / 640 ) / 2, 132 * DRAW_HEIGHT / 448 );
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
	// codecSetLight(0);
}

/*
 * メモリー選択コール時の処理
 */
static void CodecStartWait( Work *work );
static void CodecMode( Work *work );
static void CodecEndWait( Work *work );

static void MemoryCallMode(Work * work)
{
  enum {
    STATE_INIT,
    STATE_OPEN_WAIT,
    STATE_SELECT,
    STATE_CLOSE_WAIT,
    STATE_MEMORY_CANCEL
  };

  switch(work->status)
    {
    case STATE_INIT:                       /* メモリーコールモード初期化 */
      /* まずレイアウトを開く */
      // codecSetLight(CODEC_LIGHT_MEMORY);   /* "MEMORY" 点灯              */
      CODEC_HideMesgPanel( &work->panel ); /* メッセージパネルを隠す     */
      codecMemCallOpen(work->memcall_select);  /* リストを開く               */
      CodecSE(CODEC_MEM_WNDOPEN);         /* 開くときの SE              */
      work->status = STATE_OPEN_WAIT;
      break;

    case STATE_OPEN_WAIT:  /* 選択リストが開くまで待つ   */
      printf("open wait memory list.\n");
      if(codecMemCallStatus() == 0)
	{
	  /* 最後に選択した項目を得る */
	  work->memcall_select = CODEC_mem_last_select;
	  if(work->memcall_select < 0) work->memcall_select = 0;
	  
	  // 存在する項目の数を得る
	  work->mem_cnt = MemCallCount();
	  if(work->mem_cnt > 0)
	    work->freq_tmp = MemCallGetFreq(work->memcall_select); // DEBUG By K.Uehara 01/08/13
	  else
	    work->freq_tmp = -1;
	  work->status = STATE_SELECT;
	}
      break; 
    case STATE_SELECT:     /* 選択もしくはキャンセル等の処理 */

      // printf("sel = %d\n", work->memcall_select);

      if(work->pad->press & (PAD_U | PAD_D | PAD_R | PAD_L))
	{
	  int sel = work->memcall_select;
	  
	  CodecSE(CODEC_SE_SEL_PARSON);  /* 選択音 */

	  if(work->mem_cnt > 0)
	    {
	      if(work->pad->press & (PAD_R | PAD_L))
		{
		  sel ^= 1;
		  if(!codecMemCallSelect(sel))
		    {
		      work->memcall_select = sel;
		      work->freq_tmp = MemCallGetFreq(sel);
		      break;
		    }
		}
	      if(work->pad->press & PAD_U)
		{
		  sel -= 2;
		  sel &= 7;
		  while(codecMemCallSelect(sel))
		    {
		      sel ^= 1;
		      if(!codecMemCallSelect(sel)) break;
		      sel ^= 1;  /* 戻してから */
		      sel -= 2;  /* 上に動かす */
		      sel &= 7;
		    }
		  
		  work->memcall_select = sel;
		  work->freq_tmp = MemCallGetFreq(sel);
		  break;
		}
	      if(work->pad->press & PAD_D)
		{
		  sel += 2;
		  sel &= 7;
		  while(codecMemCallSelect(sel))
		    {
		      sel ^= 1;
		      if(!codecMemCallSelect(sel)) break;
		      sel ^= 1;  /* 戻してから */
		      sel += 2;  /* 下に動かす */
		      sel &= 7;
		    }
		  
		  work->memcall_select = sel;
		  work->freq_tmp = MemCallGetFreq(sel);
		  break;
		}
	    }
	}
      if(work->pad->press & PAD_OK)
	{
	  work->status = STATE_CLOSE_WAIT;
	  if(work->mem_cnt > 0)
	    {
	      CODEC_mem_last_select = work->memcall_select;
	      work->memcall_status = 1;
	      work->freq = work->freq_tmp;
	      codecSetFreq(work->freq);
	    }
	  else
	    {
	      work->memcall_status = 0;
	      CodecSE(CODEC_MEM_WNDCLOSE);
	    }
	  codecMemCallClose();
	}
      if(work->pad->press & PAD_CANCEL)
	{
	  if(work->mem_cnt > 0) CODEC_mem_last_select = work->memcall_select;
	  work->status = STATE_CLOSE_WAIT;
	  work->memcall_status = 0;
	  codecMemCallClose();
	  CodecSE(CODEC_MEM_WNDCLOSE);
	}
      /*
       * メモリー呼び出しモードでも、SELECT ボタンで無線モードを抜ける
       */
      if(work->pad->status & PAD_SEL)
	{
	  codecMemCallClose();
	  work->status = STATE_MEMORY_CANCEL;
	}
      break;
    case STATE_CLOSE_WAIT:       /* リストを閉じた後始末、
			      通常選択もしくは通話への移行   */
      if(codecMemCallStatus() == 0)
	{
	  //codecSetLight(0);   /* "MEMORY" 点灯              */
	  CODEC_ShowMesgPanel( &work->panel );
	  SetFunc(work, CodecMode);  /* 通常の無線選択に戻る */
	  codecSetFreq(work->freq);
	  if(work->memcall_status)  /* 呼出の選択があった場合 */
	    {
	      work->status = 3;
	      CodecSE(CODEC_SE_SEND);
	      work->call_cnt = 0;
	    }
	}
      break;
    case STATE_MEMORY_CANCEL:
      if(codecMemCallStatus() == 0)
	{
	  work->finish_id = 1;
	  CODEC_ShowMesgPanel( &work->panel );
	  codecSetFreq(work->freq);
	  SetFunc(work, CodecEndWait);  /* 通常の無線選択に戻る */
	}
      break;
    }
}

#if defined(BP_VITA)
extern int GV_direction_pressed_with_threshold( GV_PAD *pad, int padDirection, unsigned char desiredAnalogDelta );
#endif

static int DoCodecSelect( Work *work )
{
  GV_PAD *pad = work->pad;
  struct selectinf *si;
  int stat = 0;
  

  si = &( work->inf.select );

#ifdef DEBUG_MODE
  /*
    if(_DEBUG_aging)
    {
    SetFunc(work, MemoryCallMode);
    return SELECTING;
    }
  */
#endif

#if defined(BP_VITA)
   if( GV_direction_pressed_with_threshold( pad,  PAD_PRESS_D, DESIRED_ANALOG_DELTA_FOR_CALL ) )
#else
  if(pad->press & PAD_D)
#endif  
  {
     SetFunc(work, MemoryCallMode);
     return SELECTING;
  }

  if( pad->press & ( PAD_CANCEL | PAD_SEL ) )
  {
    //codecSetLight(0);
    codecLayoutTuneLight(0);
    return SELECT_CANCEL;
  }
#if defined(BP_VITA)
  else if( ( pad->press & PAD_OK )
     || GV_direction_pressed_with_threshold( pad,  PAD_PRESS_U, DESIRED_ANALOG_DELTA_FOR_CALL ) )
#else
  else if( pad->press & ( PAD_OK | PAD_U ) )
#endif
  {
        //codecSetLight(0);
        codecLayoutTuneLight(0);
        return SELECT_SEND;
  }
  else if( pad->status & ( PAD_L | PAD_R ) )
  {
     int dir = 0;

    stat |= CODEC_LIGHT_TUNING;
    if( pad->status & PAD_L )
    {
      dir = -1;
      stat |= CODEC_LIGHT_LEFT;
    }
    else
    {
      dir = 1;
      stat |= CODEC_LIGHT_RIGHT;
    }
    if( si->prev_dir == dir )
    {
      if( si->repeat_count < SELECT_FIRST_REPEAT )
      {
	      ;
      }
      else if( dir != 0 )
      {
	      CodecSE(CODEC_SE_TUNING);
	      work->freq += dir;
      }
      si->repeat_count ++;
    }
    else 
    {
      CodecSE(CODEC_SE_TUNING);
      work->freq += dir;
      si->prev_dir = dir;
      si->repeat_count = 0;
    }

#if BP_TGS_DEMO()
    if( work->freq == 14096 )
       work->freq += dir;
#endif

    if( work->freq > FREQ_MAX )
    {
      work->freq = FREQ_MIN;
    }
    else if( work->freq < FREQ_MIN )
    {
      work->freq = FREQ_MAX;
    }
  } 
  else
  {
    si->prev_dir = 0;
    si->repeat_count = 0;
  }
  codecLayoutTuneLight(stat);
  
  return SELECTING;
}

/* 無線機データロード開始 */

static void CodecLoad( Work *work, int code )
{
  int size;
  int start;
  void *buffer;
  int codecTop;
  int codecOffset;
  
  DBG("CodecLoad()  work = %p\n", work);
  last_freq = work->freq;

  // LARGE
  size = ( ( ( u_int )( code & 0xFF000000 ) >> 24 ) + 1 ) * FS_SECTOR_SIZE * 4;

  start = ( ( code & 0x00FFFFFF ) << 4 ) / FS_SECTOR_SIZE;
  
#if 0 //BP_PS2
  DBG("size = %d\n");
  buffer = codecMalloc( size );		// NEED TO RETOUCH !!!
  ASSERT( buffer != NULL );
#else
  size *= 2;
  buffer = malloc( size );
#endif
  
  work->param.data_buffer = buffer;
  codecTop = ( ( code & 0x00FFFFFF ) << 4 );
  codecOffset = codecTop % FS_SECTOR_SIZE;
  work->param.data = ( char * )buffer + codecOffset;
  FS_LoadFileRequest( FS_FILE_CODEC, start, size, buffer, codecTop );
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
  return BP_ReferThreadStatus( work->param.id, NULL );
}

/* 解釈タスクスタック,読み込んだデータの解放 */

static void CodecScriptEnd( Work *work )
{
  DBG("CodecScriptEnd()\n");
  DBG("[1]\n");
  codecFree( work->param.stack );       // NEED TO RETOUCH !!
  DBG("[2]\n");

#if 0 //BP_PS2
  codecFree( work->param.data_buffer ); // NEED TO RETOUCH !!
#else
  free(work->param.data_buffer);
#endif

  DBG("[3]\n");
}

/* "CALL"シグナル描画 */
static void DrawCall( Work *work )
{
  /* 他のモード(携帯,ノード,画像転送端末など)では、処理を行わない */
  if(GV_PauseLevel & ( GV_PAUSE_STOP | GV_PAUSE_DEBUG ) ) return;

  switch(work->call_step)
    {
    case CALL_STEP_WAIT:
      /* 待機中は何もしない */
      break;
    case CALL_STEP_INIT:
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_INIT\n");
#endif

      /* 呼び出し音は回数制限つき(カウンタをリセット) */
      work->alarm_cnt = 0;

      /* アイコン表示無しならば、SE のみのモードで実行 */
      if(work->icon_disable)
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_call) printf("Call ICON Disable.\n");
#endif
	  if(work->l2d_handle >= 0)
	    {
	      /* 表示中であれば強制的に消す */
	      L2D_BreakAction(work->l2d_handle);
	      L2D_ReleaseLayout(work->l2d_handle);
	      work->l2d_handle = -1;
	    }
	  work->call_step = CALL_STEP_SOUNDONLY;
	  work->calling_time = 0;
	  break;
	}
#ifdef DEBUG_MODE
      if(work->alarm_disable && _DEBUG_codec_call)
	printf("Call Alarm Disable\n");
#endif

      if(work->l2d_handle < 0) 
	{
	  work->l2d_handle = L2D_LoadLayout2(0x003311ec  /* "call" */,
					     4, 0, 0,
					     (GV_PAUSE_PAUSE|GV_PAUSE_STOP));
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_call)
	    printf("work->l2d_handle = %d\n", work->l2d_handle);
#endif
	  /* ハンドルが獲得できない場合、次のフレームに再度取得を試みる。 */
	  if(work->l2d_handle < 0) break;

	  // ASSERT(work->l2d_handle >= 0);
	}
      else
	if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	  L2D_BreakAction(work->l2d_handle);

      /* 開きアクション */
#ifdef AUX_CALL_ENABLE
      {
	int stat = L2D_EvokeAction(work->l2d_handle,
				   work->call_act_open[ work->call_mode ]);
#ifdef DEBUG_MODE
	if(_DEBUG_codec_call) printf("stat = %d\n", stat);
#endif
	ASSERT(L2D_STAT_ACK == stat);
      }
#else
      {
	int stat = L2D_EvokeAction(work->l2d_handle, work->call_act_open);
#ifdef DEBUG_MODE
	if(_DEBUG_codec_call) printf("stat = %d\n", stat);
#endif
	ASSERT(L2D_STAT_ACK == stat);
      }
#endif
      {
	void * parts;
	
#ifdef DEBUG_MODE
	if(_DEBUG_codec_call) printf("NULL-1 is shown.\n");
#endif
	parts = L2D_GetParts(work->l2d_handle, 0x00773672  /* "NULL-1" */);
	ASSERT(parts != NULL);
	if(NULL != parts)
	  {
	    int code = 0x00927bc4  /* "default" */;
	    L2D_MorfObject(parts, code, code, 1.0F);
	  }

#ifdef DEBUG_MODE
	if(_DEBUG_codec_call) printf("ROOT is shown.\n");
#endif
	parts = L2D_GetParts(work->l2d_handle, 0x002a4634  /* "ROOT" */);
	ASSERT(parts != NULL);
	if(NULL != parts)
	  {
	    int code = 0x00927bc4  /* "default" */;
	    L2D_MorfObject(parts, code, code, 1.0F);
	  }
      }

      work->call_step = CALL_STEP_START;
      work->calling_time = 0;
      
      break;
    case CALL_STEP_START:   /* 呼び出し開始 */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_START\n");
#endif
      work->calling_time -= TIME_BASE;
      if(work->calling_time < 0)
	{
	  work->calling_time = CALLING_TIME;
	  if((work->alarm_cnt < CALL_ALARM_MAX) && !work->alarm_disable)
	    {
	      CodecSE(CODEC_SE_CALL);
	      work->alarm_cnt++;
	    }
	}
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      L2D_EvokeAction(work->l2d_handle, work->call_act_idle);
      work->call_step = CALL_STEP_CALLING;
      break;
      
    case CALL_STEP_CALLING: /* 呼び出し中 */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_CALLING\n"); 
#endif
      work->calling_time -= TIME_BASE;
      if(work->calling_time < 0)
	{
	  work->calling_time = CALLING_TIME;
	  if((work->alarm_cnt < CALL_ALARM_MAX) && !work->alarm_disable)
	    {
	      CodecSE(CODEC_SE_CALL);
	      work->alarm_cnt++;
	    }
	}
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      
      /* 終ったらループ */
      L2D_EvokeAction(work->l2d_handle, work->call_act_idle);
      break;

    case CALL_STEP_TIMEOUT:  /* 呼び出し終了 */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_TIMEOUT\n");
#endif
      if(work->l2d_handle >= 0)
	{
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	    L2D_BreakAction(work->l2d_handle);
#ifdef AUX_CALL_ENABLE
	  L2D_EvokeAction(work->l2d_handle,
			  work->call_act_close[ work->call_mode ]);
#else
	  L2D_EvokeAction(work->l2d_handle, work->call_act_close);
#endif /* AUX_CALL_ENABLE */
	}
      work->call_step = CALL_STEP_FINISH;
      break;

    case CALL_STEP_FINISH:  /* CALL サイン表示終了 */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_FINISH\n");
#endif
      if(work->l2d_handle >= 0)
	{
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
	  L2D_ReleaseLayout(work->l2d_handle);
	}
      work->l2d_handle = -1;
      work->call_step = CALL_STEP_WAIT;
      break;
    case CALL_STEP_BREAK:  /* CALL サイン中断 */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_BREAK\n");
#endif
      /* 再生中のアクションがあれば中断 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_ReleaseLayout(work->l2d_handle);
      work->l2d_handle = -1;
      work->call_step = CALL_STEP_WAIT;
      break;
    case CALL_STEP_SOUNDONLY: /* CALL の SE のみリピート */
#ifdef DEBUG_MODE
      if(_DEBUG_codec_call) printf("CALL_STEP_SOUNDONLY\n");
#endif
      work->calling_time -= TIME_BASE;
      if(work->calling_time < 0)
	{
	  work->calling_time = CALLING_TIME;
	  if((work->alarm_cnt < CALL_ALARM_MAX) && !work->alarm_disable)
	    {
	      CodecSE(CODEC_SE_CALL);
	      work->alarm_cnt++;
	    }
	}
      break;
    }
}

/* 画面フェードアウトセット */

static void FadeOutSet( Work *work, int count )
{
  work->count = count;
  if(NULL != (bg_work = NewCodecIntoDemo(CODEC_DEMO_IN, work->fade_mode)))
    GV_SetActorChild(work, bg_work);
#ifndef KP_WINDOWS
  DG_UnDrawFrameCount = 0;  /* 必ずリセットする */
#else
  DG_SetUnDrawFrameCount(0);	/* 必ずリセットする */
#endif
  /* NewCodecFadeInOut(1, 0, 0, 0, 1, 0, FADE_TIME, -1, NULL); */
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
  DBG("CodecModeInit()\n");
  /* DG_Chanl( DG_CHANL_MAIN )->flag = 0; */
  /* 描画実験用。実際にはチャネル4 は立ててはいけない。
     立てると、描画順の都合上、他のチャネルの画面が表示されなくなる。 */
  /* DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 1; */
  DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 0;  /* 明示的に */

  /* 無線モードの間だけ、チャネル 4 をハイレゾリューション化する */
  DG_Chanl( DG_CHANL_MENU )->high_reso = 1;
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
			 126 * height / (DRAW_HEIGHT / 2) );
  }

//  CODEC_DrawMessage( &work->panel, "" );
  CODEC_HideMesgPanel( &work->panel );
  CODEC_DrawMessage( &work->panel, NULL);

}

static void CodecModeEnd( Work *work )
{
  /*
  DG_Chanl( DG_CHANL_MAIN  )->flag = 1;
  */
  DG_Chanl( DG_CHANL_MENU )->bg_clear_flag = 0;

  DG_Chanl( DG_CHANL_MENU )->high_reso = 0;

  DG_SetPrivilegeMode(0);
  // GM_MenuStatus = work->old_menu_flags;
  GM_PopMenuStatus();
  DG_FogColorMaster = work->old_fog;  /* Fog 値を復帰 */
  DG_FogParam1 = work->old_fog_param1;
  DG_FogParam2 = work->old_fog_param2;
#ifndef PSX2
  DG_FogParam = work->old_fog_param ;
#endif

  /*
    GM_ResetMenuStatus(MENU_WEAPON_OFF |
    MENU_ITEM_OFF   |
    MENU_RADAR_OFF  |
    MENU_GAGE_OFF);
  */


#ifdef _DEBUG_
  {
    long64 size;

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

  BP_ConsoleScreenSaverResume();        //BP JG - screen saver active again.

}

/* 解釈タスクを起こす */

static void WakeupItp( Work *work, int res )
{
	work->param.result = res;

	// 起こす前にリクエストをクリア
	work->param.request = CODEC_REQ_NOREQ;
	BP_WakeupThread( work->param.id );
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
    STATE_FRAME_EXIT,     /* 枠消去アクション */
    STATE_BG_RECOV_START, /* 背景復帰処理開始指示エントリ       */
    STATE_BG_RECOV,       /* 背景サイズ復帰                     */
    STATE_BG_RECOV_END,   /* 背景サイズ復帰終了                 */
    STATE_ERASE_FRAME,    /* 枠、背景消去、ゲーム画面再表示開始 */
    STATE_MEM_RECOV,
    STATE_MEM_RECOV_WAIT, /* メモリ復帰待ち中                   */
    STATE_MEM_RECOV_NEXT,
    STATE_FADEIN_WAIT,    /* フェードイン終了待ち               */
    STATE_EXEC_PROC       /* 無線処理終了、終了プロセス実行     */
  };

  switch(work->status)
    {
    case STATE_INIT:           /* 初期化     */
      DBG("CodecEndWait(): STATE_INIT\n");
      // codecDispArrow(0);                 /* 送り矢印消去 */
      codecWaiting(-1);
      last_responsed_freq = last_freq;   /* 最終応答周波数を更新 */

      if((work->type == TYPE_NORMAL) && work->finish_id)
	{
	  // work->status = STATE_BG_RECOV_START;
	  codecLayoutExit();
	  work->status = STATE_FRAME_EXIT;
	  work->finish_id = 0;
	  break;
	}
      /* アクションあり閉じもしくは続きの無線コールがあるならば、
	 フェードアウトせずに枠を閉じる。 */
      if(IsEndCloseAction(work) || (NULL != work->reserve_list))
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
      CodecSE(CODEC_SE_WNDCLOSE);
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
	if((work->face_work[0] != NULL) ||
	   (work->face_work[1] != NULL) || ret) break;
      }
      GV_CallChildSignalFunc(work, CDC_SIGNAL_NOISE_KILL, 0);

      DBG("<1>\n");
      if(!IsEndFade(work) || (NULL != work->reserve_list))
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

      /* 予約された連続コールがあるならば、
	 DIRECT, CALL, DEMO のいずれでも、次の無線コールを発生させる */
      if(NULL != work->reserve_list)
	{
	  GCL_SetArgTop(work->reserve_list);
	  work->reserve_freq = GCL_GetNextInt();
	  work->reserve_code = GCL_GetNextInt();
	  work->reserve_list = GCL_NextStr();
	  work->cont_call = 1;
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
      if(!IsEndCloseAction(work))
	{
	  codecLayoutDisp(0);
	  work->status = STATE_BG_RECOV_START;
	  break;
	}
      codecLayoutExit();
      work->status = STATE_FRAME_EXIT;

    case STATE_FRAME_EXIT:
      if(1 == codecExitStatus()) break;
      work->status = STATE_BG_RECOV_START;

    case STATE_BG_RECOV_START:
      DBG("CodecEndWait(): STATE_BG_RECOV_START\n");
      CODEC_CloseMesgPanel( &work->panel );
      work->status = STATE_BG_RECOV;
      CodecBG_Recover(bg_work, 0);
      /* break が無いのは bug ではない。
	 (設定を行ったのと同フレーム中に以降の処理に入るため) */

    case STATE_BG_RECOV:      /* 背景サイズ復帰 */
      DBG("CodecEndWait(): STATE_BG_RECOV\n");
      if(!IsBGrecovered()) break;
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
      work->status = STATE_MEM_RECOV_WAIT;
#ifndef KP_WINDOWS
      DG_UnDrawFrameCount = 4;
#else
      DG_SetUnDrawFrameCount(4);
#endif
      break;
    case STATE_MEM_RECOV_WAIT:
    
      /* 画面の再表示を指示する前に、パケット領域を復帰させておく */
      if(codecMemRecov()) break;  /* 遅延開放が終了するまで繰り返す */

      work->status = STATE_MEM_RECOV_NEXT;
      break;
    case STATE_MEM_RECOV_NEXT:

      DBG("STATE MEM RECOV NEXT\n");
      codecRecoveryScreen(); 
#ifndef KP_WINDOWS
      DG_UnDrawFrameCount = 0;
#else
      DG_SetUnDrawFrameCount(0);
#endif
      /*
	どうでもいいけど「宇宙の騎士テッカマン」には
	「アンドロー梅田」ってのがいたな。
      */
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
      printf("proc_id = %d\n", work->proc_id);
      if(work->proc_id != 0)
	{
	  int proc_id = work->proc_id;
	  work->proc_id = 0;
	  GCL_ExecProc(proc_id, NULL);
	  printf("Next proc_id = %d\n", work->proc_id);
	}

      /* 演説モードであれば、ch1 の状態を復帰 */
      if(work->speech_mode && work->speach_ch1)
	GM_StreamSdPauseControl(1, 0);

      printf("CodecEndWait(): finished\n");
      break;
    }
}

      

static void CodecDoScript( Work *work )
{
  /* 無線スクリプトを別タスクで解釈し、その要求から画面を作成する */
  enum {
    STATE_INIT,
    STATE_REQ_WAIT,
    STATE_MESG_DISP,
    STATE_SKIP_SCRIPT,
    STATE_KEY_WAIT,
    STATE_VOX_PLAYING,
    STATE_FACE_LOADING,
    STATE_FACE_SET,
    STATE_FACE_SETTING,
    STATE_HAIR_SET,
    STATE_OBJECT_SET,
    STATE_DEF_MTN_SET,
    STATE_CALL_WAIT,
    STATE_OPEN_FADE,
    STATE_FRAME_WAIT,
    STATE_DISP_CTRL,
    STATE_FADE_WAIT,
    STATE_SET_PLAYER,
    STATE_MIND_MASK,
    STATE_SELECT,
    STATE_SELECTING,
    STATE_SAVE_MODE,
    STATE_CONT_CALL,
    STATE_MOVIE_START,
    STATE_MOVIE_PLAYING,
    STATE_MOVIE_CANCEL,
    STATE_SET_FREQ,
    STATE_TIME_WAIT,
    STATE_EARTH_Q,
    STATE_VIBRATION,
    STATE_VIBPLAY,
    STATE_BUG_FACE,
    STATE_NOISE,
    STATE_BUGEYES,
    STATE_FINISH
  };

  DrawCodecMode( work );

  switch( work->status )
    {
    case STATE_INIT:
      DBG("CodecDoScript(): STATE_INIT\n");
      if( work->param.request != CODEC_REQ_SETUP )
	{
	  // codecTalkPTT(0);
	  if(!work->speech_mode)
	    {
	      /* 心の声ドライバ起動 */
	      GV_SetActorChild(work, NewCodecMindVoice());
	      work->call_block = 1;  /* 心の声ドライバ動作中は、
					呼出/応答不可とする */
	      printf("***************************************\n"
		     "**      Mind Voice Driver Start      **\n"
		     "***************************************\n");		     
	    }
	  else
	    {
	      work->call_block = 0;
	      printf("++++++++++++++++++++++++++++++++++\n"
		     "++         No Mind Voice        ++\n"
		     "++++++++++++++++++++++++++++++++++\n");		     
	    }



	  codecSetMaterAnimDisable(0);
	  work->face_bug_status[0] = 0;
	  work->face_bug_status[1] = 0;
	  work->waiting_icon = 0;
	  work->movie_start = 0;
	  work->player_name = -1;
	  work->vox_playing = 0;
	  work->mov_playing = 0;
	  work->voice_cancel = 0;
	  work->pre_cap_status = 0;
	  work->mind_mask = 0;
	  work->save_mode = 0;
	  work->disp_enable = 0;
	  work->pre_enable = 0;
	  work->start_mtn = 0;

	  work->cont_call = 0;   /* 継続コールは無しの状態に初期化 */

	  work->status = STATE_REQ_WAIT;
	  codecWaiting(-1);
	}
      break;
    case STATE_REQ_WAIT:
      DBG("CodecDoScript(): STATE_REQ_WAIT\n");
      switch( work->param.request )
	{
	case CODEC_REQ_MESG_WAIT:
	  DBG("\tCODEC_REQ_MESG_WAIT\n");

	  /* 自分が話している場合はデフォルト目パチを禁止する */
	  {
	    int name = work->param.req[0];
	    if(work->voice_cancel) name = -1;
	    codecEyeAnimationSwitch(0, name);
	    codecEyeAnimationSwitch(1, name);
	  }

	  if(work->param.req[0] == work->player_name)
	    {
	      CDC_DisableMindVoice();
	      /* プレイヤが発言している場合は PTT を点灯させる */
	      // codecTalkPTT(1);
	    }
	  else
	    {
	      if(!work->mind_mask) CDC_EnableMindVoice();
	      /* プレイヤ以外の発言であれば PTT を消灯する */
	      // codecTalkPTT(0);
	    }

	  CODEC_HideMesgPanel( &work->panel );

	  CODEC_DrawMessage( &work->panel, NULL);
	  
	  work->status = STATE_MESG_DISP;
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
	  work->main_vox_handle =
	    GM_VoxStream(work->param.req[0], GM_STREAM_CHANNEL_0);

	  /* ストリーミング開始時にノイズを載せることで、
	     切替え時のギャップを隠す */
	  {
	    void * noise_work;
	    int i;

	    for(i = 0; i < 2; i++)
	      {
		noise_work = NewCodecNoise(i, CDC_NOISE_GHOST, 40);
		if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	      }
	  }

	  printf("Execute GM_VoxStream(): handle = %d\n",
		 work->main_vox_handle);
	  work->status = STATE_VOX_PLAYING;
	  work->pre_cap_status = 0;
	  work->vox_playing = 1;
	  // codecWaiting(0);
	  break;
	case CODEC_REQ_FACE_LOAD:
	  DBG("\tCODEC_REQ_FACE_LOAD\n");
	  {
	    if(cdbios_get_status()) break;
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

	case CODEC_REQ_OBJECT_SET: /* 付属物オブジェクトの付加 */
	  DBG("\tCODEC_REQ_OBJECT_SET\n");
	  work->status = STATE_OBJECT_SET;
	  break;
	  
	case CODEC_REQ_DEF_MTN_SET:
	  DBG("\tCODEC_REQ_DEF_MTN_SET\n");
	  work->status = STATE_DEF_MTN_SET;
	  break;
	case CODEC_REQ_DISP_CTRL:
	  DBG("\tCODEC_REQ_DISP_CTRL\n");
	  work->status = STATE_DISP_CTRL;
	  break;

	case CODEC_REQ_SET_PLAYER:
	  DBG("\tCODEC_REQ_SET_PLAYER\n");
	  work->status = STATE_SET_PLAYER;
	  break;
	case CODEC_REQ_MIND_MASK:
	  work->status = STATE_MIND_MASK;
	  break;
	case CODEC_REQ_SELECT:
	  work->status = STATE_SELECT;
	  break;
	case CODEC_REQ_SAVE_MODE:
	  work->save_mode = 1;
	  work->save_step = 0;
	  CDC_DisableMindVoice();
	  work->status = STATE_SAVE_MODE;
	  break;

	case CODEC_REQ_CONT_CALL:     /* 継続コール予約 */
	  work->status = STATE_CONT_CALL;
	  break;
	  
	case CODEC_REQ_MOVIE_START:

	  DBG("\tCODEC_REQ_MOVIE_START\n");
	  /* 既に音声がキャンセル済であれば、新たな再生は行わない */
	  if(work->voice_cancel)
	    {
	      work->status = STATE_SKIP_SCRIPT;
	      break;
	    }

	  // codecLayoutMovieSwitch(1);

	  /* IPU-Movie 表示タスク起動 */
	  GV_SetActorChild(work,
			   NewCodecMovie(codecGetObject(CDCOBJ_MovieRect)));
	  work->movie_start = 1;
	  work->main_ipu_handle =
	    GM_IpuStream(work->param.req[0], GM_STREAM_CHANNEL_0);

	  work->status = STATE_MOVIE_START;
	  work->pre_cap_status = 0;
	  work->mov_playing = 1;
	  // codecWaiting(0);
	  break;
	case CODEC_REQ_FREQ_SET:
	  work->status = STATE_SET_FREQ;
	  break;
	case CODEC_REQ_TIME_WAIT:
	  work->status = STATE_TIME_WAIT;
	  work->wait_cnt = work->param.req[0];
	  break;
	case CODEC_REQ_EARTH_Q:
	  work->status = STATE_EARTH_Q;
	  break;
	case CODEC_REQ_VIBRATION:
	  work->status = STATE_VIBRATION;
	  break;
	case CODEC_REQ_VIBPLAY:
	  work->status = STATE_VIBPLAY;
	  break;
	case CODEC_REQ_BUG_FACE:
	  work->status = STATE_BUG_FACE;
	  break;

	case CODEC_REQ_NOISE:
	  work->status = STATE_NOISE;
	  break;
	case CODEC_REQ_BUGEYES:
	  work->status = STATE_BUGEYES;
	  break;
	}
      break;
    case STATE_MESG_DISP:
      {
         char* message = (char *)work->param.req[1];
         message = BP_GCL_LOOKUP_NEW_FONT_STRING(message);

         CODEC_DrawMessage( &work->panel, message);
         CODEC_ShowMesgPanel( &work->panel );
         work->status = STATE_KEY_WAIT;
         if(work->vox_playing) work->status = STATE_VOX_PLAYING;
         if(work->mov_playing) work->status = STATE_MOVIE_PLAYING;
         if(work->status == STATE_KEY_WAIT)
         {
            work->waiting_icon = 1;
         }
         
         // codecWaiting(wait);
         
         if(!work->vox_playing && !work->mov_playing) 
            CDC_KillMindVoice();
      }
      
      break;

    case STATE_SKIP_SCRIPT:
      WakeupItp(work, 0);
      work->status = STATE_REQ_WAIT;
      break;

    case STATE_KEY_WAIT:
      if(work->pad->press & (PAD_X | PAD_Y))
	{
	  void * noise_work;

	  work->autoskip ^= 1;

	  /* 早送りノイズを載せる */
	  if(work->autoskip)
	    {
	      /* 一旦全てのノイズ関連キャラを殺す(サンドノイズ以外) */
	      GV_CallChildSignalFunc(work, CDC_SIGNAL_KILL_EXIST_SAND, 0);

	      noise_work = NewCodecNoise(0, CDC_NOISE_VFAST, 0);
	      if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	      noise_work = NewCodecNoise(1, CDC_NOISE_VFAST, 0);
	      if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	    }
	  else
	    // 早送りノイズのみを殺す
	    GV_CallChildSignalFunc(work, CDC_SIGNAL_VFAST_KILL, 0);
	    // GV_CallChildSignalFunc(work, CDC_SIGNAL_NOISE_KILL, 0);
	}
      if( (work->pad->press & ( PAD_OK | PAD_CANCEL )) || work->autoskip )
      {
        WakeupItp( work, 0 );
        work->status = STATE_REQ_WAIT;
        // CODEC_DrawMessage( &work->panel, NULL );
        codecWaiting(0);
      }

      // AS MCampbell - Add support for automatically skipping empty codec strings. Empty codec strings in cases where
      // existing lines needed to be removed.
      // Fixes: MGSTWO-3300
      {
         char* pMessage = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)work->param.req[1]);
         if (pMessage && !strlen(pMessage))
         {
            WakeupItp( work, 0 );
            work->status = STATE_REQ_WAIT;
            codecWaiting(0);
         }
      }
      break;
    case STATE_FACE_LOADING:   /* 顔モデル読み込み中 */
      work->call_cnt += TIMEBASE;
      if(!CDC_SyncCodecDataLoad())
	{
	  CDC_InitLoadData();
	  work->status = STATE_CALL_WAIT;
	}
      break;
    case STATE_FACE_SET:
#ifdef EFFECT_NOISE
  /* ノイズを載せる */
	{
	  void * noise_work;

	  /* フェード中の顔切替えの場合はノイズを載せない */
	  if(!(work->fade_mode && !work->act_open) ||
	     (NULL != work->face_work[work->param.req[0]]))
	    {
	      noise_work =
		NewCodecNoise(work->param.req[0], CDC_NOISE_SAND, TIME_BASE * 4);
	      if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	    }

	  /* バグ顔を消す */
	  GV_CallChildSignalFunc(work, SIGNAL_BUGFACE_KILL, 0);
	  work->status = STATE_FACE_SETTING;
	}
#endif
	break;
    case STATE_FACE_SETTING:
      {
	void * face_work;
	int snd;

	DBG("side[%d]  EVM-ID[%d]\n", work->param.req[0], work->param.req[1]);

	snd = (!(work->param.req[0]))
	  ? CODEC_SE_CAMERA_CHG_L : CODEC_SE_CAMERA_CHG_R;

	if((!work->fade_mode && work->act_open) ||
	   (NULL != work->face_work[work->param.req[0]]))
	  CodecSE(snd);

	face_work = NewCodecFace(0,
				 work->dummy_bone,
				 work->param.req[0],
				 work->param.req[1],
				 work->param.req[2]);

	work->face_bug_status[work->param.req[0]] = 0;  /* バグり顔じゃない */
	
	/* 顔モデルのワークを登録する */
	SetFaceWork(work->param.req[0], face_work);
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;

    case STATE_HAIR_SET:
      DBG("CodecDoScript(): STATE_HAIR_SET\n");
      {
	void * model = NULL;
	void * bound = NULL;
	
	DBG("model ID: 0x%08x\n", work->param.req[1]);
	model = CDC_LoadEvmDef(work->param.req[1]);
	bound = (cdcHair_bound < 0)
	  ? NULL : CDC_GetFileEntry(cdcHair_bound, 'k');
	
	if(!work->face_bug_status[work->param.req[0]])
	  codecSetupHair(work->param.req[0], /* side       */
			 model,              /* model      */
			 work->param.req[2], /* sample_num */
			 bound,              /* bound      */
			 cdcHair_hit_param,  /* hit_param  */
			 cdcHair_ambient,    /* ambient    */
			 work->param.req[3]);/* nowind     */
	else
	  codecSetupBugHair(work->param.req[0], /* side       */
			    model,              /* model      */
			    work->param.req[2], /* sample_num */
			    bound,              /* bound      */
			    cdcHair_hit_param,  /* hit_param  */
			    cdcHair_ambient,    /* ambient    */
			    work->param.req[3]);/* nowind     */

	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;

    case STATE_OBJECT_SET:
      DBG("CodecDoScript(): STATE_OBJECT_SET\n");

      printf("object model = 0x%08x\n", work->param.req[1]);

#if 1
      /*
       * 顔モデルに付属品を付ける(主に使用目的はオタコンの眼鏡だったりする)
       */
      cdcFaceSetObject(work->param.req[0],  /* 表示サイド       */
		       work->param.req[1],  /* 表示モデル       */
		       work->param.req[2],  /* モーション識別名 */
		       work->param.req[3]); /* 基準関節番号     */
#endif 
      WakeupItp(work, 0);
      work->status = STATE_REQ_WAIT;
      break;

    case STATE_DEF_MTN_SET:
      DBG("-a-\n");
      work->defmtn_id[work->param.req[0]] = work->param.req[1];
      codecSetDefaultMotion(work->param.req[0], work->param.req[1]);
      {
	int val = (work->param.req[0] << 24) | (work->param.req[1] & 0xffffff);
	GV_CallChildSignalFunc(work, SIGNAL_BUGFACE_DEFMTN, val);
      }
      WakeupItp(work, 0);
      work->status = STATE_REQ_WAIT;
      break;
    case STATE_CALL_WAIT:
      DBG("CodecDoScript(): STATE_CALL_WAIT\n");
      // SetDrawCall(work, 1);
      work->call_cnt += TIMEBASE;
      if(work->call_cnt >= CALL_MIN_WAIT)
	{
	  codecCallAction(0);
	  /* アクション無し指定がされていれば、開きアクション無しで開始する */
	  if(!work->fade_mode) /* フェード無しの場合 */
	    {
	      /* アクション無し(開き状態) */
	      // if(!work->act_open) codecOpenFrame(1);
	    }
	  codecLayoutDisp(1);   /* まだ表示されていない場合のため表示を促す */
	  if(work->fade_mode && work->act_open)
	    {
	      setup_fade_mask(work);
	      work->status = STATE_OPEN_FADE;
	    }
	  else
	    {
	      work->status = STATE_FRAME_WAIT;
	    }

	}
      break;
    case STATE_OPEN_FADE:
      DBG("CodecDoScript(): STATE_OPEN_FADE\n");
      /* フェード有り、アクション有りの場合は,
	 装飾枠を開く前にフェードを完了する */
      if(!fade_in_wait(work))
	{
	  SPR_Destroy_2D_Object(work->fade_mask);
	  work->fade_mask = NULL;
	  work->status = STATE_FRAME_WAIT;
	  work->start_mtn = 0;
	  // if(!work->act_open) codecOpenFrame(1);

	  // GM_StreamSdPauseControl(0, 1);
	}
      break;
      
    case STATE_FRAME_WAIT:     /* 装飾枠アクションの終了待ち */
      DBG("CodecDoScript(): STATE_FRAME_WAIT\n");
      if(!codecActionStatus())
	{
	  if((work->type == TYPE_NORMAL) ||
	     (!work->fade_mode && work->act_open))
	    CodecSE(CODEC_SE_CONNECT);
	  WakeupItp(work, 0);
	  work->status = STATE_REQ_WAIT;
	}
      break;
    case STATE_VOX_PLAYING:
      DBG("CodecDoScript(): STATE_VOX_PLAYING\n");
      {
	int status;

	status = GM_GetCodecCapStatus();    /* 字幕のタイミング情報を得る */
	/* キャンセルをチェックする */
	if(work->pad->press & (PAD_A|PAD_B|PAD_X|PAD_Y))
	  {
	    GM_StreamStop(work->main_vox_handle);   /* 音声を停止する */
	    cdcFaceVoiceCancel(0);
	    cdcFaceVoiceCancel(1);
	    CDC_KillMindVoice();
	    codecSetMaterAnimDisable(0);
	    work->vox_playing = 0;
	    work->autoskip = 0;
	    work->voice_cancel = 1;
	    if(!status) WakeupItp(work, 0);
	    /* CODEC_DrawMessage(&work->panel, NULL); */
	    work->status = STATE_REQ_WAIT;
	    break;
	  }
	if(status ^ work->pre_cap_status)
	  {
	    CODEC_DrawMessage(&work->panel, NULL);
	    CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
	    if(status)
	      {
		WakeupItp(work, 0);
		work->status = STATE_REQ_WAIT;
	      }
	  }
	work->pre_cap_status = status;
	{
	  int st_s;
	  if((st_s = GM_StreamStatus(work->main_vox_handle)) == GM_STREAM_STATE_END)
	  {
	      GM_StreamStop(work->main_vox_handle);
	      WakeupItp(work, 0);
	      CODEC_DrawMessage(&work->panel, NULL);
	      CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
	      work->vox_playing = 0;
	      work->status = STATE_REQ_WAIT;
	  }

     // AS MCampbell - Treat these codec streams as special cases. The timing was adjusted to compensate for the new 
     // audio and captions. Since the stream now lasts longer, terminate based on the new audio.
     // BugFix: MGSTWO-3268
     // BugFix: MGSTWO-3141
     {
        const char* pStreamName = GM_GetStreamName(work->main_vox_handle);
        if ( pStreamName && (strstr(pStreamName, "vc265061") || strstr(pStreamName, "vc091101")) )
        {
           int sd_channel = GM_GetSoundPlayBackChannel(work->main_vox_handle);

           // If the audio stream has started playing and it is done, force stop the codec stream.
           if (BP_GetDirectOutputStreamPosition(sd_channel) > 0)
           {
              if (sd_channel != -1 && BP_IsDirectOutputStreamDone(sd_channel))
              {
                 GM_StreamStop(work->main_vox_handle);
                 work->status = STATE_REQ_WAIT;
              }
           }
        }
     }

	  DBG("GM_StreamStatus() = %d\n", st_s);
	}
      }
      DBG("END: STATE_VOX_PLAYING\n");
      break;

    case STATE_DISP_CTRL:
      DBG("CodecDoScript(): STATE_DISP_CTRL\n");
      {
	if(work->disp_enable == work->param.req[0])
	  {
	    work->status = STATE_REQ_WAIT;
	    WakeupItp(work, 0);
	    break;
	  }
	  

	/* フェード開始が指定されていたら、
	   アクションの有無にかかわらずこのタイミングで枠を表示 */
	if(work->fade_mode)
	  {
	    if(NULL == work->fade_mask)
	      {
		setup_fade_mask(work);
		work->fade_param = 0;
		break;
	      }
	    codecLayoutDisp(1);
	    work->status = STATE_REQ_WAIT;
	    work->start_mtn = 0;
	  }
	else
	  {
	    work->status = STATE_REQ_WAIT;
	    WakeupItp(work, 0);
	  }
	work->disp_enable = work->param.req[0];
      }
      break;
    case STATE_FADE_WAIT:
      DBG("CodecDoScript(): STATE_FADE_WAIT\n");
      work->fade_param += ((128 + (FADE_TIME - 1)) / FADE_TIME);
      if(work->fade_param < 128)
#ifndef KP_WINDOWS
	work->fade_mask->head.alpha =
	  SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
	/* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
	work->fade_mask->head.alpha =
	  SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+1);
#endif
      else
	{
	  SPR_Destroy_2D_Object(work->fade_mask);
	  work->fade_mask = NULL;
	  work->status = STATE_REQ_WAIT;
	  WakeupItp(work, 0);
	}  
      break;
    case STATE_SET_PLAYER:
      work->player_name = work->param.req[0];
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;
    case STATE_MIND_MASK:
      work->mind_mask = (work->param.req[0]) ? 1 : 0;
      if(work->mind_mask)
	CDC_DisableMindVoice();
      else
	CDC_EnableMindVoice();
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;

    case STATE_SELECT:      /* 選択項目表示 */
      {
	work->sel_total = work->param.req[0];
	work->sel_num   = work->param.req[1];
	work->sel_list  = (char **)work->param.req[2];
	work->sel_pre = -1;

	if((work->sel_num < 0) || (work->sel_num >= work->sel_total))
	  work->sel_num = 0;

	CODEC_DrawMessage(&work->panel, NULL);
	CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01

	/* 項目選択中は早送りノイズなどを中止する */
	GV_CallChildSignalFunc(work, CDC_SIGNAL_VFAST_KILL, 0);
	work->autoskip = 0;     /* 自動早送りは中止         */
	  /* MindVoice がキャンセル済であれば、
	     再度起動する */
	if(work->voice_cancel)
	  {
	    GV_SetActorChild(work, NewCodecMindVoice());
	    work->call_block = 1;  /* 心の声ドライバ動作中は、
				      呼出/応答不可とする */
	  }
	work->voice_cancel = 0; /* キャンセルした事実も無し */
	codecErasePageIcon();
      }
      /* カメラ移動を禁止 */
      codecCameraControlSwitch(0, 0);
      codecCameraControlSwitch(1, 0);
      CDC_DisableMindVoice();      /* メニュー中は MindVoice 禁止 */
      work->status = STATE_SELECTING;
      break;
    case STATE_SELECTING:   /* 項目選択中   */

      if((work->pad->press & PAD_OK))
	{
	  CodecSE(SD_S_WIN01);
	  codecCameraControlSwitch(0, 1);
	  codecCameraControlSwitch(1, 1);
	  CDC_EnableMindVoice();
	  work->param.res[0] = work->sel_num;
	  work->status = STATE_REQ_WAIT;
	  WakeupItp(work, 0);
	  break;
	}
      if((work->pad->press & PAD_CANCEL))
	{
	  CodecSE(SD_S_V_CANS02);
	  codecCameraControlSwitch(0, 1);
	  codecCameraControlSwitch(1, 1);
	  CDC_EnableMindVoice();
	  work->status = STATE_REQ_WAIT;
	  WakeupItp(work, -1);
	  break;
	}

      if((work->pad->press & PAD_U))
	if(--work->sel_num < 0) work->sel_num += work->sel_total;

      if((work->pad->press & PAD_D))
	if(++work->sel_num >= work->sel_total)
	  work->sel_num -= work->sel_total;

      if(work->sel_pre != work->sel_num)
	{
	  int i, sel;

	  printf("Change Select: %d\n", work->sel_num);
	  CodecSE(SD_S_CUR01);
	  /*
	   * 選択項目が前回と変わったら,表示しなおす。
	   */
	  CODEC_DrawMessage(&(work->panel), NULL); /* 一旦消去 */
	  for(i = 0; i < work->sel_total; i++)
	    {
	      sel = (i == work->sel_num) ? 1 : 0;
	      CODEC_DrawSelect(&(work->panel), sel,
			       work->sel_total, i, BP_GCL_LOOKUP_NEW_FONT_STRING(work->sel_list[i]));
	      CODEC_ShowMesgPanel( &work->panel );  // Add K.Uehara 02/07/01
	    }
	  work->sel_pre = work->sel_num;
	}

      break;
    case STATE_SAVE_MODE:   /* セーブモード起動 */
      if(save_mode_waiting(work))
	{
	  CDC_EnableMindVoice(); /* セーブモードが終るまでは MindVoice 禁止 */
	  WakeupItp(work, work->save_return);
	  work->status = STATE_REQ_WAIT;
	}
      break;

    case STATE_CONT_CALL:   /* 継続コール予約 */

      work->reserve_freq = work->param.req[0];    /* 周波数     */
      work->reserve_code = work->param.req[1];    /* 実行コード */
      work->cont_call = 1;
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;
    case STATE_MOVIE_START:
      if(work->movie_start != 2) break;
      if(codecLayoutMovieStatus()) break;
      CDC_SetMovieDispStatus(1);  /* 表示開始 */
      work->status = STATE_MOVIE_PLAYING;
      break;
    case STATE_MOVIE_PLAYING:
      DBG("CodecDoScript(): STATE_MOVIE_PLAYING\n");
      {
	int status;
	int press;
	status = GM_GetCodecCapStatus();    /* 字幕のタイミング情報を得る */
	/* キャンセルをチェックする */
	press = work->pad->press;
	if(press & (PAD_A|PAD_B|PAD_X|PAD_Y))
	  {
	    GM_StreamStop(work->main_ipu_handle);   /* 音声を停止する */
	    CDC_KillMindVoice();
	    CDC_KillMovie();       /* IPU-Movie 再生タスク終了 */
	    // CDC_SetMovieDispStatus(0);
	    work->movie_start = 5;
	    codecSetMaterAnimDisable(0);
	    work->mov_playing = 0;
	    work->autoskip = 0;
	    work->voice_cancel = 1;
	    /* CODEC_DrawMessage(&work->panel, NULL); */
	    if(!status) WakeupItp(work, 0);
	    work->status = STATE_MOVIE_CANCEL;
	    break;
	  }
	if(status ^ work->pre_cap_status)
	  {
	    CODEC_DrawMessage(&work->panel, NULL);
	    CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
	    if(status)
	      {
		WakeupItp(work, 0);
		work->status = STATE_REQ_WAIT;
	      }
	  }
	work->pre_cap_status = status;
	{
	  int st_s;
	  if((st_s = GM_StreamStatus(work->main_ipu_handle)) == GM_STREAM_STATE_END)
	    {
	      // CDC_SetMovieDispStatus(0);
	      work->movie_start = 3;
	      GM_StreamStop(work->main_ipu_handle);
	      CDC_KillMovie();       /* IPU-Movie 再生タスク終了 */
	      WakeupItp(work, 0);
	      CODEC_DrawMessage(&work->panel, NULL);
	      CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
	      work->mov_playing = 0;
	      work->status = STATE_REQ_WAIT;
	    }
	  DBG("GM_StreamStatus() = %d\n", st_s);
	}
      }
      DBG("END: STATE_MOVIE_PLAYING\n");
      break;
    case STATE_MOVIE_CANCEL:
      if(codecLayoutMovieStatus()) break;  /* 終了中であれば何もしない */
      work->status = STATE_REQ_WAIT;
      break;
    case STATE_SET_FREQ:
      DBG("CodecDoScript(): STATE_SET_FREQ\n");
      
      work->freq = work->param.req[0];
      codecSetFreq(work->freq);
      last_freq = work->freq;   /* 最終アクセス周波数として設定 */

      /* 両サイドの画面にノイズを載せる */
      {
	int side;
	void * noise_work;

	for(side = 0; side < 2; side++)
	  {
	    noise_work = NewCodecNoise(side, CDC_NOISE_VFAST, 20);
	    if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	  }
      }
      work->status = STATE_REQ_WAIT;
      CodecSE(CODEC_SE_CONNECT);
      WakeupItp(work, 0);
      break;

      /*
       * 現時点では,以下の三つの命令はダミーとしておく。
       */
    case STATE_TIME_WAIT:
      work->wait_cnt -= TIME_BASE;
      if(work->wait_cnt <= 0)       /* 指定された時間だけ待つ */
	{
	  work->status = STATE_REQ_WAIT;  /* 時間が来たら次の命令を実行する */
	  WakeupItp(work, 0);
	}
      break;
    case STATE_EARTH_Q:
      codecEarthQuake(work->param.req[0],
		      work->param.req[1], work->param.req[2]);
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;
    case STATE_VIBRATION:
      if(work->param.req[0] < 0)
	GV_CallChildSignalFunc(work, CDC_SIGNAL_VIB_KILL, 0);
      else
	GV_SetActorChild(work, NewCodecVibration(work->param.req[0],
						 work->param.req[1]));
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;

    case STATE_VIBPLAY:
      {
	extern void	*NewPadVibrationRadio( int file );
	void * vib_work = NewPadVibrationRadio( work->param.req[0] );
	if(NULL != vib_work) GV_SetActorChild(work, vib_work);
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;
    case STATE_BUG_FACE:
      {
	void * face_work;
	int side, spk_id;
	int * models;
	int span_time, span_bias;

	side = work->param.req[0] >> 24;        /* サイドを抽出   */
	spk_id = work->param.req[0] & 0xffffff; /* 話者 ID を抽出 */
	models = (int *)(work->param.req[1]);     /* モデルリストのポインタ */
	span_time = work->param.req[2];         /* 最低間隔 */
	span_bias = work->param.req[3];         /* 上乗せ時間最大 */
	
	DBG("side[%d]  EVM-ID[%d]\n", side, models[0]);
	/*
	  face_work = NewCodecFace(0,
	  work->dummy_bone,
	  side,
	  models[0],
	  spk_id);
	*/
	face_work = NewBugFace(side,
			       work->dummy_bone,
			       spk_id,
			       span_time,
			       span_bias,
			       models);

	work->face_bug_status[side] = 1;  /* バグり顔 */

	GV_SetActorChild(work, face_work);

	/* 顔モデルのワークを登録する */
	SetFaceWork(side, BugFaceGetFaceWork(face_work));
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;
    case STATE_NOISE:
#ifdef EFFECT_NOISE

      if(work->param.req[1])
	{
	  void * noise_work;

	  noise_work = NewCodecNoise(work->param.req[0],
				     CDC_NOISE_SAND, work->param.req[2]);
	  /* ノイズを載せる */
	  if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	}
      else
	{
	  if(!work->autoskip)
	    GV_CallChildSignalFunc(work, CDC_SIGNAL_NOISE_KILL, 0);
	  else  /* 早送り中は早送りノイズ以外を殺す */
	    GV_CallChildSignalFunc(work, CDC_SIGNAL_KILL_EXIST_VFAST, 0);
	}
#endif
      work->status = STATE_REQ_WAIT;
      WakeupItp(work, 0);
      break;

    case STATE_BUGEYES:
      {
	int side, sw;

	side = work->param.req[0];
	sw   = work->param.req[1];
	codecBugEyesSwitch(side, sw);
	WakeupItp(work, 0);
	work->status = STATE_REQ_WAIT;
      }
      break;
    case STATE_FINISH:
      DBG("CodecDoScript(): STATE_FINISH\n");
      if(work->finish_cnt <= 0)
	{
	  if(work->movie_start != 0) break;
	  GV_CallChildSignalFunc(work, CDC_SIGNAL_VFAST_SE_STOP, 0);
	  CODEC_DrawMessage( &work->panel, NULL );
	  // codecTalkPTT(0);       /* PTT 消灯 */
	  CDC_KillMindVoice();   /* 心の声タスク終了         */
	  CDC_KillMovie();       /* IPU-Movie 再生タスク終了 */
	  SetFunc(work, CodecEndWait);
	}
      else
	work->finish_cnt -= TIME_BASE;
      break;
    }


  /* フェード開始であり、ストリームモーションが設定済であれば
     フェードを進行させる。 */
  if(work->fade_mode && !work->act_open && work->start_mtn)
    {
      work->fade_param += ((128 + (FADE_TIME - 1)) / FADE_TIME);
      if(work->fade_param < 128)
#ifndef KP_WINDOWS
	work->fade_mask->head.alpha = 
	  SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
	/* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
	work->fade_mask->head.alpha = 
	  SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+1);
#endif
      else
	{
	  SPR_Destroy_2D_Object(work->fade_mask);
	  work->fade_mask = NULL;
	  work->start_mtn = 0;
	  // GM_StreamSdPauseControl(0, 1);
	}
    }

  /* 顔を表示する条件が整っていたら,顔の表示を開始する */
  if(work->disp_enable && !work->pre_enable &&
     (codecGetMotionStatus(0) ||
      codecGetMotionStatus(1) || work->voice_cancel))
    {
      if(work->fade_mode && !work->act_open)
	{
	  /* 最初のモーションフレームが来たということなので、
	     一旦ストリームに PAUSE をかける */
	  work->start_mtn = 1;
	  // GM_StreamSdPauseControl(0, 0); /* ポーズ */
	}
      {
	void * noise_work;
	noise_work = NewIntervalNoise(0);
	if(NULL != noise_work) GV_SetActorChild(work, noise_work);
	noise_work = NewIntervalNoise(1);
	if(NULL != noise_work) GV_SetActorChild(work, noise_work);
      }
      
      if(work->act_open || !work->voice_cancel)
	{
	  codecOpenFrame(0);
	  if(work->act_open)
	    CodecSE(CODEC_SE_WNDOPEN);  /* ウィンドウを開くときは、
					   必ず SE を入れる 2001/09/03 */
	}
      else
	{
	  codecOpenFrame(1);
	}
      
      // codecFaceDisp(0, 1, !work->act_open);
      // codecFaceDisp(1, 1, !work->act_open);
      work->pre_enable = 1;
     }
  if(work->pre_enable && (work->pre_enable < 3)) work->pre_enable++;
  if(work->pre_enable == 3)
    {
      int fade = 0;
      if(work->fade_mode && !work->act_open) fade = 1;
      codecFaceDisp(0, 1, fade);
      codecFaceDisp(1, 1, fade);
      work->pre_enable = 4;
    }

  if(work->waiting_icon && codecIsKeyWaitOK())
    {
      codecWaiting(1);
      work->waiting_icon = 0;
    }
  
  /* 表示が開始されていて、なおかつムービーが開始されていたら、
     openMovie アクションを実行し、ムービー画面を表示開始する。 */
  switch(work->movie_start)
    {
    case 0: /* 何もしない */
      break;
    case 1: /* ムービー表示開始 */
      if(!work->disp_enable) break;
      work->movie_start = 2;
      codecLayoutMovieSwitch(1);  /* openMovie アクション実行 */
      break;
    case 2: /* ムービー表示中 */
      break;
    case 3: /* ムービー終了 */
      work->movie_start = 4;
      codecLayoutMovieSwitch(0);  /* closeMovie アクション実行 */
      break;
    case 4: /* ムービー終了中 */
      if(codecLayoutMovieStatus() == 0)
	{
	  CDC_SetMovieDispStatus(0);  /* ムービー表示終了 */
	  work->movie_start = 0;
	}
      break;
    case 5: /* ムービー強制終了 */
      codecLayoutMovieSwitch(2);
      CDC_SetMovieDispStatus(0);
      work->movie_start = 0;
      break;
    }
}

static void CodecMode( Work *work )
{
   extern void MENU_ForceClose();

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
    STATE_NO_RESPONSE_ERASE,
    STATE_FDIN_WAIT
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

    BP_ConsoleScreenSaverSuspend();        //BP JG - prevent the screen saver from activating during the next game sequence

    // Make sure the weapons and items menus are closed before starting the codec. This prevents a soft lock when the
    // user is pressing the touch pad and a codec automatically launches before the user releases.
#if defined(BP_VITA)
    MENU_ForceClose();
#endif

    /*
     * 予約されたコールとして入ってきた場合
     */
    if(work->cont_call)
      {
	work->freq = work->reserve_freq;  /* 周波数を予約されていたものに
					     切替える */
	work->code = work->reserve_code;  /* 予約されているコードを
					     呼び出しコードとして設定する */
	codecSetFreq(work->freq);  /* 表示を新しい周波数に変更する */
	work->status = STATE_LOAD;
	break;
      }

    if( work->type == TYPE_NORMAL ){
      work->status = STATE_SELECT;
      InitCodecSelect( work );
    } else {
      /* デモ */
      if(work->fade_mode && work->act_open)
	work->status = STATE_FDIN_WAIT;
      else
	work->status = STATE_LOAD;
    }
    codecSetFreq(work->freq);  /* 初期状態の表示周波数を設定する */
    /* codecLayoutDisp(1); */
    break;
  case STATE_SELECT:
    /* 周波数選択 */
    {
      int res;
      if(work->call_block)
	{
#ifdef DEBUG_MODE
	  printf("MindVoice is speaking.\n");
#endif
	  break;
	}
      res = DoCodecSelect( work );
      codecSetFreq(work->freq);



#ifdef DEBUG_MODE
      if(_DEBUG_aging)
	{
	  work->freq = 14112;
	  codecSetFreq(work->freq);
	  res = SELECT_SEND;
	}
#endif




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
	CodecSE(CODEC_SE_SEND);
	codecCallAction(1);
	if( ( code = CODEC_GetCode( work->freq ) ) < 0 ){
	  /* 該当するコードがなかった */
	  work->status = STATE_NO_RESPONSE;
	  work->call_cnt = 0;
	  break;
	}
      } else {
	code = work->code;
	if(work->cont_call)
	  {
	    if(!work->reserve_mode) CodecSE(CODEC_SE_SEND);
	  }
	else
	  if((work->type == TYPE_DIRECT) && work->act_open)
	    CodecSE(CODEC_SE_SEND);
      }
      // SetDrawCall(work, 1);
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
    // SetDrawCall( work, 0 );

    CodecScriptSetup( work );
    SetFunc( work, CodecDoScript );
    break;
  case STATE_NO_RESPONSE:
    work->call_cnt += TIMEBASE;
    // SetDrawCall( work, 1 );
    CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
    if(work->call_cnt >= CALL_MIN_WAIT)
      work->status = STATE_NO_RESPONSE_DISP;
    break;
  case STATE_NO_RESPONSE_DISP:
    {
      /* "応答ありません" もしくは "No Response" などを表示 */
      char *msg;
      
      // SetDrawCall( work, 0 );
      msg = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource(3, 0));
      CODEC_DrawMessage( &work->panel, msg );
      CODEC_ShowMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
      codecCallAction(0);
      work->status = STATE_NO_RESPONSE_ERASE;
    }
    break;
  case STATE_NO_RESPONSE_ERASE:
    if( work->pad->press & ( PAD_A | PAD_B | PAD_X | PAD_Y ) ){
      CODEC_DrawMessage( &work->panel, NULL );
      CODEC_HideMesgPanel( &work->panel );	// Add K.Uehara 02/07/01
      work->status = STATE_SELECT;
    }
    break;
  case STATE_FDIN_WAIT: 
    /* フェードあり、アクションありの場合,フェードインを待つ */
    if(fade_in_wait(work)) break;
    work->status = STATE_LOAD;
    break;
  }
}

static int codec_erase(Work * work)
{
  /* この段階で,消費想定容量 128KB を確保する。
     確保できなければメモリが足りないものとみなし,無線の実行を却下する。 */
  if(NULL == (work->tmp_area = GV_Malloc(CODEC_RESERVE_SIZE))) return -1;

  /* この段階でレイアウトのロードを試みる。
     ロードできなければ、何もせずにエラーを返す。 */
  /* 起動モードによって、PUSH SELECT BUTTON TO EXIT */
  {
    int exit_disp = 0;
    exit_disp = (work->type == TYPE_NORMAL) ? 1 : 0;

    work->layout_work = NewCodecLayout(GV_StrCode("codec"), exit_disp);
    if(NULL == work->layout_work)
      {
	GV_Free(work->tmp_area);
	return -1;
      }
    GV_SetActorChild(work, work->layout_work);
  }


  if(work->type == TYPE_NORMAL)
    work->fade_mode = work->fade_out = 0;

  /* 無線前のメニュー関連フラグを一旦取得し,保存しておく */
  // work->old_menu_flags = GM_MenuStatus;
  GM_PushMenuStatus();

  /* メニューチャネルに描かれている、
     LIFE表示、武器選択、アイテム選択、レーダー等の表示は、
     無線画面中禁止とする。 */
  GM_SetMenuStatus(MENU_WEAPON_OFF |
		   MENU_ITEM_OFF   |
		   MENU_RADAR_OFF  |
		   MENU_GAGE_OFF);

  /* レーダームービーが再生中であれば強制停止 */
  RDR_MovieForceBreak();

  CodecInit( work );  /* システムにポーズをかける */
  DG_SetPrivilegeMode(1);
  GV_Free(work->tmp_area);   /* 予約ヒープを開放する */

  /* メニューやLIFE, レーダーが消えるまで、2 フレーム待つ */
  work->finish_cnt = 2;

  /* フェードインモードで無線モードになる場合は、
     フェードインを開始する */
  work->fade_mask = NULL;
  if(work->fade_mode && (work->type != TYPE_NORMAL)) setup_fade_mask(work);

  /* 心の声は,この時点では絶対に無い筈なので、call_block を初期化する */
  work->call_block = 0;

  /* 暗転状態から開始する場合は,アルファ値 を 0 にする */
  if(work->fade_mode && work->fadein_break)
    {
      work->fade_param = 0;
#ifndef KP_WINDOWS
      work->fade_mask->head.alpha =
	SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param);
#else
  /* Windows版では、ALPHA TESTの影響を避けるため、補正をかける */
      work->fade_mask->head.alpha =
	SCE_GS_SET_ALPHA(1, 2, 2, 2, work->fade_param+2);
#endif
    }

  DBG("codec_erase() finish.\n");

  return 0;
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

  /* 以前の fog 値を保存し、無線用の fog 値を設定する */
  work->old_fog = DG_FogColorMaster;
  work->old_fog_param1 = DG_FogParam1;
  work->old_fog_param2 = DG_FogParam2;
#ifndef PSX2
  work->old_fog_param = DG_FogParam ;
#endif
#ifdef BP_PSX2_GCC
  DG_FogColorMaster = (CVECTOR){0, 0, 0, 0};  /* まっくろ */
#else //BP
//#endif
//#ifdef KP_XBOX
  memset( &DG_FogColorMaster, 0, sizeof( CVECTOR ) );
#endif
 
  DG_FogParam1 = 255.0F;
  DG_FogParam2 = 255.0F;
  DG_SetFogParam( 65536.0f*20.0f, 65536.0f*21.0f );

  /*
   * 中央の周波数表示メータ等表示開始
   */

  /* レイアウト表示開始 */
  codecLayoutDisplayStart();

  CodecPartsInit(work);
  if(!work->fade_mode && work->act_open) codecLayoutDisp(1);
  codecSetFreq(work->freq);
}

/*
 * 無線モードとストリーミング再生の状態を確認し,
 * 無線を起動できない状態であれば、非0 を返す。
 * 起動できる状態であれば、しかるべき処置をとり、0 を返す。
 */
#ifdef KP_XBOX
#define DBMSG
#else
#ifdef DEBUG_MODE
#define DBMSG(...)  // printf(args)
#else
#define DBMSG(...)
#endif
#endif

/*
 * ストリーミングとの兼ね合いで無線が起動可能かどうかチェックする
 *
 *    0   不可。却下。おとといきやがれ。
 *    1   可能。即入居可。
 *    2   終了待ち。
 */
static int codec_check_evoke_codec(Work * work, int type)
{
  int st[2];

  /* 現在のストリーミング再生ステータスを取得する */
  work->speach_ch1 = 0;
  st[0] = GM_CheckMenuStatus(MENU_STREAM_CH_0);
  st[1] = GM_CheckMenuStatus(MENU_STREAM_CH_1);


#ifdef DEBUG_MODE
  if(_DEBUG_codec)
    {
      static char * name[] = { "(NONE)", "NORMAL", "CALL", "DEMO", "DIRECT" };
      static char * stat[] = { "OFF", "ON" };

      printf("<CODEC> Evoke Check: type = [%s] : Stream Ch.0[%s] Ch.1[%s]\n",
	     name[type], stat[ (st[0] != 0) ], stat[ (st[1] != 0) ]);
    }
#endif 

  switch(type)
    {
    case TYPE_NORMAL:
      if(GM_CheckMenuStatus(MENU_RADIO_DISABLE))
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into) printf("Cannot into codec: now Codec disable.\n");
#endif
	  return 0;   /* 無線禁止 */
	}
      if(GV_PauseLevel)
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into) printf("Cannot into codec: now Pausing.\n");
#endif
	  return 0;                       /* ポーズ中不可     */
	}
      if(GM_CheckGameStatus(STATE_PLAY_DEMO))
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into)
	    printf("Cannot into codec: now Demonstration.\n");
#endif
	  return 0; /* デモ中不可       */
	}
      if(DG_UnDrawFrameCount)
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into)
	    printf("Cannot into codec: disable redraw.\n");
#endif
	  return 0;                 /* 際描画禁止中不可 */
	}

      if(st[0])
	{
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into)
	    printf("Cannot into codec: stream 0 is playing.\n");
#endif
	  return 0;
	}
      if(st[1])
	{
	  if(!work->speech_mode)
	    {
#ifdef DEBUG_MODE
	      if(_DEBUG_codec_into)
		printf("Cannot into codec: stream 1 is playing (not speech mode.\n");
#endif
	      return 0;
	    }
	  /*
	   * ポーズをかける
	   */
	  GM_StreamSdPauseControl(1, 1);
	  work->speach_ch1 = 1;
	  return 1;
	}
      break;

    case TYPE_CALL:
      if(GM_CheckMenuStatus(MENU_RADIO_DISABLE)) return 0;   /* 無線禁止 */
      if(GV_PauseLevel)
	return 2;                       /* ポーズ中不可     */
      if(st[0]) return 2;
      if(st[1])
	{
	  if(!work->speech_mode)
	    {
#ifdef DEBUG_MODE
	      if(_DEBUG_codec_into) printf("CODEC block: stream 1 is playing (not speech mode.\n");
#endif
	      return 2;
	    }

	  /*
	   * ポーズをかける
	   */
	  GM_StreamSdPauseControl(1, 1);
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into) printf("  speech mode: Ch.1 Paused ( speech mode ).\n");
#endif
	  work->speach_ch1 = 1;
	  return 1;
	}
      break;

    case TYPE_DEMO:
    case TYPE_DIRECT:
      if(st[0] || st[1]) GM_StreamStopAll();
      return 1;   /* 止めたからあとは勝手にやらせてもらうぜ */
    }

  return 1;
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
  

  switch( work->status )
    {
    case STATE_CALL:
      /* CALL */

      /* ポーズ中であれば、ポーズが解けるまで処理を進行させない */
      if(GV_PauseLevel & GV_PAUSE_PAUSE)
	{
	  DBMSG("<CODEC> call PAUSE\n");
	  break;
	}

      /* 無線処理をブロック中であれば、
	 ブロックが解除されるまで処理を進行させない */
      if(work->codec_must_block) break;

      /* ゲームオーバー処理中であれば,CALL をキャンセルする */
      if(GM_IsGameOver())
	{
	  work->type = 0;
	  work->status = STATE_WAIT;

	  /* 再生中であれば、強制停止、開放 */
	  if(work->l2d_handle >= 0) L2D_ReleaseLayout(work->l2d_handle);
	  work->l2d_handle = -1;
	  work->call_step = CALL_STEP_WAIT;
	  work->evoke_set = 0;
	  break;
	}

      /*
       * CALL アイコンの表示制御
       */
      if(work->type != TYPE_DIRECT)
	if(work->call_step == CALL_STEP_WAIT)
	  {
	    work->call_step = CALL_STEP_INIT;
#ifdef AUX_CALL_ENABLE
	    work->call_mode = (work->type == TYPE_DEMO)
	      ? CALL_FORCE : CALL_AUX;
#endif /* AUX_CALL_ENABLE */
	  }

      /*
       * 起動が確定しているかどうかをチェック
       */
      work->count -= TIME_BASE;
      switch(work->type)
	{
	case TYPE_CALL:
	  if(work->pad->status & PAD_SEL) work->evoke_set = 1;
	  break;
	case TYPE_DEMO:
	  if(work->pad->status & PAD_SEL) work->evoke_set = 1;
	  /* その後の処理は DIRECT と同じ */
	case TYPE_DIRECT:
	  if(work->count < 0) work->evoke_set = 1;
	  break;
	}
      
      /*
       * 起動が確定しないまま時間が終了したら、待機に戻る
       */
      if(!work->evoke_set && (work->count < 0))
	{
	  work->status = STATE_WAIT;
	  if(work->call_step != CALL_STEP_WAIT)
	    work->call_step = CALL_STEP_TIMEOUT;
	  work->type = 0;
	  work->proc_id = 0;
	  break;
	}

      if(!work->evoke_set) break;

      /* 再生中であれば、強制停止、開放 */
      if(work->l2d_handle >= 0) L2D_ReleaseLayout(work->l2d_handle);
      work->l2d_handle = -1;
      work->call_step = CALL_STEP_WAIT;

      /* 起動が確定していれば、起動準備を行う */
      work->count = 0;
      switch(codec_check_evoke_codec(work, work->type))
	{
	case 0:  /* 不可。却下 */
	  {
	    work->status = STATE_WAIT;
	    if(work->call_step != CALL_STEP_WAIT)
	      work->call_step = CALL_STEP_TIMEOUT;
	    work->type = 0;
	    work->proc_id = 0;
	  }
	  break;
	case 1:  /* 即起動可 */
	  {
	    if((work->l2d_handle >= 0) ||
	       (work->call_step == CALL_STEP_SOUNDONLY))
	      work->call_step = CALL_STEP_BREAK;
	    if(!codec_erase(work))
	      {
		work->status = STATE_ERASE;
		break;
	      }
	    /* 起動出来なかった場合は、素通りして待機と同じ扱いに。 */
	  }
	case 2: /* ちょほいとまちなは */
	  break;
	}

      break;
      
    case STATE_WAIT:
      
      work->evoke_set = 0;    /* 起動は確定していない */
      work->disp_enable = 0;
      work->cont_call = 0;    /* 原則として継続コールは無し */

      /* ゲームオーバー中であれば、あらゆる無線をキャンセル */
      if(GM_IsGameOver())
	{
	  work->type = 0;
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_into) printf("Now GAMEOVER processing.\n");
#endif
	  break;
	}

      /* 無線の処理そのものがポーズなどによってブロックされていれば,
	 その間無線処理自体をブロック */
      if(work->codec_must_block) break;

      /* NORMAL 起動チェックと準備 */
      if(!work->type && (work->pad->status & PAD_SEL))
	if(codec_check_evoke_codec(work, TYPE_NORMAL) == 1)
	  {
	    work->type = TYPE_NORMAL;
	    work->finish_id = 0;
	    work->act_open = 1;
	    work->act_close = 1;
	    work->fade_mode = 0;
	    work->fade_out = 0;
	  }

      if( work->type != 0 )
	{
	  int type = work->type;

	  printf("DG_UnDrawFrameCount = %d\n", DG_UnDrawFrameCount);
	  
	  if(type == TYPE_NORMAL)
	    {
	      if(!codec_erase(work))
		work->status = STATE_ERASE;
	      else
		work->type = 0;  /* メモリ不足で入れない場合は、
				    何もなかったことにする。 */
	    }
	  else
	    {
	      if(work->type == TYPE_CALL)
		if(GM_CheckGameStatus(STATE_PLAY_DEMO))
		  {
		    printf("<CODEC> Warning: playdemo. disable CALL mode.\n");
		    break; /* デモ中は不可 */
		  }
	      work->status = STATE_CALL;
	      switch(type)
		{
		case TYPE_CALL:   work->count = NORMAL_CALL_TIME; break;
		case TYPE_DEMO:	  work->count = DEMO_CALL_TIME;	  break;
		case TYPE_DIRECT: work->count = 1;                break;
		}
	      // if(type != TYPE_DIRECT) CodecSE(CODEC_SE_CALL);
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
	      if(work->fadein_break)
		work->fade_param = 0;
	      
	      if(!fade_out_wait(work)) work->finish_cnt = 0;
	    }
	}
      break;
    case STATE_FADE_OUT:
      if( IsBGsetuped() )
	{
	  DBG("Point A\n");
	  /* FadeInSet( work, FADE_TIME ); */
	  GM_SetMenuStatus(MENU_RADIO_ON);  /* 無線ステータスを ON にする */
	  SetFunc( work, CodecMode );
	  if( DG_UnDrawFrameCount > 0 ){
#ifndef KP_WINDOWS
	    DG_UnDrawFrameCount = 0;
#else
        DG_SetUnDrawFrameCount(0);
#endif
	  }
	}
      break;
    }
}

static int icon_control(Work * work)
{
  SPR_OBJ * obj;

  if(work->l2d_handle < 0)
    return 0;

  if(NULL == (obj = L2D_GetObject(work->l2d_handle, 0x002a4634  /* "ROOT" */)))
    return 0;

  if(!work->icon_show)
    {
      SPR_HIDE(obj);
      return 1;
    }
  else
    {
      SPR_SHOW(obj);
      return 0;
    }
}

/*
 * CALL アイコンの表示を許可/禁止する
 */
void CODEC_CallIconSwitch(int sw)
{
  Work * work = codec_work;
  if(NULL == codec_work) return;
  work->icon_show = (!sw) ? 0 : 1;
}


void CODEC_Block(int sw)
{
  Work * work = codec_work;

  if(NULL == work) return;

  work->codec_must_block = (!sw) ? 0 : 1;
}

static void Act( Work *work )
{
  codecMemCheck();

  if(!icon_control(work))
    DrawCall(work);   /* 現在の call ステータスに合わせ,CALL アイコンを制御 */

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
  if(work->l2d_handle >= 0) L2D_ReleaseLayout(work->l2d_handle);
  if(work->fade_mask != NULL) SPR_Destroy_2D_Object(work->fade_mask);
  if(codec_work == work)
    {
      DBG("Old Codec Daemon is Die.\n");
      codec_work = NULL;
    }
#ifdef KP_WINDOWS
	CDC_ReleaseLogicalDirectory();
#endif
}

static int ReceiveSignal(void * workp, int signal, int value)
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
    case CDC_SIGNAL_SAVE_DIE:
      work->save_mode = 0;
      work->save_return = value;
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

	  work->l2d_handle = -1;   /* CALL l2d はまだ読み込んでいない */
	  work->call_act_idle = CALL_idleCall;

#ifdef AUX_CALL_ENABLE
	  /* AUX コール対応 */
	  work->call_act_open[CALL_FORCE] = CALL_forceCallP;
	  work->call_act_close[CALL_FORCE] = CALL_forceCloseP;

	  work->call_act_open[CALL_AUX] = CALL_auxCallP;
	  work->call_act_close[CALL_AUX] = CALL_auxCloseP;

	  if(work->dummy_bone != DUMMY_BONE)
	    {
	      work->call_act_open[CALL_FORCE] = CALL_forceCallT;
	      work->call_act_close[CALL_FORCE] = CALL_forceCloseT;

	      work->call_act_open[CALL_AUX] = CALL_auxCallT;
	      work->call_act_close[CALL_AUX] = CALL_auxCloseT;
	    }
#else
	  /* AUX コール表示非対応 */
	  work->call_act_open = CALL_openCallPlant;
	  work->call_act_close = CALL_closeCallPlant;

	  if(work->dummy_bone != DUMMY_BONE)
	    {
	      work->call_act_open = CALL_openCallTanker;
	      work->call_act_close = CALL_closeCallTanker;
	    }
#endif

	  /* シグナル受諾関数を指定する */
	  GV_SetActorSignalFunc(work, ReceiveSignal);
	}
	work->speech_mode = 0;  /* デフォルトは 0 */
	work->call_step = CALL_STEP_WAIT;
	work->call_block = 0;   /* デフォルトでは 0 */
	work->codec_must_block = 0;
	work->icon_show = 1;
	codec_work = work;

#ifdef DEBUG_MODE
	CODEC_AddFaceDebugMenu();
	CODEC_AddNoiseDebugMenu();
#ifndef KP_XBOX
	GM_AddDebugMenu(&debug_menu);
	GM_AddDebugMenu(&debug_menu2);
	GM_AddDebugMenu(&debug_menu3);
	GM_AddDebugMenu(&debug_menu4);
#endif
#endif /* DEBUG_MODE */

	return work;
}

/* ---------------------------------------------------------------------- */
/*
	コール起動
*/

void *CODEC_Call( int type, int freq, int code, int procid )
{
	Work *work = codec_work;

	/* CALL 中に CALL がかかった場合のため、一旦リセットを行う。
	   リセットを行う条件は以下の通り。

	   (1) 任意CALL中に他の CALL(CALL/DEMO/DIRECT)
	   (2) 強制CALL中(DEMO/DIRECT) に他の強制CALL(DEMO/DIRECT)が
	       かかった場合。同じ無線であれば,リセットせずに継続する。
	*/
	if(work->type != 0)
	  {
	    if(((work->type == TYPE_CALL) &&
		((type == TYPE_DEMO) || ( type == TYPE_DIRECT))) ||
	       (((work->type == TYPE_DEMO) || (work->type == TYPE_DIRECT)) &&
		((type == TYPE_DEMO) || ( type == TYPE_DIRECT)) &&
		((work->freq != freq) ||
		 (work->code != code) ||
		 (work->proc_id != procid))))
	      {
		if(work->l2d_handle >= 0)
		  {
		    if(L2D_STAT_BUSY ==
		       L2D_ActionStatus(codec_work->l2d_handle))
		      L2D_BreakAction(codec_work->l2d_handle);
		    L2D_ReleaseLayout(codec_work->l2d_handle);
		    codec_work->l2d_handle = -1;
		  }
		codec_work->call_step = CALL_STEP_WAIT;
		codec_work->proc_id = 0;  /* "reset" の瞬間に初期化 */
		SetFunc( codec_work, CodecStartWait );
	      }
	    else
	      {
		/*
		  条件に合致しなければ、既にかかっている無線が優先なので、
		  何もしない
		*/
		return NULL;
	      }
	  }
	
	work->type = type;
	work->freq = freq;
	work->code = code;
	work->proc_id = procid;

	printf("<CODEC>Call : type = %d, freq = %d, code = %d, procid = %d\n",
	       type, freq, code, procid);

	return NULL;
}

/* ---------------------------------------------------------------------- */
/*
	シナリオとのインターフェース
*/

int CodecMemCallSet(void)
{
  int area = -1;
  int freq = -1;
  char name_id = -1;
  extern int CODEC_MemCallReset(void);

  if(NULL != GCL_GetOption('r'))  /* reset: 無線メモリーリセット */
    CODEC_MemCallReset();  /* 無線メモリーリセット */

  if(NULL != GCL_GetOption('a'))  /* 'area': 登録エリア */
    area = GCL_GetNextInt();

  if(NULL != GCL_GetOption('f'))  /* 'freq': 周波数     */
    freq = GCL_GetNextInt();

  if(NULL != GCL_GetOption('n'))  /* 'name': 名前 ID    */
    name_id = GCL_GetNextInt();

  if((area < 0) || (freq < 0) || (name_id < 0))
    {
      GCL_Status = -1;
      return -1;
    }

#if BP_TGS_DEMO()
  {
     if(freq == 14096)
     {
        GCL_Status = -1;
        return -1;
     }
  }
#endif

  GCL_Status = MemCallSetName(area, freq, name_id);

  return 0;
}


int NewCodec( void )
{
  /*
   * 無線機モード起動関数
   */
#ifndef KP_WINDOWS
  ASSERT(codec_work != NULL);  /* 「chara 無線システム」が起動していない */
#else
  if ( !codec_work ) {
    printf("[ Warning ] 「chara 無線システム」が起動していない\n") ;
    return(0) ;
  }
#endif


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

    /* DEBUG By K.Uehara 01/08/13 */
    /* 開始 / 終了時のアクションとして、デフォルト値を設定する */
    codec_work->fade_mode     = 0; /* 通常の無線モード開始 */
    codec_work->fade_out      = 0; /* 通常の無線モード終了 */
    codec_work->act_open      = 1; /* 開きアクションあり   */
    codec_work->act_close     = 1; /* 閉じアクションあり   */
    codec_work->fadein_break  = 0;
    codec_work->fadeout_break = 0;
    codec_work->alarm_disable = 0; /* CALL/DEMO であればアラームを鳴らす */
    codec_work->icon_disable  = 0; /* CALL アイコンを表示する            */

    freq = GCL_GetNextInt();
    code = GCL_GetNextInt();
    type = GCL_GetNextInt();
	if( GCL_NextStr() != NULL ){
		proc_id = GCL_GetNextInt();
		ASSERT(proc_id != 0);
	} else {
		proc_id = 0;
	}
	printf("call: proc_id = %d\n", proc_id);
    CODEC_Call( type, freq, code, proc_id );
  }
  if( GCL_GetOption( 'r' ) != NULL ){

    if((codec_work->type == TYPE_DEMO) ||
       (codec_work->type == TYPE_DIRECT)) return 0;

    /* reset */
    DBG("!!!!!!!!!!!!! CODEC RESET !!!!!!!!!!!!!!!!!\n");
    codec_work->type = 0;
    if(codec_work->l2d_handle >= 0)
      {	
	if(L2D_STAT_BUSY == L2D_ActionStatus(codec_work->l2d_handle))
	  L2D_BreakAction(codec_work->l2d_handle);
	L2D_ReleaseLayout(codec_work->l2d_handle);
	codec_work->l2d_handle = -1;
      }
    codec_work->call_step = CALL_STEP_WAIT;
    codec_work->proc_id = 0;  /* "reset" の瞬間に初期化 */
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
    if(flag & 16)
      {
	DBG("[CODEC] ditect enable fadein break\n");
	/* フェードインを、完全にフェードアウトした状態から開始する */
	if(codec_work->fade_mode)
	  codec_work->fadein_break = 1;
      }
    if(flag & 32)
      {
	DBG("[CODEC] ditect enable fadeout break\n"); 
	/* フェードアウトを、完全にフェードアウトした状態で終了する */
	if(codec_work->fade_out) codec_work->fadeout_break = 1;
      }
    if(flag & 64)
      {
	DBG("[CODEC] ditect disable alarm\n");
	codec_work->alarm_disable = 1;
      }
    if(flag & 128)
      {
	DBG("[CODEC] ditect disable call icon\n");
	codec_work->icon_disable = 1;
      }
  }
  if( (GCL_GetOption( 'd' ) != NULL ) &&
      (codec_work->type == TYPE_DIRECT) ) {
    /* 再描画禁止オプションは、DIRECT の場合のみ有効
       (NORMAL, CALL, DEMO では無視) */
#ifndef KP_WINDOWS
    DG_UnDrawFrameCount = DG_UNDRAW_MAX;  /* 指定されている場合のみ。*/
#else
    DG_SetUnDrawFrameCount(DG_UNDRAW_MAX);
#endif
  }

  codec_work->reserve_list = GCL_GetOption('T'); /* 継続呼び出しブロックがある場合 */
  
  // -contC オプションがある場合は,継続呼び出しを Send ではなく Call として扱う
  codec_work->reserve_mode = 0;
  if( NULL != GCL_GetOption( 'C' )) codec_work->reserve_mode = 1;
  
  /* 'speach' オプションがあれば、無線IN時のストリーミングの扱いを変更 */
  if( NULL != GCL_GetOption('s'))
    codec_work->speech_mode = (GCL_GetNextInt()) ? 1 : 0;

  return 0;
}
  
/* 無線の状態を返す */
int CodecStatus(void)
{
  Work * work = codec_work;

  if(NULL == work) return -1;
  return work->type;
}
