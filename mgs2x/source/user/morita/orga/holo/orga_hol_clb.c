//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_hol_clb.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_hol_clb.c,v 1.1.1.3 2002/11/19 11:46:24 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "orga_holo.h"
#include "../../brk_utl/brk_utl.x"


#if 0
/* シークを止めるために off側のコールバックを呼ぶ */
static inline void CallOffenceWhenThrough( TARGET *off, TARGET *def, FVECTOR *pos )
{
    off->hit = *pos ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
	( *off->callback )( off, def, off->work ) ;
}

/*

  シークがホロに当たっているか どうかをポリゴン判定で

*/
static void CheckHotPoint( Work *work, TARGET *off, TARGET *def )
{
    int         i, j ;
    SVECTOR    *s, *n ;
    DG_OBJ     *obj ;
    DG_MDLPACK *m ;
    FMATRIX     mtx ;
    FVECTOR     offs, frc ;

    /*ローカル座標系での着弾位置と着弾方向を求める*/
    _sceVu0SubVector( &frc , &off->center, &off->offset ) ;
    _sceVu0CopyVector( &offs, &def->hit ) ;
    offs.vw = 1.0f ;
    frc.vw  = 0.0f ;
    _sceVu0InversMatrix( &mtx, &work->objs->world ) ;
    _sceVu0ApplyMatrix( &frc , &mtx, &frc  ) ;
    _sceVu0ApplyMatrix( &offs, &mtx, &offs ) ;

    /*ポリゴン当たり判定*/
    obj = work->objs->objs ;
    for ( m=obj->model->packs, i=obj->model->n_packs ; --i>=0 ; m++ )
    {
	s = (SVECTOR *)m->verts ;
	n = (SVECTOR *)m->norms ;
	for ( j=m->n_verts ; --j>=0 ; s++, n++ )
	    if ( !(n->pad & 0x8000) )
		if ( BRK_UTL_CheckLineInPolygon( &offs, &frc, s-2 ) )
		{
		    _sceVu0ApplyMatrix( &offs, &work->objs->world, &offs ) ;
		    CallOffenceWhenThrough( off, def, &offs ) ;
		    break ;
		}
    }
}

#endif

/* 直線と点の距離を求める */
static inline float CheckDestanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;
    return -_sceVu0InnerProduct( &l, &v ) ;
}

/*

  ロープに当たっているかどうか

*/
static void CheckRopeHit( Work *work, TARGET *off, TARGET *def )
{
    int  idx = 0 ;

    if ( off->weapon_type & (WP_BULLET|WP_M92) )
    {
	if ( CheckDestanceLineAndPoint( &def->hit, &off->power->force,
					&work->himo_r ) < ORG_HOLO_RAD )
	{
	    if ( GM_GameLevel < GM_LEVEL_EXTREME || off->side & PLAYER_SIDE )
		idx = ORG_HOL_PlyFreeRgtIdx[work->flag] ;
	}
	else if ( CheckDestanceLineAndPoint( &def->hit, &off->power->force,
					     &work->himo_l ) < ORG_HOLO_RAD )
	{
	    if ( GM_GameLevel < GM_LEVEL_EXTREME || off->side & PLAYER_SIDE )
		idx = ORG_HOL_PlyFreeLftIdx[work->flag] ;
	}
    }

    else if ( off->weapon_type & (WP_BLOW|WP_BLAST) )
    {
	if ( GM_GameLevel < GM_LEVEL_EXTREME )
	{
	    if ( fpu_Abs(def->hit.vz - work->himo_l.vz) <
		 fpu_Abs(def->hit.vz - work->himo_r.vz) )
		idx = ORG_HOL_PlyFreeRgtIdx[work->flag] ;
	    else
		idx = ORG_HOL_PlyFreeLftIdx[work->flag] ;
	}
    }

    if ( idx )
    {
	work->key_idx = idx ;
	work->ratio = 256 ;
	work->key   = 0   ;
	work->tics  = 0   ;
    }
}


/*

  ホロのコールバック

 */
void ORG_HOL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work    *work = (Work*)ptr ;

    /*
    if ( off->class & TARGET_SEEK )
	CheckHotPoint( work, off, def ) ;
	*/

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;
	CheckRopeHit( work, off, def ) ;
    }
}
