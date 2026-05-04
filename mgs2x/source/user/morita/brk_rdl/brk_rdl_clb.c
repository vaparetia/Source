//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_rdl_clb.c
   レードル揺れ

   2000/09/07 T. Morita
   $Id: brk_rdl_clb.c,v 1.1.1.3 2002/11/19 11:45:43 Yoshizawa1 Exp $
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
#include "brk_radle.h"


void BRK_RDL_SetSound( RADLE *radle, FVECTOR *pos )
{
    int se_id ;

    if ( radle->se_id )
	se_id = radle->se_id ;
    else
    {
	/* モデルの大きさで音色を選択し,大きさによって音の高さ変える */
	if ( radle->target.size.vx < 100.0f )
	    se_id = (radle->scale.vy <= 0.7f ? SD_A_OTAMA_CS:
		     radle->scale.vy <= 1.0f ? SD_A_OTAMA_E : SD_A_OTAMA_DS) ;
	else
	    se_id = (radle->scale.vy <= 0.7f ? SD_A_FLYPAN_C :
		     radle->scale.vy <= 0.8f ? SD_A_FLYPAN_B :
		     radle->scale.vy <= 1.0f ? SD_A_FLYPAN_A :
		     radle->scale.vy <= 1.2f ? SD_A_FLYPAN_G : SD_A_FLYPAN_F) ;
    }
    GM_SeSetMode( se_id, pos, GM_SEMODE_BOMB ) ;
}

static void BRK_RDL_MoveRadle( Work *work, RADLE *radle,
							   FVECTOR *frc,FVECTOR *pos )
{
    /* レードルを動かす */
    work->n_radle &= ~BRK_RDL_INACTIVE ;
    radle->mode = BRK_MOD_MOVE ;
    radle->rot_vx = (short)(-frc->vz * BRK_RDL_MOVE_RATE) ;
    radle->rot_vz = (short)( frc->vx * BRK_RDL_MOVE_RATE) ;

    /* この音を聞いて敵兵が寄る */
    GM_SetNoise( NOISE_S, pos, work->where ) ;

    /* SEを鳴らす */
    BRK_RDL_SetSound( radle, pos ) ;
}

static inline void BRK_RDL_HitRadle( Work *work, RADLE *radle,
									TARGET  *off , TARGET  *def,
									FVECTOR *offs, FVECTOR *frc ) 
{
    extern void *NewSpark( FMATRIX *world ) ;
	FMATRIX     mtx ;

	/* 跳弾を出す */
	if ( off->weapon_type & (WP_BULLET|WP_M92) ) {
		_sceVu0ApplyMatrix( offs, &radle->objs->world, offs ) ;

		UTL_VectoMat( &mtx, &off->power->force, offs, Y ) ;
		GM_CurrentMap = work->where ;
		NewSpark( &mtx ) ;
	}
	
	/* 揺らす */
	BRK_RDL_MoveRadle( work, radle, frc, offs ) ;
	
	/* 弾を止める */
	BRK_UTL_CallOffenceWhenThrough( off, def ) ;
}

void BRK_RDL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    RADLE *radle = (RADLE *)ptr ;
    Work *work = radle->work ;


    if ( off->class & TARGET_SEEK )
	  BRK_UTL_CallOffenceWhenThrough( off, def ) ;

    if ( def->damaged & TARGET_POWER ) {
		/* clear damage */
		GM_ClearTargetDamage( def ) ;

		if ( off->weapon_type & (WP_PUNCHALL|WP_BLOW|WP_BLAST|WP_STUNGRENADE)){
			FVECTOR    *force ;

			if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
			  return ;

			/* 振動 */
			BRK_UTL_PK_Vibrate( &def->center, off ) ;

			/* 揺らす */
			BRK_RDL_MoveRadle( work, radle, force, &def->center ) ;

		} else if ( off->weapon_type & (WP_BULLET|WP_M92) ) {
			
			FVECTOR     offs, frc ;
			FMATRIX     mtx ;
			DG_OBJ     *obj = radle->objs->objs ;
			DG_MDLPACK *m ;
			int         i, j ;
#ifdef KP_XBOX
			DG_VERTEX_KMSS *kmss = obj->model->vbuff ;
#endif
			
			_sceVu0CopyVector( &frc , &off->power->force ) ;
			_sceVu0CopyVector( &offs, &def->hit ) ;
			/*ローカル座標系での着弾位置と着弾方向を求める*/
			offs.vw = 1.0f ;
			frc.vw  = 0.0f ;
			MakeInvWorld( &mtx, radle ) ;
			_sceVu0ApplyMatrix( &frc , &mtx, &frc  ) ;
			_sceVu0ApplyMatrix( &offs, &mtx, &offs ) ;			

			/*弾のポリゴン当たり判定*/
			for ( m=obj->model->packs, i=obj->model->n_packs ; --i>=0 ; m++ ) {
#ifdef PSX2
				SVECTOR    *s, *n ;

				s = (SVECTOR *)m->verts ;
				n = (SVECTOR *)m->norms ;
				for ( j=m->n_verts ; --j>=0 ; s++, n++ ) {
					if ( !(n->pad & 0x8000) ) {
						if ( BRK_UTL_CheckLineInPolygon( &offs, &frc, s-2 )){
							BRK_RDL_HitRadle( work, radle,
											  off, def,
											 &offs, &frc ) ;
							return ;
						}
					}
				}
#else
				int v0, v1, v2 ;

				v0 = v1= v2= *(short*)m->index ;
				for ( j=0 ; j<m->n_indices ; j++ ) {
					v2 = v1 ;
					v1 = v0 ;
					v0 =*((short*)m->index + j) ;
					if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
						if ( BRK_UTL_CheckLineInPolygon( &offs, &frc,
														 kmss+v0,
														 kmss+v1,
														 kmss+v2 ) ) {
							BRK_RDL_HitRadle( work, radle,
											  off, def,
											 &offs, &frc ) ;
							return ;
						}
					}
				}
#endif
			}
		}
	}
}
