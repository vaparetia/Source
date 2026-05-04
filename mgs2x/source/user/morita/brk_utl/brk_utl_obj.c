//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_utl_act.c
   壊れ用 汎用ルーチン

   2000/09/14 T. Morita
   $Id: brk_utl_obj.c,v 1.1.1.3 2002/11/19 11:45:51 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_utl.h"
#include "../brk_hzd/brk_hazard.h"





/*
  壊れ物

  パンチ＆振動

*/
FVECTOR *BRK_UTL_BlowBlastCheck( FVECTOR *pos, TARGET *off )
{
    float          dist ;
    static FVECTOR frc  ;

    if ( off->weapon_type & (WP_PUNCHALL|WP_BLOW) )
	dist = 800.0f ;
    else if ( off->weapon_type & WP_BLAST )
	dist = 3000.0f ;
    else
	return NULL ;

    _sceVu0SubVector( &frc, pos, &off->center ) ;
    if ( (int)(frc.vx/dist) || (int)(frc.vy/2000.0f) || (int)(frc.vz/dist) )
	return NULL ;

    _sceVu0Normalize( &frc, &frc ) ;
    _sceVu0ScaleVector( &frc, &frc, 1000.0f ) ;

    return &frc ;
}





/*
  壊れ物

  パンチ＆振動

*/

static int BRK_UTL_VibrateCount = 0 ;
//static int BRK_UTL_SoundCount   = 0 ;
void BRK_UTL_PK_Vibrate( FVECTOR *pos, TARGET *off )
{

#if 0
    if ( GM_StagePlayTime - BRK_UTL_SoundCount > 1 )
    {
	if ( off->weapon_type & (WP_PUNCHR| WP_PUNCHL) )
	{
	    if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_L )
	    {
		GM_SeSetMode( SD_P_GUNPNC01, pos, GM_SEMODE_BOMB ) ;
		BRK_UTL_SoundCount = GM_StagePlayTime ;
	    }
	    else if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_M )
	    {
		GM_SeSetMode( SD_P_GUNPNC01, pos, GM_SEMODE_BOMB ) ;
		BRK_UTL_SoundCount = GM_StagePlayTime ;
	    }
	    else
	    {
		GM_SeSetMode( SD_P_PUNCH02, pos, GM_SEMODE_BOMB ) ;
		BRK_UTL_SoundCount = GM_StagePlayTime ;
	    }
	}
	else if ( off->weapon_type & WP_KICK )
	{
	    if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_LL )
	    {
		GM_SeSetMode( SD_P_NKTPNC01, pos, GM_SEMODE_BOMB ) ;
		BRK_UTL_SoundCount = GM_StagePlayTime ;
	    }
	    else
	    {
		GM_SeSetMode( SD_P_KICK02, pos, GM_SEMODE_BOMB ) ;
		BRK_UTL_SoundCount = GM_StagePlayTime ;
	    }
	}
    }
#endif


    if ( GM_StagePlayTime - BRK_UTL_VibrateCount > 1 )
    {
	if ( off->weapon_type & (WP_PUNCHR| WP_PUNCHL) )
	{
	    if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_L )
	    {
		NewPadVibration2( 15227622 /*rai_punch_03*/, 0 ) ;
		BRK_UTL_VibrateCount = GM_StagePlayTime ;
	    }
	    else if ( GM_WeaponTypes[PL_GetPlayerWeapon()] & WP_TYPE_PUNCH_M )
	    {
		NewPadVibration2( 15227621 /*rai_punch_02*/, 0 ) ;
		BRK_UTL_VibrateCount = GM_StagePlayTime ;
	    }
	    else
	    {
		NewPadVibration2( 15227620 /*rai_punch_01*/, 0 ) ;
		BRK_UTL_VibrateCount = GM_StagePlayTime ;
	    }
	}
	else if ( off->weapon_type & WP_KICK )
	{
	    if ( off->weapon_type & WP_KICK1 )
	    {
		NewPadVibration2( 15931293 /*rai_kick_01 */, 0 ) ; 
		BRK_UTL_VibrateCount = GM_StagePlayTime ;
	    }
	    else
	    {
		NewPadVibration2( 15931294 /*rai_kick_02 */, 0 ) ; 
		BRK_UTL_VibrateCount = GM_StagePlayTime ;
	    }
	}
    }

}
