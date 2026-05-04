//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   getitem.c
   アイテム取得処理

   2000/06/09 M.Sonoyama
   $Id: getitem.c,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "BP_TrophySystem.h"

/*----------------------------------------------------------------*/

char	*GM_WeaponNames[] = {
    "None", "M9", "USP", "SOCOM", "PSG1", 
	"RGB6", "NIKITA", "STINGER", "CLAYMORE", "C4", 
	"CHAFF.G", "STUN.G", "D.MIC", "HF.BLADE", "COOLANT", 
    "AKS-74U", "MAGAZINE", "GRENADE", "M4", "PSG1-T", 
	"D.MIC", "BOOK"
} ;

int	GM_WeaponTypes[] = {
    NONE_TYPE, M92_TYPE, USP_TYPE, SOCOM_TYPE, PSG_TYPE, 
	RGB_TYPE, NIKITA_TYPE, STG_TYPE, CLAY_TYPE, C4_TYPE, 
	CHAFF_TYPE, STUN_TYPE, MIC_TYPE, BLADE_TYPE, SPRAY_TYPE, 
    AKS_TYPE, GRENADE_TYPE, GRENADE_TYPE, M4_TYPE, PSG_TYPE, 
	MIC_TYPE, BOOK_TYPE
} ;

char	*GM_ItemNames[] = {
    "None", "RATION", "SCOPE", "MEDICINE", "BANDAGE", 
#ifdef ENGLISH
	"PENTAZEMIN", "B.D.U", "B.ARMOR", "STEALTH", "MINE.D", 
#else
	"DIAZEPAM", "B.D.U", "B.ARMOR", "STEALTH", "MINE.D", 
#endif
	"SENSOR A", "SENSOR B", "N.V.G", "THERM.G", "SCOPE", 
	"DG.CAMERA", "BOX 1", "CIGS", "CARD", "SHAVER", 
	"PHONE", "CAMERA", "BOX 2", "BOX 3", "WET BOX", 
	"AP SENSR", "BOX 4", "BOX 5", "RAZOR", "SCM.SUPR", 
	"AK.SUPR", "CAMERA", "BANDANA", "DOG TAGS", "MO DISC",
	"USP.SUPR", "SP.WIG", "WIG A", "WIG B", "WIG C",
	"WIG D"
} ;

int	GM_ItemTypes[] = {
    IT_TYPE_NORMAL, IT_TYPE_NORMAL, IT_TYPE_NORMAL, IT_TYPE_NORMAL, IT_TYPE_NORMAL, 
	IT_TYPE_NORMAL, UNIFORM_TYPE, JACKET_TYPE, IT_TYPE_NORMAL, IT_TYPE_NORMAL, 
	IT_TYPE_NORMAL, BSENSB_TYPE, GOGGLES_TYPE, GOGGLES_TYPE, SCOPE_TYPE, 
	DGCAMERA_TYPE, C_BOX_TYPE, CIGAR_TYPE, IT_TYPE_NORMAL, IT_TYPE_NORMAL, 
	IT_TYPE_NORMAL, DGCAMERA_TYPE, C_BOX_TYPE, C_BOX_TYPE, C_BOX_TYPE, 
	IT_TYPE_NORMAL, C_BOX_TYPE, C_BOX_TYPE, IT_TYPE_NORMAL,	IT_TYPE_ONLY_ONE, 
	IT_TYPE_ONLY_ONE, IT_TYPE_NORMAL, BANDANA_TYPE, IT_TYPE_NORMAL, IT_TYPE_NORMAL,
	IT_TYPE_ONLY_ONE, MUGENWIG_TYPE, IT_TYPE_ONLY_ONE, IT_TYPE_ONLY_ONE, IT_TYPE_ONLY_ONE, 
	IT_TYPE_ONLY_ONE
} ;

/*----------------------------------------------------------------*/

static	short	*gm_weapons = NULL, *gm_weaponsmax = NULL ;
static	short	*gm_items = NULL, *gm_itemsmax = NULL ;

/*----------------------------------------------------------------*/

static void bp_check_box_trophy( int id )
{
   //N.B. can't get wet box in plant mission.  Don't require it for trophy.
   switch( id )
   {
   case IT_CBBox:
   case IT_CBBoxB:
   case IT_CBBoxC:
   case IT_CBBoxD:
   case IT_CBBoxE:
      {
         //Picked up a box item.
         if( ( gm_items[IT_CBBox] != 0 )
            &&( gm_items[IT_CBBoxB] != 0 )
            &&( gm_items[IT_CBBoxC] != 0 )
            &&( gm_items[IT_CBBoxD] != 0 )
            &&( gm_items[IT_CBBoxE] != 0 )
            )
         {
            BP_TrophySystem_UnlockTrophy( kTRP_AllBoxes );
         }
      }
      break;
   case IT_Camera:
      {
         //Picked up digital camera.
         BP_TrophySystem_UnlockTrophy( kTRP_GotDigitalCamera );
      }
      break;
   }
}

static	int	GetItem( id, n )
int			id, n ;
{
	int		type ;

	ASSERT( n > 0 ) ;
	ASSERT( id > 0 && id < MAX_ITEMS ) ;
	type = GM_ItemTypes[ id ] ;
	if ( gm_items[ id ] == gm_itemsmax[ id ] ) return GM_ITEMGET_FULL ;
	if ( gm_items[ id ] < 0 ) {
		gm_items[ id ] = 0 ;
	}
	if ( type & IT_TYPE_ONLY_ONE ) gm_items[ id ] = 1 ;
	else						   gm_items[ id ] += n ;
	if ( gm_items[ id ] > gm_itemsmax[ id ] ) {
		gm_items[ id ] = gm_itemsmax[ id ] ; 
	}

   bp_check_box_trophy( id );

	return GM_ITEMGET_OK ;
}

/*----------------------------------------------------------------*/

static	int	GetWeapon( spec, id, n )
int			spec, id, n ;
{
	int		type ;

	ASSERT( id > 0 && id < MAX_WEAPONS ) ;
	type = GM_WeaponTypes[ id ] ;
	if ( gm_weapons[ id ] < 0 ) {
		if ( type & WP_TYPE_BODY ) {
			if ( spec == GM_WEAPON_BODY_BOX ) {
				gm_weapons[ id ] = 0 ;
			} else {
				return GM_ITEMGET_GET_WEAPON_FIRST ;
			}
		} else {
			gm_weapons[ id ] = 0 ;			
		}
	}
	if ( gm_weapons[ id ] == gm_weaponsmax[ id ] ) return GM_ITEMGET_FULL ;
	if ( type & WP_TYPE_ONLY_ONE ) gm_weapons[ id ] = 1 ;
	else						   gm_weapons[ id ] += n ;
	if ( gm_weapons[ id ] > gm_weaponsmax[ id ] ) {
		gm_weapons[ id ] = gm_weaponsmax[ id ] ; 
	}
	return GM_ITEMGET_OK ;
}

/*----------------------------------------------------------------*/

/* アイテムボックスから取得 */
int		GM_GetItem( spec, id, n )
int		spec, id, n ;
{
	if ( spec == GM_ITEM_BOX ) {
		return GetItem( id, n ) ;
	} else {
		return GetWeapon( spec, id, n ) ;
	}
}

/*----------------------------------------------------------------*/

/* 武器、装備品管理配列へのポインタを
   主人公によって切り替える。
   0 : スネーク 1 : ライデン */
void		GM_SetCurrentItemSet( int which )
{
	if ( which == 0 ) {
		gm_weapons = _GM_Weapons ;
		gm_weaponsmax = _GM_WeaponsMax ;
		gm_items = _GM_Items ;
		gm_itemsmax = _GM_ItemsMax ;
	} else {
		gm_weapons = _GM_WeaponsR ;
		gm_weaponsmax = _GM_WeaponsMaxR ;
		gm_items = _GM_ItemsR ;
		gm_itemsmax = _GM_ItemsMaxR ;
	}
}

int			GM_IncrementWeapon( int id, int n ) 
{
	return 	GetWeapon( GM_WEAPON_BOX, id, n ) ;
}

int			GM_IncrementItem( int id, int n )
{
	return 	GetItem( id, n ) ;
}

int			GM_DecrementWeapon( int id, int n )
{
	/* 無限処理 */
	if ( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_MUGEN ) return gm_weapons[ id ] ;
	gm_weapons[ id ] -= n ;
	if ( gm_weapons[ id ] < 0 ) gm_weapons[ id ] = 0 ;
	return gm_weapons[ id ] ;
}

int			GM_DecrementItem( int id, int n )
{
	gm_items[ id ] -= n ;
	if ( gm_items[ id ] < 0 ) gm_items[ id ] = 0 ;
	return gm_items[ id ] ;
}

int			GM_SetWeaponNum( int id, int n )
{
	gm_weapons[ id ] = n ;
	if ( gm_weapons[ id ] > gm_weaponsmax[ id ] ) {
		gm_weapons[ id ] = gm_weaponsmax[ id ] ;
	}
	return gm_weapons[ id ] ;
}

int			GM_SetItemNum( int id, int n )
{
	gm_items[ id ] = n ;
	if ( gm_items[ id ] > gm_itemsmax[ id ] ) {
		gm_items[ id ] = gm_itemsmax[ id ] ;
	}
	return gm_items[ id ] ;
}

int			GM_WeaponNum( int id )
{
	return gm_weapons[ id ] ;
}

int			GM_ItemNum( int id )
{
	return gm_items[ id ] ;
}

int			GM_WeaponMaxNum( int id )
{
	return gm_weaponsmax[ id ] ;
}

int			GM_ItemMaxNum( int id )
{
	return gm_itemsmax[ id ] ;
}

/* 武器、アイテム取られ */
int			GM_COM_SnatchEquip( void )
{
	memcpy( _GM_WeaponsSaved, gm_weapons, sizeof( short ) * MAX_WEAPONS ) ;
	memcpy( _GM_ItemsSaved, gm_items, sizeof( short ) * MAX_ITEMS ) ;
	memset( gm_weapons, -1, sizeof( short ) * MAX_WEAPONS ) ;
	memset( gm_items, 0, sizeof( short ) * MAX_ITEMS ) ;
	gm_weapons[ WP_None ] = 1 ;
	gm_items[ IT_None ] = 1 ;
	return 0 ;
}

/* 武器、アイテム取り返し */
int			GM_COM_RegainEquip( void )
{
	int		i ;

	for ( i = 0; i < MAX_WEAPONS; i ++ ) {
		if ( gm_weapons[ i ] < 0 ) gm_weapons[ i ] = 0 ;
		else if ( gm_weapons[ i ] >= 0 && _GM_WeaponsSaved[ i ] < 0 ) {
			_GM_WeaponsSaved[ i ] = 0 ;
		}
		gm_weapons[ i ] += _GM_WeaponsSaved[ i ] ;
		if ( gm_weapons[ i ] > gm_weaponsmax[ i ] ) {
			 gm_weapons[ i ] = gm_weaponsmax[ i ] ;
		}
	}
	for ( i = 0; i < MAX_ITEMS; i ++ ) {
		if ( gm_items[ i ] < 0 ) gm_items[ i ] = 0 ;
		else if ( gm_items[ i ] >= 0 && _GM_ItemsSaved[ i ] < 0 ) {
			_GM_ItemsSaved[ i ] = 0 ;
		}
		gm_items[ i ] += _GM_ItemsSaved[ i ] ;
		if ( gm_items[ i ] > gm_itemsmax[ i ] ) {
			 gm_items[ i ] = gm_itemsmax[ i ] ;
		}
	}
	return 0 ;
}
