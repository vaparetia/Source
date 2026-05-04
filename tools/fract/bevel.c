/*
  CREATE RANDOM POLYGON

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: bevel.c,v 1.5 2002/06/14 05:21:25 usr04098 Exp $
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

static void calc_normal_from_vertex( SVECTOR *n0 , SVECTOR *n1 , SVECTOR *n2 ,
                                     SVECTOR *v00, SVECTOR *v10, SVECTOR *v20 )
{
    FVECTOR a0, a1, v ;
    float l ;

    a0.vx = v00->vx - v10->vx ; a1.vx = v10->vx - v20->vx ;
    a0.vy = v00->vy - v10->vy ; a1.vy = v10->vy - v20->vy ;
    a0.vz = v00->vz - v10->vz ; a1.vz = v10->vz - v20->vz ;
    v.vx  = a0.vz*a1.vy - a0.vy*a1.vz ;
    v.vy  = a0.vx*a1.vz - a0.vz*a1.vx ;
    v.vz  = a0.vy*a1.vx - a0.vx*a1.vy ;
    l = sqrt( v.vx*v.vx + v.vy*v.vy + v.vz*v.vz ) ;
#ifdef _XBOX
    l /= 32767.0f ;
#else
    l /= 4096.0f ;
#endif

    n0->vx = n1->vx = n2->vx = v.vx/l ;
    n0->vy = n1->vy = n2->vy = v.vy/l ;
    n0->vz = n1->vz = n2->vz = v.vz/l ;
}

/*****************************************************************

  ベヴェル処理
  

 *****************************************************************/
int create_bevel( Poly *p,
		  SVECTOR *v, SVECTOR *n, short *u,
#ifdef _XBOX
		  short *idx,
#endif
		  FVECTOR *cen, int depth, int axis )
{
#ifdef _XBOX
    static unsigned int v_id[] = {
	(0x8000 |(3<<8)| 0), (0x8000 |(4<<8)| 1),
	(0x4000 |(0<<8)| 0), (0x4000 |(1<<8)| 1),
	(0x4000 |(2<<8)| 2), (0xc000 |(4<<8)| 0),
	(0xc000 |(5<<8)| 1), (0xc000 |(3<<8)| 2),
	(0x0000 |(2<<8)| 2), (0x0000 |(0<<8)| 0),
    } ;
#else
    static unsigned int v_id[] = {
       (0x8000 | 0), (0x8000 | 1), (0x0000 | 0), (0x0000 | 1), (0x0000 | 2),
       (0x8000 | 1), (0x8000 | 2), (0x8000 | 0), (0x0000 | 2), (0x0000 | 0),
    } ;
#endif
    int n_verts = 0 ;
    int i, *id = v_id ;
    int idx_num, n_idx ;

    n_idx = 0 ;
    for ( i=sizeof(v_id)/sizeof(int) ; --i>=0 ;  id++ )
    {
	FVECTOR *t ;
printf( "\n" ) ;

	idx_num = (*id & 0xf) ;

#ifdef _XBOX
	if ( idx ) {
	    *idx++ = (*id >> 8) & 0xf ;
#ifdef _XBOX
printf( "%d - ", *(idx-1 )  ) ;
#endif

	    n_idx++ ;
	}
	if ( !(*id & 0x4000) )
	    continue ;
#endif
	t = &vrtx[p->id[idx_num]] ;
	v->vx = t->vx - cen->vx + (axis!=X ? 0 : *id&0x8000 ? -depth : depth) ;
	v->vy = t->vy - cen->vy + (axis!=Y ? 0 : *id&0x8000 ? -depth : depth) ;
	v->vz = t->vz - cen->vz + (axis!=Z ? 0 : *id&0x8000 ? -depth : depth) ;

#ifdef _XBOX
printf( "(%d %d %d)", v->vx, v->vy, v->vz  ) ;
#endif

	if ( depth > 0 )
	    u[X] = p->uv[idx_num][X], u[Y] = p->uv[idx_num][Y] ;
	if ( !(n->vw & 0x8000) )
	    calc_normal_from_vertex( n, n-1, n-2, v, v-1, v-2 ) ;
	n_verts++ ;
	v++ ;
	n++ ;
	u += XY ;
    }

printf( "nv%d n_idx%d %d\n", n_verts, n_idx, depth ) ;
    return (n_verts << 0) | (n_idx << 16) ;
}
