//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_clb.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_b_gls_clb.c,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
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

#include "brk_big_glass.h"


void BRK_BGLS_PutGlassWeb( Work *work, FVECTOR *hit )
{
    FVECTOR *v0, *v1, ofst ;
    float    uv_b[4] = { 0.5f, 0.5f, 0.5f, 0.5f } ;
    TARGET  *t = &work->target ;

    if ( (work->n_frac += 1) >= BRK_BGLS_N_SCAR )
	work->n_frac = 0 ;

    /* 取りあえずプリミティブの形を作る */
    v0 = work->frac[0]->pos[0] + work->n_frac*5 ;
    v1 = work->frac[1]->pos[0] + work->n_frac*5 ;
    v0[0].vx = v0[2].vx = v1[0].vx = v1[2].vx = hit->vx - BRK_BGLS_SCAR_W ;
    v0[1].vx = v0[3].vx = v1[1].vx = v1[3].vx = hit->vx + BRK_BGLS_SCAR_W ;
    v0[0].vy = v0[1].vy = v1[0].vy = v1[1].vy = hit->vy - BRK_BGLS_SCAR_W ;
    v0[2].vy = v0[3].vy = v1[2].vy = v1[3].vy = hit->vy + BRK_BGLS_SCAR_W ;
    v0[0].vz = v0[1].vz = v0[2].vz = v0[3].vz = t->offset.vz-5 ;
    v1[0].vz = v1[1].vz = v1[2].vz = v1[3].vz = t->offset.vz+5 ;
    v0[4].vx = v0[4].vy = v0[4].vz = t->offset.vz - 5000 ;
    v1[4].vx = v1[4].vy = v1[4].vz = t->offset.vz + 5000 ;

    /* ガラスからはみ出ていないかを見る */
    _sceVu0SubVector( &ofst, hit, &t->offset ) ;
    if ( (int)(ofst.vx / (t->size.vx-BRK_BGLS_SCAR_W)) )
    {
	if ( ofst.vx > 0 )/* 横がはみ出た 左右のどっち側？ */
	    uv_b[0] *= 1.0f - (v0[1].vx - (t->offset.vx + t->size.vx))/BRK_BGLS_SCAR_W,
		v0[1].vx = v0[3].vx = v1[1].vx = v1[3].vx = t->offset.vx + t->size.vx ;
	else
	    uv_b[1] *= 1.0f - ((t->offset.vx - t->size.vx) - v0[0].vx)/BRK_BGLS_SCAR_W,
		v0[0].vx = v0[2].vx = v1[0].vx = v1[2].vx = t->offset.vx - t->size.vx ;
    }
    if ( (int)(ofst.vy / (t->size.vy-BRK_BGLS_SCAR_W)) )
    {
	if ( ofst.vy > 0 )/* 縦がはみ出た 上下のどっち側？ */
	    uv_b[2] *= 1.0f - (v0[2].vy - (t->offset.vy + t->size.vy))/BRK_BGLS_SCAR_W,
		v0[2].vy = v0[3].vy = v1[2].vy = v1[3].vy = t->offset.vy + t->size.vy ;
	else
	    uv_b[3] *= 1.0f - ((t->offset.vy - t->size.vy) - v0[0].vy)/BRK_BGLS_SCAR_W,
		v0[0].vy = v0[1].vy = v1[0].vy = v1[1].vy = t->offset.vy - t->size.vy ;
    }

    /* テクスチャーのUVを計算（はみ出たら,それを考慮する） */
    {
	DG_PRIM2_UVRGB *u0, *u1 ;
	DG_TEX  *t ;
	float    w, h, l ;

	u0 = (DG_PRIM2_UVRGB *)work->frac[0]->uvrgb[0] + work->n_frac*5 ;
	u1 = (DG_PRIM2_UVRGB *)work->frac[1]->uvrgb[1] + work->n_frac*5 ;
	t = DG_GetTexture( GV_StrCode( "g_dankon_alp" ) ) ;
	l = frnd() ; w = 0.707f * cosf( l ) ; h = 0.707f * sinf( l ) ;/*ランダムに回転*/
	u0[0].u = u1[0].u = FTOI12( (0.5f-w*uv_b[1]+h*uv_b[3]) * t->u_scale + t->u_offset ) ;
	u0[0].v = u1[0].v = FTOI12( (0.5f-h*uv_b[1]-w*uv_b[3]) * t->v_scale + t->v_offset ) ;
	u0[1].u = u1[1].u = FTOI12( (0.5f+w*uv_b[0]+h*uv_b[3]) * t->u_scale + t->u_offset ) ;
	u0[1].v = u1[1].v = FTOI12( (0.5f+h*uv_b[0]-w*uv_b[3]) * t->v_scale + t->v_offset ) ;
	u0[2].u = u1[2].u = FTOI12( (0.5f-w*uv_b[1]-h*uv_b[2]) * t->u_scale + t->u_offset ) ;
	u0[2].v = u1[2].v = FTOI12( (0.5f-h*uv_b[1]+w*uv_b[2]) * t->v_scale + t->v_offset ) ;
	u0[3].u = u1[3].u = FTOI12( (0.5f+w*uv_b[0]-h*uv_b[2]) * t->u_scale + t->u_offset ) ;
	u0[3].v = u1[3].v = FTOI12( (0.5f+h*uv_b[0]+w*uv_b[2]) * t->v_scale + t->v_offset ) ;
	u0[0].a = u0[1].a = u0[2].a = u0[3].a = 128 ;
	u1[0].a = u1[1].a = u1[2].a = u1[3].a = 128 ;
    }
}

void BRK_BGLS_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    void *NewGunSmoke( FVECTOR *start, FVECTOR *speed, float size, u_char mode) ;
    FVECTOR v ;
    FVECTOR *pos ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BLAST|WP_STUNGRENADE) )
	{
	    _sceVu0SubVector( &v, &def->offset, &off->center ) ;
	    if ( (int)(v.vx / 4000.0f) ||
		 (int)(v.vy / 3000.0f) ||
		 (int)(v.vz / 4000.0f) )
		return ;
	}
	else if ( off->weapon_type & WP_WEAPONCORE )
	{
	    /* たまをとめる */
	    BRK_UTL_CallOffenceWhenThrough( off, def ) ;
	    return ;
	}
	else if ( !(off->weapon_type & (WP_BULLET|WP_M92)) )
	    return ;

	if ( !work->act )
	{
	    if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    {
		pos = &def->hit ;
		/* 煙を出す */
		_sceVu0ScaleVector( &v, &off->power->force, -0.01f ) ;
		NewGunSmoke( pos, &v, 100.0f, 0 ) ;

		/* ちょっとした破片を出す */
		BRK_BGLS_StartActPieces( work,  4, pos, &v            ,  2.0f ) ;
		BRK_BGLS_StartActPieces( work, 16, pos, &DG_ZeroVector, 20.0f ) ;

		/* たまをとめる */
		BRK_UTL_CallOffenceWhenThrough( off, def ) ;

		/* ガラスの弾痕を出す */
		BRK_BGLS_PutGlassWeb( work, pos ) ;
		/* SEを鳴らす */
		GM_SeSetMode( SD_A_HIBI01, pos, GM_SEMODE_BOMB ) ;
	    }
	    else
	    {
		pos = &def->offset ;
		/* これをやらないとくずれない */
		if ( !work->n_frac )
		    work->n_frac = 1 ;
	    }

	    /* 敵兵見つかり用 */
	    GM_SetNoise( NOISE_S, pos, work->where ) ;
	    //if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	    {
		_sceVu0CopyVector( &work->ene_find.pos, pos ) ;
		work->ene_find.type = EF_TYPE_LV2 ;
	    }

	    if ( work->life )
		if ( off->weapon_type & (WP_STUNGRENADE|WP_BLOW|WP_BLAST) )
		{
		    /* 振動 */
		    BRK_UTL_PK_Vibrate( &def->hit, off ) ;

		    work->life = 0 ;
		}
	    if ( !work->life )
	    {
		if ( !BRK_BGLS_InitPart( work ) )
		{
		    work->life_acs = 0 ;
		    work->life = 4 ;
		    work->act = BRK_BGLS_ActFracture ;
		    if ( work->pr_name )
			GCL_ExecProc( work->pr_name, NULL ) ;
		    GM_FreeTarget( def ) ;
		    /* とりあえずENEFINDを切る */
		    GM_FreeEneFind( &work->ene_find ) ;
		}
	    }
	    else
		work->life-- ;
	}
    }
}
