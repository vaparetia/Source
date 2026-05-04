//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_har.c 
   オルガ オルガ髪の毛

   1999/12/21 T.Morita
   $Id: orga_har.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct holowork_t
{
    GV_ACT    actor ;

    FMATRIX  *world ;

    int       key   ;
    int       n_key ;
    DG_OBJS  *objs  ;
    VERTEX_ANIME_WORK *anime ;
} Work ;


static void Die( Work *work )
{
    ExitVertexAnimation( work->anime ) ;
    DG_DequeueObjs( work->objs ) ;
    DG_FreeObjs( work->objs );
}

static void Act( Work *work )
{
    work->objs->world = *work->world ;

    SimpleVertexAnimation( work->anime ) ;

    if ( work->anime->count == 0 )
    {
        work->anime->p[work->key] = 0.0f ;
	if ( ++work->key >= work->n_key )
	    work->key = 0 ;
        work->anime->p[work->key] = 1.0f ;
        work->anime->count = 8 + (irnd()&15) ;
    }
}

static int GetResources( Work *work, FMATRIX *world, int id, FMATRIX *lights ) 
{
    int   i ;
    CV2_DEF *cdef = GV_GetCache( GV_CacheID( id, 'c' ) ) ;
    DG_DEF  *def  = GV_GetCache( GV_CacheID( id, 'k' ) ) ;

    def->n_models = def->n_x_models = 1 ;

    work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
    DG_QueueObjs( work->objs ) ;
    work->objs->world = *world ;
    DG_SetLightMatrix( work->objs, lights ) ;
    work->anime = InitVertexAnimation( work->objs->objs,
				       cdef->models,
				       DG_VANIME_VERTS,
				       cdef->n_models ) ;
    for ( i=0 ; i<cdef->n_models ; i++ )
	work->anime->key[i] = &cdef->models[i], work->anime->p[i] = 0.0f ;
    work->anime->p[0] = 1.0f ;
    work->key   = 0 ;
    work->n_key = cdef->n_models ;
    work->world = world ;

    return 0 ;
}

void *NewOrgaHair( FMATRIX *world, int id, FMATRIX *lights )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, world, id, lights ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
