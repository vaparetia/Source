//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_hzx.c
   じゃがいも壊れ 当たり判定用ルーチン

   1999/12/15 T. Morita
   2000/10/16 1.14 T.Morita 
   $Id: brk_pto_hzd.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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


#if 0
int BRK_PTO_HzdSegLineCheck( HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    float px, pz, sx, sz, tx, tz ;
    float r, s, rd ;

    px = p2->vx    - p1->vx    ; pz = p2->vz    - p1->vz    ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*pz - sz*px) == 0.0f )                                
      return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*px - tx*pz)/rd ;
    if ( r > 1.0f || r < 0.0f )               
      return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
      return  0 ;
    return 1 ;
}

void BRK_PTO_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;
}

 /* n の大きさは, １です。*/
int BRK_PTO_HzdSegReactVector2( FVECTOR *n, FVECTOR *p, FVECTOR *bounce, int i )
{
    float f ;
    FVECTOR v ;

    _sceVu0ScaleVector( &v, n, i ? 1.0f : -1.0f ) ;
    if ( (f = 1.1f * _sceVu0InnerProduct( p, &v )) > 0 )
	return 0 ;
    _sceVu0ScaleVector( &v, &v, f ) ;
    _sceVu0SubVector( p, p, &v ) ;
    return  1 ;
}

/* sphereの符号で内側(+)か外側(-)が決まる */
int HZX_BoxCheck( BOX *b, FVECTOR *pos, FVECTOR *pos_v, FVECTOR *bounce, float sphere )
{
    FVECTOR p, v, c, *s ;
    FMATRIX m ;
    TARGET *t = &b->target  ;
    float   l, size[XYZ] = { t->size.vx - sphere, t->size.vy - sphere, t->size.vz - sphere } ;
    float  *pp = (float *)&p ;
    int     i = 0, flag = 0 ;

    _sceVu0InversMatrix( &m, &t->world ) ;
    ApplyMatrixXYZ( &c, &t->world, &t->offset ) ;
    _sceVu0AddVector( &c, &c, &t->center ) ;    /* ワールド座標にて箱の中心位置を計算する */
    _sceVu0SubVector( &p, pos, &c ) ;
    ApplyMatrixXYZ( &p, &m, &p ) ;              /* pos を箱のローカル座標に変換する       */

    /* 箱の内側？ */
    if ( !(int)(p.vx/size[X]) && !(int)(p.vy/size[Y]) && !(int)(p.vz/size[Z]) )
	for ( i=XYZ ; --i>=0 ; ) /* 各軸に対して平面の交点を調べる（ここからワールド座標で計算） */
	{
	    if ( i == Y && pp[Y] > 0.0f )              /* 箱の上面は空いているから               */
		continue ;
	    s = (FVECTOR *)&t->world.m[i] ;            /* 平面の法線,つまりは,箱の行列の各軸成分 */
	    if ( (l=_sceVu0InnerProduct( pos_v, s )) != 0.0 ) /* 0 だったら平面と直線は平行      */
	    {
		size[i] = pp[i]>0 ? size[i]-3.0f : 3.0f-size[i] ; /* どっち側の平面？ */
		_sceVu0ScaleVector( &v, s, size[i] ) ; /* 平面を求める */
		_sceVu0AddVector( &v, &v, &c ) ;       /* 中心位置からsize分だけ移動した法線sの平面 */
		_sceVu0SubVector( &v, &v, pos ) ;
		l = _sceVu0InnerProduct( &v, s ) / l ; /* これで平面と交点が媒介変数lで表せる    */
		if ( l >= -1.0 && l <= 1.0f )          /* 本来は 0.0 < l < 1.0だがメリ込み防止で */
		{
		    _sceVu0ScaleVector( &v, pos_v, l ) ;
		    if ( BRK_PTO_HzdSegReactVector2( s, pos_v, bounce, size[i]<0 ) )
			flag = s->vy>0.95 ? 1/*床*/ : 2/*壁*/,
			    _sceVu0AddVector( pos, pos, &v ) ;/* posを当たった場所にする */
		}
	    }
	}

    /* 箱の空いている方から出そう？ */
    else if ( (int)(p.vy/size[Y]) && p.vy > 0.0f )
	flag = 0 ;

    /* 既にメリ込んでいる？ */
    else if ( !(int)(p.vx/t->size.vx) && !(int)(p.vy/t->size.vy) && !(int)(p.vz/t->size.vz) )
    {
	if ( (int)(p.vx/size[X]) )
	    BRK_PTO_HzdSegReactVector2( (FVECTOR*)&t->world.m[X], pos_v, bounce, p.vx<0 ) ;
	c.vx = (int)(p.vx/size[X]) ? ( p.vx>0 ? size[X]-1.0f : -size[X]+1.0f ) - p.vx : 0.0f ;
	if ( (int)(p.vy/size[Y]) )
	    BRK_PTO_HzdSegReactVector2( (FVECTOR*)&t->world.m[Y], pos_v, bounce, p.vy<0 ) ;
	c.vy = (int)(p.vy/size[Y]) ? ( p.vy>0 ? size[Y]-1.0f : -size[Y]+1.0f ) - p.vy : 0.0f ;
	if ( (int)(p.vz/size[Z]) )
	    BRK_PTO_HzdSegReactVector2( (FVECTOR*)&t->world.m[Z], pos_v, bounce, p.vz<0 ) ;
	c.vz = (int)(p.vz/size[Z]) ? ( p.vz>0 ? size[Z]-1.0f : -size[Z]+1.0f ) - p.vz : 0.0f ;

#if 0	
	if ( (int)(p.vy/size[Y]) )
		AN_Test_Eye2( pos, 2 ) ;
#endif

	ApplyMatrixXYZ( &c, &t->world, &c ) ;
	_sceVu0AddVector( pos, pos, &c ) ;

	flag = 2 ;
    }

    return flag ;
}

int BRK_PTO_HzdCheck( Work *work, FVECTOR *pos, FVECTOR *pos_v,
		      FVECTOR *bounce, float sphere )
{
    int   atr[2] ;
    float flr[2] ;
    HZX_SEG seg[2] ;
    int flag = 0 ;
    FVECTOR new ;

    if ( !(flag=HZX_BoxCheck( &work->box, pos, pos_v, bounce, sphere )) )/*箱の内側の当たり*/
	{
	    _sceVu0AddVector( &new, pos, pos_v ) ;
	    if ( HZX_NearHazardCheck( work->hzx, pos, BRK_HZX_S_SPHERE,
				      HZX_CHK_ALL, HZX_SEG_ALL, sphere ) )
	    {
		HZX_GetNearHazard( seg, atr ) ;
		if ( BRK_PTO_HzdSegLineCheck( seg, pos, &new ) )
		    BRK_PTO_HzdSegReactVector( seg, pos_v ), flag |= 2 ;
	    }
	    if ( HZX_LevelHazardCheck( work->hzx, pos, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	    {
		HZX_GetLevelHeight( flr ) ;
		if ( new.vy < (flr[0] += sphere) )
		    pos->vy = flr[0], pos_v->vy *= bounce->vy, flag |= 1 ;
	    }
	}
    return flag ;
}

#endif



int BRK_PTO_HzdPotatoCheck( Work *work, float rad )
{
    int     i, j, flag = 0 ;
    float   f, d ;
    FVECTOR v ;
    POTATO *p, *o ;

    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	if ( p->act )
	    for ( j=i, o=p+1 ; --j>=0 ; o++ )
		if ( o->act )
		{
		    SubVector( &v, &p->pos, &o->pos ) ;
		    d = v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ;
		    //f = p->target.size.vx + o->target.size.vx ;
		    if ( d <= /* f*f */ rad )
		    {
			f = ( (o->pos_v.vx - p->pos_v.vx) * v.vx +
			      (o->pos_v.vy - p->pos_v.vy) * v.vy +
			      (o->pos_v.vz - p->pos_v.vz) * v.vz) / d ;
			/* 条件は重なりおよびスピード超過の防止 */

			_sceVu0ScaleVector( &v, &v, f<0.0f ? 0.2f : f<0.7f ? f : 0.7f ) ;
			_sceVu0SubVector( &o->pos_v, &o->pos_v, &v ) ;
			_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;

			if ( o->act == BRK_PTO_ActNone )
			    o->act = BRK_PTO_Act, o->parts.flag = flag = 1 ;
			if ( p->act == BRK_PTO_ActNone )
			    p->act = BRK_PTO_Act, p->parts.flag = flag = 1 ;
		    }
		}
    return flag ;
}

int BRK_PTO_HzdPotatoOneCheck( Work *work, POTATO *o, float rad )
{
    int     i, flag = 0 ;
    float   f, d ;
    FVECTOR v ;
    POTATO *p ;

    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	if ( p->act && o!=p )
	{
	    SubVector( &v, &p->pos, &o->pos ) ;
	    d = v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ;
	    f = p->target.size.vx + rad ;
	    if ( d <= f*f )
	    {
		f = ( (o->pos_v.vx - p->pos_v.vx) * v.vx +
		      (o->pos_v.vy - p->pos_v.vy) * v.vy +
		      (o->pos_v.vz - p->pos_v.vz) * v.vz) / d ;

		/* 条件は重なり及びスピード超過の防止 */
		_sceVu0ScaleVector( &v, &v, f<0.0f ? 0.1f : f<0.7f ? f : 0.7f ) ;
		_sceVu0SubVector( &o->pos_v, &o->pos_v, &v ) ;
		_sceVu0AddVector( &p->pos_v, &p->pos_v, &v ) ;
		/*諸処なる事情で跳ねさす（転がると蓋に当たるので）*/
		o->pos_v.vy += 8.0f ;
		p->pos_v.vy += 8.0f ;

		if ( o->act == BRK_PTO_ActNone )
		    o->act = BRK_PTO_Act, o->parts.flag = flag = 1 ;
		if ( p->act == BRK_PTO_ActNone )
		    p->act = BRK_PTO_Act, p->parts.flag = flag = 1 ;
	    }
	}
    return flag ;
}

