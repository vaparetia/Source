/*
  CREATE RANDOM POLYGON

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: create.c,v 1.5 2002/04/10 00:52:38 usr04098 Exp $
  */
#ifndef _WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include "alltypes.h"





/*****************************************************************

  初歩的関数
  

 *****************************************************************/
float inner_product( FVECTOR *v0, FVECTOR *v1 )
{
    return v0->vx*v1->vx + v0->vy*v1->vy + v0->vz*v1->vz ;
}

void outer_product( FVECTOR *a, FVECTOR *v0, FVECTOR *v1 )
{
    FVECTOR t ;

    t.vx = v0->vy*v1->vz - v0->vz*v1->vy ;
    t.vy = v0->vz*v1->vx - v0->vx*v1->vz ;
    t.vz = v0->vx*v1->vy - v0->vy*v1->vx ;

    *a = t ;
}

static float backface_culling( int v1, int v2, int v3 )
{
    float a ;

    a  = (vrtx[v1].vx - vrtx[v2].vx)*(vrtx[v3].vy - vrtx[v2].vy) ;
    a -= (vrtx[v1].vy - vrtx[v2].vy)*(vrtx[v3].vx - vrtx[v2].vx) ;
    return a ;
}

float frand()
{
    return 1.0f - random()/((float)0x7fffffff) ;
}





/*****************************************************************

  すべての頂点の中心
  

 *****************************************************************/
static void calc_center_all( FVECTOR *c, FVECTOR *s, FVECTOR *v, int n_vrtx )
{
    int  i ;
    FVECTOR *vtx = v ;

    c->vx = c->vy = c->vz = 0 ;
    for ( i=n_vrtx ; --i>=0 ; v++ )
    {
	c->vx += v->vx ;
	c->vy += v->vy ;
	c->vz += v->vz ;
    }
    c->vx /= n_vrtx ;
    c->vy /= n_vrtx ;
    c->vz /= n_vrtx ;
    s->vx = s->vy = s->vz = 0 ;
    for ( i=n_vrtx, v=vtx ; --i>=0 ; v++ )
    {
	if ( abs( v->vx - c->vx ) > s->vx )
	    s->vx = abs( v->vx - c->vx ) ;
	if ( abs( v->vy - c->vy ) > s->vy )
	    s->vy = abs( v->vy - c->vy ) ;
	if ( abs( v->vz - c->vz ) > s->vz )
	    s->vz = abs( v->vz - c->vz ) ;
    }
}
static void calc_center_poly( Poly *p )
{
    int  i, n_verts ;
    int *v = p->id ;

    n_verts = p->id[3]==-1 ? 3 : 4 ;
    p->center.vx = p->center.vy = p->center.vz = 0.0f ;
    for ( i=n_verts ; --i>=0 ; v++ )
    {
	p->center.vx += vrtx[*v].vx ;
	p->center.vy += vrtx[*v].vy ;
	p->center.vz += vrtx[*v].vz ;
    }
    p->center.vx /= n_verts ;
    p->center.vy /= n_verts ;
    p->center.vz /= n_verts ;
}



/*****************************************************************

  UV計算
  

 *****************************************************************/
static void calc_uvs_poly( Poly *p, FVECTOR *c, FVECTOR *s )
{
    int  i, n_verts ;

    p->uv[3][X] = p->uv[3][Y] = 0 ;
    n_verts = p->id[3]== -1 ? 3 : 4 ;
    for ( i=n_verts ; --i>=0 ; )
    {
	p->uv[i][X] = (vrtx[p->id[i]].vx - c->vx + s->vx)/s->vx/2 * 4096 ;
	p->uv[i][Y] = (vrtx[p->id[i]].vy - c->vy + s->vy)/s->vy/2 * 4096 ;
	p->uv[i][X] = p->uv[i][X]<0 ? 0 : p->uv[i][X]>4096 ? 4096 : p->uv[i][X] ;
	p->uv[i][Y] = p->uv[i][Y]<0 ? 0 : p->uv[i][Y]>4096 ? 4096 : p->uv[i][Y] ;
    }
}



/*****************************************************************

  ある頂点を含むポリゴンを検索
  

 *****************************************************************/
void find_v1_poly( Poly **pool, int v1, Poly *not )
{
    Poly *p = poly ;
    int   i = n_poly ;

    for ( ; --i>=0 ; p++ )
	if ( p != not )
	    if ( p->id[0] == v1 || p->id[1] == v1 || p->id[2] == v1 || p->id[3] == v1 )
		*pool++ = p ;
}

Poly *find_v2_poly( int v1, int v2, Poly *not )
{
    Poly *p = poly ;
    int   i = n_poly ;

    for ( ; --i>=0 ; p++ )
	if ( p != not )
	    if ( p->id[0] == v1 || p->id[1] == v1 || p->id[2] == v1 || p->id[3] == v1 )
		if ( p->id[0] == v2 || p->id[1] == v2 || p->id[2] == v2 || p->id[3] == v2 )
		    return p ;
    return NULL ;
}



/*****************************************************************

  頂点同士の距離
  

 *****************************************************************/
static int cache = 1 ;
static float dist_vtrx( int v1, int v2 )
{
    float dx, dy, dz ;
    static float d[MAX_VRTX][MAX_VRTX] ;

    if ( cache /* ? d[v1][v2]>=0.0f : d[v1][v2]<=0.0f*/ )
    {
	dx = vrtx[v1].vx - vrtx[v2].vx ;
	dy = vrtx[v1].vy - vrtx[v2].vy ;
	dz = vrtx[v1].vz - vrtx[v2].vz ;
	d[v1][v2] = d[v2][v1] = sqrt( dx*dx + dy*dy + dz*dz ) ;
    }
    return d[v1][v2] ;
}


/*****************************************************************

  三角形および四角形の面積
  

 *****************************************************************/
static float calc_triangle( int v0, int v1, int v2 )
{
    float  s, a, b, c ;

    a = dist_vtrx( v0, v1 ) ;
    b = dist_vtrx( v1, v2 ) ;
    c = dist_vtrx( v0, v2 ) ;
    s = ( a + b + c ) / 2.0f ;

    return sqrt( s * (s - a) * (s - b) * (s - c) ) ;
}

static float calc_square( Poly *p )
{
    float  sq ;

    sq  = calc_triangle( p->id[0], p->id[1], p->id[2] ) ;
    if ( p->id[3] != -1 )
	sq += calc_triangle( p->id[1], p->id[2], p->id[3] ) ;

    return sq ;
}




/*****************************************************************

  端の頂点かどうか
  

 *****************************************************************/
int included_in_side( int v1, int v2 )
{
    if ( h_side[v1] != -1 && h_side[v2] != -1 )
	if ( (vrtx[v1].vy >= 0.0f && vrtx[v2].vy >= 0.0f ) ||
	     (vrtx[v1].vy <  0.0f && vrtx[v2].vy <  0.0f ) )
	    return 1 ;
    if ( w_side[v1] != -1 && w_side[v2] != -1 )
	if ( (vrtx[v1].vx >= 0.0f && vrtx[v2].vx >= 0.0f ) ||
	     (vrtx[v1].vx <  0.0f && vrtx[v2].vx <  0.0f ) )
	    return 2 ;
    return 0 ;
}


/*****************************************************************

  頂点をランダムに移動させる
  

 *****************************************************************/
void move_rand_vrtx( int *v )
{
    int i ;
    for ( i=4 ; --i>=0 ; v++ )
	if ( *v != -1 )
	{
	    vrtx[*v].vx += frand()*mergine ;
	    vrtx[*v].vy += frand()*mergine ;
	    //vrtx[*v].vz += frand()*mergine ;
	    if ( h_side[*v] != -1 )
	    {
		vrtx[h_side[*v]] = vrtx[*v] ;
		vrtx[h_side[*v]].vy = -(vrtx[*v].vy > 0 ? height : -height) ;
	    }
	    if ( w_side[*v] != -1 )
	    {
		vrtx[w_side[*v]] = vrtx[*v] ;
		vrtx[w_side[*v]].vy = -(vrtx[*v].vy > 0 ? width : -width) ;
	    }
	}
}


/*****************************************************************

  三角形および四角形ポリゴンの分割
  

 *****************************************************************/
static void devide_4poly( Poly *p )
{
    Poly *pp = &poly[n_poly] ;
    float t, s ;

    if ( p->id[3] != -1 )
    {
#if 0
	float t1, t2 ;

	t1 = calc_triangle( p->id[0], p->id[1], p->id[2] ) ; /*分割面積の差分が少ない方を選ぶ*/
	t2 = calc_triangle( p->id[1], p->id[2], p->id[3] ) ;
	t  = ( t1-t2 ) * ( t1-t2 ) ;
	t1 = calc_triangle( p->id[2], p->id[0], p->id[3] ) ;
	t2 = calc_triangle( p->id[0], p->id[3], p->id[1] ) ;
	s  = ( t1-t2 ) * ( t1-t2 ) ;
#else
	float t1, t2, t3, ss, tt ;

	t1 = dist_vtrx( p->id[0], p->id[1] ) ; /*辺の長さが分散してない方を選ぶ*/
	t2 = dist_vtrx( p->id[1], p->id[2] ) ;
	t3 = dist_vtrx( p->id[0], p->id[2] ) ;
	//t  = ( t1-t2 ) * ( t1-t2 ) + ( t2-t3 ) * ( t2-t3 ) + ( t3-t1 ) * ( t3-t1 ) ;
	tt  = ( t1 + t2 + t3 ) / 3 ;
	t  = (t1-tt) * (t1-tt) + (t2-tt) * (t2-tt) + (t3-tt) * (t3-tt) ;

	t1 = dist_vtrx( p->id[3], p->id[1] ) ;
	t2 = dist_vtrx( p->id[1], p->id[2] ) ;
	t3 = dist_vtrx( p->id[3], p->id[2] ) ;
	//t += ( t1-t2 ) * ( t1-t2 ) + ( t2-t3 ) * ( t2-t3 ) + ( t3-t1 ) * ( t3-t1 ) ;
	tt  = ( t1 + t2 + t3 ) / 3 ;
	t  = (t1-tt) * (t1-tt) + (t2-tt) * (t2-tt) + (t3-tt) * (t3-tt) ;

	t1 = dist_vtrx( p->id[0], p->id[2] ) ;
	t2 = dist_vtrx( p->id[0], p->id[3] ) ;
	t3 = dist_vtrx( p->id[2], p->id[3] ) ;
	//s  = ( t1-t2 ) * ( t1-t2 ) + ( t2-t3 ) * ( t2-t3 ) + ( t3-t1 ) * ( t3-t1 ) ;
	ss  = ( t1 + t2 + t3 ) / 3 ;
	s  = (t1-ss) * (t1-ss) + (t2-ss) * (t2-ss) + (t3-ss) * (t3-ss) ;
	t1 = dist_vtrx( p->id[0], p->id[1] ) ;
	t2 = dist_vtrx( p->id[0], p->id[3] ) ;
	t3 = dist_vtrx( p->id[1], p->id[3] ) ;
	ss  = ( t1 + t2 + t3 ) / 3 ;
	s  = (t1-ss) * (t1-ss) + (t2-ss) * (t2-ss) + (t3-ss) * (t3-ss) ;
	//s += ( t1-t2 ) * ( t1-t2 ) + ( t2-t3 ) * ( t2-t3 ) + ( t3-t1 ) * ( t3-t1 ) ;
#endif
	if ( t > s )
	{
	    *pp = *p ;
	    p->id[0] = pp->id[2] ;/**/
	    p->id[1] = pp->id[0] ;
	    p->id[2] = pp->id[3] ;
	    p->id[3] = pp->id[1] ;
	}
	pp->id[0] = p->id[1] ;
	pp->id[1] = p->id[2] ;
	pp->id[2] = p->id[3] ;

        if ( backface_culling( pp->id[0], pp->id[1], pp->id[2] ) > 0.0f )
            pp->id[3]=pp->id[0], pp->id[0]=pp->id[1], pp->id[1]=pp->id[3] ;
        if ( backface_culling( p->id[0], p->id[1], p->id[2] ) > 0.0f )
            p->id[3]=p->id[0], p->id[0]=p->id[1], p->id[1]=p->id[3] ;
            
	pp->id[3] = p->id[3] = -1 ;
	n_poly++ ;
    }
}

static void devide_3poly( Poly *p )
{
    float a, b, c ;
    int v1, v2, *v, t ;
    Poly *pp ;
    FVECTOR *f ;

    a = dist_vtrx( p->id[0], p->id[1] ) ;
    b = dist_vtrx( p->id[1], p->id[2] ) ;
    c = dist_vtrx( p->id[0], p->id[2] ) ;

    /* 最も長い辺の頂点番号 */
    a>b && a>c ? (v1=p->id[0], v2=p->id[1], v=&p->id[2]) :
    b>a && b>c ? (v1=p->id[1], v2=p->id[2], v=NULL     ) :
	         (v1=p->id[0], v2=p->id[2], v=&p->id[1]) ;

    vrtx[n_vrtx].vx = ( vrtx[v1].vx + vrtx[v2].vx ) / 2.0f + frand()*mergine ;
    vrtx[n_vrtx].vy = ( vrtx[v1].vy + vrtx[v2].vy ) / 2.0f + frand()*mergine ;
    vrtx[n_vrtx].vz = ( vrtx[v1].vz + vrtx[v2].vz ) / 2.0f ;
    if ( v )
	p->id[3] = p->id[0], p->id[0] = *v, *v = p->id[3] ;
    p->id[3] = n_vrtx ;
    devide_4poly( p ) ;

    if ( (t = included_in_side( v1, v2 )) )/* 端の辺かどうかを見る */
    {
	f  = &vrtx[n_vrtx+1] ;
	*f =  vrtx[n_vrtx  ] ;

	if ( t == 1 )/* 縦の端 ？ */
	    f->vy += f->vy<0 ? height : -height, v = h_side ;
	if ( t == 2 )/* 横の端 ？ */
	    f->vx += f->vx<0 ? width  : -width , v = w_side ;
	v[n_vrtx  ] = n_vrtx+1 ;/* 端配列に登録 */
	v[n_vrtx+1] = n_vrtx   ;
	v1 = v[v1] ;            /* 反対側の辺に変更しておく */
	v2 = v[v2] ;
	n_vrtx++ ;              /* 事実上頂点が増えた */
    }
    if ( (pp = find_v2_poly( v1, v2, p )) )/*v1 v2の頂点に相当する p 以外の辺を探す */
    {
	pp->id[3] = pp->id[0] ;
	if      ( v1==pp->id[0] && (v2==pp->id[1] || v2==pp->id[2]) )
	    v = &(v2==pp->id[1] ? pp->id[2] : pp->id[1]), pp->id[0] = *v, *v = pp->id[3] ;
	else if ( v2==pp->id[0] && (v1==pp->id[1] || v1==pp->id[2]) )
	    v = &(v1==pp->id[1] ? pp->id[2] : pp->id[1]), pp->id[0] = *v, *v = pp->id[3] ;
	pp->id[3] = n_vrtx ;
	devide_4poly( pp ) ;
    }
    n_vrtx++ ;
    //move_rand_vrtx( p->id ) ;
}



/*****************************************************************

  ポリゴンの分割
  

 *****************************************************************/
void create_polygon( Xdir *x )
{
    Poly    *p ;
    float t, max ;
    int  i, j ;

    /* initilize polygon data */
    vrtx[0].vx =  width/2 ; vrtx[0].vy =  height/2 ; vrtx[0].vz = 0 ;
    vrtx[1].vx =  width/2 ; vrtx[1].vy = -height/2 ; vrtx[1].vz = 0 ;
    vrtx[2].vx = -width/2 ; vrtx[2].vy =  height/2 ; vrtx[2].vz = 0 ;
    vrtx[3].vx = -width/2 ; vrtx[3].vy = -height/2 ; vrtx[3].vz = 0 ;
    n_vrtx = 4 ;
    poly[0].id[0] = 0 ; poly[0].id[1] = 1 ; poly[0].id[2] = 2 ; poly[0].id[3] = 3 ;
    n_poly = 1 ;

    /* initialize side vertexes */
    for ( j=MAX_VRTX ; --j>=0 ; )
	w_side[j] = h_side[j] = -1 ;
    w_side[0] = 2 ; w_side[2] = 0 ;
    w_side[1] = 3 ; w_side[3] = 1 ;
    h_side[0] = 1 ; h_side[1] = 0 ;
    h_side[2] = 3 ; h_side[3] = 2 ;

    for ( j=lvl ; --j>=0 ; )
    {
	max = 0.0f ;
	p = &poly[0] ;
	for( i=n_poly ; --i>=0 ; )
	{
	    t = calc_square( &poly[i] ) ;
	    if ( max < t )
		max = t, p=&poly[i] ;
	}
	devide_4poly( p ) ;
	devide_3poly( p ) ;
    }

    calc_center_all( &x->center, &x->size, vrtx, n_vrtx ) ;
    for ( i=n_poly ; --i>=0 ; )
    {
	poly[i].num = i ;
	calc_center_poly( &poly[i] ) ;
	calc_uvs_poly( &poly[i],  &x->center, &x->size ) ;
    }


#if 0
    printf( "sz %f %f\n", x->size.vx, x->size.vy ) ;
    for ( i=n_poly ; --i>=0 ; )
	printf( "p[%d]", i ),
	    printf( "%d %d %d %d\n", poly[i].id[0],poly[i].id[1],poly[i].id[2],poly[i].id[3]  ) ;
    for ( i=n_vrtx ; --i>=0 ; )
	printf( "v[%d]", i ),
	    printf( "%f %f %f\n", vrtx[i].vx,vrtx[i].vy,vrtx[i].vz  ) ;
    for ( i=n_vrtx ; --i>=0 ; )
	printf( " h[%2d]%3d w[%2d]%3d\n", i,h_side[i], i,w_side[i] ) ;
#endif
    printf( "%d verts %d ploys\n", n_vrtx, n_poly ) ;
}
