//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_plt_clb.c
   皿壊れ コールバック

   2000/04/25 T. Morita
   $Id: brk_plt_clb.c,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
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

#include "brk_plate.h"


static void MakeBrokenPlate( PILED *p, FVECTOR *pos, FVECTOR *frc )
{
    PLATE *plt ;
    PILED  *pp ;
    int     i  ;

    /* 壊れた皿は消す */
    p->flag |=  BRK_F_IS_BROKEN ;
    DG_InvisibleObjs( p->objs ) ;

    if ( p->flag & BRK_F_HAS_CHILD )/* 自分の上にまだ皿がある？ */
    {
	pos->vy += 40.0f ;
	plt = BRK_PLT_StartActPlate( p->work, pos, frc, 40.0f, 3.0f ) ;
	BRK_PLT_StartActPart( p->work, plt, frc, 10.0f, 3.0f ) ;
	DG_InvisibleObjs( plt->objs ) ;

	/* 上にある皿を捜し */
	for ( pp=p+1 ;  ; pp++ )
	{
	    if ( !(pp->flag & BRK_F_IS_BROKEN) )
		pp->flag |= BRK_F_IS_ACTIVE ;/* ACTIVEフラグをつける */
	    if ( !(pp->flag & BRK_F_HAS_CHILD) )
		break ;
	}
    }
    else                            /* じゃあ 自分の下にまだ皿があるの？ */
    {
	for ( i=BRK_PLT_N_PLATE ; --i>=0 ; )
	{
	    pos->vy += 40.0f ;
	    BRK_PLT_StartActPlate( p->work, pos, frc, 40.0f, 3.0f ) ;
	}
	if ( !(p->flag & BRK_F_IS_PARENT) )      /* 自分が一番下でなかったら */
	{
	    for ( pp=p-1 ; pp->flag & BRK_F_IS_BROKEN ; pp-- )
		if ( pp->flag & BRK_F_IS_PARENT )/* 下にある皿を捜す */
		    break ;
	    pp->flag &= ~BRK_F_HAS_CHILD ;       /* CHILDフラグを消す */
	}
    }
}


void BRK_PLT_PiledTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PILED  *p = (PILED *)ptr ;
    FVECTOR pos ;
    FVECTOR frc ;
    FVECTOR *force ;
    Work *work = p->work ;

    if ( def->damaged & TARGET_POWER )
    {
        /* clear damage */
        GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BLAST|WP_STUNGRENADE))
	{
	    if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
		return ;
	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->center, off ) ;
	}
	else if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    force =  &off->power->force ;
	else
	    return ;

	_sceVu0CopyVector( &pos, &p->objs->world.m[W] ) ;
	_sceVu0ScaleVector( &frc, force, -0.05f ) ;
	work->n_piled &= ~BRK_PLT_INACTIVE ;

	/* 破片を出す */
	BRK_PLT_StartActPiece( work, BRK_PLT_N_BRKPIECE, &pos, &frc, 30.0f, 20.0f ) ;
	BRK_PLT_StartActDust( work, 5, &pos, &DG_ZeroVector, 50.0f, 5.0f ) ;

	if ( BRK_PLT_SeState <= 0 )
	    GM_SeSetMode( SD_A_SARABRE1, &pos, GM_SEMODE_BOMB ),
		BRK_PLT_SeState = BRK_PLT_SE_DFLT ;
	/* 敵兵見つかり用 */
	if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	{
	    /* この音を聞いて敵兵が寄る */
	    GM_SetNoise( NOISE_MM, &pos, work->where ) ;
	    _sceVu0CopyVector( &work->ene_find.pos, &pos ) ;
	    work->ene_find.type = EF_TYPE_LV2 ;
	}

	/* 破片を作ったり上の物が動いたりする */
	MakeBrokenPlate( p, &pos, &frc ) ;

	/* 壊れプロックを呼び出す */
	if ( work->proc_id )
	    GCL_ExecProc( work->proc_id, NULL ) ;

	GM_FreeTarget( def ) ;
    }
}
