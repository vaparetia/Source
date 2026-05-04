//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_monitor.c
   制御室モニター壊れ

   1999/12/13 T. Morita
   $Id: brk_mon_clb.c,v 1.1.1.3 2002/11/19 11:45:36 Yoshizawa1 Exp $
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

#include "brk_monitor.h"



extern void *NewCrushWithForce( FVECTOR *pos, FVECTOR *force );
extern void *NewBreakPieceGlass( FVECTOR *pos, FVECTOR *dir ) ;

void BRK_MON_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    FVECTOR lpos, ldir ;
    FVECTOR *pos ;
    MON     *m ;
    int      i ;

    if ( def->damaged & TARGET_POWER )
    {
        GM_ClearTargetDamage( def ) ;

	pos = work->prim[0]->pos[0] ;
        if ( off->weapon_type & (WP_BULLET|WP_M92|WP_PSG1T) )
	    for ( i=0 ; i<BRK_MON_N_PRIMS ; i++, pos+=4 )
	    {
		_sceVu0CopyVector( &lpos, &def->hit ) ;
		_sceVu0Normalize( &ldir, &off->power->force ) ;
		if ( !BRK_UTL_CheckLineInFloatPolygon( &lpos, &ldir, pos+1 ) )
		    if ( !BRK_UTL_CheckLineInFloatPolygon( &lpos, &ldir, pos ) )
			continue ;

		m = work->monitor + BRK_MON_N_PRIMS-1-i ;
		/* もう壊れているから関係ない */
		if ( m->type == 4 )
		    return ;

		BRK_UTL_CallOffenceWhenThrough( off, def ) ;

		m->change = 0 ;
		m->type_old = m->type ;
		m->type     = m->table[4] & 0x7 ;
		if ( m->type == 4 )
		{
		    /* パーンと火花が飛ぶ */
		    if ( m->type_old != 7 )
			NewCrushWithForce( &lpos, &off->power->force ) ;
		    /* ガラスの破片 */
		    NewBreakPieceGlass( &lpos, &off->power->force ) ;
		    GM_SeSetMode( SD_A_MONITOR1, &off->hit, GM_SEMODE_BOMB ) ;
		    work->n_monitor-- ;
		}

		work->flag |= 1 << (BRK_MON_N_PRIMS-1-i) ;
		if ( work->proc )
		{
		    GCL_ARGS arg = { 1, &work->flag } ;
		    GCL_ExecProc( work->proc, &arg ) ;
		}

		/* 消す方 */
		BRK_MON_SetRGB( work, m->type_old, i, -1, 0 ) ;
		/* 点ける方 */
		BRK_MON_SetRGB( work, m->type    , i, -1, 1 ) ;
		return ;
	    }
    }
}
