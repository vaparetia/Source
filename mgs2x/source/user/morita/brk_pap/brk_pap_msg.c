//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pap_msg.c
   紙 壊れ メッセージ

   2000/01/15 T. Morita
   $Id: brk_pap_msg.c,v 1.1.1.3 2002/11/19 11:45:39 Yoshizawa1 Exp $
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

#include "brk_paper.h"
#include "../brk_utl/brk_utl.x"


void BRK_PAP_AlreadyMessy( Work *work )
{
    static FVECTOR piece_size = { BRK_DUST_SPHERE, 3.0f, BRK_DUST_SPHERE, 0 } ;
    static FVECTOR paper_size = { 150.0f ,30.0f, 150.0f, 0 } ;
    int     i, j ;
    PILED *piled ;
    PIECE *p ;
    PAPER *paper ;

    work->n_piled &= ~BRK_PAP_INACTIVE ;
    work->n_paper &= ~BRK_PAP_INACTIVE ;
    work->n_piece &= ~BRK_PAP_INACTIVE ;
    for ( j=work->n_piled, piled=work->piled ; --j>=0 ; piled++ )
    {
	/* 紙 */
	paper = work->paper ;
	for ( i=BRK_PAP_N_PAPER ; --i>=0 ; paper++ )
	{
	    DG_InvisibleObjs( paper->objs ) ;
	    paper->mov.pos_v.vz = paper->mov.pos_v.vx = 0.0f ;
	    paper->mov.pos_v.vy = -3000.0f ;
	    paper->mov.pos.vx = piled->objs->world.m[W][X] + frnd()*2000.0f ;
	    paper->mov.pos.vy = piled->objs->world.m[W][Y] + 500.0f  ;
	    paper->mov.pos.vz = piled->objs->world.m[W][Z] + frnd()*2000.0f ;
	    if ( BRK_CheckHazard( work->hzd, &paper->mov.pos, &paper->mov.pos_v,
				  &BRK_HZD_NoBounce, &paper_size )&1 )/*当たりに降らせる*/
		if ( !BRK_UTL_PutCenterHazard( work->hzd, &paper->mov.pos, NULL, 1000.0f, 0.0f ) )
		{
		    RotateMatrixXY( &paper->objs->world, &DG_UnitMatrix, 3072, irnd() & 4095 ) ;
		    TransMatrix( &paper->objs->world, &paper->mov.pos ) ;
		    _sceVu0CopyMatrix( &paper->objs->objs[0].world, &paper->objs->world ) ;
		    DG_VisibleObjs( paper->objs ) ;
		    BRK_UTL_GetLightMatrix( &paper->mov.pos, paper->lights, 0.8f, work->where ) ;
		}
	}

	/* 細かい破片 */
	for ( i=10 ; --i>=0 ; )
	{
	    /*位置や角度をランダムに設定する */
	    p = &work->piece[work->n_piece] ;
	    p->mov.rot_vx = p->mov.rot_vy = 0 ;
	    p->mov.rot_x = 1024 ;
	    p->mov.rot_y = irnd() & 4095 ;
	    p->mov.pos_v.vz = p->mov.pos_v.vx = 0.0f ;
	    p->mov.pos_v.vy = -3000.0f ;
	    p->mov.pos.vx = piled->objs->world.m[W][X] + frnd()*1500.0f ;
	    p->mov.pos.vy = piled->objs->world.m[W][Y] + 500.0f          ;
	    p->mov.pos.vz = piled->objs->world.m[W][Z] + frnd()*1500.0f ;
	    if ( BRK_CheckHazard( work->hzd, &p->mov.pos, &p->mov.pos_v,
				  &BRK_HZD_NoBounce, &piece_size )&1 ) /* 当たりに降らせる */
	    {
		/*床に落ちた奴だけ表示*/
		RotateMatrixXY( &p->pos->world, &DG_UnitMatrix,	p->mov.rot_x, p->mov.rot_y ) ;
		TransMatrix( &p->pos->world, &p->mov.pos ) ;
		BRK_UTL_ComdlColor( p->pos, 0.8f, work->where ) ;
		if ( ++work->n_piece >= BRK_PAP_N_PIECE )
		    work->n_piece = 0 ;
	    }
	}

	/* 大元のモデル および 当たり を消す */
	DG_DequeueObjs( piled->objs ) ;
	DG_FreeObjs( piled->objs ) ;
	piled->objs = NULL ;
	GM_FreeTarget( &piled->target ) ;
    }
    work->n_piled |= BRK_PAP_INACTIVE ;
    work->n_paper |= BRK_PAP_INACTIVE ;
    work->n_piece |= BRK_PAP_INACTIVE ;
}

void BRK_PAP_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    BRK_PAP_AlreadyMessy( work ) ;
	    break ;
	}
}
