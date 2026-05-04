//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_msg.c
   瓶壊れ メッセージ

   2000/10/15 T. Morita
   $Id: brk_btl_msg.c,v 1.1.1.3 2002/11/19 11:45:24 Yoshizawa1 Exp $
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

#include "brk_bottle.h"

void BRK_BTL_AlreadyMessy( Work *work, int mask )
{
    int     i, j ;
    PART   *p ;
    BOTTLE *b = work->bottles ;

    work->n_bottles &= ~BRK_BOTTLE_INACTIVE ;
    for( j=work->n_bottles ; --j>=0 ; b++ )
    {
	if ( irnd() & 0x1000 )
	{
	    if ( !mask )
	    {
		BRK_BTL_WineSplash( work, b ) ;
		BRK_BTL_FreeBottle( b ) ;/*この瓶は完全に壊れた*/
	    }
	    continue ;
	}
	/* 壊れモデルを初期化 */
	if ( BRK_BTL_InitModel( b, work->where, b->brk_af,
				DG_FLAG_PAINT|DG_FLAG_FINISHCALC, work->lit ) < 0 )
	    printf( "Can't Create Obj!! :: NewPutBottleObject\n" ) ;
	else
	{
	    /*ワインを垂らす*/
	    BRK_BTL_WineSplash( work, b ) ;
	    b->act = BRK_BTL_ActFinished ; /*瓶のアクトは,壊れモードに*/

	    /* とりあえず底以外の壊れは無し */
	    for ( p=b->parts, i=0 ; i<b->objs->n_models ; i++, p++ )
		if ( i )
		{
		    p->target->class |= TARGET_SKIP ;   /* このターゲットに弾はもう当たらない */
		    p->obj->flag |= DG_FLAG_INVISIBLE ; /* 当たったパーツを消す */
		    p->act = NULL ;
		}
		else
		{
		    p->act = BRK_BTL_ActPartOnShelf ;
		    _sceVu0CopyVector( &p->pos_v, &DG_ZeroVector ) ;
		    p->rot_v.vy = p->rot_v.vz = 0 ;
		    p->rot.vy = irnd()&4095 ;
		    BRK_BTL_MovePart( p, work->where ) ;
		}
	}
    }

    work->n_bottles |= BRK_BOTTLE_INACTIVE ;
}

void BRK_BTL_ReceiveMessage( Work *work )
{
    int     i, break_msk = -1 ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    if ( break_msk < 0 )
		break_msk = 0x10 ;
	    break ;
	case 2:
	    break_msk = 0 ;
	    break ;
	}
    if ( break_msk >= 0 )
	    BRK_BTL_AlreadyMessy( work, break_msk ) ;
}

