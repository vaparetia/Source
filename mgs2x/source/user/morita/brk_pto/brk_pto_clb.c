//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_clb.c
   じゃがいも壊れ

   1999/12/13 T. Morita
   2000/10/16 1.17 T.Morita 
   $Id: brk_pto_clb.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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

#include "brk_potato.h"
#include "../brk_utl/brk_utl.x"


static void SetNeedle( DG_OBJS *objs, int n_obj, FVECTOR *pos, FVECTOR *dir )
{
    extern void VertexSearch( FVECTOR *vans, FVECTOR *nans,
			      DG_OBJS *objs, int objnum, FVECTOR *target) ;
    FVECTOR vans, nans ;

    VertexSearch( &vans, &nans, objs, n_obj, pos ) ;
}

extern void *NewCrushDust( FVECTOR *pos, FVECTOR *force, int mode ) ;

void BRK_PTO_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    POTATO *pto = (POTATO *)ptr ;
    PART   *p ;
    int     i ;
    DG_MDL *m ;
    DG_OBJ *o ;
    FVECTOR v ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	/* M92の時は,壊れない */
	if ( off->weapon_type & WP_M92 )
	{
	    SetNeedle( pto->objs, 0, &def->hit, &off->power->force ) ;
	    _sceVu0ScaleVector( &pto->pos_v, &off->power->force, 0.05f ) ;
	    pto->pos_v.vy = 30.0f ;
	    pto->rot_vx = DG_FTOI( pto->pos_v.vx * 0.1f );
	    pto->rot_vy = DG_FTOI( pto->pos_v.vy * 0.1f );

	    pto->work->flag = 1 ;
	    pto->act = BRK_PTO_Act ;

	    return ;
	}
	else if ( off->weapon_type & (WP_BLOW|WP_BLAST|WP_STUNGRENADE) )
	{
	    _sceVu0SubVector( &v, &def->center, &off->center ) ;
	    if ( (int)(v.vx / off->size.vx) ||
		 (int)(v.vy / off->size.vy) ||
		 (int)(v.vz / off->size.vz) ||
		 (int)(v.vx / 3000.0f) || (int)(v.vy / 3000.0f) || (int)(v.vz / 3000.0f) )
		return ;
	}
	else if ( !(off->weapon_type & WP_BULLET) )
	    return ;

	/* じゃがいもが壊れる時のカスの初期化 */
	if ( BRK_PTO_InitPotato( pto, pto->work->where, pto->work->p_def,
				 DG_FLAG_SHADE|DG_FLAG_FINISHCALC, BRK_PTO_ActBreak ) >=0 )
	{
	    if ( (pto->parts.prof = (PART *)GV_Malloc( sizeof(PART) * pto->work->p_def->n_models )) )
	    {
		GM_FreeTarget( &pto->target ) ;

		switch( pto->work->type )
		{
		case 1:
		    if ( off->power )
		    {
			_sceVu0ScaleVector( &v, &off->power->force, -0.1f ) ;
			NewCrushDust( &pto->pos, &v, 3 ) ;
		    }
		    GM_SeSetMode( SD_A_RICVGT01, &pto->pos, GM_SEMODE_BOMB ) ;
		    break ;
		case 0:
		    GM_SeSetMode( SD_A_RICVGT01, &pto->pos, GM_SEMODE_BOMB ) ;
		    break ;
		}

		pto->work->flag = 1 ;

		/* setup for breaking the object */
		p = pto->parts.prof ;
		m = pto->work->p_def->models ;
		o = pto->objs->objs ;
		for ( i=pto->work->p_def->n_models ; --i>=0 ; m++, p++, o++ )
		{
		    p->obj = o ;
		    p->pos.vx = m->tx + pto->pos.vx ;
		    p->pos.vy = m->ty + pto->pos.vy ;
		    p->pos.vz = m->tz + pto->pos.vz ;
#if 0
		    p->pos_v.vx = DG_FTOI( -m->tx*(frnd()+1.0f) + off->power->force.vx*0.05f );
		    p->pos_v.vy = DG_FTOI( -m->ty*(frnd()+1.0f) + 40.0f );
		    p->pos_v.vz = DG_FTOI( -m->tz*(frnd()+1.0f) + off->power->force.vz*0.05f );
#else
		    p->pos_v.vx = ( -40.0f*frnd() + off->power->force.vx*0.05f );
		    p->pos_v.vy = ( -40.0f*frnd() + 40.0f );
		    p->pos_v.vz = ( -40.0f*frnd() + off->power->force.vz*0.05f );
#endif
		    p->rot_vx = DG_FTOI( p->pos_v.vx * 0.1f );
		    p->rot_vy = DG_FTOI( p->pos_v.vy * 0.1f );

		    p->act = BRK_PTO_ActPartBreaking ;
		}
	    }
	    else
		DG_DequeueObjs( pto->objs ), DG_FreeObjs( pto->objs );
	}
    }
}


static inline void HitAtBox( FVECTOR *offs, FVECTOR *hit, BOX *b, int k,
							 FVECTOR  *min, FVECTOR  *max,
#ifdef PSX2
							 SVECTOR *n
#else
 							 DG_VERTEX_KMSS *n
#endif
							)
{
	DG_PRIM2_UVRGB *rgb = b->scar[k]->uvrgb[0] ;
	FVECTOR        *pos = b->scar[k]->pos[0]   ;
	FVECTOR su, sl ;
	
	/* プリミティブのポリゴンを選択 */
	rgb += b->n_scar[k] * BRK_N_POLY_VERTS ;
	pos += b->n_scar[k] * BRK_N_POLY_VERTS ;
	if ( ++b->n_scar[k] >= BRK_N_SCAR )
	  b->n_scar[k] = 0 ;
	
	/* 弾痕の大きさを計算 */
	sl.vx = su.vx = n->vx>>10 ? 0.0f : 30.0f ;
	sl.vy = su.vy = n->vy>>10 ? 0.0f : 30.0f ;
	sl.vz = su.vz = n->vz>>10 ? 0.0f : 30.0f ;
	sl.vx *= n->vy>>10 ? -1.0f : 1.0f ;
	sl.vy *= n->vz>>10 ? -1.0f : 1.0f ;
	sl.vz *= n->vx>>10 ? -1.0f : 1.0f ;
	
	/* 弾痕をつける */
	rgb[0].a = rgb[1].a = rgb[2].a = rgb[3].a = 80 ;
	_sceVu0AddVector( &pos[0], hit, &sl ) ;
	_sceVu0AddVector( &pos[1], hit, &su ) ;
	_sceVu0SubVector( &pos[2], hit, &su ) ;
	_sceVu0SubVector( &pos[3], hit, &sl ) ;
	MinMaxVector( min, max, pos ) ;/*はみ出し禁止*/
	
	/* hitを元に戻す */
	_sceVu0CopyVector( hit, offs ) ;
}

static void SetScar( BOX *b, TARGET *off, TARGET *def )
{
    int         i, j, k ;
    DG_OBJ     *obj ;
    DG_MDLPACK *m ;
    FMATRIX     mtx ;
    FVECTOR     offs, hit, frc ;


    /*各蓋 箱に対し 弾のポリゴン当たり判定*/
    for ( k=5 ; --k>=0 ; ) {
		obj = &b->objs->objs[k] ;

		/*ローカル座標系での着弾位置と着弾方向を求める*/
		_sceVu0CopyVector( &frc , &off->power->force ) ;
		_sceVu0CopyVector( &offs, &def->hit ) ;
		offs.vw = 1.0f ;
		frc.vw  = 0.0f ;
		_sceVu0InversMatrix( &mtx, &obj->world ) ;
		_sceVu0ApplyMatrix( &frc , &mtx, &frc  ) ;
		_sceVu0ApplyMatrix( &offs, &mtx, &offs ) ;
		_sceVu0CopyVector( &hit, &offs ) ;

		for ( m=obj->model->packs, i=obj->model->n_packs ; --i>=0 ; m++ ) {
			FVECTOR min = { obj->model->lx, obj->model->ly, obj->model->lz } ;
			FVECTOR max = { obj->model->ux, obj->model->uy, obj->model->uz } ;

#ifdef PSX2
			SVECTOR    *s, *n ;

			s = (SVECTOR *)m->verts ;
			n = (SVECTOR *)m->norms ;
			for ( j=m->n_verts ; --j>=0 ; s++, n++ ) {
				if ( !(n->pad & 0x8000) ) {
					if ( BRK_UTL_CheckLineInPolygon( &hit, &frc, s-2 ) ) {
						HitAtBox( &offs, &hit, b, k,
								  &min, &max, n ) ;
					}
				}
			}
#else
			DG_VERTEX_KMSS *kmss = obj->vbuff ;
			int v0, v1, v2 ;

			v0 = v1= v2= *(short*)m->index ;
			for ( j=0 ; j<m->n_indices ; j++ ) {
				if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
					if ( BRK_UTL_CheckLineInPolygon( &offs, &frc,
													 kmss+v0,
													 kmss+v1,
													 kmss+v2 ) ) {
						HitAtBox( &offs, &hit, b, k,
								  &min, &max, kmss+v0 ) ;
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


void BRK_PTO_BoxTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    FVECTOR v, h, l ;
    FVECTOR *force ;
    int     i ;
    BOX    *b = &work->box ;

    if ( off->class & TARGET_SEEK )
	BRK_UTL_CallOffenceWhenThrough( off, def ) ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( b->non_dmg )
	    return ;
	else if ( off->weapon_type & (WP_PUNCHALL|WP_BLOW|WP_BLAST|WP_STUNGRENADE))
	{
	    if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
		return ;
	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->center, off ) ;
	}
	else if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    SetScar( b, off, def ) ;
	    GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;
	    force = &off->power->force ;
	}
	else
	    return ;
	b->non_dmg = 8 ;

	/*当たった場所をローカル座標に変換して挙動を制御する */
	_sceVu0ApplyMatrix( &l, &def->world, &def->offset ) ;
	_sceVu0AddVector( &l, &l, &def->center ) ;
	_sceVu0SubVector( &l, &l, &def->hit ) ;
	_sceVu0Normalize( &h, &l ) ;
	_sceVu0ScaleVector( &v, &h, _sceVu0InnerProduct( force, &h ) ) ;
	_sceVu0SubVector( &h, force, &v ) ;
	h.vw = (h.vx*h.vx + h.vz*h.vz) * 0.0005f ;

	if ( b->vitality <= 0 )/* 倒れた後の箱の挙動 */
	{
	    b->brot_v.vx = DG_FTOI( v.vx*0.05f );/* 揺らす力 */
	    b->brot_v.vz = DG_FTOI( v.vz*0.05f );
	    b->brot_v.vy = 0 ;

	    b->act = BRK_PTO_ActOpenedBox ;      /*箱は揺れる*/
	    for ( i=b->objs->n_models ; --i>0 ; )/*蓋の揺れ設定*/
		b->rot[i] += (irnd()&1023) - 512 ;
	}
	else
	{
	    b->brot_v.vx = DG_FTOI( v.vx*0.1f ); /* 揺らす力 強め */
	    b->brot_v.vz = DG_FTOI( v.vz*0.1f );
	    b->brot_v.vy = DG_FTOI( ( l.vx*h.vz-l.vz*h.vx>0.0f ? h.vw : -h.vw ) );/* 回転する力 */

	    b->act = BRK_PTO_ActBox ; /*箱は揺れる*/
	    if ( b->vitality > 2 ) /*箱の耐久は十分か？*/
		b->vitality-- ;
	    else                   /*箱の耐久がもうなくなる？*/
	    {
		POTATO *p ;

		/*じゃがいものメモリを確保できそうか？*/
		for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
		    if ( BRK_PTO_InitPotato( p, work->where, work->m_def,
					     DG_FLAG_SHADE|DG_FLAG_ONEPIECE,
					     BRK_PTO_Act ) >=0 )
			DG_InvisibleObjs( p->objs ) ;
		    else
			break ;

		if ( i < 15 ) /* ある程度確保できそうなら箱を倒す */ 
		{
		    b->brot_v.vz += (short)(v.vx*0.1f) ;/* 倒す時は強めに揺らす */
		    if ( work->flag & BRK_FLG_BOTHSIDE )
			b->brot_v.vz += (short)(v.vz*0.1f) ;
		    else
			b->brot_v.vx = 60 ;

		    /*箱に蓋付きあたりを貼る*/
		    for ( i=b->objs->n_models ; --i>0 ; )
		    {
			FVECTOR t_size, t_pos ;
			DG_MDL *m = &b->objs->def->models[i] ;

			BRK_PTO_MakeSizeAndCenter( &t_size, &t_pos, &m->ux, &m->lx ) ;
			work->box_hzd[i] = BRK_MakeHazard( BRK_HZD_OUTSIDE| BRK_HZD_ROTATE,
							   &b->objs->objs[i].world, NULL,
							   &t_size, &t_pos, NULL ) ;
		    }
		    work->box_hzd[0]->flag = BRK_HZD_Y_BOX| BRK_HZD_ROTATE ;/* Y軸箱当たりに変更 */

		    /* 箱が倒れるアクトに切替え */
		    b->vitality-- ;
		    b->act = BRK_PTO_ActFallingDownBox ;

		    /* 壊れプロックを呼び出す */
		    if ( work->proc_id )
			GCL_ExecProc( work->proc_id, NULL ) ;

		    /* SEを鳴らす */
		    GM_SeSetMode( SD_A_DANBOR01, &b->pos, GM_SEMODE_BOMB ) ;
		    /* 敵兵用の見つかり */
		    GM_SetNoise( NOISE_MM, &b->pos, work->where ) ;
		    work->ene_find.type = EF_TYPE_LV3 ;
		    GM_PutEneFind( &work->ene_find ) ;
		}
		else /* ある程度確保できそうでないなら解放する */ 
		    for ( i=work->n_potato, p=work->potato ; --i>=0 && p->objs ; p++ )
			if ( p->objs )
			{
			    GM_FreeTarget( &p->target ) ;
			    DG_DequeueObjs( p->objs ) ;
			    DG_FreeObjs( p->objs ) ;
			    p->objs = NULL ;
			}
	    }
	}
    }
}
