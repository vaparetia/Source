/*
   brk_rol_clb.c
   転がりオブジェクト

   1999/12/13 T. Morita
   $Id: brk_rol_clb.c,v 1.1.1.3 2002/11/19 11:45:44 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_roll.h"



void BRK_ROL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    BOX  *b    = &work->box ;

    if ( !(def->damaged & TARGET_POWER) )
    {
	GM_ClearTargetDamage( def ) ;

	GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;

	b->mov.rot_vx = (irnd()&0x100)-128  ;/* 揺らす力 強め */
	b->mov.rot_vy = (irnd()&0x100)-128  ;

	_sceVu0ScaleVector( &b->mov.pos_v, &off->power->force, 0.01f ) ;
	b->mov.pos.vy += b->mov.pos_v.vy = 50.0f ;
	b->flag = 1 ;
    }
}
