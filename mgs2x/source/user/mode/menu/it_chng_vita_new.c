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
#include "menu.h"

#include "common_chng_vita.h"

extern int NewPlayerRecoverCold(void);
extern int GM_ItemChanged;

/*------------------------------------------------------------------*/

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

// this controls the display order (bottom to top)
// probably want to sort it by usage
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

/*------------------------------------------------------------------*/

/* 変更不可のチェック */
static int CheckEnableChanging(int item)
{
   /* 無い */
   if (item != IT_None && GM_ItemNum(item) == 0)
   {
      return 0;
   }
   return !PL_CheckNoUseItem(item);
}

/*------------------------------------------------------------------*/

static int ItemAvailable(int item)
{
   if (GM_ItemNum(item) > 0
      || (item == IT_Ration && (MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM) != 0))
   {
      return TRUE;
   }
   return FALSE;
}

/*------------------------------------------------------------------*/

/* アイテム使用処理 */
static void UseItem(SCommonChangeWork *work)
{
   if (work->pad->press & PL_PAD_ITEMUSE)
   {
      SGroup *group = &work->groups[work->current_group_index];
      int item = group->items[group->current_item_index];
      if (GM_ItemNum(item) > 0)
      {
         switch (item)
         {
         case IT_Ration:         /* レーション */
            if (GM_Vitality + GM_VitalityAdjust < GM_VitalityMax
               || GM_CheckPlayerStatus(PLAYER_BLOOD_DROP))
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
            if ( GM_CheckPlayerStatus(PLAYER_COLD) )
            {
               GM_DecrementItem(IT_Medicine, 1);
               NewPlayerRecoverCold();
               GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02);
            }
            break;

         case IT_Styptic:        /* 止血剤 */
            if ( GM_CheckPlayerStatus(PLAYER_BLOOD_DROP) )
            {
               GM_DecrementItem(IT_Styptic, 1);
               GM_ResetPlayerStatus(PLAYER_BLOOD_DROP);
               GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02);
            }
            break;

         case IT_Diazepam:          /* ジアゼパム */
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
}

/*------------------------------------------------------------------*/

static int GetItem()
{
   return GM_Item;
}

/*------------------------------------------------------------------*/

static void Act(SCommonChangeWork *work)
{
   GM_ItemChanged = 0;

   // changes item somehow...
   if (MENU_DefaultCheckItem())
   {
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
   }

   CommonChangeAct(work);

   // Arm fix:
   // check if the player is changing categories quickly
   // and if so, shake off worm
   // fix for MGSTWO-3395
   if (CommonChangeGetCurrentItem(work) == IT_Ration)
   {
      if (MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM)
      {
         if (work->struggle_power > 64)
         {
            extern void ShipwormShakeOffRation();
            ShipwormShakeOffRation();
            work->struggle_power = 0;
         }
      }
   }
   else
   {
      work->struggle_power = 0;
   }

   if (work->state == kCommonMenuState_Open)
   {
      UseItem(work);
   }
}

/*------------------------------------------------------------------*/

static void Die(SCommonChangeWork *work)
{
   CommonChangeDie(work);
}

/*------------------------------------------------------------------*/
void UpdateItemPanelPropsAfterListFlip(void* work)
{
   extern int gBP_FlipWeaponItemLists;

   SCommonChangeWork* menu_work = work;
   SceSystemGestureRectangle rect;
   SceTouchPanelInfo panelInfo;
   int panelWidth, panelHeight;
   SceSystemGestureTouchRecognizer* pTapRecognizer = NULL;

   if (gBP_FlipWeaponItemLists)
   {
      menu_work->panelX = WP_PX; // If flipped, items appear where the weapons menu appears
      menu_work->rhs = TRUE;
      menu_work->explainTextX = 100;
   }
   else
   {
      menu_work->panelX = IT_PX;
      menu_work->rhs = FALSE;
      menu_work->explainTextX = 100 + PW;
   }

   sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo);
   panelWidth = (panelInfo.maxAaX - panelInfo.minAaX);
   panelHeight = (panelInfo.maxAaY - panelInfo.minAaY);

   rect.x = panelInfo.minAaX + (menu_work->panelX)*panelWidth/VR_WIDTH;
   rect.y = panelInfo.minAaY + (PY)*panelHeight/VR_HEIGHT;
   rect.width = (PW)*panelWidth/VR_WIDTH;
   rect.height = (PH)*panelHeight/VR_HEIGHT;

   pTapRecognizer = GestureGetRecognizerByHandle(menu_work->tapRecognizerHandle);
   if (pTapRecognizer)
   {
      sceSystemGestureUpdateTouchRecognizerRectangle(pTapRecognizer, &rect);
   }
}

/*------------------------------------------------------------------*/

void  *NewItemChangeVita(int which)
{
   SCommonChangeWork *work = (SCommonChangeWork *) GV_CreateActor(GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(SCommonChangeWork), PLAYER_MENU_ACTOR_PRIO);
   if (work != NULL)
   {
      GV_SetActor(&work->actor, Act, Die);

      CommonChangeInitialize(work, which);
      UpdateItemPanelPropsAfterListFlip(work);
      work->maxGroups = array_size(GroupInfoLists);
      work->groupInfoLists = GroupInfoLists;
      work->fpItemAvailable = ItemAvailable;
      work->fpPutPanel = MENU_PutItemPanel;
      work->fpCanSelect = CheckEnableChanging;
      work->fpQuickChangeItem = MENU_QuickChangeItem;
      work->fpUpdateExplainMessage = MENU_UpdateItemExplainMessage;
      work->fpSwitchItem = MENU_ChangeItem;
      work->fpGetItem = GetItem;
      work->pItemNames = GM_ItemNames;
      work->openMenuStatus = MENU_ITEM_OPEN;
      work->offMenuStatus = MENU_ITEM_OFF | MENU_WEAPON_OPEN;
      work->forcedClosedMenuStatus = MENU_ITEM_DISABLE;
      work->forcedClosedPlayerStatus = PLAYER_ITEM_DISABLE | PLAYER_ITEM_QUICK_ONLY;
      work->pAutoSelectItem = &MENU_AutoSelectItem;
      work->autoSelectPullUpMenuButton = PAD_L2;
   }
   return work;
}
