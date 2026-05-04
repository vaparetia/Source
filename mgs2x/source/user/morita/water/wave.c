#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "libdg.h"
#include "gameheader.h"

#define _MAIN_WAVE_
#include "wave.h"



int InitRipple( Work *w )
{
    int    i ;
    FVECTOR *v ;

    w->r0.h   = 0.4F ;
    w->r0.aa  = 5    ;
    w->r0.a1  = 256*6  ;
    w->r0.a2  = 256*12 ;
    w->r0.ws  = 1    ;
    w->r0.a0  = 0    ;
    w->r0.a   = 0    ;
    w->r0.top = 0    ;

    w->r1.h   = 0.2F ;
    w->r1.aa  = 8    ;
    w->r1.a1  = 256*12 ;
    w->r1.a2  = 256*12 ;
    w->r1.ws  = 1    ;
    w->r1.a0  = 0    ;
    w->r1.a   = 0    ;
    w->r1.top = 0    ;
 
    /* initialize distance table */
#define R  10.024F
#define R2 20.048F
    v = &wave_vtxs[i=num_vtxs-1] ;
    for( ; i>=0 ; i--, v-- )
    {
        w->r0.d_tbl[i] = sqrtf( v->vx   * v->vx    +  v->vz   * v->vz   )/(R *1.41421356F)*NRING ;
	w->r1.d_tbl[i] = sqrtf((v->vx+R)*(v->vx+R) + (v->vz+R)*(v->vz+R))/(R2*1.41421356F)*NRING ;
    }
}
#endif




static void Act( Work *work )
{
    work->body.objs->world.m[3][1] = 1000.0f  ;
    work->body.objs->world.m[3][2] = 18500.0f ;
//    work->body.objs->world.m[1][3] = 1000.0f  ;
//    work->body.objs->world.m[2][3] = 18500.0f ;
//    ActRipple( work ) ;
//    ChngTexture( work ) ;
}

static void Die( Work *work )
{
    GM_FreeObject( &work->body ) ;
}

static int GetResources( Work *work )
{
    InitRipple( work ) ;

    return 0 ;
}

void *NewWaterWave( int name, int where )
{
    Work *work ;
    
    if ( (work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}




