/*
	ストリーム頂点アニメ

	出力用関数群

	T.Morita  Feb 21 2000
	$Id: create.c,v 1.7 2001/11/30 04:12:08 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "alltypes.h"
#include "allvars.h"


static int make_vertex()
{
    int     i, j ;
    ANIM   *a ;
    LIST   *l ;

    for ( l=list ; l ; l=l->next )
	if ( l->n_mesh )
	    for ( j=l->n_anime, a=l->anime ; --j>=0 ; a++ )
	    {
		a->verts = calloc( sizeof(short) * 3, n_frame ) ;
		for ( i=0 ; i<n_frame ; i++ )
		{
		    a->verts[i*XYZ+X] = (short)(a->org[i].vx * scaling) ;
		    a->verts[i*XYZ+Y] = (short)(a->org[i].vy * scaling) ;
		    a->verts[i*XYZ+Z] = (short)(a->org[i].vz * scaling) ;
		}
	    }
    printf( "Successfully vertexes are made.\n" ) ;

    return 0 ;
}

static void set_normal_from_vertex( FVECTOR *n, int offset, FVECTOR *v, float l )
{
    if ( offset >= 0 )
    {
	n += offset ;
	n->vx += v->vx/l ;
	n->vy += v->vy/l ;
	n->vz += v->vz/l ;
	n->vw += 1.0f ;
    }
}

static void calc_normal_from_vertex( FVECTOR *base, int n0,  int n1 , int n2 , int n3,
				     FVECTOR *v00, FVECTOR *v10, FVECTOR *v20 )
{
    FVECTOR  a0, a1, v ;
    float    l ;

    a0.vx = v00->vx - v10->vx ;    a1.vx = v10->vx - v20->vx ;
    a0.vy = v00->vy - v10->vy ;    a1.vy = v10->vy - v20->vy ;
    a0.vz = v00->vz - v10->vz ;    a1.vz = v10->vz - v20->vz ;

    v.vx  = a0.vz*a1.vy - a0.vy*a1.vz ;
    v.vy  = a0.vx*a1.vz - a0.vz*a1.vx ;
    v.vz  = a0.vy*a1.vx - a0.vx*a1.vy ;

    l = sqrt( v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ) ;
    set_normal_from_vertex( base, n0, &v, l ) ;
    set_normal_from_vertex( base, n1, &v, l ) ;
    set_normal_from_vertex( base, n2, &v, l ) ;
    set_normal_from_vertex( base, n3, &v, l ) ;
}

static int  make_normal()
{
    int      i, j ;
    ANIM   **idx, *a ;
    POLY    *p ;
    LIST    *l ;
    FVECTOR *f ;

    for ( l=list ; l ; l=l->next )
	if ( l->n_mesh )
	{
	    idx = calloc( sizeof(ANIM * ), l->n_mesh ) ;
	    f   = calloc( sizeof(FVECTOR), l->n_mesh ) ;

	    /* 法線情報のメモリ確保 */
	    for ( i=l->n_anime, a=l->anime ; --i>=0 ; a++ )
		a->norms = malloc( sizeof(short) * 3 * n_frame ) ;

	    /* mesh->ANIM へのインデックス */
	    for ( j=0 ; j<l->n_mesh ; j++ )
	    {
		for ( i=l->n_anime, a=l->anime ; --i>=0 ; a++ )
		    if ( a->vert_id == j )
		    {
			idx[j] = a ;
			break ;
		    }
		if ( i<0 )
		    printf( "Vertex NOT FOUND !!! Are you OK?\n" ) ;
	    }

	    for ( i=0 ; i<n_frame ; i++ )
	    {
		bzero( f, sizeof(FVECTOR) * l->n_mesh ) ;

		/* このフレームでの頂点から各ポリゴンの法線を計算し,各
		   頂点の法線に足し込む f[?].vw に法線を足した数が入る */
		for ( j=0, p=l->poly ; j<l->n_poly ; j++, p++ )
		{
		    //printf( "<<%d>>v0[%d](%x)v1[%d](%x)v2[%d](%x)v3[%d](%x)%x\n", j,   p->v[0],&f[p->v[0]],   p->v[1],&f[p->v[1]],    p->v[2],&f[p->v[2]],     p->v[3],&f[p->v[3]], idx[380] ) ;

		    calc_normal_from_vertex( f, p->v[0], p->v[1], p->v[2], p->v[3],
					     &idx[p->v[0]]->org[i],
					     &idx[p->v[1]]->org[i],
					     &idx[p->v[2]]->org[i] ) ;
		    //printf( "v0(380)%x\n",idx[380] ) ;
		}
		/* 平均を出し,ANIMに結果を反映させる */
		for ( j=0 ; j<l->n_mesh ; j++ )
		{
		    idx[j]->norms[XYZ*i+X] = (short)(f[j].vx / f[j].vw * 4096.0f) ;
		    idx[j]->norms[XYZ*i+Y] = (short)(f[j].vy / f[j].vw * 4096.0f) ;
		    idx[j]->norms[XYZ*i+Z] = (short)(f[j].vz / f[j].vw * 4096.0f) ;
		}
	    }
	    free( idx ) ;
	    free( f   ) ;
	}
    printf( "Successfully normals are made.\n" ) ;
    return 0 ;
}

static int make_non_moving_anime()
{
    int      i, j, vid ;
    ANIM    *idx, *a ;
    LIST    *l ;
    FVECTOR *f ;

    for ( l=list ; l ; l=l->next )
	if ( l->n_mesh )
	    for ( j=0 ; j<l->n_mesh ; j++ )
	    {
		idx = NULL ;
		vid = -1   ;
		for ( i=l->n_mesh, a=l->anime ; --i>=0 ; a++ )
		    if ( a->vert_id == j )
			vid = j ;
		    else if ( a->vert_id == -1 )
			idx = a ;
		if ( vid == -1 && idx ) /* アニメしない頂点を発見 */
		{
		    idx->vert_id = j ;
		    idx->org     = f = malloc( n_frame * sizeof(FVECTOR) ) ;
		    for( i=n_frame ; --i>=0 ; f++ )
			*f = l->mesh[j] ;
		    l->n_anime++ ;
		}
		else if ( vid == -1 ) /* 正常な動作ならありえない */
		{
		    fprintf( stderr, "Too much vertex (move + non-move > n_mesh) vid%d->%d\n",
			     vid, j ) ;
		    return -1 ;
		}
	    }

     printf( "Successfully non-movings are made.\n" ) ;
     return 0 ;
}

int make_datas()
{
    if ( make_non_moving_anime() < 0 )
	return -1 ;
    if ( flag & FLG_VERTEX )
	if ( make_vertex() < 0 )
	    return -1 ;
    if ( flag & FLG_NORMAL )
	if ( make_normal() < 0 )
	    return -1 ;

    return 0 ;
}

