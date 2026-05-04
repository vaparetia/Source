// ----------------------------------------------------------------------------
// BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
// BP
// ----------------------------------------------------------------------------
/*
   it_chng_vita.c
   アイテムチェンジテスト（その２）
   based on it_chng2.c

   2000/09/27 M.Sonoyama
   $Id: it_chng2.c,v 1.1.1.3 2002/11/19 11:45:12 Yoshizawa1 Exp $
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

/*------------------------------------------------------------------*/

enum
{
   IG_None = 0,      /* 素手 */
   IG_Cig,
   IG_Card,
   IG_Sensor,
   IG_Spprsr,
   IG_Etc1,
   IG_Goggle,
   IG_Scope,
   IG_Armor,
   IG_Box1,
   IG_Ration,
   IG_Medicine,
   IG_DogTag,
   IG_Etc2,
   IG_Wig,
   MAX_GROUPS
};

static int GrpNone[]       = { IT_None };
static int GrpCig[]        = { IT_Tabacco };
static int GrpCard[]       = { IT_Card, IT_MODisc };
static int GrpSensors[]    = { IT_BombSenserA, IT_BombSenserB, IT_MineDetector, IT_VibSensor };
static int GrpSpprsrs[]    = { IT_SocomSpprsr, IT_AKSpprsr, IT_UspSpprsr };
static int GrpEtc1[]       = { IT_Shaver, IT_PHS };
static int GrpGoggles[]    = { IT_NightVision, IT_Thermal };
static int GrpScopes[]     = { IT_Scope, IT_DummyScope, IT_Camera, IT_TnkCamera, IT_DummyTnkCamera };
static int GrpArmor[]      = { IT_Jacket, IT_Uniform };
static int GrpBoxs1[]      = { IT_CBBox, IT_CBBoxWet, IT_CBBoxB, IT_CBBoxC, IT_CBBoxD, IT_CBBoxE };
static int GrpRation[]     = { IT_Ration };
static int GrpMedicine[]   = { IT_Styptic, IT_Diazepam, IT_Medicine };
static int GrpDogTags[]    = { IT_DogTag };
static int GrpEtc2[]       = { IT_MugenBandana, IT_MugenWig, IT_Stealth };
static int GrpWigs[]       = { IT_WigA, IT_WigB, IT_WigC, IT_WigD };

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
   { GrpNone,      array_size(GrpNone)      },
   { GrpCig,       array_size(GrpCig)       },
   { GrpCard,      array_size(GrpCard)      },
   { GrpSensors,   array_size(GrpSensors)   },
   { GrpSpprsrs,   array_size(GrpSpprsrs)   },
   { GrpEtc1,      array_size(GrpEtc1)      },
   { GrpGoggles,   array_size(GrpGoggles)   },
   { GrpScopes,    array_size(GrpScopes)    },
   { GrpArmor,     array_size(GrpArmor)     },
   { GrpBoxs1,     array_size(GrpBoxs1)     },
   { GrpRation,    array_size(GrpRation)    },
   { GrpMedicine,  array_size(GrpMedicine)  },
   { GrpDogTags,   array_size(GrpDogTags)   },
   { GrpEtc2,      array_size(GrpEtc2)      },
   { GrpWigs,      array_size(GrpWigs)      },
};

static int GroupType[] =
{
   IG_Armor,
   IG_Box1,
   IG_Sensor,
   IG_Goggle,
   IG_Scope,
   IG_Ration,
   IG_None,
   IG_Medicine,
   IG_Cig,
   IG_Card,
   IG_Spprsr,
   IG_Etc1,
   IG_DogTag,
   IG_Etc2,
   IG_Wig,
};
#define GroupLists(_l,_n)   GroupInfoLists[GroupType[_l]].lists[_n]
#define ListSize(_l)        GroupInfoLists[GroupType[_l]].list_size

extern int GM_ItemChanged;

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
      work->dragThreshold.x = panelInfo.minAaX + (panelInfo.maxAaX - panelInfo.minAaX) / 5;
      work->dragThreshold.y = panelInfo.minAaY + 4 * (panelInfo.maxAaY - panelInfo.minAaY) / 5;
   }
}

static int gesture_in_box(Work *work, SceSystemGestureVector2 *pos)
{
   // touch anywhere on the left
   return pos->x < work->dragThreshold.x;
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
   MENU_UpdateItemExplainMessage(work->current_item);
}

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

/* 変更不可のチェック */
static int _CheckDisableChanging(int item)
{
   /* 無い */
   if (item != IT_None && GM_ItemNum(item) == 0)
   {
      return 1;
   }
   return PL_CheckNoUseItem(item);
}

static int CheckDisableChanging(void)
{
   if ( _CheckDisableChanging(GM_Item) )
   {
      GM_Item = IT_None;
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
         if (GM_ItemNum(n) > 0
            || (n == IT_Ration && (MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM) != 0))
         {
            work->l_flag[i][j] = 1;
            ++c;
            /* 切り替え対象装備の所属するグループを記憶 */
            if (n == GM_Item)
            {
               work->current_equip_group = i;
            }
            if ( !(GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX) )
            {
               /* 旧形式クイックチェンジ時 */
               if (GM_Item == IT_None)
               {
                  if (n == GM_ItemPrev)
                  {
                     work->prev_equip_group = i;
                  }
               }
               else
               {
                  if (n == IT_None)
                  {
                     work->prev_equip_group = i;
                  }
               }
            }
            else
            {
               /* 新方式クイックチェンジ時 */
               if (n == GM_ItemPrev)
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
         if ( item == GroupLists(i, j) )
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
            MENU_PutItemPanel(IT_PX + PW + 10, y + sy, item, a1, a2, flag);
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
      MENU_PutItemPanel(IT_PX, y + sy, item, alpha, alpha, flag);
      // this is the little grey/orange indicator box
      MENU_PutItemGroup(IT_PX - 10, y + sy, GroupType[group], alpha, group_col);
   }

   // draw the page indicators
   for (i = 0; i < work->page_count; ++i)
   {
      MENU_PutPagePrim(10, 200 - 32*i, 128, MENU_GRP_PREV_COL);
   }
   MENU_PutPagePrim(10, 200 - 32*work->current_page - 32*sy/VR_HEIGHT, 128, MENU_GRP_CURRENT_COL);
}

/*------------------------------------------------------------------*/

/* アイテム使用処理 */
static void UseItem(Work *work)
{
   if (0) // tapped "use item"
   {
      switch (work->current_item)
      {
      case IT_Ration:         /* レーション */
         if ( GM_ItemNum(IT_Ration) > 0 &&
            ( (GM_Vitality + GM_VitalityAdjust < GM_VitalityMax) ||
            ( GM_CheckPlayerStatus(PLAYER_BLOOD_DROP) ) ) )
         {
            GM_DecrementItem(IT_Ration, 1);
            GM_VitalityAdjust += GM_RATION_HEAL;
            GM_ResetPlayerStatus(PLAYER_BLOOD_DROP);
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_KAIHUKU1);
            if (++GM_RationUseCount > 30000)
            {
               GM_RationUseCount = 30000;
            }
         }
         break;

      case IT_Medicine:          /* 風邪薬 */
         if ( GM_ItemNum(IT_Medicine) > 0 &&
            GM_CheckPlayerStatus(PLAYER_COLD) )
         {
            GM_DecrementItem(IT_Medicine, 1);
            NewPlayerRecoverCold();
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02);
         }
         break;

      case IT_Styptic:        /* 止血剤 */
         if ( GM_ItemNum(IT_Styptic) > 0 &&
            GM_CheckPlayerStatus(PLAYER_BLOOD_DROP) )
         {
            GM_DecrementItem(IT_Styptic, 1);
            GM_ResetPlayerStatus(PLAYER_BLOOD_DROP);
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02);
         }
         break;

      case IT_Diazepam:          /* ジアゼパム */
         if (GM_ItemNum(IT_Diazepam) > 0)
         {
            GM_DecrementItem(IT_Diazepam, 1);
            /* 重ねがけＯＫ */
            GM_DiazepamCount += DIRECT_TICK(GM_DIAZEPAM_COUNT);
            if (GM_DiazepamCount > 1000000)
            {
               GM_DiazepamCount = 1000000;
            }
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02);
         }
         break;
      }
   }
}

/*------------------------------------------------------------------*/

static void OpenMenu(Work *work)
{
   int n = 0;

   if ( work->quick == 0 && OpenEnable(work) )
   {
      /* 初期化 */
      work->start_item = work->current_item = GM_Item;
      work->quick = 1;
      work->open_time_desc = 0;
      work->state = MENU_WIN_OPENING;
   }
   if ( work->quick > 0 && work->quick < QUICK_TIME
      && (!GM_CheckPlayerStatus(PLAYER_MENU_OPEN) || work->state == MENU_WIN_CLOSING) )
   {
      ++work->quick;
   }

   /* クイックチェンジのみ可の場合リターン */
   if (GM_CheckPlayerStatus(PLAYER_ITEM_QUICK_ONLY) || work->state == MENU_WIN_CLOSED)
   {
      if (GM_Item != IT_None)
      {
         MENU_PutItemPanel(IT_PX, PY, GM_Item, 128, 128, 0);
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
            MENU_ChangeItem(work->current_item);
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
         work->start_item = work->current_item = GM_Item;
         GetCurrentGroup(GM_Item, &work->current_group, &n);
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
         if (work->open_time_desc == OPEN_TIME_E && work->current_item != IT_None)
         {
            UpdateMessageTexture(work);
            MENU_PutExplainText(EXP_WIN2_IX + 64, EXP_WIN2_Y, 0);
         }
      }

      UseItem(work);

      /* 自動選択処理 */
      if (work->state == MENU_WIN_OPEN && work->open_time_list == OPEN_TIME_L && MENU_AutoSelectItem != -1)
      {
         // temp
         MENU_AutoSelectItem = -1;
#if 0
         int cg = 0, cl = 0, tg = 0, tl = 0;
         GetCurrentGroup(MENU_AutoSelectItem, &tg, &tl);
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
               MENU_AutoSelectItem = -1;
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
               if (winX < IT_PX + PW)
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
               else if (winX < IT_PX + 2*PW)
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

   GM_ItemChanged = 0;

   GM_ResetMenuStatus(MENU_ITEM_OPEN);

   // changes item somehow...
   if (MENU_DefaultCheckItem())
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

   int menuClosed = GM_CheckPlayerStatus(PLAYER_ITEM_DISABLE) || GM_CheckMenuStatus(MENU_ITEM_DISABLE);

   if (menuClosed)
   {
      /* メニューが閉じている状態 */
      if (work->tapHappened)
      {
         work->tapHappened = FALSE;
         if (MENU_QuickChangeItem())
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
            MENU_PutItemPanel(IT_PX, PY, GM_Item, 128, 128, 0);
         }
      }
      else
      {
         /* 通常時は無装備以外の */
         if (GM_Item != IT_None)
         {
            MENU_PutItemPanel(IT_PX, PY, GM_Item, 128, 128, 0);
         }
      }
   }

   switch (work->state)
   {
   case MENU_WIN_OPEN:
   case MENU_WIN_CLOSING:
      GM_SetMenuStatus(MENU_ITEM_OPEN);
      break;
   }

   OpenMenu(work);

   /* ゲージ消す */
   if (GM_CheckMenuStatus(MENU_ITEM_OPEN))
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

void  *NewItemChangeVita(int which)
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
