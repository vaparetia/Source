//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  layout_2d.c
  2D レイアウトエディタおよびコンバータで作成された、2D レイアウトデータの
  再生を行うモジュール。

  2001/01/10  Y.Kira

  $Id: layout_2d.c,v 1.1.1.3 2002/11/19 11:43:50 Yoshizawa1 Exp $
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

#include "libgv.h"
#include "gameheader.h"
#include "debugmenu.h"

#include "font.h"
#include "libfs.h"

#include "BP_EndianSupport.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */


#define _layout_2d_c_
#include "layout_config.h"
#include "layout_data.h"
#include "layout_2d.h"

#ifdef PSX2

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(...)
#endif /* _DEBUG_ */

#else

#include <stdarg.h>
#ifdef _DEBUG_
static inline void DBG( char *f, ... )
{
	va_list		argptr ;
	char		buff[512];
	va_start( argptr, f );
	vsprintf( buff, f, argptr );
	printf( buff );
	va_end( argptr );
}
#else
static void inline DBG( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vsprintf( buff, f, argptr );
	va_end( argptr );
}
#endif /* _DEBUG_ */

#endif

typedef struct _Work {
  GV_ACT_EX    actor;
  
  l2dData    * data[LAYOUT_MAX];        /* 現在ロード済のデータ   */
  l2dAction  * action[LAYOUT_MAX];      /* 現在再生中のアクション */

  /* シグナルハンドラ関数へのポインタ */
  void (*sign_handle[ LAYOUT_MAX ])(void * workp, int sign, int value);

  /* シグナルハンドルに渡す Work ポインタの値 */
  void * sign_work[ LAYOUT_MAX ];

  int          pause_level[ LAYOUT_MAX ];

  char         action_pause[ LAYOUT_MAX ]; /* 各ハンドルのポーズ状態 */
  int          play_speed[ LAYOUT_MAX ];   /* 再生速度係数 */

  char         all_pause;             /* レイアウトドライバ自体のポーズ状態 */

} Work;


static Work * now_work = NULL;   /* 現在稼働中の Actor のワーク */

#ifdef PSX2

#ifdef DEBUG_MODE
static int l2d_proc_mesg = 0;
static int l2d_disp_stat = 0;

static GM_DEBUG_MENU debug_exec_func = {
  class:  "2D-LAYOUT",
  menu:   "DISP EXEC FUNC",
  max:    2,
  items:  (char *[]){"ON", "OFF"},
  values: (int *[]){1, 0},
  target: &l2d_proc_mesg,
  mask:   0x01,
};

static GM_DEBUG_MENU debug_disp_stat = {
  class:  "2D-LAYOUT",
  menu:   "DISP STATUS",
  max:    2,
  items:  (char *[]){"ON", "OFF"},
  values: (int *[]){1, 0},
  target: &l2d_disp_stat,
  mask:   0x01,
};

#define EXEC_FUNC(args...)  if(l2d_proc_mesg) printf(args)
#define DISP_STAT(args...)  if(l2d_disp_stat) printf(args)

#else

#define EXEC_FUNC(...)
#define DISP_STAT(...)

#endif /* DEBUG_MODE */

#else

#ifdef DEBUG_MODE
static int l2d_proc_mesg = 0;
static int l2d_disp_stat = 0;

static char	*on_off_list[2] = {"ON", "OFF"};
static int debug_values[2] = {1,0};
static GM_DEBUG_MENU debug_exec_func = {
	/* *next   */NULL,
	/* *class  */"2D-LAYOUT",
	/* *menu   */"DISP EXEC FUNC",
	/* **items */on_off_list,
	/* *values */debug_values,
	/* *target */&l2d_proc_mesg,
	/* mask    */0x01,
	/* *func   */NULL,
	/* strid   */0,
	/* type    */0,
	/* max     */2,
	/* current */0,
	/* padding */0
};

static GM_DEBUG_MENU debug_disp_stat = {
	/* *next   */NULL,
	/* *class  */"2D-LAYOUT",
	/* *menu   */"DISP STATUS",
	/* **items */on_off_list,
	/* *values */debug_values,
	/* *target */&l2d_disp_stat,
	/* mask    */0x01,
	/* *func   */NULL,
	/* strid   */0,
	/* type    */0,
	/* max     */2,
	/* current */0,
	/* padding */0
};

static void inline EXEC_FUNC( char *f, ... )
{
	va_list		argptr ;
	char		buff[512] ;
	va_start( argptr, f );
	if(l2d_proc_mesg){
		vsprintf( buff, f, argptr );
		//printf( buff );
		printf( "%s", buff );
	}
	va_end( argptr );
}
static void inline DISP_STAT( char *f, ... )
{
	va_list		argptr ;
	char		buff[512] ;
	va_start( argptr, f );
	if(l2d_disp_stat){
		vsprintf( buff, f, argptr );
		//printf( buff );
		printf( "%s", buff );
	}
	va_end( argptr );
}

#else

static void inline EXEC_FUNC( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vsprintf( buff, f, argptr );
	va_end( argptr );
}
static void inline DISP_STAT( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vsprintf( buff, f, argptr );
	va_end( argptr );
}

#endif /* DEBUG_MODE */

#endif

static int fetch_track(Work * work, int handle, l2dTrack * track, int fetch);
static int search_status(l2dSprite * spr, int strcode);

/* ------------------------------------------------------------------------- */
/*                                対外関数                                   */
/* ------------------------------------------------------------------------- */

/* レイアウトデータのセットアップ */
int L2D_SetupLayout2(void * entry_ptr, int chanl,
		     int base_pri, int add_flag, int pause_level, float safeZoneOffsetY)
{
  int handle;
  l2dData * data;

  EXEC_FUNC("entering L2D_SetupLayout()\n");
  
  if(NULL == now_work) {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
  }

  /*
   * 空いているレイアウトのハンドルを得る
   */
  for(handle = 0; handle < LAYOUT_MAX; handle++)
    if(NULL == now_work->data[handle]) break;
  if(handle >= LAYOUT_MAX) {
      DISP_STAT("layout_2d.c: could not alloc 2D-layout discriptor.\n");
      return L2D_STAT_BUSY;  /* 全てのハンドルが使用中 */
  }

  data = l2dSetupData(entry_ptr, chanl, base_pri, add_flag, safeZoneOffsetY);
  now_work->data[handle] = data;

  if(NULL == data) {
      DISP_STAT("layout_2d.c: interpretation failed.\n");
      return L2D_STAT_UNLOAD;  /* ロードできなかった */
  }
  now_work->action_pause[handle] = 0; /* ポーズは解除状態 */
  now_work->pause_level[handle] = pause_level;
  now_work->sign_handle[handle] = NULL;  /* シグナルハンドラ関数は未登録に */
  now_work->sign_work[handle] = NULL;
  now_work->play_speed[handle] = 1;  /* デフォルトは等速 */
  return handle;
}

typedef struct SLayoutLookupTag
{
   char *mName;
   int mFrontEnd;
   int mStrCode;
}
SLayoutLookup;

static SLayoutLookup sLayoutLookups[] =
{
   {"0009fde8", TRUE},
   {"004ac919"},
   {"0059fef4"},
   {"0069fe70"},
   {"0079fe30"},
   {"007a016c"},
   {"00980369"},
   {"5_1ch", TRUE},
   {"ai_ray_view"},
   {"album"},
   {"blk_bg", TRUE},
   {"boss_menu", TRUE},
   {"boss_pause"},
   {"boss_rush_stage_r_rai_b_r", TRUE},
   {"boss_rush"},
   {"boss_telop", TRUE},
   {"breath"},
   {"call"},
   {"casting"},
   {"chm1_stage_n_title", TRUE},
   {"chm1", TRUE},
   {"chm2_stage_n_title", TRUE},
   {"chm2", TRUE},
   {"chm3_stage_n_title", TRUE},
   {"chm3", TRUE},
   {"clear_code_stage_ending", TRUE},
   {"clear_code", TRUE},
   {"code_name"},
   {"codec"},
   {"color", TRUE},
   {"dcm"},
   {"dcm_shat"},
   {"tdcm"},
   {"densou_log", TRUE},
   {"densou_main", TRUE},
   {"densou_title", TRUE},
   {"dogtag"},
   {"dot_and_font", TRUE},
   {"equip"},
   {"fog1_stage_n_title", TRUE},
   {"fog1", TRUE},
   {"fog2_stage_n_title", TRUE},
   {"fog2", TRUE},
   {"fog3_stage_n_title", TRUE},
   {"fog3", TRUE},
   {"fpv_controls_jpn_ps2"},
   {"game_over"},
   {"genzi_stage_n_title", TRUE},
   {"genzi", TRUE},
   {"grd_stage_n_title", TRUE},
   {"grd", TRUE},
   {"htl", TRUE},
   {"lockon"},
   {"log", TRUE},
   {"map"},
   {"mapb"},
   {"mapbug"},
   {"missions_character_select", TRUE},
   {"missions_pliskin_ps2", TRUE},
   {"missions_raiden_ninja_ps2", TRUE},
   {"missions_raiden_ps2", TRUE},
   {"missions_snake_def_ps2", TRUE},
   {"missions_snake_old_ps2", TRUE},
   {"missions_snake_txd_ps2", TRUE},
   {"missions_x_raiden_ps2", TRUE},
   {"moses_a", TRUE},
   {"moses_a1", TRUE},
   {"moses_b", TRUE},
   {"moses_b1", TRUE},
   {"moses_c", TRUE},
   {"moses_c1", TRUE},
   {"moses_close", TRUE},
   {"ngl"},
   {"nkt"},
   {"node_enc", TRUE},
   {"node_frame_stage_n_title", TRUE},
   {"node_frame", TRUE},
   {"node_menu_stage_n_title", TRUE},
   {"node_menu", TRUE},
   {"node_name_stage_n_title", TRUE},
   {"node_name", TRUE},
   {"ny_map_bg_stage_n_title", TRUE},
   {"ny_map", TRUE},
   {"ny_map_stage_n_title", TRUE},
   {"ny_map_bg", TRUE},
   {"papet", TRUE},
   {"photo_save_stage_r_plt0_r", TRUE},
   {"photo_save", TRUE},
   {"plant_map", TRUE},
   {"prestmenu", TRUE},
   {"psg"},
   {"ray_view"},
   {"ray_wall", TRUE},
   {"roop_wall", TRUE},
   {"save_load_face_f01a"},
   {"save_load_face_f01b"},
   {"save_load_face_f01c"},
   {"save_load_face_f01d"},
   {"save_load_face_f01e"},
   {"save_load_face_f01f"},
   {"save_load_stage_d080p01"},
   {"save_load"},
   {"scope"},
   {"scrn", TRUE},
   {"scroll_stage_n_title", TRUE},
   {"scroll", TRUE},
   {"snake_tales_select", TRUE},
   {"special", TRUE},
   {"start_warning"},
   {"stg_alt"},
   {"stg"},
   {"tgl"},
   {"title_face", TRUE},
   {"title_logo", TRUE},
   {"title_menu", TRUE},
   {"vr_book", TRUE},
   {"vr_clear_alt"},
   {"vr_clear_code_ps2"},
   {"vr_clear"},
   {"vr_dcm"},
   {"vr_geme_over"},
   {"vr_pause_map"},
   {"vr_pause"},
   {"vr_system_five"},
   {"vr_window"},
   {"vtr"},
   {"zako_survival", TRUE},
};
static int sLayoutLookupsInitialized = FALSE;

int layout_lookup_comparer(void const *va, void const *vb)
{
   SLayoutLookup const *a = (SLayoutLookup const *)va;
   SLayoutLookup const *b = (SLayoutLookup const *)vb;
   return a->mStrCode - b->mStrCode;
}

/* レイアウトデータのロード */
int L2D_LoadLayout2(int strcode, int chanl,
		    int base_pri, int add_flag, int pause_level)
{
   float safeZoneOffsetY = 0;
   void * entry_ptr;

   if (!sLayoutLookupsInitialized)
   {
      int i = 0;
      for (i = 0; i < sizeof(sLayoutLookups)/sizeof(SLayoutLookup); ++i)
      {
         sLayoutLookups[i].mStrCode = GV_StrCode(sLayoutLookups[i].mName);
      }
      qsort(sLayoutLookups, sizeof(sLayoutLookups)/sizeof(SLayoutLookup), sizeof(SLayoutLookup), layout_lookup_comparer);
      sLayoutLookupsInitialized = TRUE;
  }

   {
      // use a binary search to find the key

      char *name = "Unknown";
      int mid;
      int low = 0;
      int high = sizeof(sLayoutLookups)/sizeof(SLayoutLookup) - 1;

      do
      {
         mid = (low + high) >> 1;
         if( strcode < sLayoutLookups[mid].mStrCode )
         {
            high = mid - 1;         // Search lower half
         }
         else if( strcode > sLayoutLookups[mid].mStrCode )
         {
            low  = mid + 1;         // Search upper half
         }
         else
         {
            name = sLayoutLookups[mid].mName;
            if (sLayoutLookups[mid].mFrontEnd)
            {
               // front end
               safeZoneOffsetY = -2;
            }
            break;
         }
      }
      while( low <= high );

      printf("[[[  Loading o2d file: '%s'  ]]]\n", name);

   }

  EXEC_FUNC("entering L2D_LoadLayout()\n");
  /* データのエントリを探す */
  entry_ptr = GV_GetCache(GV_CacheID(strcode & 0xffffff, 'o'));
  if(NULL == entry_ptr)
    {
      DISP_STAT("Layout not found (0x%08x)\n", strcode & 0xffffff);
      return L2D_STAT_UNLOAD;
    }
  /* 後の処理は L2D_SetupLayout2() と同じ */
  return L2D_SetupLayout2(entry_ptr, chanl, base_pri, add_flag, pause_level, safeZoneOffsetY);
}

/* 従来の互換関数 */
int L2D_SetupLayout(void * entry_ptr, int chanl, int base_pri, int add_flag, float safeZoneOffsetY)
{
  return L2D_SetupLayout2(entry_ptr, chanl,
			  base_pri, add_flag, GV_PAUSE_PAUSE, safeZoneOffsetY);
}

/* 従来の互換関数 */
int L2D_LoadLayout(int strcode, int chanl, int base_pri, int add_flag)
{
//   BP_Debug_DrawString(CStringExtras::Stringize_s("Frame: %d", gpRenderBackend->GetFrameCount()), &currentX, &currentY);


  return L2D_LoadLayout2(strcode, chanl, base_pri, add_flag, GV_PAUSE_PAUSE);
}

/* レイアウトに対し、シグナルハンドラを登録する */
int L2D_SetSignalHandle(int handle, void * workp,
			void (*func)(void * workp, int sign, int value))
{
  EXEC_FUNC("entering L2D_SetSignalHandle()\n");
  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }

  now_work->sign_handle[handle] = func;
  now_work->sign_work[handle] = workp;
  return L2D_STAT_ACK;
}

/* ハンドルに設定されている,現在のシグナルハンドラ関数のポインタを得る */
void (* L2D_GetSignalHandle(int handle))(void * workp, int sign, int value)
{
  EXEC_FUNC("entering L2D_GetSignalHandle()\n");
  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return NULL;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return NULL;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return NULL;
    }

  return now_work->sign_handle[handle];
}



/* アクション倍速再生係数を設定 */
int L2D_SetActionPlaySpeed(int handle, int speed)
{
  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }

  if(speed < 1) speed = 1;
  now_work->play_speed[handle] = speed;  /* 再生速度係数 */

  return L2D_STAT_ACK;
}



/* アクションの総数を取得 */
int L2D_GetActionNumber(int handle)
{
  EXEC_FUNC("entering L2D_GetActionNumber()\n");
  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }

  return now_work->data[handle]->act_nums;
}

/* アクションを強制停止 */
int L2D_BreakAction(int handle)
{
  EXEC_FUNC("entering L2D_BreakAction()\n");
  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }

  /* 動作中でなければ何もしない */
  if(NULL == now_work->action[handle]) return 0;

  /*
   * 動作中であれば、アクションを停止する。
   */
  now_work->action[handle] = NULL;

  return 0;
}

/* 指定されたハンドルに対し、アクションの一時停止状態を設定する */
int L2D_PauseAction(int handle, L2D_PAUSE_STAT pause)
{
  Work * work = now_work;


  EXEC_FUNC("entering L2D_PauseAction()\n");

  if(NULL == now_work)
    {
      EXEC_FUNC("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }
  switch(pause)
    {
    case L2D_PAUSE:      work->action_pause[handle] = 1; break;
    case L2D_CONTINUE:   work->action_pause[handle] = 0; break;
    }
  return L2D_STAT_ACK;
}


/* 2D レイアウトドライバの挙動自体の一時停止状態を設定する */
int L2D_PauseAllAction(L2D_PAUSE_STAT pause)
{
  Work * work = now_work;

  EXEC_FUNC("entering L2D_PauseAllAction()\n");

  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }

  switch(pause)
    {
    case L2D_PAUSE:    work->all_pause = 1;  break;
    case L2D_CONTINUE: work->all_pause = 0;  break;
    }
  return L2D_STAT_ACK;
}


/* アクションを起動 */
int L2D_EvokeAction(int handle, int strcode)
{
  int i, j;
  l2dAction * act;
  l2dTrack  * track;

  EXEC_FUNC("entering L2D_EvokeAction()\n");

  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }
  if(NULL != now_work->action[handle])
    {
      DISP_STAT("layout_2d.c: layout[%d] is busy (action doing).\n");
      return L2D_STAT_BUSY;
    }

  for(i = 0; i < now_work->data[handle]->act_nums; i++)
    if((now_work->data[handle]->act[i].code & 0xffffff) == (strcode & 0xffffff))
      {
	DISP_STAT("Running action %d\n", i);
	act = now_work->action[handle] = now_work->data[handle]->act + i;
	act->finished_cnt = 0;
	/* トラックの初期化 */
	for(j = 0; j < act->tracks_num; j++)
	  {
	    track = act->tracks + j;
	    track->pc = 0;
	    track->cmd = -1;
	    track->form_old = track->target->now_status;
	    track->form_new = -1;
	    track->time_limit = 0;
	    track->time_cnt   = 0;
	    fetch_track(now_work, handle, track, 0);
	  }
	return L2D_STAT_ACK;
      }
  return L2D_STAT_ACT_NOT_FOUND;
}

int L2D_EvokeActionByNumber(int handle, int actnum)
{
  int j;
  l2dAction * act;
  l2dTrack  * track;

  EXEC_FUNC("entering L2D_EvokeActionByNumber()\n");

  if(NULL == now_work)
    {
      DISP_STAT("layout_2d.c: 2D-layout daemon is not ready.\n");
      return L2D_STAT_NOT_ENOUGH;
    }
  if((handle < 0) || (handle >= LAYOUT_MAX))
    {
      DISP_STAT("layout_2d.c: bad layout discriptor(%d)\n", handle);
      return L2D_STAT_BAD_HANDLE;
    }
  if(NULL == now_work->data[handle])
    {
      DISP_STAT("layout_2d.c: layout discripter %d is not allocated.\n", handle);
      return L2D_STAT_UNLOAD;
    }
  if(NULL != now_work->action[handle])
    {
      DISP_STAT("layout_2d.c: layout[%d] is busy (action doing).\n");
      return L2D_STAT_BUSY;
    }

  if((actnum < 0) || (actnum >= now_work->data[handle]->act_nums))
    {
      DISP_STAT("layout_2d.c: action %d is not defined in layout[%d].\n");
      return L2D_STAT_ACT_NOT_FOUND;
    }

  act = now_work->action[handle] = now_work->data[handle]->act + actnum;
  act->finished_cnt = 0;
  /* トラックの初期化 */
  for(j = 0; j < act->tracks_num; j++)
    {
      track = act->tracks + j;
      track->pc = 0;
      track->cmd = -1;
      track->form_old = track->target->now_status;
      track->form_new = -1;
      track->time_limit = 0;
      track->time_cnt   = 0;
      fetch_track(now_work, handle, track, 0);
    }
  return L2D_STAT_ACK;
}

/*
 * レイアウトデータを開放
 */
int L2D_ReleaseLayout(int handle)
{
  EXEC_FUNC("Layout2D: entering L2D_ReleaseLayout()\n");
  if(NULL == now_work) return L2D_STAT_NOT_ENOUGH;
  if((handle < 0) || (handle >= LAYOUT_MAX)) return L2D_STAT_BAD_HANDLE;

  if(now_work->data[handle] != NULL) {
      DISP_STAT("Layout2D: Free Layout Data\n");
      l2dFreeData(now_work->data[handle]); /* 開放 */
  }
  now_work->data[handle] = NULL;
  now_work->action[handle] = NULL;
  EXEC_FUNC("Layout2D: exit L2D_ReleaseLayout()\n");
  return L2D_STAT_ACK;
}

/*
 * 任意名称の 2D オブジェクトを取得
 */
SPR_OBJ * L2D_GetObject(int handle, int strcode)
{
  if(NULL == now_work) {
	  printf( "layout_2d.c : system is not launched!!!!!!! see your gcl\n" ) ;
	  return NULL;
  }
  if( handle < 0 ) {
	  printf( "layout_2d.c : invalid handle!!!!!!! either L2D is not in list/*.src or No memory!!!!\n" ) ;
	  return NULL;
  }
  if( handle >= LAYOUT_MAX) {
	  printf( "layout_2d.c : handle overfull!!!!!!! Too many L2d\n" ) ;
	  return NULL;
  }
  if(NULL == now_work->data[handle]) {
	  printf( "layout_2d.c : Data not found in L2d system\n" ) ;
	  return NULL;
  }
  return l2dGetObject(now_work->data[handle], strcode);
}

/*
 * 任意名称のパーツ構造体のポインタを取得
 */
void * L2D_GetParts(int handle, int strcode)
{
  if(NULL == now_work) {
	  printf( "layout_2d.c : system is not launched!!!!!!! see your gcl\n" ) ;
	  return NULL;
  }
  if( handle < 0 ) {
	  printf( "layout_2d.c : invalid handle!!!!!!! either L2D is not in list/*.src or No memory!!!!\n" ) ;
	  return NULL;
  }
  if( handle >= LAYOUT_MAX) {
	  printf( "layout_2d.c : handle overfull!!!!!!! Too many L2d\n" ) ;
	  return NULL;
  }
  if(NULL == now_work->data[handle]) {
	  printf( "layout_2d.c : Data not found in L2d system\n" ) ;
	  return NULL;
  }
  return l2dGetParts(now_work->data[handle], strcode);
}

/*
 * アクション中であるかどうかを検出
 */
int L2D_ActionStatus(int handle)
{
  if(NULL == now_work) return L2D_STAT_NOT_ENOUGH;
  if((handle < 0) || (handle >= LAYOUT_MAX)) return L2D_STAT_BAD_HANDLE;
  if(NULL == now_work->data[handle]) return L2D_STAT_UNLOAD;
  if(NULL != now_work->action[handle]) return L2D_STAT_BUSY;
  return L2D_STAT_ACK;
}

/*
 * 指定のオブジェクトの持つ二つの状態の間で、任意のレートによる補間を行う
 */
int L2D_MorfObject(void * parts, int code1, int code2, float rate)
{
  l2dSprite * spr;
  int form1, form2;

  spr = (l2dSprite *)parts;
  form1 = search_status(spr, code1);
  form2 = search_status(spr, code2);
  if((form1 >= 0) && (form2 >= 0))
    (spr->conv_func)(spr, form1, form2, rate);
  return 0;
}

l2dStatus* BP_GetStatus(void * parts, int code)
{
   int form;
   l2dSprite * spr;
   l2dStatus * stat;  /* ??1(???) */
   spr = (l2dSprite *)parts;
   form = search_status(spr, code);
   stat = spr->stat + form;
   return stat;
}

/* ------------------------------------------------------------------------- */
/*
 * 状態を完全にオブジェクトに反映させる
 */
static void setup_status(l2dSprite * spr, int form)
{
  (spr->conv_func)(spr, form, form, 1.0F);
}


/*
 * 該当する StrCode を持つ状態の、インデックスを返す
 */

static int search_status(l2dSprite * spr, int strcode)
{
  int i;

  for(i = 0; i < spr->status_num; i++)
    if(spr->stat[i].code == strcode) return i;
  return -1;  /* 見付からない場合 */
}

static int fetch_track(Work * work, int handle, l2dTrack * track, int fetch)
{
  l2dAction * action;
  l2dSprite * spr;
  unsigned int cmd;
  int ret = 0;

  spr = track->target;
  track->pc += fetch;
  cmd = BP_LE_SwapUInt( track->steps_le[ track->pc ] );
  action = work->action[handle];
  switch(cmd & ACT_CMD_MASK)
    {
    case ACT_CMD_END:    /* トラックの終了 */
      action->finished_cnt++;
      ret = -1;
      break;
    case ACT_CMD_WAIT:   /* 待機命令       */
      track->time_cnt -= track->time_limit;  /* 余りの時間は持ち越し */
      track->time_limit = cmd & 0xffff;      /* 待ち時間を得る       */
      break;
    case ACT_CMD_SET:    /* 状態強制設定命令 */
      track->time_cnt -= track->time_limit;
      track->time_limit = 0;                 /* 概念上、時間 0 で処理*/
      /* 設定する状態のインデックスを得る */
      track->form_new = search_status(spr, cmd & 0xffffff);
      break;
    case ACT_CMD_MORF:   /* 補間命令         */
      {
	int code;
	
	code = BP_LE_SwapUInt( track->steps_le[ track->pc + 1] ) & 0xffffff;
	track->time_cnt -= track->time_limit;
	track->time_limit = cmd & 0xffff;
	track->form_new = search_status(spr, code); 
	track->spin_dir  = (cmd & 0x10000) >> 16;
      }
      break;
    case ACT_CMD_SIGN: /* シグナル命令     */
      /* シグナルの引数を得る */
      track->time_cnt -= track->time_limit;
      track->time_limit = 0;                 /* 概念上、時間 0 で処理*/
      track->form_new = BP_LE_SwapUInt( track->steps_le[ track->pc + 1 ] );
      break;
    }
  return ret;
}

/*
 * トラック 1フレーム分の処理
 */
static int play_track(Work * work, int handle, l2dTrack * track)
{
  l2dSprite * spr;
  unsigned int cmd; 
  int fetch = 0;
  int doing = 1;
  int ret = 0;
  int TB = TIME_BASE * work->play_speed[handle];  /* 速度に応じて変更 */

  spr = track->target;
  while(doing)
    {
      fetch = 0;
      cmd = BP_LE_SwapUInt( track->steps_le[track->pc] );
      track->cmd = cmd;
      switch(cmd & ACT_CMD_MASK)
	{
	case ACT_CMD_END:   /* 終端                   */
	  doing = 0;
	  ret = -1;
	  break;
	case ACT_CMD_WAIT:  /* 時間待ち               */
	  if(track->time_cnt >= track->time_limit)
	    fetch = 1;
	  else
	    {
	      /* 時間待ちの必要がある場合は、
		 一旦コマンドの実行をここで中断して次のフレームを待つ */
	      doing = 0;
	      
	      track->time_cnt += TB;
	    }
	  break;
	case ACT_CMD_SET:   /* 状態強制移行           */
	  {
	    /* この命令は、フレーム待ちを要しない */
	    setup_status(spr, track->form_new);
	    track->form_old = track->form_new;
	    track->target->now_status = track->form_old;
	    fetch = 1;
	  }
	  break;
	case ACT_CMD_MORF:  /* 時系列直線補間状態移行 */
	  {
	    float rate;

	    if(track->time_cnt >= track->time_limit)
	      {
		setup_status(spr, track->form_new);
		track->form_old = track->form_new;
		track->target->now_status = track->form_old;
		fetch = 2;
		break;
	      }

	    track->time_cnt += TB;
	    spr->spin_dir = track->spin_dir;
	    ASSERT(track->time_limit != 0.0F);
	    rate = (float)track->time_cnt / (float)track->time_limit;
	    if(rate > 1.0F) rate = 1.0F;
	    (spr->conv_func)(spr, track->form_old, track->form_new, rate);
	    doing = 0;
	  }
	  break;
	case ACT_CMD_SIGN:  /* シグナル */
	  {
	    /* この命令は,フレーム待ちを要しない */
	    int sign = cmd & 0xffffff;   /* シグナル名 */
	    int value = track->form_new; /* 引数       */
	    if(NULL != work->sign_handle[handle])
	      {
		printf("call handler: %p\n", track);
		(work->sign_handle[handle])
		  (work->sign_work[handle], sign, value);
	      }
	    fetch = 2;
	  }
	  break;
	  
	}
      /* 命令取得の必要がある場合 */
      if(fetch)	fetch_track(work, handle, track, fetch);
    }
  return ret;
}

static void Act(Work * work)
{
  int i;
  int handle;

  /* 2D レイアウトドライバ全体がポーズ中であれば、何もしない */
  if(work->all_pause) return;

  for(handle = 0; handle < LAYOUT_MAX; handle++)
    {
      /* 現在のポーズレベルと、
	 レイアウトのポーズレベルに一致するものがあれば何もしない */
      if(GV_PauseLevel & work->pause_level[handle]) continue;

      /* ハンドルがポーズ状態であれば何もしない */
      if(work->action_pause[handle]) continue;

      /* 再生中のアクションが無ければ何もしない */
      if(work->action[handle] == NULL) continue;

      /* 各トラックを再生 */
      for(i = 0; i < work->action[handle]->tracks_num; i++)
	play_track(work, handle, work->action[handle]->tracks + i);
      
      /* 全てのトラックが終了したら、再生中のアクションを何も無いことにする */
      if(work->action[handle]->finished_cnt
	 == work->action[handle]->tracks_num)
	work->action[handle] = NULL;
    }
}

/*
 * デストラクタ
 */
static void Die(Work * work)
{
  int handle;

  for(handle = 0; handle < LAYOUT_MAX; handle++)
    {
      /* 実行中のアクションを中断する */
      work->action[handle] = NULL;

      /* ロードされたデータがある場合は、全て開放する */
      if(work->data[handle] != NULL) L2D_ReleaseLayout(handle);
      work->data[handle] = NULL;
    }
  if(work == now_work) now_work = NULL;
}

/*
 * 起動リソースの取得
 */
static int GetResources(Work * work, int name, int where)
{
  int handle;

  if(now_work != NULL) {
      printf("layout_2d.c : Daemon is ready.\n");
      return -1;  /* 登録済であれば、エラー */
  }
  for(handle = 0; handle < LAYOUT_MAX; handle++)
    {
      work->data[handle] = NULL;     /* ロードされているデータを無しにする */
      work->action[handle] = NULL;   /* 再生中アクションを無しにする       */
    } 
  now_work = work;
#ifdef DEBUG_MODE
  GM_AddDebugMenu(&debug_exec_func);
  GM_AddDebugMenu(&debug_disp_stat);
#endif
  return 0;
}

static int GetResourcesP(Work * work)
{
  int handle;

  if(now_work != NULL) {
      printf("layout_2d.c : Daemon is ready.\n");
	  return -1;  /* 登録済であれば、エラー */
  }
  for(handle = 0; handle < LAYOUT_MAX; handle++)
    {
      work->data[handle] = NULL;     /* ロードされているデータを無しにする */
      work->action[handle] = NULL;   /* 再生中アクションを無しにする       */
    }
  now_work = work;
#ifdef DEBUG_MODE
  GM_AddDebugMenu(&debug_exec_func);
  GM_AddDebugMenu(&debug_disp_stat);
#endif
  return 0;
}


/*
 * シナリオ呼び出し用コンストラクタ
 */
void * NewLayout2D_Player(int name, int where)
{
  Work * work;

  /* レイアウトプレイヤは無線やノードなど、
     ゲーム全体に PAUSE がかかった状況下でも動かねばならない */
  if(NULL == (work = GV_NewActorPrio(LAYOUT_ACT_LEVEL,
				      sizeof(Work), LAYOUT_PRIO)))
    return NULL;

  /* Actor の登録 */
  GV_SetActor(work, Act, Die);
  GV_ActorEX(&work->actor);

  /* リソースの取得 */
  if(GetResources(work, name, where) < 0)
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}

/*
 * プログラム起動用コンストラクタ
 */
void * NewLayout2D_Player_for_Prog(void)
{
  Work * work;

  if(NULL == (work = GV_NewActorPrio(LAYOUT_ACT_LEVEL,
				     sizeof(Work), LAYOUT_PRIO))) return NULL;
 
  /* Actor の登録 */
  GV_SetActor(work, Act, Die);
  GV_ActorEX(&work->actor);

  /* リソースの取得 */
  if(GetResourcesP(work) < 0)
    {
      GV_DestroyActor(work);
      return NULL;
    }

  now_work = work;

  return work;
}
