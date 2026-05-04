/*
   brk_flr_clb.c
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_flr_clb.c,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
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
#include "brk_floor.h"


void BRK_FLR_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    extern void *NewSpark( FMATRIX *world ) ;
    int         i, j ;
    SVECTOR    *s, *n ;
    DG_OBJ     *obj ;
    DG_MDLPACK *m ;
    FMATRIX     inv ;
    FVECTOR     offs, frc ;
    int         damage ;

    if ( def->damaged & TARGET_POWER )
    {
        /* clear damage */
        GM_ClearTargetDamage( def ) ;

        /* 武器によってダメージは違う */
        if ( off->weapon_type & (WP_BULLET|WP_PUNCH|WP_KICK|WP_M92) )
            damage = 1 ;
        else if ( off->weapon_type & (WP_BLOW|WP_BLAST) )
            damage = 3 ;
        else 
            return ;

        /*ローカル座標系での着弾位置と着弾方向を求める*/
        _sceVu0CopyVector( &frc , &off->power->force ) ;
        _sceVu0CopyVector( &offs, &def->hit ) ;
        offs.vw = 1.0f ;
        frc.vw  = 0.0f ;
        _sceVu0InversMatrix( &inv, &work->objs->world ) ;
        _sceVu0ApplyMatrix( &frc , &inv, &frc  ) ;
        _sceVu0ApplyMatrix( &offs, &inv, &offs ) ;
    }
}

int BRK_FLR_InitTarget( Work *work, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t   = &work->target   ;
    POWER_TARGET *p   = &work->power    ;
    DG_DEF       *def = work->objs->def ;
    FVECTOR uv = { def->ux, def->uy, def->uz, 0 } ;
    FVECTOR lv = { def->lx, def->ly, def->lz, 0 } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size,  &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos,  &t_pos, 0.5f ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_ROTATE|TARGET_THROUGH, where, BOTH_SI
DE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector )
 ;
    GM_SetTargetCallBack( t, BRK_SWG_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}
