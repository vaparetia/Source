//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libdg.h"

#include "../include/libdg_x.h"
#include "../include/util.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#include  "../include/util.h"

typedef struct _Work {
    GV_ACT_EX actor   ;
	    
    DG_OBJS *shadow ;
    OBJECT  *body   ;
    CONTROL *control;
    FMATRIX *lights ;
    int     *flag   ;
} Work ;

static void MakePosition( Work *work, DG_OBJS *src, DG_OBJS *dst, int i, FMATRIX *drop )
{
    FVECTOR branch ;
    DG_MDL *m ;
    FVECTOR pos = DG_ZeroVector ;
    FVECTOR *ans = (FVECTOR *)&dst->objs[i].world.m[3] ;

    m = &src->def->models[i] ;
    for ( i=m->parent ; i!=-1 ; i=m->parent )
    {
	_sceVu0ApplyMatrix( &branch, &src->objs[i].world, (FVECTOR *)&m->tx ) ;
	_sceVu0AddVector( &pos, &pos, &branch ) ;
	m = &src->def->models[i] ;
    }
    _sceVu0ApplyMatrix( &pos, drop, &pos ) ;
    _sceVu0AddVector( ans, &pos, (FVECTOR*)&work->shadow->world.m[3] ) ;
    ans->vw = 1.0f ;
}

static void Act( Work *work )
{
    int i ;
    FMATRIX drop_shadow ;
    DG_OBJ *o = work->shadow->objs ;
    DG_OBJ *p = work->body->objs->objs ;
    static FVECTOR lgt = { 0.0f, 1.0f, 0.0f, 0.0f } ;

    if ( work->flag ? *work->flag : 1 )
	DG_VisibleObjs( work->shadow ) ;
    else
	DG_InvisibleObjs( work->shadow ) ;

    work->shadow->world = work->body->objs->world ;
//    if ( work->control->grounded & 1 ) {
//	work->shadow->world.m[3][Y] = work->control->mov.vy - work->control->height + 30.0f  ;
//    } else {
	work->shadow->world.m[3][Y] = work->control->levels[ 0 ] + 30.0f ;
//    }
    _sceVu0DropShadowMatrix( &drop_shadow,
			     &lgt,/* 光源方向 (FVECTOR*)&work->lights[0].m[3],*/
			     work->control->level[0]->p1.h,
			     work->control->level[0]->p3.h,
			     work->control->level[0]->p2.h,
			     0/*平行光源*/ ) ;

    for ( i=0  ;  i<21  ; i++, o++, p++ )
	_sceVu0MulMatrix( &o->world, &drop_shadow, &p->world ),
	    MakePosition( work, work->body->objs, work->shadow, i, &drop_shadow ) ;
}

static void Die( Work *work )
{
    if ( work->shadow )
	DG_DequeueObjs( work->shadow ), DG_FreeObjs( work->shadow ) ;
}

static int GetResources( Work *work, OBJECT *body, CONTROL *control,  FMATRIX *lights, int *flag )
{
    static FMATRIX lgt_mtx[2] = {
	{ {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}} },
	{ {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}} }
    } ;

    work->control = control;
    work->lights  = lights ;
    work->flag    = flag   ;
    work->body    = body   ;

    if ( !(work->shadow = DG_MakeObjs( body->objs->def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SEMITRANS|DG_FLAG_FORCEMSAA, 0 )) )
	PERROR( "Can't make DG_OBJS(May be no memory) : NewDropShadow\n" ) ;
    DG_QueueObjs( work->shadow ) ;
    DG_SetLightMatrix( work->shadow, lgt_mtx ) ;

#if 0
    {
	DG_SUBPACK *sub[2] ;
	DG_OBJ *o = work->shadow->objs ;
	int     i, j ;

	o = work->shadow->objs ;
	for( i=work->shadow->def->n_models ; --i>=0 ; o++ )
	{
	    sub[0] = o->sub_packet[0] ;
	    sub[1] = o->sub_packet[1] ;
	    for( j=o->n_packs ; --j>=0 ; sub[0]++, sub[1]++ )
		sub[1]->packet.vu1_param.alpha.data = sub[0]->packet.vu1_param.alpha.data =
		    SCE_GS_SET_ALPHA(0,1,2,1, 20 ) ;
	}
    }
#endif

    return 0 ;
}


void *NewDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &( work->actor ), Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( (GetResources( work, body, control, lights, flag ) < 0) && body && control && lights )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return ( void * )work ;
}
