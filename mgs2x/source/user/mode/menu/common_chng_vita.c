// ----------------------------------------------------------------------------
// BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
// BP
// ----------------------------------------------------------------------------
/*
   common_chng_vita.c
   shared by it_chng_vita_new and wp_chng_vita_new

   2000/09/27 M.Sonoyama
   $Id: it_chng2.c,v 1.1.1.3 2002/11/19 11:45:12 Yoshizawa1 Exp $
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sceerror.h>

#include "gameheader.h"
#include "sprite_2d.h"
#include "menu.h"

#include "gesture_vta.h"

#include "common_chng_vita.h"

// DO NOT CHECK IN!
//#define ALL_AVAILABLE_HACK

#define OPEN_TIME_TAP   (8)
#define OPEN_TIME_GROUP (6)      /* グループオープン時間 */
#define OPEN_TIME_LIST  (6)      /* リストオーブン時間 */

#define TAB_HEIGHT (16)          // height of each group tab
#define TAB_SPACE (3)            // space between each group tab
#define TAB_WIDTH (75)           // width of group tab
#define TAB_EXTEND (5)           // amount to increase group tab width when selected
#define TAB_THRESHOLD (TAB_WIDTH - 15) // where the thumb should be to move from group to items
#define TAB_THRESHOLD_OFFSET (10) // if the thumb starts beyond the threshold, add this
#define TAB_HYSTERESIS (3)       // to prevent bounce
#define ITEMS_X (TAB_WIDTH + TAB_EXTEND + 5)

#define TAB_BASE_COLOR     MENU_RGB(77, 77, 65)
#define TAB_SELECTED_COLOR MENU_RGB(162, 162, 136)
#define TAB_TEXT_COLOR     240, 240, 200

// the speed (in VR_WIDTH/HEIGHT units) of the simulated thumb when running pad demos
#define AUTOSELECT_GROUP_SPEED 3
#define AUTOSELECT_GROUPTOITEM_SPEED 5
#define AUTOSELECT_ITEM_SPEED 6

#define TICKS_UNTIL_GROUP_CHANGE 16

/*------------------------------------------------------------------*/

void CommonChangeInitialize(SCommonChangeWork *work, int which)
{
   int i;
   for (i = 0; i < MAX_GROUPS; ++i)
   {
      work->mCurrentItemTypeForGroup[i] = 0;
   }
   work->gesture_item_initialized = FALSE;
   work->pad = GV_PadData + which;
   work->pAutoSelectItem = NULL;
   work->struggle_power = 0;
}

static void ConvertDragToVrCoordinates(SCommonChangeWork *work)
{
   // convert drag pos to MGS coords
   SceTouchPanelInfo panelInfo;
   sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
   // VR = VRAM
   work->dragCurrentPosition.x = (work->dragCurrentPosition.x - panelInfo.minAaX) * VR_WIDTH / (panelInfo.maxAaX - panelInfo.minAaX);
   work->dragCurrentPosition.y = (work->dragCurrentPosition.y - panelInfo.minAaY) * VR_HEIGHT / (panelInfo.maxAaY - panelInfo.minAaY);

   if (work->rhs)
   {
      work->dragCurrentPosition.x = VR_WIDTH - work->dragCurrentPosition.x;
   }
}

/*------------------------------------------------------------------*/
static void QuickChangeItemHelper(SCommonChangeWork* work)
{
   // AS MCampbell - Added this helper function to keep the logic for quick selecting items in one place. A previous
   // bug fix for MGSTWO-2806 modified the code to check GM_ItemPrev before quick changing items. This was to prevent 
   // an invalid item from being equipped when in the water. Unfortunately this was not the correct fix. This correct
   // fix is to only perform the validity check when there is no item currently equipped. This fixes MGSTWO-3090 which
   // would not allow the user to quick select to a valid item in some cases.
   if (GM_Item == IT_None) 
   {
      if (work->fpCanSelect(GM_ItemPrev))
      {
         if (work->fpQuickChangeItem())
         {
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
         }
      }
   }
   else
   {
      if (work->fpQuickChangeItem())
      {
         GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
      }
   }
}

static void CommonChangeGestureUpdate(SCommonChangeWork *work)
{
   SceSystemGestureTouchEvent touchEvent;
   int i, eventCount, error;
   SceSystemGestureTouchRecognizer* pDragRecognizer = NULL;
   SceSystemGestureTouchRecognizer* pTapRecognizer = NULL;

   work->dragEventState = SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE;
   pDragRecognizer = GestureGetRecognizerByHandle(work->dragRecognizerHandle);
   if (!pDragRecognizer)
   {
      work->dragEventID = 0;
      work->tapHappened = FALSE;
      return;
   }

   if (work->dragEventID == 0)
   {
      eventCount = sceSystemGestureGetTouchEventsCount(pDragRecognizer);
      for (i = 0; i < eventCount; ++i)
      {
         sceSystemGestureGetTouchEventByIndex(pDragRecognizer, i, &touchEvent);
         work->dragCurrentPosition = touchEvent.property.drag.currentPosition;
         ConvertDragToVrCoordinates(work);
         if ( touchEvent.eventState == SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN )
         {
            work->dragEventID = touchEvent.eventID;
            work->shouldChangeCurrentEntry = FALSE;
            work->max_open_time_list = 0;
            break;
         }
      }
   }
   else if (work->dragEventID > 0)
   {
      SceInt32 result = sceSystemGestureGetTouchEventByEventID(pDragRecognizer, work->dragEventID, &touchEvent);
      if (result == SCE_SYSTEM_GESTURE_ERROR_EVENT_DATA_NOT_FOUND) // Stale event.
      {
         sceSystemGestureResetTouchRecognizer(pDragRecognizer);
         work->dragEventID = 0;
      }
      else
      {
         work->dragEventState = touchEvent.eventState;
         work->dragCurrentPosition = touchEvent.property.drag.currentPosition;
         ConvertDragToVrCoordinates(work);
         work->dragDirection = touchEvent.property.drag.deltaVector;

         switch (touchEvent.eventState)
         {
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_ACTIVE:
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_END:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE:
            work->dragEventID = 0;
            break;
         }
      }
   }

   work->tapHappened = FALSE;
   pTapRecognizer = GestureGetRecognizerByHandle(work->tapRecognizerHandle);
   if (pTapRecognizer)
   {
      if (sceSystemGestureUpdateTouchRecognizer(pTapRecognizer) == 0)
      {
         eventCount = sceSystemGestureGetTouchEventsCount(pTapRecognizer);
         for (i = 0; i < eventCount; ++i)
         {
            sceSystemGestureGetTouchEventByIndex(pTapRecognizer, i, &touchEvent);
            if (touchEvent.eventState != SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED)
            {
               work->tapHappened = TRUE;
            }
         }
      }
   }
}

/*------------------------------------------------------------------*/

static void get_initial_drag_rectangle(SCommonChangeWork *work, SceTouchPanelInfo *panelInfo, SceSystemGestureRectangle *rect)
{
   int panelWidth  = panelInfo->maxAaX - panelInfo->minAaX;
   int panelHeight = panelInfo->maxAaY - panelInfo->minAaY;

   // the panel plus the corner
   if (!work->rhs)
   {
      rect->x = panelInfo->minAaX;
      rect->width = (work->panelX + PW)*panelWidth/VR_WIDTH;
   }
   else
   {
      rect->x = panelInfo->minAaX + work->panelX*panelWidth/VR_WIDTH;
      rect->width = panelWidth - rect->x;
   }
   rect->y = panelInfo->minAaY + PY*panelHeight/VR_HEIGHT;
   rect->height = panelHeight - rect->y;
}

/*------------------------------------------------------------------*/

static void GestureInitialize(SCommonChangeWork *work)
{
   if (!work->gesture_item_initialized)
   {
      SceTouchPanelInfo panelInfo;
      SceSystemGestureRectangle rect;
      int panelWidth, panelHeight;
      SceSystemGestureTouchRecognizer* pTapRecognizer = NULL;
      SceSystemGestureTouchRecognizer* pDragRecognizer = NULL;

      work->gesture_item_initialized = TRUE;

      work->dragCurrentPosition.x = 0;
      work->dragCurrentPosition.y = 0;
      work->dragEventID = 0;
      work->dragEventState = SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE;
      work->tapHappened = FALSE;

      sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
      panelWidth = (panelInfo.maxAaX - panelInfo.minAaX);
      panelHeight = (panelInfo.maxAaY - panelInfo.minAaY);

      rect.x = panelInfo.minAaX + work->panelX*panelWidth/VR_WIDTH;
      rect.y = panelInfo.minAaY + PY*panelHeight/VR_HEIGHT;
      rect.width = PW*panelWidth/VR_WIDTH;
      rect.height = PH*panelHeight/VR_HEIGHT;

      work->tapRecognizerHandle = GestureAllocRecognizerHandle(kGesture_RecognizerType_Tap);
      assert(work->tapRecognizerHandle != kGesture_InvalidHandle);

      pTapRecognizer = GestureGetRecognizerByHandle(work->tapRecognizerHandle);
      if (pTapRecognizer)
      {
         sceSystemGestureUpdateTouchRecognizerRectangle(pTapRecognizer, &rect);
      }

      // adjust the touch rectangle for drag select to fill the corner
      get_initial_drag_rectangle(work, &panelInfo, &rect);

      work->dragRecognizerHandle = GestureAllocRecognizerHandle(kGesture_RecognizerType_Drag);
      assert(work->dragRecognizerHandle != kGesture_InvalidHandle);
      pDragRecognizer = GestureGetRecognizerByHandle(work->dragRecognizerHandle);

      if (pDragRecognizer)
      {
         sceSystemGestureUpdateTouchRecognizerRectangle(pDragRecognizer, &rect);
      }
   }
}

/*------------------------------------------------------------------*/

// checks what is available

static void CheckAvailable(SCommonChangeWork *work)
{
   int groupIndex;
   int itemIndex;
   int itemType;
   int groupItemType;
   int listItemType;
   SGroup *group;
   int i;

   work->group_count = 0;
   for (groupIndex = 0; groupIndex < work->maxGroups; ++groupIndex)
   {
      group = &work->groups[work->group_count];
      group->item_count = 0;

      for (itemIndex = 0; itemIndex < work->groupInfoLists[groupIndex].list_size; ++itemIndex)
      {
         itemType = work->groupInfoLists[groupIndex].lists[itemIndex];
#ifdef ALL_AVAILABLE_HACK
         if (itemIndex >= 0) // always true
#else
         if (work->fpItemAvailable(itemType))
#endif
         {
            group->items[group->item_count] = itemType;
            /* 切り替え対象装備の所属するグループを記憶 */
            if (itemType == work->fpGetItem())
            {
               work->current_group_index = work->group_count;
               work->prev_group_index = work->group_count;
               work->current_displayed_group_index = work->group_count;
               group->current_item_index = group->item_count;
               work->mCurrentItemTypeForGroup[groupIndex] = itemType;
            }
            ++group->item_count;
         }
      }
      if (group->item_count > 0)
      {
         group->current_group_index = groupIndex;
         ++work->group_count;
      }
      else
      {
         work->mCurrentItemTypeForGroup[groupIndex] = 0;
      }
   }

   // check that any of the current item types are not available
   // and if not reset them
   // also, re-link the current_item_index for groups
   for (i = 0; i < work->group_count; ++i)
   {
      group = &work->groups[i];
      groupIndex = group->current_group_index;
      group->current_item_index = 0;

      groupItemType = work->mCurrentItemTypeForGroup[groupIndex];
      if (groupItemType != 0)
      {
         if (!work->fpItemAvailable(groupItemType))
         {
            work->mCurrentItemTypeForGroup[groupIndex] = 0;
         }
         else
         {
            // find the item type in the list
            for (itemIndex = 0; itemIndex < group->item_count; ++itemIndex)
            {
               listItemType = group->items[itemIndex];
               if (listItemType == groupItemType)
               {
                  group->current_item_index = itemIndex;
                  break;
               }
            }
         }
      }
   }
}

/*------------------------------------------------------------------*/

static int GetGroupY(SCommonChangeWork *work, int group_index)
{
   int winH = VR_HEIGHT; // VR = VRAM
   int stepY = (TAB_HEIGHT + TAB_SPACE);
#if DRAW_CENTERED
   int totalY = work->group_count*stepY - TAB_SPACE;
   int y = (winH + totalY - 2*stepY) / 2 - group_index*stepY;
#else
   int y = winH - 100 - stepY*group_index;
#endif
   return y;
}

/*------------------------------------------------------------------*/

static int GetItemsBottomY(SCommonChangeWork *work, SGroup *group)
{
   int groupCenterY = GetGroupY(work, work->current_group_index) + TAB_HEIGHT/2;
   int totalY = group->item_count*(PH + STEP_Y) - STEP_Y;
   int topY = groupCenterY - totalY / 2;
   int bottomY = groupCenterY + totalY / 2;
   int halfPanelHeight = (PH) / 2;
   if (topY < halfPanelHeight)
   {
      bottomY += (halfPanelHeight - topY);
   }
   if (bottomY > PY + PH)
   {
      bottomY = PY + PH;
   }

   return bottomY;
}

/*------------------------------------------------------------------*/

static void DisplayList(SCommonChangeWork *work, SGroup *group)
{
   int bottomY = GetItemsBottomY(work, group);
   int itemsX = work->rhs ? (VR_WIDTH - ITEMS_X - PW) : ITEMS_X;

   if (work->open_time_list > 0)
   {
      int alpha = 128 * work->open_time_list / OPEN_TIME_LIST;
      /* 表示データリストの作成 */
      for (int itemIndex = 0; itemIndex < group->item_count; ++itemIndex)
      {
         int item = group->items[itemIndex];
         int isCurrentItem = (itemIndex == group->current_item_index);
         int canSelect = work->fpCanSelect(item);
         int flag = isCurrentItem ? PANEL_FLAG_ACTIVE : PANEL_FLAG_NOACTIVE;
         /* フラグの設定（選択中のアイテムにフラグを立てる） */
         if ( !canSelect )
         {
            flag |= PANEL_FLAG_NO_USE;
         }

         /* パネルの表示 */
         int y = bottomY - (PH) - (PH + STEP_Y) * itemIndex;
         int curAlpha = (work->state == kCommonMenuState_Closing && isCurrentItem && canSelect) ? 128 : alpha;

         work->fpPutPanel(itemsX, y, item, curAlpha, curAlpha, flag);
      }
   }
   else if (work->state == kCommonMenuState_Closing)
   {
      // lerp the selected item to the corner
      int item = group->items[group->current_item_index];
      if (work->fpCanSelect(item))
      {
         int startX = itemsX;
         int startY = bottomY - (PH) - (PH + STEP_Y) * group->current_item_index;
         int endX = work->panelX;
         int endY = PY;
         int lerp = 128 * work->open_time_group / OPEN_TIME_GROUP;
         int x = (lerp * startX + (128 - lerp) * endX)/128;
         int y = (lerp * startY + (128 - lerp) * endY)/128;
         int alpha = 128;
         work->fpPutPanel(x, y, item, alpha, alpha, PANEL_FLAG_ACTIVE);
      }
   }
}

/*------------------------------------------------------------------*/

static void DisplayGroups(SCommonChangeWork *work)
{
   /* アルファの設定 */
   int time = work->open_time_group;
   float falfa = ((float)(time))/((float)(OPEN_TIME_GROUP));
   float alfalfa = (3.0f - 2.0f*falfa)*falfa*falfa;
   int alpha = (int)(128.0f * falfa + 0.5f);

   //Doing this before rendering the rest of the item/weapon menu fixes the highlight bugs (MGSTWO-2318).
   //It seems that one of the DG_SetDmapackTex() calls in PutPanelPrim() overrides the alpha settings.
   {
      SGroup * const group = &work->groups[work->current_group_index];
      int const item = group->items[group->current_item_index];

      if (work->state == kCommonMenuState_Open && item != 0)
      {
         work->fpUpdateExplainMessage(item);
         MENU_PutExplainText(work->explainTextX, EXP_WIN_Y, 0);
      }
      /* グループ直下のリストの表示 */
      DisplayList(work, group);
   }   


   /* 表示処理 */
   for (int groupIndex = 0; groupIndex < work->group_count; ++groupIndex)
   {
      int y = GetGroupY(work, groupIndex);

      SGroup *group = &work->groups[groupIndex];
      int item = group->items[group->current_item_index];
      int width = TAB_WIDTH - work->slideAmount - (1.0 - alfalfa)*100;
      int color = TAB_BASE_COLOR;

      if (groupIndex == work->current_displayed_group_index)
      {
         width += TAB_EXTEND;
         color = TAB_SELECTED_COLOR;
      }

      if (work->rhs)
      {
         MENU_PutBoxPrim(VR_WIDTH - width, y, width, TAB_HEIGHT, 128, color);
         MENU_PutTextPrim(VR_WIDTH - width + 5, y + 2, work->pItemNames[item], TAB_TEXT_COLOR, 128, MENU_MODE_LEFT);
      }
      else
      {
         MENU_PutBoxPrim(0, y, width, TAB_HEIGHT, 128, color);
         MENU_PutTextPrim(width - 5, y + 2, work->pItemNames[item], TAB_TEXT_COLOR, 128, MENU_MODE_RIGHT);
      }
   }
}

/*------------------------------------------------------------------*/

static int is_item_in_group(SGroup *group, int item)
{
   int aye = 0;
   int i;
   for (i = 0; i < group->item_count; ++i)
   {
      if (group->items[i] == item)
      {
         aye = 1;
         break;
      }
   }
   return aye;
}

/*------------------------------------------------------------------*/

static int is_item_in_inventory(SCommonChangeWork *work, int item)
{
   int aye = 0;
   int i, j;
   for (i = 0; i < work->group_count; ++i)
   {
      SGroup *group = &work->groups[i];
      for (j = 0; j < group->item_count; ++j)
      {
         if (group->items[j] == item)
         {
            aye = 1;
            break;
         }
      }
   }
   return aye;
}

/*------------------------------------------------------------------*/

static void AutoSelectUpdate(SCommonChangeWork *work)
{
   int pullUpMenu = 0;
   int itemToSelect = -1;

   if (work->pAutoSelectItem)
   {
      itemToSelect = *work->pAutoSelectItem;
      // for some reason the cutscene sets 0 even though it's a valid item
      if (itemToSelect == 0 || !is_item_in_inventory(work, itemToSelect))
      {
         itemToSelect = -1;
      }
   }
   if (work->pad->status & work->autoSelectPullUpMenuButton)
   {
      pullUpMenu = 1;
      work->dragEventID = 1;
   }
   else
   {
      // must turn off the moment the shoulder button is released
      work->dragEventID = 0;
   }

   work->dragDirection.x = 0;
   work->dragDirection.y = 0;

   switch (work->autoSelectStage)
   {
   case kAutoSelectStage_Off:
      if (pullUpMenu)
      {
         work->dragCurrentPosition.x = 1;
         work->dragCurrentPosition.y = PY;
         work->autoSelectPause = 0;
         work->autoSelectStage = kAutoSelectStage_Enter;
      }
      break;
   case kAutoSelectStage_Enter:
      if (!pullUpMenu)
      {
         // must have been a tap
         work->autoSelectStage = kAutoSelectStage_Off;
         // toggle

         QuickChangeItemHelper(work);
      }
      else if (work->open_time_list >= OPEN_TIME_LIST)
      {
         work->autoSelectStage = kAutoSelectStage_SelectGroup;
      }
      break;
   case kAutoSelectStage_SelectGroup:
      {
         if (!pullUpMenu)
         {
            *work->pAutoSelectItem = -1;
            work->autoSelectStage = kAutoSelectStage_Off;
         }
         else if (itemToSelect != -1)
         {
            SGroup *group = &work->groups[work->current_group_index];
            if (is_item_in_group(group, itemToSelect))
            {
               if (group->items[group->current_item_index] != itemToSelect)
               {
                  work->autoSelectStage = kAutoSelectStage_GroupToItem;
               }
               else
               {
                  *work->pAutoSelectItem = -1;
               }
            }
            else
            {
               // determine which way to go
               int i;
               for (i = 0; i < work->group_count; ++i)
               {
                  SGroup *otherGroup = &work->groups[i];
                  if (is_item_in_group(otherGroup, itemToSelect))
                  {
                     // desired group is below current group
                     work->dragDirection.y = AUTOSELECT_GROUP_SPEED;
                     break;
                  }
                  if (i == work->current_group_index)
                  {
                     // desired group must be above current group
                     work->dragDirection.y = -AUTOSELECT_GROUP_SPEED;
                     break;
                  }
               }
               work->dragCurrentPosition.y += work->dragDirection.y;
            }
         }
      }
      break;
   case kAutoSelectStage_GroupToItem:
      {
         if (!pullUpMenu)
         {
            *work->pAutoSelectItem = -1;
            work->autoSelectStage = kAutoSelectStage_Off;
         }
         else if (itemToSelect == -1)
         {
            work->dragCurrentPosition.x = 1;
            work->autoSelectStage = kAutoSelectStage_SelectGroup;
         }
         else
         {
            SGroup *group = &work->groups[work->current_group_index];
            if (!is_item_in_group(group, itemToSelect))
            {
               work->autoSelectStage = kAutoSelectStage_ItemToGroup;
            }
            else if (group->items[group->current_item_index] != itemToSelect)
            {
               work->dragDirection.x = AUTOSELECT_GROUPTOITEM_SPEED;
               work->dragCurrentPosition.x += work->dragDirection.x;
               if (work->dragCurrentPosition.x > work->to_item_threshold)
               {
                  work->autoSelectStage = kAutoSelectStage_SelectItem;
               }
            }
            else
            {
               *work->pAutoSelectItem = -1;
            }
         }
      }
      break;
   case kAutoSelectStage_ItemToGroup:
      {
         if (!pullUpMenu)
         {
            *work->pAutoSelectItem = -1;
            work->autoSelectStage = kAutoSelectStage_Off;
         }
         else
         {
            work->dragDirection.x = -AUTOSELECT_GROUPTOITEM_SPEED;
            work->dragCurrentPosition.x += work->dragDirection.x;
            if (work->dragCurrentPosition.x < TAB_THRESHOLD - TAB_HYSTERESIS)
            {
               work->autoSelectStage = kAutoSelectStage_SelectGroup;
            }
         }
      }
      break;
   case kAutoSelectStage_SelectItem:
      {
         if (!pullUpMenu)
         {
            *work->pAutoSelectItem = -1;
            work->autoSelectStage = kAutoSelectStage_Off
               ;
         }
         else if (itemToSelect == -1)
         {
            // just wait...
         }
         else
         {
            SGroup *group = &work->groups[work->current_group_index];
            if (!is_item_in_group(group, itemToSelect))
            {
               work->autoSelectStage = kAutoSelectStage_ItemToGroup;
            }
            else if (group->items[group->current_item_index] == itemToSelect)
            {
               *work->pAutoSelectItem = -1;
            }
            else
            {
               // determine whether to go up or down...
               int i;
               work->dragDirection.x = 0;
               for (i = 0; i < group->item_count; ++i)
               {
                  if (i == group->current_item_index)
                  {
                     // item to select is above selected item
                     work->dragDirection.y = -AUTOSELECT_ITEM_SPEED;
                     break;
                  }
                  else if (group->items[i] == itemToSelect)
                  {
                     // item to select is below selected item
                     work->dragDirection.y = AUTOSELECT_ITEM_SPEED;
                     break;
                  }
               }
               work->dragCurrentPosition.y += work->dragDirection.y;
            }
         }
      }
      break;
   }
}

/*------------------------------------------------------------------*/

static void UpdateSelection(SCommonChangeWork *work)
{
   if (!work->choosingItems)
   {
      if (abs(work->dragDirection.x) <= abs(work->dragDirection.y))
      {
         int stepY = TAB_HEIGHT + TAB_SPACE;
         int y = GetGroupY(work, 0) + TAB_HEIGHT;
         int groupIndex = (y - work->dragCurrentPosition.y)/stepY;
         if (groupIndex < 0)
         {
            groupIndex = 0;
            work->current_displayed_group_index = work->current_group_index;
         }
         else
         {
            if (groupIndex >= work->group_count)
            {
               groupIndex = work->group_count - 1;
            }

            if(GM_CheckGameStatus(STATE_PAD_DEMO))
            {
               work->shouldChangeCurrentEntry = TRUE;
            }
            else
            {
               if(work->prev_group_index == groupIndex)
               {
                  ++work->current_group_frame_count;
               }
               else
               {
                  work->prev_group_index = groupIndex;
                  work->current_group_frame_count = 0;
                  GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 );
                  // Arm fix:
                  // check if the player is changing categories quickly
                  // and if so, shake off worm
                  // fix for MGSTWO-3395
                  work->struggle_power += 5;
               }

               if(work->current_group_frame_count >= TICKS_UNTIL_GROUP_CHANGE)
               {
                  work->current_group_frame_count = 0;
                  work->shouldChangeCurrentEntry = TRUE;
               }
               else
               {
                  work->shouldChangeCurrentEntry = FALSE;
               }
            }
            
            work->current_displayed_group_index = groupIndex;
         }

         if (work->shouldChangeCurrentEntry)
         {
            if (groupIndex != work->current_group_index)
            {
               work->current_group_index = groupIndex;
               GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 );
            }
         }
      }
   }
   else
   {
      SGroup *group = &work->groups[work->current_group_index];
      int bottomY = GetItemsBottomY(work, group);
      int stepy = PH + STEP_Y;

      int iconIndex = (bottomY - work->dragCurrentPosition.y)/stepy;
      if (iconIndex < 0)
      {
         iconIndex = 0;
      }
      else if (iconIndex >= group->item_count)
      {
         iconIndex = group->item_count - 1;
      }
      if (iconIndex != group->current_item_index)
      {
         group->current_item_index = iconIndex;
         work->mCurrentItemTypeForGroup[work->current_group_index] = group->items[group->current_item_index];
         GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 );
      }

      work->current_displayed_group_index = work->current_group_index;
   }
}

/*------------------------------------------------------------------*/

int CommonChangeGetCurrentItem(SCommonChangeWork *work)
{
   SGroup *group = &work->groups[work->current_group_index];
   return group->items[group->current_item_index];
}

/*------------------------------------------------------------------*/

void UpdateMenuDragRect(SCommonChangeWork* work)
{
   // Drag touch recognizer rect encompasses just the weapon or item icon if the menu is closed to allow other front 
   // touch actions. If the menu is open, expand the rectangle to be the entire front touch screen.
   SceTouchPanelInfo panelInfo;
   SceSystemGestureRectangle rect;
   int panelWidth, panelHeight;
   SceSystemGestureTouchRecognizer* pDragRecognizer = NULL;

   sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
   panelWidth = (panelInfo.maxAaX - panelInfo.minAaX);
   panelHeight = (panelInfo.maxAaY - panelInfo.minAaY);

   if (work->dragEventID > 0)
   {
      rect.width = panelWidth/2;
      if (!work->rhs)
      {
         rect.x = panelInfo.minAaX;
      }
      else
      {
         rect.x = panelInfo.minAaX + rect.width;
      }
      rect.y = panelInfo.minAaY;
      rect.height = panelHeight;
   }
   else
   {
      get_initial_drag_rectangle(work, &panelInfo, &rect);
   }

   pDragRecognizer = GestureGetRecognizerByHandle(work->dragRecognizerHandle);
   if (pDragRecognizer)
   {
      sceSystemGestureUpdateTouchRecognizerRectangle(pDragRecognizer, &rect);
   }
}

/*------------------------------------------------------------------*/

static void Closed(SCommonChangeWork *work, int menuForcedClosed)
{
   if (work->pad->release & work->autoSelectPullUpMenuButton)
   {
      work->tapHappened = TRUE;
   }
   /* メニューが閉じている状態 */
   if (work->tapHappened)
   {
      work->tapHappened = FALSE;
      QuickChangeItemHelper(work);
   }

   if (work->flash_time)
   {
      work->flash_time--;
   }
   // flash the selected item
   int item = work->fpGetItem();
   if ((work->flash_time & 4) == 0)
   {
      work->fpPutPanel(work->panelX, PY, item, 128, 128, 0);
   }

   if (!menuForcedClosed && work->dragEventID > 0)
   {
      if (++work->open_time_tap == OPEN_TIME_TAP)
      {
         work->state = kCommonMenuState_Opening;
         work->open_time_group = 0;
         work->open_time_list = 0;

         /* 開きはじめ */
         GM_SetPlayerStatus(PLAYER_PAD_OFF | PLAYER_MENU_OPEN);
         GV_PauseOnActorSystem(GV_PAUSE_MENU);
         GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDISP02);
         CheckAvailable(work);

         // If the menu is open allow the drag recognizer to monitor the entire front screen, if the menu is closed only 
         // monitor where the weapon or item icons are located so the front touch screen can be used for other things.

         // AS MCampbell - Moved the rectangle update here so it only gets updated to the full front screen when the 
         // menu is opening. This fixes a bug where you can quicktap and then access the menu without pressing the 
         // menu icon.
         // Bug: MGSTWO-3205
         UpdateMenuDragRect(work);
      }
   }
   else
   {
      work->open_time_tap = 0;
   }
}

/*------------------------------------------------------------------*/

static void Opening(SCommonChangeWork *work)
{
   if (work->dragEventID > 0)
   {
      if (work->open_time_group < OPEN_TIME_GROUP)
      {
         work->open_time_group++;
      }
      else if (work->open_time_list < OPEN_TIME_LIST)
      {
         work->open_time_list++;
         if (work->open_time_list > work->max_open_time_list)
         {
            work->max_open_time_list = work->open_time_list;
         }
      }
      else
      {
         work->state = kCommonMenuState_Open;
      }
      work->slideAmount = 0;
      work->choosingItems = FALSE;
      work->to_item_threshold = work->dragCurrentPosition.x + TAB_THRESHOLD_OFFSET;
      if (work->to_item_threshold < TAB_THRESHOLD)
      {
         work->to_item_threshold = TAB_THRESHOLD;
      }
      UpdateSelection(work);
   }
   else
   {
      work->state = kCommonMenuState_Closing;

      // If the menu is open allow the drag recognizer to monitor the entire front screen, if the menu is closed only 
      // monitor where the weapon or item icons are located so the front touch screen can be used for other things.
      UpdateMenuDragRect(work);
   }

   DisplayGroups(work);
}

/*------------------------------------------------------------------*/

static void Open(SCommonChangeWork *work)
{
   if (work->dragEventID > 0)
   {
      // pull in the tab threshold as the thumb moves left
      int to_item_threshold = work->dragCurrentPosition.x + TAB_THRESHOLD_OFFSET;
      if (to_item_threshold < TAB_THRESHOLD)
      {
         to_item_threshold = TAB_THRESHOLD;
      }
      if (to_item_threshold < work->to_item_threshold)
      {
         work->to_item_threshold = to_item_threshold;
      }

      // check whether we're selecting groups or items
      if (work->dragCurrentPosition.x > work->to_item_threshold)
      {
         work->slideAmount += 3;
         if (work->slideAmount > 20) work->slideAmount = 20;
         work->choosingItems = TRUE;
      }
      else if (work->dragCurrentPosition.x < TAB_THRESHOLD - TAB_HYSTERESIS)
      {
         work->slideAmount -= 3;
         if (work->slideAmount < 0) work->slideAmount = 0;
         work->choosingItems = FALSE;
      }
      UpdateSelection(work);
   }
   else
   {
      work->state = kCommonMenuState_Closing;

      // If the menu is open allow the drag recognizer to monitor the entire front screen, if the menu is closed only 
      // monitor where the weapon or item icons are located so the front touch screen can be used for other things.
      UpdateMenuDragRect(work);
   }

   DisplayGroups(work);
}

/*------------------------------------------------------------------*/

static void Closing(SCommonChangeWork *work)
{
   if (work->open_time_list > 0)
   {
      --work->open_time_list;
   }
   else if (work->open_time_group > 0)
   {
      --work->open_time_group;
   }
   else
   {
      SGroup *group = &work->groups[work->current_group_index];
      int item = group->items[group->current_item_index];
      if (item != work->fpGetItem())
      {
         if (work->fpCanSelect(item))
         {
            work->fpSwitchItem(item);
            GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
            work->flash_time = 32;
         }
      }
      else if (work->max_open_time_list == 0 && !work->shouldChangeCurrentEntry && !work->tapHappened)
      {
         QuickChangeItemHelper(work);
      }

      work->state = kCommonMenuState_Closed;
      work->open_time_tap = 0;

      GM_ResetPlayerStatus(PLAYER_PAD_OFF | PLAYER_MENU_OPEN);

      GV_PauseOffActorSystem(GV_PAUSE_MENU);
   }

   DisplayGroups(work);
}

/*------------------------------------------------------------------*/

// first person view is defined by this variable
// don't want to be able to switch items or weapons in first person
extern int PL_SubjectToggle;

/*------------------------------------------------------------------*/
void CommonChangeForceCloseMenu(SCommonChangeWork* work)
{
   if (work->dragEventID != 0)
   {
      work->dragEventID = 0;
      work->state = kCommonMenuState_Closed;
      work->open_time_tap = 0;

      GM_ResetPlayerStatus(PLAYER_PAD_OFF | PLAYER_MENU_OPEN);
      GV_PauseOffActorSystem(GV_PAUSE_MENU);
   }
}

/*------------------------------------------------------------------*/
void CommonChangeAct(SCommonChangeWork *work)
{
   extern int gBP_Demo_Active;
   int menuForcedClosed = GM_CheckPlayerStatus(work->forcedClosedPlayerStatus) || GM_CheckMenuStatus(work->forcedClosedMenuStatus) || PL_SubjectToggle != 0;

   GestureInitialize(work);

   if (GM_CheckGameStatus(STATE_PAD_DEMO) && !menuForcedClosed)
   {
      AutoSelectUpdate(work);
   }
   else
   {
      int controlDisabled = GV_PadControlDisabled(work->rhs ? PAD_R2 : PAD_L2);
      if (!menuForcedClosed && !controlDisabled)
      {
         CommonChangeGestureUpdate(work);
      }
      else
      {
         work->dragEventID = 0;
      }
   }

   if (work->struggle_power > 0)
   {
      --work->struggle_power;
   }

   GM_ResetMenuStatus(work->openMenuStatus);

   /* ポーズ中 */
   /* 非表示設定 */
   if ( (GV_PauseLevel & GV_PAUSE_PAUSE)
      || GM_CheckMenuStatus(work->offMenuStatus)
      || GM_CheckPlayerStatus(PLAYER_FORCE | PLAYER_LOCKER))
   {
      return;
   }

   GM_GageStatus = GM_GAGE_STATE_APPEAR;
   GM_ResetMenuStatus(MENU_MENU_NEWPRESS);

   if (menuForcedClosed || (gBP_Demo_Active && !GM_CheckGameStatus(STATE_PAD_DEMO)))
   {
      work->state = kCommonMenuState_Closed;
   }

   switch (work->state)
   {
   case kCommonMenuState_Closed:
      Closed(work, menuForcedClosed);
      break;
   case kCommonMenuState_Opening:
      Opening(work);
      GM_SetMenuStatus(work->openMenuStatus);
      GM_GageStatus = GM_GAGE_STATE_DISAPPEAR;
      break;
   case kCommonMenuState_Open:
      Open(work);
      GM_SetMenuStatus(work->openMenuStatus);
      GM_GageStatus = GM_GAGE_STATE_DISAPPEAR;
      break;
   case kCommonMenuState_Closing:
      Closing(work);
      GM_SetMenuStatus(work->openMenuStatus);
      GM_GageStatus = GM_GAGE_STATE_DISAPPEAR;
      break;
   }
}

/*------------------------------------------------------------------*/

void CommonChangeDie(SCommonChangeWork *work)
{
   GestureReleaseRecognizerHandle(work->dragRecognizerHandle);
   GestureReleaseRecognizerHandle(work->tapRecognizerHandle);
   
   work->dragRecognizerHandle = 0;
   work->tapRecognizerHandle = 0;
   work->gesture_item_initialized = FALSE;

   GV_PauseOffActorSystem(GV_PAUSE_MENU);
}
