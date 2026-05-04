#pragma once

#ifndef __COMMON_CHNG_VITA_H__
#define __COMMON_CHNG_VITA_H__

#include <systemgesture.h>
#include "gesture_vta.h"

/*------------------------------------------------------------------*/

typedef struct GroupInfoTag
{
   int *lists;
   int list_size;
}
GroupInfo;

#define array_size(x) (sizeof(x)/sizeof(x[0]))

#define MAX_ITEMS (6) // can only display 5 but there's a max of 6 when in debug
#define MAX_GROUPS (15)

typedef struct SGroupTag
{
   int type;
   int item_count;
   int items[MAX_ITEMS];
   int current_group_index;
   int current_item_index;
}
SGroup;

typedef enum ECommonMenuStateTag
{
   kCommonMenuState_Closed = 0,
   kCommonMenuState_Opening,
   kCommonMenuState_Open,
   kCommonMenuState_Closing
}
ECommonMenuState;

typedef enum
{
   kAutoSelectStage_Off,
   kAutoSelectStage_Enter,
   kAutoSelectStage_SelectGroup,
   kAutoSelectStage_GroupToItem,
   kAutoSelectStage_ItemToGroup,
   kAutoSelectStage_SelectItem,
   kAutoSelectStage_Exit
}
EAutoSelectStage;

typedef struct SCommonChangeWorkTag
{
   GV_ACT actor;

   GV_PAD *pad;

   ECommonMenuState state;

   int group_count;
   SGroup groups[MAX_GROUPS];
   int current_group_index;
   int prev_group_index;
   int current_displayed_group_index;
   int current_group_frame_count;

   int shouldChangeCurrentEntry;
   int max_open_time_list;

   int open_time_tap;
   int open_time_group;
   int open_time_list;
   int to_item_threshold;
   int choosingItems;
   int slideAmount;
   int flash_time;

   int panelX;
   int rhs;
   int explainTextX;
   int maxGroups;
   GroupInfo *groupInfoLists;

   int gesture_item_initialized;
   GestureRecognizerHandle dragRecognizerHandle;
   GestureRecognizerHandle tapRecognizerHandle;
   SceUInt32 dragEventID;
   SceSystemGestureTouchState dragEventState;
   SceSystemGestureVector2 dragCurrentPosition;
   SceSystemGestureVector2 dragDirection;
   int tapHappened;

   int (*fpItemAvailable)(int item);
   void (*fpPutPanel)(int x, int y, int item, int alpha1, int alpha2, int flags);
   int (*fpCanSelect)(int item);
   int (*fpSwitchItem)(int item);
   int (*fpQuickChangeItem)();
   void (*fpUpdateExplainMessage)(int item);
   int (*fpGetItem)();
   char **pItemNames;

   int openMenuStatus;
   int offMenuStatus;
   int forcedClosedMenuStatus;
   unsigned long long forcedClosedPlayerStatus;

   // storage for current item type per group
   int mCurrentItemTypeForGroup[MAX_GROUPS];

   // auto selection (for tutorials/cutscenes)
   int *pAutoSelectItem;
   EAutoSelectStage autoSelectStage;
   int autoSelectPause;
   int autoSelectPullUpMenuButton;

   // for sea lice throw off
   int struggle_power;
}
SCommonChangeWork;

extern void CommonChangeInitialize(SCommonChangeWork *work, int which);
extern void CommonChangeAct(SCommonChangeWork *work);
extern void CommonChangeDie(SCommonChangeWork *work);

extern int CommonChangeGetCurrentItem(SCommonChangeWork *work);

extern void CommonChangeForceCloseMenu(SCommonChangeWork* work);

#endif // __COMMON_CHNG_VITA_H__
