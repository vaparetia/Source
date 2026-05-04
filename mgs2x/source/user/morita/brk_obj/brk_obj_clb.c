//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_obj_clb.c
   オブジェクト壊れ

   1999/12/13 T. Morita
   $Id: brk_obj_clb.c,v 1.1.1.3 2002/11/19 11:45:37 Yoshizawa1 Exp $
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

#include "brk_object.h"



void BRK_OBJ_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    BRK_OBJ *obj = (BRK_OBJ *)ptr ;
    Work    *work = obj->work ;
    PART    *p ;
    int      i, flag ;
    DG_MDL  *m ;
    DG_OBJ  *o ;
    FVECTOR  frc ;
    DG_DEF  *p_def ;
    FVECTOR *force ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( !(work->flag & BRK_F_NO_KICK) &&
	     off->weapon_type & (WP_PUNCHALL|WP_BLOW) )
	{
	    if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
		return ;
	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->center, off ) ;
	}	    
	else if ( off->weapon_type & (WP_BLAST|WP_STUNGRENADE) )
	{
	    if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
		return ;
	}
	else if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    force = &off->power->force ;
	else
	    return ;

	_sceVu0ScaleVector( &frc, force, 0.05f ) ;

	/* メモリーの確保とモデルの初期化 */
	p_def = obj->work->part_def ;
	flag = (obj->objs->flag & (DG_FLAG_SHADE|DG_FLAG_PAINT)) | DG_FLAG_FINISHCALC ;
	if ( (obj->parts.prof = (PART *)GV_Malloc( sizeof(PART) * p_def->n_models )) )
	{
	    if ( BRK_OBJ_InitObject( obj, p_def, flag, BRK_OBJ_ActBreak ) >=0 )
	    {
		/* オブジェを稼働状態にする */
		work->n_obj &= ~BRK_OBJ_INACTIVE ;

		GM_FreeTarget( &obj->target ) ;

		/* SEを鳴らす*/
		if ( work->flag & BRK_F_RND_BRK_SE )
		    GM_SeSetMode( (irnd()&3)+work->brk_se_id, &obj->pos, GM_SEMODE_BOMB ) ;
		else
		    GM_SeSetMode( work->brk_se_id, &obj->pos, GM_SEMODE_BOMB ) ;

		/* 敵兵見つかり用 */
		if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
		{
		    GM_SetNoise( NOISE_S, &obj->pos, work->where ) ;
		    _sceVu0CopyVector( &work->ene_find.pos, &obj->pos ) ;
		    work->ene_find.type = EF_TYPE_LV2 ;
		}

		/* 壊れモデルに移行するための初期化（速度,位置,回転など） */
		obj->n_parts = p_def->n_models ;
		p = obj->parts.prof ;
		m = p_def->models ;
		o = obj->objs->objs ;
		for ( i=obj->n_parts ; --i>=0 ; m++, p++, o++ )
		{
		    p->obj = o ;

		    if ( work->flag & BRK_F_BRK_SPREAD )
		    {
			p->pos.vx = obj->pos.vx + p_def->tx + m->tx ;
			p->pos.vy = obj->pos.vy + p_def->ty + m->ty ;
			p->pos.vz = obj->pos.vz + p_def->tz + m->tz ;
			p->pos_v.vx = 80.0f*frnd() ;
			p->pos_v.vy = 80.0f* rnd() + work->pop_up ;
			p->pos_v.vz = 80.0f*frnd() ;
		    }
		    else
		    {
			p->pos.vx = obj->pos.vx + p_def->tx + m->tx ;
			p->pos.vy = obj->pos.vy + p_def->ty + m->ty ;
			p->pos.vz = obj->pos.vz + p_def->tz + m->tz ;
			p->pos_v.vx = -(m->ux + m->lx)*frnd() ;
			p->pos_v.vy =  (m->uy + m->ly)* rnd() + work->pop_up ;
			p->pos_v.vz = -(m->uz + m->lz)*frnd() ;
		    }
		    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, work->explode ) ;
		    _sceVu0AddVector( &p->pos_v, &p->pos_v, &frc ) ;
		    p->pos_v.vy = fpu_Abs( p->pos_v.vy ) ;
		    p->rot_v.vx = (irnd() & 63) - 32 ;
		    p->rot_v.vz = (irnd() & 63) - 32 ;
		    p->rot_v.vy = (irnd() & 63) - 32 ;
		    p->flag     = 1 ;
		    p->se_flag  = 1 ;
		}

		/* 壊れプロックがあったら 呼び出す */
		if ( work->proc_id )
		    GCL_ExecProc( work->proc_id, NULL ) ;

		/* 埃の指定があったら 埃を出す*/
		GM_CurrentMap = work->where ;
		work->counter = 60*20 ;
		if ( work->dust_mode == 2 )     /* 塩 */
		    NewSaltSplay( &obj->objs->world ) ;
		else if ( work->dust_mode == 3 )/* ケチャップ*/
		    NewKetchapSpread( work->hzd, &obj->pos,
				      force, 0x70000a3a,
				      2.0f + 0.5f*(int)(obj - work->obj) ) ;
		else if ( work->dust_mode == 4 )/* ソース */
		    NewKetchapSpread( work->hzd, &obj->pos,
				      force, 0x70000810,
				      1.0f + 0.5f*(int)(obj - work->obj) ) ;
		else if ( work->dust_mode == 5 )
		    NewCrushDust( &obj->pos, &frc, 3 ) ;
		else if ( work->dust_mode == 6 )
		    NewCrushDust( &obj->pos, &frc, 4 ) ;
		else if ( work->dust_mode >=0 )
		    NewCrushDust( &obj->pos, &frc, work->dust_mode ) ;

		/* 破片の指定があったら 破片を出す*/
		if ( work->comdl[0] )
		    BRK_OBJ_StartActPieces( work, 32,
					    &obj->pos,
					    &DG_ZeroVector,
					    work->r_size.vx*0.5f*work->explode, 20 ) ;
	    }
	    else
		GV_Free( obj->parts.prof ) ;
	}
    }
}
