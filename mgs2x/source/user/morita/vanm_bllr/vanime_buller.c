/*
   vanime_buller.c
   頂点アニメブラー

   2000/04/15 T. Morita
   $Id: vanime_buller.c,v 1.1.1.3 2002/11/19 11:46:36 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "vanime_buller.h"



static void Die( Work *work )
{
    if ( work->anime )
	ExitVertexAnimation( work->anime ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static void Act( Work *work )
{
    FMATRIX *mtx ;

    if ( (work->rot += work->rot_v) > (float)M_PI )
        work->rot -= 2.0f*(float)M_PI ;
    mtx = &work->objs->world ;
    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, work->rot ) ;
    _sceVu0MulMatrix( mtx, &work->world, mtx ) ;
    _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
    TransMatrix( mtx, &work->pos ) ;

    SimpleVertexAnimation( work->anime ) ;

    if ( work->flag & 1 )
	AN_Test_Eye2( &work->pos, 1 ), AN_Test_Eye2( &work->dst, 1 ) ;

    if ( work->anime->count == 0 )
    {
        work->anime->p[work->key] = 0.0f ;
	if ( ++work->key >= work->n_key )
	{
	    if ( !work->flag )
		GV_DestroyActor( work ) ;
	    else if ( work->flag == 2 )
	    {
		work->anime->p[0] = 1.0f ;
		SimpleVertexAnimation( work->anime ) ;
		work->anime->p[0] = 0.0f ;
		work->key = 1 ;
	    }
	    else
		work->key = 0 ;
	}
        work->anime->p[work->key] = 1.0f ;
        work->anime->count = work->base + ( irnd() & ( (1 << work->rand) - 1) ) ;
    }
}

static int GetResourcesCall( Work *work, int id, float rot_v, float speed,
			     FVECTOR *pos, FVECTOR *pos_v, int base, int rand, int flag )
{
    int      i ;
    CV2_DEF *cdef ;
    DG_DEF  *def  ;
    extern void BIG_VectoMat( FMATRIX *, FVECTOR *, FVECTOR *, int ) ;

    _sceVu0CopyVector( &work->pos, pos   ? pos   : &DG_ZeroVector ) ;
    _sceVu0CopyVector( &work->dst, pos_v ? pos_v : &DG_ZeroVector ) ;

    if ( !id )
	PERROR( "No model ID!!(-m option missing) :: NewVanimeBuller\n" ) ;
    if ( !(cdef = GV_GetCache( GV_CacheID( id, 'c' ) )) )
	PERROR( "No CV2 found in data.cnf!! :: NewVanimeBuller\n" ) ;
    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS found in data.cnf!! :: NewVanimeBuller\n" ) ;
    def->n_models = def->n_x_models = 1 ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewVanimeBuller\n" ) ;
    DG_QueueObjs( work->objs ) ;

    if ( (work->flag = flag) != 3 )
	GV_SetActorChild( work, NewOpticalCamouflage( work->objs, flag ) ) ;
    if ( pos_v )
    {
	_sceVu0SubVector( &work->pos_v, pos_v, &work->pos ) ;
	_sceVu0Normalize( &work->pos_v, &work->pos_v ) ;
	BIG_VectoMat( &work->world, &work->pos_v, &DG_ZeroVector, 0x0012 ) ;
	_sceVu0ScaleVector( &work->pos_v, &work->pos_v, speed ) ;
    }
    else
	work->world = DG_UnitMatrix ;

    /* 回転速度 */
    work->rot_v = rot_v / 180.0f * (float)M_PI ;
    /* フレームコントロール初期化 */
    work->base = base ;
    work->rand = rand ;

    /* アニメーション初期化 */
    if ( !(work->anime = InitVertexAnimation( work->objs->objs,
					      cdef->models,
					      DG_VANIME_VERTS|DG_VANIME_NORMS,
					      cdef->n_models )) )
	PERROR( "InitVertexAnimation() returns NULL!! :: NewVanimeBuller\n" ) ;

    for ( i=0 ; i<cdef->n_models ; i++ )
	if ( cdef->models[i].n_verts == cdef->models[0].n_verts )
	    work->anime->key[i] = &cdef->models[i], work->anime->p[i] = 0.0f ;
	else
	    PERROR( "Animation Vertexies are not same!! :: NewVanimeBuller\n" ) ;
    work->anime->p[0] = 1.0f ;
    work->key   = 0 ;
    work->n_key = cdef->n_models ;

    work->root =  NULL ;

    return 0 ;
}


static int GetResources( Work *work )
{
    int     buf[3], flag = 0 ;
    FVECTOR pos, pos_v ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
	flag |= 1 ;
    }
    if ( GCL_GetOption( 'd' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &pos_v ) ;
	flag |= 2 ;
    }
    if ( GetResourcesCall( work,
			   GCL_GetOptionValue( 'm', 0 ),
			   GCL_GetOptionValue( 'r', 0 ),
			   GCL_GetOptionValue( 's', 0 ),
			   flag&1 ? &pos : NULL , flag&2 ? &pos_v : NULL,
			   GCL_GetOptionValue( 'b', 1 ),
			   GCL_GetOptionValue( 'w', 0 ),
			   GCL_GetOptionValue( 'f', 0 ) ) )
	return -1 ;
    return 0 ;
}

void *NewVanimeBullerCall( int id, float rot_v, float speed,
			   FVECTOR *pos, FVECTOR *pos_v, int base, int rand, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResourcesCall( work, id, rot_v, speed, pos, pos_v, base, rand, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewVanimeBuller( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
