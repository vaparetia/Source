//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_clb.c
   瓶壊れ コールバック

   1999/12/02 T. Morita
   $Id: brk_btl_clb.c,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
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

#include "brk_bottle.h"

/*とりあえず正確にワインを垂らすためのコード*/
void BRK_BTL_WineSplash( Work *work, BOTTLE *b )
{
    FVECTOR *h ;
    FVECTOR uv = DG_ZeroVector, lv = DG_ZeroVector ;
    float   limit ;

#if 0
    for ( h=work->hzd ; h->vw > -5.0f ; )/*このfor文は丸々要らない*/
#endif
    if ( (h = work->hzd) )/*このfor文は丸々要らない*/
	switch( (int)h->vw )
	{
	case BTK_BTL_HZD_BOX:
	case BTK_BTL_HZD_TUNNEL:
	    _sceVu0AddVector( &uv, &h[0] , &h[1] ) ;
	    _sceVu0SubVector( &uv, &uv, &b->pos ) ;
	    _sceVu0SubVector( &lv, &h[0] , &h[1] ) ;
	    _sceVu0SubVector( &lv, &lv, &b->pos ) ;
	    break ;
	case BTK_BTL_HZD_BALL:
	    break ;
	default:
		;
	}

    if ( !(b->flag & BRK_BTL_F_NOWINE) )
    {
	GM_CurrentMap = work->where ;
	/* ワインがたれる */
	switch( work->hzd_id )
	{
	case BRK_HZD_W01A1_BAR:     
	    NewWineStream( &b->objs->world, uv.vz ) ;
	    break ;

	case BRK_HZD_W24B_BOTTLEKEEP:
	case BRK_HZD_W24B_REFRESH:
	    limit = MIN( fpu_Abs(uv.vz), fpu_Abs(lv.vz) ) ;
	    limit = MIN( limit         , fpu_Abs(uv.vx) ) ;
	    limit = MIN( limit         , fpu_Abs(lv.vx) ) ;
	    NewWineStreamLimit( &b->objs->world, uv.vx, limit ) ;
	    break ;

	case BRK_HZD_W01D_STORERAGE:
	    NewWineStreamFall( &b->objs->world, work->where ) ;
	    break ;
	}
    }
}

void BRK_BTL_BreakBottle( TARGET *off, TARGET *def, PART *p, FVECTOR *force )
{
    PART   *q, *up, *low ;
    BOTTLE *b = p->bottle ;
    Work   *work = b->work ;
    int     i ;
    FVECTOR offs, v ;

    /* def->hit が変なので  当たった場所をvにする */
    _sceVu0ApplyMatrix( &v, &p->target->world, &p->target->offset ) ;
    _sceVu0AddVector( &v, &v, &p->target->center ) ;

    GM_SeSetMode( SD_A_BOTTLE01, &v, GM_SEMODE_BOMB ) ;
	GM_SetNoise( NOISE_MM, &v, work->where ) ;
	if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	{
	    work->ene_find.type = EF_TYPE_LV2 ;
	}
	_sceVu0CopyVector( &work->ene_find.pos, &v ) ;

    if ( b->act != BRK_BTL_ActBreaking && b->act != BRK_BTL_ActFinished )
    {
	/* 初めてなので壊れモデルを初期化 */
	if ( BRK_BTL_InitModel( b, work->where, b->brk_af,
				DG_FLAG_PAINT|DG_FLAG_FINISHCALC, work->lit ) < 0 )
	    printf( "Can't Create Obj!! :: NewPutBottleObject\n" ) ;
	else
	{
	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->hit, off ) ;
	    /* ワインを出す */
	    BRK_BTL_WineSplash( work, b ) ;
	    /* 水飛沫を出す */
	    NewCrushDust( &v, force, 1 ) ;
	}
    }
    b->act = BRK_BTL_ActBreaking ; /*瓶のアクトは,壊れモードに*/
    work->n_bottles &= ~BRK_BOTTLE_INACTIVE ;/*ボトルのアクションをアクティブにする*/

    /*壊れる方*/
    p->target->class |= TARGET_SKIP ;   /* このターゲットに弾はもう当たらない*/
    p->obj->flag |= DG_FLAG_INVISIBLE ; /* 当たったパーツを消す */
    p->act = NULL ;
    BRK_BTL_StartActPieces( work, &v, 20.0f, NULL, 12, b->flag ) ;

#if MAKING
    if ( p->wireframe )
    {
	GV_DestroyOtherActor( p->wireframe ) ;
	p->wireframe = NULL ;
    }
#endif

    /* 壊れた本数によって別の壊れぷロックをよぶ */
    work->n_broken++ ;
    if ( work->n_broken > work->n_bottles/4 )
    {
	if ( work->proc_half )
	    GCL_ExecProc( work->proc_half, NULL ) ;
    }
    else if ( work->n_broken > work->n_bottles*3/4 )
    {
	if ( work->proc_full )
	    GCL_ExecProc( work->proc_full, NULL ) ;
    }

    /*落ちる方*/
    up = low = NULL ;
    _sceVu0SubVector( &offs, &v, &b->pos ) ;
    for ( q=b->parts, i=b->objs->n_models ; --i>=0 ; q++ )
	if ( !(q->obj->flag & DG_FLAG_INVISIBLE) )
	{
	    /*残る方(下側)*/
	    q = p - 1 ;
	    if ( p!=b->parts )
		if ( !(q->obj->flag & DG_FLAG_INVISIBLE) )
		{
		    while( q != b->parts-1 &&
			   !(q->obj->flag & DG_FLAG_INVISIBLE) )
			(q--)->act = BRK_BTL_ActPartCopyParent ;
		    low = ++q ;
		    if ( !(q->rot.vx/8) && !(q->rot.vz/8) )
		    {
			q->act = BRK_BTL_ActPartOnShelf ;
			if ( (int)(offs.vx/20.0f) )
			{
			    q->pos_v.vx = -force->vz*0.05f ;
			    q->pos_v.vz =  force->vx*0.05f ;
			    q->pos_v.vx = -offs.vx * 0.05f ;
			    q->rot_v.vy = (short)( (offs.vx > 12.8f ? 12.8f : offs.vx) * 2.0f) ;
			    i = (int)(offs.vx * offs.vy / 64.0f) ;
			    q->rot_v.vz = i/128 ? i>0 ? 128 : -128 : i ;
			}
			else
			{
			    q->pos_v.vx = force->vx*0.05f ;
			    q->pos_v.vz = force->vz*0.05f ;
			}
		    }
		    else
		    {
			q->rot_v.vx = (irnd() & 16) - 8 + (irnd() & 7) ;
			q->rot_v.vy = 0 ;
			q->rot_v.vz = (irnd() & 16) - 8 + (irnd() & 7) ;
			q->pos_v.vx = q->pos_v.vz = 0.0f ;
			q->pos_v.vy = 30.0f ;
			q->act = BRK_BTL_ActPartFalling ;
		    }
		}

	    /*落ちる方(上側)*/
	    q = p + 1 ;
	    if ( p != &b->parts[b->objs->n_models-1] )
		if ( !(q->obj->flag & DG_FLAG_INVISIBLE) )
		{
		    up = q ;
		    if ( !(q->rot.vx/8) && !(q->rot.vz/8) )
		    {
			q->pos_v.vy = 10.0f ;
			i = (int)(offs.vx / 64.0f) ;
			q->rot_v.vz = i/128 ? i>0 ? 256 : -256 : i ;
		    }
		    else
		    {	       
			q->rot_v.vx = (irnd() & 16) - 8 + (irnd() & 7) ;
			q->rot_v.vy = 0 ;
			q->rot_v.vz = (irnd() & 16) - 8 + (irnd() & 7) ;
			q->pos_v.vy = 30.0f ;
		    }
		    q->pos_v.vx = q->pos_v.vz = 0 ;
		    q->act = BRK_BTL_ActPartFalling ;

		    offs.vy = up->pos.vy ;
		    v.vw = v.vx = v.vz = 0.0f ;
		    v.vy = up->obj->model->ly - 1*(low ? low->obj->model->ly : p->obj->model->ly) ;
		    _sceVu0ApplyMatrix( &v, &up->target->world, &v ) ;
		    _sceVu0AddVector( &up->pos, &up->pos, &v ) ;

		    BRK_BTL_MovePart( up, work->where ) ;

		    q++ ;
		    while( q != &b->parts[b->objs->n_models] &&
			   !(q->obj->flag & DG_FLAG_INVISIBLE))
			(q++)->act = BRK_BTL_ActPartCopyParent ;
		}
	    return ;
	}

    BRK_BTL_FreeBottle( b ) ;/*この瓶は完全に壊れた*/
    b->act = BRK_BTL_ActFinished ;
}

void BRK_BTL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    PART   *p = (PART *)ptr ;
    FVECTOR *force ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BLOW|WP_BLAST|WP_STUNGRENADE|WP_PUNCHALL) )
	{
	    if ( p->bottle->act || !(def->class & TARGET_CHILD) )
		return ;
	    if ( !(force = BRK_UTL_BlowBlastCheck( &def->center, off )) )
		return ;
	}
	else if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    force = &off->power->force ;
	else
	    return ;
	BRK_BTL_BreakBottle( off, def, p, force ) ;
    }
}
