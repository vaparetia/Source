//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ice_hzx.c
   アイスクーラー 当たり判定用ルーチン

   1999/12/15 T. Morita
   $Id: brk_ice_hzd.c,v 1.1.1.3 2002/11/19 11:45:32 Yoshizawa1 Exp $
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


int BRK_ICE_HzdIceboxCheck( Work *work, float rad )
{
    int     i, j, flag = 0 ;
    float   f, d ;
    FVECTOR v ;
    ICEBOX *p, *o ;

    for ( i=work->n_ice & ~BRK_ICE_INACTIVE, p=work->ice ; --i>=0 ; p++ )
	if ( p->flag >= 0 )
	    for ( j=i, o=p+1 ; --j>=0 ; o++ )
		if ( o->flag >= 0 )
		{
		    SubVector( &v, &p->mov.pos, &o->mov.pos ) ;
		    d = v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ;
		    //f = p->target.size.vx + o->target.size.vx ;
		    if ( d <= /* f*f */ rad )
		    {
			f = ( (o->mov.pos_v.vx - p->mov.pos_v.vx) * v.vx +
			      (o->mov.pos_v.vy - p->mov.pos_v.vy) * v.vy +
			      (o->mov.pos_v.vz - p->mov.pos_v.vz) * v.vz) / d ;
			/* 条件は重なりおよびスピード超過の防止 */

			_sceVu0ScaleVector( &v, &v, f<0.0f ? 0.2f : f<0.7f ? f : 0.7f ) ;
			_sceVu0SubVector( &o->mov.pos_v, &o->mov.pos_v, &v ) ;
			_sceVu0AddVector( &p->mov.pos_v, &p->mov.pos_v, &v ) ;

			//if ( o->flag == 0 )
			    o->flag = flag = 1 ;
			//if ( p->flag == 0 )
			    p->flag = flag = 1 ;
			work->n_ice &= ~BRK_ICE_INACTIVE ;
		    }
		}
    return flag ;
}
