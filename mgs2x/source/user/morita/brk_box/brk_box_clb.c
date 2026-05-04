/*
   brk_box_clb.c
   じゃがいも壊れ

   1999/12/13 T. Morita
   $Id: brk_box_clb.c,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
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

#include "brk_box.h"



void BRK_BOX_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    BOX *p = (BOX *)ptr ;
    int i ;

    if ( def->damaged )
    {
        p->rot_vx = -off->power->force.vz*0.3f ;
        p->rot_vz = -off->power->force.vx*0.3f ;

	p->act = BRK_BOX_Act ;

	GM_ClearTargetDamage( def ) ;
    }
}
