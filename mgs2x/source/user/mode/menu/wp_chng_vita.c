// ----------------------------------------------------------------------------
// BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
// BP
// ----------------------------------------------------------------------------
/*
   wp_chng_vita.c
   アイテムチェンジテスト（その２）
   based on wp_chng2.c and it_chng_vita.c

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "gameheader.h"
#include "sprite_2d.h"
#include "menu.h"

#include <systemgesture.h>
#include "gesture_vta.h"

extern int NewPlayerRecoverCold(void);

#undef OPEN_TIME_G
#undef OPEN_TIME_L
#define  OPEN_TIME_G    (6)      /* グループオープン時間 */
#define  OPEN_TIME_L    (6)      /* リストオーブン時間 */

//#define ALL_AVAILABLE_HACK

/*------------------------------------------------------------------*/

#define	Weapons		GM_WeaponNames

enum
{
   WG_None = 0,		/* 素手 */
   WG_HandGun,			/* 銃 */
   WG_Rifle,			/* ライフル */
   WG_Throw,			/* 投擲 */
   WG_Heavy,			/* 重火器 */
   WG_Blade,			/* ブレード */
   WG_Mic,				/* マイク */
   WG_Book,			/* 雑誌 */
   WG_Set,				/* 設置 */
   WG_Spray,			/* スプレー */
   MAX_GROUPS
};

static int GrpNone[]   = { WP_None };
static int GrpGun[]    = { WP_m92, WP_Usp, WP_Socom };
static int GrpRifle[]  = { WP_m4, WP_Aks, WP_Psg1, WP_Psg1T };
static int GrpThrow[]  = { WP_Grenade, WP_ChaffGrenade, WP_StunGrenade, WP_Magazine };
static int GrpHeavy[]  = { WP_Rgb6, WP_Nikita, WP_Stinger };
static int GrpBlade[]  = { WP_Blade };
static int GrpMic[]    = { WP_Mic, WP_DemoMic };
static int GrpBook[]   = { WP_Book };
static int GrpSet[]    = { WP_Claymore, WP_C4Bomb };
static int GrpSpray[]  = { WP_ColdSpray };

#define  MAX_GROUP_LISTS      (6)

typedef struct _group_info
{
   int *lists;     /* リスト内要素データ配列へのポインタ */
   int list_size;               /* リスト内最大要素数 */
}
GroupInfo;

#define array_size(x) (sizeof(x)/sizeof(x[0]))

static GroupInfo GroupInfoLists[] =
{
   { GrpNone ,      array_size(GrpNone )      },
   { GrpGun  ,      array_size(GrpGun  )      },
   { GrpRifle,      array_size(GrpRifle)      },
   { GrpThrow,      array_size(GrpThrow)      },
   { GrpHeavy,      array_size(GrpHeavy)      },
   { GrpBlade,      array_size(GrpBlade)      },
   { GrpMic  ,      array_size(GrpMic  )      },
   { GrpBook ,      array_size(GrpBook )      },
   { GrpSet  ,      array_size(GrpSet  )      },
   { GrpSpray,      array_size(GrpSpray)      },
};


static int	GroupType[] =
{
   WG_Heavy,			/* 重火器 */
   WG_Throw,			/* 投擲 */
   WG_Rifle,			/* ライフル */
   WG_HandGun,			/* 銃 */
   WG_None,			/* 素手 */
   WG_Spray,			/* スプレー */
   WG_Set,				/* 設置 */
   WG_Book,			/* 雑誌 */
   WG_Mic,				/* マイク */
   WG_Blade,			/* ブレード */
};

#define GroupLists(_l,_n)   GroupInfoLists[GroupType[_l]].lists[_n]
#define ListSize(_l)        GroupInfoLists[GroupType[_l]].list_size

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*
   ２Ｄ表示関連設定など
 */

#define MAX_PANEL_PRIM     ( (DISP_GROUPS + 1) * (DISP_LISTS + 1) ) /* 確保するパネルプリミティブ数 */
#define MAX_GROUP_PRIM     ( (DISP_GROUPS + 1) ) /* 確保するグループ表示プリミティブ数 */

/*------------------------------------------------------------------*/

typedef struct _Work
{
   GV_ACT actor;

   int quick;
   int state;

   int current_group;
   int current_list_item[MAX_GROUPS];
   int start_item;
   int current_item;

   int open_time_group;
   int open_time_list;
   int open_time_desc;                       /* 説明文表示時間（OPEN_TIME_Eで完全に開く） */ // for description

   int g_flag[MAX_GROUPS];
   int l_flag[MAX_GROUPS][MAX_GROUP_LISTS];
   int available_group_count;
   int page_count;
   int current_page;

   int available_groups[MAX_GROUPS];

   int current_group_item_count;
   int current_group_items[MAX_GROUP_LISTS];

   int selected_time;                        /* 選択時点滅用カウンタ */
   int current_equip_group;                  /* 現在装備中の装備の含まれるグループ番号 */
   int prev_equip_group;                     /* 以前装備していた装備の含まれるグループ番号 */

   int gesture_item_initialized;
   SceSystemGestureTouchRecognizer holdRecognizer;
   SceSystemGestureTouchRecognizer dragRecognizer;
   SceSystemGestureTouchRecognizer tapRecognizer;
   SceSystemGestureTouchRecognizer doubleTapRecognizer;
   int holdHappened;
   int dragHappened;
   int tapHappened;
   int doubleTapHappened;
   int dragging;
   int dragAmount;

   SceUInt32 dragEventID;
   SceSystemGestureVector2 dragCurrentPosition;
   SceSystemGestureVector2 dragThreshold;
   SceSystemGestureVector2 dragOrigin;
   SceSystemGestureVector2 tapPosition;
}
Work;

enum
{
   MENU_WIN_CLOSED = 0,
   MENU_WIN_OPENING,
   MENU_WIN_OPEN,
   MENU_WIN_CLOSING,
};

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

// want to respond quicker to a press (has to be enough to not be confused with a tap)
// may need to tweak this for frame rate
#undef QUICK_TIME
#define QUICK_TIME (6)

static void gesture_item_initialize(Work *work)
{
   if (!work->gesture_item_initialized)
   {
      SceTouchPanelInfo panelInfo;
      SceSystemGestureTouchRecognizerParameter multiTapParam;

      work->gesture_item_initialized = TRUE;

      sceSystemGestureCreateTouchRecognizer(&work->holdRecognizer, SCE_SYSTEM_GESTURE_TYPE_TAP_AND_HOLD, SCE_TOUCH_PORT_FRONT, NULL, NULL);
      sceSystemGestureCreateTouchRecognizer(&work->dragRecognizer, SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_FRONT, NULL, NULL);
      sceSystemGestureCreateTouchRecognizer(&work->tapRecognizer, SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, NULL, NULL);
      multiTapParam.tap.maxTapCount = 2;
      sceSystemGestureCreateTouchRecognizer(&work->doubleTapRecognizer, SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, NULL, &multiTapParam);
      work->dragEventID = 0;
      sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
      work->dragThreshold.x = panelInfo.minAaX + 3 * (panelInfo.maxAaX - panelInfo.minAaX) / 5;
      work->dragThreshold.y = panelInfo.minAaY + 4 * (panelInfo.maxAaY - panelInfo.minAaY) / 5;
   }
}

static int gesture_in_box(Work *work, SceSystemGestureVector2 *pos)
{
   // touch anywhere on the right
   return pos->x > work->dragThreshold.x;
}

static void gesture_item_update(Work *work)
{
   SceSystemGestureTouchEvent touchEvent;

   gesture_item_initialize(work);

   work->holdHappened = FALSE;
   sceSystemGestureUpdateTouchRecognizer(&work->holdRecognizer);
   {
      // look for hold to pop up menu
      int eventCount = sceSystemGestureGetTouchEventsCount(&work->holdRecognizer);
      for (int i = 0; i < eventCount; ++i)
      {
         sceSystemGestureGetTouchEventByIndex(&work->holdRecognizer, i, &touchEvent);
         if ( touchEvent.eventState == SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN
              && gesture_in_box(work, &touchEvent.property.tapAndHold.pressedPosition) )
         {
            // pop up the menu!
            work->holdHappened = TRUE;
         }
      }
   }

   work->dragHappened = FALSE;
   sceSystemGestureUpdateTouchRecognizer(&work->dragRecognizer);
   {
      // drag
      if (work->dragEventID == 0)
      {
         int eventCount = sceSystemGestureGetTouchEventsCount(&work->dragRecognizer);
         for (int i = 0; i < eventCount; ++i)
         {
            sceSystemGestureGetTouchEventByIndex(&work->dragRecognizer, i, &touchEvent);
            // check it's in the corner
            if ( touchEvent.eventState == SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN
                 && gesture_in_box(work, &touchEvent.property.drag.currentPosition) )
            {
               work->dragEventID = touchEvent.eventID;
               work->dragOrigin = touchEvent.property.drag.currentPosition;
               work->dragging = TRUE;
               work->dragAmount = 0;
               break;
            }
         }
      }
      if (work->dragEventID > 0)
      {
         sceSystemGestureGetTouchEventByEventID(&work->dragRecognizer, work->dragEventID, &touchEvent);
         {
            SceTouchPanelInfo panelInfo;
            sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);

            switch (touchEvent.eventState)
            {
            case SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN:
            case SCE_SYSTEM_GESTURE_TOUCH_STATE_ACTIVE:
               work->dragCurrentPosition = touchEvent.property.drag.currentPosition;
               work->dragAmount = (work->dragCurrentPosition.y - work->dragOrigin.y) * VR_HEIGHT / (panelInfo.maxAaY - panelInfo.minAaY);
               break;

            case SCE_SYSTEM_GESTURE_TOUCH_STATE_END:
            case SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED:
            case SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE:
               {
                  work->dragHappened = TRUE;
                  work->dragEventID = 0;
                  work->dragging = FALSE;
                  work->dragAmount = (work->dragCurrentPosition.y - work->dragOrigin.y) * VR_HEIGHT / (panelInfo.maxAaY - panelInfo.minAaY);
               }
               break;
            }
         }
      }
   }

   work->tapHappened = FALSE;
   sceSystemGestureUpdateTouchRecognizer(&work->tapRecognizer);
   {
      // look for selection
      int eventCount = sceSystemGestureGetTouchEventsCount(&work->tapRecognizer);
      for (int i = 0; i < eventCount; ++i)
      {
         sceSystemGestureGetTouchEventByIndex(&work->tapRecognizer, i, &touchEvent);
         work->tapPosition = touchEvent.property.tap.position;
         work->tapHappened = TRUE;
      }
   }
   work->doubleTapHappened = FALSE;
   sceSystemGestureUpdateTouchRecognizer(&work->doubleTapRecognizer);
   {
      // look for selection
      int eventCount = sceSystemGestureGetTouchEventsCount(&work->doubleTapRecognizer);
      for (int i = 0; i < eventCount; ++i)
      {
         sceSystemGestureGetTouchEventByIndex(&work->doubleTapRecognizer, i, &touchEvent);
         switch (touchEvent.property.tap.tappedCount)
         {
         case 2:
            work->tapPosition = touchEvent.property.tap.position;
            work->doubleTapHappened = TRUE;
            break;
         }
      }
   }
}

/*------------------------------------------------------------------*/
/* 説明文テクスチャを更新する（必ず説明文テクスチャを描画していないときに行うこと） */
static void UpdateMessageTexture(Work *work)
{
   MENU_UpdateWeaponExplainMessage(work->current_item);
}

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

/* 変更不可のチェック */
static int _CheckDisableChanging(int weapon)
{
   return PL_CheckNoUseItem(weapon);
}

static int CheckDisableChanging(void)
{
   if ( _CheckDisableChanging(GM_Weapon) )
   {
      GM_Weapon = WP_None;
      return 1;
   }
   return 0;
}

static inline int OpenEnable(Work *work)
{
   if ( (!GM_CheckPlayerStatus(PLAYER_MENU_OPEN) || work->state == MENU_WIN_CLOSING)
      && work->holdHappened )
   {
      return 1;
   }
   return 0;
}

// checks what is available

static void CheckAvailable(Work *work)
{
   int i, j, n, c;

   work->current_equip_group = -1;
   work->prev_equip_group = -1;
   work->available_group_count = 0;
   for (i = 0; i < MAX_GROUPS; ++i)
   {
      c = 0;
      for (j = 0; j < ListSize(i); ++j)
      {
         n = GroupLists(i, j);
#ifdef ALL_AVAILABLE_HACK
         if (j >= 0) // always true
#else
         if (GM_WeaponNum(n) > 0
            || (GM_WeaponNum(n) == 0 && (GM_WeaponTypes[ n ] & WP_TYPE_BODY)))
#endif
         {
            work->l_flag[i][j] = 1;
            ++c;
            /* 切り替え対象装備の所属するグループを記憶 */
            if (n == GM_Weapon)
            {
               work->current_equip_group = i;
            }
            if (!( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX))
            {
               /* 旧形式クイックチェンジ時 */
               if (GM_Weapon == WP_None)
               {
                  if (n == GM_WeaponPrev)
                  {
                     work->prev_equip_group = i;
                  }
               }
               else
               {
                  if (n == WP_None)
                  {
                     work->prev_equip_group = i;
                  }
               }
            }
            else
            {
               /* 新方式クイックチェンジ時 */
               if (n == GM_WeaponPrev)
               {
                  work->prev_equip_group = i;
               }
            }
         }
         else
         {
            work->l_flag[i][j] = 0;
            if (work->current_list_item[i] == j)
            {
               work->current_list_item[i] = -1;
            }
         }
      }
      if (work->current_list_item[i] == -1)
      {
         work->current_list_item[i] = 0;
         for (j = 0; j < ListSize(i); ++j)
         {
            if (work->l_flag[i][j] == 1)
            {
               work->current_list_item[i] = j;
               break;
            }
         }
      }
      work->g_flag[i] = c;
      if (c)
      {
         work->available_groups[work->available_group_count++] = i;
      }
   }
   work->page_count = (work->available_group_count + 4) / 5;
   for (int i = 0; i < work->available_group_count; ++i)
   {
      if (work->available_groups[i] == work->current_equip_group)
      {
         work->current_page = i / 5;
         break;
      }
   }
}

static void GetCurrentGroup(int item, int *g, int *l)
{
   int i, j;

   for (i = 0; i < MAX_GROUPS; ++i)
   {
      for (j = 0; j < ListSize(i); ++j)
      {
         if (item == GroupLists(i, j))
         {
            *g = i;
            if (l != NULL)
            {
               *l = j;
            }
            return;
         }
      }
   }
}

/* 近い方の検索 */
static int GetNearGroupDir(Work *work, int from_g, int to_g)
{
   int i, n, count, p_count = 0, m_count = 0, dir;
   /* 正方向でのチェック */
   n = from_g;
   for (i = 0, count = 0; i < MAX_GROUPS; ++i, ++n)
   {
      if (n >= MAX_GROUPS)
      {
         n -= MAX_GROUPS;
      }
      if (n == to_g)
      {
         p_count = count;
         break;
      }
      if (work->g_flag[n] > 0)
      {
         count++;
      }
   }
   /* 負方向でのチェック */
   n = from_g;
   for (i = 0, count = 0; i < MAX_GROUPS; ++i, --n)
   {
      if (n < 0)
      {
         n += MAX_GROUPS;
      }
      if (n == to_g)
      {
         m_count = count;
         break;
      }
      if (work->g_flag[n] > 0)
      {
         count++;
      }
   }
   /* 近いほうを選択 */
   if (m_count <= p_count)
   {
      dir = (m_count != 0) ? -1 : 0;
   }
   else
   {
      dir = (p_count != 0) ? 1 : 0;
   }
   return dir;
}

static int GetNearListDir(Work *work, int gp, int from_l, int to_l)
{
   int i, n, count, p_count = 0, m_count = 0, dir, max;
   max = ListSize(gp);
   /* 正方向でのチェック */
   n = from_l;
   for (i = 0, count = 0; i < max; ++i, ++n)
   {
      if (n >= max)
      {
         n -= max;
      }
      if (n == to_l)
      {
         p_count = count;
         break;
      }
      if (work->l_flag[gp][n] > 0)
      {
         count++;
      }
   }
   /* 負方向でのチェック */
   n = from_l;
   for (i = 0, count = 0; i < max; ++i, --n)
   {
      if (n < 0)
      {
         n += max;
      }
      if (n == to_l)
      {
         m_count = count;
         break;
      }
      if (work->l_flag[gp][n] > 0)
      {
         count++;
      }
   }
   /* 近いほうを選択 */
   if (m_count <= p_count)
   {
      dir = (m_count != 0) ? -1 : 0;
   }
   else
   {
      dir = (p_count != 0) ? 1 : 0;
   }
   return dir;
}

/*------------------------------------------------------------------*/

static void DisplayList(Work *work, int g, int sy)
{
   int time, max, y;
   int i, j, a1, a2;

   time = work->open_time_list;
   max = ListSize(g);
   y = GY;
   if (time > 0)
   {
      {
         /* アルファの調整 */
         /* タイトル部のアルファ計算 */
         a2 = 128 * time / OPEN_TIME_L;
         if (a2 < 0)
         {
            a2 = 0;
         }
         /* アイコンウィンドウのアルファ計算（タイトル部の影響を受ける） */
         a1 = ( GY - (y + sy) );
         if (a1 < 0)
         {
            a1 = -a1;
         }
         if (a1 > PH)
         {
            a1 = PH;
         }
         a1 = a2 - a2 * a1 / PH;
         /* やっぱりタイトル部はアイコン部との平均を取ってみる */
         a2 = (a1 + a2) / 2;
      }

      /* 表示データリストの作成 */
      j = 0;
      for (i = 0; i < max; ++i)
      {
         if (work->l_flag[g][i] > 0)
         {
            int item = GroupLists(g,i);
            work->current_group_items[work->current_group_item_count++] = item;
            int flag = item == work->current_item ? PANEL_FLAG_ACTIVE : PANEL_FLAG_NOACTIVE;
            /* フラグの設定（選択中のアイテムにフラグを立てる） */
            if ( _CheckDisableChanging(item) )
            {
               flag |= PANEL_FLAG_NO_USE;
            }
            /* パネルの表示 */
            y = PY - ( (PY - GY) + (PH + STEP_Y) * (j - 1) ) * time / OPEN_TIME_G;
            MENU_PutWeaponPanel(WP_PX - PW - 20, y + sy, item, a1, a2, flag);
            j++;
         }
      }
   }
}

/*------------------------------------------------------------------*/

static void DisplayGroup(Work *work, int sy)
{
   int time;
   int i, y, alpha;

   int lower = 5 * work->current_page;
   int upper = lower + 5;
   if (upper > work->available_group_count)
   {
      upper = work->available_group_count;
   }

   time = work->open_time_group;

   /* アルファの設定 */
   alpha = 128 * time / OPEN_TIME_G;

   work->current_group_item_count = 0;

   // draw the current page
   /* 表示処理 */
   for (i = lower; i < upper; ++i)
   {
      int group = work->available_groups[i];
      int item = GroupLists(group, work->current_list_item[group]);
      int group_col;           /* グループパネル表示カラー */
      y = PY - ( (PY - GY) + (PH + STEP_Y) * (i - lower - 1) ) * time / OPEN_TIME_G;
      /* グループ名の表示 */
      group_col = 0;
      if (group == work->prev_equip_group)
      {
         group_col = MENU_GRP_PREV_COL;
      }
      if (group == work->current_equip_group)
      {
         group_col = MENU_GRP_CURRENT_COL;

         /* グループ直下のリストの表示 */
         DisplayList(work, group, sy);
      }
      int flag = item == work->current_item ? PANEL_FLAG_ACTIVE : PANEL_FLAG_NOACTIVE;
      MENU_PutWeaponPanel(WP_PX, y + sy, item, alpha, alpha, flag);
      // this is the little grey/orange indicator box
      MENU_PutWeaponGroup(WP_PX - 10, y + sy, GroupType[group], alpha, group_col);
   }

   // draw the page indicators
   for (i = 0; i < work->page_count; ++i)
   {
      MENU_PutPagePrim(VR_WIDTH - 10 - 5, 200 - 32*i, 128, MENU_GRP_PREV_COL);
   }
   MENU_PutPagePrim(VR_WIDTH - 10 - 5, 200 - 32*work->current_page - 32*sy/VR_HEIGHT, 128, MENU_GRP_CURRENT_COL);
}

/*------------------------------------------------------------------*/

static void OpenMenu(Work *work)
{
   int n = 0;

   if ( work->quick == 0 && OpenEnable(work) )
   {
      /* 初期化 */
      work->start_item = work->current_item = GM_Weapon;
      work->quick = 1;
      work->open_time_desc = 0;
      work->state = MENU_WIN_OPENING;
   }
   if (work->quick > 0 && work->quick < QUICK_TIME
      && (!GM_CheckPlayerStatus(PLAYER_MENU_OPEN) || work->state == MENU_WIN_CLOSING))
   {
      ++work->quick;
   }

   /* クイックチェンジのみ可の場合リターン */
   if (GM_CheckPlayerStatus(PLAYER_WEAPON_QUICK_ONLY) || work->state == MENU_WIN_CLOSED)
   {
      if (GM_Weapon != WP_None)
      {
         MENU_PutWeaponPanel(WP_PX, PY, GM_Weapon, 128, 128, 0);
      }
      return;
   }

   if (work->state == MENU_WIN_CLOSING)
   {
      if (--work->open_time_group == 0)
      {
         GM_ResetPlayerStatus(PLAYER_PAD_OFF | PLAYER_MENU_OPEN);
         GM_GageStatus = GM_GAGE_STATE_APPEAR;

         GV_PauseOffActorSystem(GV_PAUSE_MENU);

         work->state = MENU_WIN_CLOSED;
         work->quick = 0;

         if (work->start_item != work->current_item)
         {
            MENU_ChangeWeapon(work->current_item);
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
            work->selected_time = 32;     /* 選択用カウンタ */
         }
      }
   }

   if (work->quick == QUICK_TIME)
   {
      if (work->state == MENU_WIN_OPENING)
      {
         /* 開きはじめ */
         GM_SetPlayerStatus(PLAYER_PAD_OFF | PLAYER_MENU_OPEN);
         GV_PauseOnActorSystem(GV_PAUSE_MENU);

         work->state = MENU_WIN_OPEN;
         GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDISP02);

         /* 初期化 */
         work->start_item = work->current_item = GM_Weapon;
         GetCurrentGroup(GM_Weapon, &work->current_group, &n);
         work->current_list_item[work->current_group] = n;
         /* 現在の武器をチェック */
         CheckAvailable(work);
      }
      if (work->state == MENU_WIN_OPEN)
      {
         if (work->open_time_group < OPEN_TIME_G)
         {
            work->open_time_group++;
         }
         else if (work->open_time_list < OPEN_TIME_L)
         {
            work->open_time_list++;
         }
      }

      if (!work->dragging)
      {
         work->dragAmount -= work->dragAmount/2;
      }
      DisplayGroup(work, work->dragAmount);
      /* 説明分表示 */
      if (work->state == MENU_WIN_OPEN && work->open_time_list == OPEN_TIME_L)
      {
         if (work->open_time_desc < OPEN_TIME_E)
         {
            work->open_time_desc++;
         }
         if (work->open_time_desc == OPEN_TIME_E && work->current_item != WP_None)
         {
            UpdateMessageTexture(work);
            MENU_PutExplainText(EXP_WIN2_WX, EXP_WIN2_Y, 0);
         }
      }

      /* 自動選択処理 */
      if (work->state == MENU_WIN_OPEN && work->open_time_list == OPEN_TIME_L && MENU_AutoSelectWeapon != -1)
      {
         // temp!
         MENU_AutoSelectWeapon = -1;
#if 0
         int cg = 0, cl = 0, tg = 0, tl = 0;
         GetCurrentGroup(MENU_AutoSelectWeapon, &tg, &tl);
         GetCurrentGroup(work->current_item, &cg, &cl);
         if (tg != cg)
         {
            /* グループを合わせる */
            if (GetNearGroupDir(work, cg, tg) < 0)
            {
//               press = PAD_D;
            }
            else
            {
//               press = PAD_U;
            }
         }
         else
         {
            if (cl != tl)
            {
               /* リストを合わせる */
               if (GetNearListDir(work, cg, cl, tl) < 0)
               {
//                  press = PAD_R;
               }
               else
               {
//                  press = PAD_L;
               }
            }
            else
            {
               MENU_AutoSelectWeapon = -1;
            }
         }
#endif
      }
      else
      {
         if (work->dragHappened)
         {
            work->dragHappened = FALSE;
            if (abs(work->dragAmount) > 25)
            {
               if (work->dragAmount < -25)
               {
                  work->current_page--;
                  if (work->current_page < 0) work->current_page = work->page_count - 1;
                  work->dragAmount = VR_HEIGHT + work->dragAmount;
               }
               else if (work->dragAmount > 25)
               {
                  work->current_page++;
                  if (work->current_page == work->page_count) work->current_page = 0;
                  work->dragAmount = work->dragAmount - VR_HEIGHT;
               }
            }
         }

         if (work->tapHappened || work->doubleTapHappened)
         {
            int winX, winY;
            int gy = GY;
            int stepy = PH + STEP_Y;

            SceTouchPanelInfo panelInfo;
            sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
            // convert tap pos to MGS coords
            // VR = VRAM
            winX = (work->tapPosition.x - panelInfo.minAaX) * VR_WIDTH / (panelInfo.maxAaX - panelInfo.minAaX);
            winY = (work->tapPosition.y - panelInfo.minAaY) * VR_HEIGHT / (panelInfo.maxAaY - panelInfo.minAaY);

            int iconIndex = (gy - winY + 2*stepy)/stepy;
            if (iconIndex >= 0 && iconIndex < 5)
            {
               if (winX > WP_PX)
               {
                  int groupIndex = 5*work->current_page + iconIndex;
                  if (groupIndex < work->available_group_count)
                  {
                     int group = work->available_groups[groupIndex];
                     int item = GroupLists(group, work->current_list_item[group]);
                     work->current_item = item;
                     work->current_equip_group = group;
                     if (work->doubleTapHappened)
                     {
                        work->state = MENU_WIN_CLOSING;
                     }
                  }
               }
               else if (winX > WP_PX - PW - 20)
               {
                  if (iconIndex < work->current_group_item_count)
                  {
                     int item = work->current_group_items[iconIndex];
                     work->current_item = item;
                     if (work->doubleTapHappened)
                     {
                        work->state = MENU_WIN_CLOSING;
                     }
                  }
               }
            }
         }
      }
   }
}

/*------------------------------------------------------------------*/

static void Act(Work *work)
{
   gesture_item_update(work);

   GM_ResetMenuStatus(MENU_WEAPON_OPEN);

   // changes item somehow...
   if (MENU_DefaultCheckWeapon())
   {
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
   }

   /* ポーズ中 */
   /* 非表示設定 */
   if ( (GV_PauseLevel & GV_PAUSE_PAUSE)
      || GM_CheckMenuStatus(MENU_ITEM_OFF | MENU_WEAPON_OPEN)
      || GM_CheckPlayerStatus(PLAYER_FORCE | PLAYER_LOCKER))
   {
      return;
   }

   GM_ResetMenuStatus(MENU_MENU_NEWPRESS);

   int menuClosed = GM_CheckPlayerStatus(PLAYER_WEAPON_DISABLE) || GM_CheckMenuStatus(MENU_WEAPON_DISABLE);

   if (menuClosed)
   {
      /* メニューが閉じている状態 */
      if (work->tapHappened)
      {
         work->tapHappened = FALSE;
         if (MENU_QuickChangeWeapon())
         {
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
         }
      }

      /* 選択中のアイテム表示 */
      if (work->selected_time)
      {
         // flash the selected item
         work->selected_time--;
         /* 選択直後は点滅表示を行う */
         if ((work->selected_time & 4) == 0)
         {
            MENU_PutWeaponPanel(WP_PX, PY, GM_Weapon, 128, 128, 0);
         }
      }
      else
      {
         /* 通常時は無装備以外の */
         if (GM_Weapon != WP_None)
         {
            MENU_PutWeaponPanel(WP_PX, PY, GM_Weapon, 128, 128, 0);
         }
      }
   }

   switch (work->state)
   {
   case MENU_WIN_OPEN:
   case MENU_WIN_CLOSING:
      GM_SetMenuStatus(MENU_WEAPON_OPEN);
      break;
   }

   OpenMenu(work);

   /* ゲージ消す */
   if (GM_CheckMenuStatus(MENU_WEAPON_OPEN))
   {
      GM_GageStatus = GM_GAGE_STATE_DISAPPEAR;
   }
}

/*------------------------------------------------------------------*/

static void Die(Work *work)
{
   GV_PauseOffActorSystem(GV_PAUSE_MENU);
   gesture_frontpanel_dec_ref();
}

/*------------------------------------------------------------------*/

void  *NewWeaponChangeVita(int which)
{
   Work *work = (Work *) GV_CreateActor(GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(Work), PLAYER_MENU_ACTOR_PRIO);
   if (work != NULL)
   {
      GV_SetActor(&work->actor, Act, Die);
      work->gesture_item_initialized = FALSE;
      gesture_frontpanel_inc_ref();
   }
   return work;
}
