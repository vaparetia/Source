//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_mon_act.c
   制御室モニター壊れ

   2001/06/26 T. Morita
   $Id: brk_mon_act.c,v 1.1.1.3 2002/11/19 11:45:36 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_monitor.h"

#define SPEED   0.005f
#define MARGINE_UV 0.01f
#define MARGINE_LV 0.06f
#define MARGINE_W 0.01f



void BRK_MON_ActLaster( Work *work )
{
    int      i, idx ; 
    FVECTOR *pos ;
    MON     *m ;

    DG_SwitchBuffPrim2( work->line ) ;

    pos = (void*)SCRPAD_ADDR ;
    idx = 0 ;
    m = work->monitor ;
    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; m++ )
    {
	if ( (m->laster += SPEED) > (1.0f/3.0f-MARGINE_LV) )
	    m->laster = MARGINE_UV ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameR, i, 2,
			  -MARGINE_W, m->laster ) ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameR, i, 3,
			   MARGINE_W, m->laster ) ;
    }

    for ( i=BRK_MON_N_PRIMS/2 ; --i>=0 ; m++ )
    {
	if ( (m->laster += SPEED) > (1.0f/3.0f-MARGINE_LV) )
	    m->laster = MARGINE_UV ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameL, i, 2,
			  -MARGINE_W, m->laster ) ;
	BRK_MON_MakeShape( pos++, BRK_MOT_FrameL, i, 3,
			   MARGINE_W, m->laster ) ;
    }

    memcpy( work->line->pos[work->line->buffer_clock],
	    SCRPAD_ADDR,
	    sizeof(FVECTOR)*BRK_MON_N_PRIMS*2 ) ;
}

void BRK_MON_ActMonitor( Work *work )
{
    int  i ;
    MON *m ;

    /* DGのクロックを反転しておく */
    for ( i=BRK_MON_N_TYPE ; --i>=0 ; )
	DG_SwitchBuffPrim2( work->prim[i] ) ;

    /* モニターの環境音 */
    if ( work->n_monitor > 0 )
	if ( ++work->se_tic == 6*5/TIME_BASE )
	{
	    work->se_tic = 0 ;
#ifdef PSX2
	    GM_SeSetMode( SD_A_MONINOZ1, &work->center, GM_SEMODE_NORMAL) ;
	    if ( work->n_monitor > 9 )
		GM_SeSetMode( SD_A_MONINOZ2, &work->center, GM_SEMODE_NORMAL) ;
#else
		if ( (work->se_tgl ^= 1) ) {
			GM_SeSetMode( SD_A_MONINOZ1, &work->center, GM_SEMODE_NORMAL) ;
		} else {
			GM_SeSetMode( SD_A_MONINOZ2, &work->center, GM_SEMODE_NORMAL) ;
		}
#endif
	}

    /* モニターがラスタなどの制御 */
    m = work->monitor ;
    for ( i=BRK_MON_N_PRIMS ; --i>=0 ; m++ )
	if ( m->change == 0xf8 )
	    continue ;
	else if ( --m->change <= 0 )
	{
	    if ( m->change <= -1 )
		m->change = (m->table[m->type] & 0xf8) / 2 * 5 / TIME_BASE ;
	    else
		m->type_old = m->type, m->type = m->table[m->type] & 0x7 ;
	    /* 消す方 */
	    BRK_MON_SetRGB( work, m->type_old, i, -1, 0 ) ;
	    /* 点ける方 */
	    BRK_MON_SetRGB( work, m->type    , i, -1, 1 ) ;
	    /* 捜査線を消す? */
	    BRK_MON_LasterRGB( work, m->type, i ) ;
	}
}
