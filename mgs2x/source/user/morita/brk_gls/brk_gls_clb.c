//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_clb.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_gls_clb.c,v 1.1.1.3 2002/11/19 11:45:29 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_glass.h"



static void BRK_GLS_SetToBreakPrim( Work *work )
{
    /*  可視不可視制御 */
    DG_InvisiblePrim2( work->glass ) ;
    DG_VisiblePrim2( work->broken  ) ;
    DG_VisiblePrim2( work->thick   ) ;
    if ( work->mirror_tx )
    {
	work->broken->flag    |=  DG_PRIM2_INVISIBLE1 ;
	work->thick->flag     |=  DG_PRIM2_INVISIBLE1 ;
	//work->mirror_bg->flag &= ~DG_PRIM2_INVISIBLE1 ;
	work->mirror->flag    &= ~DG_PRIM2_INVISIBLE0 ;
	//work->mirror_br->flag &= ~DG_PRIM2_INVISIBLE0 ;
	//work->mirror_th->flag &= ~DG_PRIM2_INVISIBLE0 ;
    }

    if ( work->fog_mist )
	GV_DestroyOtherActor( work->fog_mist ), work->fog_mist = NULL ;
#if 0
    if ( work->piece_l )
	work->piece_l->flag &= ~DG_FLAG_INVISIBLE ;
    if ( work->piece_s )
	work->piece_s->flag &= ~DG_FLAG_INVISIBLE ;
#endif
}

static void CallOffenceWhenThrough( Work *work, TARGET *off, TARGET *def )
{
    if ( work->proc )
	GCL_ExecProc( work->proc, NULL ) ;

    GM_SetNoise( NOISE_S, &def->hit, work->where ) ;
    /* 敵兵見つかり用 */
    if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
    {
	_sceVu0CopyVector( &work->ene_find.pos, &def->hit ) ;
	work->ene_find.type = EF_TYPE_LV2 ;
    }

    /* 弾を止める処理 */
    off->hit = def->hit ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
        ( *off->callback )( off, def, off->work ) ;
}


static inline void SetAlpha( int i, DG_PRIM2_UVRGB *uvs, int alpha )
{
    while( --i>=0 )
	(uvs++)->a = alpha ;
}

static inline void SetZeroPoint( int i, FVECTOR *pos )
{
    while( --i>=0 )
	_sceVu0CopyVector( pos++, &DG_ZeroVector ) ;
}


static inline void BreakPiece( Work *work, POLY *p,
			       FVECTOR *hit, FVECTOR *force,
			       int se_id, int n_piece )
{
#if 0
    FVECTOR pos ;
    extern void *NewGlassDustPrim( HZD_BOX *hzd,
				   FMATRIX *world,
				   FVECTOR *pos,
				   FVECTOR *frc,
				   int n_piece,
				   int tex_id, int se_id ) ;

    _sceVu0AddVector( &pos, p->v0, p->v1 ) ;
    _sceVu0AddVector( &pos, &pos, p->v2 ) ;
    _sceVu0ScaleVector( &pos, &pos, 0.333333333f ) ;
    pos.vw = 1.0f ;
    _sceVu0ApplyMatrix( &pos, &work->world, &pos ) ;

    GM_CurrentMap = work->where ;
    NewGlassDustPrim( work->hzd,
		      &work->world,
		      &pos,
		      force,
		      n_piece,
		      8291175 /*dbx_dust_alp_ovl.bmp*/,
		      sei_id ) ;
#else
    extern void *NewGlassDustPrim( HZD_BOX *hzd,
				   FMATRIX *world,
				   FVECTOR *p0, FVECTOR *p1, FVECTOR *p2,
				   FVECTOR *hit, FVECTOR *frc,
				   int n_piece,
				   int tex_id, int se_id ) ;
    GM_CurrentMap = work->where ;
    NewGlassDustPrim( work->hzd,
		      &work->world,
		      p->v0, p->v1,  p->v2,
		      hit, force,
		      n_piece,
		      8291175 /*dbx_dust_alp_ovl.bmp*/,
		      se_id ) ;
#endif
}

static inline int BRK_GLS_CheckTriangle( FVECTOR *pos,
										 FVECTOR *v0,
										 FVECTOR *v1,
										 FVECTOR *v2 )
{
    float a,b,c ;

    /* 交点が3頂点の内側にあるかどうかを判定する */
    a = BackFaceCullingXY( v0, v1, pos ) ;
    b = BackFaceCullingXY( v1, v2, pos ) ;
    c = BackFaceCullingXY( v2, v0, pos ) ;

    if ( (a>0.0f && b>0.0f && c>0.0f) || (a<0.0f && b<0.0f && c<0.0f) )
	return 1 ;

    return 0 ;
    
}

int BRK_GLS_CheckLineInPolygon( FVECTOR *lpos, FVECTOR *ldir, POLY *p )
{
    FVECTOR a0, a1, pnrm ;
    float a, b ;

    /* 法線 pnrmを求める    */
    _sceVu0SubVector( &a0, p->v0, p->v1 ) ;
    _sceVu0SubVector( &a1, p->v1, p->v2 ) ;
    _sceVu0OuterProduct( &pnrm, &a0, &a1 ) ;

    /* 直線が平面に対して平行なら貫通しない */
    if ( (b = _sceVu0InnerProduct( ldir, &pnrm )) == 0.0f )
	return 0 ;

    /* 直線と平面の交点を求める */
    _sceVu0SubVector( &a0, p->v0, lpos ) ;
    a = _sceVu0InnerProduct( &pnrm, &a0 ) ;
    _sceVu0ScaleVector( &a0, ldir, a/b ) ;
    _sceVu0AddVector( &a0, &a0, lpos ) ;

    return BRK_GLS_CheckTriangle( &a0, p->v0, p->v1, p->v2 ) ;
}

/* 近くにポリゴンがあるかどうか */
int BRK_GLS_CheckNearPolygon( FVECTOR *pos, POLY *p )
{
    FVECTOR v, t ;
    FVECTOR c    ;

    _sceVu0AddVector( &v, p->v0, p->v1 ) ;
    _sceVu0AddVector( &v, &v, p->v2 ) ;
    _sceVu0ScaleVector( &v, &v, 0.333333333f ) ;
    _sceVu0SubVector( &c, &v, pos ) ;
    _sceVu0SubVector( &t, &v, p->v1 ) ;
    if ( _sceVu0InnerProduct( &t, &t ) > _sceVu0InnerProduct( &c, &c ) )
	return 1 ;

    return 0 ;
}


/* 

   FANストリップを作る

*/
void BRK_GLS_MakeStrip( Work *work, POLY *p, POLY *q )
{
    FVECTOR  *v0, *v1, *v2 ;
    FVECTOR   v, *vert ;
    int       i ;
    float     leng ;
    FVECTOR        *f_gls_p, *b_gls_p, *thick ;
    DG_PRIM2_UVRGB *f_gls_u, *b_gls_u ;
    float           size_x, size_y, pos_x, pos_y ;
    float           face ;
    DG_TEX         *t ;

    size_x = work->def->models->ux - work->def->models->lx ;
    size_y = work->def->models->uy - work->def->models->ly ;
    t = work->broken_tx ;

    f_gls_p = p->f_gls_p ;
    b_gls_p = p->b_gls_p ;
    thick   = p->thick_p ;
    f_gls_u = p->f_gls_u ;
    b_gls_u = p->b_gls_u ;

    /* 割れた先でどの辺にするか決める 下で作るのはv2を基点としたFANス
       トリップ */
    p->p0==q ? (v0 = p->v0, v1 = p->v1, v2 = p->v2) :
    p->p1==q ? (v0 = p->v1, v1 = p->v2, v2 = p->v0) :
	       (v0 = p->v2, v1 = p->v0, v2 = p->v1) ;
    face = -BackFaceCullingXY( v0, v1, v2 ) ;

    /* 長さを求める */
    _sceVu0SubVector( &v, v1, v0 ) ;
    leng = sceVu0Sqrt( _sceVu0InnerProduct( &v, &v ) ) * 0.1f ;

    for ( i=0 ; i<BRK_GLS_NVERTS ; i++ )
    {
	/* ローカル位置(vert)を作る */
	if ( i == BRK_GLS_NVERTS-1 )
	    vert = v1 ;
	else if ( i&1 )
	    vert = v2 ;
	else if ( !i )
	    vert = v0 ;
	else
	{
	    FVECTOR scl ;
	    float l ;

	    /* ギザギザ作り */
	    _sceVu0SubVector( &v, v1, v0 ) ;
	    _sceVu0ScaleVector( &v, &v, (float)i/BRK_GLS_NVERTS ) ;
	    _sceVu0AddVector( &v, &v, v0 ) ;
	    _sceVu0SubVector( &scl, v2, &v ) ;
	    l = leng * frnd() / sceVu0Sqrt( scl.vx*scl.vx + scl.vy*scl.vy ) ;
	    _sceVu0ScaleVector( &scl, &scl, l ) ;
	    _sceVu0AddVector( &v, &v, &scl ) ;
	    v.vw = 1.0f ;
	    vert = &v ;
	}

	/* ワールド位置に変換 */
	_sceVu0ApplyMatrix( f_gls_p, &work->world, vert ) ;

	/* UVを調整する */
	pos_x = vert->vx - work->def->models->lx ;
	pos_y = vert->vy - work->def->models->ly ;
	f_gls_u->u = FTOI12( pos_x / size_x * t->u_scale + t->u_offset ) ;
	f_gls_u->v = FTOI12( pos_y / size_y * t->v_scale + t->v_offset ) ;
	b_gls_u->u = f_gls_u->u ;
	b_gls_u->v = f_gls_u->v ;
	f_gls_u->f = (!i || i&1 ? 0x8fff :
		      face<0.0f ? 0x0000 : /*左回り*/
		                  0x0020) ;/*右回り*/
	b_gls_u->f = (!i || i&1 ? 0x8fff :
		      face<0.0f ? 0x0020 : /*右回り*/ 
		                  0x0000) ;/*左回り*/

	/* 厚みをつけて最終的な位置に修正 */
	_sceVu0ScaleVector( &v, &work->world.m[Z], work->thickness ) ;
	_sceVu0SubVector( b_gls_p, f_gls_p, &v ) ;
	_sceVu0AddVector( f_gls_p, f_gls_p, &v ) ;
	f_gls_p->vw = 1.0f ;
	b_gls_p->vw = 1.0f ;

	/* 厚み部分のポリゴン */
	if ( !(i&1) )
	{
	    _sceVu0CopyVector( thick++, b_gls_p ) ;
	    _sceVu0CopyVector( thick++, f_gls_p ) ;
	}

	f_gls_p++ ;
	b_gls_p++ ;
	f_gls_u++ ;
	b_gls_u++ ;
    }

    /* 表示開始する(αを120に設定する) */
    SetAlpha( BRK_GLS_NVERTS+1, p->thick_u, 120 ) ;

    p->flag = 1 ;
}

void BRK_GLS_BreakPiece( Work *work, POLY *p, POLY *q, int flag )
{
    switch( flag )
    {
    case 1:/* 1辺がガタガタになる */
	BRK_GLS_MakeStrip( work, p, q ) ;

    default:
	p->flag = flag ;
	break ;

    case 2:/* 壊れきる */
	SetAlpha( BRK_GLS_NVERTS  , p->f_gls_u, 0 ) ;
	SetAlpha( BRK_GLS_NVERTS  , p->b_gls_u, 0 ) ;
	SetAlpha( BRK_GLS_NVERTS+1, p->thick_u, 0 ) ;
	SetZeroPoint( BRK_GLS_NVERTS  , p->f_gls_p ) ;
	SetZeroPoint( BRK_GLS_NVERTS  , p->b_gls_p ) ;
	SetZeroPoint( BRK_GLS_NVERTS+1, p->thick_p ) ;

	/* 反応をなくすためのフラグ */
	p->flag = 3 ;

	if ( work->offense )
	    BreakPiece( work, p, &work->target.hit,
			&work->offense->power->force, SD_A_GLSFALL1, 30 ) ;
#if 0
	BRK_GLS_StartActPieces( work,
				&work->target.hit,
				&work->offense->power->force,
				40 ) ;
#endif

	if ( p->p0 )
	    BRK_GLS_BreakPiece( work, p->p0, p, p->p0->flag+1 ) ;
	if ( p->p1 )
	    BRK_GLS_BreakPiece( work, p->p1, p, p->p1->flag+1 ) ;
	if ( p->p2 )
	    BRK_GLS_BreakPiece( work, p->p2, p, p->p2->flag+1 ) ;
    }
}

POLY *BRK_GLS_TargetHit( Work *work, TARGET *off, TARGET *def )
{
    int     i ;
    POLY   *p ;
    FMATRIX inv ;
    FVECTOR frc, offs ;

    /* ローカルの当たった位置と方向を計算する */
    _sceVu0CopyVector( &frc , &off->power->force ) ;
    _sceVu0CopyVector( &offs, &def->hit ) ;
    offs.vw = 1.0f ;
    frc.vw  = 0.0f ;
    _sceVu0InversMatrix( &inv, &work->world ) ;
    _sceVu0ApplyMatrix( &frc , &inv, &frc  ) ;
    _sceVu0ApplyMatrix( &offs, &inv, &offs ) ;
    _sceVu0SubVector( &offs,  &offs, &def->offset ) ;

    /*弾のポリゴン当たり判定*/
    for ( p=work->poly, i=0 ; i<work->n_poly ; i++,p++ ) {
		if ( p->flag < 3 )
		  if ( BRK_GLS_CheckLineInPolygon( &offs, &frc, p ) ) {
			  return p ;
		  }
	}
    return NULL ;
}

int BRK_GLS_BreakAtRandom( Work *work, int se_id )
{
    int     flag ;
    int     i ;
    POLY   *p ;

    flag = 0 ;

    /* ランダムに適当に壊す */
    for ( p=work->poly, i=0 ; i<work->n_poly ; i++,p++ )
	if ( p->flag < 3 )
	    if ( !(irnd() & 0x110) )
	    {
		flag  =1 ;
		BRK_GLS_BreakPiece( work, p, NULL, 2 ) ;
		//if ( !(irnd() & 0x111) )
		    BreakPiece( work, p, &work->target.hit, 
				(FVECTOR *)work->world.m[Z], se_id, 15 ) ;
	    }
    BRK_GLS_SetToBreakPrim( work ) ;

    return flag ;
}

void BRK_GLS_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    int     i ;
    POLY   *p ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    work->offense = off ;

	    if ( (p = BRK_GLS_TargetHit( work, off, def )) )
	    {
		BRK_GLS_SetToBreakPrim( work ) ;
		BRK_GLS_BreakPiece( work, p, NULL, 2 ) ;
		CallOffenceWhenThrough( work, off, def ) ;
		GM_SeSetMode( SD_A_GLASS01, &def->hit,
			      GM_SEMODE_BOMB ) ;
	    }
	}

	else if ( off->weapon_type & WP_BLAST )
	{
	    FVECTOR *force ;

	    if ( (force = BRK_UTL_BlowBlastCheck( (FVECTOR *)work->world.m[W], off )) )
		if ( BRK_GLS_BreakAtRandom( work, SD_A_GLSFALL1 ) )
		{
		    /* 振動 */
		    BRK_UTL_PK_Vibrate( &def->hit, off ) ;
		    GM_SeSetMode( SD_A_GLASS01, (FVECTOR *)work->world.m[W],
				  GM_SEMODE_BOMB ) ;
		    CallOffenceWhenThrough( work, off, def ) ;
		}
	}

	else if ( off->weapon_type & WP_WEAPONCORE ) {
	    FVECTOR offs ;
	    FMATRIX inv ;

	    if ( off->name == WP_Claymore ) {
			return ;
		}

	    if ( off->name == WP_ThrowG ) {
			_sceVu0CopyVector( &off->size, (FVECTOR*)work->world.m[Z] ) ;
			CallOffenceWhenThrough( work, off, def ) ;
			return ;
		}

	    _sceVu0CopyVector( &offs, &def->hit ) ;
	    offs.vw = 1.0f ;
	    _sceVu0InversMatrix( &inv, &work->world ) ;
	    _sceVu0ApplyMatrix( &offs, &inv, &offs ) ;
	    _sceVu0SubVector( &offs,  &offs, &def->offset ) ;

	    /* ランダムに適当に壊す */
	    for ( p=work->poly, i=0 ; i<work->n_poly ; i++,p++ ) {
			if ( p->flag < 3 ) {
				if ( BRK_GLS_CheckNearPolygon( &offs, p ) ) {
					BRK_GLS_SetToBreakPrim( work ) ;
					CallOffenceWhenThrough( work, off, def ) ;
					GM_SeSetMode( SD_A_GLASS01, &def->hit,
								  GM_SEMODE_BOMB ) ;
				}
			}
		}
		
		work->offense = NULL ;
    }
}
}

