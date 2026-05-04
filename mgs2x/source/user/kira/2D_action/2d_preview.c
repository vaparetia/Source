//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  2d_preview.c
  2Dレイアウト プレビュー環境用

  2001/02/21  Y.Kira
  $Id: 2d_preview.c,v 1.3 2002/11/23 12:42:25 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../kano/preview/prekey.h"
#include	"../../kano/preview/preview_def.h"
#include	"../../kano/preview/prefile.h"
#include        "sprite_2d.h"
#include        "layout_2d.h"
#include        "2d_preview.h"


#define SIGNAL_MAX   16

typedef struct {
  GV_ACT_EX actor;

  /*
   * メニューのカーソル移動等
   */
  int      selected;



  int      handle;      /* レイアウトハンドル値             */

  int      file_max; 
  int      file_num;

  int      action_max;  /* 最大アクション数                 */
  int      action;      /* 現在再生中のアクション           */

  /* 背景の RGBA 値
   * 2D レイアウトプレビュー時は、通常の3D表示を隠蔽し、背景の RGBA 値を
   * 変更できるようにする。
   */
  SPR_OBJ   * bg_sprite;  /* 背景色用スプライト */
  unsigned char r, g, b, a;

  /* シグナル受信履歴 */
  int         signal_begin;
  int         signal_nums;
  int         frame_time;
  struct {
    int t;       /* シグナル発生時刻 */
    int signal;  /* シグナル名称値   */
    int value;   /* シグナル引数     */
  } signal_history[ SIGNAL_MAX ];

  /*
   * プレビュー上のオプション
   */
  int      play_loop:1; /* プレビュー中ループするかどうか   */
  int      bg:1;        /* 色つき背景をつけるかどうか       */
  /*
   * フラグ類
   */
  int      playing:1;   /* 再生中は非0になる。              */
  int      evoke:1;     /* 非0 のときに Evoke を発行する    */
  
} Work;

enum {
  SELECT_FILE,
  SELECT_ACTION,
  SELECT_LOOP,
  SELECT_RED,
  SELECT_GREEN,
  SELECT_BLUE,
  SELECT_ALPHA,
  SELECT_MAX
};



static Work             * now_work = NULL;
static PreviewKey_Param   PreviewKey0;


static void SignalHandler(void * workp, int signal, int value)
{
  Work * work = workp;
  int idx;
  
  idx = (work->signal_begin + work->signal_nums) % SIGNAL_MAX;
  work->signal_history[idx].signal = signal;
  work->signal_history[idx].value  = value;
  work->signal_history[idx].t      = work->frame_time * TIME_BASE;

  work->signal_nums++;
  if(work->signal_nums >= SIGNAL_MAX)
    {
      work->signal_begin =
	(work->signal_begin + (work->signal_nums - SIGNAL_MAX)) % SIGNAL_MAX;
      work->signal_nums = SIGNAL_MAX;
    }
}

static void PreKeyInit0(void)
{
    PreviewKey0.status=PreviewKey0.auto_status
		=PreviewKey0.press=0;
    PreviewKey0.repeat_count=0;
}

static void PreKeyAct0(void)
{
     PreviewKey0.press=GV_PadData[0].press;

    if(GV_PadData[0].status && PreviewKey0.status==GV_PadData[0].status){
		if(PreviewKey0.repeat_count>PreviewKey0.repeat_next_count){
			if(PreviewKey0.repeat_next_count>REPEAT_SECOND){
				PreviewKey0.repeat_count=PreviewKey0.repeat_next_count;
			}
			else{
				PreviewKey0.repeat_next_count+=REPEAT_NEXT;
			}
			PreviewKey0.auto_status=PreviewKey0.status;
		}
		else PreviewKey0.auto_status=0;

		PreviewKey0.repeat_count++;
    }
    else{
		PreviewKey0.auto_status
			=PreviewKey0.status=GV_PadData[0].status;
		PreviewKey0.repeat_count=0;
		PreviewKey0.repeat_next_count=REPEAT_FIRST;
    }
}

static void load_layout(int num)
{
  Work * work = now_work;
  char * filename;

  if(num < 0) return;

  if(work->handle >= 0)
    {
      L2D_BreakAction(work->handle);
      L2D_ReleaseLayout(work->handle);
    }
  filename = PreviewFile.layout_filenames[num];
  work->file_num = num;
  printf("load file %s.o2d\n", filename);
  if(0 <= (work->handle =
	   L2D_LoadLayout(GV_StrCode(filename), 4, 0, 0)))
    {
      work->action = 0;
      work->action_max = L2D_GetActionNumber(work->handle);
      printf("action max = %d\n", work->action_max);

      work->signal_begin = 0;
      work->signal_nums = 0;
      L2D_SetSignalHandle(work->handle, work, SignalHandler);
    }
}


static void Main_Menu_Cursor(void)
{
  Work * work = now_work;
  long64 pad = PreviewKey.auto_status;
  long64 press = PreviewKey.press;

  if(NULL == work) return;

  if(pad & PAD_U) work->selected--;
  if(pad & PAD_D) work->selected++;
  if(work->selected < 0) work->selected = SELECT_MAX - 1;
  if(work->selected >= SELECT_MAX) work->selected = 0;
  switch(work->selected)
    {
    case SELECT_FILE:
      if(work->file_max > 0)
	{
	  int pre;
	  
	  pre = work->file_num;
	  if(pad & PAD_R)
	    if(++work->file_num >= work->file_max) work->file_num = 0;
	  if(pad & PAD_L)
	    if(--work->file_num < 0) work->file_num = work->file_max - 1;
	  
	  if(pre == work->file_num) break;
	  load_layout(work->file_num);
	}
      break;
    case SELECT_ACTION:
      if(work->action_max > 0)
	{
	  if(press & PAD_A)
	    {
	      L2D_BreakAction(work->handle);
	      work->evoke = 1;
	    }

	  if(pad & PAD_R)
	    if(++(work->action) >= work->action_max) work->action = 0;
	  if(pad & PAD_L)
	    if(--(work->action) < 0) work->action = work->action_max - 1;
	}
      break;
    case SELECT_LOOP:
      /* ループモードの選択 */
      if(pad & (PAD_R | PAD_L))
	{
	  if(work->play_loop)
	    work->play_loop = 0;
	  else
	    work->play_loop = 1;
	}
      break;
    case SELECT_RED:
      if(pad & PAD_R) work->r++;
      if(pad & PAD_L) work->r--;
      break;
    case SELECT_GREEN:
      if(pad & PAD_R) work->g++;
      if(pad & PAD_L) work->g--;
      break;
    case SELECT_BLUE:
      if(pad & PAD_R) work->b++;
      if(pad & PAD_L) work->b--;
      break;
    case SELECT_ALPHA:
      if(pad & PAD_R) work->a++;
      if(pad & PAD_L) work->a--;
      break;
    default:
      break;
    }
}

static void Main_Menu_DebugPrint(void)
{
  Work * work = now_work;
  int x = LOCATE_X + 27;
  int y = LOCATE_Y + 18 * 2;
  char file_name[12];
  char act_name[12];
  char loop_sw[4];
  char bg_sw[4];

  if(NULL == work) return;

  sprintf(file_name, "LAYOUT%02d", work->file_num);
  sprintf(act_name, "ACTION%02d", work->action);
  sprintf(loop_sw, "%s", (work->play_loop) ? "ON" : "OFF");
  sprintf(bg_sw, "%s", (work->bg) ? "ON" : "OFF");
  DEBUG_Locate(x, y, 0);
  DEBUG_Printf("FILE_NAME   <%s>\n", file_name);
  DEBUG_Printf("ACTION      <%s>\n", act_name);
  DEBUG_Printf("LOOP        <%s>\n", loop_sw);
  DEBUG_Printf("RED   [%d]\n", work->r);
  DEBUG_Printf("GREEN [%d]\n", work->g);
  DEBUG_Printf("BLUE  [%d]\n", work->b);
  DEBUG_Printf("ALPHA [%d]\n", work->a);
  x = LOCATE_X + 18;
  y = LOCATE_Y + 18 * 2 + (work->selected) * (8 * 2 + 1);
  DEBUG_Locate(x, y, 0);
  DEBUG_Printf(">\n");
}

static void Main_Menu_SignalHistory(void)
{
  Work * work = now_work;
  int x = LOCATE_X + 270;
  int y = LOCATE_Y;
  int i, idx;

  if(NULL == work) return;

  DEBUG_Locate(x, y, 0);
  DEBUG_Printf("SIGNAL HISTORY\n");
  y += 18;
  DEBUG_Locate(x, y, 0);
  DEBUG_Printf(" TIME SIG      VALUE\n");
  /*            00000 0x000000 0x00000000 */
  for(i = 0; i < work->signal_nums; i++)
    {
      idx = (work->signal_begin + i) % SIGNAL_MAX;
      DEBUG_Printf("%5d 0x%06x 0x%08x\n",
		   work->signal_history[idx].t,
		   work->signal_history[idx].signal,
		   work->signal_history[idx].value);
    }
}

int Layout_Control(void)
{
  Work * work = now_work;
  int x = LOCATE_X + 9;
  int y = LOCATE_Y;

  if(NULL == work)
    {
      printf("NULL\n");
      return 1;
    }

  DEBUG_Locate(x, y, 0);
  DEBUG_Printf("2D LAYOUT DRIVER\n");
  
  Main_Menu_DebugPrint();
  Main_Menu_Cursor();
  Main_Menu_SignalHistory();
  if(PreviewKey.press & PAD_B) return 1;

  return 0;
}

static void Act(Work * work)
{
  work->frame_time++;

  if(work->bg)
    {
      SPR_SHOW(work->bg_sprite);
      work->bg_sprite->sprite.col.r = work->r;
      work->bg_sprite->sprite.col.g = work->g;
      work->bg_sprite->sprite.col.b = work->b;
      work->bg_sprite->sprite.col.a = work->a;
    }
  else
    SPR_HIDE(work->bg_sprite);

  if(work->handle >= 0)
    {
      /*
       * プレイ中にアクションが終了したら、play_loop フラグの内容によって
       * 挙動を変える。(0: 終了 / 1: 再び起動)
       */
      if(work->playing)
	if(!L2D_ActionStatus(work->handle))
	  {
	    if(work->play_loop)
	      work->evoke = 1;
	    else
	      work->playing = 0;
	  }
      if(work->evoke)
	{
	  work->frame_time = 0;  /* 時間カウンタを初期化 */
	  /* シグナル履歴を初期化 */
	  work->signal_begin = 0;
	  work->signal_nums = 0;
	  L2D_EvokeActionByNumber(work->handle, work->action);
	  work->playing = 1;
	  work->evoke = 0;
	}
      
      PreKeyAct0();
    }
}

static void Die(Work * work)
{
  if(work->handle >= 0)
    L2D_ReleaseLayout(work->handle);

  SPR_Destroy_2D_Object(work->bg_sprite);  /* 背景色スプライトを破棄 */
}

static int GetResources(Work * work)
{
  if(NULL != now_work) return -1;
  now_work = work;

  /* 2Dレイアウトマネージャの起動 */
  GV_SetActorChild(work, NewLayout2D_Player_for_Prog());

  work->handle     = -1;   /* 初期化: 未ロード */
  
  work->file_max = PreviewFile.layout_filenames_size;  /* ファイル数 */
  work->file_num = (work->file_max == 0) ? -1 : 0;

  work->action_max = 0;
  work->action     = -1;

  load_layout(work->file_num);

  work->play_loop = 0;
  work->bg = 1;    /* デフォルトでは色つき背景をつける */
  work->playing = 0;
  work->evoke = 0;

  /* 背景色スプライトは、チャネル 0 に表示する */
  work->bg_sprite = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
#if 0 //BP_PS2
//#ifdef PSX2
  SPR_SetPosSprite(work->bg_sprite, &(SPR_POS){0.0F, 0.0F});
#else
  {
	  SPR_POS		tmp_pos = {0.0F, 0.0F} ;
	  SPR_SetPosSprite(work->bg_sprite, &tmp_pos );
  }
#endif
  SPR_SetSizeSprite(work->bg_sprite, 512.0F, 384.0F);
  work->bg_sprite->head.flags |= SPR_FLAG_ALPHA;
  work->bg_sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);

  PreKeyInit0();

  return 0;
}

void * NewLayoutPreview(void)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
