//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_mgz_clb.c
   雑誌壊れ コールバック関数

   2000/06/20 T. Morita
   $Id: put_bok_clb.c,v 1.1.1.3 2002/11/19 11:46:29 Yoshizawa1 Exp $
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

#include "put_book.h"


void PUT_BOK_BreakBook( BOOK *p )
{
    //PUT_BOK_StartActPiece( p->work, 10, &p->mov.pos, &pop, 50.0f, 40.0f ) ;
    NewTs_Min_Fog( &p->mov.pos ) ;

    /* SEを鳴らす */
    GM_SeSetMode( SD_I_EROBBR01, &p->mov.pos, GM_SEMODE_BOMB ) ;

    /* 挙動の初期化 */
    p->mov.pos_v.vy = 80.0f ;
    p->mov.rot_v.vx = (irnd()&0x00e0)-128 ;
    p->mov.rot_v.vy = (irnd()&0x00e0)-128 ;
    p->open_v = -DEG2RAD(20.0f) ;

	/* 当たり判定ありのアクト */
    p->flag = 12 ; /* collision immeadiately */
}

void PUT_BOK_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    BOOK *p = (BOOK *)ptr ;
    FVECTOR v ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( p->work->non_dmg )
	    return ;

	if ( off->weapon_type & (WP_PUNCH|WP_TUMBLE|WP_BLAST|WP_STUNGRENADE) )
	{
	    _sceVu0SubVector( &v, &def->center, &off->center ) ;

	    if ( (int)(v.vx / off->size.vx) ||
		 (int)(v.vy / off->size.vy) ||
		 (int)(v.vz / off->size.vz) ||
		 (int)(v.vx / 3000.0f) ||
		 (int)(v.vy / 3000.0f) ||
		 (int)(v.vz / 3000.0f) )
		return ;
	}
	else if ( !(off->weapon_type & (WP_BULLET|WP_M92)) )
	    return ;

	/* 親に動いたことを知らせる */
	GV_CallParentSignalFunc( p->work, 0, 0 ) ;

	/* 本が動く */
	PUT_BOK_BreakBook( p ) ;
    }
}
