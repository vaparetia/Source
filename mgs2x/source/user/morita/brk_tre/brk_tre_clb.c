//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tre_clb.c
   植物揺れ

   1999/12/17 T. Morita
   $Id: brk_tre_clb.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "../brk_utl/brk_utl.x"
#include "brk_tree.h"

void BRK_TRE_ShakeTree( Work *work, PART *part, FVECTOR *force )
{
    int i ;
    PART *p ;

    part->rot_vx = (short)( force->vz*0.05f) ;
    part->rot_vz = (short)(-force->vx*0.05f) ;
    part->act = BRK_TRE_ActShaking ;

    work->n_parts &= ~BRK_TRE_INACTIVE ;
    for ( p=work->parts, i=work->n_parts ; --i>=0 ; p++ )
	if ( part == &work->parts[p->mdl->parent] )
	{
	    p->rot_vx = -part->rot_vx*0 + (irnd()&127)-64 ;
	    p->rot_vz = -part->rot_vz*0 + (irnd()&127)-64 ;
	    p->act = BRK_TRE_ActShaking ;
	}
}
void BRK_TRE_HitProcess( Work *work, FVECTOR *pos )
{
    /* 壊れプロックを呼び出す */
    if ( work->proc_id )
	GCL_ExecProc( work->proc_id, NULL ) ;
    GM_SetNoise( NOISE_S, pos, work->where ) ;
}

void BRK_TRE_BlowLeaf( Work *work, FVECTOR *force, FVECTOR *pos )
{
#if 0
    if ( (work->n_pos = (work->n_pos+1) % BRK_N_LEAVES) == work->front )
	work->front = i ;
#else
    if ( ++work->n_pos >= BRK_N_LEAVES )
	work->n_pos = 0 ;
#endif
    NewLeafFly( work->leaf, &work->leaf->pos[work->n_pos],
		pos, force, &work->leaves[work->n_pos], work->hzx ) ;
}

void BRK_TRE_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PART *part = (PART *)ptr ;
    Work *work = part->work ;
    int         i, j ;
    FMATRIX     mtx ;
    FVECTOR     offs, frc ;

    if ( def->damaged & TARGET_POWER ) {
		/* clear damage */
		GM_ClearTargetDamage( def ) ;

		if ( off->weapon_type & (WP_PUNCHALL|WP_BLOW|WP_BLAST) ) {
			FVECTOR *force ;

			force = BRK_UTL_BlowBlastCheck( (FVECTOR *)part->obj->world.m[W],
										   off ) ;
			if ( !force )
			  return ;

			/* 振動 */
			//BRK_UTL_PK_Vibrate( (FVECTOR *)part->obj->world.m[W], off ) ;
			_sceVu0CopyVector( &offs, (FVECTOR *)part->obj->world.m[W] ) ;
			offs.vx += frnd()*200.0f ;
			offs.vy +=  rnd()*200.0f+1100.0f ;
			offs.vz += frnd()*200.0f ;
			BRK_TRE_ShakeTree( work, part, force ) ;
			BRK_TRE_BlowLeaf( work, force, &offs ) ;
			BRK_TRE_HitProcess( work, &offs ) ;

		} else if ( off->weapon_type & (WP_BULLET|WP_M92) )	{
			DG_MDLPACK *m ;

			/*ローカル座標系での着弾位置と着弾方向を求める*/
			_sceVu0CopyVector( &frc , &off->power->force ) ;
			_sceVu0CopyVector( &offs, &def->hit ) ;
			offs.vw = 1.0f ;
			frc.vw  = 0.0f ;
			_sceVu0InversMatrix( &mtx, &part->obj->world ) ;
			_sceVu0ApplyMatrix( &frc , &mtx, &frc  ) ;
			_sceVu0ApplyMatrix( &offs, &mtx, &offs ) ;
			
			/*ポリゴン当たり判定*/
			m = part->obj->model->packs ;
			for ( i=part->obj->model->n_packs ; --i>=0 ; m++ ) {
#ifdef PSX2
				SVECTOR    *s, *n ;

				s = (SVECTOR *)m->verts ;
				n = (SVECTOR *)m->norms ;
				for ( j=m->n_verts ; --j>=0 ; s++, n++ )
				  if ( !(n->pad & 0x8000) )
					if ( BRK_UTL_CheckLineInPolygon( &offs, &frc, s-2 ) ) {
						BRK_TRE_ShakeTree( work, part, &off->power->force ) ;
						_sceVu0CopyVector( &offs, (FVECTOR *)part->obj->world.m[W] ) ;
						offs.vx += frnd()*200.0f ;
						offs.vy +=  rnd()*200.0f+200.0f ;
						offs.vz += frnd()*200.0f ;
						BRK_TRE_BlowLeaf( work, &off->power->force, &offs ) ;
						BRK_TRE_HitProcess( work, &def->hit ) ;
						return ;
					}
#else
				int v0, v1, v2 ;
				DG_VERTEX_KMSS *kmss = part->obj->vbuff ;

				v0 = v1= v2= *(short*)m->index ;
				for ( j=0 ; j<m->n_indices ; j++ ) {
					if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
						if ( BRK_UTL_CheckLineInPolygon( &offs, &frc,
														 kmss+v0,
														 kmss+v1,
														 kmss+v2 ) ) {
							BRK_TRE_ShakeTree( work, part, &off->power->force ) ;
							_sceVu0CopyVector( &offs, (FVECTOR *)part->obj->world.m[W] ) ;
							offs.vx += frnd()*200.0f ;
							offs.vy +=  rnd()*200.0f+200.0f ;
							offs.vz += frnd()*200.0f ;
							BRK_TRE_BlowLeaf( work, &off->power->force, &offs ) ;
							BRK_TRE_HitProcess( work, &def->hit ) ;
							return ;
						}
					}
					v2 = v1 ;
					v1 = v0 ;
					v0 =*((short*)m->index + j) ;
				}
#endif
			}
		}
    }
}


