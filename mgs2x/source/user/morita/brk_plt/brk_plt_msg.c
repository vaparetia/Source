//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_plt_msg.c
   皿壊れ メッセージ

   2000/04/25 T. Morita
   $Id: brk_plt_msg.c,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
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
#include "../brk_utl/brk_utl.x"

void BRK_PLT_AlreadyMessy( Work *work )
{
    static FVECTOR size = { BRK_DUST_SPHERE, 3.0f, BRK_DUST_SPHERE, 0 } ;
    int     i, j ;
    PART  *p ;
    PILED *piled ;
    PART  *part ;

    work->n_piled &= ~BRK_PLT_INACTIVE ;
    work->n_piece &= ~BRK_PLT_INACTIVE ;
    work->n_part  &= ~BRK_PLT_INACTIVE ;
    work->n_plate &= ~BRK_PLT_INACTIVE ;
    for ( j=work->n_piled, piled=work->piled ; --j>=0 ; piled++ )
	if ( !(piled->flag & BRK_F_IS_PARENT) || irnd() & 0x1000 )
	{
	    piled->flag |= BRK_F_IS_BROKEN ;
	    DG_InvisibleObjs( piled->objs ) ;
	    GM_FreeTarget( &piled->target ) ;

	    /* 細かい破片 */
	    for ( i=15 ; --i>=0 ; )
	    {
		/*位置や角度をランダムに設定する */
		p = &work->piece[work->n_piece] ;
		p->mov.rot_vx = p->mov.rot_vy = p->mov.rot_x = 0 ;
		p->mov.rot_y = irnd() & 4095 ;
		p->mov.pos_v.vz = p->mov.pos_v.vx = 0.0f ;
		p->mov.pos_v.vy = -2000.0f ;
		p->mov.pos.vx = piled->objs->world.m[W][X] + frnd()*(i&1 ? 500.0f : 80.0f) ;
		p->mov.pos.vy = piled->objs->world.m[W][Y] + 50.0f ;
		p->mov.pos.vz = piled->objs->world.m[W][Z] + frnd()*(i&1 ? 500.0f : 80.0f) ;
		if ( BRK_CheckHazard( work->hzd, &p->mov.pos, &p->mov.pos_v,
				      &BRK_HZD_NoBounce, &size )&1 )/*当たりに降らせる*/
		{
		    RotateMatrixXY( &p->pos->world, &DG_UnitMatrix,
				    p->mov.rot_x, p->mov.rot_y ) ;
		    TransMatrix( &p->pos->world, &p->mov.pos ) ;
		    BRK_UTL_ComdlColor2( p->pos, Y ) ;/*床に落ちた奴だけ表示*/
		    if ( ++work->n_piece >= BRK_PLT_N_PIECE )
			work->n_piece = 0 ;
		}
	    }

	    /* 皿の破片 */
	    for ( i=work->p_def->n_models ; --i>=0 ; )
	    {
		/*位置や角度をランダムに設定する */
		part = &work->part[work->n_plate] ;
		part->flag = 0 ;
		part->mov.rot_x = part->mov.rot_y = part->mov.rot_vx = 0 ;
		part->mov.rot_vy = irnd() & 4095 ;
		part->mov.pos_v.vz = part->mov.pos_v.vx = 0.0f ;
		part->mov.pos_v.vy = -2000.0f ;
		part->mov.pos.vx = piled->objs->world.m[W][X] + frnd()*(i&1 ? 500.0f : 80.0f) ;
		part->mov.pos.vy = piled->objs->world.m[W][Y] + 50.0f ;
		part->mov.pos.vz = piled->objs->world.m[W][Z] + frnd()*(i&1 ? 500.0f : 80.0f) ;
		if ( BRK_CheckHazard( work->hzd, &part->mov.pos, &part->mov.pos_v,
				      &BRK_HZD_NoBounce, &size )&1 )/*当たりに降らせる*/
		{
		    RotateMatrixXY( &part->pos->world, &DG_UnitMatrix,
				    part->mov.rot_x, part->mov.rot_y ) ;
		    TransMatrix( &part->pos->world, &part->mov.pos ) ;
		    BRK_UTL_ComdlColor2( part->pos, Y ) ;/*床に落ちた奴だけ表示*/
		    part++ ;
		}
	    }
	    if ( ++work->n_plate >= 2*BRK_PLT_N_PLATE )
		work->n_plate = 0 ;
	}
	else
	    piled->flag &= ~BRK_F_HAS_CHILD ; /* 自分の上に皿はもない */
	
    work->n_piled |= BRK_PLT_INACTIVE ;
    work->n_plate |= BRK_PLT_INACTIVE ;
    work->n_part  |= BRK_PLT_INACTIVE ;
    work->n_piece |= BRK_PLT_INACTIVE ;
}

void BRK_PLT_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    BRK_PLT_AlreadyMessy( work ) ;
	    break ;
	}
}
