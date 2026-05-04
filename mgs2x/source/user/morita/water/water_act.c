/*
	action.c
	    波アクション

	1999/08/10 T.Morita
	$Id: water_act.c,v 1.1.1.3 2002/11/19 11:46:38 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>

#include	"libutl.h"
#include	"gameheader.h"

#include	"water.h"


#include "../wave/wave_sin.h"

static void WTR_ActWater( Work *w )
{
    int x, z, bf ;
    short *mp, *mp1 ;
    MDL *m = &w->m_wave ;
    int ofst = 1024/(m->col-1) ;
    SVECTOR v ;
    SVECTOR *vt0, *vt1, *nr0, *nr1 ;
    short *vtx, *uvs, *uv0, *uv1, uv ;
    float scale = (m->tex->v_scale*256.0f/255.0f) ;
    float offset = m->tex->v_offset-scale*0.5f/256.0f ;

    bf = w->wave0*1024/w->speed ;
    w->wave0 = (w->wave0+1)%w->speed ;

    mp = &m->map[2*m->row*m->col] ;
    vtx = m->vtx[DG_Clock] ;
    uvs = m->uvs[DG_Clock]+Y ;
    for ( z=m->row ; --z>0 ; )
    {
	mp1 = mp ;
	uv = 0 ;
	for (  x=m->col ; --x>0 ; )
	{
	    vt0 = (SVECTOR*)(vtx + *--mp) ; nr0 = (SVECTOR*)(m->nrm + *mp) ; *(uvs + *mp/2) = uv ;
	    vt1 = (SVECTOR*)(vtx + *--mp) ; nr1 = (SVECTOR*)(m->nrm + *mp) ; *(uvs + *mp/2) = uv ;

	    vt1->vx = (4096*WATR_SIZE/m->col)*x + (M_SIN(bf +     x*ofst)+
						   M_SIN(bf + (x+z)*ofst))/4 ;
	    v.vx = vt1->vx - vt0->vx ;
	    vt0->vx = vt1->vx ;
	    vt1->vy = -(M_COS( bf-125+  x*ofst ) + M_COS(-bf+125+    z*ofst ) +
			M_COS( bf-125+2*z*ofst ) + M_COS( bf-125+(x+z)*ofst ))/6 ;
	    v.vy = vt1->vy - vt0->vy ;
	    vt0->vy = vt1->vy ;
	    vt1->vz = (4096*WATR_SIZE/m->col)*z + (M_SIN(-bf +     z*ofst)+
						   M_SIN( bf +   2*z*ofst)+
						   M_SIN( bf + (z+x)*ofst) )/4 ;
	    v.vz = vt1->vz - vt0->vz ;
	    vt0->vz = vt1->vz ;

            nr0->vz = nr0->vy = nr1->vz = nr1->vy = 90-v.vy/2>0 ? 90-v.vy/2:0 ;
            nr0->vx = nr1->vx = nr1->vz*nr1->vz/128 ;
	}
	*(vtx + *--mp) = *(vtx + *--mp1)-WATR_BASE*WATR_SIZE ;
	*(vtx + *mp+Y) = *(vtx + *mp1+Y) ;
	*(vtx + *mp+Z) = *(vtx + *mp1+Z) ;
	*(m->nrm + *mp+R) = *(m->nrm + *mp1+R) ;
	*(m->nrm + *mp+G) = *(m->nrm + *mp1+G) ;
	*(m->nrm + *mp+B) = *(m->nrm + *mp1+B) ;
	*(m->nrm + *mp+K) = 0x8fff ;
	*(uvs + *mp/2) = uv ;
	*(vtx + *--mp) = *(vtx + *--mp1)-WATR_BASE*WATR_SIZE ;
	*(vtx + *mp+Y) = *(vtx + *mp1+Y) ;
	*(vtx + *mp+Z) = *(vtx + *mp1+Z) ;
	*(m->nrm + *mp+R) = *(m->nrm + *mp1+R) ;
	*(m->nrm + *mp+G) = *(m->nrm + *mp1+G) ;
	*(m->nrm + *mp+B) = *(m->nrm + *mp1+B) ;
	*(m->nrm + *mp+K) = 0x8fff ;
	*(uvs + *mp/2) = uv ;
    }
    for (  x=m->col ; --x>=0 ; mp-- )
	*(vtx + *--mp+Z) -= WATR_BASE*WATR_SIZE, *(uvs + *mp/2) = uv ;
}

void WTR_ActAllWater( Work *work )
{
    WTR_ActWater( work ) ; /* must be first */
}
