//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"


static int Weapon_Ids[] = {
	WP_None,
	WP_m92, WP_Usp, WP_Socom,
	WP_m4, WP_Aks, WP_Psg1, WP_Psg1T,
	WP_Grenade, WP_ChaffGrenade, WP_StunGrenade, WP_Magazine,
	WP_Rgb6, WP_Nikita, WP_Stinger,
	WP_Blade,
	WP_Mic, WP_DemoMic,
	WP_Book,
	WP_Claymore, WP_C4Bomb,
	WP_ColdSpray
} ;
static char *Weapon_Names[] = {
	"none",
	"m92", "usp", "socom",
	"m4", "aks", "psg1", "psg1t",
	"grenade", "chaffgrenade", "stungrenade", "magazine",
	"rgb6", "nikita", "stinger",
	"blade",
	"mic", "demoMic",
	"book",
	"claymore", "c4Bomb",
	"coldSpray"
} ;


void PL_ActChangeWeaponMenu()
{
	static int prev ;
	int i ;

	if ( prev != GM_Weapon ) {
		prev = GM_Weapon ;

		for ( i=0 ; i< (sizeof(Weapon_Ids) / sizeof(int)) ; i++ )
		  if ( GM_Weapon == Weapon_Ids[i] ) {
			  printf( "Changed weapon to %s\n", Weapon_Names[i] ) ;
			  break ;
		  }
	}

#if 0
	static int index = 0 ;
	if ( GV_PadDataDirect[ 0 ].press & PAD_R2 ){
		index = (index + 1 ) % (sizeof(Weapon_Ids) / sizeof(int)) ;
		printf( "Changed weapon to %s\n", Weapon_Names[index] ) ;
	}

	GM_DebugPrint_Off = 0 ;
	
	DEBUG_Locate( 10, 10, 0 );
	DEBUG_Color( 200, 200, 200, 128 );
		GM_Weapon = Weapon_Ids[index] ;
	DEBUG_Printf( "%s\n", Weapon_Names[index] ) ;
	DEBUG_ResetColor();
#endif
}



static	int	Item_Ids[] = {
	IT_None,
	IT_Tabacco,
	IT_Card, IT_MODisc,
	IT_BombSenserA, IT_BombSenserB, IT_MineDetector, IT_VibSensor,
	IT_SocomSpprsr, IT_AKSpprsr, IT_UspSpprsr,
	IT_Shaver, IT_PHS,
	IT_NightVision, IT_Thermal,
	IT_Scope, IT_DummyScope, IT_Camera, 
    IT_TnkCamera, IT_DummyTnkCamera,
	IT_Jacket, IT_Uniform,
	IT_CBBox, IT_CBBoxWet, IT_CBBoxB, IT_CBBoxC, IT_CBBoxD, IT_CBBoxE,
	IT_Ration,
	IT_Styptic, IT_Diazepam, IT_Medicine,
	IT_DogTag,
	IT_MugenBandana, IT_MugenWig, IT_Stealth,
	IT_WigA, IT_WigB, IT_WigC, IT_WigD
} ; 

static	char *Item_Names[] = {
	"none",
	"tabacco",
	"card", "modisc",
	"bombsenser a", "bombsenser b", "minedetector", "vibsensor",
	"socomspprsr", "akspprsr", "uspspprsr",
	"shaver", "phs",
	"nightVision", "thermal",
	"scope", "dummyScope", "camera", 
    "tnkCamera", "dummytnkcamera",
	"jacket", "uniform",
	"cbbox", "cbboxwet", "cbbox b", "cbbox c", "cbbox d", "cbbox e",
	"ration",
	"styptic", "diazepam", "medicine",
	"dogTag",
	"mugenBandana", "mugenwig", "stealth",
	"wig a", "wig b", "wig c", "wig d"
} ; 

void PL_ActChangeItemMenu()
{
	static int index = 0 ;

return  ;
	if ( GV_PadDataDirect[ 0 ].press & PAD_L2 ){
		index = (index + 1 ) % (sizeof(Item_Ids) / sizeof(int)) ;
	}

	GM_DebugPrint_Off = 0 ;

	MENU_Locate( 20, 10, 0 );
	MENU_Color( 200, 200, 200, 128 );
	GM_Item = Item_Ids[index] ;
	MENU_Printf( "%s\n", Item_Names[index] ) ;
	MENU_ResetColor();

//	GM_DebugPrint_Off = 1 ;
}
