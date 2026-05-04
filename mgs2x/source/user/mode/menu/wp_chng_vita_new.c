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

/*------------------------------------------------------------------*/

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

#define array_size(x) (sizeof(x)/sizeof(x[0]))

// order (bottom to top)
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

/*------------------------------------------------------------------*/

static int CheckEnableChanging(int item)
{
   return !PL_CheckNoUseWeapon(item);
}

/*------------------------------------------------------------------*/

static int WeaponAvailable(int item)
{
   if (GM_WeaponNum(item) > 0
      || (GM_WeaponNum(item) == 0 && (GM_WeaponTypes[item] & WP_TYPE_BODY) != 0))
   {
      return TRUE;
   }
   return FALSE;
}

/*------------------------------------------------------------------*/

static int GetWeapon()
{
   return GM_Weapon;
}

/*------------------------------------------------------------------*/

static void Act(SCommonChangeWork *work)
{
   // changes item somehow...
   if (MENU_DefaultCheckWeapon())
   {
      GM_SeSet(GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01);
   }

   CommonChangeAct(work);
}

/*------------------------------------------------------------------*/

static void Die(SCommonChangeWork *work)
{
   CommonChangeDie(work);
}

/*------------------------------------------------------------------*/
void UpdateWeaponPanelPropsAfterListFlip(void* work)
{
   extern int gBP_FlipWeaponItemLists;

   SCommonChangeWork* menu_work = work;
   SceSystemGestureRectangle rect;
   SceTouchPanelInfo panelInfo;
   int panelWidth, panelHeight;
   SceSystemGestureTouchRecognizer* pTapRecognizer = NULL;

   if (gBP_FlipWeaponItemLists)
   {
      menu_work->panelX = IT_PX; // If flipped, weapons appear where the items menu appears
      menu_work->rhs = FALSE;
      menu_work->explainTextX = 100 + PW;
   }
   else
   {
      menu_work->panelX = WP_PX; 
      menu_work->rhs = TRUE;
      menu_work->explainTextX = 100;
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

void  *NewWeaponChangeVita(int which)
{
   SCommonChangeWork *work = (SCommonChangeWork *) GV_CreateActor(GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(SCommonChangeWork), PLAYER_MENU_ACTOR_PRIO);
   if (work != NULL)
   {
      GV_SetActor(&work->actor, Act, Die);

      CommonChangeInitialize(work, which);
      UpdateWeaponPanelPropsAfterListFlip(work);
      work->maxGroups = array_size(GroupInfoLists);
      work->groupInfoLists = GroupInfoLists;
      work->fpItemAvailable = WeaponAvailable;
      work->fpPutPanel = MENU_PutWeaponPanel;
      work->fpCanSelect = CheckEnableChanging;
      work->fpQuickChangeItem = MENU_QuickChangeWeapon;
      work->fpUpdateExplainMessage = MENU_UpdateWeaponExplainMessage;
      work->fpSwitchItem = MENU_ChangeWeapon;
      work->fpGetItem = GetWeapon;
      work->pItemNames = GM_WeaponNames;
      work->openMenuStatus = MENU_WEAPON_OPEN;
      work->offMenuStatus = MENU_WEAPON_OFF | MENU_ITEM_OPEN;
      work->forcedClosedMenuStatus = MENU_WEAPON_DISABLE;
      work->forcedClosedPlayerStatus = PLAYER_WEAPON_DISABLE | PLAYER_WEAPON_QUICK_ONLY;
      work->pAutoSelectItem = &MENU_AutoSelectWeapon;
      work->autoSelectPullUpMenuButton = PAD_R2;
   }
   return work;
}
