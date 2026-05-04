//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_clb.c
  エマコールバック関数

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_clb.c,v 1.1.1.3 2002/11/19 11:45:59 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"

static void BloodShed( Work *work, TARGET *def, TARGET *off )
{
    FMATRIX *mtx ;

    if ( (mtx = EMA_GetConnectObjMatrix( work, def )) )
	NewBlood( mtx, &def->hit, &off->power->force, 0, 0 ) ;
}

void EMA_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work= (Work *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	if ( EMA_Flag(EMA_F_EVENT_SNIPE) )
	{
	    printf( "Emma: I'm Hit def%lx off%lx %d\n",
		    def->weapon_type, off->weapon_type,
		    EMA_Flag(EMA_F_HURT_BY_VMP) ) ;
	    if ( EMA_Flag(EMA_F_HURT_BY_VMP) )
	    {
		/* 首締めイベントの時はスティンガー一発死に */
		if ( off->weapon_type & (WP_M92|WP_BULLET) )
		{
		    def->weapon_type = WP_SOCOM ;

		    EMA_UtilStopStream( work ) ;
		    work->voice_id  = SD_V_EMADMG01 ;
		    work->voice_tim = 1 ;

		    printf( "Emma: I'm Hit %lx\n", def->weapon_type ) ;
		}
		else if ( off->weapon_type & WP_STINGER )
		{
		    printf( "Emma: I'm Hit by STINGER\n" ) ;
		    work->voice_id  = SD_V_EMADMG01 ;
		    work->voice_tim = 1 ;
		    work->npc.action.status |= NPC_ACT_STATUS_DEATH ;
		}
		

	    }
	    if ( EMA_Flag( EMA_F_IS_ON_BRIDGE ) )
	    {
		if ( off->weapon_type & WP_CLAYMORE )
		    def->weapon_type = WP_SOCOM ;
		if ( off->weapon_type & WP_THROWG )
		{
		    printf( "CLAYMORE %d %d", off->name, WP_Claymore ) ;
		    if ( off->name == WP_Claymore )
			def->weapon_type = WP_CLAYMORE ;
		}
	    }

	    EMA_SetFlag( EMA_F_ATTACKED_BY ) ;

	    if ( !(off->weapon_type & WP_NOPLAYER) )
		EMA_SetFlag( EMA_F_HURT_BY_PLY ) ;/* プレーヤに傷つけれた */
	    else
		EMA_ResetFlag( EMA_F_HURT_BY_PLY ) ;
	    if ( off->weapon_type & WP_BULLET )
		BloodShed( work, def, off ) ;
	}
	else if ( EMA_Flag(EMA_F_NON_DAMAGE) || work->non_damage )
	{
	    if ( off->weapon_type & (WP_BLOW|WP_PUNCHALL|WP_BULLET|WP_BLAST|WP_WEAPONCORE|WP_STUNFAR|WP_M92|WP_PSG1T) )
		GM_ClearTargetDamage( def ) ;
	}
	else
	{
	    EMA_SetFlag( EMA_F_ATTACKED_BY ) ;

	    if ( work->npc.action.current_mar != EMA_BASE )
	    {
		EMA_UtilStopStream( work ) ;

		switch( work->mar_mtn )
		{
		case MAR(EMA_CAPT,CAP_THROW):
		case MAR(EMA_CAPT,CAP_HANG):
		case MAR(EMA_CAPT,CAP_IDLE):
		case MAR(EMA_CAPT,CAP_WALK):
		case MAR(EMA_CAPT,CAP_RELEASE):
		case MAR(EMA_CAPT,CAP_RELEASE_DOWN):
		case MAR(EMA_CAPT,CAP_RESIST):
		case MAR(EMA_CAPT,CAP_TIE):
		case MAR(EMA_CAPT,CAP_ESCAPE):
		    if ( off->weapon_type & (WP_BLOW|WP_PUNCHALL|WP_BULLET) )
		    {
			work->voice_id  = SD_V_EMADMG01 ;
			work->voice_tim = 1 ;
		    }
		}
		if ( off->weapon_type & (WP_BLOW|WP_BULLET) )
		    EMA_SetInvincible( work ) ;
		if ( off->weapon_type & WP_BLAST )
		    EMA_CommandGiveDamage( work->npc.action.life ) ;
	    }

	    if ( !(off->weapon_type & WP_NOPLAYER) )
		EMA_SetFlag( EMA_F_HURT_BY_PLY ) ;/* プレーヤに傷つけれた */
	    else
		EMA_ResetFlag( EMA_F_HURT_BY_PLY ) ;
	    if ( off->weapon_type & WP_BULLET )
		BloodShed( work, def, off ) ;
	}
	work->offtrg = off ;
    }
}
