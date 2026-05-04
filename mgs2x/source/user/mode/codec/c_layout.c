//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  c_layout.c
  無線画面のレイアウトに従い 2D オブジェクトを配置する。

  2000/09/01     Y.Kira

  $Id: c_layout.c,v 1.1.1.3 2002/11/19 11:44:57 Yoshizawa1 Exp $
*/
/*
  無線画面のレイアウトに従い、2D オブジェクトを配置する。
  また、応答があった場合の画面アクション等も制御する。

  ※現状では、レイアウトに従いオブジェクトを配置したり、
    画面アクションを制御するのはレイアウトマネージャの役割。
    あくまで操作とアクションのインタフェースをとるだけ。 
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
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"
#include "memcall.h"

#define _c_layout_c_
#include "c_layout.h"
#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "../../kira/2D_lib/util_2d.h"
#include "codec_config.h"
#include "codec_act.h"

#ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...)  printf(__VA_ARGS__)
#endif /* _DEBUG_*/
#endif
#ifdef KP_XBOX
#define DBG
#endif

typedef enum {
  CS_Idle,      /* 表示までの待機中                                 */
  CS_DispStart, /* 無線レイアウト表示開始                           */
  CS_Enter,     /* 画面初期化中                                     */
  CS_Begin,     /* 無線 IN 直後                                     */
  CS_Close,     /* 閉じている状態                                   */
  CS_OpenStart, /* 開き動作開始                                     */
  CS_Opening,   /* 開く途中                                         */
  CS_OpenSet,   /* 開き状態設定開始                                 */
  CS_Open,      /* 開いている状態                                   */
  CS_CloseStart,/* 閉じ動作開始                                     */
  CS_Closing,   /* 閉じる途中                                       */
  CS_Call,      /* 呼び出し中アクション再生                         */
  CS_Calling,   /* 呼出中                                           */
  CS_MemOpen,   /* メモリーコールモード呼び出し状態移行開始         */
  CS_MemOpening,/* メモリーコールモード移行中                       */
  CS_MemCall,   /* メモリーコールモード状態                         */
  CS_MemClose,  /* メモリーコールモードリスト収納開始               */
  CS_MemClosing,/* メモリーコールモードリスト収納中                 */
  CS_MovieOpen, /* ムービー画面開始                                 */
  CS_MovieClose,/* ムービー画面終了                                 */
  CS_MovieOpening, /* ムービー開き中                                */
  CS_MovieClosing, /* ムービー閉じ中                                */
  CS_MoviePlaying, /* ムービー再生中                                */
  CS_MovieAbort,   /* ムービー強制終了                              */
  CS_MovieAborting,/* 中断処理中                                    */
  CS_KeyWaitSet,   /* キー入力待ち開始                              */
  CS_KeyWait,      /* キー入力待ち表示                              */
  CS_KeyWaitErase, /* キー入力待ち消去                              */

  CS_Exit,      /* 終了アクション再生開始                           */
  CS_End,       /* 終了アクション再生中                             */
  CS_Finish,    /* 終了アクション終了。何もしていない状態           */

} CodecStep;

#define TIMEBASE  (BP_BASE_TICK())

#define INNER_TIME  40   /* 中枠が開ききるまでに必要な所要時間 */
#define OUTER_TIME  80   /* 外枠が開き切るまでに必要な所要時間 */
#define OUTER_DELAY 40   /* 中枠が動き始めてから外枠が動作を開始するまでの
			     遅延時間 */
#define OUTER_WID   (OUTER_TIME - OUTER_DELAY) /* 外枠が実際に動いている時間 */

#define STANDBY_STEP 10


#define LINE_R  60
#define LINE_G  220
#define LINE_B  200

#define CH          4    /* 描画チャネル */

/* #define NO_FREQ */

typedef struct _CodecLayoutWork {
  GV_ACT_EX   actor;

  /* 表示ステータス */
  int        disp_stat;  /* 表示フラグ                               */
  int        step;
  int        l2d_handle; /* レイアウトマネージャから返されたハンドル */
  int        mater_anim;
  int        mater_wait;
  int        mater_cnt;
  int        skin;

  /* 表示指標や,プログラム側でコントロールするオブジェクトのポインタ */
  SPR_OBJ   * root_obj;    /* root オブジェクト                    */
  SPR_OBJ   * movie_rect;  /* ムービー表示矩形領域をあらわす box   */
  SPR_OBJ   * no_response; /* 応答が無い場合の字幕を貼る矩形の box */

  void      * memlist_work; /* メモリー呼出表示 Actor の Work */


  int         memcall_select; /* メモリー呼出選択 */


  int         earthQ_time;
  int         earthQ_amp;
  int         earthQ_cnt;


  int         exit_disp;  /* PUSH SELECT BUTTON TO EXIT 表示の許可/禁止 */

  int         arrow_sw:1;  /* 矢印表示フラグ                       */
  int         earthQ:1;    /* 地震フラグ                           */

} CodecLayoutWork;

CodecLayoutWork * codec_layout_now_work = NULL;  /* 初期状態では NULL */

#define Work CodecLayoutWork
#define now_work codec_layout_now_work

static int _disp_freq = 0;



/*
 * 無線レイアウトを構成するオブジェクトのポインタを返す
 */
SPR_OBJ * codecGetObject(int name)
{
  Work * work = now_work;


  if(NULL == work) return NULL;
  return L2D_GetObject(work->l2d_handle, name);
}


/*
 * 画面を揺らす
 */
#define DEFAULT_AMP 16


void codecEarthQuake(int sw, int q_time, int amp)
{
  Work * work = now_work;
  if(NULL == work) return;
  work->earthQ = (sw) ? 1 : 0;
  work->earthQ_time = q_time;
  work->earthQ_amp  = (!amp) ? DEFAULT_AMP : amp;
  work->earthQ_cnt  = 0;
}

#define OBJ_ROOT 0x002a4634

static void earth_quake(Work * work)
{
  static int x_bias = 0;
  static int y_bias = 0;
  int sign_x, sign_y;
  SPR_POS pos;
  SPR_OBJ * root;

  root = L2D_GetObject(work->l2d_handle, OBJ_ROOT);
  if(!work->earthQ)
    {
		SPR_POS p = {0.0F, 0.0F};
      SPR_SetPosEmpty(root, &p);
      return;
    }
  
  sign_x = (x_bias < 0) ? 1 : -1;
  sign_y = (y_bias < 0) ? 1 : -1;

  x_bias = sign_x * ((BP_PS2_rand() >> 16) % work->earthQ_amp);
  y_bias = sign_y * ((BP_PS2_rand() >> 16) % work->earthQ_amp);
  pos.x = x_bias;
  pos.y = y_bias;
  SPR_SetPosEmpty(root, &pos);

  if(work->earthQ_time > 0)
    {
      if((work->earthQ_cnt += TIME_BASE) >= work->earthQ_time)
	work->earthQ = 0;  /* 地震を止める */
    }
}

/*
 * 周波数マニュアル選択時の増減に伴う, TUNE表示の矢印点灯
 */
void codecLayoutTuneLight(int stat)
{
  static const int tune_parts[] = {
    0x0088a13b  /* "tune_l" */,
    0x00900ff9  /* "tune_Mozi" */,
    0x0088a141  /* "tune_r" */
  };
  Work * work = now_work;
  void * parts;
  int i;
  int name = 0x003daba1  /* "FreqStop" */;

  if(NULL == work) return;

  if(stat & CODEC_LIGHT_LEFT) name = 0x0089ed6c  /* "FreqSub" */;
  if(stat & CODEC_LIGHT_RIGHT) name = 0x0089a34e  /* "FreqAdd" */;
  if(!(stat & (CODEC_LIGHT_LEFT | CODEC_LIGHT_RIGHT)))
    name = 0x00927bc4  /* "default" */;

  for(i = 0; i < 3; i++)
    {
      parts = L2D_GetParts(work->l2d_handle, tune_parts[i]);
      if(NULL == parts) continue;
      L2D_MorfObject(parts, name, name, 1.0F);
    }
}

/*
 * 無線レイアウト表示開始
 */
void codecLayoutDisplayStart(void)
{
  Work * work = now_work;
  if(NULL == work) return;
  work->step = CS_DispStart;
}

/*
 * ムービー再生関連のアクションを呼び出す
 */
void codecLayoutMovieSwitch(int sw)
{
  Work * work = now_work;
  
  if(NULL == work) return;
  switch(sw)
    {
    case 0: work->step = CS_MovieClose; break;
    case 1: work->step = CS_MovieOpen;  break;
    case 2: work->step = CS_MovieAbort; break;
    }
}

int codecLayoutMovieStatus(void)
{
  Work * work = now_work;
  int ret = 1;
  if(NULL == work) return -1;

  switch(work->step)
    {
    case CS_MoviePlaying:
    case CS_OpenSet:
      ret = 0;
      break;
    default:
      ret = 1;
      break;
    }
  return ret;
}

/*
 * メモリーコールモードの状態にするアクションを呼び出す
 */
int codecMemCallOpen(int selected)
{
  Work * work = now_work;

  work->memcall_select = selected;
  work->step = CS_MemOpen;

  return 0;
}

int codecMemCallSelect(int select)
{
  Work * work = now_work;
  if(NULL == work) return -1;
  if(work->step != CS_MemCall) return -1;
  return MemCallSelect(select);
}

int codecMemCallClose(void)
{
  Work * work = now_work;
  work->step = CS_MemClose;
  return 0;
}



int codecMemCallStatus(void)
{
  Work * work = now_work;
  if(NULL == work) return 0;

  if((work->step == CS_MemCall)||(work->step == CS_Close)) return 0;

  return 1;
}


int codecActionStatus(void)
{
  int status;

  if(NULL == now_work) return -1;
  status =  (now_work->step == CS_Open ||
	     now_work->step == CS_Close) ? 0 : 1;
  return status;
}

int codecWaiting(int sw)
{
  switch(sw)
    {
    case -1: now_work->step = CS_Close;      break;
    case 0:  now_work->step = CS_OpenSet;    break;
    case 1:  now_work->step = CS_KeyWaitSet; break;
    }
  return 0;
}

int codecIsKeyWaitOK(void)
{
  Work * work = now_work;
  if(NULL == work) return 0;
  if((work->step == CS_Open) ||
     (work->step == CS_KeyWait)) return 1;
  return 0;
}



int codecSetFreq(int freq)
{
  _disp_freq = freq % 1000;
  return _disp_freq;
}


int codecOpenFrame(int act)
{
  if(NULL == now_work) return -1;
  now_work->step = (!act) ? CS_OpenStart : CS_OpenSet;

  return 0;
}

int codecErasePageIcon(void)
{
  if(NULL == now_work) return -1;
  now_work->step = CS_KeyWaitErase;
  return 0;
}


int codecCloseFrame(void)
{
  if(NULL == now_work) return -1;
  now_work->step = CS_CloseStart;
  return 0;
}

int codecSetMaterAnimDisable(int flag)
{
  if(NULL == now_work) return -1;

  /* now_work->mater_anim = flag; */
  return 0;
}

/*
 * PTT 点灯制御
 */
int codecTalkPTT(int sw)
{
  Work * work = now_work;
  void * parts;
  int    stat_name;

  if(NULL == work) return -1;

  if(NULL == (parts = L2D_GetParts(work->l2d_handle,
				   0x0001cef4  /* "ptt" */))) return 0;

  stat_name = (!sw)
    ? 0x00927bc4  /* "default" */
    : 0x0020f5e9  /* "brightPTT" */;

  L2D_MorfObject(parts, stat_name, stat_name, 1.0F);
  return 0;
}

/*
 * Call 中のアクションを起動
 */
int codecCallAction(int sw)
{
  Work * work = now_work;
  if(NULL == work) return -1;
  work->step = (!sw) ? CS_Close : CS_Call;
#if 1
  if(!sw)
    {
      void * parts;
      if(NULL == (parts = L2D_GetParts(work->l2d_handle,
				       0x0001cef4  /* "ptt" */))) return 0;
      L2D_MorfObject(parts,
		     0x00927bc4  /* "default" */,
		     0x00927bc4  /* "default" */, 1.0F);
    }
#endif
  return 0;
}

int codecLayoutExit(void)
{
  Work * work = now_work;
  if(NULL == work) return -1;

  work->step = CS_Exit;
  return 0;
}

int codecExitStatus(void)
{
  Work * work = now_work;
  int ret = 1;
  if(NULL == work) return -1;
  switch(work->step)
    {
    case CS_Exit:
    case CS_End:
      ret = 1;
      break;
    case CS_Finish:
      ret = 0;
      break;
    default:
      ret = -1;
      break;
    }
  return ret;
}

int codecLayoutDisp(int sw)
{
  Work * work = now_work;

  if(work == NULL) return -1;

  work->disp_stat = (sw) ? 1 : 0;
  printf("disp flag = %d\n", now_work->disp_stat);
  return 0;
}


static void setup_number(Work * work, int num_name, int num)
{
  static int stat[] = {
    CDCNUM_num0, CDCNUM_num1, CDCNUM_num2, CDCNUM_num3, CDCNUM_num4,
    CDCNUM_num5, CDCNUM_num6, CDCNUM_num7, CDCNUM_num8, CDCNUM_num9
  };
    
  void * parts;

  /* 数字パーツ領域を取得 */
  parts = L2D_GetParts(work->l2d_handle, num_name);
  if(NULL == parts) return;
  L2D_MorfObject(parts, stat[num], stat[num], 0.0F);  /* 状態を瞬時に設定 */
}

static void setup_freq(Work * work)
{
  static int name[] = { CDCOBJ_num0_01, CDCOBJ_num0_10, CDCOBJ_num1_00 };
  int i, freq;

  freq = _disp_freq;
  for(i = 0; i < 3; i++)
    {
      setup_number(work, name[i], freq % 10);
      freq /= 10;
    }
}

/*
 * 中央メータの上下
 */
static void mater_anim(Work * work)
{
  int level;
  static int level_name[] = { CDCLEV_level6, CDCLEV_level7,
			      CDCLEV_level8, CDCLEV_level9 };
  int label;
  void * parts;

  if(!work->mater_anim) return ;

  work->mater_cnt -= TIME_BASE;
  if(work->mater_cnt > 0) return;
  
#ifdef PSX2
  work->mater_cnt += ((BP_PS2_rand() >> 16) % 20) + 45;
  level = ((BP_PS2_rand() >> 16) % 40) / 10;
#else //XBOX
  work->mater_cnt += (rand() % 20) + 45;
  level = (rand()  % 40) / 10;
#endif
  
  label = level_name[level];

  parts = L2D_GetParts(work->l2d_handle, CDCOBJ_level_b_light);
  if(NULL != parts)
    {
      L2D_MorfObject(parts, label, label, 1.0F);
    }
  
  parts = L2D_GetParts(work->l2d_handle, CDCOBJ_level_t_light);
  if(NULL != parts)
    {
      L2D_MorfObject(parts, label, label, 1.0F);
    }

}

/*
 * 明るさ調整
 */
#define ADD_R  80
#define ADD_G  80
#define ADD_B  80

static void blight_adjust(void * workp, SPR_OBJ * obj)
{
  switch(obj->head.id)
    {
    case SP_EMPTY: break;
      
    case SP_POINT:
      obj->point.col.r += ADD_R;
      obj->point.col.g += ADD_G;
      obj->point.col.b += ADD_B;
      break;
      
    case SP_LINE:
      {
	int i;

	for(i = 0; i < 2; i++)
	  {
	    obj->line.col[i].r += ADD_R;
	    obj->line.col[i].g += ADD_G;
	    obj->line.col[i].b += ADD_B;
	  }
      }
      break;

    case SP_LINESTRIP:
      {
	int i;

	for(i = 0; i < obj->linestrip.pcnt; i++)
	  {
	    obj->linestrip.plist[i].col.r += ADD_R;
	    obj->linestrip.plist[i].col.g += ADD_G;
	    obj->linestrip.plist[i].col.b += ADD_B;
	  }
      }
      break;
      
    case SP_BOX:
    case SP_BOX_F:
      obj->box.col.r += ADD_R;
      obj->box.col.g += ADD_G;
      obj->box.col.b += ADD_B;
      break;
    case SP_SPRITE:
    case SP_SPRITE_F:
      obj->sprite.col.r += ADD_R;
      obj->sprite.col.g += ADD_G;
      obj->sprite.col.b += ADD_B;
      break;
    case SP_ZOOM:
    case SP_ZOOM_F:
      obj->zoom.col.r += ADD_R;
      obj->zoom.col.g += ADD_G;
      obj->zoom.col.b += ADD_B;
      break;

    case SP_SPIN:
    case SP_SPIN_F:
      obj->spin.col.r += ADD_R;
      obj->spin.col.g += ADD_G;
      obj->spin.col.b += ADD_B;
      break;

    case SP_POLY:
    case SP_POLY_F:
      {
	int i;

	for(i = 0; i < 4; i++)
	  {
	    obj->poly.col[i].r += ADD_R;
	    obj->poly.col[i].g += ADD_G;
	    obj->poly.col[i].b += ADD_B;
	  }
      }
      break;
    case SP_TRIANGLE:
      {
	int i;

	for(i = 0; i < 3; i++)
	  {
	    obj->triangle.col[i].r += ADD_R;
	    obj->triangle.col[i].g += ADD_G;
	    obj->triangle.col[i].b += ADD_B;
	  }
      }
    case SP_CLIP:
      break;
    }
}


static void Act(Work * work)
{
  /* 設定されている周波数に従い、画面表示上の周波数を変更する */
  setup_freq(work);

  if(work->disp_stat)
    SPR_SHOW(work->root_obj);
  else
    SPR_HIDE(work->root_obj);

  work->mater_anim = 0;
  switch(work->step)
    {
    case CS_Idle:      /* 表示までの待機中(まだ何もしない)                 */
      break;

    case CS_DispStart: /* 無線レイアウト表示開始                           */
      L2D_EvokeAction(work->l2d_handle, CDCACT_DefaultAction);
      work->step = CS_Enter;
      break;

    case CS_Exit:      /* 終了アクション開始                               */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      printf("CS_Exit\n");
      L2D_EvokeAction(work->l2d_handle, CDCACT_Exit);
      work->step = CS_End;
      break;
    case CS_End:       /* 終了アクション再生待ち */
      printf("CS_End\n");
      /* 数字を消す */
      {
	SPR_OBJ * obj;
	obj = L2D_GetObject(work->l2d_handle, 0x002c2f54  /* "frq_null" */);
	SPR_HIDE(obj);
      }
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_Finish;
      codecLayoutDisp(0);
      break;
    case CS_Finish:    /* 終了アクション終了 */
      /* 何もしない */
      printf("CS_Finish\n");
      break;

    case CS_Enter:     /* 初期アクション表示状態                           */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      {
	void * parts;
	int mode;
	mode = (work->exit_disp) ? CDCACT_showPRESS : CDCACT_hidePRESS;
	parts = L2D_GetParts(work->l2d_handle,
			     0x0065f89d  /* "press_select_null" */);
	L2D_MorfObject(parts, mode, mode, 1.0F);
	L2D_EvokeAction(work->l2d_handle, CDCACT_Enter);
	work->step = CS_Begin;
      }
      break;

    case CS_Begin:     /* 無線 IN アクション状態                           */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	work->step = CS_Close;
      break;
    case CS_Close:     /* 閉じている状態                                   */
      /* 閉じている間は何もしない */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	/* 何かしていたら止めさせる */
	L2D_BreakAction(work->l2d_handle);
      break;
    case CS_OpenStart: /* 開き開始                                         */
      /* 閉じている間は何もしない */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	/* 何かしていたら止めさせる */
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, CDCACT_Open);
      work->step = CS_Opening;
      break;
    case CS_Opening:   /* 開く途中                                         */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_OpenSet;
      break;

    case CS_OpenSet:   /* 開き状態に設定開始 */
      work->mater_anim = 1;
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      
      L2D_EvokeAction(work->l2d_handle, CDCACT_Talk);
      work->step = CS_Open;
      break;

    case CS_Open:      /* 開いている状態                                   */
      work->mater_anim = 1;
      /* "Talk" アクションを再生し続ける */
      /*
	if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, CDCACT_Talk);
      */
      break;

    case CS_CloseStart:/* 閉じ開始                                         */
      /* アクション実行中であれば中断 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      
      L2D_EvokeAction(work->l2d_handle, CDCACT_Close);
      work->step = CS_Closing;
      break;
    case CS_Closing:   /* 閉じる途中                                       */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_Close;
      break;
    case CS_Call:      /* 呼び出しアクション                               */
      /* 何かアクションを実行中であれば、
	 中断して強制的に呼び出しアクションを開始 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, CDCACT_Call);
      work->step = CS_Calling;
      break;
    case CS_Calling:   /* 呼び出し中アクションループ中                      */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, CDCACT_Call);
      break;

    case CS_MemOpen:   /* メモリーコールモード移行                          */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);

      L2D_EvokeAction(work->l2d_handle, CDCACT_MemOpen);
      work->step = CS_MemOpening;
      break;

    case CS_MemOpening:/* メモリーコールモード移行中                        */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	{
	  /* ここで記録されている周波数と人名をプログラムで表示 */
	  GV_SetActorChild(work,
			   NewMemCallList(work->l2d_handle,
					  work->memcall_select));
	  work->step = CS_MemCall;
	}
      break;

    case CS_MemCall:   /* メモリーコールモードリスト表示中                  */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, CDCACT_MemSelect);
      break;

    case CS_MemClose:  /* メモリーコールモードリスト収納開始                */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      GV_CallChildSignalFunc(work, GV_SIGNAL_KILL, 0);
      L2D_EvokeAction(work->l2d_handle, CDCACT_MemClose);
      work->step = CS_MemClosing;
      break;

    case CS_MemClosing:/* メモリーコールモードリスト収納中                  */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_Close;
      break;

    case CS_MovieOpen:  /* ムービー画面開始 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      work->step = CS_MovieOpening;
      L2D_EvokeAction(work->l2d_handle, CDCACT_OpenMovie);
      break;

    case CS_MovieOpening:  /* ムービー画面開き中 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_MoviePlaying;
      break;
    case CS_MoviePlaying:  /* ムービー再生中     */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, CDCACT_Movie);
      break;
    case CS_MovieClose:    /* ムービー画面閉じ   */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, CDCACT_CloseMovie);
      work->step = CS_MovieClosing;
      break;

    case CS_MovieClosing:  /* ムービー画面閉じ中 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->step = CS_OpenSet;
      break;

    case CS_MovieAbort:    /* ムービー強制中断 */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, CDCACT_hideMovie);
      work->step = CS_MovieAborting;
      break;
      
    case CS_MovieAborting:
      {
	int stat = L2D_ActionStatus(work->l2d_handle);
	printf("Aborting: stat = %d\n", stat);
	if(L2D_STAT_BUSY == stat) break;
	work->step = CS_OpenSet;
      }
      break;
    case CS_KeyWaitSet:
      work->mater_anim = 1;
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	 L2D_BreakAction(work->l2d_handle);
      work->step = CS_KeyWait;
      break;
    case CS_KeyWait:       /* キー入力待ち表示 */
      work->mater_anim = 1;
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, CDCACT_KeyWait);
      break;
    case CS_KeyWaitErase:
      work->mater_anim = 1;
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      {
	void * parts = L2D_GetParts(work->l2d_handle, 0x00a43320/* pagebox */);
	if(NULL != parts)
	  L2D_MorfObject(parts,
			 0x00c97194  /* "pagebox-key-1-0" */,
			 0x00c97194  /* "pagebox-key-1-0" */,
			 1.0F);
      }
      work->step = CS_OpenSet;
      break;
    }
  mater_anim(work);   /* メータアニメを行う */
  earth_quake(work);
}

static void Die(Work * work)
{
  if(work == now_work) now_work = NULL;
  /* オブジェクトを破壊すれば、その下位のオブジェクトは全て破壊される。
     よって、ここでは際上位オブジェクトを破壊する。*/
  L2D_ReleaseLayout(work->l2d_handle);
  work->l2d_handle = -1;
}

static int GetResources(Work * work, int skin, int exit_disp)
{
  work->disp_stat = 1;
  work->earthQ = 0;
  work->step = CS_Idle;
  work->mater_wait = 0;
  work->mater_cnt = 0;
  work->l2d_handle = L2D_LoadLayout2(skin, 4,
				     LAYOUT_PRIORITY, SPR_FLAG_PRIV, 0);
  if(work->l2d_handle < 0)
    {
#ifdef DEBUG_MODE
      printf("l2d load failed: err = %d\n", work->l2d_handle);
#endif 
      return -1;
    }
  // ASSERT(work->l2d_handle >= 0);  /* レイアウトがロードできなかった */
  work->arrow_sw = 0;  /* テキスト送り矢印表示スイッチ */
  
  work->exit_disp = exit_disp;

  /* メモリーコールモードのレイアウト準備 */
  // GV_SetActorChild(work, NewMemCallMenu(work->l2d_handle));
  
  /* 主要なオブジェクトを取得しておく */
  work->root_obj    = L2D_GetObject(work->l2d_handle, CDCOBJ_ROOT);
  
  ASSERT(work->root_obj != NULL);
  
  /* 初期状態のアクションとして、Enter を呼び出す。 */
  // L2D_EvokeAction(work->l2d_handle, CDCACT_DefaultAction);
  // L2D_EvokeAction(work->l2d_handle, CDCACT_Enter);
  
  return 0;
}

void * NewCodecLayout(int skin, int exit_disp)
{
  Work * work;
  
  OPERATOR();
  if(now_work != NULL) return NULL;
  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;
  
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  
  if(GetResources(work, skin, exit_disp))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  now_work = work;
  return work;
}
