//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ice_clb.c
   アイスクーラー

   1999/12/13 T. Morita
   $Id: brk_ice_clb.c,v 1.1.1.3 2002/11/19 11:45:32 Yoshizawa1 Exp $
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

#include "brk_icebox.h"


void BRK_ICE_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    ICEBOX *ice = (ICEBOX *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
#if 0
	if ( BRK_ICE_InitIceDustPrims( ice ) )
	    GM_ClearTargetDamage( def ) ;
	else
	{
	    DG_DequeueObjs( ice->objs ) ;
	    DG_FreeObjs( ice->objs );
	    ice->objs = NULL ;
	    ice->flag = -2 ;         /*葬り去る*/
	    ice->work->n_ice &= ~BRK_ICE_INACTIVE ; /* 氷のアクトを有効にする */

	    GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;

	    GM_FreeTarget( def ) ;  /* 氷のターゲットを取り除く */
	}
#else

	if ( BRK_ICE_InitIceDustComdl( ice ) )
	    GM_ClearTargetDamage( def ) ;
	else
	{
	    DG_DequeueObjs( ice->objs ) ;
	    DG_FreeObjs( ice->objs );
	    ice->objs = NULL ;
	    ice->flag = -2 ;         /*葬り去る*/
		ice->work->n_ice &= ~BRK_ICE_INACTIVE ; /* 氷のアクトを有効にする */

      // Special case error handling for invalid sound position
      {
         int didHaveNAN = 0;

         if( def->hit.x != def->hit.x )
         {
            def->hit.x = 0.0f;
            didHaveNAN = 1;
         }

         if( def->hit.y != def->hit.y )
         {
            def->hit.y = 0.0f;
            didHaveNAN = 1;
         }

         if( def->hit.z != def->hit.z )
         {
            def->hit.z = 0.0f;
            didHaveNAN = 1;
         }

         if( didHaveNAN )
         {
            //BPTODO - Some bad collision results is causing a wrong sound position which messes up the surround calculations.
            // This prevents a crash.
            printf("SOUND/COLLISION? ISSUE: Determine what is causing this invalid case for hit position in 'brk_ice_clb.c'.\n");
         }
      }

	    GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;

	    GM_FreeTarget( def ) ;  /* 氷のターゲットを取り除く */
	}
	
#endif
    }
}

void BRK_ICE_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    BOX  *b    = &work->box ;
    FVECTOR *force ;

    if ( !(def->damaged & TARGET_POWER) )
	return ;
    GM_ClearTargetDamage( def ) ;

    if ( b->non_dmg > 0 )
	return ;

    if ( off->weapon_type & (WP_BULLET|WP_M92) )
	force = &off->power->force ;
    else if ( off->weapon_type & (WP_BLOW|WP_BLAST|WP_PUNCHALL) )
    {
	if ( !(force = BRK_UTL_BlowBlastCheck( &def->hit, off )) )
	    return ;
	/* 振動 */
	BRK_UTL_PK_Vibrate( &def->hit, off ) ;
    }
    else
	return ;

    b->non_dmg = 8 ;
    b->snd_flag = 0;

    GM_SetNoise( NOISE_MM, &def->hit, work->where ) ;

    if ( work->ice )
    {
	GM_SeSetMode( SD_A_ICECAN01, &def->hit, GM_SEMODE_BOMB ) ;
	b->mov.rot_vx = (irnd()&0x100)-128  ;/* 揺らす力 強め */
	b->mov.rot_vy = (irnd()&0x100)-128  ;

	_sceVu0ScaleVector( &b->mov.pos_v, force, 0.01f ) ;

	b->mov.pos.vy += b->mov.pos_v.vy = 50.0f ;
	b->flag = 1 ;
    }
    else
    {
	GM_SeSetMode( SD_A_ICECAN03, &def->hit, GM_SEMODE_BOMB ) ;
	b->mov.rot_vx = (irnd()&0x040)- 32 ; /* 揺らす力 弱め */
	b->mov.rot_vy = (irnd()&0x100)-128 ;
	b->flag = 2 ;

	/*氷のメモリを確保できそうか？*/
	if ( BRK_ICE_InitIce( work ) < 0 )
	    BRK_ICE_FreeIce( work ) ;
	else if ( BRK_ICE_InitOpenBox( work ) < 0 )
	    BRK_ICE_FreeIce( work ) ;
	else
	{
	    b->mov.rot_vx = 1024 ;
	    b->mov.rot_vy = (irnd()&0x0100)-128 ;
	    _sceVu0ScaleVector( &b->mov.pos_v, force, 0.01f ) ;
	    b->mov.pos.vy += b->mov.pos_v.vy = 50.0f ;
	    b->ice_fount = 0 ;
	    b->flag = 3 ;

	    /* 壊れプロックを呼び出す */
	    if ( work->proc_id )
		GCL_ExecProc( work->proc_id, NULL ) ;
	}
    }
}
