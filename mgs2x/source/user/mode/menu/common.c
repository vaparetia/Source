//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   common.c
   装備メニュー選択形式非依存共通ルーチン

   200/07/26 K.Takabe
   $Id: common.c,v 1.2 2002/12/05 18:42:02 takaki Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"menu.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "libfs.h"

#if defined(BP_VITA)
extern void MENU_Item_Panel_ResetAnim(int isEnabled);
#endif

#include "sonoyama/raiden/pl_work.h"

/*------------------------------------------------------------------*/
/* 装備自動選択関連 */
int		MENU_AutoSelectItem = -1 ;
int		MENU_AutoSelectWeapon = -1 ;

/* 装備メニュー関連ステータス */
int				MENU_StatusFlag = 0 ;



/*------------------------------------------------------------------*/
/* 武器のクイックチェンジ */
int MENU_QuickChangeWeapon( void )
{
	int		change = 0 ;
	if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
		/* 旧方式クイックチェンジ */
		if ( GM_Weapon == WP_None ) {
			if ( GM_WeaponPrev != WP_None &&
				( GM_WeaponNum( GM_WeaponPrev ) > 0 ||
				 ( GM_WeaponNum( GM_WeaponPrev ) == 0 && 
				  ( GM_WeaponTypes[ GM_WeaponPrev ] & WP_TYPE_BODY ) ) ) ) {
				GM_Weapon = GM_WeaponPrev ;
				change = 1 ;
				GM_WeaponChanged = 1 ;
			}
		} else {
			GM_WeaponPrev = GM_Weapon ;
			GM_Weapon = WP_None ;
			change = 1 ;
			GM_WeaponChanged = 1 ;
		}
	} else {
		/* 新方式クイックチェンジ（１つ前のものとチェンジ） */
		if ( GM_WeaponPrev == GM_Weapon ) GM_WeaponPrev = WP_None ;
		if ( ( GM_WeaponPrev != GM_Weapon ) && 
			( GM_WeaponPrev == WP_None || 
			 ( GM_WeaponNum( GM_WeaponPrev ) > 0 ||
			  ( GM_WeaponNum( GM_WeaponPrev ) == 0 &&
			   ( GM_WeaponTypes[ GM_WeaponPrev ] & WP_TYPE_BODY ) ) ) ) ) {
#if 1
			if ( PL_CheckNoUseWeapon( GM_WeaponPrev ) == 0 ){
				int		old_weapon ;
				old_weapon = GM_Weapon ;
				GM_Weapon = GM_WeaponPrev ;
				GM_WeaponPrev = old_weapon ;
				GM_WeaponChanged = 1 ;
				change = 1 ;
			} else {
				/* 装備禁止アイテムにチェンジしようとした */
			}
#else
			int		old_weapon ;
			old_weapon = GM_Weapon ;
			GM_Weapon = GM_WeaponPrev ;
			GM_WeaponPrev = old_weapon ;
			GM_WeaponChanged = 1 ;
			change = 1 ;
#endif
		}
	}

   // Arm fix:
   // make sure weapon is holstered when returning to the weapon with another quick tap
   // fixes MGSTWO-3062
   if (GM_WeaponChanged && GM_Weapon == WP_None)
   {
      PlayerPad.weaponState = WS_Holster;
   }
   // end Arm fix

   return ( change );
}

/* アイテムのクイックチェンジ */
int MENU_QuickChangeItem( void )
{
	int		change = 0 ;
	/* クイックチェンジ */
	if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
		/* 旧方式クイックチェンジ */
		if ( GM_Item == IT_None ) {
			if ( GM_ItemPrev != IT_None &&
				( GM_ItemNum( GM_ItemPrev ) > 0 ) ) {
				GM_Item = GM_ItemPrev ;
				GM_ItemChanged = 1 ;
				change = 1 ;
			}
		} else {
			GM_ItemPrev = GM_Item ;
			GM_Item = IT_None ;
			GM_ItemChanged = 1 ;
			change = 1 ;
		}
	} else {
		/* 新方式クイックチェンジ（１つ前のものとチェンジ） */
		if ( GM_ItemPrev == GM_Item ) GM_ItemPrev = IT_None ;
		if ( ( GM_ItemPrev != GM_Item ) &&
			( GM_ItemPrev == IT_None || GM_ItemNum( GM_ItemPrev ) > 0 ) ){
#if 1
			if ( PL_CheckNoUseItem( GM_ItemPrev ) == 0 ){
				int		old_item ;
				old_item = GM_Item ;
				GM_Item = GM_ItemPrev ;
				GM_ItemPrev = old_item ;
				GM_ItemChanged = 1 ;
				change = 1 ;
			} else {
				/* 装備禁止アイテムにチェンジしようとした */
			}
#else
				int		old_item ;
				old_item = GM_Item ;
				GM_Item = GM_ItemPrev ;
				GM_ItemPrev = old_item ;
				GM_ItemChanged = 1 ;
				change = 1 ;
#endif
		}
	}

#if defined(BP_VITA)
   // Enable the icon animation on the items panel when the AP sensor is equipped.
   if (GM_Item == IT_VibSensor) 
   {
      MENU_Item_Panel_ResetAnim(1);
   }
   else
   {
      MENU_Item_Panel_ResetAnim(0);
   }
#endif

	return ( change );
}
/* 武器装備の変更 */
int MENU_ChangeWeapon( int weapon )
{
   if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
		/* 旧方式クイックチェンジ */
		if ( GM_Weapon != WP_None ){
			GM_WeaponPrev = GM_Weapon ;
		}
		GM_Weapon = weapon ;
		GM_WeaponChanged = 1 ;
	} else {
		/* 新方式クイックチェンジ（１つ前のものとチェンジ） */
		GM_WeaponPrev = GM_Weapon ;
		GM_Weapon = weapon ;
		GM_WeaponChanged = 1 ;
	}

   // Arm fix:
   // make sure the weapon state is set when switching away from a weapon
   // fixes MGSTWO-2521
   if (GM_WeaponChanged && GM_Weapon == WP_None)
   {
      PlayerPad.weaponState = WS_Holster;
   }
   // end Arm fix
   
   return ( 1 );
}
/* アイテム装備の変更 */
int MENU_ChangeItem( int item )
{
	if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
		/* 旧方式クイックチェンジ */
		if ( GM_Item != IT_None ) {
			GM_ItemPrev = GM_Item ;
		}
		GM_Item = item ;
		GM_ItemChanged = 1 ;
	} else {
		/* 新方式クイックチェンジ（１つ前のものとチェンジ） */
		GM_ItemPrev = GM_Item ;
		GM_Item = item ;
		GM_ItemChanged = 1 ;
	}

#if defined(BP_VITA)
   // Enable the icon animation on the items panel when the AP sensor is equipped.
   if (GM_Item == IT_VibSensor) 
   {
      MENU_Item_Panel_ResetAnim(1);
   }
   else
   {
      MENU_Item_Panel_ResetAnim(0);
   }
#endif

	return 1;
}

/* 武器基本チェック */
int MENU_DefaultCheckWeapon( void )
{
	int		change = 0 ;
	int		type ;

	/* 装備中のものがなくなった */
	if ( GM_Weapon != WP_None && 
		( GM_WeaponNum( GM_Weapon ) < 0 ||
		 ( GM_WeaponNum( GM_Weapon ) == 0 &&
		  !( GM_WeaponTypes[ GM_Weapon ] & WP_TYPE_BODY ) ) ) ) {
		GM_Weapon = WP_None ;
		GM_WeaponChanged = 1 ;
		change = 1 ;
	}
	/* 使用不可にされた */
	type = GM_WeaponTypes[ GM_Weapon ] ;
	if ( ( ( PL_MenuNoUseWeaponScn | PL_MenuNoUseWeapon ) & ( I64(1) << GM_Weapon ) ) ||
		 ( type & PL_MenuNoUseWeaponType ) ) {
		GM_WeaponPrev = GM_Weapon ;
		GM_Weapon = WP_None ;
		GM_WeaponChanged = 1 ;
		change = 1 ;		
	}
	return ( change );
}

/* アイテム基本チェック */
int MENU_DefaultCheckItem( void )
{
	int		change = 0 ;
	int		type ;

	/* 装備中のものがなくなった */
	if ( GM_Item != IT_None && GM_ItemNum( GM_Item ) <= 0 ) {
		GM_Item = IT_None ;
		GM_ItemChanged = 1 ;
		change = 1 ;
	}
	/* 使用不可にされた */
	type = GM_ItemTypes[ GM_Item ] ;
	if ( ( ( PL_MenuNoUseItemScn | PL_MenuNoUseItem ) & ( I64(1) << GM_Item ) ) ||
		 ( type & PL_MenuNoUseItemType ) ) {
		GM_ItemPrev = GM_Item ;
		GM_Item = IT_None ;
		GM_ItemChanged = 1 ;
		change = 1 ;		
	}
	return ( change );
}

/*------------------------------------------------------------------*/
void MENU_UpdateItemExplainMessage( int num )
{
	void	*mes ;
	if ( num == IT_Uniform && !( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ){
		num = MAX_ITEMS + 0 ;
	}
	if ( num == IT_Diazepam && !( GM_Configuration & GM_CONFIG_STORY_TANKER ) ){
		num = MAX_ITEMS + 1 ;
	}
	if ( ( num == IT_Ration ) &&
		( GM_ItemNum( num ) == 0 ) &&
		( ( MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM ) != 0 ) ){
		num = MAX_ITEMS + 2 ;
	}
   if ( !BP_Area_EU() )
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   {
	   mes = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( RESOURCE_ITEM_MENU_EXPLAIN, num ));
   }
   else
//#else
   {
	   mes = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( RESOURCE_ITEM_MENU_EXPLAIN, num ));
   }
//#endif
	if ( mes == NULL ) return ;
	MENU_CreateExplainText( mes );
	/* テクスチャ展開時に多重変更をチェックしているので毎フレーム
	   説明文テクスチャ生成関数を呼び出しても問題ない */
}
void MENU_UpdateWeaponExplainMessage( int num )
{
	/* 説明文テクスチャを更新 */
	void	*mes ;
	if ( num == WP_Socom && GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 2 ;
	}
	if ( num == WP_Aks && GM_PlayerStateFlag & PL_AK_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 1 ;
	}
	if ( num == WP_Usp && GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 0 ;
	}
	if ( num == WP_Mic && MENU_StatusFlag & MENU_STATUS_MIC_EXPLAIN2 ){
		num = MAX_WEAPONS + 3 ;
	}
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   if ( !BP_Area_EU() )
   {
	   mes = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( RESOURCE_WEAPON_MENU_EXPLAIN, num ));
   }
   else
   {
//#else
	   mes = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( RESOURCE_WEAPON_MENU_EXPLAIN, num ));
   }
//#endif
	if ( mes == NULL ) return ;
	MENU_CreateExplainText( mes );
	/* テクスチャ展開時に多重変更をチェックしているので毎フレーム
	   説明文テクスチャ生成関数を呼び出しても問題ない */
}
