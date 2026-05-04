//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_vnd_clb.c 
   プット自動販売機 コールバック

   1999/12/26 T.Morita
   $Id: brk_vnd_clb.c,v 1.1.1.3 2002/11/19 11:45:53 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "brk_vending.h"


void BRK_VND_CanCallBack( TARGET *off, TARGET *def, void *ptr )
{
    CAN  *c = (CAN *)ptr ;
    Work *work = c->work ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	c->rot_v.vx = irnd()&1023 ;
	c->rot_v.vy = irnd()&1023 ;
	if ( off->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) )
	{
	    work->n_can &= ~BRK_VND_INACTIVE ;/* 缶のシステムを起こす */

	    c->flag = 2 ;/* 缶起動 */
	    _sceVu0ScaleVector( &c->pos_v, &off->power->force, 0.05f ) ;
	    c->pos_v.vy = 60.0f ; 
	    c->rot_v.vx = irnd()&1023 ;
	    c->rot_v.vy = irnd()&1023 ;
	}
    }
}

void BRK_VND_VenderCallBack( TARGET *off, TARGET *def, void *ptr )
{
    static FVECTOR  VendingOffset = { 0.0f, 500.0f, 0.0f, 1.0f } ;
    VENDER *p = (VENDER *)ptr ;
    Work   *work = p->work ;
    CAN    *c ;
    int     i ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) )
	    if ( p->can_num < work->n_can/work->n_vender )
	    {
		if ( p->proc )
		    GCL_ExecProc( p->proc, NULL ) ;
		work->n_can &= ~BRK_VND_INACTIVE ;/* 缶のシステムを起こす */
		for( i=work->n_can, c=work->can ; --i>=0 ; c++ )
		    if ( !c->objs )
		    {
			c->flag   = 4    ;/* 缶を起動 3frame無敵 */
			c->work   = work ;
			_sceVu0ScaleVector( &c->pos_v, (FVECTOR *)p->objs->world.m[Z], 30.0f ) ;
			c->pos_v.vy += 60.0f ;
			c->rot.vx = DEGtoANG(90) ;
			c->rot.vy = DEGtoANG(90) ;
			c->rot.vz = 0 ;
			c->rot_v.vx = irnd()&1023 ;
			c->rot_v.vy = irnd()&1023 ;
			_sceVu0ApplyMatrix( &c->pos, &p->objs->world, &VendingOffset ) ;
			break ;
		    }
	    }
    }
}
