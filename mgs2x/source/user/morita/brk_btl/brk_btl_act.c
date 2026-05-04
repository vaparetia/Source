//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_act.c
   瓶壊れ アクト

   1999/12/02 T. Morita
   $Id: brk_btl_act.c,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
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
static FVECTOR GravityVector = { 0,-BRK_GRAVITY,0,0 } ;
static FVECTOR BounceVector  = { -BRK_BOUNCE, -BRK_BOUNCE, -BRK_BOUNCE, 0.0f } ;
//static FVECTOR NoYBounceVector  = { -BRK_BOUNCE, 0, -BRK_BOUNCE, 0.0f } ;

/******************************************************************

  瓶のガラス破片の動きを処理する関数群

 ******************************************************************/
/*
  瓶のガラス破片の動きを処理する関数
*/
void BRK_BTL_ActPieces( Work *work )
{
    int i, flag=0 ;
    PIECE *p ;
    FVECTOR v ;
    static FVECTOR Size    = { 10.0f, 10.0f, 10.0f } ;
    static FVECTOR Bounce  = { 1.48f, 1.60f, 1.48f, 0.0f } ;

    for ( i=BRK_N_PIECE, p=work->piece_prof ; --i>=0 ;  p++ )
    {
	if ( p->flag )
	{
	    RotateMatrixXY( &p->cmdl_p->world, &DG_UnitMatrix, 
			    p->rot_x += p->rot_vx,
			    p->rot_y += p->rot_vy ) ;
	    _sceVu0AddVector( &p->pos_v, &p->pos_v, &GravityVector ) ;
	    switch ( BRK_CheckHazard( work->hzd_box,
				      &p->pos, &p->pos_v, &Bounce, &Size ) )
	    {
	    case 1:
	    case 3:
		_sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
		_sceVu0ScaleVector( &v, &p->pos_v, 0.125f ) ;
		if ( !(int)v.vx && !(int)v.vy && !(int)v.vz )
		    p->rot_x = 0, p->flag = 0 ;
		break ;
	    case 0:
		_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	    }
	    TransMatrix( &p->cmdl_p->world, &p->pos ) ;
	}
	flag |= p->flag ;

#if MAKING
	AN_Test_Eye3( &p->pos ) ;
#endif

    }
    if ( !flag )
	work->n_piece &= ~BRK_PIECE_ACTIVE ;
}

/*
  瓶のガラス破片の動きを開始させる関数
*/
void BRK_BTL_StartActPieces( Work *work, FVECTOR *pos, float size, FVECTOR *frc, int i, int col )
{
    PIECE *p ;
    int    r, g, b ;

    r = (col>>16) & 0xff ;
    g = (col>> 8) & 0xff ;
    b = (col>> 0) & 0xff ;
    work->n_piece &= ~BRK_PIECE_ACTIVE ;
    while( --i>=0 )
    {
	if ( work->n_piece >= BRK_N_PIECE )
	    work->n_piece = 0 ;
	p = &work->piece_prof[work->n_piece++] ;
	p->cmdl_p->color.vx = r ; /* 破片の色を設定する */
	p->cmdl_p->color.vy = g ;
	p->cmdl_p->color.vz = b ;
	p->cmdl_p->color.vw = 128 ;
	p->rot_x  = 0  ;
	p->rot_y  = irnd()&2047 ;
	p->rot_vx = 512 ;
	p->rot_vy = 0  ;
	p->pos.vx = pos->vx + (p->pos_v.vx = size*frnd()) ;
	p->pos.vy = pos->vy + (p->pos_v.vy = size*frnd()) ;
	p->pos.vz = pos->vz + (p->pos_v.vz = size*frnd()) ;
	if ( frc )
	    p->pos_v.vx += frc->vx*0.005f, p->pos_v.vz += frc->vz*0.005f ;
	p->flag = 6 ;
	//p->flag = 0 ;//debugcode
    }
    work->n_piece |= BRK_PIECE_ACTIVE ;
}




/******************************************************************

  瓶の基本的な動きを支援する関数群

 ******************************************************************/
/*
  破片と 瓶または,その破片 が当たっているかどうかを調べる関数
 */
static int CheckHitToOthers( Work *work, BOTTLE *b, PART *p, float p_rate )
{
    int     i, j, flag = 0 ;
    BOTTLE *btl  ;
    PART   *pp, *pr ;
    FVECTOR pos  ;
    int BRK_BTL_ActSlide( Work *work, BOTTLE *b ) ;
    void BRK_BTL_ActPartOnShelf( Work *work, PART *p ) ;
    FVECTOR v ;

    if ( p )
	_sceVu0ApplyMatrix( &pos, &p->obj->world, &p->target->offset ),
	    _sceVu0AddVector( &pos, &pos, &p->target->center ), b = p->bottle ;
    else
	_sceVu0CopyVector( &pos, &b->pos ) ;

    for ( i=work->n_bottles, btl=work->bottles ; --i>=0 ; btl++ )
	if ( b == btl ) /* 自分の瓶の壊れパーツの中で */
	{
	    if ( p )
		;
#if 0
		for( pp=btl->parts, j=btl->objs->n_models ; --j>=0 ; pp++ )
		{
		    _sceVu0ApplyMatrix( &v, &pp->target->world, &pp->target->offset ) ;
		    _sceVu0AddVector( &v, &v, &pp->target->center ) ;
		    _sceVu0SubVector( &v, &v, &pos     ) ;
		    if ( v.vx*v.vx + v.vz*v.vz <= 80.0f*80.0f )
		    {
			v.vy = 0 ;
			_sceVu0ScaleVector( &pp->pos_v, &v, -p_rate ) ;
			pp->act = BRK_BTL_ActPartOnShelf ;
			flag  = 1 ;
			(p ? p->pos_v : b->pos_v) = DG_ZeroVector ;
		    }
		}
#endif
	}

	else if ( btl->objs ) /* 自分以外の存在している瓶の中で */
	{
	    /* 割れている場合 */
	    if ( (j=btl->objs->n_models) > 1 )
	    {
		for( pp=btl->parts, pr=NULL ; --j>=0 ; pp++ )
		{
		    if ( !pr )
			pr = pp ;
		    if ( pp->obj->flag & DG_FLAG_INVISIBLE )
			pr = NULL ;
		    else
		    {
			if ( pp != pr )
			    pp->act = BRK_BTL_ActPartCopyParent ;
			_sceVu0ApplyMatrix( &v, &pp->target->world, &pp->target->offset ) ;
			_sceVu0AddVector( &v, &v, &pp->target->center ) ;
			_sceVu0SubVector( &v, &pos, &v ) ;
			if ( v.vx*v.vx + v.vz*v.vz <= 80.0f*80.0f )
			{
			    pr->pos_v.vx = v.vx * -p_rate + frnd()*10.0f ;
			    pr->pos_v.vz = v.vz * -p_rate + frnd()*10.0f ;
			    pr->act = pr->act ? pr->act :
				 !(pr->rot.vx/8) && !(pr->rot.vz/8) ?
				BRK_BTL_ActPartOnShelf : BRK_BTL_ActPartFalling ;
			    flag  = 1 ;
			    //(p ? p->pos_v : b->pos_v) = DG_ZeroVector ;
#if MAKING
			    AN_Test_Eye2( &pp->pos, 2 ) ;
#endif
			}
		    }
		}
	    }

	    /* 割れていない場合 */
	    else
	    {
		_sceVu0SubVector( &v, &pos, &btl->pos ) ;
		if ( v.vx*v.vx + v.vz*v.vz <= 80.0f*80.0f )
		{
		    btl->pos_v.vx = v.vx * -p_rate + frnd()*10.0f ;
		    btl->pos_v.vz = v.vz * -p_rate + frnd()*10.0f ;
		    btl->act = btl->act ? btl->act : BRK_BTL_ActSlide ;
		    flag  = 1 ;
		    //(p ? p->pos_v : b->pos_v) = DG_ZeroVector ;
#if MAKING
		    AN_Test_Eye2( &btl->pos, 2 ) ;
#endif
		}
	    }
	}

    return flag ;
}

/*
  DG_OBJの現状態での最低位置の頂点を計算する
*/
void BRK_BTL_FindLowestPoint( DG_OBJ *obj, FVECTOR *min )
{
    DG_MDLPACK *o = obj->model->packs ;
    FVECTOR     f ;
#ifdef PSX2
    int         i, j ;
    SVECTOR    *s ;
#else
    int         j ;
	DG_VERTEX_KMSS *kmss = obj->vbuff ;
#endif

    min->vy = 60000000.0f ;
#ifdef PSX2
    for ( i=obj->model->n_packs ; --i>=0 ; o++ ) {
		for ( j=o->n_verts, s=(SVECTOR*)o->verts ; --j>=0 ; s++ )	{
			vu0_SV0toFV( s, &f ) ;
			f.vw =1.0f ;
			_sceVu0ApplyMatrix( &f, &obj->world, &f ) ;
			if ( f.vy < min->vy )
			  *min = f ;
		}
	}
#else
	for ( j=obj->model->n_verts ; --j>=0 ; kmss++ ) {
		f.vx = kmss->vx ;
		f.vy = kmss->vy ;
		f.vz = kmss->vz ;
		f.vw = 1.0f ;
		_sceVu0ApplyMatrix( &f, &obj->world, &f ) ;
		if ( f.vy < min->vy )
		  *min = f ;
	}
#endif
    min->vx = min->vz = 0.0f ;
    min->vy *= -1 ;
}

void BRK_BTL_MovePart( PART *p, int where )
{
    FVECTOR v, l ;
#define NO_BOUND

    /* OBJの world の更新 */
    p->obj->world = DG_UnitMatrix ;
#ifdef NO_BOUND
    BRK_BTL_FindLowestPoint( p->obj, &v ) ;                   /* 一番低い点を見つける。         */
    TransMatrix( &p->obj->world, &v ) ;
#else
    p->obj->world.m[W][Y] = -p->obj->model->ly ;              /* モデルの一番低い点を           */
#endif
    RotateMatrix( &p->obj->world, &p->obj->world, &p->rot ) ; /* 中心にして回転させ             */
    BRK_BTL_FindLowestPoint( p->obj, &l ) ;                   /* 一番低い点を見つける。         */
    _sceVu0AddVector( &v, &l, &p->pos ) ;                     /* 位置に動かす(pos=モデルの中心) */
    TransMatrix( &p->obj->world, &v ) ;

    GM_MoveTarget2Map( p->target, &p->obj->world, where ) ;
}


static void BRK_BTL_MoveBottle( BOTTLE *b, int where )
{
    int i ;
    TARGET *t = b->target ;

    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->rot ) ;
    TransMatrix( &b->objs->world, &b->pos ) ;

    for ( i=b->brk_af->n_models+1 ; --i>=0 ; t++ )
	GM_MoveTarget2Map( t, &b->objs->world, where ) ;
}



/******************************************************************

  瓶の動き制御の関数群

  瓶の動きは,単一モデルである。この時は,パーツのターゲットも稼働状態に
あるがそれは,どの部分に当たったのかを容易にするためのものである。静止状態は,
act が NULL のときか BRK_BTL_ActFinished になっている時である。
        NULL                壊れていない状態での静止状態
	BRK_BTL_ActFinished 壊れている状態での静止状態

 ******************************************************************/
int BRK_BTL_ActFinished( Work *work, BOTTLE *b )
{
    return 0 ;
}

int BRK_BTL_ActBreaking( Work *work, BOTTLE *b )
{
    int     i, flag = 0 ;
    PART   *p = b->parts ;

    /* breaking the object */
    for ( i=b->brk_af->n_models ; --i>=0 ; flag|=(int)p->act, p++ )
	if ( p->act )
	    (*p->act)( work, p ) ;
    if ( !flag )
	b->act = BRK_BTL_ActFinished ;

    return 1 ;
}

int BRK_BTL_ActBroken( Work *work, BOTTLE *b )
{
    GM_SeSetMode( SD_A_BOTTLE02, &b->pos, GM_SEMODE_NORMAL ) ;

    BRK_BTL_FreeBottle( b ) ;
    BRK_BTL_StartActPieces( work, &b->pos, 20.0f, NULL, 10, b->flag ) ;
    b->act = BRK_BTL_ActFinished ;

    return 0 ;
}

int BRK_BTL_ActFalling( Work *work, BOTTLE *b )
{
    AddSVector( &b->rot, &b->rot, &b->rot_v ) ; 
    b->pos_v.vx *= BRK_PVEL_R ;
    b->pos_v.vz *= BRK_PVEL_R ;
    _sceVu0AddVector( &b->pos_v, &b->pos_v, &GravityVector ) ;
    if ( BRK_BTL_HzdCheck( work, &b->pos, &b->pos_v, &BounceVector, BRK_HZX_SPHERE, b->target+1 ) )
	b->act = BRK_BTL_ActBroken ;
    _sceVu0AddVector( &b->pos, &b->pos, &b->pos_v ) ;
    BRK_BTL_MoveBottle( b, work->where ) ;

    return 1 ;
}

int BRK_BTL_ActTilting( Work *work, BOTTLE *b )
{
    b->rot_v.vx += b->rot_v.vx > 0 ? 3 : b->rot_v.vx < 0 ? -3 : 0 ;
    b->rot_v.vz += b->rot_v.vz > 0 ? 3 : b->rot_v.vz < 0 ? -3 : 0 ;
    AddSVector( &b->rot, &b->rot, &b->rot_v ) ; 
    _sceVu0AddVector( &b->pos, &b->pos, &b->pos_v ) ;
    if ( (b->rot_v.vx >= 0 ? (b->rot.vx&1023) > 512 : (b->rot.vx&1023) < 512) ||
	 (b->rot_v.vz >= 0 ? (b->rot.vz&1023) > 512 : (b->rot.vz&1023) < 512) )
	b->act = BRK_BTL_ActFalling ;
    BRK_BTL_MoveBottle( b, work->where ) ;

    return 1 ;
}

int BRK_BTL_ActSlide( Work *work, BOTTLE *b )
{
    b->pos_v.vy = -BRK_GRAVITY ;
    if ( !BRK_BTL_HzdCheck( work, &b->pos, &b->pos_v, &BounceVector, BRK_HZX_SPHERE, b->target+1 ) )
    {
	b->pos_v.vy = 0.0f ;
	b->rot_v.vx = 12 ;
	b->rot.vx = 0 ;
	b->rot.vz = 0 ;
	b->act = BRK_BTL_ActTilting ;
    }
    else
    {
	b->pos.vx += b->pos_v.vx *= BRK_VEL_R ;
	b->pos.vz += b->pos_v.vz *= BRK_VEL_R ;
	if ( !(int)b->pos_v.vx && !(int)b->pos_v.vz )
	    b->act = NULL ;
	BRK_BTL_MoveBottle( b, work->where ) ;
    }
    CheckHitToOthers( work, b, NULL, 0.1f ) ;
    return 1 ;
}





/******************************************************************

  瓶のパーツの動き制御の関数群

  制御の元になるのは,位置的に一番下のパーツになる。それにくっついてい
るパーツは,単に親の位置をコピーし,表示するだけである。他の瓶と当たって
いるかどうかは親が独自で判別するしくみになっている。

  静止状態は,act が NULL のとき

 ******************************************************************/
/*
  親の動きをコピーする関数
 */
void BRK_BTL_ActPartCopyParent( Work *work, PART *p )
{
    if ( (p-1)->obj->flag & DG_FLAG_INVISIBLE )
	p->target->class |= TARGET_SKIP, p->obj->flag |= DG_FLAG_INVISIBLE ;
    if ( (p-1)->act == NULL )
	p->act = NULL ;
    p->rot  	  = (p-1)->rot        ;
    p->rot_v	  = (p-1)->rot_v      ;
    p->pos  	  = (p-1)->pos        ;
    p->pos_v	  = (p-1)->pos_v      ;
    p->obj->world = (p-1)->obj->world ;
    GM_MoveTarget2( p->target, &p->obj->world ) ;
}

/*
  棚の上で傾く動きの関数
 */
void BRK_BTL_ActPartTiltingOnShelf( Work *work, PART *p )
{
    PART   *q ;
    FVECTOR v, l ;

    /* 回転および移動の更新 */
    p->rot_v.vx += p->rot.vx > 0 ? 3 : p->rot.vx < 0 ? -3 : 0 ;
    p->rot_v.vz += p->rot.vz > 0 ? 3 : p->rot.vz < 0 ? -3 : 0 ;
    if ( p->rot.vx )
    {
	if ( (p->rot.vx += p->rot_v.vx) == 0 )
	    p->rot.vx++ ;
	if ( p->rot.vx>=0 ? (p->rot.vx&2047) > 1024 : (p->rot.vx&2047) < 1024 )
	    p->act = NULL, p->rot.vx = p->rot.vx>0 ? 1024 : -1024 ;
    }
    if ( p->rot.vz )
    {
	if ( (p->rot.vz += p->rot_v.vz) == 0 )
	    p->rot.vz++ ;
	if ( p->rot.vz>=0 ? (p->rot.vz&2047) > 1024 : (p->rot.vz&2047) < 1024 )
	    p->act = NULL, p->rot.vz = p->rot.vz>0 ? 1024 : -1024 ;
    }
    p->rot.vy += p->rot_v.vy ;

    /* OBJの world の更新 */
    p->obj->world = DG_UnitMatrix ;
#ifdef NO_BOUND
    BRK_BTL_FindLowestPoint( p->obj, &v ) ;                   /* 一番低い点を見つける。         */
    TransMatrix( &p->obj->world, &v ) ;
#else
    p->obj->world.m[W][Y] = -p->obj->model->ly ;              /* モデルの一番低い点を           */
#endif
    RotateMatrix( &p->obj->world, &p->obj->world, &p->rot ) ; /* 中心にして回転させ             */
    BRK_BTL_FindLowestPoint( p->obj, &l ) ;                   /* 一番低い点を見つける。         */

    CheckHitToOthers( work, NULL, p, 0.2f ) ;
    for ( q=p+1 ; q->act==BRK_BTL_ActPartCopyParent ; q++ )
    {
	if ( p->act )
	    if ( BRK_CheckTargetHazard( work->hzd_box, &p->obj->world, &p->pos_v,
					&BounceVector, q->target ) &2  )
	    {
		p->act = NULL ;
		printf( "TiltingOn POS_000000V(%.2f %.2f %.2f) kita%x\n", p->pos_v.vx, p->pos_v.vy, p->pos_v.vz, p ) ;
#if 0
		p->pos_v.vy = 0.0f ;
		p->pos.vx += -p->pos_v.vx*5.0f ;
		p->pos.vz += -p->pos_v.vz*5.0f ;
#endif
	    }
	CheckHitToOthers( work, NULL, q, 0.2f ) ;
    }
    TransMatrix( &p->obj->world, &l ) ;
    TransMatrix( &p->obj->world, &p->pos ) ;
    GM_MoveTarget2( p->target, &p->obj->world ) ;

    //printf( "TiltingOn (%.2f %.2f %.2f) %x\n", p->pos.vx, p->pos.vy, p->pos.vz, p ) ;
}

/*
  落ちていく（空中に浮いている）時の制御の関数
 */
void BRK_BTL_ActPartFalling( Work *work, PART *p )
{
    /* 回転および移動の更新 */
    AddSVector( &p->rot, &p->rot, &p->rot_v ) ;
    p->pos_v.vx *= BRK_PVEL_R  ;
    p->pos_v.vy -= BRK_GRAVITY ;
    p->pos_v.vz *= BRK_PVEL_R  ;

//printf( "Falling pos(%.2f %.2f %.2f)->", p->pos.vx, p->pos.vy, p->pos.vz ) ;
    BRK_BTL_MovePart( p, work->where ) ;

    if ( BRK_BTL_HzdCheck( work, &p->pos, &p->pos_v, &BounceVector, BRK_HZX_SPHERE, p->target ) & 0x4 )
    {
	if ( p->pos_v.vy >= 0.0f )
	{
	    if ( p->pos_v.vy > 25.0f )/*速度が速いので割れる*/
	    {
		p->target->class |= TARGET_SKIP ;   /* 壊れたターゲットには当たらない */
		p->obj->flag |= DG_FLAG_INVISIBLE ; /* 壊れたパーツを消す */
		BRK_BTL_StartActPieces( work, &p->pos, 20.0f, NULL, 10, p->bottle->flag ) ;
		GM_SeSetMode( SD_A_BOTTLE02, &p->pos, GM_SEMODE_NORMAL ) ;
		p->act = NULL ;
	    }
	    else                      /*速度が遅いので割れずに倒れる*/
	    {
		if ( !p->rot.vz && !p->rot.vx )
		{
		    p->rot_v.vy = (irnd()&31)-16 ;  /* Y軸に回転することで斜めっぽくなる */
		    if ( irnd()&0x0100 )
			p->rot.vz = 0, p->rot.vx = p->rot_v.vy ? p->rot_v.vy : 1 ;/* X軸に倒す */
		    else
			p->rot.vx = 0, p->rot.vz = p->rot_v.vy ? p->rot_v.vy : 1 ;/* Z軸に倒す */
		}
		p->act = BRK_BTL_ActPartTiltingOnShelf ;
	    }
	    p->pos_v.vy = 0.0f ;
	}
    }
    else
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    //printf( "\tFalling pos(%.2f %.2f %.2f) %x\n", p->pos.vx, p->pos.vy, p->pos.vz, p ) ;

    BRK_BTL_MovePart( p, work->where ) ;
}

/*
  棚の上で傾く動きの関数
 */
void BRK_BTL_ActPartTilting( Work *work, PART *p )
{
    /* 回転および移動の更新 */
    p->rot_v.vx += p->rot_v.vx > 0 ? 3 : p->rot_v.vx < 0 ? -3 : 0 ;
    p->rot_v.vz += p->rot_v.vz > 0 ? 3 : p->rot_v.vz < 0 ? -3 : 0 ;
    AddSVector( &p->rot, &p->rot, &p->rot_v ) ; 
    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    if ( (p->rot_v.vx >= 0 ? (p->rot.vx&1023) > 256 : (p->rot.vx&1023) < 256) ||
	 (p->rot_v.vz >= 0 ? (p->rot.vz&1023) > 256 : (p->rot.vz&1023) < 256) )
	p->act = BRK_BTL_ActPartFalling ;

    BRK_BTL_MovePart( p, work->where ) ;
}

/*
  棚の上で揺れたり,Y軸回転する制御の関数
 */
void BRK_BTL_ActPartOnShelf( Work *work, PART *p )
{
    p->rot.vx += p->rot_v.vx = p->rot_v.vx - (p->rot_v.vx + p->rot.vx)/8 ;
    p->rot.vz += p->rot_v.vz = p->rot_v.vz - (p->rot_v.vz + p->rot.vz)/8 ;
    p->rot.vy += p->rot_v.vy = p->rot_v.vy * (BRK_ROT_R-1) / BRK_ROT_R ;

    if ( !BRK_BTL_HzdCheck( work, &p->pos, &p->pos_v, &BounceVector, BRK_HZX_SPHERE, p->target ) )
	p->rot_v.vx = 12, p->rot.vx = p->rot.vz = 0, p->act = BRK_BTL_ActPartTilting ;
    _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_PVEL_R ) ;
    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;

    if ( !(p->rot.vz/25||p->rot_v.vz/5) && !(p->rot.vz/25||p->rot_v.vz/5) &&
	 !(int)p->pos_v.vx && !(int)p->pos_v.vy && !(int)p->pos_v.vz )
	p->rot.vx = p->rot.vz = 0, p->act = NULL ;

    /* 他のボトルに当たっている？ */
    CheckHitToOthers( work, NULL, p, 0.2f ) ;

    BRK_BTL_MovePart( p, work->where ) ;
}
