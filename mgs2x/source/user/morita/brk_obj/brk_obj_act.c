//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_obj_act.c
   オブジェクト壊れ アクト

   1999/12/13 T. Morita
   $Id: brk_obj_act.c,v 1.1.1.3 2002/11/19 11:45:37 Yoshizawa1 Exp $
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
#include "../brk_utl/brk_utl.x"


static FVECTOR BRK_OBJ_Bounce    = { BRK_BOUNCE, BRK_BOUNCE, BRK_BOUNCE, 0 } ;
static FVECTOR BRK_OBJ_BouncePce = { BRK_BOUNCE+0.3f, BRK_BOUNCE+0.3f, BRK_BOUNCE+0.3f, 0 } ;
static FVECTOR BRK_OBJ_PceSize = { 4.0f, 4.0f, 4.0f, 0 } ;



/*

  SE用関数群

*/
void  BRK_OBJ_ActSound( Work *work )
{
    if ( BRK_OBJ_NoDropSound )
	BRK_OBJ_NoDropSound-- ;
}







/*

  各アクト関数群で利用されるサブルーチン

*/
/* モデルのサイズ 中心点を求める */
float BRK_OBJ_StickPartsOnFloor( DG_OBJ *p, FVECTOR *size, FVECTOR *cen )
{
    int         j ;
    DG_MDLPACK *o = p->model->packs ;
    FVECTOR     f, min, max ;
#ifdef PSX2
    int         i ;
    SVECTOR    *s ;
#else
	DG_VERTEX_KMSS *kmss = p->model->vbuff ;
#endif
	
    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
#ifdef PSX2
    for ( i=p->model->n_packs ; --i>=0 ; o++ ) {
		for ( j=o->n_verts, s=(SVECTOR*)o->verts ; --j>=0 ; s++ ) {
			SVector2FVector( &f, s ) ;
			f.vw = 0.0f ;
			_sceVu0ApplyMatrix( &f, &p->world, &f ) ;
			MinMaxVector( &min, &max, &f ) ;
		}
	}
#else
	for ( j=p->model->n_verts ; --j>=0 ; kmss++ ) {
		f.vx = kmss->vx ;
		f.vy = kmss->vy ;
		f.vz = kmss->vz ;
		f.vw = 0.0f ;
		_sceVu0ApplyMatrix( &f, &p->world, &f ) ;
		MinMaxVector( &min, &max, &f ) ;
	}
#endif
	_sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
    _sceVu0AddVector( cen, &max, &min ) ;
    _sceVu0ScaleVector( cen, cen, 0.5f ) ;

    return -min.vy ;
}

/* モデルの安定角度にする */
static int BRK_OBJ_Stabilize( short *rot, short *rot_v, int div, int mask )
{
    int i, r, n ;
    int flag ;

    *rot &= 4095 ;
    for ( n=0 ; *rot>n+div/2 ; n+=div ) ;
    n |= mask ;
    r = n - *rot ;
    for ( i=3 ; r/2 && --i>=0 ; r/=2 ) ;
    r = *rot + (*rot_v = r) ;
#if 0
printf( "Stabilize rot%d rotv%d n%d r%d div%d det%d\n", *rot, *rot_v,  n,r, div,
(r >= n && *rot <= n) || (r <= n && *rot >= n)
 ) ;
#endif
    if ( (r >= n && *rot <= n) || (r <= n && *rot >= n) )
        *rot = n, *rot_v = 0, flag = 0 ;
    else
	*rot = r, flag = 1 ;
    return flag ;
}

/*

  破片用関数群

*/
/* 破片の移動開始 */
void BRK_OBJ_StartActPieces( Work *work, int i,
			     FVECTOR *pos, FVECTOR *pos_v,
			     float width, float pop )
{
    PIECE  *p ;
    FVECTOR r ;

    /* アクトを再開 */
    work->n_piece &= ~BRK_OBJ_INACTIVE ;

    /* それぞれの初期値を決定 */
    while( --i >= 0 )
    {
	p = &work->piece[work->n_piece] ;
	r.vx = width*frnd() ;
	r.vy = width* rnd() + pop ;/* 上方向だけにするため 0<rnd()<1を使う */
	r.vz = width*frnd() ;
	p->flag = 1 ;
	p->rot_x = irnd()&4095 ;
	p->rot_y = irnd()&4095 ;
	_sceVu0AddVector( &p->pos  , pos  , &r ) ;
	_sceVu0AddVector( &p->pos_v, pos_v, &r ) ;

	RotateMatrixXY( &p->comdl->world, &DG_UnitMatrix, p->rot_x,p->rot_y ) ;
	TransMatrix( &p->comdl->world, &p->pos ) ;

	p->comdl->color.vw = 128 ;

	if ( ++work->n_piece >= BRK_N_PIECE )
	    work->n_piece = 0 ;
    }    
}

/*破片を落ち着かせる*/
void BRK_OBJ_ActPieceFallDown( Work *work, PIECE *p )
{
    if ( !BRK_OBJ_Stabilize( &p->rot_x, &p->rot_vx, 2048, 0 ) )
	p->pos.vy += rnd()*2.0f, p->flag = 0 ;
    p->rot_y += p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -BRK_GRAVITY*2 ;
    RotateMatrixXY( &p->comdl->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if( !BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &BRK_OBJ_PceSize ) )
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ), p->flag = 2 ;
    TransMatrix( &p->comdl->world, &p->pos ) ;
}
/*破片を動かす*/
void BRK_OBJ_ActPieceMove( Work *work, PIECE *p )
{
    RotateMatrixXY( &p->comdl->world, &DG_UnitMatrix,
		    p->rot_x+=p->rot_vx, p->rot_y+=p->rot_vy ) ;
    p->pos_v.vy -= BRK_GRAVITY ;
    switch( BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			     &BRK_OBJ_BouncePce, &BRK_OBJ_PceSize ) )
    {
    case 1:
    case 3:
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;

	if ( p->pos_v.vy < 6.0f*2 && !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	    p->flag = (work->flag & BRK_F_STABLE_MSK ? 2 : 0) ;
	else if ( p->pos_v.vy < 40.0f )
	    p->rot_vx = (irnd() & 64) - 32 ;
	else
	    p->rot_vx = -p->rot_vx*15/16 ;
	break ;
    case 0:
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( &p->comdl->world, &p->pos ) ;
}

/* 破片アクト */
void BRK_OBJ_ActPiece( Work *work, PIECE *p, int i )
{
    int flag  = 0 ;

    for ( ; --i>=0 ; flag|=p->flag, p++ )
    {
	//printf( "BRK_OBJ_ActPieces[%d] %d %.0f %.0f %.0f\n", i, p->flag,p->pos.vx,p->pos.vy,p->pos.vz ) ;

	switch( p->flag )
	{
	case 1:
	    BRK_OBJ_ActPieceMove( work, p ) ;
	    break ;
	case 2:
	    BRK_OBJ_ActPieceFallDown( work, p ) ;
	    break ;
	}
    }

    /* アクトを停止させる */
    if ( !flag )
	work->n_piece |= BRK_OBJ_INACTIVE ;
}



/*

  オブジェクトアクト関数群

*/

static FVECTOR BRK_OBJ_SizeBreak = { 10.0f, 10.0f, 10.0f, 0 } ;

void BRK_OBJ_ActBreakDrop( Work *work, PART *p )
{
    FVECTOR *s, size, cen, ofst ;

    /* 安定角度に集約させる*/
    if ( work->flag & BRK_F_STABLE_MSK )
    {
	if ( work->flag & BRK_F_STABLE_X )
	{
	    BRK_OBJ_Stabilize( &p->rot.vx, &p->rot_v.vx, 1024, 1024 ) ;
	    BRK_OBJ_Stabilize( &p->rot.vz, &p->rot_v.vz, 4096, 0    ) ;
	}
	if ( work->flag & BRK_F_STABLE_Z )
	{
	    BRK_OBJ_Stabilize( &p->rot.vx, &p->rot_v.vx, 4096, 0    ) ;
	    BRK_OBJ_Stabilize( &p->rot.vz, &p->rot_v.vz, 1024, 1024 ) ;
	}
    }
    else
	AddSVector( &p->rot, &p->rot, &p->rot_v ) ;
    RotateMatrix( &p->obj->world, &DG_UnitMatrix, &p->rot ) ;
    p->pos_v.vy -= BRK_GRAVITY ;
    if ( work->flag & BRK_F_STRICT_FLR )
    {
	BRK_OBJ_StickPartsOnFloor( p->obj, s=&size, &cen ) ;
	_sceVu0AddVector( &ofst, &p->pos, &cen ) ;
    }
    else
	s = &BRK_OBJ_SizeBreak ;

    switch( BRK_CheckHazard( work->hzd, &ofst, &p->pos_v, &BRK_OBJ_Bounce, s ) )
    {
    case 1:
    case 3:
	_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
	if ( p->pos_v.vy > 20.0f )
	{
	    p->rot_v.vx = (irnd() & 63) - 32 ;
	    p->rot_v.vz = (irnd() & 63) - 32 ;
	}
	else if ( p->pos_v.vy > 6.0f)
	{
	    if ( p->se_flag && !BRK_OBJ_NoDropSound )
	    {
		p->se_flag = 0 ;
		BRK_OBJ_NoDropSound = 8*BRK_OBJ_HowManyActs ;
		if ( work->flag & BRK_F_RND_DRP_SE )
		    GM_SeSetMode( (irnd()&3)+work->drp_se_id, &p->pos, GM_SEMODE_NORMAL ) ;
		else
		    GM_SeSetMode( work->drp_se_id, &p->pos, GM_SEMODE_NORMAL ) ;
	    }
	    ScaleSVector( &p->rot_v, &p->rot_v, 256*(BRK_PROT_R-1) / BRK_PROT_R) ;
	}
	else if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	    p->flag = work->flag & BRK_F_STABLE_MSK ? 2 : 0 ;

	/* 重心が外れているかどうか */
	if ( BRK_UTL_PutCenterHazard( work->hzd, &ofst, &p->pos_v, 1000.0f, 25.0f ) )
	{
	    p->pos_v.vy = 20.0f ;
	    p->rot_v.vx = (irnd() & 63) - 32 ;
	    p->rot_v.vz = (irnd() & 63) - 32 ;
	    p->flag = 1 ;
	}
	break ;

    case 2:
	/*回転を止める*/
	p->rot_v.vx = p->rot_v.vy = p->rot_v.vz = 0 ;
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	break ;

    case 0:
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( &p->obj->world, &p->pos ) ;
}

void BRK_OBJ_ActBreakFallDown( Work *work, PART *p )
{
    FVECTOR *s, size, cen, ofst ;

    if ( work->flag & BRK_F_STABLE_X )
	if ( !BRK_OBJ_Stabilize( &p->rot.vz, &p->rot_v.vz, 4096, 0    ) &&
	     !BRK_OBJ_Stabilize( &p->rot.vx, &p->rot_v.vx, 1024, 1024 ) )
	    p->pos.vy += rnd()*2.0f, p->flag = 0 ;

    if ( work->flag & BRK_F_STABLE_Z )
	if ( !BRK_OBJ_Stabilize( &p->rot.vx, &p->rot_v.vx, 4096, 0    ) &&
	     !BRK_OBJ_Stabilize( &p->rot.vz, &p->rot_v.vz, 1024, 1024 ) )
	    p->pos.vy += rnd()*2.0f, p->flag = 0 ;

    p->rot.vy += p->rot_v.vy = p->rot_v.vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -BRK_GRAVITY*2 ;

    RotateMatrix( &p->obj->world, &DG_UnitMatrix, &p->rot ) ;
    if ( work->flag & BRK_F_STRICT_FLR )
    {
	BRK_OBJ_StickPartsOnFloor( p->obj, s=&size, &cen ) ;
	_sceVu0AddVector( &ofst, &p->pos, &cen ) ;
    }
    else
	s = &BRK_OBJ_SizeBreak ;

    if( BRK_CheckHazard( work->hzd, &ofst, &p->pos_v, &BRK_HZD_NoBounce, s ) )
	_sceVu0SubVector( &p->pos, &ofst, &cen ), p->pos.vy +=1.0f ;
    else
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ), p->flag = 2 ;
    TransMatrix( &p->obj->world, &p->pos ) ;
}

/* 壊れ状態 */
int BRK_OBJ_ActBreak( Work *work, BRK_OBJ *o )
{
    int   i ;
    PART *p = o->parts.prof ;
    int flag = 0 ;

    for ( i=o->n_parts ; --i>=0 ; flag+=p->flag, p++ )
    {
	if ( p->flag == 1 )
	    BRK_OBJ_ActBreakDrop( work, p ) ;
	else if ( p->flag == 2 )
	    BRK_OBJ_ActBreakFallDown( work, p ) ;
    }

    if ( !flag )
    {
	o->act = NULL ;
	if ( o->n_parts > 0 )
	    GV_Free( o->parts.prof ) ;
	o->parts.prof = NULL ;
	o->n_parts    = 0    ;
    }

    return 1 ;
}

/* 移動状態 */
int BRK_OBJ_ActMove( Work *work, BRK_OBJ *o )
{
    FVECTOR *s, size, cen, ofst ;

    if ( !o->objs || work->flag & BRK_F_NO_GRAVITY )
	o->act = BRK_OBJ_ActNone ;
    else
    {
	RotateMatrix( &o->objs->world, &DG_UnitMatrix, &o->rot ) ;
	o->pos_v.vy -= BRK_GRAVITY ;
	if ( work->flag & BRK_F_STRICT_FLR )
	{
	    BRK_OBJ_StickPartsOnFloor( o->objs->objs, s=&size, &cen ) ;
	    _sceVu0AddVector( &ofst, &o->pos, &cen ) ;
	}
	else
	    s = &work->r_size ;
	switch ( BRK_CheckHazard( work->hzd, &o->pos, &o->pos_v, &BRK_OBJ_Bounce, s ) )
	{
	case 1:
	case 3:
	    _sceVu0ScaleVector( &o->pos_v, &o->pos_v, BRK_VEL_R ) ;
	    if ( o->pos_v.vy < BRK_GRAVITY )
	    {
		o->rot.vx = (int)(o->pos.vx/o->target.size.vx) & 4095 ;
		if ( !(int)o->pos_v.vx && !(int)o->pos_v.vz )
		    o->pos.vy +=1.0f, o->act = BRK_OBJ_ActNone ;
		o->pos_v.vy = 0.0f ;
	    }
	    else
		o->rot_v.vx = (short)(-BRK_BOUNCE * o->rot_v.vx) ;
	case 2:
	    _sceVu0SubVector( &o->pos, &ofst, &cen ) ;
	    break ;
	case 0:
	    ScaleSVector( &o->rot_v, &o->rot_v,
			  256*(BRK_ROT_R-1) / BRK_ROT_R) ;
	    AddSVector( &o->rot, &o->rot, &o->rot_v ) ;
	    _sceVu0AddVector( &o->pos, &o->pos, &o->pos_v ) ;
	}
	TransMatrix( &o->objs->world, &o->pos ) ;
	GM_MoveTargetMap( &o->target, &o->pos, work->where ) ;
    }

    return 1 ;
}

/* 足への当たり */
int BRK_OBJ_ActOnFoot( Work *work, BRK_OBJ *o )
{
    _sceVu0SubVector( &o->pos_v, o->parts.foot, &o->pos ) ;
    _sceVu0ScaleVector( &o->pos_v, &o->pos_v, 0.01f ) ;
    o->pos = *o->parts.foot ;

    printf( "kiteru\n" ) ;

    return 1 ;
    return 0 ;
}

/* 静止状態 */
int BRK_OBJ_ActNone( Work *work, BRK_OBJ *o )
{
    return 0 ;
}


/* オブジェクトアクト */
void BRK_OBJ_ActObject( Work *work )
{
    int  i, flag = 0 ;
    BRK_OBJ *p ;

    if ( work->flag & BRK_F_COLLISION )
	BRK_OBJ_HzdObjectCheck( work, BRK_OBJ_RAD*BRK_OBJ_RAD ) ;
    for ( i=work->n_obj, p=work->obj ; --i>=0 ; p++ )
	if ( p->act && p->act != BRK_OBJ_ActNone )
	    flag |= (*p->act)( work, p ) ;

    /* アクトを停止させる */
    if ( !flag )
	work->n_obj |= BRK_OBJ_INACTIVE ;
}
