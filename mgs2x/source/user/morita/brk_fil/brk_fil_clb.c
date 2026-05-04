//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fil_clb.c
   紙舞い壊れ コールバック関数

   2000/04/25 T. Morita
   $Id: brk_fil_clb.c,v 1.1.1.3 2002/11/19 11:45:28 Yoshizawa1 Exp $
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

#include "brk_file.h"


void BRK_FIL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PILED *p = (PILED *)ptr ;
    Work *work = p->work ;
    static FVECTOR pop = { 0.0f, 60.0f, 0.0f, 0.0f } ;
    FVECTOR *pos ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	/* これ以外の武器では,破壊不能 */
	if ( !(off->weapon_type & (WP_BULLET|WP_M92|WP_BLOW|WP_BLAST|WP_STUNGRENADE)) )
	    return ;

	/* 振動 */
	BRK_UTL_PK_Vibrate( &def->hit, off ) ;

	pos = (FVECTOR *)p->objs->world.m[W] ;
	/* 敵兵見つかり用 */
	if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	{
	    GM_SetNoise( NOISE_S, pos, work->where ) ;
	    _sceVu0CopyVector( &work->ene_find.pos, pos ) ;
	    work->ene_find.type = EF_TYPE_LV2 ;
	}
	GM_SeSetMode( SD_A_RICNAP01, pos, GM_SEMODE_NORMAL ) ;

	/* 埃や紙切れ が舞う */
	BRK_FIL_StartActDust ( p->work, 3, pos, &pop, 40.0f,-10.0f ) ;
	BRK_FIL_StartActPiece( p->work, 6, pos, &pop, 50.0f, 40.0f ) ;
	BRK_FIL_StartActFile( p, pos, &pop, 50.0f, 80.0f ) ;
	/* 起動 */
	work->n_piled &= ~BRK_FIL_INACTIVE ;
	p->flag = 0 ;

	/* 壊れプロックを呼び出す */
	if ( p->work->proc_id )
	    GCL_ExecProc( p->work->proc_id, NULL ) ;

	/* 大元のモデル および 当たり を消す */
	DG_DequeueObjs( p->objs ) ;
	DG_FreeObjs( p->objs ) ;
	p->objs = NULL ;
	GM_FreeTarget( def ) ;
    }
}
